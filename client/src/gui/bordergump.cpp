
#include "engine.h"
#include "gui/bordergump.h"
#include "gui/tiledgumpimage.h"
#include "gui/gumpimage.h"
#include "muls/gumpart.h"

cBorderGump::cBorderGump(unsigned short id, unsigned short hue) {
	moveHandle_ = true;
	setGump(id, hue, true);
}

cBorderGump::~cBorderGump() {
}

void cBorderGump::setGump(unsigned short id, unsigned short hue, bool forceupdate) {
	if (forceupdate || this->id != id || this->hue != hue) {
		clear(); // Clear all children

		this->id = id;
		this->hue = hue;

		// Upper Left
		uleft = new cGumpImage(id, hue);
		uleft->update();
		addControl(uleft);

		// Upper Right
		uright = new cGumpImage(id + 2, hue);
		uright->update();
		addControl(uright);

		// Lower Left
		lleft = new cGumpImage(id + 6, hue);
		lleft->update();
		addControl(lleft);

		// Lower Right
		lright = new cGumpImage(id + 8, hue);
		lright->update();
		addControl(lright);

		// Top
		top = new cTiledGumpImage(id + 1, hue);
		top->update();
		addControl(top);

		// Left
		left = new cTiledGumpImage(id + 3, hue);
		left->update();
		addControl(left);

		// Right
		right = new cTiledGumpImage(id + 5, hue);
		right->update();
		addControl(right);

		// Bottom
		bottom = new cTiledGumpImage(id + 7, hue);
		bottom->update();
		addControl(bottom);

		// Center
		center = new cTiledGumpImage(id + 4, hue);
		center->update();
		addControl(center);
        
		alignControls(); // Align the controls
	}
}

/*
	Change the position of the gump controls.
*/
void cBorderGump::onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight) {
	if (oldwidth != width_ || oldheight != height_) {
		alignControls();
	}
}

/*
	Align the parts of this bordergump
*/
void cBorderGump::alignControls() {
	uleft->setPosition(0, 0);
	uright->setPosition(width_ - uright->width(), 0);
	lleft->setPosition(0, height_ - lleft->height());
	lright->setPosition(width_ - lright->width(), height_ - lright->height());
	top->setBounds(uleft->width(), 0, width_ - uleft->width() - uright->width(), uleft->height());
	left->setBounds(0, uleft->height(), uleft->width(), height_ - uleft->height() - lleft->height());
	right->setBounds(width_ - uright->width(), uright->height(), uright->width(), height_ - uright->height() - lright->height());
	bottom->setBounds(lleft->width(), height_ - lleft->height(), width_ - lleft->width() - lright->width(), lleft->height());
	center->setBounds(uleft->width(), uleft->height(), width_ - uleft->width() - uright->width(), height_ - uleft->height() - lleft->height());
}
