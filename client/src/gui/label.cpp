
#include "exceptions.h"
#include "gui/container.h"
#include "gui/label.h"
#include "muls/unicodefonts.h"
#include "muls/asciifonts.h"
#include "muls/localization.h"

cLabel::cLabel() {
	surface = 0;
	font_ = 0;
	hue_ = 0x3b2;
	border_ = true;
	align_ = ALIGN_LEFT;
	autoSize_ = true;
	exactHitTest_ = false;
	htmlMode_ = false;
	borderColor_ = 0;
	emboss_ = false;
}

cLabel::cLabel(const QString &text, unsigned short font, unsigned short hue, bool border, enTextAlign align, bool autoSize) {
	surface = 0;
	text_ = text;
	font_ = font;
	hue_ = hue;
	border_ = border;
	align_ = align;
	autoSize_ = autoSize;
	exactHitTest_ = false;
	htmlMode_ = false;
	borderColor_ = 0;
	emboss_ = false;
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

	if (autoSize_ || width_ == 0) {
		surface = UnicodeFonts->buildText(font_, text_, hue_, false, border_, align_, htmlMode_, borderColor_, emboss_);
	} else {
		surface = UnicodeFonts->buildTextWrapped(font_, text_, width_, hue_, false, border_, align_, htmlMode_, borderColor_, emboss_);
	}

	if (autoSize_ && surface) {
		setSize(surface->realWidth(), surface->realHeight());
	} else if (!autoSize_ && surface) {
		if (!height_) {
			setHeight(surface->realHeight());
		}
		if (!width_) {
			setWidth(surface->realWidth());
		}
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
		surface->draw(x_ + xoffset, y_ + yoffset, alpha_);
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

void cLabel::processDefinitionAttribute(QString name, QString value) {
	if (name == "text") {
		setText(value);
	} else if (name == "font") {
		setFont(Utilities::stringToUInt(value));
	} else if (name == "hue") {
		setHue(Utilities::stringToUInt(value));
	} else if (name == "autosize") {
		setAutoSize(Utilities::stringToBool(value));
	} else if (name == "exacthittest") {
		setExactHitTest(Utilities::stringToBool(value));
	} else if (name == "border") {
		setBorder(Utilities::stringToBool(value));
	} else if (name == "htmlmode") {
		setHtmlMode(Utilities::stringToBool(value));
	} else if (name == "emboss") {
		setEmboss(Utilities::stringToBool(value));
	} else if (name == "textalign") {
		if (value == "left") {
			setAlign(ALIGN_LEFT);
		} else if (value == "center") {
			setAlign(ALIGN_CENTER);
		} else if (value == "right") {
			setAlign(ALIGN_RIGHT);
		}
	} else if (name == "bordercolor") {
		uint color = Utilities::stringToUInt(value);
		if (color != 0) {
			borderColor_ = cSurface::color((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
		} else {
			borderColor_ = 0;
		}
	} else if (name == "localized") {
		setText(Localization->get(Utilities::stringToUInt(value)));
	} else {
		cControl::processDefinitionAttribute(name, value);
	}
}

void cLabel::processDefinitionElement(QDomElement element) {
	if (element.tagName() == "text") {
		setText(element.text());
	} else {
		cControl::processDefinitionElement(element);
	}
}
