
#if !defined(__DYNAMICITEM_H__)
#define __DYNAMICITEM_H__

#include "game/dynamicentity.h"
#include "game/statictile.h"

class cMobile;

#pragma warning(disable: 4250)

class cDynamicItem : public cDynamicEntity, public cStaticTile {
public:
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

	void setId(unsigned short data);
	void setHue(unsigned short data);

	void cleanPosition(); // Removes the item from its current position and moves it into the limbo
protected:
	State positionState_;
	cDynamicItem *container_; // Container (only valid in Contained state)
	cMobile *wearer_; // The mobile equipping this item (only valid in Equipped state)
	enLayer layer_; // The layer this item is equipped on (only valid in Equipped state)
};


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

#endif
