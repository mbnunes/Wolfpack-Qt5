
#if !defined( __GUMPART_H__ )
#define __GUMPART_H__

#include <qfile.h>
#include <qdatastream.h>

#include "utilities.h"
#include "surface.h"
#include "texture.h"

class cGumpart : public cTextureCache {
protected:
	// Include all possible parameters for a loaded gump here
	struct stGumpIdent {
		ushort id;
		ushort hue;
		bool partialHue;

		operator <(const stGumpIdent &b) const {
			return id + hue + (partialHue ? 1 : 0) < b.id + b.hue + (b.partialHue ? 1 : 0);
		}
	};

	TextureCache *tcache;
	QFile index;
	QFile data;
	QDataStream indexStream;
	QDataStream dataStream;
	QMap<stGumpIdent, cTexture*> textureCache;

public:
	cGumpart();
	~cGumpart();

	void load();
	void unload();
	void reload();

	void registerTexture(cTexture *texture);
	void unregisterTexture(cTexture *texture);
	uint cacheSize() const;

	cTexture *readTexture(unsigned short id, unsigned short hue = 0, bool partialHue = false); // Cached
	cSurface *readSurface(unsigned short id, unsigned short hue = 0, bool partialHue = false, bool texture = false); // Uncached
};

extern cGumpart *Gumpart;

#endif
