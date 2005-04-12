
#include <qglobal.h>

#include "gui/image.h"
#include "muls/gumpart.h"

cImage::cImage() {
	surface = 0;
	//autoSize_ = autoSize;
}

cImage::~cImage() {
	if (surface) {
		surface->decref();
	}
}

cControl *cImage::getControl(int x, int y) {
	cControl *result = 0;

	if (surface) {
		SDL_Surface *surface = this->surface->surface();

		if (surface && x >= 0 && y >= 0 && x < surface->w && y < surface->h) {
			if (SDL_MUSTLOCK(surface)) {
				SDL_LockSurface(surface);
			}

			// Access the pixel in question
			SDL_PixelFormat *pf = surface->format;
			unsigned char *ptr = ((unsigned char*)surface->pixels) + surface->pitch * y + x * pf->BytesPerPixel;
			unsigned int background = SDL_MapRGB(pf, 0, 0, 0);
			switch (pf->BytesPerPixel) {
				case 4:
					if (*((unsigned int*)ptr) != background) {
						result = this;
					}
					break;
				case 2:
					if (*((unsigned short*)ptr) != background) {
						result = this;
					}
					break;
				default:
					break;
			}

			if (SDL_MUSTLOCK(surface)) {
				SDL_UnlockSurface(surface);
			}
		}
	}

	return result;
}
