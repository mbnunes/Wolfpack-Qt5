
#include "gui/window.h"
#include "gui/gui.h"
#include "texture.h"
#include "utilities.h"

#include <QMouseEvent>
#include <qgl.h>

cWindow::cWindow() : cContainer() {
	tracking = false;
	movable_ = false;
	closable_ = false;
	enableStencil_ = false;
}

cWindow::~cWindow() {
	if (Gui->activeWindow() == this) {
		Gui->setActiveWindow(0);
	}
}

bool cWindow::isWindow() const {
	return true;
}

void cWindow::update() {
}

// Draw this window
void cWindow::draw(int xoffset, int yoffset) {
	if (enableStencil_) {
		glClear(GL_STENCIL_BUFFER_BIT);
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glStencilFunc(GL_NOTEQUAL, 0, 0xff);
	}

	/*
		For performance we could generate a display list here.
	*/
	cContainer::draw(xoffset, yoffset);

	if (enableStencil_) {
		glDisable(GL_STENCIL_TEST);
	}
}

// This function has to be overriden for dragging the window around to work
cControl *cWindow::getControl(int x, int y) {
	cControl *control = cContainer::getControl(x, y);

	if (control && control != this && (isMovable() || isClosable()) && control->isMoveHandle()) {
		return this;
	}

	return control;
}

void cWindow::onMouseDown(QMouseEvent *e) {
	if (isMovable() && e->button() == Qt::LeftButton) {
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

void cWindow::processDefinitionAttribute(QString name, QString value) {
	if (name == "stencil") {
		enableStencil_ = Utilities::stringToBool(value);
	} else if (name == "closable") {
		closable_ = Utilities::stringToBool(value);
	} else if (name == "movable") {
		movable_ = Utilities::stringToBool(value);
	} else {
		cContainer::processDefinitionAttribute(name, value);
	}
}

void cWindow::onClick(QMouseEvent *e) {
	// Right mouse button closes the container
	if (isClosable() && e->button() == Qt::RightButton) {
		Gui->queueDelete(this);
	} else {
		cContainer::onClick(e);
	}
}
