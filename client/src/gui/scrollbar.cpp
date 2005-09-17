
#include "gui/scrollbar.h"
#include <QMouseEvent>
#include <math.h>
#include "utilities.h"

void cVerticalScrollbar::scrollUp(cControl *sender) {
	setPos(qMax<int>(min(), pos() - 1));
}

void cVerticalScrollbar::scrollDown(cControl *sender) {
	setPos(qMin<int>(max(), pos() + 1));
}

cVerticalScrollbar::cVerticalScrollbar(int x, int y, unsigned int height) : btnUp(0), btnDown(0), handle(0) {
	btnUp = new cImageButton(0, 0, 0xFA, 0xFB);
	setBounds(x, y, btnUp->width(), height); // Set Width before adding another button
	connect(btnUp, SIGNAL(onButtonPress(cControl*)), this, SLOT(scrollUp(cControl*)));
	btnUp->setPressRepeatRate(60);
	addControl(btnUp);

	btnDown = new cImageButton(0, 0, 0xFC, 0xFD);
	btnDown->setPosition(0, height - btnDown->height());
	connect(btnDown, SIGNAL(onButtonPress(cControl*)), this, SLOT(scrollDown(cControl*)));
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

cVerticalScrollbar::~cVerticalScrollbar() {
}

cControl *cVerticalScrollbar::getControl(int x, int y) {
	cControl *result = cContainer::getControl(x, y);

	if (result == background || result == handle) {
		return this; // All events should go to us instead
	} else {
		return result;
	}
}

void cVerticalScrollbar::onMouseDown(QMouseEvent *e) {
	QPoint pos = mapFromGlobal(e->pos());

	if (pos.y() >= handle->y() && pos.y() < handle->y() + handle->height()) {
		mouseDownY = pos.y() - handle->y();
		draggingHandle_ = true;
	}
}

void cVerticalScrollbar::onMouseUp(QMouseEvent *e) {
	draggingHandle_ = false;
}

void cVerticalScrollbar::onMouseMotion(int xrel, int yrel, QMouseEvent *e) {
	if (draggingHandle_) {
		int y = mapFromGlobal(e->pos()).y(); // Get the y position relative to our control
		y -= mouseDownY;
		int newpos = getPosFromTrackerY(y);
		setPos(newpos);
		//handle->setPosition(handle->x(), newy);
	}
}

unsigned int cVerticalScrollbar::getTrackerYFromPos(int pos) {
	if (pos == (int)min_) {
		return btnUp->height();
	} else if (pos == (int)max_) {
		return height_ - btnDown->height() - handle->height();
	} else {
		return btnUp->height() + getInnerHeight() * ((pos_ - min_) / qMax<float>(1, max_ - min_));
		//return btnUp->height() + (int)ceil(pos * pixelPerStep);
	}
}

unsigned int cVerticalScrollbar::getPosFromTrackerY(int y) {
	if (y < btnUp->height()) {
		return min_;
	} else if (y > height_ - btnDown->height() - handle->height()) {
		return max_;
	} else {
		return qMin<int>(max_, qMax<int>(min_, ((y - (int)btnUp->height()) / pixelPerStep)));
	}
}

void cVerticalScrollbar::onScroll(int oldpos) {
	emit scrolled(pos_);
}

void cVerticalScrollbar::setHandleId(ushort id) {
	if (handle) {
		handle->setId(id);
		handle->setY(getTrackerYFromPos(pos_));
		pixelPerStep = qMax<float>(0, (float)getInnerHeight() / (float)getValues());
	}
}

void cVerticalScrollbar::setBackgroundId(ushort id) {
	if (background) {
		background->setId(id);
	}
}

void cVerticalScrollbar::setUpButtonIds(ushort unpressed, ushort pressed, ushort hover) {
	if (btnUp) {
		btnUp->setStateGump(BS_UNPRESSED, unpressed);
		btnUp->setStateGump(BS_PRESSED, pressed);
		btnUp->setStateGump(BS_HOVER, hover);
		setBounds(x_, y_, btnUp->width(), height_); // Set Width before adding another button

		// Reposition background
		background->setBounds(0, btnUp->height(), btnUp->width(), height_ - btnUp->height() - btnDown->height());
		handle->setY(getTrackerYFromPos(pos_));
		pixelPerStep = qMax<float>(0, (float)getInnerHeight() / (float)getValues());
	}
}

void cVerticalScrollbar::setDownButtonIds(ushort unpressed, ushort pressed, ushort hover) {
	if (btnDown) {
		btnDown->setStateGump(BS_UNPRESSED, unpressed);
		btnDown->setStateGump(BS_PRESSED, pressed);
		btnDown->setStateGump(BS_HOVER, hover);
		btnDown->setPosition(0, height_ - btnDown->height());
		setBounds(x_, y_, btnDown->width(), height_); // Set Width before adding another button

		// Reposition background
		background->setBounds(0, btnUp->height(), btnUp->width(), height_ - btnUp->height() - btnDown->height());
		handle->setY(getTrackerYFromPos(pos_));
		pixelPerStep = qMax<float>(0, (float)getInnerHeight() / (float)getValues());
	}
}

void cVerticalScrollbar::onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight) {
	pixelPerStep = qMax<float>(0, (float)getInnerHeight() / (float)getValues());
	cContainer::onChangeBounds(oldx, oldy, oldwidth, oldheight);
}

