
#if !defined(__WORLDVIEW_H__)
#define __WORLDVIEW_H__

#include "gui/container.h"
#include "gui/tiledgumpimage.h"
#include "gui/window.h"
#include "gui/cursor.h"
//Added by qt3to4:
#include <QMouseEvent>
#include <Q3CString>

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

	void onMouseDown(QMouseEvent *e);
	void onMouseMotion(int xrel, int yrel, QMouseEvent *e);
	void onMouseUp(QMouseEvent *e);

	void processDoubleClick(QMouseEvent *e);

	void addSysMessage(const Q3CString &message, unsigned short hue = 0, unsigned char font = 3);
	void addSysMessage(const QString &message, unsigned short hue = 0x3b2, unsigned char font = 0);

	void getWorldRect(int &x, int &y, int &width, int &height);

	void draw(int xoffset, int yoffset);

	void moveTick();
};

extern cWorldView *WorldView;

#endif
