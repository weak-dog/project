#include "csp.h"
#include "ui_csp.h"

Csp::Csp(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Csp)
{
    type=0;
    ui->setupUi(this);
    //连接数据库
    if(QSqlDatabase::contains("qt_sql_default_connection"))
        database = QSqlDatabase::database("qt_sql_default_connection");
    else
        database=QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName("csp.db");
    if(!database.open()){
        qDebug()<<"Error: Fail to connect database"<<database.lastError();
    }else{
        qDebug()<<"Succeed to connect database";
    }
    sql_query=QSqlQuery(database);

    firstUpload=true;
    secondButUpload=false;
    //初始化套接字
    tcpServer1=new QTcpServer(this);
    tcpServer2=new QTcpServer(this);
    tcpServer3=new QTcpServer(this);
    tcpSocket1=new QTcpSocket(this);//客户端消息
    tcpSocket2=new QTcpSocket(this);//ks消息
    tcpSocket3=new QTcpSocket(this);//客户端文件
    //绑定监听
    tcpServer1->listen(QHostAddress::Any,10002);
    tcpServer2->listen(QHostAddress::Any,10003);
    tcpServer3->listen(QHostAddress::Any,10004);

    //监听客户端连接(消息端口)
    connect(tcpServer1,&QTcpServer::newConnection,
            [=]()
            {
                //取出建立好的套接字
                tcpSocket1=tcpServer1->nextPendingConnection();
                QString ip=tcpSocket1->peerAddress().toString();
                quint16 port=tcpSocket1->peerPort();
                QString str=QString("[%1:%2]成功连接(消息)").arg(ip).arg(port);
                ui->textEdit->append(str);
                //接受客户端消息
                connect(tcpSocket1,&QTcpSocket::readyRead,
                        [=]()
                {
                    QByteArray array=tcpSocket1->readAll();
                    messages.append(QString(array));
                    qDebug()<<"从客户端收到消息："+QString(array);
                    QStringList splitArray=QString(array).split("@");
                    ui->textEdit->append("从客户端收到消息: "+QString(array));
                    if(messages.size()==1){
                        //收到的是命令，查找第三部分判断指令种类
                        if(splitArray[2]=="login"){//登录
                            ui->textEdit->append("客户端请求登录");
                            QString account=splitArray[0];
                            QString password=splitArray[1];
                            bool result=checkLogin(account,password);
                            if(result){//登录成功
                                tcpSocket1->write(QString("success").toUtf8().data());
                                ui->textEdit->append("登录成功");
                            }else{
                                tcpSocket1->write(QString("failure").toUtf8().data());
                                ui->textEdit->append("登录失败");
                            }
                            messages.clear();
                        }else if(splitArray[2]=="upload"){//上传
                            type=2;
                            qDebug()<<"客户端请求上传文件";
                            fileSize=splitArray[3].toInt();
//                            qDebug()<<"filesize"<<fileSize;
                            fileSh=splitArray[4];
//                            qDebug()<<"fileSh"<<fileSh;
                            bool isFirstUpload=checkFirst(fileSh);//是否为首次上传
                            if(isFirstUpload==true){//首次上传
                                qDebug()<<"首次上传";
                                ui->textEdit->append("首次上传");
                                firstUpload=true;
                                tcpSocket1->write(QString("first").toLatin1());
                                qDebug()<<"发送成功";
                            }else{//后续上传
                                //查询数据库得到{Cs}
                                QStringList Css=getCs(fileSh);
                                QString CS;
                                for(int i=0;i<Css.size()-1;i++)CS+=(Css[i]+"#");
                                CS+=Css[Css.size()-1];
                                firstUpload=false;
                                QString res_CS=QString("second@%1").arg(CS);
                                qDebug()<<"res_Cs"<<res_CS;
                                tcpSocket1->write(res_CS.toUtf8().data());
                            }
                        }else if(splitArray[2]=="download"){//下载，收到@@download@token
                            type=3;
                            qDebug()<<"-----------客户端请求下载文件";
                            token=splitArray[3];
                            //从数据库查找ptr
                            sql_query.exec(QString("select ptr from files where token='%1'").arg(token));
                            if(sql_query.next()){
                                ptr=sql_query.value(0).toString();
                            }
                            if(ptr!="new"){
                                token=ptr;
                                qDebug()<<"token更新";
                            }
                            //从数据库查找Cs
                            sql_query.exec(QString("select cs,krrd from files where token='%1'").arg(token));
                            if(sql_query.next()){
                                Cs=sql_query.value(0).toString();
                                krrd=sql_query.value(1).toString();
                            }
                            filePath="/home/weakdog/cspfiles/"+token;
                            sendSize=0;
                            QFileInfo info(filePath);
                            fileSize=info.size();
                            ui->textEdit->append("filesize:"+QString::number(fileSize));
                            file.setFileName(filePath);
                            file.open(QIODevice::ReadOnly);
                            //发送Cs@filesize
                            QString cf=Cs+"@"+QString::number(fileSize);
                            tcpSocket1->write(cf.toLatin1());
                            //发送文件内容
                            sendData();
                            //向KS发送krrd
                            QString ip="127.0.0.1";
                            qint64 port=10006;
                            QString down="download@"+krrd;
                            ui->textEdit->append(down);
                            tcpSocket2->connectToHost(QHostAddress(ip),port);
                            tcpSocket2->write(down.toLatin1());
                            ui->textEdit->append("向ks发送krrd成功");
                            tcpSocket2->disconnectFromHost();
                            tcpSocket2->close();
                            messages.clear();
                        }else{//更新r，收到@@changer@token
                            qDebug()<<"客户端请求更新r";
                            type=4;
                            token=splitArray[3];
                            qDebug()<<"收到token:"<<token;
                            //查询token对应的update
                            QString sq=QString("select ptr from files where token='%1'").arg(token);
                            sql_query.exec(sq);
                            sql_query.next();
                            ptr=sql_query.value(0).toString();
                            if(ptr!="new")token=ptr;
                            //查询token对应的Cs,krrd
                            sq=QString("select cs,krrd from files where token='%1'").arg(token);
                            sql_query.exec(sq);
                            sql_query.next();
                            Cs=sql_query.value(0).toString();
                            krrd=sql_query.value(1).toString();
                            QString res_CS=QString("changer@%1").arg(Cs);
                            ui->textEdit->append(res_CS);
                            tcpSocket1->write(res_CS.toLatin1());
                        }
                    }else if(messages.size()==2){
                        if(type==2){//上传
                            if(firstUpload){//首次上传，收到token@Cs
                                //打开文件,以token命名文件
                                token=splitArray[0];
                                Cs=splitArray[1];
                                qDebug()<<"token"<<token;
                                qDebug()<<"Cs"<<Cs;
                                filePath= "/home/weakdog/cspfiles/"+token;
                                recvSize=0;
                                file.setFileName(filePath);
                                bool isOk=file.open(QIODevice::WriteOnly);
                                if(isOk==false){
                                    ui->textEdit->append("open error");
                                }
                                //将文件信息存入数据库
                                ptr="new";
                                QString sq=QString("INSERT INTO files (sh,token,cs,ptr) VALUES('%1','%2','%3','%4')").arg(fileSh).arg(token).arg(Cs).arg(ptr);
                                ui->textEdit->append(sq);
                                qDebug()<<sq;
                                if(!sql_query.exec(sq)){
                                    ui->textEdit->append("插入数据库失败");
                                }else{
                                    ui->textEdit->append("插入数据库成功");
                                }
                                messages.clear();
                            }else{//非首次上传
                                if(splitArray[0]=="mismatch"){//mismatch@cs@pow@user
                                    //打开文件,以CS的前20个字符作为文件名
                                    token=splitArray[1];
                                    Cs=splitArray[2];
                                    filePath= "/home/weakdog/cspfiles/"+token;
                                    recvSize=0;
                                    file.setFileName(filePath);
                                    bool isOk=file.open(QIODevice::WriteOnly);
                                    if(isOk==false){
                                        ui->textEdit->append("open error");
                                    }
                                    ptr="new";
                                    //将文件信息存入数据库
                                    QString sq=QString("INSERT INTO files (sh,token,cs,ptr) VALUES('%1','%2','%3','%4')").arg(fileSh).arg(token).arg(Cs).arg(ptr);
                                    qDebug()<<sq;
                                    if(!sql_query.exec(sq)){
                                        ui->textEdit->append("插入数据库失败");
                                    }else{
                                        ui->textEdit->append("插入数据库成功");
                                    }
                                    messages.clear();
                                }else{//match@{token}
                                    QString tokens=splitArray[1];
                                    QStringList tokenslist=tokens.split("#");
                                    int r_index;
                                    bool result=getIndex(tokenslist,r_index);
                                    if(result){//发送match@r_index@index
                                        token=tokenslist[r_index];
                                        filePath= "/home/weakdog/cspfiles/"+token;
                                        qDebug()<<"r_index"<<r_index;
                                        //随机生成index
                                        qDebug()<<"filePath"<<filePath;
                                        QString index=fileIndex(filePath);
                                        qDebug()<<"随机生成index："<<index;
                                        //通过index计算块文件哈希值
                                        blockHash=block(filePath,index.toInt());
                                        //发送match@r_index@index
                                        QString mri="match@"+QString::number(r_index,10)+"@"+index;
                                        qDebug()<<"mri"<<mri;
                                        tcpSocket1->write(mri.toLatin1());
                                        //查询数据库得到token对应的krrd
                                        QString sq=QString("select krrd from files where token='%1'").arg(token);
                                        ui->textEdit->append(sq);
                                        sql_query.exec(sq);
                                        sql_query.next();
                                        krrd=sql_query.value(0).toString();
                                        //连接ks，发送token对应的krrd
                                        QString ip="127.0.0.1";
                                        qint64 port=10006;
                                        QString up="upload@"+krrd;
                                        ui->textEdit->append(up);
                                        tcpSocket2->connectToHost(QHostAddress(ip),port);
                                        tcpSocket2->write(up.toLatin1());
                                        ui->textEdit->append("向ks发送krrd成功");
                                        tcpSocket2->disconnectFromHost();
                                        tcpSocket2->close();
                                    }else{//发送mismatch
                                        QString reply="mismatch";
                                        tcpSocket1->write(reply.toLatin1());
                                    }
                                }
                            }
                        }else if(type==4){//更新r
                            if(messages.size()==2){//收到sh@token2@r^r@Cs
                                fileSh=splitArray[0];
                                QString token2=splitArray[1];
                                QString rr=splitArray[2];
                                Cs=splitArray[3];
                                //计算新的krrd
                                QString krrd2=hashXor(krrd,rr);
                                //查找ptr对应token的所有记录，全部更新为token2
                                sql_query.exec(QString("select * from files where ptr='%1'").arg(token));
                                while(sql_query.next())
                                {
                                    QString tempToken=sql_query.value(1).toString();
                                    qDebug()<<"tempToken"<<tempToken;
                                    QSqlQuery sql_query2=QSqlQuery(database);
                                    //更新数据库
                                    QString sq=QString("update files set ptr='%1' where token='%2'").arg(token2).arg(tempToken);
                                    qDebug()<<sq;
                                    sql_query2.exec(sq);
                                }
                                //token对应的记录ptr更新为token2
                                QString sq=QString("update files set ptr = '%1' where token='%2'").arg(token2).arg(token);
                                sql_query.exec(sq);
                                ptr="new";
                                //插入一条新的记录
                                sq=QString("INSERT INTO files (sh,token,cs,ptr,krrd) VALUES('%1','%2','%3','%4','%5')").arg(fileSh).arg(token2).arg(Cs).arg(ptr).arg(krrd2);
                                ui->textEdit->append(sq);
                                qDebug()<<sq;
                                if(!sql_query.exec(sq)){
                                    ui->textEdit->append("插入数据库失败");
                                }else{
                                    ui->textEdit->append("插入数据库成功");
                                }
                                //更改文件名
                                QString rename=QString("mv /home/weakdog/cspfiles/%1 /home/weakdog/cspfiles/%2").arg(token).arg(token2);
                                system(rename.toLatin1().data());
                                qDebug()<<"更新群组密钥成功";
                                messages.clear();
                            }
                        }
                    }else{//收到r的验证值或upload2
                        if(splitArray[0]=="blockcheck"){
                            qDebug()<<"收到客户端hashblock："<<splitArray[1];
                            qDebug()<<"blockHash："<<blockHash;
                            if(splitArray[1]==blockHash){//验证成功
                                //返回结果
                                QString blockResult="success";
                                tcpSocket1->write(blockResult.toLatin1());
                                //清空缓冲区
                                messages.clear();
                            }else{
                                qDebug()<<"block验证失败";
                            }
                            messages.clear();
                        }else{//upload2@token@Cs
                            //打开文件,以token命名文件
                            token=splitArray[1];
                            Cs=splitArray[2];
                            filePath= "/home/weakdog/cspfiles/"+token;
                            recvSize=0;
                            file.setFileName(filePath);
                            bool isOk=file.open(QIODevice::WriteOnly);
                            if(isOk==false){
                                ui->textEdit->append("open error");
                            }
                            ptr="new";
                            //将文件信息存入数据库
                            QString sq=QString("INSERT INTO files (sh,token,cs,ptr) VALUES('%1','%2','%3','%4')").arg(fileSh).arg(token).arg(Cs).arg(ptr);
                            qDebug()<<sq;
                            if(!sql_query.exec(sq)){
                                ui->textEdit->append("插入数据库失败");
                            }else{
                                ui->textEdit->append("插入数据库成功");
                            }
                            messages.clear();
                        }
                    }
                    }
                        );
            });

    //监听ks端连接
    connect(tcpServer2,&QTcpServer::newConnection,
            [=]()
            {
                //取出建立好的套接字
                tcpSocket2=tcpServer2->nextPendingConnection();
                //获取对方的IP和端口
                QString ip=tcpSocket2->peerAddress().toString();
                quint16 port=tcpSocket2->peerPort();
                QString str=QString("[%1:%2]成功连接").arg(ip).arg(port);

                connect(tcpSocket2,&QTcpSocket::readyRead,
                        [=]()
                        {
                            QByteArray array=tcpSocket2->readAll();
                            QStringList splitArray=QString(array).split("@");
                            if(splitArray[0]=="upload"){
                                token=splitArray[1];
                                krrd=splitArray[2];
                                //更新数据库
                                QString sq=QString("update files set krrd = '%1' where token = '%2'").arg(krrd).arg(token);
                                qDebug()<<"更新数据库"<<sq;
                                sql_query.exec(sq);
                            }else{
                                QString dd=QString(array);
                                qDebug()<<"从ks收到更新密钥dd:"<<dd;
                                //查询数据库，得到所有cs@ckrd
                                sql_query.exec(QString("select * from files"));
                                while(sql_query.next()){
                                    qDebug()<<"why?";
                                    token=sql_query.value(1).toString();
                                    krrd=sql_query.value(3).toString();
                                    QString newKrrd=hashXor(krrd,dd);
                                    qDebug()<<"krrd:"<<krrd;
                                    qDebug()<<"newkrrd:"<<newKrrd;
                                    qDebug()<<"token:"<<token;
                                    //更新数据库
                                    QSqlQuery sql_query2=QSqlQuery(database);
                                    QString sq=QString("update files set krrd = '%1' where token='%2'").arg(newKrrd).arg(token);
                                    sql_query2.exec(sq);
                                    qDebug()<<sq;
                                }
                                qDebug()<<"更新系统密钥完成";
                            }

                        });
            });

    //监听客户端连接(文件端口)
    connect(tcpServer3,&QTcpServer::newConnection,
            [=]()
            {
                //取出建立好的套接字
                tcpSocket3=tcpServer3->nextPendingConnection();
                //获取对方的IP和端口
                QString ip=tcpSocket3->peerAddress().toString();
                quint16 port=tcpSocket3->peerPort();
                QString str=QString("[%1:%2]成功连接(文件)").arg(ip).arg(port);
                ui->textEdit->append(str);

                //接收文件
                connect(tcpSocket3,&QTcpSocket::readyRead,
                        [=]()
                        {
                            QByteArray buf=tcpSocket3->readAll();
                            //文件信息
                            qint64 len=file.write(buf);
                            recvSize+=len;
                            qDebug()<<recvSize;
                            if(recvSize==fileSize){
                                file.close();
                                qDebug()<<"接收文件完成";
                                QMessageBox::information(this,"完成","文件接收完成");
                            }
                        }
                        );
            });
}

