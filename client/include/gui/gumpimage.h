
#if !defined(__GUMPIMAGE_H__)
#define __GUMPIMAGE_H__

#include "control.h"
#include "texture.h"

class cGumpImage : public cControl {
Q_OBJECT

protected:
	float alpha_;
	bool dirty;
	cTexture *texture;
	unsigned short id_;
	unsigned short hue_;
	bool partialHue_;
	bool autoSize_;
public:
	void update();

	cGumpImage(unsigned short id, unsigned short hue = 0, bool partialHue = false, bool autoSize = true);
	virtual ~cGumpImage();

	void setAlpha(float alpha);
	float alpha() const;

	inline bool autoSize() const { return autoSize_; }
	inline unsigned short id() const { return id_; }
	inline unsigned short hue() const { return hue_; }
	inline bool partialHue() const { return partialHue_; }

	void draw(int xoffset, int yoffset);

	inline void setAutoSize(bool data) {
		autoSize_ = data;
		if (texture) {
			int width = texture->realWidth();
			int height = texture->realHeight();
			if (width_ != width || height_ != height) {
				setSize(width, height);
			}
		}
	}

	inline void setId(unsigned short id) {
		if (id_ != id) {
			id_ = id;
			dirty = true;
		}
	}

	inline void setHue(unsigned short hue) {
		if (hue != hue_) {
			hue_ = hue;
			dirty = true;
		}
	}

	inline void setPartialHue(bool partialhue) {
		if (partialhue != partialHue_) {
			partialHue_= partialhue;
			dirty = true;
		}
	}
};

inline void cGumpImage::setAlpha(float alpha) {
	alpha_ = alpha;
}

inline float cGumpImage::alpha() const {
	return alpha_;
}

#endif
