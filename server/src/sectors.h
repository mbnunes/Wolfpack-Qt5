#if !defined( __SECTORS_H__ )
#define __SECTORS_H__

#include <map>
#include "singleton.h"

class Coord_cl;
class cUObject;
class cItem;
class cBaseChar;

// 8 uses more memory but is faster
// 16 uses a lot less memory but is slower
// If you change this, you need to change init as well
#define SECTOR_SIZE 8

// In reality these hold a list of items you can iterate trough
// So they *can* contain more than one sector!
class cSectorIterator
{
private:
	cUObject **items;
	unsigned int count;
	unsigned int pos;

public:
	cSectorIterator( unsigned int count, cUObject **items );
	virtual ~cSectorIterator();

	cUObject *first();
	cUObject *next();
};

class cSectorMaps;

class cSectorMap
{
	friend class cSectorMaps;

protected:
	struct stSector
	{
		unsigned int count;
		cUObject **data;
	};
	
	stSector **grid;
	unsigned int gridWidth;
	unsigned int gridHeight;
	char *error_;
	
	unsigned int calcBlockId( unsigned int x, unsigned int y ) throw();

	bool addItem( unsigned short x, unsigned short y, cUObject *object ) throw();
	bool removeItem( unsigned short x, unsigned short y, cUObject *object ) throw();
	
	bool countItems( unsigned int x, unsigned int y, unsigned int &count ) throw();
	bool getItems( unsigned int x, unsigned int y, cUObject **items ) throw();
public:
	cSectorMap();
	virtual ~cSectorMap();
	
	bool init( unsigned int width, unsigned int height ) throw();
	const char *error() const { return error_; }

	// Get an iterator for one specific block
	cSectorIterator *getBlock( unsigned int x, unsigned int y ) throw();
};

// Proxy Class
class cSectorMaps
{
private:
	std::map< unsigned char, cSectorMap* > itemmaps;
	std::map< unsigned char, cSectorMap* > charmaps;

	void remove( const Coord_cl &pos, cItem *pItem );
	void remove( const Coord_cl &pos, cBaseChar *pChar );

	void add( const Coord_cl &pos, cItem *pItem );
	void add( const Coord_cl &pos, cBaseChar *pChar );
public:
	cSectorMaps();
	virtual ~cSectorMaps();

	void addMap( unsigned char map, unsigned int width, unsigned int height );
	void add( cUObject *object );
	void remove( cUObject *object );
};

typedef SingletonHolder< cSectorMaps > SectorMaps;

#endif
