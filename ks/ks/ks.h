#ifndef KS_H
#define KS_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QFileInfo>
#include <QDebug>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <string.h>
#include <unistd.h>

QT_BEGIN_NAMESPACE
namespace Ui { class Ks; }
QT_END_NAMESPACE

class Ks : public QMainWindow
{
    Q_OBJECT

public:
    Ks(QWidget *parent = nullptr);
    ~Ks();
    QString get_rand_key(unsigned char * key,int l);//生成随机密钥
    QString hashXor(QString hexStr1,QString hexStr2);//十六进制字符串异或
private slots:
    void on_pushButton_clicked();

private:
    Ui::Ks *ui;
    QTcpServer *tcpServer1;//监听客户端消息
    QTcpServer *tcpServer2;//监听csp消息
    QTcpSocket *tcpSocket1;//与客户端消息传输
    QTcpSocket *tcpSocket2;//与csp消息传输
    QString d;//系统密钥
    QString d2;//新的系统密钥
    QString dd;//新旧密钥异或
    QString krrd;
};
#endif // KS_H
