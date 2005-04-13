
#if !defined(__WINDOW_H__)
#define __WINDOW_H__

#include "gui/container.h"

class cTexture;

/*
	This class represents a top level window.
*/
class cWindow : public cContainer {
protected:
	bool movable_;
	bool closable_;	
	bool tracking;
public:
	cWindow();
	virtual ~cWindow();

	// This is a window (see cControl::isWindow)
	bool isWindow() const;

	// Manage the closable property
	void setClosable(bool data);
	bool isClosable() const;

	// Manage the movable property
	void setMovable(bool data);
	bool isMovable();

	// The update method has to recreate the texture
	void update();

	// Draw this window
	void draw(int xoffset, int yoffset);

	// This function has to be overriden for dragging the window around to work
	cControl *getControl(int x, int y);

	// These event handlers are mostly for closing and moving the window
	void onMouseDown(QMouseEvent *e);
	void onMouseUp(QMouseEvent *e);
	void onMouseMotion(int xrel, int yrel, QMouseEvent *e);
};

inline void cWindow::setClosable(bool data) {
	closable_ = data;
}

inline bool cWindow::isClosable() const {
	return closable_;
}

inline void cWindow::setMovable(bool data) {
	movable_ = data;
}

inline bool cWindow::isMovable() {
	return movable_;
}

#endif
