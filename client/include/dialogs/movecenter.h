
#if !defined(__MOVECENTER_H__)
#define __MOVECENTER_H__

#include "gui/window.h"

class cMoveCenterDialog : public cWindow {
Q_OBJECT

public:
	cMoveCenterDialog();
	virtual ~cMoveCenterDialog();

public slots:
	void moveUp(cControl *sender);
	void moveDown(cControl *sender);
	void moveRight(cControl *sender);
	void moveLeft(cControl *sender);
};

#endif
