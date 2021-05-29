#include "client.h"
#include "ui_client.h"

Client::Client(QString account_,QString password_,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Client)
{
    ui->setupUi(this);
    key=new unsigned char[32];
    account=account_;
    password=password_;
    if(QSqlDatabase::contains("qt_sql_default_connection"))
        database = QSqlDatabase::database("qt_sql_default_connection");
    else
        database=QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName("client.db");
    if(!database.open()){
        qDebug()<<"Error: Fail to connect database"<<database.lastError();
    }else{
        qDebug()<<"Succeed to connect database";
    }
    fileModel=new QSqlTableModel(this);
    fileModel->setTable("fileName");
    ui->tableView->setModel(fileModel);
    fileModel->setFilter(QString("username='%1'").arg(account));
    fileModel->select();
    fileModel->setHeaderData(0,Qt::Horizontal,"文件名");
    sql_query=QSqlQuery(database);
    type=0;
    //初始化套接字
    tcpServer1=new QTcpServer(this);
    tcpSocket1=new QTcpSocket(this);
    tcpSocket2=new QTcpSocket(this);
    tcpSocket3=new QTcpSocket(this);
    tcpServer1->listen(QHostAddress::Any,10001);

    //监听ks消息
    connect(tcpServer1,&QTcpServer::newConnection,
            [=](){
                //取出建立好的套接字
                tcpSocket2=tcpServer1->nextPendingConnection();
                //获取对方的IP和端口
                QString ip=tcpSocket2->peerAddress().toString();
                quint16 port=tcpSocket2->peerPort();
                connect(tcpSocket2,&QTcpSocket::readyRead,
                        [=]()
                        {
                            array=tcpSocket2->readAll();
                            test3.restart();
                            splitArray=QString(array).split("@");
                            QString krr=splitArray[1];
                            qDebug()<<"r"<<r;
                            qDebug()<<"krr"<<krr;
                            kr=Security().hashXor(krr,r);
                            qDebug()<<"kr"<<kr;
                            QByteArray temp=QByteArray::fromHex(kr.toLatin1());
                            key=(unsigned char*)temp.data();
                            if(splitArray[0]=="download"){
                                //解密文件
                                qDebug()<<"cipherPath:"<<filePath;
                                QFileInfo info(filePath);
                                qDebug()<<"filesize"<<info.size();
                                Security().Decrypt_File(key,filePath);
                                //更改文件名
                                QString rename=QString("mv %1 %2").arg(filePath+".decrypted").arg(filePath+"2");
                                qDebug()<<rename;
                                system(rename.toLatin1().data());
                                qDebug()<<"下载文件成功";
                                time1=test1.elapsed();
                                time3=test3.elapsed();
                                qDebug()<<"下载文件时间："<<time1<<"ms";
                                qDebug()<<"接收文件时间："<<time2<<"ms";
                                qDebug()<<"解密文件时间："<<time3<<"ms";
                                //断开连接
                                tcpSocket1->disconnectFromHost();
                                tcpSocket1->close();
                            }else{//上传文件
                                //TODO加密文件
                                blockHash=Security().block(key,filePath,index.toInt());
                                QString bb="blockcheck@"+blockHash;
                                //发送块哈希给服务器
                                tcpSocket1->write(bb.toLatin1());
                            }
                        });
            });

    //从csp接收消息
    connect(tcpSocket1,&QTcpSocket::readyRead,
            [=]()
            {
                array=tcpSocket1->readAll();
                messages.append(array);
                splitArray=QString(array).split("@");
                ui->textEdit->append("收到消息"+QString(array));
                qDebug()<<"收到消息"+QString(array);
                if(type==1){//上传
                    if(messages.size()==1){
                        if(splitArray[0]=="first"){//首次上传
                            time2=test2.elapsed();
                            qDebug()<<"本次上传为首次上传";
                            //生成群组密钥r
                            unsigned char* ur=new unsigned char[32];
                            r=Security().get_rand_key(ur,32);//generate random key
                            qDebug()<<"群组密钥r:"<<r;
                            //计算token=H(hf,r)
                            token=Security().HMAC_SM3(fileHash,r);
                            qDebug()<<"token:"<<token;
                            //生成随机密钥s
                            unsigned char* us=new unsigned char[32];
                            Security().get_rand_key(us,32);//generate random key
                            QString s=QByteArray((char*)us,32).toHex();
                            test4.restart();
                            QString Cs=calcCs(r,s,filePath,hsm);
                            time4=test4.elapsed();
                            //发送token@Cs
                            QString tc=token+"@"+Cs;
                            qint32 len2=tcpSocket1->write(tc.toLatin1());
                            if(len2>0)qDebug()<<"发送token@Cs成功";
                            else qDebug()<<"发送token@Cs失败";
                            //发送文件
                            test5.restart();
                            sendData();
                            time5=test5.elapsed();
                            //断开连接
                            tcpSocket1->disconnectFromHost();
                            tcpSocket1->close();
                            tcpSocket3->disconnectFromHost();
                            tcpSocket3->close();
                            //计算kr^r
                            QString krr=Security().hashXor(kr,r);
                            //连接ks发送token@krr
                            QString ip="127.0.0.1";
                            qint64 port=10005;
                            tcpSocket2->connectToHost(QHostAddress(ip),port);
                            QString tk=token+"@"+krr;
                            tcpSocket2->write(tk.toLatin1());
                            ui->textEdit->append("发送token@krr成功");
                            //断开连接
                            tcpSocket2->disconnectFromHost();
                            tcpSocket2->close();
                            //插入数据库
                            QString sq=QString("INSERT INTO file (filename,h,hsm,token,username) VALUES('%1','%2','%3','%4','%5')").arg(fileName).arg(fileHash).arg(hsm).arg(token).arg(account);
                            ui->textEdit->append(sq);
                            if(!sql_query.exec(sq)){
                                ui->textEdit->append("插入数据库失败");
                                qDebug() << sql_query.lastError();
                            }else{
                                ui->textEdit->append("插入数据库成功");
                                fileModel->select();
                            }
                            messages.clear();
                            time1+=test1.elapsed();
                            qDebug()<<"首次上传耗时："<<time1<<"ms";
                            qDebug()<<"验证是否为首次耗时："<<time2<<"ms";
                            qDebug()<<"加密明文耗时："<<time3<<"ms";
                            qDebug()<<"策略加密耗时："<<time4<<"ms";
                            qDebug()<<"文件传输耗时："<<time5<<"ms";
                        }else{//非首次上传，second@Css
                            qDebug()<<"本次上传非首次上传";
                            QString Css=splitArray[1];
                            bool compRes=compareCs(Css,rl);
                            if(compRes==false){//重新创建群组并上传文件
                                qDebug()<<"不在现存群组中";
                                //生成群组密钥r
                                unsigned char* ur=new unsigned char[32];
                                r=Security().get_rand_key(ur,32);//generate random key
                                qDebug()<<"群组密钥r:"<<r;
                                //计算H(hf,r)
                                token=Security().HMAC_SM3(fileHash,r);
                                qDebug()<<"token:"<<token;
                                //生成随机密钥s
                                unsigned char* sb=new unsigned char[32];
                                Security().get_rand_key(sb,32);//generate random key
                                QString s=QByteArray((char*)sb,32).toHex();
                                QString Cs=calcCs(r,s,filePath,hsm);
                                //发送mismatch@token@Cs
                                QString tc="mismatch@"+token+"@"+Cs;
                                qint32 len2=tcpSocket1->write(tc.toUtf8().data());
                                if(len2>0)ui->textEdit->append("发送Cs成功");
                                else ui->textEdit->append("发送Cs失败");
                                //发送文件
                                sendData();
                                //断开连接
                                tcpSocket1->disconnectFromHost();
                                tcpSocket1->close();
                                tcpSocket3->disconnectFromHost();
                                tcpSocket3->close();
                                //计算kr^r
                                QString krr=Security().hashXor(kr,r);
                                qDebug()<<"krr:"<<krr;
                                //连接ks发送token@krr
                                QString ip="127.0.0.1";
                                qint64 port=10005;
                                tcpSocket2->connectToHost(QHostAddress(ip),port);
                                QString tk=token+"@"+krr;
                                tcpSocket2->write(tk.toLatin1());
                                qDebug()<<"发送token@krr成功";
                                //断开连接
                                tcpSocket2->disconnectFromHost();
                                tcpSocket2->close();
                                //插入数据库
                                QString sq=QString("INSERT INTO file (filename,h,hsm,token,username) VALUES('%1','%2','%3','%4','%5')").arg(fileName).arg(fileHash).arg(hsm).arg(token).arg(account);
                                ui->textEdit->append(sq);
                                if(!sql_query.exec(sq)){
                                    ui->textEdit->append("插入数据库失败");
                                    qDebug() << sql_query.lastError();
                                }else{
                                    ui->textEdit->append("插入数据库成功");
                                    fileModel->select();
                                }
                                messages.clear();
                            }else{//通过{Cs}解密得到{r}
                                qDebug()<<"rl:"<<rl;
                                //计算{H(hf,r)}
                                QString tokens;
                                for(int i=0;i<rl.size()-1;i++){
                                    QString temp=Security().HMAC_SM3(fileHash,rl[i])+"#";
                                    tokens+=temp;
                                }
                                tokens+=Security().HMAC_SM3(fileHash,rl[rl.size()-1]);
                                //发送match@tokens
                                QString mt=QString("match@%1").arg(tokens);
                                qDebug()<<"mt:"<<mt;
                                tcpSocket1->write(mt.toUtf8().data());
                                ui->textEdit->append("发送mt成功");
                            }
                        }
                    }else if (messages.size()==2){//非首次上传
                        if(splitArray[0]=="match"){//接收的是r_index和index
                            r_index=splitArray[1];
                            index=splitArray[2];
                            r=rl[r_index.toInt()];
                            token=Security().HMAC_SM3(fileHash,r);
                            qDebug()<<"r:"<<r;
                            qDebug()<<"index:"<<index;
                            qDebug()<<"token:"<<token;
                            file.remove();//删除选择文件时创建的密文
                        }else{//收到"mismatch"，发送upload2@token@Cs
                            qDebug()<<"能解开Cs但文件不对应";
                            //生成群组密钥r
                            unsigned char* ur=new unsigned char[32];
                            r=Security().get_rand_key(ur,32);//generate random key
                            qDebug()<<"群组密钥r:"<<r;
                            //计算token=H(hf,r)
                            token=Security().HMAC_SM3(fileHash,r);
                            qDebug()<<"token:"<<token;
                            //生成随机密钥s
                            unsigned char* sb=new unsigned char[32];
                            Security().get_rand_key(sb,32);//generate random key
                            QString s=QByteArray((char*)sb,32).toHex();
                            QString Cs=calcCs(r,s,filePath,hsm);
                            //发送upload2@token@Cs
                            QString utc="upload2@"+token+"@"+Cs;
                            qint32 len2=tcpSocket1->write(utc.toUtf8().data());
                            if(len2>0)ui->textEdit->append("发送token@Cs成功");
                            else ui->textEdit->append("发送token@Cs失败");
                            //发送文件
                            sendData();                            
                            //断开连接
                            tcpSocket1->disconnectFromHost();
                            tcpSocket1->close();
                            tcpSocket3->disconnectFromHost();
                            tcpSocket3->close();
                            //计算kr^r
                            kr=QByteArray((char*)key,32).toHex();//kr转QString
                            QString krr=Security().hashXor(kr,r);
                            //连接ks发送token@krr
                            QString ip="127.0.0.1";
                            qint64 port=10005;
                            tcpSocket2->connectToHost(QHostAddress(ip),port);
                            QString tk=token+"@"+krr;
                            tcpSocket2->write(tk.toLatin1());
                            ui->textEdit->append("发送token@krr成功");
                            //断开连接
                            tcpSocket2->disconnectFromHost();
                            tcpSocket2->close();
                            //插入数据库
                            QString sq=QString("INSERT INTO file (filename,h,hsm,token,username) VALUES('%1','%2','%3','%4','%5')").arg(fileName).arg(fileHash).arg(hsm).arg(token).arg(account);
                            qDebug()<<sq;
                            if(!sql_query.exec(sq)){
                                ui->textEdit->append("插入数据库失败");
                                qDebug() << sql_query.lastError();
                            }else{
                                ui->textEdit->append("插入数据库成功");
                                fileModel->select();
                            }
                            messages.clear();
                        }
                    }else{//收到验证r的结果
                        if(splitArray[0]=="success"){
                            //插入数据库
                            QString sq=QString("INSERT INTO file (filename,h,hsm,token,username) VALUES('%1','%2','%3','%4','%5')").arg(fileName).arg(fileHash).arg(hsm).arg(token).arg(account);
                            qDebug()<<sq;
                            ui->textEdit->append(sq);
//                            if(!sql_query.exec(sq)){
//                                ui->textEdit->append("插入数据库失败");
//                                qDebug() << sql_query.lastError();
//                            }else{
//                                ui->textEdit->append("插入数据库成功");
//                                fileModel->select();
//                            }
                            qDebug()<<"后续上传成功";
                        }
                        tcpSocket1->disconnectFromHost();
                        tcpSocket1->close();
                        //删除密文
                        QFile rf;
                        rf.setFileName(cipherPath);
                        rf.remove();
                        time6+=test6.elapsed();
                        qDebug()<<"后续上传耗时："<<time6<<"ms";//输出计时
                        messages.clear();
                    }
                }else if(type==2){//下载
                    test2.restart();
                    //接收到Cs@filesize
                    fileSize=splitArray[1].toInt();
                    //解密得到r
                    DecCs(splitArray[0],r);
                    qDebug()<<"解密得到r"<<r;
                    recvSize=0;
                    qDebug()<<"收到文件大小为:"+QString::number(fileSize);
                    messages.clear();
                }else if(type==3){//更新r：changer@cs
                    QString Cs0=splitArray[1];
                    //接收的为changer@Cs
                    DecCs(splitArray[1],r);
                    //生成新的r
                    unsigned char* ur=new unsigned char[32];
                    QString r2=Security().get_rand_key(ur,32);//generate random key
                    QString r3=Security().hashXor(r2,r);
                    qDebug()<<"r:"<<r;
                    qDebug()<<"r2："<<r2;
                    qDebug()<<"rr:"<<r3;
                    qDebug()<<"token(old):"<<token;
                    //计算新的token
                    token=Security().HMAC_SM3(fileHash,r2);
                    qDebug()<<"token(new):"<<token;
                    //计算新的Cs
                    QString Cs=calcCs2(hsm,r2,splitArray[1]);
                    //计算短哈希
                    fileSh=Security().genSh(fileHash);
                    //发送sh@token2@r^r'@Cs
                    QString strC=fileSh+"@"+token+"@"+r3+"@"+Cs;
                    tcpSocket1->write(strC.toLatin1());
                    tcpSocket1->disconnectFromHost();;
                    tcpSocket1->close();
                    messages.clear();
                    time7=test7.elapsed();
                    qDebug()<<"更新r用时"<<time7<<"ms";
                }
            }
            );

    //从csp接收文件
    connect(tcpSocket3,&QTcpSocket::readyRead,
            [=]()
            {
                QByteArray buf=tcpSocket3->readAll();
                qint64 len=file.write(buf);
                recvSize+=len;
                if(recvSize==fileSize){
                    file.close();
                    qDebug()<<"文件接收完成";
                    QMessageBox::information(this,"完成","文件接收完成");
                    time2=test2.elapsed();
                    //断开连接
                    tcpSocket3->disconnectFromHost();
                    tcpSocket3->close();
                    recvSize=0;
                }
            }
            );
}