//发送文件内容
void Csp::sendData()
{
    qint64 len=0;
    do{
        char buf[4*1024]={0};
        //往文件中读数据
        len=file.read(buf,sizeof(buf));
        //发送数据
        tcpSocket3->write(buf,len);
        sendSize+=len;
    }while(len>0);
    //是否发送文件完毕
    if(sendSize==fileSize){
        ui->textEdit->append("文件发送完毕");
        file.close();
        //断开客户端
    }
}

Csp::~Csp()
{
    delete ui;
}

//查询数据库判断密码是否正确
bool Csp::checkLogin(QString account, QString password){
    sql_query.exec(QString("select * from login where account='%1' and password='%2'").arg(account).arg(password));
    if(sql_query.next()){
        return true;
    }else{
        return false;
    }
}

//查询数据库判断是否为首次上传
bool Csp::checkFirst(QString fileSh){
    sql_query.exec(QString("select * from files where sh='%1'").arg(fileSh));
    if(sql_query.next()){
        return false;
    }else{
        return true;
    }
}

//查询数据库得到哈希值对应的所有cs
QStringList Csp::getCs(QString fileSh){
    QStringList Css;
    sql_query.exec(QString("select cs from files where sh='%1'").arg(fileSh));
    while(sql_query.next())
    {
        Css.append(sql_query.value(0).toString());
    }
    return Css;
}

