
#include "updater.h"
#include "log.h"
#include "utilities.h"
#include "../version.h"

#include <QMessageBox>
#include <QUrl>

cDownloadDialog::cDownloadDialog(QWidget *parent) {
	setupUi(this);

	buffer = new QBuffer(&currentData, this);
	http = new QHttp(this);
	currentFile = 0;

	connect(http, SIGNAL(dataReadProgress(int, int)), SLOT(dataReadProgress(int, int)));
	connect(http, SIGNAL(done(bool)), SLOT(done(bool)));
	connect(http, SIGNAL(requestFinished(int, bool)), SLOT(requestFinished(int, bool)));
	connect(http, SIGNAL(requestStarted(int)), SLOT(requestStarted(int)));
}

void cDownloadDialog::startDownload(const Modules &downloadModules) {
	requests.clear();
	http->abort();
	qApp->processEvents();
	currentFile = 0;
	
	uint totalSize = 0;

	foreach (const cModuleVersion &module, downloadModules) {
		QUrl url(module.url());
		QHttpRequestHeader header("GET", url.path());		
		header.setValue("Host", url.host());
		header.setValue("User-Agent", QString(UOFILEVERSION_STR));
		
		http->setHost(url.host(), (url.port() != -1) ? url.port() : 80);
		int id = http->request(header, QByteArray(), buffer);

		// Save the id along with the module information
		requests.insert(id, module);

		totalSize += module.size();
	}

	progressBar->setRange(0, totalSize);
	progressBar->setValue(0);
}

void cDownloadDialog::dataReadProgress(int done, int total) {
	progressBar->setValue((int)(currentProgressStart + (done / (float)total) * currentProgressSlice));
}

void cDownloadDialog::done(bool error) {
	if (error) {
        reject(); // An error occured
	} else {
		accept();
	}
}

void cDownloadDialog::rollback() {
	foreach (const cModuleVersion &module, requests.values()) {
		destination.remove(module.name());
	}
}

void cDownloadDialog::requestFinished(int id, bool error) {
	Requests::iterator it = requests.find(id);
	if (it != requests.end()) {
		if (error) {
			QMessageBox::critical(this, tr("Download Error"), tr("An error occured while downloading %1: %2.\n").arg(currentName).arg(http->errorString()));
			goto abort;
		} else {
			// Check the response
			QHttpResponseHeader response = http->lastResponse();

			if (response.statusCode() == 404) {
				QMessageBox::critical(this, tr("Download Error"), tr("The remote version file was not found: %1.\n").arg(it->url()), QMessageBox::Ok, QMessageBox::NoButton);
				goto abort;
			} else if (response.statusCode() == 500) {
				QMessageBox::critical(this, tr("Download Error"), tr("The server encountered an error while fetching the version file: %1.\n").arg(it->url()), QMessageBox::Ok, QMessageBox::NoButton);
				goto abort;
			} else if (response.statusCode() == 403) {
				QMessageBox::critical(this, tr("Download Error"), tr("Permission was denied to download the version information: %1.\n").arg(it->url()), QMessageBox::Ok, QMessageBox::NoButton);
				goto abort;
			} else if (response.statusCode() != 200) {
				QMessageBox::critical(this, tr("Download Error"), tr("There was an error while fetching the version file: %1.\n").arg(response.reasonPhrase()), QMessageBox::Ok, QMessageBox::NoButton);
				goto abort;
			}

            // Status seems to be ok
			QString filename = destination.absoluteFilePath(it->name());
			QFile output(filename);
			if (!output.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
				QMessageBox::critical(this, tr("Download Error"), tr("Unable to open file for writing: %1.\n").arg(output.fileName()), QMessageBox::Ok, QMessageBox::NoButton);
				goto abort;
			}

			output.write(currentData);
			output.close();
			currentData.clear();
		}
	}

	return;

	// Code above jumps here if it wants to abort the request
abort:
	http->clearPendingRequests();
	http->abort();
	rollback();
	reject();
}

void cDownloadDialog::requestStarted(int id) {
	currentData.clear();
	Requests::iterator it = requests.find(id);
	if (it != requests.end()) {
		currentFile++;

		currentName = it->name();
		currentProgressSlice = it->size();
		currentProgressStart = progressBar->value();
		label->setText(tr("Downloading %1... (%2 of %3)").arg(currentName).arg(currentFile).arg(requests.count()));
	}
}

