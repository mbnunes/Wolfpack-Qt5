
#if !defined(__ANIMATIONS_H__)
#define __ANIMATIONS_H__

#include "enums.h"
#include "utilities.h"
#include <qdatastream.h>
#include <qfile.h>
#include <QMap>
#include <qvector.h>

#define ANIMATION_FILES 4

// Used to identify sequences in the cache
struct stSequenceIdent {
	unsigned short body;
	unsigned char action;
	unsigned char direction;
	unsigned short hue;
	bool partialhue;
};

inline bool operator < (const stSequenceIdent &a, const stSequenceIdent &b) {
	if (a.body != b.body) {
		return a.body < b.body;
	} else if (a.action != b.action) {
		return a.action < b.action;
	} else if (a.direction != b.direction) {
		return a.direction < b.direction;
	} else if (a.hue != b.hue) {
		return a.hue < b.hue;
	}
	return !a.partialhue && b.partialhue;
}

/*
	One character animation action.
*/
class cSequence {
protected:
	stSequenceIdent ident;

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
	QVector<stFrame> frames;

    cTexture *texture_;
	unsigned int refcount;

	unsigned short body_;
	unsigned char action_;
	unsigned char direction_;
	unsigned short hue_;
	bool partialHue_;
public:
	cSequence(unsigned short body, unsigned char action, unsigned char direction, unsigned short hue = 0, bool partialHue_ = false);
	~cSequence();

	// Get the top of a frame relative to the bottom of the cell
	int getFrameTop(int frame);

	cTexture *texture() const;
	unsigned short body() const;
	unsigned char action() const;
	unsigned char direction() const;
	unsigned short frameCount() const;
	unsigned short hue() const;
	bool partialHue() const;

	void setIdent(const stSequenceIdent &ident);

	// Draw the given frame at the given base
	void draw(int frame, int cellx, int celly, bool flip, float alpha = 1.0f);
	bool hitTest(int frame, int x, int y, bool flip);

	// Load this sequence from a uchar stream
	void load(QDataStream &stream);

	void incref();
	void decref();

	uint getMemorySize() const;
};

inline uint cSequence::getMemorySize() const {
	uint result = sizeof(cSequence);
	if (texture_) {
		result += texture_->getMemorySize();
	}
	
	result += frameCount_ * sizeof(cSequence::stFrame);
	return result;
}

inline void cSequence::setIdent(const stSequenceIdent &ident) {
	this->ident = ident;
}

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

class cAnimations {
friend class cSequence;
protected:
	void beforeSequenceDeletion(cSequence *sequence);

	QVector<cSequence*> ownedSequences_; // Sequences that are in-memory but currently unused
	uint totalSequenceSize_; // Total amount of memory consumed by loaded sequences (in uchars, estimate)
	QMap<stSequenceIdent, cSequence*> SequenceCache;

	enBodyType bodyTypes[4096]; // Static lookup table for body types
	unsigned int flags[4096]; // Static lookup table for flags

	// Fallback Body Structure
	struct stFallback {
		unsigned short body;
		short hue;
	};

	// Fallback Body Ids if the specified body couldn't be found
	typedef QMap<unsigned short, stFallback> FallbackMap;
	FallbackMap fallback;

	struct stFileInfo {
		unsigned short body;
		unsigned char file;
	};

	typedef QMap<unsigned short, stFileInfo> FileMap;
	FileMap fileMapping;

	// Get the file id for a given body value. -1 means the file id was invalid.
	signed char getFileId(unsigned short &body) const;

	// Try to get a fallback file/body/hue
	bool getFallback(signed char &file, unsigned short &body, unsigned short &hue) const;

	// Calculate the index position of a given body id in a given file
	unsigned int getSeekOffset(signed char file, unsigned short body, unsigned char action, unsigned char direction) const;

	// Stream and file objects for the animation data files
	QFile indexFile[ANIMATION_FILES];
	QFile dataFile[ANIMATION_FILES];
	QDataStream indexStream[ANIMATION_FILES];
	QDataStream dataStream[ANIMATION_FILES];

	// Several private loading functions
	void loadMobTypesTxt();
	void loadBodyConvDef();
	void loadBodyDef();
public:
	// Constructor / Destructor
	cAnimations();
	virtual ~cAnimations();

	cSequence *readSequence(unsigned short body, unsigned char action, unsigned char direction, unsigned short hue = 0, bool partialhue = false);
	
	enBodyType getBodyType(unsigned short body) const;
	uint getFlags(unsigned short body) const;

	uint cacheSize() const;
	uint totalSequenceSize() const; // Size in uchars, estimated
	uint maximumSequenceSize() const;

    // Loading and Unloading
	void load();
	void unload();
};

inline enBodyType cAnimations::getBodyType(unsigned short body) const {
	if (body < 4096) {
		return bodyTypes[body];
	} else {
		return HUMAN; // Default
	}
}

inline uint cAnimations::getFlags(unsigned short body) const {
	if (body < 4096) {
		return flags[body];
	} else {
		return 0;
	}
}

inline uint cAnimations::totalSequenceSize() const {
	return totalSequenceSize_;
}

inline uint cAnimations::maximumSequenceSize() const {
	return 32 * 1024 * 1024; // 10 MB of sequences should be reasonable
}

extern cAnimations *Animations;

#endif
