
#include "texture.h"
#include "engine.h"

cTexture::cTexture() {
	id_ = 0;
	glGenTextures(1, &id_); // Create texture
	width_ = 0;
	height_ = 0;
	refcount = 1;
	hitTestArray = 0;
}

cTexture::cTexture(SDL_Surface *surface, bool hittest) {
	id_ = 0;
	glGenTextures(1, &id_); // Create texture
	width_ = 0;
	height_ = 0;
	hitTestArray = 0;
	refcount = 1;
	setData(surface, hittest);
}

cTexture::~cTexture() {
	free();
	if (id_) {
		glDeleteTextures(1, &id_);
	}
}

void cTexture::free() {
	if (hitTestArray) {
		delete [] hitTestArray;
		hitTestArray = 0;
	}
}

void cTexture::generateHitMap(SDL_Surface *surface) {
	unsigned int byteCount = (width_ * height_ + 31) / 32 + 1;
	hitTestArray = new unsigned int [byteCount + 1];
	hitTestArray[0] = 0; // Initialize first element
	unsigned int offset = 0;
	unsigned int bit = 0;

	SurfacePainter32 painter(surface);
	painter.lock();
	for (int x = 0; x < surface->w; ++x) {
		for (int y = 0; y < surface->h; ++y) {
			unsigned int pixel = painter.getPixel(x, y);
			unsigned char alpha = painter.alpha(pixel);
			
			if (alpha != 0) {
				hitTestArray[offset] |= 1 << bit;
			} else {
				hitTestArray[offset] &= ~(1 << bit);
			}

			// Go to the next bit
			if (++bit == 32) {
				bit = 0;
				++offset;
			}
		}
	}
	painter.unlock();
}

void cTexture::setData(SDL_Surface *surface, bool hittest) {
	free(); // Free the old texture

	if (!surface) {
		return;
	}

	// Make several assumptions
	if (surface->w < 64 || surface->h < 64) {
		throw Exception(tr("Trying to create a texture with insufficient dimensions: %1x%2.").arg(surface->w).arg(surface->h));
	}

	SDL_PixelFormat *format = surface->format;
	if (format->BytesPerPixel != 4) {
		throw Exception(tr("You have to specify a RGBA 32-bit per pixel surface to create textures."));
	}

	width_ = surface->w;
	realWidth_ = surface->w;
	height_ = surface->h;
	realHeight_ = surface->h;

	// Generate a hit test map :o
	if (hittest) {
		generateHitMap(surface);
	}

	// this overrides the currently bound texture
	bind();
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// Now upload the texture
	glTexImage2D(GL_TEXTURE_2D, 0, 4, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
}

void cTexture::bind() {
	glBindTexture(GL_TEXTURE_2D, id_);
}

void cTexture::draw(int x, int y) {
	if (id_ != 0) {
		bind();
		glBegin(GL_QUADS);
	
		int right = x + width();
		int bottom = y + height();
	
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // White. No Alpha.
		glTexCoord2f(0, 0); glVertex2f(x, y); // Upper left corner
		glTexCoord2f(1, 0); glVertex2f(right, y); // Upper Right Corner
		glTexCoord2f(1, 1); glVertex2f(right, bottom); // Lower Right Corner
		glTexCoord2f(0, 1); glVertex2f(x, bottom); // Lower Right Corner
	
		glEnd();
	}
}

bool cTexture::hitTest(int x, int y) {
	// Check the bounding box first
	if (x >= 0 && x < (int)realWidth_ && y >= 0 && y < (int)realHeight_) {
		// If we have a hit testing box check that
		if (hitTestArray) {
			unsigned int offset = (x * height_ + y); // Pixel index
			unsigned int mask = 1 << (offset % 32);
			offset /= 32; // Byte offset
			return (hitTestArray[offset] & mask) != 0;
		} else {
			return true;
		}
	}

	return false;
}
