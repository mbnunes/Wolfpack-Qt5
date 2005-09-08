
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
	alpha_ = 1.0f;
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
		texture->draw(xoffset + x_, yoffset + y_, alpha_);
	}
}

void cGumpImage::processDefinitionAttribute(QString name, QString value) {
	if (name == "gump") {
		setId(Utilities::stringToUInt(value));
	} else if (name == "hue") {
		setHue(Utilities::stringToUInt(value));
	} else if (name == "partialhue") {
		setPartialHue(Utilities::stringToBool(value));
	} else if (name == "autosize") {
		setAutoSize(Utilities::stringToBool(value));
	} else {
		cControl::processDefinitionAttribute(name, value);
	}
}
