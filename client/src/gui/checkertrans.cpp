
#include "gui/checkertrans.h"
#include "surface.h"
#include "utilities.h"
#include <qgl.h>

cCheckerTrans::cCheckerTrans(bool enabled) {
	enabled_ = enabled;
	checkerboard = 0;
}

cCheckerTrans::~cCheckerTrans() {
	if (checkerboard) {
		checkerboard->decref();
	}
}

void cCheckerTrans::processDefinitionAttribute(QString name, QString value) {
	if (name == "enabled") {
		enabled_ = Utilities::stringToBool(value);
	} else {
		cControl::processDefinitionAttribute(name, value);
	}
}

void cCheckerTrans::generateCheckerboard() {
	// Generate the checkerboard texture if not present yet
	if (!checkerboard) {
		cSurface *surface = new cSurface(width_, height_, true);
		int black = surface->color(0, 0, 0, 0);
		int white = surface->color(255, 255, 255, 255);
		for (int x = 0; x < width_; ++x) {
			for (int y = 0; y < height_; ++y) {
				if (y % 2 == 0) {
					if (x % 2 == 0) {
						surface->setPixel(x, y, white);
					} else {
						surface->setPixel(x, y, black);
					}
				} else {
					if (x % 2 == 0) {
						surface->setPixel(x, y, black);
					} else {
						surface->setPixel(x, y, white);
					}
				}				
			}
		}
		checkerboard = new cTexture(surface, false);
		delete surface;
	}
}

void cCheckerTrans::draw(int xoffset, int yoffset) {
	xoffset += x_;
	yoffset += y_;

	// Simply write to the stencil buffer
	glDisable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_EQUAL, 1.0f);
	if (enabled_) {
		glStencilFunc(GL_NEVER, 0, 1);
	} else {
		glStencilFunc(GL_NEVER, 1, 1);
	}
	glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);

	generateCheckerboard(); // generate the checkerboard texture if neccessary
	checkerboard->draw(xoffset, yoffset);

	glDisable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);

	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_NOTEQUAL, 0, 0xff);
}

void cCheckerTrans::onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight) {
	if (checkerboard && (oldwidth != width_ || oldheight != height_)) {
		checkerboard->decref();
		checkerboard = 0;
	}
}
