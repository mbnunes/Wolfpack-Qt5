
#if !defined(__MULTI_H__)
#define __MULTI_H__

#include "game/dynamicitem.h"
#include "game/statictile.h"

#include <QVector>

class cMultiItem;

class cMulti : virtual public cDynamicItem {
protected:
	QVector<cMultiItem*> tiles_;
public:
	cMulti(unsigned short x, unsigned short y, signed char z, enFacet facet, unsigned int serial);
	virtual ~cMulti();

	QVector<cMultiItem*> tiles() const;

	void setId(ushort data);
	void draw(int cellx, int celly, int leftClip, int topClip, int rightClip, int bottomClip);
	void refreshTiles();
	void refreshTiles(ushort x, ushort y);
	void removeTile(cMultiItem *tile);

	void move(ushort x, ushort y, signed char z);
};

class cMultiItem : virtual public cStaticTile {
protected:
	cMulti *multi_;
public:
	cMultiItem(ushort x, ushort y, signed char z, enFacet facet);
	~cMultiItem();

	cMulti *multi() const;
	void setMulti(cMulti *multi);
};

inline QVector<cMultiItem*> cMulti::tiles() const {
	return tiles_;
}

inline cMulti *cMultiItem::multi() const {
	return multi_;
}

inline void cMultiItem::setMulti(cMulti *multi) {
	multi_ = multi;
}

#endif
