
#include "engine.h"
#include "gui/container.h"
#include "gui/itemimage.h"
#include "muls/art.h"

cItemImage::cItemImage(unsigned short id, unsigned short hue, bool partialhue, bool stacked, bool translucent, bool landtile) {
	item = 0;
	setItem(id, hue, partialhue, stacked, translucent, landtile);
}

cItemImage::~cItemImage() {
	if (item) {
		item->decref();
	}
}

void cItemImage::setItem(unsigned short id, unsigned short hue, bool partialhue, bool stacked, bool translucent, bool landtile) {
	if (!this->item || this->id != id || this->hue != hue || this->partialhue != partialhue || this->stacked != stacked || this->translucent != translucent || this->landtile != landtile) {
		if (item) {
			item->decref();
		}
	
		this->id = id;
		this->hue = hue;
		this->partialhue = partialhue;
		this->translucent = translucent;
		this->stacked = stacked;
		this->landtile = landtile;

		if (landtile) {
			item = Art->readLand(id, hue, partialhue, stacked, translucent);
		} else {
			item = Art->readItem(id, hue, partialhue, stacked, translucent);
		}
		item->incref();

		int oldwidth = width_;
		int oldheight = height_;
		
		SDL_Surface *surface = item->surface();
		if (surface) {
			width_ = surface->w;
			height_ = surface->h;
		}
		onChangeBounds(x_, y_, oldwidth, oldheight);
	}
}

cControl *cItemImage::getControl(int x, int y) {
	cControl *result = 0;

	SDL_Surface *item = this->item ? this->item->surface() : 0;

	if (item && x >= 0 && y >= 0 && x < item->w && y < item->h) {
		if (SDL_MUSTLOCK(item)) {
			SDL_LockSurface(item);
		}

		// Access the pixel in question
		SDL_PixelFormat *pf = item->format;
		unsigned char *ptr = ((unsigned char*)item->pixels) + item->pitch * y + x * pf->BytesPerPixel;
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

		if (SDL_MUSTLOCK(item)) {
			SDL_UnlockSurface(item);
		}
	}
	return result;
}
