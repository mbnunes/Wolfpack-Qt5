
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
	parentBounds_.x = 0;
	parentBounds_.y = 0;
	parentBounds_.w = 0;
	parentBounds_.h = 0;
	align_ = CA_NONE;
	anchors_ = 0;
	movable_ = false;
	moveHandle_ = false;
	dirty_ = true; // We haven't drawn anything yet
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
	parentBounds_.x += x_ - data;
	onChangeBounds(oldx, y_, width_, height_);
}

void cControl::setY(int data) {	
	int oldy = y_;	
	y_ = data;
	parentBounds_.y += y_ - data;
	onChangeBounds(x_, oldy, width_, height_);
}

void cControl::setWidth(int data) {
	int oldwidth = width_;
	width_ = data;
	parentBounds_.w = data;
	onChangeBounds(x_, y_, oldwidth, height_);
}

void cControl::setHeight(int data) {
	int oldheight = height_;
	height_ = data;
	parentBounds_.h = data;
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
	parentBounds_.h = height;
	parentBounds_.w = width;
	parentBounds_.x += x_ - oldx;
	parentBounds_.y += y_ - oldy;
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
	invalidate();

	if (!data) {
		if (Gui->activeWindow() == this) {
			Gui->setActiveWindow(0);
		}
		if (Gui->inputFocus() == this) {
			Gui->setInputFocus(0);
		}
	}
}

void cControl::onMouseDown(int x, int y, unsigned char button, bool pressed) {
}

void cControl::onMouseUp(int x, int y, unsigned char button, bool pressed) {
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

// This method is called by the mainloop to ensure
// every control is updated correctly (->valid)
void cControl::update() {
	dirty_ = false;
}

void cControl::onMouseEnter() {
}

void cControl::onMouseLeave() {
}

void cControl::onBlur(cControl *newFocus) {
	invalidate();
}

void cControl::onFocus(cControl *oldFocus) {
	invalidate();
}

void cControl::onKeyDown(const SDL_keysym &key) {
}

void cControl::onKeyUp(const SDL_keysym &key) {
}

bool cControl::isContainer() const {
	return false;
}

bool cControl::isWindow() const {
	return false;
}

void cControl::onMouseMotion(int xrel, int yrel, unsigned char state) {
}

void cControl::draw(int xoffset, int yoffset) {
}

void cControl::draw(IPaintable *target, const SDL_Rect *clipping) {
}
