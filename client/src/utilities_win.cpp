
#include <windows.h>

#include "utilities.h"

namespace Utilities {
	void launchBrowser(const QCString &url) {
		ShellExecuteA(0, "open", url.data(), "", "", SW_NORMAL);
	}
};
