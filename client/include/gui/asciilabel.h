
#if !defined(__ASCIILABEL_H__)
#define __ASCIILABEL_H__

#include <qstring.h>
//Added by qt3to4:
#include <Q3CString>

#include "gui/control.h"
#include "texture.h"

class cAsciiLabel : public cControl {
Q_OBJECT
private:
	Q3CString text_;
	unsigned short hue_;
	unsigned char font_;
	enTextAlign align_;
	bool autoSize_;
	bool hueAll_;
	cTexture *texture;
public:
	cAsciiLabel(const Q3CString &text, unsigned short font = 0, unsigned short hue = 0, enTextAlign align = ALIGN_LEFT, bool autoSize = true, bool hueAll = false);
	virtual ~cAsciiLabel();
	void draw(int xoffset, int yoffset);

	// Getters
	const Q3CString &text() const { return text_; }
	unsigned char font() const { return font_; }
	unsigned short hue() const { return hue_; }
	enTextAlign align() const { return align_; }
	bool autoSize() const { return align_; }
	bool hueAll() const { return hueAll_; }

	// Setters
	inline void setText(const Q3CString &text) {
		if (text_ != text) {
			text_ = text;
			if (texture) {
				texture->decref();
				texture = 0;
			}
		}
	}
	inline void setFont(unsigned char font) {
		if (font != font_) {
			font_ = font;
			if (texture) {
				texture->decref();
				texture = 0;
			}
		}
	}
	inline void setHue(unsigned short hue) {
		if (hue != hue_) {
			hue_ = hue;
			if (texture) {
				texture->decref();
				texture = 0;
			}
		}
	}
	inline void setAlign(enTextAlign align) {
		if (align != align_) {
			align_ = align;
			if (texture) {
				texture->decref();
				texture = 0;
			}
		}
	}
	inline void setAutoSize(bool autoSize) {
		if (autoSize != autoSize_) {
			autoSize_ = autoSize;
			if (texture && autoSize && (unsigned int)width_ != texture->realWidth() && (unsigned int)height_ != texture->realHeight()) {
				setSize(texture->realWidth(), texture->realHeight());
			}
		}
	}
	inline void setHueAll(bool hueAll) {
		if (hueAll_ != hueAll) {
			hueAll_ = hueAll;
			if (texture) {
				texture->decref();
				texture = 0;
			}
		}
	}

	void update();
};

#endif
