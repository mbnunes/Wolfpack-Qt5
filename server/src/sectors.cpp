
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

#include <math.h>

// 8 uses more memory but is faster
// 16 uses a lot less memory but is slower
// The outside world does not know about this setting. 
// So take care!
#define SECTOR_SIZE 8

cSectorMap::cSectorMap()
{
	grid = 0;
	gridHeight_ = 0;
	gridWidth_ = 0;
}

cSectorMap::~cSectorMap()
{
	for( unsigned int i = 0; i < gridHeight_ * gridWidth_; ++i )
	{
		if( grid[i] )
		{
			free( grid[i]->data );
			delete grid[i];
		}
	}
}

#undef realloc

inline void *realloc( void *ptr, unsigned int size )
{
	void *newPtr = malloc( size );
	memcpy( newPtr, ptr, size );
	free( ptr );
	return newPtr;
}

unsigned int cSectorMap::gridHeight() const
{
	return gridHeight_;
}

unsigned int cSectorMap::gridWidth() const
{
	return gridWidth_;
}

unsigned int cSectorMap::calcBlockId( unsigned int x, unsigned int y )
{
	return ( ( x / SECTOR_SIZE ) * gridHeight_ ) + ( y / SECTOR_SIZE );
}

bool cSectorMap::init( unsigned int width, unsigned int height )
{
	if( width == 0 || height == 0 )
	{
		clConsole.log( LOG_ERROR, QString( "Invalid Sectormap boundaries (Width: %1, Height: %2)." ).arg( width ).arg( height ) );
		return false;
	}

	gridHeight_ = (unsigned int)ceil( (double)height / (double)SECTOR_SIZE );
	gridWidth_ = (unsigned int)ceil( (double)width / (double)SECTOR_SIZE );

	grid = (stSector**)malloc( gridHeight_ * gridWidth_ * sizeof( stSector* ) );
	memset( grid, 0, sizeof( stSector* ) * gridHeight_ * gridWidth_ );
	return true;
}

bool cSectorMap::addItem( cUObject *object )
{
	unsigned short x = object->pos().x;
	unsigned short y = object->pos().y;

	if( x >= gridWidth_ * SECTOR_SIZE || y >= gridHeight_ * SECTOR_SIZE )
	{
		error_ = "X or Y is out of the grid boundaries.";
		return false;
	}

	// Calculate the block id
	unsigned int block = calcBlockId( x, y );

	if( !grid[block] )
	{
		grid[block] = new stSector;
		grid[block]->count = 1;
		grid[block]->data = (cUObject**)malloc( sizeof( cUObject* ) );
		grid[block]->data[0] = object;
	}
	else
	{
		// Check if the item is already inside of our array
		unsigned int i;
		for( i = 0; i < grid[block]->count; ++i )
			if( grid[block]->data[i] == object )
				return true;

		// Append our item to the array
		grid[block]->count++;
		grid[block]->data = (cUObject**)realloc( grid[block]->data, grid[block]->count * sizeof( cUObject* ) );
		grid[block]->data[ grid[block]->count - 1 ] = object;
	}

	return true;
}

bool cSectorMap::removeItem( cUObject *object )
{
	unsigned short x = object->pos().x;
	unsigned short y = object->pos().y;

	if( x >= gridWidth_ * SECTOR_SIZE || y >= gridHeight_ * SECTOR_SIZE )
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
			cUObject **newData = (cUObject**)malloc( ( grid[block]->count - 1 ) * sizeof( cUObject* ) );
			
			unsigned int offset = 0;
			unsigned int j;

			// MemCpy is nice 'n all but we're too stupid to use it
			// Copy the old elements over
			for( j = 0; j < grid[block]->count; ++j )
			{
				if( j == i )
					continue;

				newData[offset++] = grid[block]->data[j];
			}

			/*memcpy( newData, grid[block]->data, sizeof( cUObject* ) * i );
			memcpy( newData + ( i * sizeof( cUObject* ) ), grid[block]->data + sizeof( cUObject* ) * ( i + 1 ), QMAX( 0, sizeof( cUObject* ) * ( grid[block]->count - ( i + 1 ) ) ) );*/
			
			free( grid[block]->data );
			grid[block]->data = newData;
			grid[block]->count--;
			break;
		}
	}

	// Check if the block can be freed
	if( !grid[block]->count )
	{
		free( grid[block]->data );
		delete grid[block];
		grid[block] = 0; // This makes *sure* it gets resetted to 0
	}

	return true;
}
	
unsigned int cSectorMap::countItems( unsigned int block )
{
	// Not in our reach
	if( block >= gridWidth_ * gridHeight_ )
		return 0;
	
	// The block could've been empty
	if( !grid[block] )
		return 0;
	else
		return grid[block]->count;	
}

