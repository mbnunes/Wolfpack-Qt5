
#include "utilities.h"
#include <QString>
#include <sys/time.h>

namespace Utilities {
	void launchBrowser(const QString &url) {
		/*
			There is nothing we could do here yet. 
			Maybe execute some form of shell script ?
		*/
	}

	uint getTicks() {
		long ticks;
		struct timeval now;
		gettimeofday(&now, NULL);
		ticks = now.tv_sec * 1000l;
		ticks += now.tv_usec / 1000l;
		return ticks;
	}
};
