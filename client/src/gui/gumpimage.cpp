
#include "exceptions.h"
#include "utilities.h"
#include "muls/gumpart.h"
#include "gui/gumpimage.h"

void cGumpImage::update() {
	if (texture) {
		texture->decref();
	}

	texture = Gumpart->readTexture(id_, hue_, partialHue_);
	if (autoSize_) {
		width_ = texture->realWidth();
		height_ = texture->realHeight();
	}

	dirty = false;
}

cGumpImage::cGumpImage(unsigned short id, unsigned short hue, bool partialHue, bool autoSize) {	
	id_ = id;
	hue_ = hue;
	partialHue_ = partialHue;
	autoSize_ = autoSize;
	texture = 0;	
	moveHandle_ = true;
	update();
}

cGumpImage::~cGumpImage() {
	if (texture) {
		texture->decref();
	}
}

void cGumpImage::draw(int xoffset, int yoffset) {
	if (dirty) {
		update();
	}

	if (texture) {
		texture->draw(xoffset + x_, yoffset + y_);
	}
}
