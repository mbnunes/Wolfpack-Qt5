
// Library Includes
#include "qstring.h"

// Wolfpack Includes
#include "sectors.h"
#include "uobject.h"
#include "wpconsole.h"
#include "globals.h"
#include "exceptions.h"
#include "world.h"
#include "items.h"
#include "basechar.h"

cSectorMap::cSectorMap()
{
	grid = 0;
	gridHeight = 0;
	gridWidth = 0;
}

cSectorMap::~cSectorMap()
{
	for( unsigned int i = 0; i < gridHeight * gridWidth; ++i )
	{
		if( grid[i] )
		{
			delete grid[i]->data;
			delete grid[i];
		}
	}
}

unsigned int cSectorMap::calcBlockId( unsigned int x, unsigned int y ) throw()
{
	return ( ( x / SECTOR_SIZE ) * gridHeight ) + ( x / SECTOR_SIZE );
}

bool cSectorMap::init( unsigned int width, unsigned int height ) throw()
{
	if( width == 0 || height == 0 )
	{
		clConsole.log( LOG_ERROR, QString( "Invalid Sectormap boundaries (Width: %1, Height: %2)." ).arg( width ).arg( height ) );
		return false;
	}

	gridHeight = height;
	gridWidth = width;

	grid = new stSector*[ gridHeight * gridWidth ];
	memset( grid, 0, sizeof( stSector* ) * gridHeight * gridWidth );
	return true;
}

bool cSectorMap::addItem( unsigned short x, unsigned short y, cUObject *object ) throw()
{
	if( x >= gridWidth * SECTOR_SIZE || y >= gridHeight * SECTOR_SIZE )
	{
		error_ = "X or Y is out of the grid boundaries.";
		return false;
	}

	// Calculate the block id
	unsigned int block = calcBlockId( x, y );

	if( !grid[block] )
	{
		grid[block] = new stSector;
		grid[block]->count = 0;
		grid[block]->data = new cUObject*[0];
	}

	// Append our item to the array
	grid[block]->count++;
	grid[block]->data = (cUObject**)realloc( grid[block]->data, grid[block]->count * sizeof( cUObject* ) );
	grid[block]->data[ grid[block]->count - 1 ] = object;

	return true;
}

bool cSectorMap::removeItem( unsigned short x, unsigned short y, cUObject *object ) throw()
{
	if( x >= gridWidth * SECTOR_SIZE || y >= gridHeight * SECTOR_SIZE )
	{
		error_ = "X or Y is out of the grid boundaries.";
		return false;
	}

	unsigned int block = calcBlockId( x, y );
	
	// Seems like the item is already gone
	if( !grid[block] )
		return true;

	// Remove the Item from the array (most complicated part of the code)
	for( unsigned int i = 0; i < grid[block]->count; ++i )
	{
		if( grid[block]->data[i] == object )
		{
			// We found our object. Create a new array and copy the rest over
			cUObject **newData = new cUObject*[ grid[block]->count - 1 ];
			
			memcpy( newData, grid[block]->data, sizeof( cUObject* ) * i );
			memcpy( newData + ( i * sizeof( cUObject* ) ), grid[block]->data + sizeof( cUObject* ) * i, sizeof( cUObject* ) * ( grid[block]->count - ++i ) );
			grid[block]->count--;
		}
	}

	// Check if the block can be freed
	if( !grid[block]->count )
	{
		delete grid[block]->data;
		delete grid[block];
		grid[block] = 0; // This makes *sure* it gets resetted to 0
	}

	return true;
}
	
bool cSectorMap::countItems( unsigned int x, unsigned int y, unsigned int &count ) throw()
{
	if( x >= gridWidth * SECTOR_SIZE || y >= gridHeight * SECTOR_SIZE )
	{
		error_ = "X or Y is out of the grid boundaries.";
		return false;
	}

	unsigned int block = calcBlockId( x, y );
	
	if( !grid[block] )
		count = 0;
	else
		count = grid[block]->count;
	
	return true;
}

bool cSectorMap::getItems( unsigned int x, unsigned int y, cUObject **items ) throw()
{
	if( x >= gridWidth * SECTOR_SIZE || y >= gridHeight * SECTOR_SIZE )
	{
		error_ = "X or Y is out of the grid boundaries.";
		return false;
	}

	unsigned int block = calcBlockId( x, y );
	
	if( grid[block] )
		memcpy( items, grid[block]->data, sizeof( cUObject* ) * grid[block]->count );

	return true;
}

