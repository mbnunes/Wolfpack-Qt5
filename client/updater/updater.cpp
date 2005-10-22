
#include "updater.h"
#include "log.h"
#include "utilities.h"
#include "../version.h"

#include <QDomDocument>
#include <QMessageBox>
#include <QHttp>
#include <QUrl>
#include <QByteArray>
#include <QBuffer>
#include <QProgressBar>
#include <QDateTime>

// LZMA Includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

cQuestionDialog::cQuestionDialog(uint localBuild, uint remoteBuild, uint fileSize, QWidget *parent) : QDialog(parent) {
	setupUi(this);

	InfoLabel->setText(InfoLabel->text().arg(localBuild).arg(remoteBuild).arg(fileSize));
}

uint cUpdater::getLocalVersion() {
	ushort version[4] = { UOFILEVERSION };
	return version[3];
}

cVersionInfo *cUpdater::getRemoteVersion() {
	QUrl url("http://www.hartte.de/uoclient/version.xml");
	QHttp http(url.host(), (url.port() != -1) ? url.port() : 80);

	Log->print(LOG_MESSAGE, tr("Retrieving version information from %1.\n").arg(url.toString()));

	// Setup the result buffer
	QByteArray result;
	QBuffer buffer(&result);

	// Send the request
	QHttpRequestHeader header("GET", url.path());
	header.setValue("Host", url.host());
	header.setValue("User-Agent", QString(UOFILEVERSION_STR));
	http.request(header, QByteArray(), &buffer);

	// Process network events while we didn't grab the file
	qApp->processEvents();
	while (http.currentRequest().isValid()) {
		qApp->processEvents();
	}

	if (http.error() != QHttp::NoError) {
		Log->print(LOG_ERROR, tr("An error occured while fetching the remote version: %1.\n").arg(http.errorString()));
		return 0;
	}
	
	// Get the server response
	QHttpResponseHeader response = http.lastResponse();
    
	if (response.statusCode() == 404) {
		Log->print(LOG_ERROR, tr("The remote version file was not found: %1.\n").arg(url.toString()));
	} else if (response.statusCode() == 500) {
		Log->print(LOG_ERROR, tr("The server encountered an error while fetching the version file: %1.\n").arg(url.toString()));
	} else if (response.statusCode() == 403) {
		Log->print(LOG_ERROR, tr("Permission was denied to download the version information: %1.\n").arg(url.toString()));
	} else if (response.statusCode() != 200) {
		Log->print(LOG_ERROR, tr("There was an error while fetching the version file: %1.\n").arg(response.reasonPhrase()));
	}

	int errorLine, errorColumn;
	QString errorMsg;

	// The result should be here now
	QDomDocument document;
	if (!document.setContent(result, false, &errorMsg, &errorLine, &errorColumn)) {
		Log->print(LOG_ERROR, tr("An error occured while processing the version information: %1 (Line %2, Column %3).\n").arg(errorMsg).arg(errorLine).arg(errorColumn));
		return 0;
	}

	return cVersionInfo::fromXml(document.documentElement());
}

bool cUpdater::checkForUpdates() {
	// Create the Mutex responsible for making this program unique
	CreateMutex(0, TRUE, "UOCLIENT_MUTEX");

	return false;

	cVersionInfo *version = getRemoteVersion();
	
	// If no version could be retrieved, warn the user but continue anyway
	if (!version) {
		QMessageBox::warning(0, tr("Warning"), tr("No version information could be received from the server. Automatic updates are disabled."), QMessageBox::Ok, QMessageBox::NoButton);
		return false;
	}

	QDir destination = QDir::current();

	// Before doing anything else, collect the list of modules we still need
	Modules modules = version->modules();
    delete version; // No longer needed
	
	uint totalDownloadSize = 0;
	Modules downloadModules;

	foreach (const cModuleVersion &module, modules) {
		Log->print(LOG_DEBUG, tr("Checking module %1.\n").arg(module.name()));

		// Check every file in the module if its out of date
		foreach (const cComponentVersion &component, module.components()) {
			QFileInfo fileinfo(destination.absoluteFilePath(component.path()));
			QDateTime lastModification = fileinfo.lastModified();
			if (lastModification < component.lastModified()) {
				Log->print(LOG_MESSAGE, tr("File %1 needs update: %2 is less than %3.\n").arg(component.path()).arg(lastModification.toString()).arg(component.lastModified().toString()));
				downloadModules.append(module);
				break;
			}
		}
	}

	// No updates available
	if (downloadModules.isEmpty()) {
		Log->print(LOG_NOTICE, tr("No updates available.\n"));
		return false;
	}

	if (!destination.exists("downloads")) {
		if (!destination.mkdir("downloads")) {
			QMessageBox::critical(0, tr("Download Error"), tr("Unable to create download directory at %1.").arg(destination.absoluteFilePath("downloads")), QMessageBox::Ok, QMessageBox::NoButton);
			return false;
		}
	}
	QDir downloads = destination;

	// Change to download directory
	if (downloads.cd("downloads")) {
		cDownloadDialog *dialog = new cDownloadDialog;
		dialog->setDestination(downloads);
		dialog->show();
		dialog->startDownload(downloadModules);		

		qApp->setQuitOnLastWindowClosed(false);
		while (dialog->isVisible()) {
			QApplication::instance()->processEvents();
		}
		qApp->setQuitOnLastWindowClosed(true);
	} else {
		QMessageBox::critical(0, tr("Download Error"), tr("Unable to change to download directory at %1.").arg(destination.absoluteFilePath("downloads")), QMessageBox::Ok, QMessageBox::NoButton);
		return false;
	}

	if (!destination.exists("update")) {
		destination.mkdir("update");
	}
	if (!destination.exists("update")) {
		destination.mkdir("update");
	}
	if (destination.cd("update")) {
		// For every downloaded module, extract its contents to update/
		foreach (const cModuleVersion &module, downloadModules) {
			extractData(downloads.absoluteFilePath(module.name()), destination);
		}
	}
	return true;

	// If we already have the latest version, skip the question
	//if (localBuild >= remoteBuild) {
		return false; // No update available
	//}

	cQuestionDialog *dialog = new cQuestionDialog(0, 0, 0);
	dialog->show();

	qApp->setQuitOnLastWindowClosed(false);
	while (dialog->isVisible()) {
		QApplication::instance()->processEvents();
	}

	int result = dialog->result();
	delete dialog;

	// If the user chose not to update, return without taking any action
	if (result == QDialog::Rejected) {
		return false;
	}

	qApp->setQuitOnLastWindowClosed(true);
	return true;
}
