
#if !defined(__ANIMATIONS_H__)
#define __ANIMATIONS_H__

#include "enums.h"
#include "utilities.h"
#include <qdatastream.h>
#include <qfile.h>
#include <qmap.h>
#include <q3valuevector.h>

#define ANIMATION_FILES 4

/*
	One character animation action.
*/
class cSequence {
protected:
	/*
		One Frame of this Sequence
	*/
	struct stFrame {
		// Texture coordinates for the frame
		float texelLeft;
		float texelRight;
		float texelBottom;
		float texelTop;
		// Not really used for drawing but rather hit testing
		short centerx, centery;
        short width, height;
	};

	unsigned short frameCount_;
	Q3ValueVector<stFrame> frames;

    cTexture *texture_;
	unsigned int refcount;

	unsigned short body_;
	unsigned char action_;
	unsigned char direction_;
	unsigned short hue_;
	bool partialHue_;
public:
	cSequence(unsigned short body, unsigned char action, unsigned char direction, unsigned short hue = 0, bool partialHue_ = false);
	virtual ~cSequence();

	cTexture *texture() const;
	unsigned short body() const;
	unsigned char action() const;
	unsigned char direction() const;
	unsigned short frameCount() const;
	unsigned short hue() const;
	bool partialHue() const;

	// Draw the given frame at the given base
	void draw(int frame, int cellx, int celly, bool flip);

	// Load this sequence from a byte stream
	void load(QDataStream &stream);

	void incref();
	void decref();
};

// return the texture object
inline cTexture *cSequence::texture() const {
	return texture_;
}

inline bool cSequence::partialHue() const {
	return partialHue_;
}

inline unsigned short cSequence::body() const {
	return body_;
}

inline unsigned short cSequence::hue() const {
	return hue_;
}

inline unsigned char cSequence::action() const {
	return action_;
}

inline unsigned char cSequence::direction() const {
	return direction_;
}

inline unsigned short cSequence::frameCount() const {
	return frameCount_;
}

// Decrement the internal reference count
inline void cSequence::decref() {
	if (--refcount == 0) {
		delete this;
	}
}

// Increment the internal reference count
inline void cSequence::incref() {
	++refcount;
}

// A cache for reference counted cAnimation objects.
class SequenceCache : public Q3IntCache< cSequence > {
public:
	SequenceCache(int a, int b) : Q3IntCache< cSequence > (a, b) {
	}

protected:
	void deleteItem(Item d) {
		((cSequence*)d)->decref();
	}
};

class cAnimations {
friend class cAnimation;
protected:
	SequenceCache cache;

	// Calculate a cache id for the given values
	unsigned int getCacheId(unsigned short body, unsigned char action, unsigned char direction, unsigned short hue, bool partialhue);

	// Fallback Body Structure
	struct stFallback {
		unsigned short body;
		short hue;
	};

	// Fallback Body Ids if the specified body couldn't be found
	typedef QMap<unsigned short, stFallback> FallbackMap;
	FallbackMap fallback;

	typedef QMap<unsigned short, unsigned char> FileMap;
	FileMap fileMapping;

	// Get the file id for a given body value. -1 means the file id was invalid.
	signed char getFileId(unsigned short body) const;

	// Try to get a fallback file/body/hue
	bool getFallback(signed char &file, unsigned short &body, unsigned short &hue) const;

	// Calculate the index position of a given body id in a given file
	unsigned int getSeekOffset(signed char file, unsigned short body, unsigned char action, unsigned char direction) const;

	// Stream and file objects for the animation data files
	QFile indexFile[ANIMATION_FILES];
	QFile dataFile[ANIMATION_FILES];
	QDataStream indexStream[ANIMATION_FILES];
	QDataStream dataStream[ANIMATION_FILES];

	// Information about body ids.
	enBodyType bodyTypes[2048];

	// Several private loading functions
	void loadMobTypesTxt();
	void loadBodyConvDef();
	void loadBodyDef();
public:
	// Constructor / Destructor
	cAnimations();
	virtual ~cAnimations();

	cSequence *readSequence(unsigned short body, unsigned char action, unsigned char direction, unsigned short hue = 0, bool partialhue = false);
	
    // Loading and Unloading
	void load();
	void unload();
};

/*
	Calculate a cache id for the given parameters
*/
inline unsigned int cAnimations::getCacheId(unsigned short body, unsigned char action, unsigned char direction, unsigned short hue, bool partialhue) {
	// Only 2048 bodies are possible (11 bit)
	// Only 175 actions are possible (8 bit)
	// Only 5 directions are possible (3 bit)
	// Limit hues to 0xFFF (4096 hues) - (12 bit)
	// Partial hue is a 1 bit flag
	return 0;
}

extern cAnimations *Animations;

#endif
