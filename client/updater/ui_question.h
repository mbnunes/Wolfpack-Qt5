#ifndef UI_QUESTION_H
#define UI_QUESTION_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>

class Ui_UpdaterQuestion
{
public:
    QWidget *layoutWidget;
    QHBoxLayout *hboxLayout;
    QPushButton *yesButton;
    QPushButton *noButton;
    QLabel *label_2;
    QLabel *InfoLabel;

    void setupUi(QDialog *UpdaterQuestion)
    {
    UpdaterQuestion->setObjectName(QString::fromUtf8("UpdaterQuestion"));
    UpdaterQuestion->resize(QSize(279, 162).expandedTo(UpdaterQuestion->minimumSizeHint()));
    UpdaterQuestion->setWindowIcon(QIcon(QString::fromUtf8(":/updater/resources/icon.xpm")));
    UpdaterQuestion->setModal(true);
    layoutWidget = new QWidget(UpdaterQuestion);
    layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
    layoutWidget->setGeometry(QRect(50, 120, 161, 33));
    hboxLayout = new QHBoxLayout(layoutWidget);
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    yesButton = new QPushButton(layoutWidget);
    yesButton->setObjectName(QString::fromUtf8("yesButton"));

    hboxLayout->addWidget(yesButton);

    noButton = new QPushButton(layoutWidget);
    noButton->setObjectName(QString::fromUtf8("noButton"));

    hboxLayout->addWidget(noButton);

    label_2 = new QLabel(UpdaterQuestion);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    label_2->setGeometry(QRect(10, 10, 32, 32));
    label_2->setPixmap(QPixmap(QString::fromUtf8(":/updater/resources/icon.xpm")));
    label_2->setScaledContents(false);
    InfoLabel = new QLabel(UpdaterQuestion);
    InfoLabel->setObjectName(QString::fromUtf8("InfoLabel"));
    InfoLabel->setGeometry(QRect(50, 10, 255, 101));
    InfoLabel->setFrameShape(QFrame::NoFrame);
    InfoLabel->setTextFormat(Qt::RichText);
    InfoLabel->setScaledContents(false);
    InfoLabel->setAlignment(Qt::AlignTop);
    InfoLabel->setWordWrap(true);
    retranslateUi(UpdaterQuestion);
    QObject::connect(yesButton, SIGNAL(clicked()), UpdaterQuestion, SLOT(accept()));
    QObject::connect(noButton, SIGNAL(clicked()), UpdaterQuestion, SLOT(reject()));

    QMetaObject::connectSlotsByName(UpdaterQuestion);
    } // setupUi

    void retranslateUi(QDialog *UpdaterQuestion)
    {
    yesButton->setText(QApplication::translate("UpdaterQuestion", "Yes", 0, QApplication::UnicodeUTF8));
    noButton->setText(QApplication::translate("UpdaterQuestion", "No", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("UpdaterQuestion", "", 0, QApplication::UnicodeUTF8));
    InfoLabel->setText(QApplication::translate("UpdaterQuestion", "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\" white-space: pre-wrap; font-family:MS Shell Dlg; font-weight:400; font-style:normal; text-decoration:none;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">There is a new update available</span></p><p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"></p><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Latest Modification: %1</p><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Download size: %2</p><p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"></p><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Do you want to download the new version?</p></body></html>", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(UpdaterQuestion);
    } // retranslateUi

};

namespace Ui {
    class UpdaterQuestion: public Ui_UpdaterQuestion {};
} // namespace Ui

#endif // UI_QUESTION_H
