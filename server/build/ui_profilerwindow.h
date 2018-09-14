/********************************************************************************
** Form generated from reading UI file 'profilerwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROFILERWINDOW_H
#define UI_PROFILERWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
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
            ProfilerWindow->setObjectName(QStringLiteral("ProfilerWindow"));
        ProfilerWindow->resize(800, 600);
        centralWidget = new QWidget(ProfilerWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        vboxLayout = new QVBoxLayout(centralWidget);
#ifndef Q_OS_MAC
        vboxLayout->setSpacing(6);
#endif
        vboxLayout->setContentsMargins(8, 8, 8, 8);
        vboxLayout->setObjectName(QStringLiteral("vboxLayout"));
        treeView = new QTreeView(centralWidget);
        treeView->setObjectName(QStringLiteral("treeView"));

        vboxLayout->addWidget(treeView);

        ProfilerWindow->setCentralWidget(centralWidget);

        retranslateUi(ProfilerWindow);

        QMetaObject::connectSlotsByName(ProfilerWindow);
    } // setupUi

    void retranslateUi(QMainWindow *ProfilerWindow)
    {
        ProfilerWindow->setWindowTitle(QApplication::translate("ProfilerWindow", "MainWindow", 0));
    } // retranslateUi

};

namespace Ui {
    class ProfilerWindow: public Ui_ProfilerWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROFILERWINDOW_H
