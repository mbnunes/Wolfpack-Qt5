
#include <qglobal.h>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>

#include "gui/radiobutton.h"
#include "muls/gumpart.h"
#include "utilities.h"

cRadioButton::cRadioButton() {
	mouseOver_ = false;
	mouseHolding_ = false;
	spaceHolding_ = false;
	canHaveFocus_ = true; // Buttons can have the input focus
	checked_ = false;
	group_ = 0;

	width_ = 0;
	height_ = 0;

	gumps[Unchecked] = 0;	
	gumps[Hover] = 0;
	gumps[Pressed] = 0;
	gumps[Checked] = 0;
	gumps[CheckedHover] = 0;	
}

cRadioButton::~cRadioButton() {
	for (int i = 0; i < 5; ++i) {
		if (gumps[i]) {
			gumps[i]->decref();
		}
	}
}

void cRadioButton::onMouseLeave() {
	mouseOver_ = false;
}

void cRadioButton::onMouseEnter() {
	mouseOver_ = true;
}

void cRadioButton::setStateGump(State state, unsigned short id, unsigned short hue, bool partialHue) {
	if (gumps[state]) {
		gumps[state]->decref();
	}

	gumps[state] = Gumpart->readTexture(id, hue, partialHue);

	if (gumps[Unchecked]) {
		width_ = gumps[Unchecked]->realWidth();
		height_ = gumps[Unchecked]->realHeight();
	}
}

cControl *cRadioButton::getControl(int x, int y) {
	cTexture *texture = gumps[getState()];

	if (texture && texture->hitTest(x, y)) {
		return this;
	}

	return 0;
}

void cRadioButton::draw(int xoffset, int yoffset) {
	cTexture *texture = gumps[getState()];

	if (texture) {
		texture->draw(xoffset + x_, yoffset + y_);
	}
}

void cRadioButton::onMouseDown(QMouseEvent *e) {
	if (e->button() == Qt::LeftButton) {
		mouseHolding_ = true;
	}
}

void cRadioButton::onMouseUp(QMouseEvent *e) {
	if (e->button() == Qt::LeftButton) {
		mouseHolding_ = false;
		if (mouseOver_) {
			setChecked(true);
			emit stateChanged();
		}
	}
}

void cRadioButton::onKeyDown(QKeyEvent *e) {
	if (e->key() == Qt::Key_Space) {
		spaceHolding_ = true;
	}
}

void cRadioButton::onClick(QMouseEvent *e) {
}

void cRadioButton::onKeyUp(QKeyEvent *e) {
	if (e->key() == Qt::Key_Space) {
		spaceHolding_ = false;
		setChecked(true);
		emit stateChanged(); // Issue the click event
	}
}

void cRadioButton::onBlur(cControl *newFocus) {
	if (spaceHolding_) {
		spaceHolding_ = false;
	}
	cControl::onBlur(newFocus);
}

void cRadioButton::processDefinitionAttribute(QString name, QString value) {
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
	} else if (name == "group") {
		setGroup(Utilities::stringToUInt(value));
	} else {
		cControl::processDefinitionAttribute(name, value);
	}
}

void cRadioButton::processDefinitionElement(QDomElement element) {
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

void cRadioButton::setChecked(bool data) {
	checked_ = data;

	// Iterate over the parent and uncheck all radiobuttons with the same group
	if (checked_ && parent_) {
		cContainer::Controls controls = parent_->getControls();
		foreach (cControl *control, controls) {
			cRadioButton *radio = dynamic_cast<cRadioButton*>(control);
			if (radio && radio != this && radio->group() == group_) {
				radio->setChecked(false);
			}
		}
	}
}
