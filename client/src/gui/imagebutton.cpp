
#include <qglobal.h>

#include "gui/imagebutton.h"
#include "muls/gumpart.h"
#include "utilities.h"

cImageButton::cImageButton(int x, int y, unsigned short up, unsigned short down) {
	x_ = x;
	y_ = y;
	mouseOver_ = false;
	mouseHolding_ = false;
	spaceHolding_ = false;
	canHaveFocus_ = true; // Buttons can have the input focus
	pressRepeatRate_ = 0; // There is no press repeat rate by default (normal push buttons)

	for (int i = 0; i < 4; ++i) {
		gumphues[i] = 0;
		gumps[i] = 0;
		partialhue[i] = false;
	}

	width_ = 0;
	height_ = 0;

	gumps[BS_UNPRESSED] = Gumpart->readTexture(up);
	gumps[BS_PRESSED] = Gumpart->readTexture(down);
	
	if (gumps[BS_UNPRESSED]) {
		width_ = gumps[BS_UNPRESSED]->realWidth();
		height_ = gumps[BS_UNPRESSED]->realHeight();
	}

	pressRepeatTimer = new QTimer(this); // Should be auto freed
	connect(pressRepeatTimer, SIGNAL(timeout()), this, SLOT(repeatPress())); // Connect the timer to the press repeat slot
}

cImageButton::~cImageButton() {
	for (int i = 0; i < 4; ++i) {
		if (gumps[i]) {
			gumps[i]->decref();
		}
	}
}

void cImageButton::onMouseLeave() {
	mouseOver_ = false;
}

void cImageButton::onMouseEnter() {
	mouseOver_ = true;
}

void cImageButton::setStateGump(enButtonStates state, unsigned short id, unsigned short hue, bool partialHue) {
	if (gumps[state]) {
		gumps[state]->decref();
	}

	gumps[state] = Gumpart->readTexture(id, hue, partialHue);
}

cControl *cImageButton::getControl(int x, int y) {
	cTexture *texture = gumps[getState()];

	if (texture && texture->hitTest(x, y)) {
		return this;
	}

	return 0;
}

void cImageButton::draw(int xoffset, int yoffset) {
	cTexture *texture = gumps[getState()];

	if (texture) {
		texture->draw(xoffset + x_, yoffset + y_);
	}
}

void cImageButton::onMouseDown(QMouseEvent *e) {
	if (e->button() == LeftButton) {
		mouseHolding_ = true;

		// The button requests to be auto-pressed every x miliseconds
		if (pressRepeatRate_ != 0) {
			pressRepeatTimer->start(pressRepeatRate_);
			emit onClick(this);
		}
	}
}

void cImageButton::repeatPress() {
	// Send a repeated onClick event if we're above the button
	// and it's configured to do so
	if (mouseHolding_ && pressRepeatRate_ && mouseOver_) {
		emit onClick(this);
	}
}

void cImageButton::onMouseUp(QMouseEvent *e) {
	if (e->button() == LeftButton) {
		mouseHolding_ = false;
		
		// If a press-repeat timer is active, stop it. 
		// Otherwise issue the click event.
		if (pressRepeatTimer->isActive()) {
			pressRepeatTimer->stop();
		} else if (mouseOver_) {
			emit onClick(this);
		}
	}
}

void cImageButton::onKeyDown(QKeyEvent *e) {
	if (e->key() == Key_Return) {
		emit onClick(this); // Fire the onClick event if return is pressed
	} else if (e->key() == Qt::Key_Space) {
		spaceHolding_ = true;
	}
}

void cImageButton::onKeyUp(QKeyEvent *e) {
	if (e->key() == Key_Space) {
		spaceHolding_ = false;
		emit onClick(this); // Issue the click event
	}
}

void cImageButton::onBlur(cControl *newFocus) {
	if (spaceHolding_) {
		spaceHolding_ = false;
	}
	cControl::onBlur(newFocus);
}
