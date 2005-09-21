
#if !defined(__CHECKBOX_H__)
#define __CHECKBOX_H__

#include "control.h"
#include "gui.h"
#include "enums.h"
#include <QTimer>
#include <QMouseEvent>
#include <QKeyEvent>

class cCheckbox : public cControl {
Q_OBJECT
Q_ENUMS(State)
public:
	enum State {
		Unchecked = 0,
		Hover,
		Pressed,
		Checked,
		CheckedHover
	};

private:
	// There are five states
	cTexture *gumps[5]; // For every state one
	bool mouseOver_; // False by default (is the mouse over this control)
	bool mouseHolding_; // False by default (is the space key being pressed)
	bool spaceHolding_; // False by default (is the space key being pressed)
	bool checked_; // Is the checkbox checked?

	inline State getState() {
		if (!mouseHolding_ && !spaceHolding_) {
			if ((mouseOver_ || Gui->inputFocus() == this) && gumps[BS_HOVER]) {
				return checked_ ? (gumps[CheckedHover] ? CheckedHover : Checked) : Hover;
			} else {
				return checked_ ? Checked : Unchecked;
			}
		} else {
			if (mouseOver_ || spaceHolding_) {
				return Pressed;
			} else {
				if (gumps[Hover]) {
					return Hover;
				} else {
					return checked_ ? Checked : Unchecked;
				}
			}
		}
	}
public:
	cCheckbox();
	virtual ~cCheckbox();
	
	void update();
	void draw(int xoffset, int yoffset);

	void onMouseLeave(); // Toggle the mouseOver state
	void onMouseEnter(); // Toggle the mouseOver state
	void onMouseDown(QMouseEvent *e);
	void onClick(QMouseEvent *e);
	void onMouseUp(QMouseEvent *e);
	void onKeyDown(QKeyEvent *e);
	void onKeyUp(QKeyEvent *e);
	void onBlur(cControl *newFocus);

	cControl *getControl(int x, int y);

	void processDefinitionAttribute(QString name, QString value);
	void processDefinitionElement(QDomElement element);

public slots:
	void setStateGump(State state, unsigned short id, unsigned short hue = 0, bool partialHue = false);
	void setStateGump(int state, unsigned short id, unsigned short hue = 0, bool partialHue = false);

	bool checked() const;
	void setChecked(bool checked);
	inline bool mouseOver() const { return mouseOver_; } // Returns true if the mouse is over this button
	inline bool mouseHolding() const { return mouseHolding_; } // Returns true if the mouse button is hold
	inline bool spaceHolding() const { return spaceHolding_; } // Returns true if the space key is being held

signals:
	void stateChanged();
};

inline bool cCheckbox::checked() const {
	return checked_;
}

inline void cCheckbox::setChecked(bool checked) {
	checked_ = checked;
}

inline void cCheckbox::setStateGump(int state, unsigned short id, unsigned short hue, bool partialHue) {
	setStateGump(cCheckbox::State(state), id, hue, partialHue);
}

#endif
