/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In addition to that license, if you are running this program or modified
 * versions of it on a public system you HAVE TO make the complete source of
 * the version used by you available or provide people with a location to
 * download it.
 *
 * Wolfpack Homepage: http://wpdev.sf.net/
 */

#if !defined( __SECTORS_H__ )
#define __SECTORS_H__

#include <map>
#include "singleton.h"
#include "server.h"

class Coord_cl;
class cUObject;
class cItem;
class cBaseChar;

// In reality these hold a list of items you can iterate trough
// So they *can* contain more than one sector!
class cSectorIterator
{
	friend class RegionIterator4Chars;
	friend class RegionIterator4Items;

protected:
	cUObject **items;
	unsigned int count;
	unsigned int pos;

public:
	cSectorIterator( unsigned int count, cUObject **items );
	virtual ~cSectorIterator();
};

class cItemSectorIterator: public cSectorIterator
{
public:
	cItemSectorIterator( unsigned int count, cUObject **items ): cSectorIterator( count, items ) {}

	cItem *first();
	cItem *next();
};

class cCharSectorIterator: public cSectorIterator
{
public:
	cCharSectorIterator( unsigned int count, cUObject **items ): cSectorIterator( count, items ) {}

	cBaseChar *first();
	cBaseChar *next();
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
	unsigned int gridWidth_;
	unsigned int gridHeight_;
	char *error_;

	bool addItem( cUObject *object );
	bool removeItem( cUObject *object );

	unsigned int countItems( unsigned int id ); // Returns the count of items in a specific block
	unsigned int getItems( unsigned int id, cUObject **items ); // Returns the count of items copied
public:
	cSectorMap();
	virtual ~cSectorMap();

	unsigned int gridHeight() const;
	unsigned int gridWidth() const;

	bool init( unsigned int width, unsigned int height );
	const char *error() const { return error_; }

	// Get an iterator for one specific block
	unsigned int calcBlockId( unsigned int x, unsigned int y );
};

// Manager Class for cSectorMap
class cSectorMaps : public cComponent {
private:
	std::map< unsigned char, cSectorMap* > itemmaps;
	std::map< unsigned char, cSectorMap* > charmaps;

	void remove( const Coord_cl &pos, cItem *pItem );
	void remove( const Coord_cl &pos, cBaseChar *pChar );

	// Internal functions for searching items
	enum MapType
	{
		MT_CHARS,
		MT_ITEMS
	};

	cSectorIterator *findObjects( MapType type, cSectorMap *map, int x, int y );
	cSectorIterator *findObjects( MapType type, cSectorMap *map, int x1, int y1, int x2, int y2 );
public:
	void load();
	void unload();
	void reload();

	cSectorMaps();
	virtual ~cSectorMaps();

	void addMap( unsigned char map, unsigned int width, unsigned int height );
	void add( cUObject *object );
	void remove( cUObject *object );
	bool validMap(unsigned char map);

	// Find Methods
	// These methods help in finding items on the map, either in a specific block
	// or by a center coordinate and a distance parameter
	// or inside of a rectangle defined by x1,y1 and x2,y2
	cItemSectorIterator *findItems( unsigned char map, int x, int y ); // Find items in a specific block
	cItemSectorIterator *findItems( unsigned char map, int x1, int y1, int x2, int y2 );
	cItemSectorIterator *findItems( const Coord_cl &center, unsigned char distance );

	cCharSectorIterator *findChars( unsigned char map, int x, int y ); // Find items in a specific block
	cCharSectorIterator *findChars( unsigned char map, int x1, int y1, int x2, int y2 );
	cCharSectorIterator *findChars( const Coord_cl &center, unsigned char distance );
};

//typedef SingletonHolder< cSectorMaps > SectorMaps;
typedef SingletonHolder< cSectorMaps > SectorMaps;
#define MapObjects SectorMaps

// These are deprecated Replacement Classes, but they should help during the transition
class RegionIterator4Chars
{
private:
	cCharSectorIterator *iter;
public:
	RegionIterator4Chars( const Coord_cl &pos, unsigned int distance = 18 );
	virtual ~RegionIterator4Chars();

	void		Begin( void );
	bool		atEnd( void ) const;
	cBaseChar	*GetData( void );
	RegionIterator4Chars& operator++( int );
};

class RegionIterator4Items
{
private:
	cItemSectorIterator *iter;
public:
	RegionIterator4Items( const Coord_cl &pos, unsigned int distance = 18 );
	virtual ~RegionIterator4Items();

	void		Begin( void );
	bool		atEnd( void ) const;
	cItem		*GetData( void );

	// Operators
	RegionIterator4Items& operator++( int );
};

#endif
