
#if !defined(__UPDATER_H__)
#define __UPDATER_H__

#include <QDialog>
#include <QDir>
#include <QString>
#include "ui_question.h"
#include "ui_extract.h"

/*
	This dialogs asks the user if he wants to install an update.
*/
class cQuestionDialog : public QDialog, private Ui::UpdaterQuestion {
    Q_OBJECT
public:
    cQuestionDialog(uint localBuild, uint remoteBuild, uint fileSize, QWidget *parent = 0);
};

/*
	This dialog shows the extraction progress.
*/
class cExtractionDialog : public QDialog, private Ui::UpdaterExtract {
public:
	cExtractionDialog(QWidget *parent = 0);
	void setCurrent(uint progress);
	void setTotal(uint progress);
	void setText(QString text);
};

/*
	This is the updater check.
*/
class cUpdater {
public:
	static bool checkForUpdates();
	static uint getRemoteVersion();
	static uint getLocalVersion();
	static bool extractData(QString archive, QDir destination);
};

#endif
