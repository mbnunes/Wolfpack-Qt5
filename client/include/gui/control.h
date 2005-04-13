
#if !defined(__CONTROL_H__)
#define __CONTROL_H__

#include "enums.h"
#include <qevent.h>
#include <qobject.h>

class cGui;

/*
	Base class for all gui controls.
*/
class cControl : public QObject {
Q_OBJECT

friend class cGui;
friend class cContainer;
friend class cWindow;
friend class cWorldView;

protected:
	int x_, y_; // Position relative to parent
	int width_, height_; // Height and width of this control, don't need to be writeable
	cContainer *parent_;
	enControlAlign align_;
	bool disableAlign_; // If this control is currently being realigned, this flag is set
	bool needsRealign_; // This flag is used while the control is being realigned
	unsigned char anchors_; // A bitfield for the anchors of this control
	bool visible_; // Default to true
	bool moveHandle_; // If this control is dragged, is the parent form moved instead? Defaults to false.
	bool movable_; // If this control mouse-movable? Defaults to false.
	bool canHaveFocus_; // The control may have the input focus. Defaults to false.
	bool wantTabs_; // This property indicates that the TAB key should not switch focus if this control is active. Defaults to false.
	unsigned int tabIndex_; // The tab index of this control. By defaults it the highest tab index in the parent + 1

public:	
	inline cContainer *parent() { return parent_; }
	// NOTE: This function does not add or remove the control from the parents control array
	virtual void setParent(cContainer *data);

	// Get the next movable control above this
	cControl *getMovableControl();

	// Input focus
	inline bool canHaveFocus() const { return canHaveFocus_; }
	inline bool wantTabs() const { return wantTabs_; }
	inline unsigned int tabIndex() const { return tabIndex_; }
	inline void setTabIndex(unsigned int data) { tabIndex_ = data; }

	// Map a QPoint from the global coordinate space to local
	QPoint mapFromGlobal(const QPoint &point);

	// Visibility
	inline bool isVisible() const { return visible_; }
	void setVisible(bool data);

	virtual cControl *getControl(int x, int y);

	// Draw the control using OpenGL
	virtual void draw(int xoffset, int yoffset);

	// This replaces RTTI
	virtual bool isContainer() const;
	virtual bool isWindow() const;

	// Positioning
	inline int x() const { return x_; }
	inline int y() const { return y_; }
	inline int width() const { return width_; }
	inline int height() const { return height_; }
	
	virtual void setX(int data);
	virtual void setY(int data);
	virtual void setWidth(int data);
	virtual void setHeight(int data);
	virtual void setBounds(int x, int y, int width, int height);	
	inline void setSize(int width, int height) {
		setBounds(x_, y_, width, height);
	}
	inline void setPosition(int x, int y) {
		setBounds(x, y, width_, height_);
	}
	virtual void requestAlign(); // Request that this control should be realigned
	bool isDisableAlign() const { return disableAlign_; }
	bool needsRealign() const { return needsRealign_; }
	void setDisableAlign(bool data) { disableAlign_ = data; }
	void setNeedsRealign(bool data) { needsRealign_ = data; }

	// Moving
	bool isMovable() const { return movable_; }
	bool isMoveHandle() const { return moveHandle_; }
	void setMovable(bool data) { movable_ = data; }
	void setMoveHandle(bool data) { moveHandle_ = data; }

	// Anchors
	bool hasLeftAnchor() const { return (anchors_ & 0x01) != 0; }
	bool hasTopAnchor() const { return (anchors_ & 0x02) != 0; }
	bool hasRightAnchor() const { return (anchors_ & 0x04) != 0; }
	bool hasBottomAnchor() const { return (anchors_ & 0x08) != 0; }
	void setLeftAnchor(bool data) {
		if (data) {
			anchors_ |= 0x01;
		} else {
			anchors_ &= ~0x01;
		}
	}
	void setTopAnchor(bool data) {
		if (data) {
			anchors_ |= 0x02;
		} else {
			anchors_ &= ~0x02;
		}
	}
	void setRightAnchor(bool data) {
		if (data) {
			anchors_ |= 0x04;
		} else {
			anchors_ &= ~0x04;
		}
	}
	void setBottomAnchor(bool data) {
		if (data) {
			anchors_ |= 0x08;
		} else {
			anchors_ &= ~0x08;
		}
	}

	// Getters/Setters
	inline enControlAlign align() const {
		return align_;
	}

	virtual void setAlign(enControlAlign align);

	cControl();
	virtual ~cControl();

	// The parent of this control has moved
	virtual void onParentMoved(int oldx, int oldy);
	virtual void onParentResized(int oldwidth, int oldheight);
	virtual void onChangeBounds(int oldx, int oldy, int oldwidth, int oldheight);
	virtual void onMouseDown(QMouseEvent *e);
	virtual void onMouseUp(QMouseEvent *e);
	virtual void onMouseMotion(int xrel, int yrel, QMouseEvent *e);
	virtual void onMouseEnter();
	virtual void onMouseLeave();
	virtual void onBlur(cControl *newFocus);
	virtual void onFocus(cControl *oldFocus);
	virtual void onKeyDown(QKeyEvent *e);
	virtual void onKeyUp(QKeyEvent *e);
};

#endif
