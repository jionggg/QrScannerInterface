/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QStackedWidget *stackedWidget;
    QWidget *page;
    QPushButton *launchButton;
    QWidget *page_2;
    QWidget *widget;
    QGridLayout *gridLayout;
    QLabel *label_7;
    QLabel *label;
    QLabel *label_9;
    QLabel *label_8;
    QLabel *label_2;
    QLabel *label_5;
    QLabel *label_3;
    QLabel *label_6;
    QLabel *label_4;
    QPushButton *exitButton;
    QTextEdit *textEdit;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        stackedWidget = new QStackedWidget(centralwidget);
        stackedWidget->setObjectName("stackedWidget");
        stackedWidget->setGeometry(QRect(80, 40, 691, 501));
        page = new QWidget();
        page->setObjectName("page");
        launchButton = new QPushButton(page);
        launchButton->setObjectName("launchButton");
        launchButton->setGeometry(QRect(190, 160, 151, 61));
        stackedWidget->addWidget(page);
        page_2 = new QWidget();
        page_2->setObjectName("page_2");
        widget = new QWidget(page_2);
        widget->setObjectName("widget");
        widget->setGeometry(QRect(10, 20, 621, 321));
        gridLayout = new QGridLayout(widget);
        gridLayout->setObjectName("gridLayout");
        label_7 = new QLabel(widget);
        label_7->setObjectName("label_7");

        gridLayout->addWidget(label_7, 0, 2, 1, 1);

        label = new QLabel(widget);
        label->setObjectName("label");

        gridLayout->addWidget(label, 2, 0, 1, 1);

        label_9 = new QLabel(widget);
        label_9->setObjectName("label_9");

        gridLayout->addWidget(label_9, 2, 2, 1, 1);

        label_8 = new QLabel(widget);
        label_8->setObjectName("label_8");

        gridLayout->addWidget(label_8, 1, 2, 1, 1);

        label_2 = new QLabel(widget);
        label_2->setObjectName("label_2");

        gridLayout->addWidget(label_2, 0, 0, 1, 1);

        label_5 = new QLabel(widget);
        label_5->setObjectName("label_5");

        gridLayout->addWidget(label_5, 1, 1, 1, 1);

        label_3 = new QLabel(widget);
        label_3->setObjectName("label_3");

        gridLayout->addWidget(label_3, 1, 0, 1, 1);

        label_6 = new QLabel(widget);
        label_6->setObjectName("label_6");

        gridLayout->addWidget(label_6, 2, 1, 1, 1);

        label_4 = new QLabel(widget);
        label_4->setObjectName("label_4");

        gridLayout->addWidget(label_4, 0, 1, 1, 1);

        exitButton = new QPushButton(page_2);
        exitButton->setObjectName("exitButton");
        exitButton->setGeometry(QRect(640, 10, 51, 31));
        textEdit = new QTextEdit(page_2);
        textEdit->setObjectName("textEdit");
        textEdit->setGeometry(QRect(10, 351, 601, 141));
        stackedWidget->addWidget(page_2);
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 25));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        stackedWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        launchButton->setText(QCoreApplication::translate("MainWindow", "Launch", nullptr));
        label_7->setText(QCoreApplication::translate("MainWindow", "TextLabel", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "TextLabel", nullptr));
        label_9->setText(QCoreApplication::translate("MainWindow", "TextLabel", nullptr));
        label_8->setText(QCoreApplication::translate("MainWindow", "TextLabel", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "TextLabel", nullptr));
        label_5->setText(QCoreApplication::translate("MainWindow", "TextLabel", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "TextLabel", nullptr));
        label_6->setText(QCoreApplication::translate("MainWindow", "TextLabel", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "TextLabel", nullptr));
        exitButton->setText(QCoreApplication::translate("MainWindow", "Exit", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
