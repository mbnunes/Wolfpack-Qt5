
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

		while (box.isVisible()) {
			App->processEvents(50);
			SDL_Delay(20);
		}
	}
};

void SurfacePainter32::draw(int xdest, int ydest, SDL_Surface *surface) {
	int maxy = QMIN(height, ydest + surface->h);
	int maxx = QMIN(width, xdest + surface->w);
	int sourcey = 0;

	if (SDL_MUSTLOCK(surface)) {
		SDL_LockSurface(surface);
	}

	// Copy the area of pixels over
	for (int y = ydest; y < maxy; ++y) {
		// Start of the row in both images
		unsigned int *in = (unsigned int*)((unsigned char*)surface->pixels + sourcey * surface->pitch);
		unsigned int *out = (unsigned int*)(pixels + y * pitch + (xdest << 2));
		for (int x = xdest; x < maxx; ++x) {
			*(out++) = *(in++);
		}
		++sourcey;
	}

	if (SDL_MUSTLOCK(surface)) {
		SDL_UnlockSurface(surface);
	}
}

MTRand *Random = 0; // global rnd object
