
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

void cImage::draw(IPaintable *target, const SDL_Rect *clipping) {
	if (surface) {
		SDL_Surface *surface = this->surface->surface();
		if (clipping) {
			int xdiff = clipping->x - x_;
			int ydiff = clipping->y - y_;

			SDL_Rect srcrect;
			srcrect.x = QMAX(0, xdiff);
			srcrect.y = QMAX(0, ydiff);
			srcrect.w = QMIN(clipping->w, QMIN(surface->w - srcrect.x, clipping->w + xdiff));
			srcrect.h = QMIN(clipping->h, QMIN(surface->h - srcrect.y, clipping->h + ydiff));

			target->drawSurface(x_ + QMAX(0, xdiff), y_ + QMAX(0, ydiff), surface, &srcrect);
		} else {
			target->drawSurface(x_, y_, surface);
		}
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

cCustomImage::cCustomImage(SharedSurface *surface) {
	this->surface = surface;
	if (surface) {
		surface->incref();
		SDL_Surface *sdlSurface = surface->surface();
		setSize(sdlSurface->w, sdlSurface->h);
	}
}

void cCustomImage::update() {
}

void cCustomImage::onMouseDown(int, int, unsigned char, bool) {
	Utilities::messageBox(message, "Label Information", false);
}
