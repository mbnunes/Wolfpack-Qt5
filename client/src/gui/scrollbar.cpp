
#include "gui/scrollbar.h"

static void btnUpPress(cControl *ctrl) {
	cVerticalScrollBar *scrollbar = (cVerticalScrollBar*)ctrl->parent();
	scrollbar->setPos(QMAX(0, (int)scrollbar->pos() - 1));
}

static void btnDownPress(cControl *ctrl) {
	cVerticalScrollBar *scrollbar = (cVerticalScrollBar*)ctrl->parent();
	scrollbar->setPos(QMIN(scrollbar->max(), scrollbar->pos() + 1));
}

cVerticalScrollBar::cVerticalScrollBar(int x, int y, unsigned int height) {
	scrollCallback_ = 0;

	btnUp = new cImageButton(0, 0, 0xFA, 0xFB);
	setBounds(x, y, btnUp->width(), height); // Set Width before adding another button
	btnUp->setCallback(btnUpPress);
	btnUp->setPressRepeatRate(60);
	addControl(btnUp);

	btnDown = new cImageButton(0, 0, 0xFC, 0xFD);
	btnDown->setPosition(0, height - btnDown->height());
	btnDown->setCallback(btnDownPress);
	btnDown->setPressRepeatRate(60);
	addControl(btnDown);
	
	background = new cTiledGumpImage(0x100);
	background->setBounds(0, btnUp->height(), btnUp->width(), height - btnUp->height() - btnDown->height());
	addControl(background, true);

	handle = new cGumpImage(0xfe);
	handle->setPosition(0, btnUp->height());
	addControl(handle);

	draggingHandle_ = false;

	pos_ = 0;
	setRange(0, 0);	
}

cVerticalScrollBar::~cVerticalScrollBar() {
}

cControl *cVerticalScrollBar::getControl(int x, int y) {
	cControl *result = cContainer::getControl(x, y);

	if (result == background || result == handle) {
		return this; // All events should go to us instead
	} else {
		return result;
	}
}

void cVerticalScrollBar::onMouseDown(int x, int y, unsigned char button, bool pressed) {
	SDL_GetMouseState(&x, &y);

	// Get x, y relative to our pov
	cContainer *parent = parent_;
	while (parent) {
		// Modify x,y
		x -= parent->x();
		y -= parent->y();
		parent = parent->parent();
	}

	if (y >= handle->y() && y < handle->y() + handle->height()) {
		mouseDownY = y - handle->y();
		draggingHandle_ = true;
	}
}

void cVerticalScrollBar::onMouseUp(int x, int y, unsigned char button, bool pressed) {
	draggingHandle_ = false;
}

void cVerticalScrollBar::onMouseMotion(int xrel, int yrel, unsigned char buttons) {
	if (draggingHandle_) {
		// Get the mouse state
		int x, y;
		SDL_GetMouseState(&x, &y);

		// Get x, y relative to our pov
		cContainer *parent = parent_;
		while (parent) {
			// Modify x,y
			x -= parent->x();
			y -= parent->y();
			parent = parent->parent();
		}

		int newpos = getPosFromTrackerY(y);
		setPos(newpos);
		//handle->setPosition(handle->x(), newy);
	}
}

unsigned int cVerticalScrollBar::getTrackerYFromPos(int pos) {
	if (pos == min_) {
		return btnUp->height();
	} else if (pos == max_) {
		return height_ - btnDown->height() - handle->height();
	} else {
		return btnUp->height() + (int)(pos * pixelPerStep);
	}
}

unsigned int cVerticalScrollBar::getPosFromTrackerY(int y) {
	if (y < btnUp->height() + handle->height()) {
		return min_;
	} else if (y > height_ - btnDown->height() - handle->height()) {
		return max_;
	} else {
		return QMIN((int)max_, QMAX((int)min_, (int)((y - (int)btnUp->height()) / pixelPerStep)));
	}
}

void cVerticalScrollBar::onScroll(int oldpos) {
	if (scrollCallback_) {
		scrollCallback_(this, oldpos);
	}
}
