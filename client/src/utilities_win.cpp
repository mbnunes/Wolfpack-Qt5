
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include <QDir.h>

#include "mainwindow.h"
#include "utilities.h"
#include "config.h"

#include <QString>

namespace Utilities {
	// This is case insensitive
	QString getUoFilename(const QString &filename) {
		QString basePath = Config->uoPath();

		// Translate Filename
		basePath.append(Config->getString("Filenames", filename.toLower(), filename.toLower()));

		return basePath;
	}

	void launchBrowser(const QString &url) {
		QString realUrl = url;
		if (!realUrl.startsWith("http://")) {
			realUrl.prepend("http://");
		}

        QT_WA({
            ShellExecute(MainWindow->winId(), 0, (TCHAR*)realUrl.utf16(), 0, 0, SW_SHOWNORMAL);
        } , {
            ShellExecuteA(MainWindow->winId(), 0, realUrl.toLocal8Bit(), 0, 0, SW_SHOWNORMAL);
        });		
	}

	uint getTicks() {
		return GetTickCount();
	}
};