Client::~Client()
{
    delete ui;
}

//选择文件,生成随机密文
void Client::on_pushButtonSelect_clicked()
{
    filePath=QFileDialog::getOpenFileName(this,"open","../");
    test6.restart();
    test2.restart();
    if(filePath.isEmpty()==false){//如果选择文件路径有效
        qDebug()<<"选择路径："<<filePath;
        fileSize=0;
        sendSize=0;
        //计算文件哈希值
        fileHash=Security().sm3_f(filePath);
        //计算文件短哈希
        fileSh=Security().genSh(fileHash);
        //生成随机秘钥
        kr=Security().get_rand_key(key, 32);
        qDebug()<<"生成随机密钥Kr："<<kr;
        time6=test6.elapsed();
        test3.restart();
        Security().Encrypt_File(key,filePath);//加密得到随机密文
        time3=test3.elapsed();
        //获取密文信息
        QFileInfo info(filePath);
        fileName=info.fileName();
        cipherPath=filePath+".encrypted";
        info=QFileInfo(cipherPath);
        fileSize=info.size();
        qDebug()<<"fileHash:"<<fileHash;
        qDebug()<<"filesize:"<<fileSize;
        qDebug()<<"fileSh:"<<fileSh;
        //以只读方式打开文件
        file.setFileName(cipherPath);
        bool isOk=file.open(QIODevice::ReadOnly);
        if(isOk==false){
            ui->textEdit->append("open false");
            file.close();
        }
    }else{
         ui->textEdit->append("select file fault");
    }
    time2=test2.elapsed();
}

