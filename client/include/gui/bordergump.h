
#if !defined(__BORDERGUMP_H__)
#define __BORDERGUMP_H__

#include "control.h"
#include "container.h"

class cTiledGumpImage;
class cGumpImage;

class cBorderGump : public cContainer {
Q_OBJECT
private:
	bool dirty;

public:
	cBorderGump(unsigned short id, unsigned short hue = 0, bool partialHue = false);
	cBorderGump();
	virtual ~cBorderGump();
	void onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight);	
	void draw(int xoffset, int yoffset);

	ushort id(uchar i = 0) const;
	ushort hue() const;
	bool partialHue() const;

	void setIds(ushort uleftId, ushort topId, ushort urightId, ushort leftId, ushort centerId, ushort rightId, ushort lleftId, ushort bottomId, ushort lrightId);
	void setId(ushort id);
	void setHue(ushort hue);
	void setPartialHue(bool partialhue);

	void processDefinitionAttribute(QString name, QString value);
protected:
	ushort ids_[9];
	ushort hue_; // The hue of the background gump
	bool partialHue_; // Is this gump partial hued?

	cTiledGumpImage *left, *right, *top, *bottom, *center;
	cGumpImage *uleft, *uright, *lleft, *lright;
	void alignControls();
	void update();	
};

inline ushort cBorderGump::id(uchar i) const {
	return ids_[i % 9];
}

inline ushort cBorderGump::hue() const {
	return hue_;
}

inline bool cBorderGump::partialHue() const {
	return partialHue_;
}


inline void cBorderGump::setId(unsigned short id) {
	for (int i = 0; i < 9; ++i) {
		ids_[i] = id + i;
	}
	dirty = true;
}

inline void cBorderGump::setIds(ushort uleftId, ushort topId, ushort urightId, ushort leftId, ushort centerId, ushort rightId, ushort lleftId, ushort bottomId, ushort lrightId) {
	ids_[0] = uleftId;
	ids_[1] = topId;
	ids_[2] = urightId;
	ids_[3] = leftId;
	ids_[4] = centerId;
	ids_[5] = rightId;
	ids_[6] = lleftId;
	ids_[7] = bottomId;
	ids_[8] = lrightId;
	dirty = true;
}

inline void cBorderGump::setHue(unsigned short hue) {
	if (hue != hue_) {
		hue_ = hue;
		dirty = true;
	}
}

inline void cBorderGump::setPartialHue(bool partialhue) {
	if (partialhue != partialHue_) {
		partialHue_= partialhue;
		dirty = true;
	}
}

#endif
