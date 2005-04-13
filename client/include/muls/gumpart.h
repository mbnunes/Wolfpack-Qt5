
#if !defined( __GUMPART_H__ )
#define __GUMPART_H__

#include <qfile.h>
#include <qdatastream.h>

#include "utilities.h"
#include "surface.h"

class cGumpart {
protected:
	TextureCache *tcache;
	QFile index;
	QFile data;
	QDataStream indexStream;
	QDataStream dataStream;

public:
	cGumpart();
	~cGumpart();

	void load();
	void unload();
	void reload();

	cTexture *readTexture(unsigned short id, unsigned short hue = 0, bool partialHue = false); // Cached
	cSurface *readSurface(unsigned short id, unsigned short hue = 0, bool partialHue = false, bool texture = false); // Uncached
};

extern cGumpart *Gumpart;

#endif
