
#if !defined(__TEXTFIELD_H__)
#define __TEXTFIELD_H__

#include <qcstring.h>

#include "control.h"
#include "bordergump.h"
#include "texture.h"

/*
	A textfield is just another container for now
*/
class cTextField : public cControl {
protected:
	cBorderGump *background_;
	unsigned short backgroundId_;

	bool hueAll_;
	unsigned char font_;
	unsigned short hue_;
	QCString text_;
	short mouseOverHue_;
	short focusHue_;
	bool password_;

    cTexture *surfaces[3]; // Three States

	bool mouseOver_; // False by default (is the mouse over this control)
    
	void update();
	void draw(int xoffset, int yoffset);
	unsigned int caret_; // The offset in characters from the left to the text input positioner.
	unsigned int leftOffset_; // Offset in characters from the left border of the text field to the beginning of the text.
	unsigned int maxLength_; // Maximum length in characters of the text fields contents.
	unsigned int caretXOffset_; // The offset to the left border of the text field in pixels.
	int selection_; // The number of characters that are selected in relation to the current caret.

    void drawSelection(SDL_Surface *surface);
	void replaceSelection(const QCString &replacement);
	QCString getSelection();
	inline char translateChar(char c) {
		if (password_) {
			return '*';
		} else {
			return c;
		}
	}
public:
	cTextField(int x, int y, int width, int height, unsigned char font, unsigned short hue = 0, unsigned short background = 0xbb8, bool hueAll = false);
	virtual ~cTextField();
    
	// Getters
	inline bool hueAll() const { return hueAll_; }
	inline unsigned char font() const { return font_; }
	inline unsigned short hue() const { return hue_; }
	inline const QCString &text() const { return text_; }
	inline unsigned short background() const { return backgroundId_; }
	inline unsigned int leftOffset() const { return leftOffset_; }
	inline bool password() const { return password_; }

	inline void setPassword(bool data) {
		password_ = data;
		invalidateText();
	}

	unsigned int getOffset(int x); // Get the character index for the given x coordinate

	// Invalidate the Text
	inline void invalidateText() {
		for (int i = 0; i < 3; ++i) {
			if (surfaces[i]) {
				surfaces[i]->decref();
				surfaces[i] = 0;
			}
		}
		invalidate();
	}

	// Setters
	inline void setHueAll(bool data) {
		if (hueAll_ != data) {
			hueAll_ = data;
			invalidateText();
		}
	}

	inline void setFont(unsigned char data) {
		if (font_ != data) {
			font_ = data;
			invalidateText();
		}
	}

	inline void setHue(unsigned short data) {
		if (hue_ != data) {
			hue_ = data;
			invalidateText();
		}
	}

	inline void setMouseOverHue(short data) {
		if (mouseOverHue_ != data) {
			mouseOverHue_ = data;
			invalidateText();
		}
	}

	inline void setFocusHue(short data) {
		if (focusHue_ != data) {
			focusHue_ = data;
			invalidateText();
		}
	}

	inline void setText(const QCString &data) {
		if (text_ != data) {
			text_ = data;
			invalidateText();
		}
	}

	inline void setBackground(unsigned short data) {
		if (backgroundId_ != data) {
			backgroundId_ = data;
			delete background_;
			background_ = 0;
			invalidate();
		}
	}

	void setCaret(unsigned int pos);

	inline void setMaxLength(unsigned int data) {
		if (maxLength_ != data) {
			maxLength_ = data;

			if (text_.length() > maxLength_) {
				int toremove = text_.length() - maxLength_;
				text_.remove(text_.length() - toremove, toremove);
				invalidateText();
			}
		}
	}

	void onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight);
	void onMouseLeave(); // Toggle the mouseOver state
	void onMouseEnter(); // Toggle the mouseOver state
	void onMouseDown(int x, int y, unsigned char button, bool pressed);
	void onMouseUp(int x, int y, unsigned char button, bool pressed);
	void onKeyDown(const SDL_keysym &key);
	void onKeyUp(const SDL_keysym &key);

	// This is called if enter is pressed
	void onEnter();
};

#endif