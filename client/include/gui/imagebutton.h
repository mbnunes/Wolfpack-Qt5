#if !defined(__IMAGEBUTTON_H__)
#define __IMAGEBUTTON_H__

#include "control.h"
#include "gui.h"
#include "enums.h"
#include <QTimer>
#include <QMouseEvent>
#include <QKeyEvent>

class cImageButton : public cControl {
Q_OBJECT

private:
	// There are a lot of different possibilities for the states
	cTexture *gumps[3]; // For every state one
	bool mouseOver_; // False by default (is the mouse over this control)
	bool mouseHolding_; // False by default (is the space key being pressed)
	bool spaceHolding_; // False by default (is the space key being pressed)
	unsigned int pressRepeatRate_; // Number of miliseconds a press will be regarded as an onClick event.
	QTimer *pressRepeatTimer;

	inline enButtonStates getState() {
		if (!mouseHolding_ && !spaceHolding_) {
			if ((mouseOver_ || Gui->inputFocus() == this) && gumps[BS_HOVER]) {
				return BS_HOVER;
			} else {
				return BS_UNPRESSED;
			}
		} else {
			if (mouseOver_ || spaceHolding_) {
				return BS_PRESSED;
			} else {
				if (gumps[BS_HOVER]) {
					return BS_HOVER;
				} else {
					return BS_UNPRESSED;
				}
			}
		}
	}
public:
	cImageButton(int x, int y, unsigned short up, unsigned short down); // LEGACY
	cImageButton();
	virtual ~cImageButton();

	void setStateGump(enButtonStates state, unsigned short id, unsigned short hue = 0, bool partialHue = false);

	inline bool mouseOver() const { return mouseOver_; } // Returns true if the mouse is over this button
	inline bool mouseHolding() const { return mouseHolding_; } // Returns true if the mouse button is hold
	inline bool spaceHolding() const { return spaceHolding_; } // Returns true if the space key is being held
	
	// Press Repeat Rate
	inline unsigned int pressRepeatRate() const { return pressRepeatRate_; }
	inline void setPressRepeatRate(unsigned int data) {
		pressRepeatRate_ = data;
	}

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

// Slot for the press repeat 
private slots:
	void repeatPress();

signals:
	void onButtonPress(cControl *sender);
};

#endif
