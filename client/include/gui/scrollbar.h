
#if !defined(__SCROLLBAR_H__)
#define __SCROLLBAR_H__

#include "container.h"
#include "imagebutton.h"
#include "tiledgumpimage.h"
#include "gumpimage.h"

class cVerticalScrollBar;

typedef void (*fnScrollbarScrolledEvent)(cVerticalScrollBar *scrollbar, int oldpos);

class cVerticalScrollBar : public cContainer {
protected:
	cImageButton *btnUp, *btnDown;
	cGumpImage *handle;
	cTiledGumpImage *background;
	bool draggingHandle_;
	int mouseDownY;
	unsigned int min_, max_, pos_;
	float pixelPerStep;
	
	fnScrollbarScrolledEvent scrollCallback_;
public:
	cVerticalScrollBar(int x, int y, unsigned int height);
	virtual ~cVerticalScrollBar();

	// callback system
	inline fnScrollbarScrolledEvent scrollCallback() const { return scrollCallback_; }
	inline void setScrollCallback(fnScrollbarScrolledEvent callback) {
		scrollCallback_ = callback;
	}

	// Get the number of possible positions
	inline unsigned int getValues() {
		return max_ - min_ + 1;
	}

	// Get the height in pixels between the up/down buttons
	inline unsigned int getInnerHeight() {
		return height_ - btnUp->height() - btnDown->height() - handle->height();
	}

	inline void setRange(unsigned int min, unsigned int max) {
		min_ = min;
		max_ = max;
		pixelPerStep = (float)getInnerHeight() / (float)getValues();
		if (pos_ < min_) {
			setPos(min_);
		} else if(pos_ > max_) {
			setPos(max_);
		}
		invalidate();
	}

	inline void setPos(unsigned int data) {
		data = QMIN(max_, QMAX(min_, data));		
		if (data != pos_) {
			int oldpos = pos_;
			pos_ = data;
			handle->setY(getTrackerYFromPos(pos_));
			invalidate();
			onScroll(oldpos);
		}
	}

	inline unsigned int pos() const { return pos_; }
	inline unsigned int min() const { return min_; }
	inline unsigned int max() const { return max_; }

	unsigned int getTrackerYFromPos(int pos);
	unsigned int getPosFromTrackerY(int y);

	virtual cControl *getControl(int x, int y);

	void onMouseDown(int x, int y, unsigned char button, bool pressed);
	void onMouseUp(int x, int y, unsigned char button, bool pressed);
	void onMouseMotion(int xrel, int yrel, unsigned char buttons);

	virtual void onScroll(int oldpos);
};

#endif
