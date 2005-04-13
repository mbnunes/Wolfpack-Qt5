
#include "utilities.h"
#include "config.h"
#include "uoclient.h"
#include <math.h>
#include <qmessagebox.h>

namespace Utilities {
	QString getUoFilename(const QString &filename) {
		QString basePath = Config->uoPath();

		// Translate Filename
		basePath.append(Config->getString("Filenames", filename.lower(), filename.lower()));

		return basePath;
	}

	void messageBox(QString message, QString caption, bool error) {
		//MessageBox(0, message.latin1(), caption.latin1(), MB_OK | (error ? MB_ICONERROR : MB_ICONINFORMATION));
		QMessageBox box(caption, message, ( error ? QMessageBox::Critical : QMessageBox::Information ), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		box.setModal(true);
		box.show();
	}
};

MTRand *Random = 0; // global rnd object
