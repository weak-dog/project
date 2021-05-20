#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QTcpSocket>
#include <QDebug>
#include <QHostAddress>
#include <QMessageBox>
#include "client.h"
#include "security.h"


QT_BEGIN_NAMESPACE
namespace Ui { class Login; }
QT_END_NAMESPACE

class Login : public QWidget
{
    Q_OBJECT

public:
    Login(QWidget *parent = nullptr);
    ~Login();
    Client* c;

private slots:
    void on_buttonLogin_clicked();

private:
    Ui::Login *ui;
    QString account;
    QString password;
    QTcpSocket* loginSocket;
};
#endif // LOGIN_H
