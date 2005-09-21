
#include <qglobal.h>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>

#include "gui/checkbox.h"
#include "muls/gumpart.h"
#include "utilities.h"

cCheckbox::cCheckbox() {
	mouseOver_ = false;
	mouseHolding_ = false;
	spaceHolding_ = false;
	canHaveFocus_ = true; // Buttons can have the input focus
	checked_ = false;

	width_ = 0;
	height_ = 0;

	gumps[Unchecked] = 0;	
	gumps[Hover] = 0;
	gumps[Pressed] = 0;
	gumps[Checked] = 0;
	gumps[CheckedHover] = 0;	
}

cCheckbox::~cCheckbox() {
	for (int i = 0; i < 5; ++i) {
		if (gumps[i]) {
			gumps[i]->decref();
		}
	}
}

void cCheckbox::onMouseLeave() {
	mouseOver_ = false;
}

void cCheckbox::onMouseEnter() {
	mouseOver_ = true;
}

void cCheckbox::setStateGump(State state, unsigned short id, unsigned short hue, bool partialHue) {
	if (gumps[state]) {
		gumps[state]->decref();
	}

	gumps[state] = Gumpart->readTexture(id, hue, partialHue);

	if (gumps[Unchecked]) {
		width_ = gumps[Unchecked]->realWidth();
		height_ = gumps[Unchecked]->realHeight();
	}
}

cControl *cCheckbox::getControl(int x, int y) {
	cTexture *texture = gumps[getState()];

	if (texture && texture->hitTest(x, y)) {
		return this;
	}

	return 0;
}

void cCheckbox::draw(int xoffset, int yoffset) {
	cTexture *texture = gumps[getState()];

	if (texture) {
		texture->draw(xoffset + x_, yoffset + y_);
	}
}

void cCheckbox::onMouseDown(QMouseEvent *e) {
	if (e->button() == Qt::LeftButton) {
		mouseHolding_ = true;
	}
}

void cCheckbox::onMouseUp(QMouseEvent *e) {
	if (e->button() == Qt::LeftButton) {
		mouseHolding_ = false;
		if (mouseOver_) {
			setChecked(!checked());
			emit stateChanged();
		}
	}
}

void cCheckbox::onKeyDown(QKeyEvent *e) {
	if (e->key() == Qt::Key_Space) {
		spaceHolding_ = true;
	}
}

void cCheckbox::onClick(QMouseEvent *e) {
}

void cCheckbox::onKeyUp(QKeyEvent *e) {
	if (e->key() == Qt::Key_Space) {
		spaceHolding_ = false;
		setChecked(!checked());
		emit stateChanged(); // Issue the click event
	}
}

void cCheckbox::onBlur(cControl *newFocus) {
	if (spaceHolding_) {
		spaceHolding_ = false;
	}
	cControl::onBlur(newFocus);
}

void cCheckbox::processDefinitionAttribute(QString name, QString value) {
	if (name == "unchecked") {
		setStateGump(Unchecked, Utilities::stringToUInt(value));
	} else if (name == "pressed") {
		setStateGump(Pressed, Utilities::stringToUInt(value));
	} else if (name == "hover") {
		setStateGump(Hover, Utilities::stringToUInt(value));
	} else if (name == "checked") {
		setStateGump(Checked, Utilities::stringToUInt(value));
	} else if (name == "checkedhover") {
		setStateGump(CheckedHover, Utilities::stringToUInt(value));
	} else if (name == "checked") {
		setChecked(Utilities::stringToBool(value));
	} else {
		cControl::processDefinitionAttribute(name, value);
	}
}

void cCheckbox::processDefinitionElement(QDomElement element) {
	if (element.nodeName() == "pressed") {
		// Check for gump/hue/partialhue
		ushort gump = Utilities::stringToUInt(element.attribute("gump"));
		ushort hue = Utilities::stringToUInt(element.attribute("hue"));
		bool partialhue = Utilities::stringToBool(element.attribute("partialhue"));
		setStateGump(Pressed, gump, hue, partialhue);
	} else if (element.nodeName() == "unchecked") {
		// Check for gump/hue/partialhue
		ushort gump = Utilities::stringToUInt(element.attribute("gump"));
		ushort hue = Utilities::stringToUInt(element.attribute("hue"));
		bool partialhue = Utilities::stringToBool(element.attribute("partialhue"));
		setStateGump(Unchecked, gump, hue, partialhue);
	} else if (element.nodeName() == "hover") {
		// Check for gump/hue/partialhue
		ushort gump = Utilities::stringToUInt(element.attribute("gump"));
		ushort hue = Utilities::stringToUInt(element.attribute("hue"));
		bool partialhue = Utilities::stringToBool(element.attribute("partialhue"));
		setStateGump(Hover, gump, hue, partialhue);
	} else if (element.nodeName() == "checked") {
		// Check for gump/hue/partialhue
		ushort gump = Utilities::stringToUInt(element.attribute("gump"));
		ushort hue = Utilities::stringToUInt(element.attribute("hue"));
		bool partialhue = Utilities::stringToBool(element.attribute("partialhue"));
		setStateGump(Checked, gump, hue, partialhue);
	} else if (element.nodeName() == "checkedhover") {
		// Check for gump/hue/partialhue
		ushort gump = Utilities::stringToUInt(element.attribute("gump"));
		ushort hue = Utilities::stringToUInt(element.attribute("hue"));
		bool partialhue = Utilities::stringToBool(element.attribute("partialhue"));
		setStateGump(CheckedHover, gump, hue, partialhue);
	} else {
        return cControl::processDefinitionElement(element);
	}
}
