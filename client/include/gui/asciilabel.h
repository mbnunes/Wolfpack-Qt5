
#if !defined(__ASCIILABEL_H__)
#define __ASCIILABEL_H__

#include "SDL.h"
#include <qstring.h>

#include "gui/control.h"
#include "texture.h"

class cAsciiLabel : public cControl {
private:
	QCString text_;
	unsigned short hue_;
	unsigned char font_;
	enTextAlign align_;
	bool autoSize_;
	bool hueAll_;
	cTexture *texture;
public:
	cAsciiLabel(const QCString &text, unsigned short font = 0, unsigned short hue = 0, enTextAlign align = ALIGN_LEFT, bool autoSize = true, bool hueAll = false);
	virtual ~cAsciiLabel();
	void draw(int xoffset, int yoffset);

	// Getters
	const QCString &text() const { return text_; }
	unsigned char font() const { return font_; }
	unsigned short hue() const { return hue_; }
	enTextAlign align() const { return align_; }
	bool autoSize() const { return align_; }
	bool hueAll() const { return hueAll_; }

	// Setters
	inline void setText(const QCString &text) {
		if (text_ != text) {
			text_ = text;
			invalidate();
		}
	}
	inline void setFont(unsigned char font) {
		if (font != font_) {
			font_ = font;
			invalidate();
		}
	}
	inline void setHue(unsigned short hue) {
		if (hue != hue_) {
			hue_ = hue;
			invalidate();
		}
	}
	inline void setAlign(enTextAlign align) {
		if (align != align_) {
			align_ = align;
			invalidate();
		}
	}
	inline void setAutoSize(bool autoSize) {
		if (autoSize != autoSize_) {
			autoSize_ = autoSize;
			if (texture && autoSize && width_ != texture->realWidth() && height_ != texture->realHeight()) {
				setSize(texture->realWidth(), texture->realHeight());
			}
		}
	}
	inline void setHueAll(bool hueAll) {
		if (hueAll_ != hueAll) {
			hueAll_ = hueAll;
			invalidate();
		}
	}

	void update();
};

#endif