void cVerticalScrollbar::processDefinitionAttribute(QString name, QString value) {
	if (name == "handle") {
		setHandleId(Utilities::stringToUInt(value));
	} else if (name == "background") {
		setBackgroundId(Utilities::stringToUInt(value));
	} else {
		cContainer::processDefinitionAttribute(name, value);
	}
}

void cVerticalScrollbar::processDefinitionElement(QDomElement element) {
	if (element.nodeName() == "upbutton") {
		ushort unpressed = Utilities::stringToUInt(element.attribute("unpressed"));
		ushort pressed = Utilities::stringToUInt(element.attribute("pressed", QString::number(unpressed)));
		ushort hover = Utilities::stringToUInt(element.attribute("hover", QString::number(pressed)));
		setUpButtonIds(unpressed, pressed, hover);
	} else if (element.nodeName() == "downbutton" && element.hasAttribute("unpressed")) {
		ushort unpressed = Utilities::stringToUInt(element.attribute("unpressed"));
		ushort pressed = Utilities::stringToUInt(element.attribute("pressed", QString::number(unpressed)));
		ushort hover = Utilities::stringToUInt(element.attribute("hover", QString::number(pressed)));
		setDownButtonIds(unpressed, pressed, hover);
	} else if (element.nodeName() == "range" && element.hasAttribute("min") && element.hasAttribute("max")) {
		int min = Utilities::stringToInt(element.attribute("min"));
		int max = Utilities::stringToInt(element.attribute("max"));
		setRange(min, max);
	} else {
		cContainer::processDefinitionElement(element);
	}
}
void cHorizontalScrollbar::scrollLeft(cControl *sender) {
	setPos(qMax<int>(min(), pos() - 1));
}

void cHorizontalScrollbar::scrollRight(cControl *sender) {
	setPos(qMin<int>(max(), pos() + 1));
}

