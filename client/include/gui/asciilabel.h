
#if !defined(__ASCIILABEL_H__)
#define __ASCIILABEL_H__

#include <QString>

#include "gui/control.h"
#include "texture.h"

class cAsciiLabel : public cControl {
Q_OBJECT
Q_PROPERTY(QString text READ text WRITE setText)
Q_PROPERTY(ushort hue READ hue WRITE setHue)
Q_PROPERTY(uchar font READ font WRITE setFont)
Q_PROPERTY(bool hueall READ hueAll WRITE setHueAll)
Q_PROPERTY(bool autosize READ autoSize WRITE setAutoSize)
Q_PROPERTY(enTextAlign align READ align WRITE setAlign)
Q_ENUMS(enTextAlign)
private:
	QString text_;
	unsigned short hue_;
	unsigned char font_;
	enTextAlign align_;
	bool autoSize_;
	bool hueAll_;
	cTexture *texture;
public:
	cAsciiLabel(const QString &text, unsigned short font = 0, unsigned short hue = 0, enTextAlign align = ALIGN_LEFT, bool autoSize = true, bool hueAll = false);
	virtual ~cAsciiLabel();
	void draw(int xoffset, int yoffset);

	// Getters
	const QString &text() const { return text_; }
	unsigned char font() const { return font_; }
	unsigned short hue() const { return hue_; }
	enTextAlign align() const { return align_; }
	bool autoSize() const { return align_; }
	bool hueAll() const { return hueAll_; }

	// Setters
	inline void setText(const QString &text) {
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

	void processDefinitionElement(QDomElement element);
	void processDefinitionAttribute(QString name, QString value);
public slots:
	void update();
};

Q_DECLARE_METATYPE(cAsciiLabel*);

#endif
