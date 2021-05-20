/********************************************************************************
** Form generated from reading UI file 'ks.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_KS_H
#define UI_KS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Ks
{
public:
    QLabel *label;
    QPushButton *pushButton;

    void setupUi(QWidget *Ks)
    {
        if (Ks->objectName().isEmpty())
            Ks->setObjectName(QString::fromUtf8("Ks"));
        Ks->resize(483, 325);
        label = new QLabel(Ks);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(100, 10, 291, 101));
        QFont font;
        font.setPointSize(14);
        label->setFont(font);
        pushButton = new QPushButton(Ks);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(170, 120, 111, 51));

        retranslateUi(Ks);

        QMetaObject::connectSlotsByName(Ks);
    } // setupUi

    void retranslateUi(QWidget *Ks)
    {
        Ks->setWindowTitle(QCoreApplication::translate("Ks", "Ks", nullptr));
        label->setText(QCoreApplication::translate("Ks", "\344\270\255\345\233\275\345\205\261\344\272\247\345\205\232\344\270\207\345\262\201", nullptr));
        pushButton->setText(QCoreApplication::translate("Ks", "\346\233\264\346\226\260d", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Ks: public Ui_Ks {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_KS_H
