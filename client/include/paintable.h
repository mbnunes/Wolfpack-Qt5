
#if !defined(__PAINTABLE_H__)
#define __PAINTABLE_H__

#include "SDL.h"

// This is an interface
class IPaintable {
public:
	// Draw a single pixel
	virtual void drawPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) = 0;
	virtual void getPixel(int x, int y, unsigned char &r, unsigned char &g, unsigned char &b) = 0;
	virtual void drawSurface(int x, int y, SDL_Surface *surface, SDL_Rect *srcrect = 0) = 0;
	virtual void invertPixel(int x, int y) = 0;
	
	virtual void drawSelection(const SDL_Rect &rect);
};

#endif
