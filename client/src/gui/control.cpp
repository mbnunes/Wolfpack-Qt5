
#include "exceptions.h"
#include "utilities.h"
#include "gui/container.h"
#include "gui/control.h"
#include "gui/gui.h"

cControl::cControl() {
	visible_ = true;
	needsRealign_ = false;
	disableAlign_ = false;
	x_ = 0;
	y_ = 0;
	width_ = 0;
	height_ = 0;
	parent_ = 0;
	align_ = CA_NONE;
	anchors_ = 0;
	movable_ = false;
	moveHandle_ = false;
	canHaveFocus_ = false;
	wantTabs_ = false;
	tabIndex_ = 0;
}

cControl::~cControl() {
}

void cControl::setAlign(enControlAlign align) {
	enControlAlign oldalign = align_;
	align_ = align;
	requestAlign();
}

void cControl::setX(int data) {
	int oldx = x_;	
	x_ = data;
	onChangeBounds(oldx, y_, width_, height_);
}

void cControl::setY(int data) {	
	int oldy = y_;	
	y_ = data;
	onChangeBounds(x_, oldy, width_, height_);
}

void cControl::setWidth(int data) {
	int oldwidth = width_;
	width_ = data;
	onChangeBounds(x_, y_, oldwidth, height_);
}

void cControl::setHeight(int data) {
	int oldheight = height_;
	height_ = data;
	onChangeBounds(x_, y_, width_, oldheight);
}

void cControl::setBounds(int x, int y, int width, int height) {
	int oldheight = height_;
	int oldwidth = width_;
	int oldx = x_;
	int oldy = y_;
	x_ = x;
	y_ = y;
	width_ = width;
	height_ = height;
	onChangeBounds(oldx, oldy, oldwidth, oldheight);
}

void cControl::onParentMoved(int oldx, int oldy) {
}

void cControl::setParent(cContainer *data) {
	int oldx = parent_ ? parent_->x() : 0;
	int oldy = parent_ ? parent_->y() : 0;
	parent_ = data;	
	requestAlign();
	//adjustSize();
}

void cControl::onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight) {
	requestAlign();
}

void cControl::requestAlign() {
	if (parent_) {
		parent_->alignControl(this);
	}
}

void cControl::onParentResized(int oldwidth, int oldheight) {
}

void cControl::setVisible(bool data) {
	visible_ = data;
	requestAlign();

	if (!data) {
		if (Gui->activeWindow() == this) {
			Gui->setActiveWindow(0);
		}
		if (Gui->inputFocus() == this) {
			Gui->setInputFocus(0);
		}
	}
}

void cControl::onMouseDown(QMouseEvent *e) {
}

void cControl::onMouseUp(QMouseEvent *e) {
}

cControl *cControl::getControl(int x, int y) {
	if (visible_ && x >= 0 && y >= 0 && x < width_ && y < height_) {
		return this;
	} else {
		return 0;
	}
}

cControl *cControl::getMovableControl() {
	if (isMovable()) {
		return this;
	} else if (parent_) {
		return parent_->getMovableControl();
	} else {
		return 0;
	}
}

void cControl::onMouseEnter() {
}

void cControl::onMouseLeave() {
}

void cControl::onBlur(cControl *newFocus) {
}

void cControl::onFocus(cControl *oldFocus) {
}

void cControl::onKeyDown(QKeyEvent *e) {
}

void cControl::onKeyUp(QKeyEvent *e) {
}

bool cControl::isContainer() const {
	return false;
}

bool cControl::isWindow() const {
	return false;
}

void cControl::onMouseMotion(int xrel, int yrel, QMouseEvent *e) {
}

void cControl::draw(int xoffset, int yoffset) {
}

QPoint cControl::mapFromGlobal(const QPoint &point) {
	cControl *parent = parent_;
	int x = x_, y = y_;
	while (parent) {
		x += parent->x();
		y += parent->y();
		parent = parent->parent();
	}

    return QPoint(point.x() - x, point.y() - y);
}