//发送文件
void Client::on_pushButtonUpload_clicked()
{
    test6.restart();
    test2.restart();
    qDebug()<<"------------------------请求上传文件";
    //获取服务器的IP和端口
    QString ip="127.0.0.1";
    qint64 port=10002;
    qint64 port2=10004;
    tcpSocket1->connectToHost(QHostAddress(ip),port);
    tcpSocket3->connectToHost(QHostAddress(ip),port2);
    ui->textEdit->append("与csp建立连接成功");
    type=1;//上传
    //获取选择的用户
    users.clear();
    QList<QListWidgetItem *>selected=ui->listWidget->selectedItems();
    for(int i=0;i<selected.size();i++){
        users.append(selected[i]->text());
    }
    //发送上传指令,文件名，文件大小，文件短哈希
    QString command=QString("@@upload@%1@%2").arg(fileSize).arg(fileSh);
    qDebug()<<"发送文件上传命令"+command;
    qint64 len=tcpSocket1->write(command.toUtf8());
    if(len<=0){//发送头部信息失败
        ui->textEdit->append("command send false");
        file.close();
    }
}

//下载文件
void Client::on_pushButtonDownload_clicked()
{
    test1.restart();
//    test2.restart();
    qDebug()<<"-----------------------------请求下载文件";
    //获取选择的文件名
    int curRow=ui->tableView->currentIndex().row(); //选中行
    QAbstractItemModel *modessl = ui->tableView->model();
    QModelIndex indextemp = modessl->index(curRow,0);//遍历第一行的所有列
    fileName = modessl->data(indextemp).toString();
    qDebug()<<"选中文件: "+fileName;
    filePath="/home/weakdog/clientfiles2/"+fileName;
    //获取服务器的IP和端口
    QString ip="127.0.0.1";
    qint64 port=10002;
    qint64 port2=10004;
    tcpSocket1->connectToHost(QHostAddress(ip),port);
    tcpSocket3->connectToHost(QHostAddress(ip),port2);
    ui->textEdit->append("与csp建立连接成功");
    type=2;
    //从数据库中读取文件名对应的哈希值
    QString sq=QString("select token from file where filename='%1' and username='%2'").arg(fileName).arg(account);
    ui->textEdit->append(sq);
    sql_query.exec(sq);
    sql_query.next();
    token=sql_query.value(0).toString();
    ui->textEdit->append("从数据库中读到token:"+token);
    //发送下载指令
    QString command=QString("@@download@%1").arg(token);//换成文件哈希值
    //发送文件头信息
    ui->textEdit->append("发送下载文件命令:"+command);
    qint64 len=tcpSocket1->write(command.toUtf8().data());
    if(len>0){
        //打开文件
        file.setFileName(filePath);
        bool isOk=file.open(QIODevice::WriteOnly);
        if(isOk==false){
            ui->textEdit->append("open error");
        }
    }
    recvSize=0;
}

