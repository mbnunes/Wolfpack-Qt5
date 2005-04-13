
#include "exceptions.h"
#include "gui/container.h"
#include "gui/label.h"
#include "muls/unicodefonts.h"
#include "muls/asciifonts.h"

cLabel::cLabel(const QString &text, unsigned short font, unsigned short hue, bool border, enTextAlign align, bool autoSize) {
	surface = 0;
	text_ = text;
	font_ = font;
	hue_ = hue;
	border_ = border;
	align_ = align;
	autoSize_ = autoSize;
	exactHitTest_ = false;
}

cLabel::~cLabel() {
	if (surface) {
		surface->decref();
	}
}

void cLabel::update() {
	if (surface) {
		surface->decref();
	}

	surface = UnicodeFonts->buildText(font_, text_, hue_, false, border_, align_);

	if (autoSize_ && surface) {
		setSize(surface->realWidth(), surface->realHeight());
	} else if (!surface) {
		setSize(0, 0);
	}
}

void cLabel::draw(int xoffset, int yoffset) {
	if (!surface && !text_.isEmpty()) {
		update();
	}

	if (surface) {
		// TODO: Clipping if width_/heigh_ < realWidth/realHeight
		surface->draw(x_ + xoffset, y_ + yoffset);
	}
}

cControl *cLabel::getControl(int x, int y) {
	cControl *result = 0;

	if (x >= 0 && y >= 0 && x < width_ && y < height_) {
		if (exactHitTest_) {
			if (surface && surface->hitTest(x, y)) {
				result = this;
			}
		} else {
			result = this;
		}
	}

	return result;
}
