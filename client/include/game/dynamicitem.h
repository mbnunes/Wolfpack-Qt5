
#if !defined(__DYNAMICITEM_H__)
#define __DYNAMICITEM_H__

#include "game/dynamicentity.h"
#include "game/statictile.h"

class cMobile;
class cContainerGump;

#if defined(Q_CC_MSVC)
#pragma warning(disable: 4250)
#endif

class cDynamicItem : public cDynamicEntity, public cStaticTile {
Q_OBJECT
friend class cContainerGump;

public:
	typedef QVector<cDynamicItem*> Container;

	enum State {
		InLimbo = 0,
		OnMobile,
		InWorld,
        InContainer
	};

	cDynamicItem(unsigned short x, unsigned short y, signed char z, enFacet facet, unsigned int serial);
	cDynamicItem(cDynamicItem *container, unsigned int serial);
	cDynamicItem(cMobile *wearer, unsigned char layer, unsigned int serial);
	virtual ~cDynamicItem();

	void move(unsigned short x, unsigned short y, signed char z);
	virtual void move(cMobile *wearer, unsigned char layer);
	virtual void move(cDynamicItem *container);
	void moveToLimbo(); // Remove from current position and move into nirwana

	void onClick(QMouseEvent *e);
	void onDoubleClick(QMouseEvent *e);

	// Getter
	State positionState() const;
	cDynamicItem *container() const;
	cMobile *wearer() const;
	enLayer layer() const;
	ushort amount() const;
	void setAmount(ushort amount);

	virtual void setId(unsigned short data);
	void setHue(unsigned short data);
	void onMouseEnter();
	void onMouseLeave();

	void cleanPosition(); // Removes the item from its current position and moves it into the limbo

	int lastClickX() const;
	int lastClickY() const;
	void setLastClickX(int data);
	void setLastClickY(int data);
	int containerX() const;
	int containerY() const;
	void setContainerX(int data);
	void setContainerY(int data);
	bool isAlwaysMovable() const;
	void setAlwaysMovable(bool data);
	bool isInvisible() const;
	void setInvisible(bool data);

	const Container &content() const;
	cContainerGump *containerGump() const;
	void showContent(int x, int y, ushort gump);
	void showContent(ushort gump);

	void setDrawx(int data);
	void setDrawy(int data);

	void updatePriority();
	bool canMove() const;

	void draw(int cellx, int celly, int leftClip, int topClip, int rightClip, int bottomClip);
protected:
	State positionState_;
	cDynamicItem *container_; // Container (only valid in Contained state)
	cMobile *wearer_; // The mobile equipping this item (only valid in Equipped state)
	enLayer layer_; // The layer this item is equipped on (only valid in Equipped state)
	Container content_;

	bool alwaysMovable;
	bool invisible;
	ushort amount_;
	int lastClickX_;
	int lastClickY_;
	int containerX_, containerY_; // position within container
	bool deleting; // This is being deleted right now
	void removeItem(cDynamicItem *item);
	cContainerGump *containerGump_; // The container gump associated with this dynamic item
	float getAlpha() const;
};

Q_DECLARE_METATYPE(cDynamicItem*);

inline cDynamicItem *cDynamicItem::container() const {
	return container_;
}

inline cMobile *cDynamicItem::wearer() const {
	return wearer_;
}

inline enLayer cDynamicItem::layer() const {
	return layer_;
}

inline cDynamicItem::State cDynamicItem::positionState() const {
	return positionState_;
}

inline int cDynamicItem::lastClickX() const {
	return lastClickX_;
}

inline int cDynamicItem::lastClickY() const {
	return lastClickY_;
}

inline const cDynamicItem::Container &cDynamicItem::content() const {
	return content_;
}

inline int cDynamicItem::containerX() const {
	return containerX_;
}

inline int cDynamicItem::containerY() const {
	return containerY_;
}

inline void cDynamicItem::setContainerX(int data) {
	containerX_ = data;
}

inline void cDynamicItem::setContainerY(int data) {
	containerY_ = data;
}

inline cContainerGump *cDynamicItem::containerGump() const {
	return containerGump_;
}

inline void cDynamicItem::setDrawx(int data) {
	drawx_ = data;
}

inline void cDynamicItem::setDrawy(int data) {
	drawy_ = data;
}

inline void cDynamicItem::setLastClickX(int data) {
	lastClickX_ = data;
}

inline void cDynamicItem::setLastClickY(int data) {
	lastClickY_ = data;
}

inline ushort cDynamicItem::amount() const {
	return amount_;
}

inline bool cDynamicItem::isAlwaysMovable() const {
	return alwaysMovable;
}

inline void cDynamicItem::setAlwaysMovable(bool data) {
	alwaysMovable = data;
}

inline bool cDynamicItem::isInvisible() const {
	return invisible;
}

inline void cDynamicItem::setInvisible(bool data) {
	invisible = data;
}

#endif