cHorizontalScrollbar::cHorizontalScrollbar(int x, int y, uint width) : btnLeft(0), btnRight(0), handle(0) {
	btnLeft = new cImageButton(0, 0, 0xFA, 0xFB);
	setBounds(x, y, width, btnLeft->height()); // Set Width before adding another button
	connect(btnLeft, SIGNAL(onButtonPress(cControl*)), SLOT(scrollLeft(cControl*)));
	btnLeft->setPressRepeatRate(60);
	addControl(btnLeft);

	btnRight = new cImageButton(0, 0, 0xFC, 0xFD);
	btnRight->setPosition(width - btnRight->width(), 0);
	connect(btnRight, SIGNAL(onButtonPress(cControl*)), SLOT(scrollRight(cControl*)));
	btnRight->setPressRepeatRate(60);
	addControl(btnRight);

	background = new cBorderGump;
	background->setIds(0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100);
	background->setBounds(btnLeft->width(), 0, width - btnLeft->width() - btnRight->width(), btnLeft->height());
	addControl(background, true);

	handle = new cGumpImage(0xfe);
	handle->setPosition(btnLeft->width(), 0);
	addControl(handle);

	draggingHandle_ = false;

	pos_ = 0;
	setRange(0, 0);
}

cHorizontalScrollbar::~cHorizontalScrollbar() {
}

cControl *cHorizontalScrollbar::getControl(int x, int y) {
	cControl *result = cContainer::getControl(x, y);

	if (result == background || result == handle) {
		return this; // All events should go to us instead
	} else {
		return result;
	}
}

void cHorizontalScrollbar::onMouseDown(QMouseEvent *e) {
	QPoint pos = mapFromGlobal(e->pos());

	if (pos.x() >= handle->x() && pos.x() < handle->x() + handle->width()) {
		mouseDownX = pos.x() - handle->x();
		draggingHandle_ = true;
	}
}

void cHorizontalScrollbar::onMouseUp(QMouseEvent *e) {
	draggingHandle_ = false;
}

void cHorizontalScrollbar::onMouseMotion(int xrel, int yrel, QMouseEvent *e) {
	if (draggingHandle_) {
		int x = mapFromGlobal(e->pos()).x(); // Get the x position relative to our control
		x -= mouseDownX;
		int newpos = getPosFromTrackerX(x);
		setPos(newpos);
	}
}

unsigned int cHorizontalScrollbar::getTrackerXFromPos(int pos) {
	if (pos == (int)min_) {
		return btnLeft->width();
	} else if (pos == (int)max_) {
		return width_ - btnRight->width() - handle->width();
	} else {
		return btnLeft->width() + getInnerWidth() * ((pos_ - min_) / qMax<float>(1, max_ - min_));
	}
}

unsigned int cHorizontalScrollbar::getPosFromTrackerX(int x) {
	if (x < btnLeft->width()) {
		return min_;
	} else if (x > width_ - btnRight->width() - handle->width()) {
		return max_;
	} else {
		return qMin<int>(max_, qMax<int>(min_, min_ + ((x - (int)btnLeft->width()) / (float)getInnerWidth()) * getValues()));
	}
}

void cHorizontalScrollbar::onScroll(int oldpos) {
	emit scrolled(pos_);
}

void cHorizontalScrollbar::setHandleId(ushort id) {
	if (handle) {
		handle->setId(id);
		handle->setX(getTrackerXFromPos(pos_));
		pixelPerStep = qMax<float>(0, (float)getInnerWidth() / (float)getValues());
	}
}

void cHorizontalScrollbar::setBackgroundId(ushort id) {
	if (background) {
		background->setId(id);
	}
}

void cHorizontalScrollbar::setLeftButtonIds(ushort unpressed, ushort pressed, ushort hover) {
	if (btnLeft) {
		btnLeft->setStateGump(BS_UNPRESSED, unpressed);
		btnLeft->setStateGump(BS_PRESSED, pressed);
		btnLeft->setStateGump(BS_HOVER, hover);
		setBounds(x_, y_, width_, btnLeft->height()); // Set Width before adding another button

		// Reposition background
		background->setBounds(btnLeft->width(), 0, width_ - btnLeft->width() - btnRight->width(), btnLeft->height());
		handle->setX(getTrackerXFromPos(pos_));
		pixelPerStep = qMax<float>(0, (float)getInnerWidth() / (float)getValues());
	}
}

