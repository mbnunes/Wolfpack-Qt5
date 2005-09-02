
#if !defined(__TEXTURES_H__)
#define __TEXTURES_H__

#include "utilities.h"
#include "texture.h"
#include <qfile.h>
#include <qdatastream.h>

class cTextures : public cTextureCache {
private:
	int offsets[4096];
	bool largeTextures[4096];

	QFile data;
	QDataStream dataStream;
	cTexture *voidTexture;

	QMap<uint, cTexture*> textureCache;
public:
	cTextures();
	~cTextures();

	void load();
	void unload();
	void reload();

	void registerTexture(cTexture *texture);
	void unregisterTexture(cTexture *texture);
	uint cacheSize() const;

	// NEW REFERENCE
	cTexture *readTexture(unsigned short id);
};

inline uint cTextures::cacheSize() const {
	return textureCache.size();
}

extern cTextures *Textures;

#endif
