
#if !defined(__ENGINE_H__)
#define __ENGINE_H__

#include "SDL.h"
#include <qstring.h>

#include "exceptions.h"
#include "utilities.h"
#include "paintable.h"
#include "SDL_opengl.h"

// This is the only place in the code where these headers should be inserted
class cEngine : public IPaintable {
private:
	unsigned int height_;
	unsigned int width_;
	bool windowed_;
	bool lockSize_; // Lock the window size
	unsigned char *pixels; // Buffer for framebuffer effects
	
	SDL_Surface *screen; // Screen Surface

	bool changed; // The backbuffer surface has changed and needs a flip
	bool screenshot_; // Screenshot is pending
	QString screenshotFilename;

	void createScreenshot(const QString &filename);

public:
	cEngine();
	~cEngine();

	inline void flagChanged() {
		changed = true;
	}

	void poll();
	void resize(unsigned int width, unsigned int height);

	void load();
	void reload();
	void unload();

	// Do a screenshot the next time the buffer is flipped
	void screenshot(const QString &filename = QString::null);

	inline bool lockSize() const { return lockSize_; }
	void setLockSize(bool data) { lockSize_ = data; }

	// Getters
	bool windowed() { return windowed_; }
	unsigned int height() { return height_; }
	unsigned int width() { return width_; }
	
	// Get the screen pixel format
	inline SDL_PixelFormat *pixelFormat() {
		return screen->format;
	}

	// Get the surface of the engine. Do NOT draw to this.
	inline SDL_Surface *getSurface() {
		return screen;
	}

	// Compatible surface creation methods
	// If texture is true, width/height are automatically modified to match the GL requirements
	// if height/width are too big, an exception is thrown
	inline SDL_Surface *createSurface(int width, int height, bool colorkey, bool alpha, bool texture = false) {
		// Modify w/h
		if (texture) {
			int i = 64;
			while (i < width) {
				i *= 2;
			}
			width = i;
			i = 64;
			while (i < height) {
				i *= 2;
			}
			height = i;
		}

		unsigned int flags = 0;

		if (colorkey || alpha) {
			flags |= SDL_HWSURFACE;
		} else {
			flags |= SDL_SWSURFACE;
		}

		if (colorkey) {
			flags |= SDL_SRCCOLORKEY;
		}

		if (alpha) {
			flags |= SDL_SRCALPHA;
		}

		unsigned int depth = 32;
		unsigned int rmask = 0x000000ff;
		unsigned int gmask = 0x0000ff00;
		unsigned int bmask = 0x00ff0000;
		unsigned int amask = 0xff000000;

		if (!alpha) {
			amask = 0;
		}

		return SDL_CreateRGBSurface(flags, width, height, depth, rmask, gmask, bmask, amask);
	}

	// Draw Pixel
	void drawPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);

	// Transfer a given surface to the screen
	void drawSurface(int x, int y, SDL_Surface *surface, SDL_Rect *srcrect = 0);

	// Invert a pixel on the surface
	void invertPixel(int x, int y);

	void getPixel(int x, int y, unsigned char &r, unsigned char &g, unsigned char &b);
};

extern cEngine *Engine;

#endif
