
#if !defined(__PAPERDOLL_H__)
#define __PAPERDOLL_H__

#include "gui/control.h"
#include "gui/gumpimage.h"
#include "texture.h"
#include <QTimer>

class cMobile;
class cDynamicItem;

/*
	We're not inheriting from cContainer here
	since it wouldn't make much sense and inheriting
	from cControl is a lot faster for lookups and the like.
*/
class cPaperdoll : public cControl {
Q_OBJECT
Q_PROPERTY(cMobile *owner READ owner WRITE setOwner)
Q_PROPERTY(int leftmargin READ leftMargin WRITE setLeftMargin)
Q_PROPERTY(int topmargin READ topMargin WRITE setTopMargin)
private:
	bool dirty;
	bool tracking;
	QPoint mouseDownPos;
	cTexture *background;	
protected:
	cMobile *owner_;
	bool changeable_;
	cTexture *layers[LAYER_VISIBLECOUNT+1];
	QTimer pickupTimer;	
	int leftMargin_, topMargin_;
public:
	cPaperdoll();
	~cPaperdoll();

	void draw(int xoffset, int yoffset);
	cControl *getControl(int x, int y);

	void onClick(QMouseEvent *e);
	void processDoubleClick(QMouseEvent *e);

	void onMouseDown(QMouseEvent *e);
	void onMouseMotion(int xrel, int yrel, QMouseEvent *e);
	void onMouseUp(QMouseEvent *e);
	void onMouseEnter();
	void onMouseLeave();

	void processDefinitionAttribute(QString name, QString value);
protected slots:
	void pickupItem();
public slots:
	bool acceptsItemDrop(cDynamicItem *item);
	void dropItem(cDynamicItem *item);

	void setOwner(cMobile *owner);
	cMobile *owner() const;
	bool changeable() const;
	void setChangeable(bool changeable);
	void ownerDeleted();
	void update();
	void clear();
	cDynamicItem *itemAtPos(int x, int y);
	void invalidate();

	void setLeftMargin(int data);
	void setTopMargin(int data);
	int leftMargin() const;
	int topMargin() const;

	void setBackground(ushort gump, ushort hue = 0, bool partialHue = false);
};

inline void cPaperdoll::invalidate() {
	dirty = true;
}

inline cMobile *cPaperdoll::owner() const {
	return owner_;
}

inline void cPaperdoll::setChangeable(bool changeable) {
	changeable_ = changeable;
}

inline bool cPaperdoll::changeable() const {
	return changeable_;
}

inline void cPaperdoll::setLeftMargin(int data) {
	leftMargin_ = data;
}

inline void cPaperdoll::setTopMargin(int data) {
	topMargin_ = data;
}

inline int cPaperdoll::leftMargin() const {
	return leftMargin_;
}

inline int cPaperdoll::topMargin() const {
	return topMargin_;
}

#endif
