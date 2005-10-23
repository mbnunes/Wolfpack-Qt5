#ifndef UI_EXTRACT_H
#define UI_EXTRACT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>

class Ui_UpdaterExtract
{
public:
    QFrame *frame;
    QLabel *label_2;
    QFrame *line;
    QLabel *label_3;
    QLabel *label;
    QProgressBar *progressBar;
    QPushButton *cancelButton;

    void setupUi(QDialog *UpdaterExtract)
    {
    UpdaterExtract->setObjectName(QString::fromUtf8("UpdaterExtract"));
    UpdaterExtract->resize(QSize(378, 147).expandedTo(UpdaterExtract->minimumSizeHint()));
    UpdaterExtract->setMinimumSize(QSize(378, 147));
    UpdaterExtract->setMaximumSize(QSize(378, 147));
    UpdaterExtract->setWindowIcon(QIcon(QString::fromUtf8(":/updater/resources/icon.xpm")));
    UpdaterExtract->setModal(true);
    frame = new QFrame(UpdaterExtract);
    frame->setObjectName(QString::fromUtf8("frame"));
    frame->setEnabled(true);
    frame->setGeometry(QRect(0, 0, 377, 52));
    QPalette palette;
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(0), QColor(128, 128, 128));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(1), QColor(236, 233, 216));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(2), QColor(255, 255, 255));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(3), QColor(255, 255, 250));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(4), QColor(118, 116, 108));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(5), QColor(157, 155, 144));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(6), QColor(128, 128, 128));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(7), QColor(255, 255, 255));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(8), QColor(128, 128, 128));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(9), QColor(255, 255, 255));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(10), QColor(255, 255, 255));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(11), QColor(0, 0, 0));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(12), QColor(49, 106, 197));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(13), QColor(255, 255, 255));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(14), QColor(0, 0, 255));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(15), QColor(255, 0, 255));
    palette.setColor(QPalette::Active, static_cast<QPalette::ColorRole>(16), QColor(245, 244, 235));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(0), QColor(128, 128, 128));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(1), QColor(236, 233, 216));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(2), QColor(255, 255, 255));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(3), QColor(255, 255, 250));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(4), QColor(118, 116, 108));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(5), QColor(157, 155, 144));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(6), QColor(128, 128, 128));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(7), QColor(255, 255, 255));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(8), QColor(128, 128, 128));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(9), QColor(255, 255, 255));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(10), QColor(255, 255, 255));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(11), QColor(0, 0, 0));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(12), QColor(49, 106, 197));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(13), QColor(255, 255, 255));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(14), QColor(0, 0, 255));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(15), QColor(255, 0, 255));
    palette.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(16), QColor(245, 244, 235));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(0), QColor(128, 128, 128));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(1), QColor(236, 233, 216));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(2), QColor(255, 255, 255));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(3), QColor(255, 255, 250));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(4), QColor(118, 116, 108));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(5), QColor(157, 155, 144));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(6), QColor(128, 128, 128));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(7), QColor(255, 255, 255));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(8), QColor(128, 128, 128));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(9), QColor(255, 255, 255));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(10), QColor(255, 255, 255));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(11), QColor(0, 0, 0));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(12), QColor(49, 106, 197));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(13), QColor(255, 255, 255));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(14), QColor(0, 0, 255));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(15), QColor(255, 0, 255));
    palette.setColor(QPalette::Disabled, static_cast<QPalette::ColorRole>(16), QColor(245, 244, 235));
    frame->setPalette(palette);
    frame->setFrameShape(QFrame::NoFrame);
    frame->setFrameShadow(QFrame::Plain);
    label_2 = new QLabel(frame);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    label_2->setGeometry(QRect(10, 10, 32, 32));
    label_2->setPixmap(QPixmap(QString::fromUtf8("..\\icon.xpm")));
    line = new QFrame(frame);
    line->setObjectName(QString::fromUtf8("line"));
    line->setGeometry(QRect(0, 50, 377, 2));
    line->setFrameShape(QFrame::HLine);
    label_3 = new QLabel(frame);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    label_3->setGeometry(QRect(53, 15, 203, 22));
    QFont font;
    font.setFamily(QString::fromUtf8("Arial"));
    font.setPointSize(14);
    font.setBold(true);
    font.setItalic(false);
    font.setUnderline(false);
    font.setWeight(75);
    font.setStrikeOut(false);
    label_3->setFont(font);
    label = new QLabel(UpdaterExtract);
    label->setObjectName(QString::fromUtf8("label"));
    label->setGeometry(QRect(10, 60, 359, 16));
    label->setTextFormat(Qt::RichText);
    label->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    progressBar = new QProgressBar(UpdaterExtract);
    progressBar->setObjectName(QString::fromUtf8("progressBar"));
    progressBar->setGeometry(QRect(10, 80, 359, 22));
    progressBar->setValue(24);
    progressBar->setTextVisible(false);
    cancelButton = new QPushButton(UpdaterExtract);
    cancelButton->setObjectName(QString::fromUtf8("cancelButton"));
    cancelButton->setGeometry(QRect(10, 110, 81, 23));
    cancelButton->setFlat(false);
    retranslateUi(UpdaterExtract);
    QObject::connect(cancelButton, SIGNAL(clicked()), UpdaterExtract, SLOT(reject()));

    QMetaObject::connectSlotsByName(UpdaterExtract);
    } // setupUi

    void retranslateUi(QDialog *UpdaterExtract)
    {
    UpdaterExtract->setWindowTitle(QApplication::translate("UpdaterExtract", "Extracting Client Updates", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("UpdaterExtract", "", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("UpdaterExtract", "Extracting Updates", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("UpdaterExtract", "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\" white-space: pre-wrap; font-family:MS Shell Dlg; font-weight:400; font-style:normal; text-decoration:none;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Extracting ...</p></body></html>", 0, QApplication::UnicodeUTF8));
    cancelButton->setText(QApplication::translate("UpdaterExtract", "&Cancel", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(UpdaterExtract);
    } // retranslateUi

};

namespace Ui {
    class UpdaterExtract: public Ui_UpdaterExtract {};
} // namespace Ui

#endif // UI_EXTRACT_H