//查询数据库得到哈希值对应的所有cs
bool Csp::getIndex(QStringList tokenlist, int &index){
    bool result=false;
    for(index=0;index<tokenlist.size();index++){
        sql_query.exec(QString("select * from files where token='%1'").arg(tokenlist[index]));
        if(sql_query.next()){
            result=true;
            break;
        }
    }
    return result;
}

//两个32字节十六进制字符串异或
QString Csp::hashXor(QString hexStr1,QString hexStr2){
    QString result;
    for(int i=0;i<32;i++){
        QString temp1=hexStr1.mid(2*i,2);
        QString temp2=hexStr2.mid(2*i,2);
        uint hex = temp1.toUInt(NULL, 16);     // hex == 255, ok == true
        uint hex2 = temp2.toUInt(NULL, 16);
        uint res=hex^hex2;
        QString temp=QString::number(res,16);
        if(temp.size()==1)temp.insert(0,"0");
        result+=temp;
    }
    return result;
}

//计算文件sm3哈希值
QString Csp::sm3_f(QString filepath){
    unsigned char* digest=new unsigned char[32];
    unsigned int hash_len;
    QFile file;
    file.setFileName(filepath);
    file.open(QIODevice::ReadOnly);
    QByteArray array = file.readAll();
    qDebug()<<array.size();
    EVP_MD_CTX *md_ctx;
    const EVP_MD *md;
    md = EVP_sm3();
    md_ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(md_ctx, md, NULL);
    EVP_DigestUpdate(md_ctx, array, array.size());
    EVP_DigestFinal_ex(md_ctx, digest, &hash_len);
    EVP_MD_CTX_free(md_ctx);
    file.close();
    QString result=QByteArray((char*)digest,32).toHex();
    return result;
}

QString Csp::block(QString filePath,int index){
    //fileblock
    QFile file1,file2,file3;
    file1.setFileName(filePath);
    file2.setFileName("/home/weakdog/cspfiles/block");
    file1.open(QIODevice::ReadOnly);
    file2.open(QIODevice::WriteOnly);
    file1.seek(1024*index);
    char buf[1024]={0};
    //往文件中读数据
    int len=file1.read(buf,sizeof(buf));
    file2.write(buf,len);
    file1.close();
    file2.close();
    //计算哈希值
    QString blockHash=sm3_f("/home/weakdog/cspfiles/block");
    file3.setFileName("/home/weakdog/cspfiles/block");
    file3.remove();
    return blockHash;
}

QString Csp::fileIndex(QString filePath){
    //获取文件大小
    QFileInfo info(filePath);
    int totalSize=info.size();
    int maxIndex=totalSize/1024;
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
    int num = qrand()%(maxIndex);
    return QString::number(num);
}
