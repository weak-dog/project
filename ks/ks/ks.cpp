#include "ks.h"
#include "ui_ks.h"

Ks::Ks(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Ks)
{
    ui->setupUi(this);
    //初始化系统密钥
    unsigned char* ud=new unsigned char[32];
    d=get_rand_key(ud, 32);
    //初始化套接字
    tcpServer1=new QTcpServer(this);
    tcpServer2=new QTcpServer(this);
    tcpSocket1=new QTcpSocket(this);
    tcpSocket2=new QTcpSocket(this);

    tcpServer1->listen(QHostAddress::Any,10005);
    tcpServer2->listen(QHostAddress::Any,10006);

    //监听客户端消息
    connect(tcpServer1,&QTcpServer::newConnection,
            [=](){
                //取出建立好的套接字
                tcpSocket1=tcpServer1->nextPendingConnection();
                //获取对方的IP和端口
                QString ip=tcpSocket1->peerAddress().toString();
                quint16 port=tcpSocket1->peerPort();
                QString str=QString("[%1:%2]成功连接(客户端)").arg(ip).arg(port);
                qDebug()<<str;
                connect(tcpSocket1,&QTcpSocket::readyRead,
                        [=]()
                        {
                            QByteArray array=tcpSocket1->readAll();
                            QStringList splitArray=QString(array).split("@");
                            qDebug()<<"------------------------------------";
                            qDebug()<<"从客户端收到消息(token@krr): "+QString(array);
                            //计算kr_rd
                            QString token=splitArray[0];
                            QString krr=splitArray[1];
                            qDebug()<<"token："<<token;
                            qDebug()<<"krr："<<krr;
                            QString krrd=hashXor(krr,d);
                            qDebug()<<"krrd"<<krrd;
                            qDebug()<<"加密krrd结束";
                            QString ucc="upload@"+token+"@"+krrd;
                            qDebug()<<ucc;
                            //与csp建立连接并发送给csp
                            QString ip="127.0.0.1";
                            qint64 port=10003;
                            tcpSocket2->connectToHost(QHostAddress(ip),port);
                            tcpSocket2->write(ucc.toLatin1());
                            tcpSocket2->disconnectFromHost();
                            tcpSocket2->close();
                        });
            });

    //监听csp消息
    connect(tcpServer2,&QTcpServer::newConnection,
            [=](){
                //取出建立好的套接字
                tcpSocket2=tcpServer2->nextPendingConnection();
                //获取对方的IP和端口
                QString ip=tcpSocket2->peerAddress().toString();
                quint16 port=tcpSocket2->peerPort();
                QString str=QString("[%1:%2]成功连接(客户端)").arg(ip).arg(port);
                qDebug()<<str;
                connect(tcpSocket2,&QTcpSocket::readyRead,
                        [=]()
                        {
                            QByteArray array=tcpSocket2->readAll();
                            QStringList splitArray=QString(array).split("@");
                            krrd=splitArray[1];
                            QString krr=hashXor(krrd,d);
                            qDebug()<<"收到krrd:"<<krrd;
                            qDebug()<<"解密得到krr:"<<krr;
                            if(splitArray[0]=="download"){//下载文件
                                QString dk="download@"+krr;
                                //发送给客户端
                                QString ip="127.0.0.1";
                                qint64 port=10001;
                                tcpSocket1->connectToHost(QHostAddress(ip),port);
                                tcpSocket1->write(dk.toUtf8());
                                tcpSocket1->disconnectFromHost();
                                tcpSocket1->close();
                            }else{//上传文件
                                QString uk="upload@"+krr;
                                //发送给客户端
                                QString ip="127.0.0.1";
                                qint64 port=10001;
                                tcpSocket1->connectToHost(QHostAddress(ip),port);
                                tcpSocket1->write(uk.toUtf8());
                                tcpSocket1->disconnectFromHost();
                                tcpSocket1->close();
                            }
                        });
            });
}

Ks::~Ks()
{
    delete ui;
}


void Ks::on_pushButton_clicked()
{
    //向csp发送更新请求
    QString ip="127.0.0.1";
    qint64 port=10003;
    //生成新d
    unsigned char* ud=new unsigned char[32];
    d2=get_rand_key(ud, 32);
    dd=hashXor(d,d2);
    qDebug()<<"d1:"<<d;
    qDebug()<<"d2:"<<d2;
    qDebug()<<"dd:"<<dd;
    d=d2;
    tcpSocket2->connectToHost(QHostAddress(ip),port);
    tcpSocket2->write(dd.toLatin1());
    qDebug()<<"-----------------更新d";
    tcpSocket2->disconnectFromHost();
    tcpSocket2->close();
}

//生成随机密钥
QString Ks::get_rand_key(unsigned char * key,int l){
    RAND_bytes(key, l);
    QString result=QByteArray((char*)key,l).toHex();
    return result;
}

//两个32字节十六进制字符串异或
QString Ks::hashXor(QString hexStr1,QString hexStr2){
    QString result;
    for(int i=0;i<32;i++){//TODO 只计算了r的前3个字节
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
