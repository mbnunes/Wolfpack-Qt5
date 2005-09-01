
#if !defined(__WORLD_H__)
#define __WORLD_H__

#include "game/entity.h"
#include "game/dynamicentity.h"
#include "enums.h"
#include "vector.h"

#include <QObject>
#include <QTimer>
#include <QMap>
#include <qlist.h>
#include <q3intcache.h>
#include <qevent.h>
#include <qvector.h>
//Added by qt3to4:
#include <QMouseEvent>

class cMobile;
class cDynamicItem;

// I'm using a class here because of the destructor
class stGroundInfo {
public:
	cVector normals[4];
	int left, right, bottom, z;
};

// This is an internal data structure representing blocking items
class cBlockItem {
public:
	signed char z;
	uchar height;
	bool walkable;
	bool maptile;

	cBlockItem() : z( -128 ), height( 0 ), walkable( false ), maptile( false ) {}

	bool operator<( const cBlockItem &b ) const
	{
		// If the items have the same top, the one with the surface flag has precedence
		int itemTopA = height + z;
		int itemTopB = b.height + b.z;

		if ( itemTopA == itemTopB )
		{
			if ( height == 0 && walkable )
			{
				return true;
			}

			if ( b.height == 0 && b.walkable )
			{
				return false;
			}
		}

		return ( itemTopA > itemTopB );
	}
};

class cWorld : QObject {
Q_OBJECT
protected:
	// Center of our world
	unsigned short x_, y_;
	signed char z_;
	int roofCap_; // Only tiles with a z value smaller than this will be drawn
	enFacet facet_;
	cEntity *mouseOver_;
	Q3IntCache<stGroundInfo> groundCache;

	// Calculate a cell id for the cell map
	unsigned int getCellId(unsigned short x, unsigned short y) const;

	// Typedef for the entity map.
	typedef QList<cEntity*> Cell;
	typedef Cell::iterator CellIterator;
	typedef Cell::const_iterator ConstCellIterator;
	typedef QMap<unsigned int, Cell> Container;
	typedef Container::iterator Iterator;
	typedef Container::const_iterator ConstIterator;
	Container entities;

	// Load a specific cell from the given coordinates
	void loadCell(unsigned short x, unsigned short y);

	// Delete all entities on the given x,y coordinate
	void cleanupEntities();

	cEntity *getEntity(int x, int y);

	// Serial->Dynamic Map
	QMap<unsigned int, cDynamicEntity*> dynamics;

	// Draw x,y offsets and timeout
	int drawxoffset, drawyoffset;
	unsigned int smoothMoveEnd_;
	unsigned int smoothMoveStart_;
	int drawCut;
	QTimer roofTimer;
public:
	cWorld();
	~cWorld();

	// This automatically uses the same facet as the world
	void getGroundInfo(int x, int y, stGroundInfo *info);
	void clearGroundCache();

	unsigned short x() const;
	unsigned short y() const;
	signed char z() const;
	enFacet facet() const;
	cEntity *mouseOver() const;
	int roofCap() const;
	void setRoofCap(int data);	

	// This function moves the center of our world to the new
	// coordinates and loads the static tiles for the given location.
	// Fresh indicates that not only the changes should be reloaded.
	void moveCenter(unsigned short x, unsigned short y, signed char z, bool fresh = false);

	// Smooth Move the World by the given x/y coordinates
	void smoothMove(int x, int y, int zdiff, uint duration);

	// Changing the facet is special
	void changeFacet(enFacet facet);

	// Draw the current world to the given surface with the given height/width
	void draw(int x, int y, int width, int height);

	// Add the given entity to the world. It will be automatically sorted into the entity
	// array.
	void addEntity(cEntity *entity);
	void removeEntity(cEntity *entity);
	void clearEntities(); // Clear all entities

	// Notify the world of clicks relative to the upper left corner of the view
	void onClick(QMouseEvent *e);
	void onDoubleClick(QMouseEvent *e);

	// Methods for managing dynamics
	cDynamicEntity *findDynamic(unsigned int serial) const;
	cDynamicItem *findItem(unsigned int serial) const;
	cMobile *findMobile(unsigned int serial) const;
	void registerDynamic(cDynamicEntity *entity);
	void unregisterDynamic(cDynamicEntity *entity);

	QVector<cBlockItem> getBlockingItems(cMobile *mobile, ushort posx, ushort posy);
	bool mayWalk(cMobile *mobile, ushort posx, ushort posy, signed char &posz);
private:
	bool cleaningUp; // If the World is cleaning up entities, this blocks removeEntity() from doing anything
public slots:
	void checkRoofs();
};

inline unsigned int cWorld::getCellId(unsigned short x, unsigned short y) const {
	return (x << 16) + y;
}

inline cDynamicEntity *cWorld::findDynamic(unsigned int serial) const {
	QMap<unsigned int, cDynamicEntity*>::const_iterator it;
	it = dynamics.find(serial);
	if (it == dynamics.end()) {
		return 0;
	} else {
		return *it;
	}
}

inline void cWorld::registerDynamic(cDynamicEntity *entity) {
	dynamics.insert(entity->serial(), entity);
}

inline void cWorld::unregisterDynamic(cDynamicEntity *entity) {
	dynamics.remove(entity->serial());
}

inline unsigned short cWorld::x() const {
	return x_;
}

inline unsigned short cWorld::y() const {
	return y_;
}

inline signed char cWorld::z() const {
	return z_;
}

inline enFacet cWorld::facet() const {
	return facet_;
}

inline int cWorld::roofCap() const {
	return roofCap_;
}

inline void cWorld::setRoofCap(int data) {
	roofCap_ = data;
}

inline cEntity *cWorld::mouseOver() const {
	return mouseOver_;
}

extern cWorld *World;

#endif
