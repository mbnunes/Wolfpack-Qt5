
#include "exceptions.h"
#include "engine.h"
#include "utilities.h"
#include "muls/gumpart.h"
#include "gui/gumpimage.h"

void cGumpImage::update() {
	if (!texture) {
		texture = Gumpart->readTexture(id_, hue_, partialHue_);
		if (autoSize_) {
			width_ = texture->realWidth();
			height_ = texture->realHeight();
		}
		texture->incref();
	}
	dirty_ = false;
}

cGumpImage::cGumpImage(unsigned short id, unsigned short hue, bool partialHue, bool autoSize) {	
	id_ = id;
	hue_ = hue;
	partialHue_ = partialHue;
	autoSize_ = autoSize;
	texture = 0;
	update();
	moveHandle_ = true;
}

cGumpImage::~cGumpImage() {
	if (texture) {
		texture->decref();
	}
}

void cGumpImage::draw(int xoffset, int yoffset) {
	if (isDirty()) {
		update();
	}

	if (texture) {
		texture->draw(xoffset + x_, yoffset + y_);
	}
}
