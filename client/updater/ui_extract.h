#ifndef UI_EXTRACT_H
#define UI_EXTRACT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>

class Ui_UpdaterExtract
{
public:
    QProgressBar *progressBar;
    QLabel *label;
    QPushButton *cancelButton;

    void setupUi(QDialog *UpdaterExtract)
    {
    UpdaterExtract->setObjectName(QString::fromUtf8("UpdaterExtract"));
    UpdaterExtract->resize(QSize(377, 88).expandedTo(UpdaterExtract->minimumSizeHint()));
    UpdaterExtract->setWindowIcon(QIcon(QString::fromUtf8(":/updater/resources/icon.xpm")));
    UpdaterExtract->setModal(true);
    progressBar = new QProgressBar(UpdaterExtract);
    progressBar->setObjectName(QString::fromUtf8("progressBar"));
    progressBar->setGeometry(QRect(9, 28, 359, 22));
    progressBar->setValue(24);
    progressBar->setTextVisible(false);
    label = new QLabel(UpdaterExtract);
    label->setObjectName(QString::fromUtf8("label"));
    label->setGeometry(QRect(9, 9, 359, 16));
    label->setTextFormat(Qt::RichText);
    label->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    cancelButton = new QPushButton(UpdaterExtract);
    cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
    cancelButton->setGeometry(QRect(9, 55, 81, 23));
    cancelButton->setFlat(false);
    retranslateUi(UpdaterExtract);
    QObject::connect(cancelButton, SIGNAL(clicked()), UpdaterExtract, SLOT(reject()));

    QMetaObject::connectSlotsByName(UpdaterExtract);
    } // setupUi

    void retranslateUi(QDialog *UpdaterExtract)
    {
    UpdaterExtract->setWindowTitle(QApplication::translate("UpdaterExtract", "Extracting update...", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("UpdaterExtract", "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\" white-space: pre-wrap; font-family:MS Shell Dlg; font-weight:400; font-style:normal; text-decoration:none;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Extracting ...</p></body></html>", 0, QApplication::UnicodeUTF8));
    cancelButton->setText(QApplication::translate("UpdaterExtract", "&Cancel", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(UpdaterExtract);
    } // retranslateUi

};

namespace Ui {
    class UpdaterExtract: public Ui_UpdaterExtract {};
} // namespace Ui

#endif // UI_EXTRACT_H
