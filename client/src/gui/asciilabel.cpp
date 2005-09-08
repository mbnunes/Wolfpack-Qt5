
#include "exceptions.h"
#include "gui/container.h"
#include "gui/asciilabel.h"
#include "muls/asciifonts.h"
//Added by qt3to4:
#include <QString>

cAsciiLabel::cAsciiLabel(const QString &text, unsigned short font, unsigned short hue, enTextAlign align, bool autoSize, bool hueAll) {
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
}

void cAsciiLabel::draw(int xoffset, int yoffset) {
	if (!texture && !text_.isEmpty()) {
		update();
	}

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

void cAsciiLabel::processDefinitionAttribute(QString name, QString value) {
	if (name == "text") {
		setText(value);
	} else if (name == "font") {
		setFont(Utilities::stringToUInt(value));
	} else if (name == "hue") {
		setHue(Utilities::stringToUInt(value));
	} else if (name == "hueall") {
		setHueAll(Utilities::stringToBool(value));
	} else if (name == "autosize") {
		setAutoSize(Utilities::stringToBool(value));
	} else if (name == "textalign") {
		if (value == "left") {
			setAlign(ALIGN_LEFT);
		} else if (value == "center") {
			setAlign(ALIGN_CENTER);
		} else if (value == "right") {
			setAlign(ALIGN_RIGHT);
		}
	} else {
		cControl::processDefinitionAttribute(name, value);
	}
}

void cAsciiLabel::processDefinitionElement(QDomElement element) {
	if (element.tagName() == "text") {
		setText(element.text());
	} else {
		cControl::processDefinitionElement(element);
	}
}
