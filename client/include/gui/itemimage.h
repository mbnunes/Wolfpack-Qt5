
#if !defined(__ITEMIMAGE_H__)
#define __ITEMIMAGE_H__

#include "utilities.h"
#include "control.h"

#include "SDL.h"

class cItemImage : public cControl {
Q_OBJECT

private:
	SharedSurface *item;
	unsigned short id;
	unsigned short hue;
	bool partialhue;
	bool translucent;
	bool stacked;
	bool landtile;
public:
	cItemImage(unsigned short id, unsigned short hue = 0, bool partialhue = false, bool stacked = false, bool translucent = false, bool landtile = false);
	virtual ~cItemImage();

	cControl *getControl(int x, int y);
	void setItem(unsigned short id, unsigned short hue, bool partialhue, bool stacked, bool translucent, bool landtile);
};

#endif
