
#if !defined(__TEXTURES_H__)
#define __TEXTURES_H__

#include "utilities.h"
#include "texture.h"
#include <qfile.h>
#include <qdatastream.h>

class cTextures {
private:
	int offsets[4096];
	bool largeTextures[4096];

	QFile data;
	QDataStream dataStream;
	cTexture *voidTexture;
public:
	cTextures();
	~cTextures();

	void load();
	void unload();
	void reload();

	// NEW REFERENCE
	cTexture *readTexture(unsigned short id);
};

extern cTextures *Textures;

#endif
