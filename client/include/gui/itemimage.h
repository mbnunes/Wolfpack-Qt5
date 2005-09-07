
#if !defined(__ITEMIMAGE_H__)
#define __ITEMIMAGE_H__

#include "utilities.h"
#include "control.h"

class cItemImage : public cControl {
Q_OBJECT

protected:
	cTexture *item;
	unsigned short id;
	unsigned short hue;
	bool partialhue;
	bool landtile;
	bool stacked_;
public:
	cItemImage(unsigned short id, unsigned short hue = 0, bool partialhue = false, bool landtile = false);
	virtual ~cItemImage();

	cControl *getControl(int x, int y);
	void setItem(unsigned short id, unsigned short hue, bool partialhue, bool landtile);
	void draw(int xoffset, int yoffset);
};

#endif
