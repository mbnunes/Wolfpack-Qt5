
#include "gui/textbutton.h"
#include "utilities.h"
#include <QTimer>

static const ushort styles[cTextButton::StyleCount][4] = {
	// UNPRESSED, HOVER, PRESSED, Stripe Flag
	{ 0x24b8, 0x24ea, 0x251c, 0 },
	{ 0x254e, 0x2557, 0x2560, 0 },
	{ 0x2904, 0x2907, 0x290a, 1 },
	{ 0x2936, 0x2939, 0x293c, 1 },
};

cTextButton::cTextButton() {
	style_ = Style1;
	dirty = true;
	for (int i = 0; i < 3; ++i) {
		backgroundGump_[i] = 0;		
		backgroundStripes_[i] = 0;
	}
	label_[0] = 0;
	label_[1] = 0;
	backgroundHue_ = 0;

	movable_ = false;
	moveHandle_ = false;
	mouseOver_ = false;
	mouseHolding_ = false;
	spaceHolding_ = false;
	canHaveFocus_ = true; // Buttons can have the input focus
	pressRepeatRate_ = 0; // There is no press repeat rate by default (normal push buttons)

	width_ = 0;
	height_ = 0;

	pressRepeatTimer = new QTimer(this); // Should be auto freed
	connect(pressRepeatTimer, SIGNAL(timeout()), this, SLOT(repeatPress())); // Connect the timer to the press repeat slot
}

cTextButton::~cTextButton() {
	for (int i = 0; i < 3; ++i) {
		delete backgroundGump_[i];
		delete backgroundStripes_[i];
	}
	delete label_[0];
	delete label_[1];
}

void cTextButton::update() {
	if (!dirty) {
		return;
	}

	// The Background is a stripe if the style is flagged as such
	backgroundStripe = styles[style_][3] != 0;

	for (int i = 0; i < 3; ++i) {
		delete backgroundGump_[i];
		backgroundGump_[i] = 0;
		delete backgroundStripes_[i];
		backgroundStripes_[i] = 0;

		// The Background is a stripe
		if (backgroundStripe) {
			if (styles[style_][i] != 0) {
				backgroundStripes_[i] = new cStripeImage;
				backgroundStripes_[i]->setId(styles[style_][i]);
				backgroundStripes_[i]->setHue(backgroundHue_);
				backgroundStripes_[i]->setPartialHue(true);
				backgroundStripes_[i]->setMoveHandle(false);
				backgroundStripes_[i]->setMovable(false);
				backgroundStripes_[i]->setSize(width_, height_);
			}
		} else {
			if (styles[style_][i] != 0) {
				backgroundGump_[i] = new cBorderGump(styles[style_][i], backgroundHue_, true);
				backgroundGump_[i]->setMoveHandle(false);
				backgroundGump_[i]->setMovable(false);
				backgroundGump_[i]->setSize(width_, height_);
			}
		}
	}

	label_[0] = new cLabel(text_, 1, 1, false);
	label_[0]->update();
	label_[1] = new cLabel(text_, 1, 0x37, false);
	label_[1]->update();

	dirty = false;
}

void cTextButton::draw(int xoffset, int yoffset) {
	if (dirty) {
		update();
	}

	enButtonStates state = getState();

	if (backgroundStripe) {
		if (backgroundStripes_[state]) {
			backgroundStripes_[state]->draw(xoffset + x_, yoffset + y_);
		}
	} else {
		if (backgroundGump_[state]) {
			backgroundGump_[state]->draw(xoffset + x_, yoffset + y_);
		}
	}

	if (state == BS_UNPRESSED) {
		if (label_[0]) {
			int labelX = xoffset + x_ + (width_ - label_[0]->width()) / 2;
			int labelY = yoffset + y_ + (height_ - label_[0]->height()) / 2;
			label_[0]->draw(labelX, labelY);
		}
	} else {
		if (label_[1]) {
			int labelX = xoffset + x_ + (width_ - label_[1]->width()) / 2;
			int labelY = yoffset + y_ + (height_ - label_[1]->height()) / 2;
			if (state == BS_PRESSED) {
				labelY += 2; // Shifted down by 2 pixels to simulate "press"
			}
			label_[1]->draw(labelX, labelY);
		}
	}
}

void cTextButton::onMouseLeave() {
	mouseOver_ = false;
}

void cTextButton::onMouseEnter() {
	mouseOver_ = true;
}

void cTextButton::onMouseDown(QMouseEvent *e) {
	if (e->button() == Qt::LeftButton) {
		mouseHolding_ = true;

		// The button requests to be auto-pressed every x miliseconds
		if (pressRepeatRate_ != 0) {
			pressRepeatTimer->start(pressRepeatRate_);
			emit onButtonPress(this);
		}
	}
}

void cTextButton::onClick(QMouseEvent *e) {
}

void cTextButton::onMouseUp(QMouseEvent *e) {
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

void cTextButton::onKeyDown(QKeyEvent *e) {
	if (e->key() == Qt::Key_Return) {
		emit onButtonPress(this); // Fire the onClick event if return is pressed
	} else if (e->key() == Qt::Key_Space) {
		spaceHolding_ = true;
	}
}

void cTextButton::onKeyUp(QKeyEvent *e) {
	if (e->key() == Qt::Key_Space) {
		spaceHolding_ = false;
		emit onButtonPress(this); // Issue the click event
	}
}

void cTextButton::onBlur(cControl *newFocus) {
	if (spaceHolding_) {
		spaceHolding_ = false;
	}
	cControl::onBlur(newFocus);
}

cControl *cTextButton::getControl(int x, int y) {
	cControl *gump;
	
	if (backgroundStripe) {
		gump = backgroundStripes_[getState()];
	} else {
		gump = backgroundGump_[getState()];
	}

	if (gump && gump->getControl(x, y) != 0) {
		return this;
	}

	return 0;
}

void cTextButton::processDefinitionAttribute(QString name, QString value) {
	if (name == "hue") {
		setBackgroundHue(Utilities::stringToUInt(value));
	} else if (name == "style") {
		setStyle(Style(Utilities::stringToUInt(value)));
	} else if (name == "text") {
		setText(value);
	} else {
		cControl::processDefinitionAttribute(name, value);
	}
}

void cTextButton::processDefinitionElement(QDomElement element) {
	cControl::processDefinitionElement(element);
}

void cTextButton::repeatPress() {
	// Send a repeated onClick event if we're above the button
	// and it's configured to do so
	if (mouseHolding_ && pressRepeatRate_ && mouseOver_) {
		emit onButtonPress(this);
	}
}
