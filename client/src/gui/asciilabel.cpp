
#include "exceptions.h"
#include "engine.h"
#include "gui/container.h"
#include "gui/asciilabel.h"
#include "muls/asciifonts.h"

cAsciiLabel::cAsciiLabel(const QCString &text, unsigned short font, unsigned short hue, enTextAlign align, bool autoSize, bool hueAll) {
	text_ = text;
	font_ = font;
	hue_ = hue;
	align_ = align;
	autoSize_ = autoSize;
	hueAll_ = hueAll;

	texture = AsciiFonts->buildText(font_, text_, hue_, false, align_, hueAll_); // Build in advance because of autosizing

	if (autoSize_) {
		setSize(texture->realWidth(), texture->realHeight());
	}

	dirty_ = false;
}

cAsciiLabel::~cAsciiLabel() {
	if (texture) {
		texture->decref();
	}
}

void cAsciiLabel::update() {
	if (texture) {
		texture->decref();
	}

	texture = AsciiFonts->buildText(font_, text_, hue_, false, align_, hueAll_); // Build in advance because of autosizing		

	if (autoSize_) {
		setSize(texture->realWidth(), texture->realHeight());
	}

	dirty_ = false;
}

void cAsciiLabel::draw(int xoffset, int yoffset) {
	if (texture) {
		int x = x_;
		int y = y_;

		if (align_ == ALIGN_RIGHT) {
			x = x_ + width_ - texture->realWidth();
		} else if (align_ == ALIGN_CENTER) {
			x = x_ + (width_ - texture->realWidth()) / 2;
		}

        texture->draw(xoffset + x, yoffset + y);		
	}
}
