
#if !defined(__CONTROL_H__)
#define __CONTROL_H__

#include "enums.h"
#include <qevent.h>
#include <QObject>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDomElement>
#include <QMap>

class cDynamicItem;
class cGui;
class cWindow;

/*
	Base class for all gui controls.
*/
class cControl : public QObject {
Q_OBJECT
Q_PROPERTY(int x READ x WRITE setX)
Q_PROPERTY(int y READ y WRITE setY)
Q_PROPERTY(int width READ width WRITE setWidth)
Q_PROPERTY(int height READ height WRITE setHeight)
Q_PROPERTY(cContainer* parent READ parent)
Q_PROPERTY(bool topanchor READ hasTopAnchor WRITE setTopAnchor)
Q_PROPERTY(bool bottomanchor READ hasBottomAnchor WRITE setBottomAnchor)
Q_PROPERTY(bool rightanchor READ hasRightAnchor WRITE setRightAnchor)
Q_PROPERTY(bool leftanchor READ hasLeftAnchor WRITE setLeftAnchor)
Q_PROPERTY(bool visible READ isVisible WRITE setVisible)
Q_PROPERTY(bool movehandle READ isMoveHandle WRITE setMoveHandle)
Q_PROPERTY(bool movable READ isMovable WRITE setMovable)
Q_PROPERTY(uint tabindex READ tabIndex WRITE setTabIndex)
Q_PROPERTY(float alpha READ alpha WRITE setAlpha)
Q_PROPERTY(enControlAlign align READ align WRITE setAlign)

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
	float alpha_;
	QMap<QString, QString> tags; // Custom properties for this control

public slots:
	bool isVisibleOnScreen();
	cWindow *getTopWindow();
	cContainer *parent() const;
	
	// Map a QPoint from the global coordinate space to local
	QPoint mapFromGlobal(const QPoint &point);

	virtual cControl *getControl(int x, int y);

public:	
	// NOTE: This function does not add or remove the control from the parents control array
	virtual void setParent(cContainer *data);

	// Get the next movable control above this
	cControl *getMovableControl();

	// Input focus
	inline bool canHaveFocus() const { return canHaveFocus_; }
	inline bool wantTabs() const { return wantTabs_; }

	// Draw the control using OpenGL
	virtual void draw(int xoffset, int yoffset);

	// This replaces RTTI
	virtual bool isContainer() const;
	virtual bool isWindow() const;

	virtual void requestAlign(); // Request that this control should be realigned
	bool isDisableAlign() const { return disableAlign_; }
	bool needsRealign() const { return needsRealign_; }
	void setDisableAlign(bool data) { disableAlign_ = data; }
	void setNeedsRealign(bool data) { needsRealign_ = data; }

	cControl();
	virtual ~cControl();

	virtual void processDoubleClick(QMouseEvent *e);

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
	virtual void onClick(QMouseEvent *e);

	virtual void processDefinitionElement(QDomElement element);
	virtual void processDefinitionAttribute(QString name, QString value);

public slots:
	// Check if the given item is accepted as a drop
	virtual bool acceptsItemDrop(cDynamicItem *item);
	virtual void dropItem(cDynamicItem *item);

	// Positioning and Bounds
	inline int x() const { return x_; }
	inline int y() const { return y_; }
	inline int width() const { return width_; }
	inline int height() const { return height_; }

	virtual void setX(int data);
	virtual void setY(int data);
	virtual void setWidth(int data);
	virtual void setHeight(int data);
	virtual void setBounds(int x, int y, int width, int height);	
	void setSize(int width, int height);
	void setPosition(int x, int y);

	// Visuals
	void setAlpha(float alpha);
	float alpha() const;
	bool isVisible() const;
	void setVisible(bool data);

	// Tab Index
	uint tabIndex() const;
	void setTabIndex(uint data);

	// Moving
	bool isMovable() const;
	bool isMoveHandle() const;
	void setMovable(bool data);
	void setMoveHandle(bool data);

	// Anchors
	bool hasLeftAnchor() const;
	bool hasTopAnchor() const;
	bool hasRightAnchor() const;
	bool hasBottomAnchor() const;
	void setLeftAnchor(bool data);
	void setTopAnchor(bool data);
	void setRightAnchor(bool data);
	void setBottomAnchor(bool data);

	void setTag(const QString &name, const QString &data);
	QString getTag(const QString &name);
	bool hasTag(const QString &name);

	// Control Align
	enControlAlign align() const;
	virtual void setAlign(enControlAlign align);

signals:	
	void onRightClick(cControl *sender);
	void onDoubleClick(cControl *sender);
};

Q_DECLARE_METATYPE(cControl*);

inline void cControl::setSize(int width, int height) {
	setBounds(x_, y_, width, height);
}

inline void cControl::setPosition(int x, int y) {
	setBounds(x, y, width_, height_);
}

inline bool cControl::isVisible() const {
	return visible_;
}

inline uint cControl::tabIndex() const {
	return tabIndex_;
}

inline bool cControl::isMovable() const {
	return movable_;
}

inline bool cControl::isMoveHandle() const {
	return moveHandle_;
}

inline void cControl::setMovable(bool data) {
	movable_ = data;
}

inline void cControl::setMoveHandle(bool data) {
	moveHandle_ = data;
}

inline bool cControl::hasLeftAnchor() const {
	return (anchors_ & 0x01) != 0;
}

inline bool cControl::hasTopAnchor() const {
	return (anchors_ & 0x02) != 0;
}

inline bool cControl::hasRightAnchor() const {
	return (anchors_ & 0x04) != 0;
}

inline bool cControl::hasBottomAnchor() const {
	return (anchors_ & 0x08) != 0;
}

inline void cControl::setLeftAnchor(bool data) {
	if (data) {
		anchors_ |= 0x01;
	} else {
		anchors_ &= ~0x01;
	}
}

inline void cControl::setTopAnchor(bool data) {
	if (data) {
		anchors_ |= 0x02;
	} else {
		anchors_ &= ~0x02;
	}
}

inline void cControl::setRightAnchor(bool data) {
	if (data) {
		anchors_ |= 0x04;
	} else {
		anchors_ &= ~0x04;
	}
}

inline void cControl::setBottomAnchor(bool data) {
	if (data) {
		anchors_ |= 0x08;
	} else {
		anchors_ &= ~0x08;
	}
}

inline enControlAlign cControl::align() const {
	return align_;
}

inline void cControl::setAlpha(float alpha) {
	alpha_ = alpha;
}

inline float cControl::alpha() const {
	return alpha_;
}

inline cContainer *cControl::parent() const {
	return parent_;
}

#endif
