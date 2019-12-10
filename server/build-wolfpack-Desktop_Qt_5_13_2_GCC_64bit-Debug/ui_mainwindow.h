/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.13.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QVBoxLayout *vboxLayout;
    QFrame *frame;
    QVBoxLayout *vboxLayout1;
    QLabel *label;
    QTextBrowser *logWindow;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(582, 403);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        vboxLayout = new QVBoxLayout(centralWidget);
        vboxLayout->setSpacing(0);
        vboxLayout->setContentsMargins(0, 0, 0, 0);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        frame = new QFrame(centralWidget);
        frame->setObjectName(QString::fromUtf8("frame"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(frame->sizePolicy().hasHeightForWidth());
        frame->setSizePolicy(sizePolicy);
        QPalette palette;
        palette.setColor(QPalette::Active, QPalette::WindowText, QColor(255, 255, 255));
        palette.setColor(QPalette::Active, QPalette::Button, QColor(0, 64, 38));
        palette.setColor(QPalette::Active, QPalette::Light, QColor(0, 96, 57));
        palette.setColor(QPalette::Active, QPalette::Midlight, QColor(0, 80, 47));
        palette.setColor(QPalette::Active, QPalette::Dark, QColor(0, 32, 19));
        palette.setColor(QPalette::Active, QPalette::Mid, QColor(0, 42, 25));
        palette.setColor(QPalette::Active, QPalette::Text, QColor(255, 255, 255));
        palette.setColor(QPalette::Active, QPalette::BrightText, QColor(255, 255, 255));
        palette.setColor(QPalette::Active, QPalette::ButtonText, QColor(255, 255, 255));
        palette.setColor(QPalette::Active, QPalette::Base, QColor(0, 0, 0));
        palette.setColor(QPalette::Active, QPalette::Window, QColor(0, 64, 38));
        palette.setColor(QPalette::Active, QPalette::Shadow, QColor(0, 0, 0));
        palette.setColor(QPalette::Active, QPalette::Highlight, QColor(49, 106, 197));
        palette.setColor(QPalette::Active, QPalette::HighlightedText, QColor(255, 255, 255));
        palette.setColor(QPalette::Active, QPalette::Link, QColor(0, 0, 255));
        palette.setColor(QPalette::Active, QPalette::LinkVisited, QColor(255, 0, 255));
        palette.setColor(QPalette::Active, QPalette::AlternateBase, QColor(0, 32, 19));
        palette.setColor(QPalette::Inactive, QPalette::WindowText, QColor(255, 255, 255));
        palette.setColor(QPalette::Inactive, QPalette::Button, QColor(0, 64, 38));
        palette.setColor(QPalette::Inactive, QPalette::Light, QColor(0, 96, 57));
        palette.setColor(QPalette::Inactive, QPalette::Midlight, QColor(0, 80, 47));
        palette.setColor(QPalette::Inactive, QPalette::Dark, QColor(0, 32, 19));
        palette.setColor(QPalette::Inactive, QPalette::Mid, QColor(0, 42, 25));
        palette.setColor(QPalette::Inactive, QPalette::Text, QColor(255, 255, 255));
        palette.setColor(QPalette::Inactive, QPalette::BrightText, QColor(255, 255, 255));
        palette.setColor(QPalette::Inactive, QPalette::ButtonText, QColor(255, 255, 255));
        palette.setColor(QPalette::Inactive, QPalette::Base, QColor(0, 0, 0));
        palette.setColor(QPalette::Inactive, QPalette::Window, QColor(0, 64, 38));
        palette.setColor(QPalette::Inactive, QPalette::Shadow, QColor(0, 0, 0));
        palette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(236, 233, 216));
        palette.setColor(QPalette::Inactive, QPalette::HighlightedText, QColor(0, 0, 0));
        palette.setColor(QPalette::Inactive, QPalette::Link, QColor(0, 0, 255));
        palette.setColor(QPalette::Inactive, QPalette::LinkVisited, QColor(255, 0, 255));
        palette.setColor(QPalette::Inactive, QPalette::AlternateBase, QColor(0, 32, 19));
        palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(0, 32, 19));
        palette.setColor(QPalette::Disabled, QPalette::Button, QColor(0, 64, 38));
        palette.setColor(QPalette::Disabled, QPalette::Light, QColor(0, 96, 57));
        palette.setColor(QPalette::Disabled, QPalette::Midlight, QColor(0, 80, 47));
        palette.setColor(QPalette::Disabled, QPalette::Dark, QColor(0, 32, 19));
        palette.setColor(QPalette::Disabled, QPalette::Mid, QColor(0, 42, 25));
        palette.setColor(QPalette::Disabled, QPalette::Text, QColor(0, 32, 19));
        palette.setColor(QPalette::Disabled, QPalette::BrightText, QColor(255, 255, 255));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0, 32, 19));
        palette.setColor(QPalette::Disabled, QPalette::Base, QColor(0, 64, 38));
        palette.setColor(QPalette::Disabled, QPalette::Window, QColor(0, 64, 38));
        palette.setColor(QPalette::Disabled, QPalette::Shadow, QColor(0, 0, 0));
        palette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(49, 106, 197));
        palette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(255, 255, 255));
        palette.setColor(QPalette::Disabled, QPalette::Link, QColor(0, 0, 255));
        palette.setColor(QPalette::Disabled, QPalette::LinkVisited, QColor(255, 0, 255));
        palette.setColor(QPalette::Disabled, QPalette::AlternateBase, QColor(0, 64, 38));
        frame->setPalette(palette);
        frame->setAutoFillBackground(true);
        frame->setFrameShape(QFrame::NoFrame);
        frame->setFrameShadow(QFrame::Plain);
        frame->setLineWidth(0);
        vboxLayout1 = new QVBoxLayout(frame);
        vboxLayout1->setSpacing(0);
        vboxLayout1->setContentsMargins(0, 0, 0, 0);
        vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
        label = new QLabel(frame);
        label->setObjectName(QString::fromUtf8("label"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy1);
        label->setPixmap(QPixmap(QString::fromUtf8(":/gui/images/logo.png")));

        vboxLayout1->addWidget(label);


        vboxLayout->addWidget(frame);

        logWindow = new QTextBrowser(centralWidget);
        logWindow->setObjectName(QString::fromUtf8("logWindow"));
        QPalette palette1;
        palette1.setColor(QPalette::Active, QPalette::WindowText, QColor(255, 255, 255));
        palette1.setColor(QPalette::Active, QPalette::Button, QColor(236, 233, 216));
        palette1.setColor(QPalette::Active, QPalette::Light, QColor(255, 255, 255));
        palette1.setColor(QPalette::Active, QPalette::Midlight, QColor(245, 244, 235));
        palette1.setColor(QPalette::Active, QPalette::Dark, QColor(118, 116, 108));
        palette1.setColor(QPalette::Active, QPalette::Mid, QColor(157, 155, 144));
        palette1.setColor(QPalette::Active, QPalette::Text, QColor(255, 255, 255));
        palette1.setColor(QPalette::Active, QPalette::BrightText, QColor(255, 255, 255));
        palette1.setColor(QPalette::Active, QPalette::ButtonText, QColor(255, 255, 255));
        palette1.setColor(QPalette::Active, QPalette::Base, QColor(0, 0, 0));
        palette1.setColor(QPalette::Active, QPalette::Window, QColor(0, 0, 0));
        palette1.setColor(QPalette::Active, QPalette::Shadow, QColor(0, 0, 0));
        palette1.setColor(QPalette::Active, QPalette::Highlight, QColor(49, 106, 197));
        palette1.setColor(QPalette::Active, QPalette::HighlightedText, QColor(255, 255, 255));
        palette1.setColor(QPalette::Active, QPalette::Link, QColor(0, 0, 255));
        palette1.setColor(QPalette::Active, QPalette::LinkVisited, QColor(255, 0, 255));
        palette1.setColor(QPalette::Active, QPalette::AlternateBase, QColor(245, 244, 235));
        palette1.setColor(QPalette::Inactive, QPalette::WindowText, QColor(255, 255, 255));
        palette1.setColor(QPalette::Inactive, QPalette::Button, QColor(236, 233, 216));
        palette1.setColor(QPalette::Inactive, QPalette::Light, QColor(255, 255, 255));
        palette1.setColor(QPalette::Inactive, QPalette::Midlight, QColor(255, 255, 250));
        palette1.setColor(QPalette::Inactive, QPalette::Dark, QColor(118, 116, 108));
        palette1.setColor(QPalette::Inactive, QPalette::Mid, QColor(157, 155, 144));
        palette1.setColor(QPalette::Inactive, QPalette::Text, QColor(255, 255, 255));
        palette1.setColor(QPalette::Inactive, QPalette::BrightText, QColor(255, 255, 255));
        palette1.setColor(QPalette::Inactive, QPalette::ButtonText, QColor(255, 255, 255));
        palette1.setColor(QPalette::Inactive, QPalette::Base, QColor(0, 0, 0));
        palette1.setColor(QPalette::Inactive, QPalette::Window, QColor(0, 0, 0));
        palette1.setColor(QPalette::Inactive, QPalette::Shadow, QColor(0, 0, 0));
        palette1.setColor(QPalette::Inactive, QPalette::Highlight, QColor(236, 233, 216));
        palette1.setColor(QPalette::Inactive, QPalette::HighlightedText, QColor(0, 0, 0));
        palette1.setColor(QPalette::Inactive, QPalette::Link, QColor(0, 0, 255));
        palette1.setColor(QPalette::Inactive, QPalette::LinkVisited, QColor(255, 0, 255));
        palette1.setColor(QPalette::Inactive, QPalette::AlternateBase, QColor(245, 244, 235));
        palette1.setColor(QPalette::Disabled, QPalette::WindowText, QColor(128, 128, 128));
        palette1.setColor(QPalette::Disabled, QPalette::Button, QColor(236, 233, 216));
        palette1.setColor(QPalette::Disabled, QPalette::Light, QColor(255, 255, 255));
        palette1.setColor(QPalette::Disabled, QPalette::Midlight, QColor(255, 255, 250));
        palette1.setColor(QPalette::Disabled, QPalette::Dark, QColor(118, 116, 108));
        palette1.setColor(QPalette::Disabled, QPalette::Mid, QColor(157, 155, 144));
        palette1.setColor(QPalette::Disabled, QPalette::Text, QColor(128, 128, 128));
        palette1.setColor(QPalette::Disabled, QPalette::BrightText, QColor(255, 255, 255));
        palette1.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(128, 128, 128));
        palette1.setColor(QPalette::Disabled, QPalette::Base, QColor(0, 0, 0));
        palette1.setColor(QPalette::Disabled, QPalette::Window, QColor(0, 0, 0));
        palette1.setColor(QPalette::Disabled, QPalette::Shadow, QColor(0, 0, 0));
        palette1.setColor(QPalette::Disabled, QPalette::Highlight, QColor(49, 106, 197));
        palette1.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(255, 255, 255));
        palette1.setColor(QPalette::Disabled, QPalette::Link, QColor(0, 0, 255));
        palette1.setColor(QPalette::Disabled, QPalette::LinkVisited, QColor(255, 0, 255));
        palette1.setColor(QPalette::Disabled, QPalette::AlternateBase, QColor(245, 244, 235));
        logWindow->setPalette(palette1);

        vboxLayout->addWidget(logWindow);

        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Wolfpack", nullptr));
        label->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