cSectorIterator *cSectorMap::getBlock( unsigned int x, unsigned int y ) throw()
{
	if( x >= gridWidth * SECTOR_SIZE || y >= gridHeight * SECTOR_SIZE )
	{
		error_ = "X or Y is out of the grid boundaries.";
		return 0;
	}

	unsigned int count = 0;
	cUObject **items = 0;

	countItems( x, y, count );

	if( count > 0 )
	{
		items = new cUObject* [ count ];
		getItems( x, y, items );
	}

	return new cSectorIterator( count, items );
}

/*
 * cSectorIterator
 */

cSectorIterator::cSectorIterator( unsigned int count, cUObject **items )
{
	this->items = items;
	this->count = count;
	this->pos = 0;
}

cSectorIterator::~cSectorIterator()
{
	delete this->items;
}

cUObject *cSectorIterator::first()
{
	this->pos = 0;
	return next();
}

cUObject *cSectorIterator::next()
{
	// Actually this returns the current position
	// And advances to the next
	if( this->pos >= this->count )
		return 0;

	return this->items[ this->pos++ ];
}

/*
 *	cSectorMaps
 */

cSectorMaps::cSectorMaps()
{
}

cSectorMaps::~cSectorMaps()
{
}

void cSectorMaps::addMap( unsigned char map, unsigned int width, unsigned int height )
{
	// Create a map in the Char and in the Item map
	cSectorMap *itemmap = new cSectorMap;
	
	if( !itemmap->init( width, height ) )
	{
		throw wpException( QString( itemmap->error() ) );
		delete itemmap;
	}

	cSectorMap *charmap = new cSectorMap;

	if( !charmap->init( width, height ) )
	{
		throw wpException( QString( charmap->error() ) );
		delete charmap;
		delete itemmap;
	}

	charmaps.insert( std::make_pair( map, charmap ) );
	itemmaps.insert( std::make_pair( map, itemmap ) );
}
	
void cSectorMaps::add( const Coord_cl &pos, cItem *pItem )
{
	std::map< unsigned char, cSectorMap* >::const_iterator it = itemmaps.find( pos.map );

	if( it == itemmaps.end() )
		throw QString( "Couldn't find a map with the id %1." ).arg( pos.map );
	
	it->second->addItem( pos.x, pos.y, (cUObject*)pItem );
}

void cSectorMaps::add( const Coord_cl &pos, cBaseChar *pChar )
{
	std::map< unsigned char, cSectorMap* >::const_iterator it = charmaps.find( pos.map );

	if( it == charmaps.end() )
		throw QString( "Couldn't find a map with the id %1." ).arg( pos.map );
	
	it->second->addItem( pos.x, pos.y, (cUObject*)pChar );
}

void cSectorMaps::remove( const Coord_cl &pos, cItem *pItem )
{
	std::map< unsigned char, cSectorMap* >::const_iterator it = itemmaps.find( pos.map );

	if( it == itemmaps.end() )
		throw QString( "Couldn't find a map with the id %1." ).arg( pos.map );
	
	it->second->removeItem( pos.x, pos.y, (cUObject*)pItem );
}

void cSectorMaps::remove( const Coord_cl &pos, cBaseChar *pChar )
{
	std::map< unsigned char, cSectorMap* >::const_iterator it = charmaps.find( pos.map );

	if( it == charmaps.end() )
		throw QString( "Couldn't find a map with the id %1." ).arg( pos.map );
	
	it->second->removeItem( pos.x, pos.y, (cUObject*)pChar );
}

void cSectorMaps::add( cUObject *object )
{
	// Very powerful statement. It completely
	// annihilates the need to check for 
	// nullpointers in our object-map
	if( !object )
		return;

	if( isItemSerial( object->serial() ) )
	{
		P_ITEM pItem = dynamic_cast< P_ITEM >( object );
		if( pItem )
			add( pItem->pos(), pItem );
	}
	else if( isCharSerial( object->serial() ) )
	{
		P_CHAR pChar = dynamic_cast< P_CHAR >( object );
		if( pChar )
			add( pChar->pos(), pChar );
	}
}

void cSectorMaps::remove( cUObject *object )
{
	// Very powerful statement. It completely
	// annihilates the need to check for 
	// nullpointers in our object-map
	if( !object )
		return;

	if( isItemSerial( object->serial() ) )
	{
		P_ITEM pItem = dynamic_cast< P_ITEM >( object );
		if( pItem )
			remove( pItem->pos(), pItem );
	}
	else if( isCharSerial( object->serial() ) )
	{
		P_CHAR pChar = dynamic_cast< P_CHAR >( object );
		if( pChar )
			remove( pChar->pos(), pChar );
	}
}
