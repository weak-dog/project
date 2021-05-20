#include "login.h"
#include "ui_login.h"

Login::Login(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Login)
{
    ui->setupUi(this);
    loginSocket=new QTcpSocket(this);

    //接收登录结果
    connect(loginSocket,&QTcpSocket::readyRead,
            [=]()
            {
                QByteArray array=loginSocket->readAll();
                if(QString(array)=="success"){//登录成功
                    QMessageBox::information(this,"","登录成功");
                    loginSocket->disconnectFromHost();
                    loginSocket->close();
                    c=new Client(account,password);
                    c->show();
                    this->close();
                    }else{//登录失败
                        QMessageBox::information(this,"","登录失败");
                        //TODO 重新登录
                    }
            });
}

Login::~Login()
{
    delete ui;
}

void Login::on_buttonLogin_clicked()
{
    // 获取用户输入的账号和密码
    account = ui->account->text();
    password = ui->password->text();
    password=Security().HashMD5(password);
    //连接服务器
    QString ip="127.0.0.1";
    qint64 port=10002;
    loginSocket->connectToHost(QHostAddress(ip),port);
    //发送登录请求
    QString loginCommand=QString("%1@%2@login@@").arg(account).arg(password);
    loginSocket->write(loginCommand.toUtf8());
}
