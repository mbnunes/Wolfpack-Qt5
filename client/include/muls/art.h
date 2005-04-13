
#if !defined(__ART_H__)
#define __ART_H__

#include <qfile.h>
#include <qdatastream.h>
#include <qmap.h>

#include "gui/cursor.h"
#include "utilities.h"
#include "surface.h"

// Structure for the framedata read from animdata.mul
struct stArtFrame {
	unsigned short id; // Art id of this frames tile
	float texelLeft, texelTop, texelBottom, texelRight; // Texture mapping coordinates for this frame
	unsigned short x, y; // x,y coordinate within texture (hit testing)
	unsigned short width, height; // width,height in texture (hit testing)
};

// This class represents an animated art tile
class cArtAnimation {
protected:
	unsigned short id_;
	unsigned short hue_;
	bool partialHue_;
	cTexture *texture_;
	stArtFrame *frames_;
	unsigned char frameCount_;
	unsigned int startDelay_;
	unsigned int frameDelay_;
	unsigned int refcount;
public:
	cArtAnimation();
	~cArtAnimation();

    // Getters
	unsigned short id() const;
	unsigned short hue() const;
	bool partialHue() const;
	cTexture *texture() const;
	const stArtFrame *frames() const;
	unsigned char frameCount() const;
	unsigned int startDelay() const;
    unsigned int frameDelay() const;

	// Setters
	void setId(unsigned short data);
	void setHue(unsigned short data);
	void setPartialHue(bool data);
	void setTexture(cTexture *data);
	void setFrames(stArtFrame *data);
	void setFrameCount(unsigned char data);
	void setStartDelay(unsigned int data);
	void setFrameDelay(unsigned int data);

	void incref();
	void decref();
};

inline unsigned short cArtAnimation::id() const {
	return id_;
}

inline unsigned short cArtAnimation::hue() const {
	return hue_;
}

inline bool cArtAnimation::partialHue() const {
	return partialHue_;
}

inline cTexture *cArtAnimation::texture() const {
	return texture_;
}

inline const stArtFrame *cArtAnimation::frames() const {
	return frames_;
}

inline unsigned char cArtAnimation::frameCount() const {
	return frameCount_;
}

inline unsigned int cArtAnimation::startDelay() const {
	return startDelay_;
}

inline unsigned int cArtAnimation::frameDelay() const {
	return frameDelay_;
}

inline void cArtAnimation::setId(unsigned short data) {
	id_ = data;
}

inline void cArtAnimation::setHue(unsigned short data) {
	hue_ = data;
}

inline void cArtAnimation::setPartialHue(bool data) {
	partialHue_ = data;
}

inline void cArtAnimation::setTexture(cTexture *data) {
	texture_ = data;
}

inline void cArtAnimation::setFrames(stArtFrame *data) {
	frames_ = data;
}

inline void cArtAnimation::setFrameCount(unsigned char data) {
	frameCount_ = data;
}

inline void cArtAnimation::setStartDelay(unsigned int data) {
	startDelay_ = data;
}

inline void cArtAnimation::setFrameDelay(unsigned int data) {
	frameDelay_ = data;
}

inline void cArtAnimation::incref() {
	++refcount;
}

inline void cArtAnimation::decref() {
	if (--refcount == 0) {
		delete this;
	}
}

class ArtAnimationCache : public QIntCache< cArtAnimation> {
public:
	ArtAnimationCache(int a, int b) : QIntCache< cArtAnimation > (a, b) {
	}

protected:
	void deleteItem(Item d) {
		((cArtAnimation*)d)->decref();
	}
};

class cArt {
private:
	TextureCache *tcache; // Cache for textures (uses same indexing scheme as the normal cache)
	ArtAnimationCache *acache;
	QFile data, index; // Input files
	QDataStream dataStream, indexStream; // Input streams
	
	// Private structure for storing information about art animations
	struct stAnimdata {
		signed char frames[64]; // Offset from the baseid for each frame
		unsigned char count; // The number of frames
        unsigned int interval; // Interval between frames in miliseconds
		unsigned int startdelay; // Delay in miliseconds before the animation starts
	};

	QMap<unsigned short, stAnimdata> animdata;
public:
	cArt();
	~cArt();

	void load();
	void unload();
	void reload();

	// Read an animation into a texture stripe and write the texel coordinates into the given
	// frame structure. The id is used internally for caching the resulting animation texture.
	cArtAnimation *readAnimation(unsigned short id, unsigned short hue = 0, bool partialHue = false);

	// Read a cursor and fill the given cursor structure with the corresponding data
	void readCursor(stCursor *cursor, unsigned short id, unsigned short hue = 0, bool partialhue = false);

	cSurface *readLandSurface(unsigned short id, bool texture = false); // Uncached
	cTexture *readLandTexture(unsigned short id); // Cached, New Reference
	cSurface *readItemSurface(unsigned short id, unsigned short hue = 0, bool partialhue = false, bool texture = false); // Uncached
	cTexture *readItemTexture(unsigned short id, unsigned short hue = 0, bool partialhue = false); // Cached, New Reference

	// Calculates the cache id for the given parameters
	// Only the lower 12 bit of the hue are valid (0-2999 are valid hue ids)
	inline unsigned int getCacheId(unsigned short id, unsigned short hue = 0, bool partialhue = false) {
		unsigned int result = id;

		if (partialhue) {
			result |= 0x80000000; // 32st bit indicates partial hue
		}

		result |= (hue & 0xFFF) << 16; // Store the hue

		return result;
	}
};

extern cArt *Art;

#endif
