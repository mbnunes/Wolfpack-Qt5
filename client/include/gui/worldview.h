
#if !defined(__WORLDVIEW_H__)
#define __WORLDVIEW_H__

#include "gui/container.h"
#include "gui/tiledgumpimage.h"
#include "gui/window.h"
#include "gui/cursor.h"

class cWorldView : public cWindow {
Q_OBJECT

protected:
	cTiledGumpImage *left, *right, *bottom, *top;

	// Move all controls except the border by the given y amount (used for moving the sysmessages)
	void moveContent(int yoffset);

	bool ismoving;
	unsigned int nextSysmessageCleanup;

	enCursorType getCursorType();
public:
	// Run a sysmessage cleanup check
	void cleanSysMessages();

	cWorldView(unsigned short width, unsigned short height);
	virtual ~cWorldView();
	cControl *getControl(int x, int y);

	void onMouseEnter();
	void onMouseLeave();

	void onMouseDown(int x, int y, unsigned char button, bool pressed);
	void onMouseMotion(int xrel, int yrel, unsigned char button);
	void onMouseUp(int x, int y, unsigned char button, bool pressed);

	void addSysMessage(const QCString &message, unsigned short hue = 0, unsigned char font = 3);
	void addSysMessage(const QString &message, unsigned short hue = 0x3b2, unsigned char font = 0);

	void getWorldRect(int &x, int &y, int &width, int &height);

	void draw(int xoffset, int yoffset);

	void moveTick();
};

extern cWorldView *WorldView;

#endif
