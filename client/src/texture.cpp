
#include "texture.h"
#include "surface.h"
#include "utilities.h"
#include <qgl.h>

cTexture::cTexture() {
	id_ = 0;
	glGenTextures(1, &id_); // Create texture
	width_ = 0;
	height_ = 0;
	refcount = 1;
	hitTestArray = 0;
	identifier_ = 0;
	cache_ = 0;
}

cTexture::cTexture(cSurface *surface, bool hittest) {
	id_ = 0;
	glGenTextures(1, &id_); // Create texture
	width_ = 0;
	height_ = 0;
	hitTestArray = 0;
	refcount = 1;
	setData(surface, hittest);
	identifier_ = 0;
	cache_ = 0;
}

cTexture::~cTexture() {
	free();
	if (id_) {
		glDeleteTextures(1, &id_);
	}
	if (cache_) {
		cache_->unregisterTexture(this);
	}
	if (identifier_) {
		delete identifier_;
	}
}

void cTexture::free() {
	if (hitTestArray) {
		delete [] hitTestArray;
		hitTestArray = 0;
	}
}

void cTexture::generateHitMap(cSurface *surface) {
	unsigned int byteCount = (width_ * height_ + 31) / 32 + 1;
	hitTestArray = new unsigned int [byteCount + 1];
	hitTestArray[0] = 0; // Initialize first element
	unsigned int offset = 0;
	unsigned int bit = 0;

	for (int x = 0; x < surface->width(); ++x) {
		for (int y = 0; y < surface->height(); ++y) {
			unsigned int pixel = surface->getPixel(x, y);
		
			if (pixel != 0) {
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
}

void cTexture::setData(cSurface *surface, bool hittest) {
	free(); // Free the old texture

	if (!surface) {
		return;
	}

	// Make several assumptions
	if (surface->width() < 64 || surface->height() < 64) {
		throw Exception(tr("Trying to create a texture with insufficient dimensions: %1x%2.").arg(surface->width()).arg(surface->height()));
	}

	width_ = surface->width();
	realWidth_ = surface->realWidth();
	height_ = surface->height();
	realHeight_ = surface->realHeight();

	// Generate a hit test map
	if (hittest) {
		generateHitMap(surface);
	}

	// this overrides the currently bound texture
	bind();
	
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// Now upload the texture
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->bits());
}

void cTexture::bind() {
	glBindTexture(GL_TEXTURE_2D, id_);
}

void cTexture::draw(int x, int y, float alpha) {
	if (id_ != 0) {
		bind();
		glBegin(GL_QUADS);
	
		int right = x + width();
		int bottom = y + height();
	
		glColor4f(1.0f, 1.0f, 1.0f, alpha); // White. No Alpha.
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

void cTexture::setIdentifier(void *data) {
	if (identifier_ && identifier_ != data) {
		delete identifier_;
	}
	identifier_ = data;
}

void *cTexture::allocateIdentifier(uint size) {
	return operator new(size);
}
