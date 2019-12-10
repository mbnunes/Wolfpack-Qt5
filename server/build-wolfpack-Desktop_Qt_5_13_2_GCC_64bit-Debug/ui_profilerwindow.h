/********************************************************************************
** Form generated from reading UI file 'profilerwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.13.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROFILERWINDOW_H
#define UI_PROFILERWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ProfilerWindow
{
public:
    QWidget *centralWidget;
    QVBoxLayout *vboxLayout;
    QTreeView *treeView;

    void setupUi(QMainWindow *ProfilerWindow)
    {
        if (ProfilerWindow->objectName().isEmpty())
            ProfilerWindow->setObjectName(QString::fromUtf8("ProfilerWindow"));
        ProfilerWindow->resize(800, 600);
        centralWidget = new QWidget(ProfilerWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        vboxLayout = new QVBoxLayout(centralWidget);
#ifndef Q_OS_MAC
        vboxLayout->setSpacing(6);
#endif
        vboxLayout->setContentsMargins(8, 8, 8, 8);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        treeView = new QTreeView(centralWidget);
        treeView->setObjectName(QString::fromUtf8("treeView"));

        vboxLayout->addWidget(treeView);

        ProfilerWindow->setCentralWidget(centralWidget);

        retranslateUi(ProfilerWindow);

        QMetaObject::connectSlotsByName(ProfilerWindow);
    } // setupUi

    void retranslateUi(QMainWindow *ProfilerWindow)
    {
        ProfilerWindow->setWindowTitle(QCoreApplication::translate("ProfilerWindow", "MainWindow", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ProfilerWindow: public Ui_ProfilerWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROFILERWINDOW_H
