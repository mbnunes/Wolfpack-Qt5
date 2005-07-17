
#include <windows.h>

#include "utilities.h"
//Added by qt3to4:
#include <QString>

namespace Utilities {
	void launchBrowser(const QString &url) {
		ShellExecuteA(0, "open", url.toLatin1(), "", "", SW_NORMAL);
	}
};
