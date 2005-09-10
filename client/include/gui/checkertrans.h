
#if !defined(__CHECKERTRANS_H__)
#define __CHECKERTRANS_H__

#include "gui/control.h"
#include "texture.h"

class cCheckerTrans : public cControl {
Q_OBJECT
protected:
	bool enabled_;
	cTexture *checkerboard;
	void generateCheckerboard();
public:
	cCheckerTrans(bool enabled);
	~cCheckerTrans();
	void draw(int xoffset, int yoffset);

	void onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight);
	void setEnabled(bool enabled);
	bool enabled() const;
};

inline void cCheckerTrans::setEnabled(bool enabled) {
	enabled_ = enabled;
}

inline bool cCheckerTrans::enabled() const {
	return enabled_;
}

#endif
