
#if !defined(__TEXTURE_H__)
#define __TEXTURE_H__

#include "exceptions.h"

class cSurface;

/*
	This class represents an OpenGL texture.
*/
class cTexture {
protected:
	unsigned int id_; // Private texture id
	unsigned int refcount; // Reference count
	unsigned int width_, height_;
	unsigned int realWidth_, realHeight_;
	unsigned int *hitTestArray; // Cache for hit testing
	void generateHitMap(cSurface *surface);

public:
	cTexture();
	cTexture(cSurface *surface, bool hittest = true);
	~cTexture();

	// Free the currently assigned texture
	void free();

	// Set the content of this texture
	// Please note that an exception is thrown if t
	void setData(cSurface *surface, bool hittest = true);

	// Bind this texture to the rendering context
	void bind();

	// Reference count increment/decrement
	void incref();
	void decref();

	// Access the dimensions of the texture
	unsigned int width() const;
	unsigned int height() const;
	unsigned int realWidth() const;
	unsigned int realHeight() const;
    void setRealWidth(unsigned int data);
	void setRealHeight(unsigned int data);

	// Return the unique integer texture id
	unsigned int id() const;

	// Draw a quad that exactly resembles this texture at the given x/y coordinates
	void draw(int x, int y, float alpha = 1.0f);

	// Check if a given is transparent (alpha < 128) or not
	bool hitTest(int x, int y);
};

inline void cTexture::incref() {
	++refcount;
}

inline void cTexture::decref() {
	if (--refcount == 0) {
		delete this;
	}
}

inline unsigned int cTexture::id() const {
	return id_;
}

inline unsigned int cTexture::width() const {
	return width_;
}

inline unsigned int cTexture::height() const {
	return height_;
}

inline unsigned int cTexture::realWidth() const {
	return realWidth_;
}

inline unsigned int cTexture::realHeight() const {
	return realHeight_;
}

inline void cTexture::setRealWidth(unsigned int data) {
	realWidth_ = data;
}

inline void cTexture::setRealHeight(unsigned int data) {
	realHeight_ = data;
}

#endif