//更新r
void Client::on_pushButtonChange_clicked()
{
    test7.start();
    qDebug()<<"--------------请求更新s------------";
    QString ip="127.0.0.1";
    qint64 port=10002;
    tcpSocket1->connectToHost(QHostAddress(ip),port);
    //获取选取的文件名
    type=3;
    int curRow=ui->tableView->currentIndex().row(); //选中行
    QAbstractItemModel *modessl = ui->tableView->model();
    QModelIndex indextemp = modessl->index(curRow,0);//遍历第一行的所有列
    fileName = modessl->data(indextemp).toString();
    ui->textEdit->append("选中文件: "+fileName);
    qDebug()<<"选中文件: "+fileName;
    //查询文件哈希值
    QString sq=QString("select * from file where filename='%1'").arg(fileName);
    qDebug()<<sq;
    sql_query.exec(sq);
    sql_query.next();
    token=sql_query.value(3).toString();
    hsm=sql_query.value(2).toString();
    fileHash=sql_query.value(1).toString();
    QString change="@@changer@"+token;
    qDebug()<<change;
    tcpSocket1->write(change.toLatin1());
}

//发送文件内容
void Client::sendData()
{
    qDebug()<<"开始发送文件";
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
        qDebug()<<"文件发送完毕";
        sendSize=0;
        file.remove();
    }
}

