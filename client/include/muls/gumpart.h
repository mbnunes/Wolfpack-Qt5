
#if !defined( __GUMPART_H__ )
#define __GUMPART_H__

#include "SDL.h"

class cGumpart {
private:
	class stGumpartPrivateImpl *d;

public:
	cGumpart();
	~cGumpart();

	void load();
	void unload();
	void reload();

	cTexture *readTexture(unsigned short id, unsigned short hue = 0, bool partialHue = false);
	SDL_Surface *read(unsigned short id, unsigned short hue = 0, bool partialHue = false, bool texture = false, unsigned int *outWidth = 0, unsigned int *outHeight = 0);
};

extern cGumpart *Gumpart;

#endif
