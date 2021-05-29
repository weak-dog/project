/********************************************************************************
** Form generated from reading UI file 'client.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CLIENT_H
#define UI_CLIENT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Client
{
public:
    QGridLayout *gridLayout;
    QLabel *label;
    QTextEdit *textEdit;
    QSpacerItem *horizontalSpacer;
    QLabel *label_2;
    QListWidget *listWidget;
    QTableView *tableView;
    QLabel *label_3;
    QPushButton *pushButtonDownload;
    QPushButton *pushButtonSelect;
    QPushButton *pushButtonChange;
    QPushButton *pushButtonUpload;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *Client)
    {
        if (Client->objectName().isEmpty())
            Client->setObjectName(QString::fromUtf8("Client"));
        Client->resize(800, 600);
        gridLayout = new QGridLayout(Client);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(Client);
        label->setObjectName(QString::fromUtf8("label"));
        QFont font;
        font.setFamily(QString::fromUtf8("\346\245\267\344\275\223"));
        font.setPointSize(12);
        label->setFont(font);

        gridLayout->addWidget(label, 0, 0, 1, 2);

        textEdit = new QTextEdit(Client);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));

        gridLayout->addWidget(textEdit, 1, 0, 1, 3);

        horizontalSpacer = new QSpacerItem(413, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 6, 2, 1, 1);

        label_2 = new QLabel(Client);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setFont(font);

        gridLayout->addWidget(label_2, 0, 3, 1, 1);

        listWidget = new QListWidget(Client);
        new QListWidgetItem(listWidget);
        new QListWidgetItem(listWidget);
        new QListWidgetItem(listWidget);
        new QListWidgetItem(listWidget);
        new QListWidgetItem(listWidget);
        new QListWidgetItem(listWidget);
        listWidget->setObjectName(QString::fromUtf8("listWidget"));
        listWidget->setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed|QAbstractItemView::SelectedClicked);
        listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

        gridLayout->addWidget(listWidget, 4, 0, 2, 3);

        tableView = new QTableView(Client);
        tableView->setObjectName(QString::fromUtf8("tableView"));
        tableView->setEditTriggers(QAbstractItemView::AnyKeyPressed|QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed);
        tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        tableView->horizontalHeader()->setStretchLastSection(true);

        gridLayout->addWidget(tableView, 1, 3, 5, 2);

        label_3 = new QLabel(Client);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setFont(font);

        gridLayout->addWidget(label_3, 3, 0, 1, 2);

        pushButtonDownload = new QPushButton(Client);
        pushButtonDownload->setObjectName(QString::fromUtf8("pushButtonDownload"));

        gridLayout->addWidget(pushButtonDownload, 6, 4, 1, 1);

        pushButtonSelect = new QPushButton(Client);
        pushButtonSelect->setObjectName(QString::fromUtf8("pushButtonSelect"));

        gridLayout->addWidget(pushButtonSelect, 6, 0, 1, 1);

        pushButtonChange = new QPushButton(Client);
        pushButtonChange->setObjectName(QString::fromUtf8("pushButtonChange"));

        gridLayout->addWidget(pushButtonChange, 6, 3, 1, 1);

        pushButtonUpload = new QPushButton(Client);
        pushButtonUpload->setObjectName(QString::fromUtf8("pushButtonUpload"));

        gridLayout->addWidget(pushButtonUpload, 6, 1, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 2, 0, 1, 1);


        retranslateUi(Client);

        QMetaObject::connectSlotsByName(Client);
    } // setupUi

    void retranslateUi(QWidget *Client)
    {
        Client->setWindowTitle(QCoreApplication::translate("Client", "Client", nullptr));
        label->setText(QCoreApplication::translate("Client", "\346\262\241\346\234\211bug\347\232\204\345\256\242\346\210\267\347\253\257", nullptr));
        label_2->setText(QCoreApplication::translate("Client", "\346\226\207\344\273\266\345\210\227\350\241\250", nullptr));

        const bool __sortingEnabled = listWidget->isSortingEnabled();
        listWidget->setSortingEnabled(false);
        QListWidgetItem *___qlistwidgetitem = listWidget->item(0);
        ___qlistwidgetitem->setText(QCoreApplication::translate("Client", "user1", nullptr));
        QListWidgetItem *___qlistwidgetitem1 = listWidget->item(1);
        ___qlistwidgetitem1->setText(QCoreApplication::translate("Client", "user2", nullptr));
        QListWidgetItem *___qlistwidgetitem2 = listWidget->item(2);
        ___qlistwidgetitem2->setText(QCoreApplication::translate("Client", "user3", nullptr));
        QListWidgetItem *___qlistwidgetitem3 = listWidget->item(3);
        ___qlistwidgetitem3->setText(QCoreApplication::translate("Client", "user4", nullptr));
        QListWidgetItem *___qlistwidgetitem4 = listWidget->item(4);
        ___qlistwidgetitem4->setText(QCoreApplication::translate("Client", "user5", nullptr));
        QListWidgetItem *___qlistwidgetitem5 = listWidget->item(5);
        ___qlistwidgetitem5->setText(QCoreApplication::translate("Client", "user6", nullptr));
        listWidget->setSortingEnabled(__sortingEnabled);

        label_3->setText(QCoreApplication::translate("Client", "\351\200\211\346\213\251\347\276\244\347\273\204\347\224\250\346\210\267", nullptr));
        pushButtonDownload->setText(QCoreApplication::translate("Client", "\344\270\213\350\275\275", nullptr));
        pushButtonSelect->setText(QCoreApplication::translate("Client", "\351\200\211\346\213\251", nullptr));
        pushButtonChange->setText(QCoreApplication::translate("Client", "\346\233\264\346\226\260S", nullptr));
        pushButtonUpload->setText(QCoreApplication::translate("Client", "\344\270\212\344\274\240", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Client: public Ui_Client {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CLIENT_H
