
#include "engine.h"
#include "gui/worldview.h"
#include "gui/label.h"
#include "gui/asciilabel.h"
#include "gui/cursor.h"
#include "game/world.h"
#include "game/mobile.h"
#include "uoclient.h"
#include <math.h>

const unsigned int sysMessageDecay = 10000;

class cSysMessage : public cLabel {
protected:
	unsigned int created_;
public:
	unsigned int created() const;
	void setCreated(unsigned int data);

	cSysMessage(const QString &message, unsigned char font, unsigned short hue) : cLabel(message, font, hue) {
	}
};

inline unsigned int cSysMessage::created() const {
	return created_;
}

inline void cSysMessage::setCreated(unsigned int data) {
	created_ = data;
}

void cWorldView::cleanSysMessages() {
	QValueList<cControl*> toremove;
	unsigned int currentTime = SDL_GetTicks();

	Iterator it;
	for (it = controls.begin(); it != controls.end(); ++it) {
		cControl *control = *it;
		if (control != left && control != right && control != top && control != bottom) {
			cSysMessage *message = (cSysMessage*)(control);
			if (message->created() + sysMessageDecay < currentTime) {
				toremove.append(control);
			}
		}
	}

	QValueList<cControl*>::iterator it2;
	for (it2 = toremove.begin(); it2 != toremove.end(); ++it2) {
		removeControl(*it2);
		delete *it2;
	}
}

cWorldView::cWorldView(unsigned short width, unsigned short height) {
	top = new cTiledGumpImage(0xa8c);
	top->setHeight(4);
	top->setAlign(CA_TOP);
	top->setMoveHandle(true);
	addControl(top);

	left = new cTiledGumpImage(0xa8d);
	left->setWidth(5);
	left->setAlign(CA_LEFT);
	left->setMoveHandle(true);
	addControl(left);

	bottom = new cTiledGumpImage(0xa8c);
	bottom->setHeight(4);
	bottom->setAlign(CA_BOTTOM);	
	bottom->setMoveHandle(true);
	addControl(bottom);

	right = new cTiledGumpImage(0xa8d);
	right->setWidth(5);
	right->setAlign(CA_RIGHT);
	right->setMoveHandle(true);
	addControl(right);

	setBounds(x_, y_, width, height);

	ismoving = false;
	nextSysmessageCleanup = SDL_GetTicks() + 250;
}

cWorldView::~cWorldView() {
}

cControl *cWorldView::getControl(int x, int y) {
	if (x >= 0 && y >= 0 && x < width_ && y < height_) {
		return this;
	} else {
		return 0;
	}
}

void cWorldView::update() {
	if (!dirty_) {
		return; // Nothing to update
	}

	for (Iterator it = controls.begin(); it != controls.end(); ++it) {
		cControl *control = *it;
		if (control->isVisible()) {
			if (control->isDirty()) {
				control->update(); // Update the control before drawing it
			}
		}
	}

	dirty_ = false; // Flag as non-dirty
}

// Stop tracking, otherwise pass to World.
void cWorldView::onMouseUp(int x, int y, unsigned char button, bool pressed) {
	if (ismoving) {
		ismoving = false;
		Cursor->setCursor(getCursorType());
	} else {
		if (!tracking) {
			World->onClick(x - x_, y - y_, button);
		} else {
			cWindow::onMouseUp(x, y, button, pressed);
		}
	}
}

// Start tracking if we're touching the border. Otherwise pass to world.
void cWorldView::onMouseDown(int x, int y, unsigned char button, bool pressed) {
	// Try getting the control at that position
	cControl *ctrl = cContainer::getControl(x - x_, y - y_);

	// Start tracking if above border.
	if (ctrl == left || ctrl == right || ctrl == top || ctrl == bottom) {
		tracking = true;
	} else {
		if (button == SDL_BUTTON_RIGHT) {
			ismoving = true;
		}
	}
}

// Move the window around if we're tracking. Otherwise pass to world.
void cWorldView::onMouseMotion(int xrel, int yrel, unsigned char button) {
	if (tracking) {
		cWindow::onMouseMotion(xrel, yrel, button);
	} else {
		// pass to world
		Cursor->setCursor(getCursorType());
	}
}

void cWorldView::addSysMessage(const QCString &message, unsigned short hue, unsigned char font) {
}

void cWorldView::addSysMessage(const QString &message, unsigned short hue, unsigned char font) {
	cSysMessage *label = new cSysMessage(message, font, hue);
	label->refreshSurface(); // Create the surface so the height is known
	label->setCreated(SDL_GetTicks());
	label->setPosition(left->width(), bottom->y() - 50 - label->height());
	moveContent(- label->height() - 4);
	addControl(label);
	invalidate();
}

