
#include "utilities.h"
#include "config.h"
#include <QString>
#include <QDir>
#include <sys/time.h>

namespace Utilities {
	QString getUoFilename(const QString &filename) {
		QString basePath = Config->uoPath();

		// Translate Filename
		basePath.append(Config->getString("Filenames", filename.toLower(), filename.toLower()));

		// Check if the file exists and if it doesn't, search the
		// directory for similar names.
		QDir dir = QFileInfo(basePath).absoluteDir();
		QString fileName = QFileInfo(basePath).fileName().toLower();

		// search the directory
		if (!dir.exists(fileName)) {
			// This is inherently not case sensitive
			QStringList files = dir.entryList(QStringList(fileName), QDir::Files|QDir::Readable);

			// Check every entry
			foreach (QString file, files) {
				if (file.toLower() == fileName) {
					basePath = dir.absoluteFilePath(file);
					return basePath;
				}
			}
		}

		return basePath;
	}

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
