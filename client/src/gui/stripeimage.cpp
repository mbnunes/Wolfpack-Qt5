
#include "gui/stripeimage.h"
#include "utilities.h"

cStripeImage::cStripeImage() {
	moveHandle_ = true;
	id_ = 0;
	hue_ = 0;
	partialHue_ = false;
	dirty = true;
	vertical_ = false;

	left = 0;
	right = 0;
	center = 0;
}

cStripeImage::~cStripeImage() {
}

void cStripeImage::update() {
	clear(); // Clear all children

	if (id_ == 0) {
		return;  // Invalid id
	}

	// Left
	left = new cGumpImage(id_, hue_, partialHue_);
	left->update();
	addControl(left);

	// Right
	right = new cGumpImage(id_ + 2, hue_, partialHue_);
	right->update();
	addControl(right);

	// Center
	center = new cTiledGumpImage(id_ + 1, hue_, partialHue_);
	center->update();
	addControl(center);

	dirty = false; // Flag as updated
	alignControls(); // Align the controls
}

/*
	Change the position of the gump controls.
*/
void cStripeImage::onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight) {
	if (oldwidth != width_ || oldheight != height_) {
		alignControls();
	}
}

/*
	Align the parts of this bordergump
*/
void cStripeImage::alignControls() {
	if (dirty) {
		return;
	}

	if (!vertical_) {
		left->setPosition(0, 0);
		center->setBounds(left->width(), 0, width_ - left->width() - right->width(), height_);
		right->setPosition(width_ - right->width(), 0);
	} else {
		left->setPosition(0, 0);
		center->setBounds(0, left->height(), width_, height_ - left->height() - right->height());
		right->setPosition(0, height_ - right->height());
	}
}

void cStripeImage::draw(int xoffset, int yoffset) {
	if (dirty) {
		update();
	}

	if (left)
		left->setAlpha(alpha_);
	if (right)
		right->setAlpha(alpha_);
	if (center)
		center->setAlpha(alpha_);

	cContainer::draw(xoffset, yoffset);
}

void cStripeImage::processDefinitionAttribute(QString name, QString value) {
	if (name == "gump") {
		setId(Utilities::stringToUInt(value));
	} else if (name == "hue") {
		setHue(Utilities::stringToUInt(value));
	} else if (name == "partialhue") {
		setPartialHue(Utilities::stringToBool(value));
	} else if (name == "vertical") {
		setVertical(Utilities::stringToBool(value));
	} else {
		return cControl::processDefinitionAttribute(name, value);
	}
}
