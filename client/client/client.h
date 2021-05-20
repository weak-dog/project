#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFileInfo>
#include <QDebug>
#include "security.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlTableModel>

namespace Ui {
class Client;
}

class Client : public QWidget
{
    Q_OBJECT

public:
    explicit Client(QString account,QString password,QWidget *parent = nullptr);
    ~Client();
    void sendData();//发送文件数据
    QString calcCs(QString r,QString s,QString filePath,QString& hsm);
    QString calcCs2(QString hms,QString r,QString Cs);
    bool DecCs(QString cs,QString& r);
    bool compareCs(QString Cs,QStringList& s);

private slots:
    void on_pushButtonSelect_clicked();
    void on_pushButtonUpload_clicked();
    void on_pushButtonDownload_clicked();
    void on_pushButtonChange_clicked();

private:
    Ui::Client *ui;
    QTcpServer *tcpServer1;//监听ks消息
    QTcpSocket *tcpSocket1;//与csp消息传输
    QTcpSocket *tcpSocket2;//与ks消息传输
    QTcpSocket *tcpSocket3;//与csp文件传输
    //命令元素
    QString account;
    QString password;
    qint32 type;//执行操作类型
    //接收消息
    QByteArray array;//消息缓冲区
    QStringList splitArray;//接收消息分片
    QStringList messages;//消息列表
    //数据库操作
    QSqlDatabase database;
    QSqlQuery sql_query;
    //文件传输
    QFile file;//文件对象
    QString filePath;//文件路径
    QString cipherPath;//密文路径
    QString fileName;//文件名
    QString fileHash;//文件哈希值
    QString fileSh;//文件短哈希
    QString token;//去重标签H(hf,r)
    qint64 fileSize;//文件大小
    qint64 recvSize;//已接收大小
    qint64 sendSize;//已发送文件大小
    //ui相关
    QSqlTableModel * fileModel;//文件列表
    QStringList users;//共享用户
    unsigned char* key;//随机秘钥Kr
    QString kr;//随机密钥Kr
    QString r;//群组随机值
    QStringList rl;//解密Cs得到的{r}
    QString r_index;//{r}中正确的索引
    QString index;//文件块索引
    QString blockHash;//块哈希
    QString hsm;//H(s,M)
};

#endif // CLIENT_H
