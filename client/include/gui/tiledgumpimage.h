
#if !defined(__TILEDGUMPIMAGE_H__)
#define __TILEDGUMPIMAGE_H__

#include "gui/image.h"

class cTiledGumpImage : public cControl {
protected:
	cTexture *texture;
	unsigned short id_;
	unsigned short hue_;
	bool partialHue_;
public:
	void update();

	cTiledGumpImage(unsigned short id, unsigned short hue = 0, bool partialHue = false);
	virtual ~cTiledGumpImage();

	inline unsigned short id() const { return id_; }
	inline unsigned short hue() const { return hue_; }
	inline bool partialHue() const { return partialHue_; }

	void draw(int xoffset, int yoffset);

	inline void setId(unsigned short id) {
		if (id_ != id) {
			id_ = id;
			if (texture) {
				texture->decref();
				texture = 0;
			}
			invalidate();
		}
	}

	inline void setHue(unsigned short hue) {
		if (hue != hue_) {
			hue_ = hue;
			if (texture) {
				texture->decref();
				texture = 0;
			}
			invalidate();
		}
	}

	inline void setPartialHue(bool partialhue) {
		if (partialhue != partialHue_) {
			partialHue_= partialhue;
			if (texture) {
				texture->decref();
				texture = 0;
			}
			invalidate();
		}
	}
};

#endif
