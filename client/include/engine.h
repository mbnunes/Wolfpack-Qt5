
#if !defined(__ENGINE_H__)
#define __ENGINE_H__

#include "SDL.h"
#include <qstring.h>

#include "exceptions.h"
#include "utilities.h"
#include "SDL_opengl.h"

// This is the only place in the code where these headers should be inserted
class cEngine {
private:
	unsigned int height_;
	unsigned int width_;
	bool windowed_;
	bool lockSize_; // Lock the window size
	unsigned char *pixels; // Buffer for framebuffer effects
	
	SDL_Surface *screen; // Screen Surface
	unsigned int screenFlags; // The flags the screen was created with

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
	
	// Compatible surface creation methods
	// If texture is true, width/height are automatically modified to match the GL requirements
	// if height/width are too big, an exception is thrown
	SDL_Surface *createSurface(int width, int height, bool colorkey, bool alpha, bool texture = false);
};

extern cEngine *Engine;

#endif
