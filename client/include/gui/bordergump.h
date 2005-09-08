
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

	ushort id() const;
	ushort hue() const;
	bool partialHue() const;

	void setId(ushort id);
	void setHue(ushort hue);
	void setPartialHue(bool partialhue);

	void processDefinitionAttribute(QString name, QString value);
protected:
	ushort id_; // The id of the background gump
	ushort hue_; // The hue of the background gump
	bool partialHue_; // Is this gump partial hued?

	cTiledGumpImage *left, *right, *top, *bottom, *center;
	cGumpImage *uleft, *uright, *lleft, *lright;
	void alignControls();
	void update();	
};

inline ushort cBorderGump::id() const { return id_; }
inline ushort cBorderGump::hue() const { return hue_; }
inline bool cBorderGump::partialHue() const { return partialHue_; }

inline void cBorderGump::setId(unsigned short id) {
	if (id_ != id) {
		id_ = id;
		dirty = true;
	}
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
