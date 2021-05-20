#ifndef SECURITY_H
#define SECURITY_H

#include <QFile>
#include <math.h>
#include <QString>
#include <QDateTime>
#include <QCryptographicHash>
#include <QDebug>
#include <QSettings>
#include <QMessageBox>
#include <QApplication>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QString>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <openssl/hmac.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <QMessageAuthenticationCode>

class Security
{
public:
    Security();
    QString HashMD5(QString password);//字符串MD5哈希值，登录验证
    QString TextToBase64(QString text);
    QString Base64ToText(QString text);
    QByteArray HexStrToByte(QString str);
    QString sha_f(QString filepath);//计算文件sha256哈希值
    QString sm3_f(QString filepath);//计算文件sm3哈希值
    QString genSh(QString filehash);//计算文件sm3短哈希值
    QString get_rand_key(unsigned char * key,int l);//生成随机密钥
    int Encrypt_File(unsigned char* key,QString);//sm4加密文件
    int Decrypt_File(unsigned char* key,QString);//sm4解密文件
    QString hmac(QByteArray key,QString filePath);
    QString hmacS(QByteArray key,QByteArray message);
    QString HMAC_SM3(QString data, QString key);//sm3-hmac
    QString hashXor(QString hexStr1,QString hexStr2);
    QString block(QString filePath,int index);
};

#endif // SECURITY_H