void cHorizontalScrollbar::setRightButtonIds(ushort unpressed, ushort pressed, ushort hover) {
	if (btnRight) {
		btnRight->setStateGump(BS_UNPRESSED, unpressed);
		btnRight->setStateGump(BS_PRESSED, pressed);
		btnRight->setStateGump(BS_HOVER, hover);
		btnRight->setPosition(0, height_ - btnRight->height());
		setBounds(x_, y_, width_, btnRight->height()); // Set Width before adding another button

		// Reposition background
		background->setBounds(btnLeft->width(), 0, width_ - btnLeft->width() - btnRight->width(), btnLeft->height());
		handle->setX(getTrackerXFromPos(pos_));
		pixelPerStep = qMax<float>(0, (float)getInnerWidth() / (float)getValues());
	}
}

void cHorizontalScrollbar::onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight) {
	if (btnRight && btnLeft && background && handle) {
		btnRight->setX(width_ - btnRight->width());
		pixelPerStep = qMax<float>(0, (float)getInnerWidth() / (float)getValues());
		handle->setX(getTrackerXFromPos(pos_));		
		background->setBounds(btnLeft->width(), 0, width_ - btnLeft->width() - btnRight->width(), btnLeft->height());
	}

	pixelPerStep = qMax<float>(0, (float)getInnerWidth() / (float)getValues());
	cContainer::onChangeBounds(oldx, oldy, oldwidth, oldheight);
}

void cHorizontalScrollbar::processDefinitionAttribute(QString name, QString value) {
	if (name == "handle") {
		setHandleId(Utilities::stringToUInt(value));
	} else if (name == "background") {
		setBackgroundId(Utilities::stringToUInt(value));
	} else {
		cContainer::processDefinitionAttribute(name, value);
	}
}

void cHorizontalScrollbar::processDefinitionElement(QDomElement element) {
	if (element.nodeName() == "leftbutton") {
		ushort unpressed = Utilities::stringToUInt(element.attribute("unpressed"));
		ushort pressed = Utilities::stringToUInt(element.attribute("pressed", QString::number(unpressed)));
		ushort hover = Utilities::stringToUInt(element.attribute("hover", QString::number(pressed)));
		setLeftButtonIds(unpressed, pressed, hover);
	} else if (element.nodeName() == "rightbutton" && element.hasAttribute("unpressed")) {
		ushort unpressed = Utilities::stringToUInt(element.attribute("unpressed"));
		ushort pressed = Utilities::stringToUInt(element.attribute("pressed", QString::number(unpressed)));
		ushort hover = Utilities::stringToUInt(element.attribute("hover", QString::number(pressed)));
		setRightButtonIds(unpressed, pressed, hover);
	} else if (element.nodeName() == "range" && element.hasAttribute("min") && element.hasAttribute("max")) {
		int min = Utilities::stringToInt(element.attribute("min"));
		int max = Utilities::stringToInt(element.attribute("max"));
		setRange(min, max);
	} else if (element.nodeName() == "background" && element.hasAttribute("id1")) {
		ushort id1 = Utilities::stringToUInt(element.attribute("id1"));
		ushort id2 = Utilities::stringToUInt(element.attribute("id2", QString::number(id1)));
		ushort id3 = Utilities::stringToUInt(element.attribute("id3", QString::number(id1)));
		ushort id4 = Utilities::stringToUInt(element.attribute("id4", QString::number(id1)));
		ushort id5 = Utilities::stringToUInt(element.attribute("id5", QString::number(id1)));
		ushort id6 = Utilities::stringToUInt(element.attribute("id6", QString::number(id1)));
		ushort id7 = Utilities::stringToUInt(element.attribute("id7", QString::number(id1)));
		ushort id8 = Utilities::stringToUInt(element.attribute("id8", QString::number(id1)));
		ushort id9 = Utilities::stringToUInt(element.attribute("id9", QString::number(id1)));
		background->setIds(id1, id2, id3, id4, id5, id6, id7, id8, id9);
	} else {
		cContainer::processDefinitionElement(element);
	}
}
