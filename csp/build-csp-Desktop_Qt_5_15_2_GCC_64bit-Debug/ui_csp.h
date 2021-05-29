/********************************************************************************
** Form generated from reading UI file 'csp.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CSP_H
#define UI_CSP_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Csp
{
public:
    QTextEdit *textEdit;
    QPushButton *pushButton;
    QLineEdit *lineEdit;
    QPushButton *pushButton_2;

    void setupUi(QWidget *Csp)
    {
        if (Csp->objectName().isEmpty())
            Csp->setObjectName(QString::fromUtf8("Csp"));
        Csp->resize(800, 600);
        textEdit = new QTextEdit(Csp);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));
        textEdit->setGeometry(QRect(20, 20, 751, 491));
        pushButton = new QPushButton(Csp);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(480, 520, 131, 61));
        lineEdit = new QLineEdit(Csp);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        lineEdit->setGeometry(QRect(170, 530, 151, 41));
        pushButton_2 = new QPushButton(Csp);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));
        pushButton_2->setGeometry(QRect(640, 520, 131, 61));

        retranslateUi(Csp);

        QMetaObject::connectSlotsByName(Csp);
    } // setupUi

    void retranslateUi(QWidget *Csp)
    {
        Csp->setWindowTitle(QCoreApplication::translate("Csp", "Csp", nullptr));
        pushButton->setText(QCoreApplication::translate("Csp", "heihei", nullptr));
        lineEdit->setText(QCoreApplication::translate("Csp", "1", nullptr));
        pushButton_2->setText(QCoreApplication::translate("Csp", "haha", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Csp: public Ui_Csp {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CSP_H
