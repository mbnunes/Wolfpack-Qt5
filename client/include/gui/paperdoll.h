
#if !defined(__PAPERDOLL_H__)
#define __PAPERDOLL_H__

#include "gui/control.h"
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
private:
	bool dirty;
	bool tracking;
	QPoint mouseDownPos;
protected:
	cMobile *owner_;
	bool changeable_;
	cTexture *layers[LAYER_VISIBLECOUNT+1];
	QTimer pickupTimer;	
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
protected slots:
	void pickupItem();
public slots:
	void setOwner(cMobile *owner);
	cMobile *owner() const;
	bool changeable() const;
	void setChangeable(bool changeable);
	void ownerDeleted();
	void update();
	void clear();
	cDynamicItem *itemAtPos(int x, int y);	
};

inline cMobile *cPaperdoll::owner() const {
	return owner_;
}

inline void cPaperdoll::setChangeable(bool changeable) {
	changeable_ = changeable;
}

inline bool cPaperdoll::changeable() const {
	return changeable_;
}

#endif
