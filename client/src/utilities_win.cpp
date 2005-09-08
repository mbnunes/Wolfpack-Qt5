
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include "utilities.h"
//Added by qt3to4:
#include <QString>

namespace Utilities {
	void launchBrowser(const QString &url) {
		QString realUrl = url;
		if (!realUrl.startsWith("http://")) {
			realUrl.prepend("http://");
		}

		ShellExecuteA(0, "open", realUrl.toLatin1(), "", "", SW_NORMAL);
	}
};
