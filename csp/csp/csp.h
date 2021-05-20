#ifndef CSP_H
#define CSP_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QFileInfo>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <openssl/evp.h>
#include <QTime>

QT_BEGIN_NAMESPACE
namespace Ui { class Csp; }
QT_END_NAMESPACE

class Csp : public QMainWindow
{
    Q_OBJECT

public:
    Csp(QWidget *parent = nullptr);
    ~Csp();
    void sendData();//发送文件
    bool checkLogin(QString account, QString password);//查询登录
    bool checkFirst(QString fileSh);//查询是否为首次上传
    QStringList getCs(QString fileSh);//查询hash对应的cs
    bool getIndex(QStringList tokenlist,int& index);//查询哪一个token是正确的
    QString hashXor(QString hexStr1,QString hexStr2);
    QString sm3_f(QString filepath);//计算文件sm3哈希值
    QString block(QString filePath,int index);
    QString fileIndex(QString filePath);

private:
    Ui::Csp *ui;
    QTcpServer *tcpServer1;//监听客户端消息
    QTcpServer *tcpServer2;//监听ks消息
    QTcpServer *tcpServer3;//监听客户端文件
    QTcpSocket *tcpSocket1;//与客户端消息传输
    QTcpSocket *tcpSocket2;//与ks消息传输
    QTcpSocket *tcpSocket3;//与客户端文件传输
    //数据库操作
    QSqlDatabase database;
    QSqlQuery sql_query;
    QStringList messages;//消息列表
    bool firstUpload;
    bool secondButUpload;
    QFile file;//文件对象
    QString filePath;//文件路径
    QString fileSh;//文件短哈希
    qint64 fileSize;//文件大小
    qint64 recvSize;//已接收大小
    qint64 sendSize;//已发送文件大小
    int type;//操作类型：1->login|2->upload|3->download|4->changeS
    int sentCkrd;
    QStringList CsCkrd;
    QString token;
    QString Cs;
    QString krrd;
    QString fileName;
    QString blockHash;
    QString ptr;
};
#endif // CSP_H
