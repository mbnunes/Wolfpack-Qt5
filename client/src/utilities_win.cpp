
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include "mainwindow.h"
#include "utilities.h"

#include <QString>

namespace Utilities {
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
