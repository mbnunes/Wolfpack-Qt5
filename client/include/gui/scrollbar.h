
#if !defined(__SCROLLBAR_H__)
#define __SCROLLBAR_H__

#include "gui/container.h"
#include "gui/imagebutton.h"
#include "gui/bordergump.h"
#include "gui/tiledgumpimage.h"
#include "gui/gumpimage.h"
#include <QtCore>

#undef min
#undef max

class cVerticalScrollbar : public cContainer {
Q_OBJECT
Q_PROPERTY(uint pos READ pos WRITE setPos)
Q_PROPERTY(uint min READ min)
Q_PROPERTY(uint max READ max)
protected:
	cImageButton *btnUp, *btnDown;
	cGumpImage *handle;
	cTiledGumpImage *background;
	bool draggingHandle_;
	int mouseDownY;
	unsigned int min_, max_, pos_;
	float pixelPerStep;
public:
	cVerticalScrollbar(int x, int y, unsigned int height);
	virtual ~cVerticalScrollbar();

	// Get the number of possible positions
	uint getValues() const;

	// Get the height in pixels between the up/down buttons
	int getInnerHeight() const;

	void onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight);

	unsigned int getTrackerYFromPos(int pos);
	unsigned int getPosFromTrackerY(int y);

	virtual cControl *getControl(int x, int y);

	void onMouseDown(QMouseEvent *e);
	void onMouseUp(QMouseEvent *e);
	void onMouseMotion(int xrel, int yrel, QMouseEvent *e);

	virtual void onScroll(int oldpos);

	void processDefinitionAttribute(QString name, QString value);
	void processDefinitionElement(QDomElement element);

public slots:
	void scrollUp(cControl *sender);
	void scrollDown(cControl *sender);

	void setRange(uint min, uint max);
	void setPos(uint data);

	uint pos() const;
	uint min() const;
	uint max() const;

	void setHandleId(ushort id);
	void setBackgroundId(ushort id);
	void setUpButtonIds(ushort unpressed, ushort pressed, ushort hover);
	void setDownButtonIds(ushort unpressed, ushort pressed, ushort hover);

signals:
	void scrolled(int pos);
};

inline uint cVerticalScrollbar::pos() const {
	return pos_;
}

inline uint cVerticalScrollbar::min() const {
	return min_;
}

inline uint cVerticalScrollbar::max() const {
	return max_;
}

inline uint cVerticalScrollbar::getValues() const {
	return qMax<int>(1, max_ - min_);
}

inline int cVerticalScrollbar::getInnerHeight() const {
	return height_ - (btnUp ? btnUp->height() : 0) - (btnDown ? btnDown->height() : 0) - (handle ? handle->height() : 0);
}

inline void cVerticalScrollbar::setRange(unsigned int min, unsigned int max) {
	min_ = min;
	max_ = max;
	pixelPerStep = qMax<float>(0, (float)getInnerHeight() / (float)getValues());
	if (pos_ < min_) {
		setPos(min_);
	} else if(pos_ > max_) {
		setPos(max_);
	}
}

inline void cVerticalScrollbar::setPos(uint data) {
	data = qMin<uint>(max_, qMax<int>(min_, data));		
	if (data != pos_) {
		int oldpos = pos_;
		pos_ = data;
		handle->setY(getTrackerYFromPos(pos_));
		onScroll(oldpos);
	}
}

class cHorizontalScrollbar : public cContainer {
Q_OBJECT
Q_PROPERTY(uint pos READ pos WRITE setPos)
Q_PROPERTY(uint min READ min)
Q_PROPERTY(uint max READ max)

protected:
	cImageButton *btnLeft, *btnRight;
	cGumpImage *handle;
	cBorderGump *background;
	bool draggingHandle_;
	int mouseDownX;
	unsigned int min_, max_, pos_;
	float pixelPerStep;
public:
	cHorizontalScrollbar(int x, int y, unsigned int width);
	virtual ~cHorizontalScrollbar();

	// Get the number of possible positions
	inline unsigned int getValues() {
		return qMax<int>(1, max_ - min_);
	}

	// Get the height in pixels between the left/right buttons
	inline int getInnerWidth() {
		return width_ - (btnLeft ? btnLeft->width() : 0) - (btnRight ? btnRight->width() : 0) - (handle ? handle->width() : 0);
	}

	void onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight);

	unsigned int getTrackerXFromPos(int pos);
	unsigned int getPosFromTrackerX(int x);

	virtual cControl *getControl(int x, int y);

	void onMouseDown(QMouseEvent *e);
	void onMouseUp(QMouseEvent *e);
	void onMouseMotion(int xrel, int yrel, QMouseEvent *e);

	virtual void onScroll(int oldpos);

	void processDefinitionAttribute(QString name, QString value);
	void processDefinitionElement(QDomElement element);
public slots:
	void setHandleId(ushort id);
	void setBackgroundId(ushort id);
	void setLeftButtonIds(ushort unpressed, ushort pressed, ushort hover);
	void setRightButtonIds(ushort unpressed, ushort pressed, ushort hover);

	void scrollLeft(cControl *sender);
	void scrollRight(cControl *sender);

	inline void setRange(unsigned int min, unsigned int max) {
		min_ = min;
		max_ = max;
		pixelPerStep = qMax<float>(0, (float)getInnerWidth() / (float)getValues());
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
			handle->setX(getTrackerXFromPos(pos_));
			onScroll(oldpos);
		}
	}

	inline unsigned int pos() const { return pos_; }
	inline unsigned int min() const { return min_; }
	inline unsigned int max() const { return max_; }

signals:
	void scrolled(int pos);
};

#endif
