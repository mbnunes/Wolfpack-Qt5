
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
	pressRepeatTimer_ = 0; // Defaults to no timer id

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

	callback = 0;
}

cImageButton::~cImageButton() {
	for (int i = 0; i < 4; ++i) {
		if (gumps[i]) {
			gumps[i]->decref();
		}
	}
}

void cImageButton::update() {
	// Check for our own dirty flags here

	dirty_ = false;
}

void cImageButton::onMouseLeave() {
	mouseOver_ = false;
	invalidate();
}

void cImageButton::onMouseEnter() {
	mouseOver_ = true;
	invalidate();
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
	if (isDirty()) {
		update();
	}

	cTexture *texture = gumps[getState()];

	if (texture) {
		texture->draw(xoffset + x_, yoffset + y_);
	}
}

static unsigned int pressRepeatCallback(unsigned int interval, cImageButton *button) {    
	if (button->mouseHolding() && button->pressRepeatRate() != 0) {
		button->onClick();
	} else {
		return 0; // Stop timer?
	}
	return interval;
}

void cImageButton::onMouseDown(int x, int y, unsigned char button, bool pressed) {
	if (button == SDL_BUTTON_LEFT) {
		mouseHolding_ = true;
		invalidate();

		// Start a press repeat timer if requested by the owner
		if (pressRepeatRate_ != 0) {
			// Stop an old repeat timer
			if (pressRepeatTimer_) {
				SDL_RemoveTimer(pressRepeatTimer_);
				pressRepeatTimer_ = 0;
			}
			pressRepeatTimer_ = SDL_AddTimer(pressRepeatRate_, (SDL_NewTimerCallback)pressRepeatCallback, this);
		}
	} else {
		// Stop an old repeat timer
		if (pressRepeatTimer_) {
			SDL_RemoveTimer(pressRepeatTimer_);
			pressRepeatTimer_ = 0;
		}

		mouseHolding_ = false;
		invalidate();
	}
}

void cImageButton::onMouseUp(int x, int y, unsigned char button, bool pressed) {
	if (button = SDL_BUTTON_LEFT) {
		mouseHolding_ = false;
		invalidate();

		// Stop an old repeat timer
		if (pressRepeatTimer_) {
			SDL_RemoveTimer(pressRepeatTimer_);
			pressRepeatTimer_ = 0;
		} else if (mouseOver_) {
			onClick(); // Notify the button about the click
		}
	}
}

void cImageButton::onClick() {
	if (callback) {
		callback(this);
	}
}

void cImageButton::onKeyDown(const SDL_keysym &key) {
	if (key.sym == SDLK_RETURN) {
		onClick(); // Fire the onClick event if return is pressed
	} else if (key.sym == SDLK_SPACE) {
		spaceHolding_ = true;
		invalidate();
	}
}

void cImageButton::onKeyUp(const SDL_keysym &key) {
	if (key.sym == SDLK_SPACE) {
		spaceHolding_ = false;
		invalidate();
		onClick(); // Issue the click event
	}
}

void cImageButton::onBlur(cControl *newFocus) {
	if (spaceHolding_) {
		spaceHolding_ = false;
		invalidate();
	}
	cControl::onBlur(newFocus);
}
