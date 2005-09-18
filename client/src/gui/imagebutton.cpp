
#include <qglobal.h>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>

#include "gui/imagebutton.h"
#include "muls/gumpart.h"
#include "utilities.h"

ButtonStatic::ButtonStatic() {
	setObjectName("Button");
}

cImageButton::cImageButton(int x, int y, unsigned short up, unsigned short down) {
	x_ = x;
	y_ = y;
	mouseOver_ = false;
	mouseHolding_ = false;
	spaceHolding_ = false;
	canHaveFocus_ = true; // Buttons can have the input focus
	pressRepeatRate_ = 0; // There is no press repeat rate by default (normal push buttons)

	width_ = 0;
	height_ = 0;

	gumps[BS_UNPRESSED] = Gumpart->readTexture(up);
	gumps[BS_HOVER] = 0;
	gumps[BS_PRESSED] = Gumpart->readTexture(down);
	
	if (gumps[BS_UNPRESSED]) {
		width_ = gumps[BS_UNPRESSED]->realWidth();
		height_ = gumps[BS_UNPRESSED]->realHeight();
	}

	pressRepeatTimer = new QTimer(this); // Should be auto freed
	pressRepeatTimer->setSingleShot(true);
	connect(pressRepeatTimer, SIGNAL(timeout()), this, SLOT(repeatPress())); // Connect the timer to the press repeat slot
}

cImageButton::cImageButton() {
	mouseOver_ = false;
	mouseHolding_ = false;
	spaceHolding_ = false;
	canHaveFocus_ = true; // Buttons can have the input focus
	pressRepeatRate_ = 0; // There is no press repeat rate by default (normal push buttons)

	width_ = 0;
	height_ = 0;

	gumps[BS_UNPRESSED] = 0;
	gumps[BS_HOVER] = 0;
	gumps[BS_PRESSED] = 0;
	
	pressRepeatTimer = new QTimer(this); // Should be auto freed
	connect(pressRepeatTimer, SIGNAL(timeout()), this, SLOT(repeatPress())); // Connect the timer to the press repeat slot
}

cImageButton::~cImageButton() {
	for (int i = 0; i < 3; ++i) {
		if (gumps[i]) {
			gumps[i]->decref();
		}
	}
}

void cImageButton::onMouseLeave() {
	mouseOver_ = false;
}

void cImageButton::onMouseEnter() {
	mouseOver_ = true;
}

void cImageButton::setStateGump(enButtonStates state, unsigned short id, unsigned short hue, bool partialHue) {
	if (gumps[state]) {
		gumps[state]->decref();
	}

	gumps[state] = Gumpart->readTexture(id, hue, partialHue);

	if (gumps[BS_UNPRESSED]) {
		width_ = gumps[BS_UNPRESSED]->realWidth();
		height_ = gumps[BS_UNPRESSED]->realHeight();
	}
}

cControl *cImageButton::getControl(int x, int y) {
	cTexture *texture = gumps[getState()];

	if (texture && texture->hitTest(x, y)) {
		return this;
	}

	return 0;
}

void cImageButton::draw(int xoffset, int yoffset) {
	cTexture *texture = gumps[getState()];

	if (texture) {
		texture->draw(xoffset + x_, yoffset + y_);
	}
}

void cImageButton::onMouseDown(QMouseEvent *e) {
	if (e->button() == Qt::LeftButton) {
		mouseHolding_ = true;

		// The button requests to be auto-pressed every x miliseconds
		if (pressRepeatRate_ != 0) {
			pressRepeatTimer->start(QApplication::doubleClickInterval());
			emit onButtonPress(this);
		}
	}
}

void cImageButton::repeatPress() {
	// Send a repeated onClick event if we're above the button
	// and it's configured to do so
	if (mouseHolding_ && pressRepeatRate_ && mouseOver_) {
		pressRepeatTimer->start(pressRepeatRate_);
		emit onButtonPress(this);
	}
}

void cImageButton::onMouseUp(QMouseEvent *e) {
	if (e->button() == Qt::LeftButton) {
		mouseHolding_ = false;
		
		// If a press-repeat timer is active, stop it. 
		// Otherwise issue the click event.
		if (pressRepeatTimer->isActive()) {
			pressRepeatTimer->stop();
		} else if (mouseOver_) {
			emit onButtonPress(this);
		}
	}
}

void cImageButton::onKeyDown(QKeyEvent *e) {
	if (e->key() == Qt::Key_Return) {
		emit onButtonPress(this); // Fire the onClick event if return is pressed
	} else if (e->key() == Qt::Key_Space) {
		spaceHolding_ = true;
	}
}

void cImageButton::onClick(QMouseEvent *e) {
}

void cImageButton::onKeyUp(QKeyEvent *e) {
	if (e->key() == Qt::Key_Space) {
		spaceHolding_ = false;
		emit onButtonPress(this); // Issue the click event
	}
}

void cImageButton::onBlur(cControl *newFocus) {
	if (spaceHolding_) {
		spaceHolding_ = false;
	}
	cControl::onBlur(newFocus);
}

void cImageButton::processDefinitionAttribute(QString name, QString value) {
	if (name == "unpressed") {
		setStateGump(BS_UNPRESSED, Utilities::stringToUInt(value));
	} else if (name == "pressed") {
		setStateGump(BS_PRESSED, Utilities::stringToUInt(value));
	} else if (name == "hover") {
		setStateGump(BS_HOVER, Utilities::stringToUInt(value));
	} else if (name == "pressrepeatrate") {
		setPressRepeatRate(Utilities::stringToUInt(value));
	} else {
		cControl::processDefinitionAttribute(name, value);
	}
}

void cImageButton::processDefinitionElement(QDomElement element) {
	if (element.nodeName() == "pressed") {
		// Check for gump/hue/partialhue
		ushort gump = Utilities::stringToUInt(element.attribute("gump"));
		ushort hue = Utilities::stringToUInt(element.attribute("hue"));
		bool partialhue = Utilities::stringToBool(element.attribute("partialhue"));
		setStateGump(BS_PRESSED, gump, hue, partialhue);
	} else if (element.nodeName() == "unpressed") {
		// Check for gump/hue/partialhue
		ushort gump = Utilities::stringToUInt(element.attribute("gump"));
		ushort hue = Utilities::stringToUInt(element.attribute("hue"));
		bool partialhue = Utilities::stringToBool(element.attribute("partialhue"));
		setStateGump(BS_UNPRESSED, gump, hue, partialhue);
	} else if (element.nodeName() == "hover") {
		// Check for gump/hue/partialhue
		ushort gump = Utilities::stringToUInt(element.attribute("gump"));
		ushort hue = Utilities::stringToUInt(element.attribute("hue"));
		bool partialhue = Utilities::stringToBool(element.attribute("partialhue"));
		setStateGump(BS_HOVER, gump, hue, partialhue);
	} else {
        return cControl::processDefinitionElement(element);
	}
}
