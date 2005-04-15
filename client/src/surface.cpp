
#include "surface.h"
#include <string.h>

cSurface::cSurface(int width, int height, bool texture)
{
	realWidth_ = width;
	realHeight_ = height;

	if (texture) {
		int i = 64;
		while (i < width) {
			i <<= 1;
		}
		width = i;
		i = 64;
		while (i < height) {
			i <<= 1;
		}
		height = i;
	}

	width_ = width;
	height_ = height;
	pitch = width * 4;

	// Create pixel array
	data = new unsigned char[width * height * 4];
}

cSurface::~cSurface() {
	delete [] data;
}

void cSurface::fill(unsigned int color) {
	register const unsigned int value = color; // Clear value
	register unsigned int *ptr = (unsigned int*)data;
	register unsigned int *endptr = ((unsigned int*)data) + width_ * height_;
	while (ptr != endptr) {
		(*ptr++) = value;
	}
}

void cSurface::clear() {
	memset(data, 0, width_ * height_ * 4);
	/*register const unsigned int value = 0; // Clear value
	register unsigned int *ptr = (unsigned int*)data;
	register unsigned int *endptr = ((unsigned int*)data) + width_ * height_;
	while (ptr != endptr) {
		(*ptr++) = value;
	}*/
}