QString Client::calcCs(QString r,QString s,QString filePath,QString&hsm){
    //计算HMAC(s,M)
    hsm=Security().hmac(s.toLatin1(),filePath);
    //计算H(s,M)^r
    QString h_xor_r=Security().hashXor(hsm,r);
    //storage h_xor_r into file
    QFile fileS;
    fileS.setFileName("/home/weakdog/clientfiles/s.txt");
    fileS.open(QIODevice::WriteOnly);
    fileS.write(h_xor_r.toLatin1());
    fileS.close();
    //cpabe-enc h_xor_r
    QString command="cd /home/weakdog/clientfiles&&cpabe-enc pub_key s.txt '";
    for(int i=0;i<users.size()-1;i++)command+=QString("%1 or ").arg(users[i]);
    command+=users[users.size()-1];
    command+="'";
    qDebug()<<command;
    system(command.toLatin1().data());
    //read cypher to bytearray
    QFile fileCypherS;
    fileS.setFileName("/home/weakdog/clientfiles/s.txt.cpabe");
    fileS.open(QIODevice::ReadOnly);
    QByteArray buf=fileS.readAll();
    qDebug()<<"buf.size():"<<buf.size();
    fileS.remove();
    QString Cs=buf.toHex();
    Cs+="&";
    Cs+=s;
    return Cs;
}

