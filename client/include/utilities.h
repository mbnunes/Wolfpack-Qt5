
#if !defined(__UTILITIES_H__)
#define __UTILITIES_H__

#include "SDL.h"
#include <time.h>
#include <qstring.h>
#include <qintcache.h>

#include "exceptions.h"
#include "mersennetwister.h"
#include "texture.h"

// Define tr
inline QString tr(const QString &a) {
	return a;
}

// Useful mathematical definitions
const float PI = 3.14159265358979323846f;
const float PIHALF = PI / 2.0f;
const float PIFOURTH = PI / 4.0f;

// Utilities
namespace Utilities {
	void messageBox(QString message, QString caption = "Error", bool error = false);

	QString getUoFilename(const QString &filename);

	void launchBrowser(const QCString &url);

	inline unsigned int getTicks() {
		return (unsigned int)clock();
	}

	/*
		Check if two rects do have at least one intersecting pixel
	*/
	inline bool checkRectCollision(const SDL_Rect *a, const SDL_Rect *b) {
		if (a->x + a->w < b->x || a->x >= b->x + b->w) {
			return false; // No horizontal collision
		}

		if (a->y + a->h < b->y || a->y >= b->y + b->h) {
			return false; // No vertical collision
		}

		return true;
	}

	/*
		Return the direction from point a to point b.
	*/
	inline unsigned char direction(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
		unsigned int dir;
		short xdif, ydif;

		xdif = x2 - x1;
		ydif = y2 - y1;

		if ( ( xdif == 0 ) && ( ydif < 0 ) )
			dir = 0;
		else if ( ( xdif > 0 ) && ( ydif < 0 ) )
			dir = 1;
		else if ( ( xdif > 0 ) && ( ydif == 0 ) )
			dir = 2;
		else if ( ( xdif > 0 ) && ( ydif > 0 ) )
			dir = 3;
		else if ( ( xdif == 0 ) && ( ydif > 0 ) )
			dir = 4;
		else if ( ( xdif < 0 ) && ( ydif > 0 ) )
			dir = 5;
		else if ( ( xdif < 0 ) && ( ydif == 0 ) )
			dir = 6;
		else if ( ( xdif < 0 ) && ( ydif < 0 ) )
			dir = 7;
		else
			dir = 8;

		return (dir + 1) % 8;
	}

	/*
		Calculate the distance between two 2d uo coordinates
	*/
	inline unsigned int distance(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
		unsigned int uiResult = ~0U; // Initialize with *infinite*
		
		int xdiff = x1 - x2;
		int ydiff = y1 - y2;
		float rfResult = sqrt((float)(xdiff * xdiff + ydiff * ydiff));
		uiResult = (unsigned int)floor(rfResult); // truncate
		
		return uiResult;
	}
};

// This is actually a drawer that operates on a given 32bpp SDL_Surface
class SurfacePainter32 {
private:
	unsigned int pitch;
	unsigned char *pixels;
	int width, height;
	bool mustlock;
	SDL_Surface *surface;
public:
	SurfacePainter32(SDL_Surface *surface) {
		if (surface->format->BytesPerPixel != 4) {
			throw Exception(tr("Expected a surface with 32 bpp. Got a surface with %1 bpp.").arg(surface->format->BitsPerPixel));
		}

		width = surface->w;
		height = surface->h;
		pitch = surface->pitch;
		pixels = (unsigned char*)surface->pixels;
		mustlock = SDL_MUSTLOCK(surface);
		this->surface = surface;
	}

	// Create a color value
	static unsigned int color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
	static unsigned int color(unsigned short pixel, unsigned char a = 255);
	void lock();
	void unlock();

	// Draw the entire given surface to the given x,y coordinates. Must be
	// locked first.
	void draw(int xdest, int ydest, SDL_Surface *surface);

	// Set a single pixel. Must be locked first.
	void setPixel(unsigned int x, unsigned int y, unsigned int color);

	// Get a single pixel value. Must be locked first.
	unsigned int getPixel(unsigned int x, unsigned int y) const;
	unsigned char red(unsigned int color) const;
	unsigned char blue(unsigned int color) const;
	unsigned char alpha(unsigned int color) const;
	unsigned char green(unsigned int color) const;
};

