
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
	bool htmlMode_;
	uint borderColor_;
	bool emboss_;
public:
	cLabel(const QString &text, unsigned short font = 0, unsigned short hue = 0, bool border = true, enTextAlign align_ = ALIGN_LEFT, bool autoSize = true);
	cLabel();
	virtual ~cLabel();

public slots:
	cControl *getControl(int x, int y);

	// Getters
	const QString &text() const { return text_; }
	unsigned char font() const { return font_; }
	unsigned short hue() const { return hue_; }
	bool border() const { return border_; }
	enTextAlign align() const { return align_; }
	bool autoSize() const { return align_; }
	bool exactHitTest() const { return exactHitTest_; }
	uint borderColor() const;
	void setBorderColor(uint color);
	void update();
	bool htmlMode() const;
	void setHtmlMode(bool data);
	bool emboss() const;
	void setEmboss(bool data);
	

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
			if (surface && autoSize && ((unsigned int)width_ != surface->realWidth() || (unsigned int)height_ != surface->realHeight())) {
				setSize(surface->realWidth(), surface->realHeight());
			}
		}
	}

	void draw(int xoffset, int yoffset);

	void processDefinitionAttribute(QString name, QString value);
	void processDefinitionElement(QDomElement element);
};

inline bool cLabel::htmlMode() const {
	return htmlMode_;
}

inline void cLabel::setHtmlMode(bool data) {
	htmlMode_ = data;
}

inline bool cLabel::emboss() const {
	return emboss_;
}

inline void cLabel::setEmboss(bool data) {
	emboss_ = data;
}


inline uint cLabel::borderColor() const {
	return borderColor_;
}

inline void cLabel::setBorderColor(uint color) {
	borderColor_ = color;
}

#endif
