
#include "gui/window.h"
#include "texture.h"

cWindow::cWindow() : cContainer() {
	tracking = false;
	movable_ = false;
	closable_ = false;
}

cWindow::~cWindow() {
}

bool cWindow::isWindow() const {
	return true;
}

void cWindow::update() {
}

// Draw this window
void cWindow::draw(int xoffset, int yoffset) {
	/*
		For performance we could generate a display list here.
	*/
	cContainer::draw(xoffset, yoffset);
}

// This function has to be overriden for dragging the window around to work
cControl *cWindow::getControl(int x, int y) {
	cControl *control = cContainer::getControl(x, y);

	if (control && control != this && isMovable() && control->isMoveHandle()) {
		return this;
	}

	return control;
}

void cWindow::onMouseDown(QMouseEvent *e) {
	if (isMovable()) {
		tracking = true;
	}
}

void cWindow::onMouseUp(QMouseEvent *e) {
	tracking = false;
}

void cWindow::onMouseMotion(int xrel, int yrel, QMouseEvent *e) {
	if (tracking) {
		x_ += xrel;
		y_ += yrel;
	}
}
