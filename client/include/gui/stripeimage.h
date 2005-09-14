
#if !defined(__STRIPEIMAGE_H__)
#define __STRIPEIMAGE_H__

#include "gui/control.h"
#include "gui/container.h"
#include "gui/tiledgumpimage.h"
#include "gui/gumpimage.h"

class cStripeImage : public cContainer {
Q_OBJECT
private:
	bool dirty;

public:
	cStripeImage();
	virtual ~cStripeImage();
	void onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight);	
	void draw(int xoffset, int yoffset);

	ushort id() const;
	ushort hue() const;
	bool partialHue() const;
	bool vertical() const;

	void setId(ushort id);
	void setHue(ushort hue);
	void setPartialHue(bool partialhue);
	void setVertical(bool data);

	void processDefinitionAttribute(QString name, QString value);
protected:
	ushort id_; // The id of the background gump
	ushort hue_; // The hue of the background gump
	bool partialHue_; // Is this gump partial hued?
	bool vertical_;

	cTiledGumpImage *center;
	cGumpImage *left, *right;
	void alignControls();
	void update();	
};

inline bool cStripeImage::vertical() const {
	return vertical_;
}

inline void cStripeImage::setVertical(bool data) {
	if (data != vertical_) {
		vertical_ = data;
		alignControls();
	}
}

inline ushort cStripeImage::id() const {
	return id_;
}

inline ushort cStripeImage::hue() const {
	return hue_;
}

inline bool cStripeImage::partialHue() const {
	return partialHue_;
}


inline void cStripeImage::setId(unsigned short id) {
	if (id_ != id) {
		id_ = id;
		dirty = true;
	}
}

inline void cStripeImage::setHue(unsigned short hue) {
	if (hue != hue_) {
		hue_ = hue;
		dirty = true;
	}
}

inline void cStripeImage::setPartialHue(bool partialhue) {
	if (partialhue != partialHue_) {
		partialHue_= partialhue;
		dirty = true;
	}
}

#endif
