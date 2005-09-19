
#if !defined(__GUMPIMAGE_H__)
#define __GUMPIMAGE_H__

#include "control.h"
#include "texture.h"

class cGumpImage : public cControl {
Q_OBJECT
Q_PROPERTY(ushort id READ id WRITE setId)
Q_PROPERTY(ushort hue READ hue WRITE setHue)
Q_PROPERTY(bool partialhue READ partialHue WRITE setPartialHue)
Q_PROPERTY(bool autosize READ autoSize WRITE setAutoSize)

protected:
	bool dirty;
	cTexture *texture;
	unsigned short id_;
	unsigned short hue_;
	bool partialHue_;
	bool autoSize_;
public:
	void update();

	cGumpImage(unsigned short id = 0, unsigned short hue = 0, bool partialHue = false, bool autoSize = true);
	virtual ~cGumpImage();

	void draw(int xoffset, int yoffset);
	void processDefinitionAttribute(QString name, QString value);

public slots:
	bool autoSize() const;
	unsigned short id() const;
	unsigned short hue() const;
	bool partialHue() const;

	void setAutoSize(bool data);
	void setId(unsigned short id);
	void setHue(unsigned short hue);
	void setPartialHue(bool partialhue);
};

inline bool cGumpImage::autoSize() const { return autoSize_; }
inline unsigned short cGumpImage::id() const { return id_; }
inline unsigned short cGumpImage::hue() const { return hue_; }
inline bool cGumpImage::partialHue() const { return partialHue_; }

inline void cGumpImage::setAutoSize(bool data) {
	autoSize_ = data;
	if (texture) {
		int width = texture->realWidth();
		int height = texture->realHeight();
		if (width_ != width || height_ != height) {
			setSize(width, height);
		}
	}
}

inline void cGumpImage::setId(unsigned short id) {
	if (id_ != id) {
		id_ = id;
		dirty = true;
	}
}

inline void cGumpImage::setHue(unsigned short hue) {
	if (hue != hue_) {
		hue_ = hue;
		dirty = true;
	}
}

inline void cGumpImage::setPartialHue(bool partialhue) {
	if (partialhue != partialHue_) {
		partialHue_= partialhue;
		dirty = true;
	}
}

#endif