unsigned int cSectorMap::getItems( unsigned int block, cUObject **items )
{
	// Not in our reach
	if( block >= gridWidth_ * gridHeight_ )
		return 0;

	// Only copy items over if there *are* any items*
	if( grid[block] )
	{
		memcpy( items, grid[block]->data, sizeof( cUObject* ) * grid[block]->count );
		return grid[block]->count;
	}

	return 0;
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
	free( this->items );
}

/*
 *  cItemSectorIterator
 */
P_ITEM cItemSectorIterator::first()
{
	this->pos = 0;
	return next();
}

P_ITEM cItemSectorIterator::next()
{
	if( this->pos >= this->count )
		return 0;

	return static_cast< cItem* >( this->items[ this->pos++ ] );
}

/*
 *  cCharSectorIterator
 */
P_CHAR cCharSectorIterator::first()
{
	this->pos = 0;
	return next();
}

P_CHAR cCharSectorIterator::next()
{
	if( this->pos >= this->count )
		return 0;

	return static_cast< cBaseChar* >( this->items[ this->pos++ ] );
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
		{
			Coord_cl pos = pItem->pos();

			std::map< unsigned char, cSectorMap* >::const_iterator it = itemmaps.find( pos.map );

			if( it == itemmaps.end() )
				throw QString( "Couldn't find a map with the id %1." ).arg( pos.map );
			
			it->second->addItem( (cUObject*)pItem );
		}
	}
	else if( isCharSerial( object->serial() ) )
	{
		P_CHAR pChar = dynamic_cast< P_CHAR >( object );
		if( pChar )
		{
			Coord_cl pos = pChar->pos();

			std::map< unsigned char, cSectorMap* >::const_iterator it = charmaps.find( pos.map );

			if( it == charmaps.end() )
				throw QString( "Couldn't find a map with the id %1." ).arg( pos.map );
			
			it->second->addItem( (cUObject*)pChar );
		}
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
		{
			Coord_cl pos = pItem->pos();

			std::map< unsigned char, cSectorMap* >::const_iterator it = itemmaps.find( pos.map );

			if( it == itemmaps.end() )
				throw QString( "Couldn't find a map with the id %1." ).arg( pos.map );
			
			it->second->removeItem( (cUObject*)pItem );
		}
	}
	else if( isCharSerial( object->serial() ) )
	{
		P_CHAR pChar = dynamic_cast< P_CHAR >( object );
		if( pChar )
		{
			Coord_cl pos = pChar->pos();

			std::map< unsigned char, cSectorMap* >::const_iterator it = charmaps.find( pos.map );

			if( it == charmaps.end() )
				throw QString( "Couldn't find a map with the id %1." ).arg( pos.map );
			
			it->second->removeItem( (cUObject*)pChar );
		}
	}
}

cSectorIterator *cSectorMaps::findObjects( MapType type, cSectorMap *sector, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2 )
{
	// First step: count how many items we are going to hold
	unsigned int count = 0;	
	unsigned int gridSize = sector->gridWidth() * sector->gridHeight();
	unsigned int xBlock, yBlock;

	for( xBlock = x1 / SECTOR_SIZE; xBlock <= x2 / SECTOR_SIZE; xBlock++ )
		for( yBlock = y1 / SECTOR_SIZE; yBlock <= y2 / SECTOR_SIZE; yBlock++ )
			count += sector->countItems( ( xBlock * sector->gridHeight() ) + yBlock );

	// Second step: actually "compile" our list of items
	cUObject **items = (cUObject**)malloc( count * sizeof( cUObject* ) );
	unsigned int offset = 0;

	for( xBlock = x1 / SECTOR_SIZE; xBlock <= x2 / SECTOR_SIZE; xBlock++ )
		for( yBlock = y1 / SECTOR_SIZE; yBlock <= y2 / SECTOR_SIZE; yBlock++ )
		{
			// We *do* know that we allocated more memory than we need, but the effect is minimal!
			unsigned int block = ( xBlock * sector->gridHeight() ) + yBlock;

			if( block >= sector->gridWidth() * sector->gridHeight() )
					continue;

			if( sector->grid[ block ] )
			{
				for( unsigned int i = 0; i < sector->grid[ block ]->count; ++i )
				{
					cUObject *object = sector->grid[ block ]->data[ i ];

					if( object->pos().x >= x1 && object->pos().y >= y1 && object->pos().x <= x2 && object->pos().y <= y2 )
						items[ offset++ ] = object;
				}
			}
		}

	/*
		NOTE:
		Offset is our new count here. The count we measured previously
		was just there to measure the amount of memory we had to allocate
		for the list.
	*/
	
	switch( type )
	{
	case MT_ITEMS:
		return new cItemSectorIterator( offset, items );

	case MT_CHARS:
		return new cCharSectorIterator( offset, items );

	default:
		return new cSectorIterator( offset, items );
	};
}

