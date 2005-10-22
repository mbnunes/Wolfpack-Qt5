#ifndef UI_DOWNLOAD_H
#define UI_DOWNLOAD_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>

class Ui_UpdaterDownload
{
public:
    QProgressBar *progressBar;
    QPushButton *cancelButton;
    QLabel *label;

    void setupUi(QDialog *UpdaterDownload)
    {
    UpdaterDownload->setObjectName(QString::fromUtf8("UpdaterDownload"));
    UpdaterDownload->resize(QSize(377, 88).expandedTo(UpdaterDownload->minimumSizeHint()));
    UpdaterDownload->setWindowIcon(QIcon(QString::fromUtf8(":/updater/resources/icon.xpm")));
    UpdaterDownload->setModal(true);
    progressBar = new QProgressBar(UpdaterDownload);
    progressBar->setObjectName(QString::fromUtf8("progressBar"));
    progressBar->setGeometry(QRect(9, 28, 359, 22));
    progressBar->setValue(24);
    progressBar->setTextVisible(false);
    cancelButton = new QPushButton(UpdaterDownload);
    cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
    cancelButton->setGeometry(QRect(9, 55, 81, 23));
    cancelButton->setFlat(false);
    label = new QLabel(UpdaterDownload);
    label->setObjectName(QString::fromUtf8("label"));
    label->setGeometry(QRect(9, 9, 359, 16));
    label->setTextFormat(Qt::RichText);
    label->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    retranslateUi(UpdaterDownload);
    QObject::connect(cancelButton, SIGNAL(clicked()), UpdaterDownload, SLOT(reject()));

    QMetaObject::connectSlotsByName(UpdaterDownload);
    } // setupUi

    void retranslateUi(QDialog *UpdaterDownload)
    {
    UpdaterDownload->setWindowTitle(QApplication::translate("UpdaterDownload", "Extracting update...", 0, QApplication::UnicodeUTF8));
    cancelButton->setText(QApplication::translate("UpdaterDownload", "&Cancel", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("UpdaterDownload", "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\" white-space: pre-wrap; font-family:MS Shell Dlg; font-weight:400; font-style:normal; text-decoration:none;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Downloading ...</p></body></html>", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(UpdaterDownload);
    } // retranslateUi

};

namespace Ui {
    class UpdaterDownload: public Ui_UpdaterDownload {};
} // namespace Ui

#endif // UI_DOWNLOAD_H