inline unsigned int SurfacePainter32::color(unsigned short pixel, unsigned char a) {
	return color((pixel >> 7) & 0xF8, (pixel>> 2) & 0xF8, (pixel << 3) & 0xF8, a);
}

inline unsigned int SurfacePainter32::color(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	return ((a << 24) | (b << 16) | (g << 8) | r);
}

inline unsigned char SurfacePainter32::red(unsigned int color) const {
	return color & 0xFF;
}

inline unsigned char SurfacePainter32::blue(unsigned int color) const {
	return (color >> 16) & 0xFF;
}

inline unsigned char SurfacePainter32::alpha(unsigned int color) const {
	return (color >> 24) & 0xFF;
}

inline unsigned char SurfacePainter32::green(unsigned int color) const {
	return (color >> 8) & 0xFF;
}

inline void SurfacePainter32::lock() {
	if (mustlock) {
		SDL_LockSurface(surface);
	}
}

inline void SurfacePainter32::unlock() {
	if (mustlock) {
		SDL_UnlockSurface(surface);
	}
}

inline void SurfacePainter32::setPixel(unsigned int x, unsigned int y, unsigned int color) {
	// x << 2 == x * 4
	*(unsigned int*)(pixels + y * pitch + (x << 2)) = color;
}

inline unsigned int SurfacePainter32::getPixel(unsigned int x, unsigned int y) const {
	// x << 2 == x * 4
	return *(unsigned int*)(pixels + y * pitch + (x << 2));
}

// This class encapsulates a sharedsurface
class SharedSurface {
private:
	unsigned int refcount;
	SDL_Surface *data;
public:
	SharedSurface(SDL_Surface *surface) {
		data = surface;
		refcount = 1; // Starts with one reference
	}

	~SharedSurface() {
		if (data) {
			SDL_FreeSurface(data);
		}
	}

	// return the encapsulated surface
	inline SDL_Surface *surface() {
		return data;
	}

	// Decrease the reference count
	inline void decref() {
		if (--refcount == 0) {
			delete this;
		}
	}

	// Increase the reference count
	inline void incref() {
		refcount++;
	}

	// Do a hit test on the internal surface. 
	// this is a nice convenience method
	bool hitTest(int x, int y);
};

inline bool SharedSurface::hitTest(int x, int y) {
	bool result = false;

	if (data) {
		if (data && x >= 0 && y >= 0 && x < data->w && y < data->h) {
			if (SDL_MUSTLOCK(data)) {
				SDL_LockSurface(data);
			}

			// Access the pixel in question
			SDL_PixelFormat *pf = data->format;
			unsigned char *ptr = ((unsigned char*)data->pixels) + data->pitch * y + x * pf->BytesPerPixel;
			unsigned int background = SDL_MapRGB(pf, 0, 0, 0);
			switch (pf->BytesPerPixel) {
				case 4:
					if (*((unsigned int*)ptr) != background) {
						result = true;
					}
					break;
				case 2:
					if (*((unsigned short*)ptr) != background) {
						result = true;
					}
					break;
				default:
					break;
			}

			if (SDL_MUSTLOCK(data)) {
				SDL_UnlockSurface(data);
			}
		}
	}

	return result;
}

// This cache can be used to cache SharedSurface
// Please note that this cache does steal reference counts
class SurfaceCache : public QIntCache< SharedSurface > {
public:
	SurfaceCache(int a, int b) : QIntCache< SharedSurface > (a, b) {
	}

protected:
	void deleteItem(Item d) {
		((SharedSurface*)d)->decref();
	}
};

// This cache can be used to cache SharedSurface
// Please note that this cache does steal reference counts
class TextureCache : public QIntCache< cTexture > {
public:
	TextureCache(int a, int b) : QIntCache< cTexture > (a, b) {
	}

protected:
	void deleteItem(Item d) {
		((cTexture*)d)->decref();
	}
};

// Define the Random object
extern MTRand *Random;

#endif
