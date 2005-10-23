
#if !defined(__UPDATER_H__)
#define __UPDATER_H__

#include <QDialog>
#include <QBuffer>
#include <QDir>
#include <QHttp>
#include <QString>
#include <QDateTime>
#include <QVector>
#include "ui_question.h"
#include "ui_extract.h"
#include "ui_download.h"

#include "versioninfo.h"

/*
	This dialogs asks the user if he wants to install an update.
*/
class cQuestionDialog : public QDialog, private Ui::UpdaterQuestion {
    Q_OBJECT
public:
    cQuestionDialog(QDateTime lastModification, uint fileSize, QWidget *parent = 0);
};

/*
	This dialog shows the extraction progress.
*/
class cExtractionDialog : public QDialog, private Ui::UpdaterExtract {
Q_OBJECT
public:
	cExtractionDialog(QWidget *parent = 0);
	void setCurrent(uint progress);
	void setTotal(uint progress);
	void setText(QString text);
};

/*
	Download dialog.
*/
class cDownloadDialog : public QDialog, private Ui::UpdaterDownload {
Q_OBJECT
protected:
	QDir destination;
	QStringList urls;
	QHttp *http;
	QBuffer *buffer;
	QByteArray currentData;
	typedef QMap<int, cModuleVersion> Requests;
	Requests requests;
	uint currentProgressSlice;
	uint currentFile;
	uint currentProgressStart;
	QString currentName;
	bool sucessful_;
public:
	cDownloadDialog(QWidget *parent = 0);
	void startDownload(const Modules &downloadModules);
	void rollback();
	void setDestination(QDir destination);
	bool sucessful() const;
public slots:
	void dataReadProgress(int done, int total);
	void done(bool error);
	void requestFinished(int id, bool error);
	void requestStarted(int id);
};

inline bool cDownloadDialog::sucessful() const {
	return sucessful_;
}

inline void cDownloadDialog::setDestination(QDir destination) {
	this->destination = destination;
}

/*
	This is the updater check.
*/
class cUpdater {
public:
	static bool checkForUpdates();
	static cVersionInfo *getRemoteVersion();
	static uint getLocalVersion();
	static bool extractData(QString archive, QDir destination);
};

#endif
