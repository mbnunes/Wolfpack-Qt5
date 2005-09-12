
#if !defined(__TEXTBUTTON_H__)
#define __TEXTBUTTON_H__

#include "gui/control.h"
#include "gui/label.h"
#include "gui/bordergump.h"
#include "gui/gui.h"

#include <QString>

class cTextButton : public cControl {
Q_OBJECT
public:
	enum Style {
		Style1 = 0,
		Style2,
		StyleCount,
	};

private:
	bool dirty;

protected:
	cBorderGump *backgroundGump_[3];
	ushort backgroundHue_;
	cLabel *label_[2];
	
	bool mouseOver_; // False by default (is the mouse over this control)
	bool mouseHolding_; // False by default (is the space key being pressed)
	bool spaceHolding_; // False by default (is the space key being pressed)
	unsigned int pressRepeatRate_; // Number of miliseconds a press will be regarded as an onClick event.
	QTimer *pressRepeatTimer;
	Style style_;
	QString text_;

	inline enButtonStates getState() {
		if (!mouseHolding_ && !spaceHolding_) {
			if (mouseOver_ || Gui->inputFocus() == this) {
				return BS_HOVER;
			} else {
				return BS_UNPRESSED;
			}
		} else {
			if (mouseOver_ || spaceHolding_) {
				return BS_PRESSED;
			} else {
				return BS_HOVER;
			}
		}
	}

public:
	cTextButton();
	virtual ~cTextButton();

	void draw(int xoffset, int yoffset);
	void update();

public slots:
	bool mouseOver() const; // Returns true if the mouse is over this button
	bool mouseHolding() const; // Returns true if the mouse button is hold
	bool spaceHolding() const; // Returns true if the space key is being held

	Style style() const;
	void setStyle(Style style);
	const QString &text() const;
	void setText(const QString &text);
	uint pressRepeatRate() const;
	void setPressRepeatRate(unsigned int data);
	ushort backgroundHue() const;
	void setBackgroundHue(ushort hue);

	void onMouseLeave(); // Toggle the mouseOver state
	void onMouseEnter(); // Toggle the mouseOver state
	void onMouseDown(QMouseEvent *e);
	void onClick(QMouseEvent *e);
	void onMouseUp(QMouseEvent *e);
	void onKeyDown(QKeyEvent *e);
	void onKeyUp(QKeyEvent *e);
	void onBlur(cControl *newFocus);

	cControl *getControl(int x, int y);
	void repeatPress();

	void processDefinitionAttribute(QString name, QString value);
	void processDefinitionElement(QDomElement element);

signals:
	void onButtonPress(cControl *sender);
};

inline uint cTextButton::pressRepeatRate() const {
	return pressRepeatRate_;
}

inline void cTextButton::setPressRepeatRate(uint data) {
	pressRepeatRate_ = data;
}

inline bool cTextButton::mouseOver() const {
	return mouseOver_;
}

inline bool cTextButton::mouseHolding() const {
	return mouseHolding_;
}

inline bool cTextButton::spaceHolding() const {
	return spaceHolding_;
}

inline cTextButton::Style cTextButton::style() const {
	return style_;
}

inline const QString &cTextButton::text() const {
	return text_;
}

inline ushort cTextButton::backgroundHue() const {
	return backgroundHue_;
}	

inline void cTextButton::setBackgroundHue(ushort hue) {
	backgroundHue_ = hue;
	dirty = true;
}

inline void cTextButton::setStyle(Style style) {
	if (style < StyleCount) {
		style_ = style;
		dirty = true;
	}
}

inline void cTextButton::setText(const QString &text) {
	text_ = text;
	dirty = true;
}

#endif
