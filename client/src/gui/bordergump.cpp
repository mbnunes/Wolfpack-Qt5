
#include "gui/bordergump.h"
#include "gui/tiledgumpimage.h"
#include "gui/gumpimage.h"
#include "muls/gumpart.h"

cBorderGump::cBorderGump() {
	moveHandle_ = true;
	id_ = 0;
	hue_ = 0;
	partialHue_ = false;
	dirty = true;

	left = 0;
	right = 0;
	top = 0;
	bottom = 0;
	center = 0;
	uleft = 0;
	uright = 0;
	lleft = 0;
	lright = 0;
}

cBorderGump::cBorderGump(unsigned short id, unsigned short hue, bool partialHue) {
	moveHandle_ = true;
	id_ = id;
	hue_ = hue;
	partialHue_ = partialHue;
	dirty = true;
}

cBorderGump::~cBorderGump() {
}

void cBorderGump::update() {
	clear(); // Clear all children

	if (id_ == 0) {
		return;  // Invalid id
	}

	// Upper Left
	uleft = new cGumpImage(id_, hue_);
	uleft->update();
	addControl(uleft);

	// Upper Right
	uright = new cGumpImage(id_ + 2, hue_);
	uright->update();
	addControl(uright);

	// Lower Left
	lleft = new cGumpImage(id_ + 6, hue_);
	lleft->update();
	addControl(lleft);

	// Lower Right
	lright = new cGumpImage(id_ + 8, hue_);
	lright->update();
	addControl(lright);

	// Top
	top = new cTiledGumpImage(id_ + 1, hue_);
	top->update();
	addControl(top);

	// Left
	left = new cTiledGumpImage(id_ + 3, hue_);
	left->update();
	addControl(left);

	// Right
	right = new cTiledGumpImage(id_ + 5, hue_);
	right->update();
	addControl(right);

	// Bottom
	bottom = new cTiledGumpImage(id_ + 7, hue_);
	bottom->update();
	addControl(bottom);

	// Center
	center = new cTiledGumpImage(id_ + 4, hue_);
	center->update();
	addControl(center);

	dirty = false; // Flag as updated
	alignControls(); // Align the controls
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
	if (dirty) {
		return;
	}

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

void cBorderGump::draw(int xoffset, int yoffset) {
	if (dirty) {
		update();
	}

	if (left)
		left->setAlpha(alpha_);
	if (right)
		right->setAlpha(alpha_);
	if (top)
		top->setAlpha(alpha_);
	if (bottom)
		bottom->setAlpha(alpha_);
	if (center)
		center->setAlpha(alpha_);
	if (uleft)
		uleft->setAlpha(alpha_);
	if (uright)
		uright->setAlpha(alpha_);
	if (lleft)
		lleft->setAlpha(alpha_);
	if (lright)
		lright->setAlpha(alpha_);

	cContainer::draw(xoffset, yoffset);
}

void cBorderGump::processDefinitionAttribute(QString name, QString value) {
	if (name == "gump") {
		setId(Utilities::stringToUInt(value));
	} else if (name == "hue") {
		setHue(Utilities::stringToUInt(value));
	} else if (name == "partialhue") {
		setPartialHue(Utilities::stringToBool(value));
	} else {
		return cControl::processDefinitionAttribute(name, value);
	}
}
