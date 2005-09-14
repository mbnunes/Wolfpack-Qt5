
#if !defined(__SCROLLBAR_H__)
#define __SCROLLBAR_H__

#include "container.h"
#include "imagebutton.h"
#include "tiledgumpimage.h"
#include "gumpimage.h"
#include <QtCore>

class cVerticalScrollBar;

typedef void (*fnScrollbarScrolledEvent)(cVerticalScrollBar *scrollbar, int oldpos);

class cVerticalScrollBar : public cContainer {
Q_OBJECT

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
		return qMax<int>(1, max_ - min_);
	}

	// Get the height in pixels between the up/down buttons
	inline int getInnerHeight() {
		return height_ - (btnUp ? btnUp->height() : 0) - (btnDown ? btnDown->height() : 0) - (handle ? handle->height() : 0);
	}

	void onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight);

	inline void setRange(unsigned int min, unsigned int max) {
		min_ = min;
		max_ = max;
		pixelPerStep = qMax<float>(0, (float)getInnerHeight() / (float)getValues());
		if (pos_ < min_) {
			setPos(min_);
		} else if(pos_ > max_) {
			setPos(max_);
		}
	}

	inline void setPos(unsigned int data) {
		data = qMin<uint>(max_, qMax<int>(min_, data));		
		if (data != pos_) {
			int oldpos = pos_;
			pos_ = data;
			handle->setY(getTrackerYFromPos(pos_));
			onScroll(oldpos);
		}
	}

	#undef min
	#undef max
	inline unsigned int pos() const { return pos_; }
	inline unsigned int min() const { return min_; }
	inline unsigned int max() const { return max_; }

	unsigned int getTrackerYFromPos(int pos);
	unsigned int getPosFromTrackerY(int y);

	virtual cControl *getControl(int x, int y);

	void onMouseDown(QMouseEvent *e);
	void onMouseUp(QMouseEvent *e);
	void onMouseMotion(int xrel, int yrel, QMouseEvent *e);

	void setHandleId(ushort id);
	void setBackgroundId(ushort id);
	void setUpButtonIds(ushort unpressed, ushort pressed, ushort hover);
	void setDownButtonIds(ushort unpressed, ushort pressed, ushort hover);

	virtual void onScroll(int oldpos);

public slots:
	void scrollUp(cControl *sender);
	void scrollDown(cControl *sender);

signals:
	void scrolled(int pos);
};

#endif
