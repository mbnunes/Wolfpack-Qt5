
#if !defined(__LABEL_H__)
#define __LABEL_H__

#include <qstring.h>

#include "control.h"
#include "texture.h"

class cLabel : public cControl {
Q_OBJECT

private:
	cTexture *surface;
	QString text_;
	unsigned char font_; // Unicode font
	unsigned short hue_;
	bool border_;
	enTextAlign align_;
	bool autoSize_;
	bool exactHitTest_;
public:
	cLabel(const QString &text, unsigned short font = 0, unsigned short hue = 0, bool border = true, enTextAlign align_ = ALIGN_LEFT, bool autoSize = true);
	virtual ~cLabel();

	cControl *getControl(int x, int y);

	// Getters
	const QString &text() const { return text_; }
	unsigned char font() const { return font_; }
	unsigned short hue() const { return hue_; }
	bool border() const { return border_; }
	enTextAlign align() const { return align_; }
	bool autoSize() const { return align_; }
	bool exactHitTest() const { return exactHitTest_; }
	void update();

	// Setters
	inline void setExactHitTest(bool data) {
		exactHitTest_ = data;
	}

	inline void setText(const QString &text) {
		if (text_ != text) {
			text_ = text;
			if (surface) {
				surface->decref();
				surface = 0;
			}
		}
	}
	inline void setFont(unsigned char font) {
		if (font != font_) {
			font_ = font;
			if (surface) {
				surface->decref();
				surface = 0;
			}
		}
	}
	inline void setHue(unsigned short hue) {
		if (hue != hue_) {
			hue_ = hue;
			if (surface) {
				surface->decref();
				surface = 0;
			}
		}
	}
	inline void setBorder(bool border) {
		if (border != border_) {
			border_ = border;
			if (surface) {
				surface->decref();
				surface = 0;
			}
		}
	}
	inline void setAlign(enTextAlign align) {
		if (align != align_) {
			align_ = align;
			if (surface) {
				surface->decref();
				surface = 0;
			}
		}
	}
	inline void setAutoSize(bool autoSize) {
		if (autoSize != autoSize_) {
			autoSize_ = autoSize;
			if (surface && autoSize && width_ != surface->realWidth() && height_ != surface->realHeight()) {
				setSize(surface->realWidth(), surface->realHeight());
			}
		}
	}

	void draw(int xoffset, int yoffset);
};

#endif
