
#include "gui/scrollbar.h"
//Added by qt3to4:
#include <QMouseEvent>

void cVerticalScrollBar::scrollUp(cControl *sender) {
	setPos(QMAX((int)min(), (int)pos() - 1));
}

void cVerticalScrollBar::scrollDown(cControl *sender) {
	setPos(QMIN(max(), pos() + 1));
}

cVerticalScrollBar::cVerticalScrollBar(int x, int y, unsigned int height) {
	scrollCallback_ = 0;

	btnUp = new cImageButton(0, 0, 0xFA, 0xFB);
	setBounds(x, y, btnUp->width(), height); // Set Width before adding another button
	connect(btnUp, SIGNAL(onClick()), this, SLOT(scrollUp()));
	btnUp->setPressRepeatRate(60);
	addControl(btnUp);

	btnDown = new cImageButton(0, 0, 0xFC, 0xFD);
	btnDown->setPosition(0, height - btnDown->height());
	connect(btnDown, SIGNAL(onClick()), this, SLOT(scrollDown()));
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

void cVerticalScrollBar::onMouseDown(QMouseEvent *e) {
	QPoint pos = mapFromGlobal(e->pos());

	if (pos.y() >= handle->y() && pos.y() < handle->y() + handle->height()) {
		mouseDownY = pos.y() - handle->y();
		draggingHandle_ = true;
	}
}

void cVerticalScrollBar::onMouseUp(QMouseEvent *e) {
	draggingHandle_ = false;
}

void cVerticalScrollBar::onMouseMotion(int xrel, int yrel, QMouseEvent *e) {
	if (draggingHandle_) {
		int y = mapFromGlobal(e->pos()).y(); // Get the y position relative to our control

		int newpos = getPosFromTrackerY(y);
		setPos(newpos);
		//handle->setPosition(handle->x(), newy);
	}
}

unsigned int cVerticalScrollBar::getTrackerYFromPos(int pos) {
	if (pos == (int)min_) {
		return btnUp->height();
	} else if (pos == (int)max_) {
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