// Find items in a specific block
cSectorIterator *cSectorMaps::findObjects( MapType type, cSectorMap *sector, unsigned int x, unsigned int y )
{
	unsigned int block = sector->calcBlockId( x, y );
	unsigned int count = sector->countItems( block );

	if( !count )
		return new cSectorIterator( 0, 0 );

	cUObject **items = (cUObject**)malloc( sizeof( cUObject* ) * count );
	count = sector->getItems( block, items );

	switch( type )
	{
	case MT_ITEMS:
		return new cItemSectorIterator( count, items );

	case MT_CHARS:
		return new cCharSectorIterator( count, items );

	default:
		return new cSectorIterator( count, items );
	};
}

// Object specific find methods
cItemSectorIterator *cSectorMaps::findItems( const Coord_cl &center, unsigned char distance )
{
	return findItems( center.map, center.x - distance, center.y - distance, center.x + distance, center.y + distance );
}

cCharSectorIterator *cSectorMaps::findChars( const Coord_cl &center, unsigned char distance )
{
	return findChars( center.map, center.x - distance, center.y - distance, center.x + distance, center.y + distance );
}

cItemSectorIterator *cSectorMaps::findItems( unsigned char map, unsigned int x, unsigned int y )
{
	std::map< unsigned char, cSectorMap* >::const_iterator it = itemmaps.find( map );

	if( it == itemmaps.end() )
	{
		clConsole.log( LOG_ERROR, QString( "Couldn't find a map with the id %1. (cSectorMaps::findItems)" ).arg( map ) );
		return new cItemSectorIterator( 0, 0 ); // Return an empty iterator
	}

	return static_cast< cItemSectorIterator* >( findObjects( MT_ITEMS, it->second, x, y ) );
}

cCharSectorIterator *cSectorMaps::findChars( unsigned char map, unsigned int x, unsigned int y )
{
	std::map< unsigned char, cSectorMap* >::const_iterator it = charmaps.find( map );

	if( it == charmaps.end() )
	{
		clConsole.log( LOG_ERROR, QString( "Couldn't find a map with the id %1. (cSectorMaps::findChars)" ).arg( map ) );
		return new cCharSectorIterator( 0, 0 ); // Return an empty iterator
	}

	return static_cast< cCharSectorIterator* >( findObjects( MT_CHARS, it->second, x, y ) );
}

cItemSectorIterator *cSectorMaps::findItems( unsigned char map, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2 )
{
	std::map< unsigned char, cSectorMap* >::const_iterator it = itemmaps.find( map );

	if( it == itemmaps.end() )
	{
		clConsole.log( LOG_ERROR, QString( "Couldn't find a map with the id %1. (cSectorMaps::findItems)" ).arg( map ) );
		return new cItemSectorIterator( 0, 0 ); // Return an empty iterator
	}

	return static_cast< cItemSectorIterator* >( findObjects( MT_ITEMS, it->second, x1, y1, x2, y2 ) );
}

cCharSectorIterator *cSectorMaps::findChars( unsigned char map, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2 )
{
	std::map< unsigned char, cSectorMap* >::const_iterator it = charmaps.find( map );

	if( it == charmaps.end() )
	{
		clConsole.log( LOG_ERROR, QString( "Couldn't find a map with the id %1. (cSectorMaps::findChars)" ).arg( map ) );
		return new cCharSectorIterator( 0, 0 ); // Return an empty iterator
	}

	return static_cast< cCharSectorIterator* >( findObjects( MT_CHARS, it->second, x1, y1, x2, y2 ) );
}

/*
 * Old Region Iterators
 */
RegionIterator4Chars::RegionIterator4Chars( const Coord_cl &pos, unsigned int distance )
{
	iter = SectorMaps::instance()->findChars( pos, distance );
}

RegionIterator4Chars::~RegionIterator4Chars()
{
	delete iter;
}

void RegionIterator4Chars::Begin( void )
{
	iter->pos = 0;
}

bool RegionIterator4Chars::atEnd( void ) const
{
	return ( iter->pos >= iter->count );
}

P_CHAR RegionIterator4Chars::GetData( void )
{
	if( iter->pos >= iter->count )
		return 0;
	else
		return static_cast< cBaseChar* >( iter->items[ iter->pos ] );
}

RegionIterator4Chars& RegionIterator4Chars::operator ++( int i )
{
	iter->pos++;
	return *this;
}

/*
 *	RegionIterator4Items 
 *  A wrapper class around the new implementation.
 */
RegionIterator4Items::RegionIterator4Items( const Coord_cl &pos, unsigned int distance )
{
	iter = SectorMaps::instance()->findItems( pos, distance );
}

RegionIterator4Items::~RegionIterator4Items()
{
	delete iter;
}

void RegionIterator4Items::Begin( void )
{
	iter->pos = 0;
}

bool RegionIterator4Items::atEnd( void ) const
{
	return ( iter->pos >= iter->count );
}

P_ITEM RegionIterator4Items::GetData( void )
{
	if( iter->pos >= iter->count )
		return 0;
	else
		return static_cast< cItem* >( iter->items[ iter->pos ] );
}

RegionIterator4Items& RegionIterator4Items::operator++( int i )
{
	iter->pos++;
	return *this;
}
