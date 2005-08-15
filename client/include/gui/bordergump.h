
#if !defined(__BORDERGUMP_H__)
#define __BORDERGUMP_H__

#include "control.h"
#include "container.h"

class cTiledGumpImage;
class cGumpImage;

class cBorderGump : public cContainer {
Q_OBJECT
private:
	unsigned short id; // The id of the background gump
	unsigned short hue; // The hue of the background gump

	cTiledGumpImage *left, *right, *top, *bottom, *center;
	cGumpImage *uleft, *uright, *lleft, *lright;
	void alignControls();

public:
	cBorderGump(unsigned short id, unsigned short hue = 0);
	virtual ~cBorderGump();
	void onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight);
	void setGump(unsigned short id, unsigned short hue, bool forceupdate = false);
	void draw(int xoffset, int yoffset);
};

#endif