void cWorldView::moveContent(int yoffset) {
	QValueList<cControl*> toremove;

	cContainer::Iterator it;
	for (it = controls.begin(); it != controls.end(); ++it) {
		cControl *control = *it;
		if (control != left && control != top && control != bottom && control != right) {
			if (control->y() + control->height() + yoffset <= top->height()) {
				// The control would become invisible. Queue it for deletion.
				toremove.append(control);
			} else {
				// Move the control up if it would still be visible
				control->setPosition(control->x(), control->y() + yoffset);
			}
		}
	}

	// Remove controls that came out of view
	QValueList<cControl*>::iterator it2;
	for (it2 = toremove.begin(); it2 != toremove.end(); ++it2) {
		removeControl(*it2);
		delete *it2;
	}
}

void cWorldView::moveTick() {
	// we're no longer moving
	if (!ismoving) {
		return;
	}

	// TEMPORARY HACK UNTIL PLAYER OBJECT + NETWORKING EXISTS
	static unsigned int nextmove = 0;

	if (nextmove > SDL_GetTicks()) {
		return;
	}

	nextmove = SDL_GetTicks() + 75;

	// Get the direction the cursor is pointing to
	enCursorType cursor = getCursorType();
	int xdiff = 0;
	int ydiff = 0;

	switch (cursor) {
		// Up
		case CURSOR_DIR0:
			xdiff = -1;
			ydiff = -1;
			break;
		// Up-Right
		case CURSOR_DIR1:
			xdiff = 0;
			ydiff = -1;
			break;
		// Right
		case CURSOR_DIR2:
			xdiff = 1;
			ydiff = -1;
			break;
		// Down-Right
		case CURSOR_DIR3:
			xdiff = 1;
			ydiff = 0;
			break;
		// Down
		case CURSOR_DIR4:
			xdiff = 1;
			ydiff = 1;
			break;
		// Down-Left
		case CURSOR_DIR5:
			xdiff = 0;
			ydiff = 1;
			break;
		// Left
		case CURSOR_DIR6:
			xdiff = -1;
			ydiff = 1;
			break;
		// Up-Left
		case CURSOR_DIR7:
			xdiff = -1;
			ydiff = 0;
			break;
		// Default
		default:
			return; // Do nothing
	};

	// Move the world
	World->smoothMove(xdiff, ydiff);
}

enCursorType cWorldView::getCursorType() {
	int mx, my;
	SDL_GetMouseState(&mx, &my);

	// Return a normal cursor if we're outside the worldview
	if (!ismoving && (mx < x_ + left->width() || mx >= x_ + width_ - right->width() || 
		my < y_ + top->height() || my >= y_ + height_ - bottom->height())) {
		return CURSOR_NORMAL;
	}

	// Get center of world
	int centerx = x_ + width_ / 2;
	int centery = y_ + height_ / 2;

	// We assume that the cursor and the center of the
	// worldview open a triangle.
	float diffx = (float)(mx - centerx);
	float diffy = (float)(my - centery);

	// Center of the worldview
	if (abs(diffx) < 10.0f && abs(diffy) < 10.0f) {
		return CURSOR_NORMAL;
	}

    float diagonal = sqrt(diffx * diffx + diffy * diffy); // Calculate diagonal	
	float asina = asin(diffy / diagonal); // Calculate the sin of the angle

	const float treshold1 = PI / 8; // Upper treshold
	const float treshold2 = PI / 2 - treshold1; // Lower treshold

	// Since the sin is symmetric, handle both halfs of the screen separately
	if (diffx < 0) {
		if (asina < - treshold2) {
			return CURSOR_DIR0; // Up
		} else if (asina < - treshold1) {
			return CURSOR_DIR7; // Up-Left
		} else if (asina < treshold1) {
			return CURSOR_DIR6; // Left
		} else if (asina < treshold2) {
			return CURSOR_DIR5; // Down-Left
		} else {
			return CURSOR_DIR4; // Down
		}
	} else {
		if (asina < - treshold2) {
			return CURSOR_DIR0; // Up
		} else if (asina < - treshold1) {
			return CURSOR_DIR1; // Up-Left
		} else if (asina < treshold1) {
			return CURSOR_DIR2; // Left
		} else if (asina < treshold2) {
			return CURSOR_DIR3; // Down-Left
		} else {
			return CURSOR_DIR4; // Down
		}
	}

	return CURSOR_DIR0;
}

void cWorldView::onMouseEnter() {
	Cursor->setCursor(getCursorType());
}

void cWorldView::onMouseLeave() {
	// Only rsetore the cursor if we're not tracking the mosue movement
	if (!ismoving) {
		Cursor->setCursor(CURSOR_NORMAL);
	}
}

void cWorldView::getWorldRect(int &x, int &y, int &width, int &height) {
	x = x_ + left->width();
	y = y_ + top->height();
	width = width_ - left->width() - right->width();
	height = height_ - top->height() - bottom->height();
}

void cWorldView::draw(int xoffset, int yoffset) {
	// Remove outdated sysmessages
	if (nextSysmessageCleanup < SDL_GetTicks()) {
		cleanSysMessages();
		nextSysmessageCleanup = SDL_GetTicks() + 250;	
	}

	cWindow::draw(xoffset, yoffset);
}

cWorldView *WorldView = 0;
