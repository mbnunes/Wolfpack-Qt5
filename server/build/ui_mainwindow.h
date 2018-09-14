/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>
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
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(582, 403);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        vboxLayout = new QVBoxLayout(centralWidget);
        vboxLayout->setSpacing(0);
        vboxLayout->setContentsMargins(0, 0, 0, 0);
        vboxLayout->setObjectName(QStringLiteral("vboxLayout"));
        frame = new QFrame(centralWidget);
        frame->setObjectName(QStringLiteral("frame"));
        QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(0));
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(frame->sizePolicy().hasHeightForWidth());
        frame->setSizePolicy(sizePolicy);
        QPalette palette;
        palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(0), QColor(255, 255, 255));
        palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(1), QColor(0, 64, 38));
        palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(2), QColor(0, 96, 57));
        palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(3), QColor(0, 80, 47));
        palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(4), QColor(0, 32, 19));
        palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(5), QColor(0, 42, 25));
        palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(6), QColor(255, 255, 255));
        palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(7), QColor(255, 255, 255));
        palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(8), QColor(255, 255, 255));
        palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(9), QColor(0, 0, 0));
        palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(10), QColor(0, 64, 38));
        palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(11), QColor(0, 0, 0));
        palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(12), QColor(49, 106, 197));
        palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(13), QColor(255, 255, 255));
        palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(14), QColor(0, 0, 255));
        palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(15), QColor(255, 0, 255));
        palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(16), QColor(0, 32, 19));
        palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(0), QColor(255, 255, 255));
        palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(1), QColor(0, 64, 38));
        palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(2), QColor(0, 96, 57));
        palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(3), QColor(0, 80, 47));
        palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(4), QColor(0, 32, 19));
        palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(5), QColor(0, 42, 25));
        palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(6), QColor(255, 255, 255));
        palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(7), QColor(255, 255, 255));
        palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(8), QColor(255, 255, 255));
        palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(9), QColor(0, 0, 0));
        palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(10), QColor(0, 64, 38));
        palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(11), QColor(0, 0, 0));
        palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(12), QColor(236, 233, 216));
        palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(13), QColor(0, 0, 0));
        palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(14), QColor(0, 0, 255));
        palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(15), QColor(255, 0, 255));
        palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(16), QColor(0, 32, 19));
        palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(0), QColor(0, 32, 19));
        palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(1), QColor(0, 64, 38));
        palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(2), QColor(0, 96, 57));
        palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(3), QColor(0, 80, 47));
        palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(4), QColor(0, 32, 19));
        palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(5), QColor(0, 42, 25));
        palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(6), QColor(0, 32, 19));
        palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(7), QColor(255, 255, 255));
        palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(8), QColor(0, 32, 19));
        palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(9), QColor(0, 64, 38));
        palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(10), QColor(0, 64, 38));
        palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(11), QColor(0, 0, 0));
        palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(12), QColor(49, 106, 197));
        palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(13), QColor(255, 255, 255));
        palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(14), QColor(0, 0, 255));
        palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(15), QColor(255, 0, 255));
        palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(16), QColor(0, 64, 38));
        frame->setPalette(palette);
        frame->setAutoFillBackground(true);
        frame->setFrameShape(QFrame::NoFrame);
        frame->setFrameShadow(QFrame::Plain);
        frame->setLineWidth(0);
        vboxLayout1 = new QVBoxLayout(frame);
        vboxLayout1->setSpacing(0);
        vboxLayout1->setContentsMargins(0, 0, 0, 0);
        vboxLayout1->setObjectName(QStringLiteral("vboxLayout1"));
        label = new QLabel(frame);
        label->setObjectName(QStringLiteral("label"));
        QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy1);
        label->setPixmap(QPixmap(QString::fromUtf8(":/gui/images/logo.png")));

        vboxLayout1->addWidget(label);


        vboxLayout->addWidget(frame);

        logWindow = new QTextBrowser(centralWidget);
        logWindow->setObjectName(QStringLiteral("logWindow"));
        QPalette palette1;
        palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(0), QColor(255, 255, 255));
        palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(1), QColor(236, 233, 216));
        palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(2), QColor(255, 255, 255));
        palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(3), QColor(245, 244, 235));
        palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(4), QColor(118, 116, 108));
        palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(5), QColor(157, 155, 144));
        palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(6), QColor(255, 255, 255));
        palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(7), QColor(255, 255, 255));
        palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(8), QColor(255, 255, 255));
        palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(9), QColor(0, 0, 0));
        palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(10), QColor(0, 0, 0));
        palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(11), QColor(0, 0, 0));
        palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(12), QColor(49, 106, 197));
        palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(13), QColor(255, 255, 255));
        palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(14), QColor(0, 0, 255));
        palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(15), QColor(255, 0, 255));
        palette1.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(16), QColor(245, 244, 235));
        palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(0), QColor(255, 255, 255));
        palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(1), QColor(236, 233, 216));
        palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(2), QColor(255, 255, 255));
        palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(3), QColor(255, 255, 250));
        palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(4), QColor(118, 116, 108));
        palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(5), QColor(157, 155, 144));
        palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(6), QColor(255, 255, 255));
        palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(7), QColor(255, 255, 255));
        palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(8), QColor(255, 255, 255));
        palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(9), QColor(0, 0, 0));
        palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(10), QColor(0, 0, 0));
        palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(11), QColor(0, 0, 0));
        palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(12), QColor(236, 233, 216));
        palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(13), QColor(0, 0, 0));
        palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(14), QColor(0, 0, 255));
        palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(15), QColor(255, 0, 255));
        palette1.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(16), QColor(245, 244, 235));
        palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(0), QColor(128, 128, 128));
        palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(1), QColor(236, 233, 216));
        palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(2), QColor(255, 255, 255));
        palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(3), QColor(255, 255, 250));
        palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(4), QColor(118, 116, 108));
        palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(5), QColor(157, 155, 144));
        palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(6), QColor(128, 128, 128));
        palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(7), QColor(255, 255, 255));
        palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(8), QColor(128, 128, 128));
        palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(9), QColor(0, 0, 0));
        palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(10), QColor(0, 0, 0));
        palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(11), QColor(0, 0, 0));
        palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(12), QColor(49, 106, 197));
        palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(13), QColor(255, 255, 255));
        palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(14), QColor(0, 0, 255));
        palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(15), QColor(255, 0, 255));
        palette1.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(16), QColor(245, 244, 235));
        logWindow->setPalette(palette1);

        vboxLayout->addWidget(logWindow);

        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Wolfpack", 0));
        label->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