bool Client::DecCs(QString Cs,QString&r)
{
    QString Epk=Cs.split("&")[0];
    QString s=Cs.split("&")[1];
    QByteArray qb = QByteArray::fromHex (Epk.toLatin1().data());
    //story Epk into file
    QFile file1;
    file1.setFileName("/home/weakdog/clientfiles/Epk.txt.cpabe");
    file1.open(QIODevice::WriteOnly);
    file1.write(qb);
    file1.close();
    //cpabe-dec
    QString command="cd /home/weakdog/clientfiles&&cpabe-dec pub_key "+account +"_key "+"Epk.txt.cpabe";
    qDebug()<<"command"<<command;
    system(command.toLatin1().data());
    QFile file2;
    file2.setFileName("/home/weakdog/clientfiles/Epk.txt");
    bool isOk=file2.open(QIODevice::ReadOnly);
    if(isOk==false){
        qDebug()<<"dec false";
        file2.close();
        return false;
    }else{
        QFile file2;
        file2.setFileName("/home/weakdog/clientfiles/Epk.txt");
        file2.open(QIODevice::ReadOnly);
        QByteArray hxr=file2.readAll();
        QString h_xor_r=QString(hxr);
        //从数据库中读取H(s,M)
        QString sq=QString("select * from file where filename='%1' and username='%2'").arg(fileName).arg(account);
        qDebug()<<sq;
        sql_query.exec(sq);
        sql_query.next();
        hsm=sql_query.value(2).toString();
        qDebug()<<"hsm"<<hsm;
        r=Security().hashXor(hsm,h_xor_r);
        file2.remove();
        return true;
    }
}

bool Client::compareCs(QString Cs,QStringList& rl){
    QStringList csList=Cs.split("#");
    for(int i=0;i<csList.size();i++){
        QString r;
        if(DecCs(csList[i],r)){
            rl.append(r);
        }
    }
    if(rl.size()==0)return false;
    else return true;
}

QString Client::calcCs2(QString hsm,QString r,QString Cs){
    //计算H(s,M)^r
    QString h_xor_r=Security().hashXor(hsm,r);
    //storage h_xor_r into file
    QFile fileS;
    fileS.setFileName("/home/weakdog/clientfiles/r.txt");
    fileS.open(QIODevice::WriteOnly);
    fileS.write(h_xor_r.toLatin1());
    fileS.close();
    //cpabe-enc h_xor_r
    QString command="cd /home/weakdog/clientfiles&&cpabe-enc pub_key r.txt '";
    for(int i=0;i<users.size()-1;i++)command+=QString("%1 or ").arg(users[i]);
    command+=users[users.size()-1];
    command+="'";
    qDebug()<<command;
    system(command.toLatin1().data());
    //read cypher to bytearray
    QFile fileCypherS;
    fileS.setFileName("/home/weakdog/clientfiles/r.txt.cpabe");
    fileS.open(QIODevice::ReadOnly);
    QByteArray buf=fileS.readAll();
    qDebug()<<"buf.size():"<<buf.size();
    fileS.remove();
    QString Cs2=buf.toHex();
    Cs2+="&";
    QString s=Cs.split("&")[1];
    Cs2+=s;
    return Cs2;
}
