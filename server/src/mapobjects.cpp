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

// Wolfpack Includes
#include "timing.h"
#include "inlines.h"
#include "uobject.h"
#include "mapobjects.h"

#include "npc.h"
#include "world.h"
#include "items.h"
#include "player.h"
#include "basechar.h"
#include "exceptions.h"

/*!
	Optimized array implementation. Does NOT preserve element's order.
	Saves memory as much as possible. When empty takes sizeof(void*) bytes.
	- Stretches by 50% when at full capacity.
	- Shrinks by 25% when at half capacity.
	- Deallocates when empty.
 */
template<typename T, UI16 minCapacity = 4>
class OptimalArray
{
public:
	OptimalArray() : mData( NULL )
	{;}

	~OptimalArray()
	{
		if( mData )
			free( mData );
	}

	//! Returns true if the container is empty
	inline bool isEmpty() const
	{
		return ( mData == NULL || mData->size == 0 );
	}

	//! Returns the number of used elements in the array.
	inline UI16 size() const
	{
		return ( mData ? mData->size : 0 );
	}

	//! Returns the number of elements that the array can hold without re-allocating.
	inline UI16 capacity() const
	{
		return ( mData ? mData->capacity : 0 );
	}

	//! Returns a pointer to the internal array structure.
	inline T * data()
	{
		return ( mData ? &( mData->array ) : NULL );
	}

	//! Returns the ith array element. i must be valid!
	inline T & at( UI16 i )
	{
		return ( &( mData->array ) )[i];
	}

	//! Appends an element to the array.
	inline void add( const T &element )
	{
		grow();
		at( mData->size++ ) = element;
	}

	//! Removes the ith element and covers the hole with the last array element. O(1)
	inline void remove( UI16 i )
	{
		UI16 last = --mData->size;
		if( i < last )
		{
			at( i ) = at( last );
		}
		shrink();
	}

	//! Removes the ith element preserving the elements order. O(n)
	inline void linearRemove( UI16 i )
	{
		UI16 last = --mData->size;
		for( UI16 k = i; k < last; ++k )
		{
			at( k ) = at( k + 1 );
		}
		shrink();
	}

	//! Alias to add() just to treat the array as a stack.
	inline void push( const T &element )
	{
		add( element );
	}

	//! Removes the last element from the array. NEVER shrinks it. Returns the new size.
	inline UI16 pop()
	{
		return --mData->size;
	}

	inline T & operator[]( UI16 i )
	{
		return at( i );
	}

private:
	struct Instance
	{
		UI16 size;
		UI16 capacity;
		T array;
	};

	// Forbids copies.
	inline OptimalArray( const OptimalArray<T> &other )
	{;}

	// Forbids copies.
	inline const OptimalArray<T> & operator=( const OptimalArray<T> &other )
	{;}

	// If necessary, allocates/increases the container's capacity.
	inline void grow()
	{
		if( mData == NULL )
		{
			mData = reinterpret_cast<Instance *>( realloc( mData, sizeof(Instance) + ( ( minCapacity - 1 ) * sizeof(T) ) ) );
			mData->size = 0;
			mData->capacity = minCapacity;
		}
		else if( mData->size == mData->capacity )
		{
			mData->capacity += ( mData->capacity / 2 );
			mData = reinterpret_cast<Instance *>( realloc( mData, sizeof(Instance) + ( ( mData->capacity - 1 ) * sizeof(T) ) ) );
		}
	}

	// Shrinks the container's capacity to 75% if it's half full or less. Min of 'minCapacity' elements.
	inline void shrink()
	{
		if( mData->size == 0 )
		{
			// it's empty, we can deallocate memory.
			free( mData );
			mData = NULL;
		}
		else if( mData->capacity != minCapacity && mData->size <= mData->capacity / 2 )
		{
			mData->capacity = wpMax<UI16>( minCapacity, mData->capacity - ( mData->capacity / 4 ) );
			mData = reinterpret_cast<Instance *>( realloc( mData, sizeof(Instance) + ( ( mData->capacity - 1 ) * sizeof(T) ) ) );
		}
	}

private:
	Instance *mData;
};

/*!
	Available iteration algorithms:
 */
enum IteratorType
{
	ITERATE_CELL,		// iterates all objects within a single cell in the grid
	ITERATE_POINT,		// iterates all objects at a certain tile in the map
	ITERATE_RECTANGLE,	// iterates all objects within an axis-aligned rectangle in the map
	ITERATE_CIRCLE		// iterates all objects within a circle in the map
};

struct PointIteratorState
{
	UI16 x;			// x coordinate of the point in consideration
	UI16 y;			// y coordinate of the point in consideration
};

struct RectangleIteratorState
{
	UI16 x1;		// x coordinate of the min point
	UI16 y1;		// y coordinate of the min point
	UI16 x2;		// x coordinate of the max point
	UI16 y2;		// y coordinate of the max point

	UI16 cellX;		// x coordinate of the current cell in the grid
	UI16 cellY;		// y coordinate of the current cell in the grid
	UI16 lastCellX;	// x coordinate of the last cell we're gonna visit
	UI16 lastCellY;	// y coordinate of the last cell we're gonna visit
};

struct CircleIteratorState
{
	UI16 x;			// x coordinate of the center point
	UI16 y;			// y coordinate of the center point
	UI16 radius;	// circle radius
	UI32 radius2;	// squared circle radius

	UI16 cellX;		// x coordinate of the current cell in the grid
	UI16 cellY;		// y coordinate of the current cell in the grid
	UI16 lastCellX;	// x coordinate of the last cell we're gonna visit
	UI16 lastCellY;	// y coordinate of the last cell we're gonna visit
};

/*!
	The internal state of a MapObjectsIterator.
 */
struct IteratorState
{
	// number of iterators sharing this state (normally 1).
	// MUST BE the first element!
	UI08 refcount;

	// whether the iterator is active and valid
	bool valid;

	// identifies the iteration algorithm
	IteratorType type;

	// identifies the map we're into (actually a MapObjectsGrid<T>)
	MapObjectsGrid *map;

	// id of the cell being iterated
	UI32 cellId;

	// index of the current object within the cell
	UI16 i;

	// state specific to each iteration algorithm (except cell, which doesn't need one):
	union
	{
		PointIteratorState point;
		RectangleIteratorState rect;
		CircleIteratorState circle;
	}
	state;
};


/************************************************************************/
/* MapObjectsGrid: a grid structure to efficiently locate UObjects.     */
/************************************************************************/

class MapObjectsGrid
{
public:
	typedef OptimalArray<cUObject *> Cell;

public:
	MapObjectsGrid( UI16 width, UI16 height, UI08 cellsize )
		: mCellSize( cellsize ), mMapWidth( width ), mMapHeight( height )
	{
		mGridWidth = ( width / cellsize );
		mGridHeight = ( height / cellsize );
		mCellCount = ( mGridWidth * mGridHeight );

		mCells = new Cell[mCellCount];
	}

	~MapObjectsGrid()
	{
		delete []mCells;

		UI16 count = mIterators.size();
		for( UI16 i = 0; i < count; ++i )
		{
			delete mIterators[i];
		}
	}

	inline UI08 cellSize() const
	{
		return mCellSize;
	}

	inline UI16 gridWidth() const
	{
		return mGridWidth;
	}

	inline UI16 gridHeight() const
	{
		return mGridHeight;
	}

	bool add( cUObject *object )
	{
		const Coord &pos = object->pos();
		cell( cellId( pos.x, pos.y ) ).add( object );
		return true;
	}

	//! Returns true only if the item was removed by this method call.
	bool remove( cUObject *object )
	{
		const Coord &pos = object->pos();
		UI32 cellId = this->cellId( pos.x, pos.y );
		Cell &cell = this->cell( cellId );

		// linear search for our item within the cell...
		UI16 size = cell.size();
		for( UI16 i = 0; i < size; ++i )
		{
			if( cell[i] == object )
			{
				// ok, item found...
				if( adjustIterators( cellId, i ) )
					cell.linearRemove( i );
				else
					cell.remove( i );

				return true;
			}
		}

		// item not found?
		return false;
	}

	void move( cUObject *object, const Coord &newPos )
	{
		remove( object );
		cell( cellId( newPos.x, newPos.y ) ).add( object );
	}

	inline bool validCoord( const Coord &pos )
	{
		return ( pos.x < mMapWidth && pos.y < mMapHeight );
	}

	inline bool validCell( UI32 id )
	{
		return ( id < mCellCount );
	}

	// calculates the id of the cell at the given coordinates
	inline UI32 cellId( UI16 x, UI16 y )
	{
		return ( ( x / mCellSize ) * mGridHeight ) + ( y / mCellSize );
	}

	// returns a cell by id
	inline Cell & cell( UI32 id )
	{
		return mCells[id];
	}

	inline IteratorState * reserveIterator()
	{
		// reuse iterators
		if( !mAvailableIterators.isEmpty() )
			return mAvailableIterators[mAvailableIterators.pop()];
		
		// all iterators are busy - we need a new one
		IteratorState *it = new IteratorState();
		it->map = this;
		it->refcount = 0;
		mIterators.push( it );
		return it;
	}

	inline void releaseIterator( IteratorState *iterator )
	{
		mAvailableIterators.push( iterator );
	}

private:
	// returns true to perform a linear-removal, or false to use the faster removal
	bool adjustIterators( UI32 cellId, UI16 i )
	{
		bool result = false;
		UI16 count = mIterators.size();
		for( UI16 k = 0; k < count; ++k )
		{
			IteratorState *is = mIterators[k];
			if( is->valid && is->cellId == cellId && i < is->i )
			{
				--is->i;
				result = true;
			}
		}
		return result;
	}

private:
	// map stuff
	UI08 mCellSize;
	UI32 mCellCount;
	UI16 mGridWidth;
	UI16 mGridHeight;
	UI16 mMapWidth;
	UI16 mMapHeight;
	Cell *mCells;

	// iterators stuff
	OptimalArray<IteratorState *> mIterators;
	OptimalArray<IteratorState *> mAvailableIterators;
};


/************************************************************************/
/* MapObjects Implementation                                            */
/************************************************************************/

struct MapObjects::GridSet
{
	// notice the third parameter - it sets the cellsize (SECTOR_SIZE) for each structure
	GridSet( UI16 width, UI16 height ) : items( width, height, 8 ),
		multis( width, height, 16 ), chars( width, height, 8 ),
		offlineChars( width, height, 32 )
	{;}

	MapObjectsGrid items;
	MapObjectsGrid multis;
	MapObjectsGrid chars;
	MapObjectsGrid offlineChars;
};

MapObjects::MapObjects() : mMaps( 8 )
{
	mMaps.setAutoDelete( true );
}

MapObjects::~MapObjects()
{
	// empty
}

void MapObjects::load()
{
	addMap( 0, 6144, 4096 );
	addMap( 1, 6144, 4096 );
	addMap( 2, 2304, 1600 );
	addMap( 3, 2560, 2048 );
	addMap( 4, 1448, 1448 );

	cComponent::load();
}

void MapObjects::addMap( UI08 map, UI16 width, UI16 height )
{
	// the map width and height must be multiple of 8
	if( ( width % 8 ) != 0 || ( height % 8 ) != 0 )
	{
		throw wpException( QString( "Invalid map size: must be multiple of 8! (width: %1, height: %2)." ).arg( width ).arg( height ) );
	}

	GridSet *gridSet = new GridSet( width, height );

	if( mMaps.size() <= map )
	{
		mMaps.resize( map + 1 );
	}

	mMaps.insert( map, gridSet );
}

inline MapObjectsGrid * resolveGrid( MapObjects::GridSet *gridSet, cUObject *object )
{
	if( isItemSerial( object->serial() ) )
	{
		return ( reinterpret_cast<P_ITEM>( object )->isMulti() ? &gridSet->multis : &gridSet->items );
	}
	else
	{
		P_PLAYER player = dynamic_cast<P_PLAYER>( object );
		return ( player && !player->isOnline() ? &gridSet->offlineChars : &gridSet->chars );
	}
}

void MapObjects::add( cUObject *object )
{
	if( !object )	// compatibility with old code
		return;

	if( isItemSerial( object->serial() ) )
	{
		Timing::instance()->addDecayItem( reinterpret_cast<P_ITEM>( object ) );
	}

	MapObjectsGrid *grid = resolveGrid( mMaps[object->pos().map], object );
	if( grid->validCoord( object->pos() ) )
		grid->add( object );
}

void MapObjects::update( cUObject *object, const Coord &newPos )
{
	const Coord &pos = object->pos();
	MapObjectsGrid *grid = resolveGrid( mMaps[pos.map], object );

	if( pos.map == newPos.map )
	{
		// if we're still within the same map and the same cell, no need to update
		UI08 size = grid->cellSize();
		if( ( pos.x / size == newPos.x / size ) && ( pos.y / size == newPos.y / size ) )
			return;

		// we must move the object to another cell within the same map...
		if( grid->validCoord( newPos ) )
			grid->move( object, newPos );
	}
	else
	{
		// we must move the object to another map (worst case)...
		MapObjectsGrid *newGrid = resolveGrid( mMaps[newPos.map], object );
		if( newGrid->validCoord( newPos ) )
		{
			grid->remove( object );
			newGrid->add( object );
		}
	}
}

void MapObjects::updateOnlineStatus( cPlayer *player, bool online )
{
	GridSet *gridSet = mMaps[player->pos().map];
	MapObjectsGrid *from = ( online ? &gridSet->offlineChars : &gridSet->chars );
	MapObjectsGrid *to = ( online ? &gridSet->chars : &gridSet->offlineChars );
	from->remove( player );
	to->add( player );
}

void MapObjects::remove( cUObject *object )
{
	if ( !object )	// compatibility with old code
		return;

	if( isItemSerial( object->serial() ) )
	{
		Timing::instance()->removeDecayItem( reinterpret_cast<P_ITEM>( object ) );
	}

	MapObjectsGrid *grid = resolveGrid( mMaps[object->pos().map], object );
	if( grid->validCoord( object->pos() ) )
		grid->remove( object );
}

inline IteratorState * initPointIterator( IteratorState *it, UI16 x, UI16 y )
{
	it->type = ITERATE_POINT;
	PointIteratorState &state = it->state.point;
	it->cellId = it->map->cellId( x, y );
	state.x = x;
	state.y = y;
	return it;
}

inline IteratorState * initCellIterator( IteratorState *it, UI16 x, UI16 y )
{
	it->type = ITERATE_CELL;
	it->cellId = it->map->cellId( x, y );
	return it;
}

inline IteratorState * initRectIterator( IteratorState *it, UI16 x1, UI16 y1, UI16 x2, UI16 y2 )
{
	// make sure the two points are really the min and max points, respectively
	if( x2 < x1 )
	{
		UI16 temp = x1;
		x1 = x2;
		x2 = x1;
	}
	if( y2 < y1 )
	{
		UI16 temp = y1;
		y1 = y2;
		y2 = y1;
	}

	it->type = ITERATE_RECTANGLE;
	RectangleIteratorState &state = it->state.rect;
	state.x1 = x1;
	state.y1 = y1;
	state.x2 = x2;
	state.y2 = y2;

	return it;
}

inline IteratorState * initCircleIterator( IteratorState *it, UI16 x, UI16 y, UI16 radius )
{
	it->type = ITERATE_CIRCLE;
	CircleIteratorState &state = it->state.circle;
	state.x = x;
	state.y = y;
	state.radius = radius;
	state.radius2 = radius * radius;
	return it;
}

MapItemsIterator MapObjects::listItemsAtCoord( UI08 map, UI16 x, UI16 y )
{
	return MapItemsIterator( initPointIterator( mMaps[map]->items.reserveIterator(), x, y ) );
}

MapItemsIterator MapObjects::listItemsInBlock( UI08 map, UI16 x, UI16 y )
{
	return MapItemsIterator( initCellIterator( mMaps[map]->items.reserveIterator(), x, y ) );
}

MapItemsIterator MapObjects::listItemsInRect( UI08 map, UI16 x1, UI16 y1, UI16 x2, UI16 y2 )
{	
	return MapItemsIterator( initRectIterator( mMaps[map]->items.reserveIterator(), x1, y1, x2, y2 ) );
}

MapItemsIterator MapObjects::listItemsInCircle( UI08 map, UI16 x, UI16 y, UI16 radius )
{
	return MapItemsIterator( initCircleIterator( mMaps[map]->items.reserveIterator(), x, y, radius ) );
}

MapMultisIterator MapObjects::listMultisAtCoord( UI08 map, UI16 x, UI16 y )
{
	return MapMultisIterator( initPointIterator( mMaps[map]->multis.reserveIterator(), x, y ) );
}

MapMultisIterator MapObjects::listMultisInBlock( UI08 map, UI16 x, UI16 y )
{
	return MapMultisIterator( initCellIterator( mMaps[map]->multis.reserveIterator(), x, y ) );
}

MapMultisIterator MapObjects::listMultisInRect( UI08 map, UI16 x1, UI16 y1, UI16 x2, UI16 y2 )
{
	return MapMultisIterator( initRectIterator( mMaps[map]->multis.reserveIterator(), x1, y1, x2, y2 ) );
}

MapMultisIterator MapObjects::listMultisInCircle( UI08 map, UI16 x, UI16 y, UI16 radius )
{
	return MapMultisIterator( initCircleIterator( mMaps[map]->multis.reserveIterator(), x, y, radius ) );
}

MapCharsIterator MapObjects::listCharsAtCoord( UI08 map, UI16 x, UI16 y, bool offline )
{
	GridSet *gridSet = mMaps[map];
	MapObjectsGrid &grid = ( offline ? gridSet->offlineChars : gridSet->chars );
	return MapCharsIterator( initPointIterator( grid.reserveIterator(), x, y ) );
}

MapCharsIterator MapObjects::listCharsInBlock( UI08 map, UI16 x, UI16 y, bool offline )
{
	GridSet *gridSet = mMaps[map];
	MapObjectsGrid &grid = ( offline ? gridSet->offlineChars : gridSet->chars );
	return MapCharsIterator( initCellIterator( grid.reserveIterator(), x, y ) );
}

MapCharsIterator MapObjects::listCharsInRect( UI08 map, UI16 x1, UI16 y1, UI16 x2, UI16 y2, bool offline )
{
	GridSet *gridSet = mMaps[map];
	MapObjectsGrid &grid = ( offline ? gridSet->offlineChars : gridSet->chars );
	return MapCharsIterator( initRectIterator( grid.reserveIterator(), x1, y1, x2, y2 ) );
}

MapCharsIterator MapObjects::listCharsInCircle( UI08 map, UI16 x, UI16 y, UI16 radius, bool offline )
{
	GridSet *gridSet = mMaps[map];
	MapObjectsGrid &grid = ( offline ? gridSet->offlineChars : gridSet->chars );
	return MapCharsIterator( initCircleIterator( grid.reserveIterator(), x, y, radius ) );
}


/************************************************************************/
/* Iteration Code                                                       */
/************************************************************************/

inline cUObject * nextObjectInCell( IteratorState *is )
{
	MapObjectsGrid::Cell &cell = is->map->cell( is->cellId );
	if( is->i < cell.size() )
	{
		return cell[is->i++];
	}

	is->valid = false;
	return NULL;
}

inline cUObject * nextObjectAtPoint( IteratorState *is )
{
	PointIteratorState &state = is->state.point;
	MapObjectsGrid::Cell &cell = is->map->cell( is->cellId );
	UI16 size = cell.size();
	while( is->i < size )
	{
		cUObject *object = cell[is->i++];
		const Coord &pos = object->pos();
		if( pos.x == state.x && pos.y == state.y )
		{
			return object;
		}
	}

	is->valid = false;
	return NULL;
}

inline cUObject * nextObjectInRectangle( IteratorState *is )
{
	MapObjectsGrid *map = is->map;
	RectangleIteratorState &state = is->state.rect;

	do
	{
		// fetch next object within the current cell
		MapObjectsGrid::Cell &cell = map->cell( is->cellId );
		UI16 size = cell.size();
		while( is->i < size )
		{
			cUObject *object = cell[is->i++];
			const Coord &pos = object->pos();
			if( state.x1 <= pos.x && state.x2 >= pos.x && state.y1 <= pos.y && state.y2 >= pos.y )
			{
				return object;
			}
		}

		// we're done with the current cell, step into the next cell...
		is->i = 0;
		if( ++state.cellY > state.lastCellY )
		{
			++state.cellX;
			state.cellY = ( state.y1 / map->cellSize() );
			is->cellId = ( state.cellX * map->gridHeight() + state.cellY );
		}
		else
		{
			++is->cellId;
		}
	}
	while( state.cellX <= state.lastCellX );

	is->valid = false;
	return NULL;
}

inline cUObject * nextObjectInCircle( IteratorState *is )
{
	MapObjectsGrid *map = is->map;
	CircleIteratorState &state = is->state.circle;

	do
	{
		// fetch next object within the current cell
		MapObjectsGrid::Cell &cell = map->cell( is->cellId );
		UI16 size = cell.size();
		while( is->i < size )
		{
			cUObject *object = cell[is->i++];
			const Coord &pos = object->pos();
			SI16 dx = ( pos.x - state.x );
			SI16 dy = ( pos.y - state.y );
			if( ( ( dx * dx ) + ( dy * dy ) ) < state.radius2 )
			{
				return object;
			}
		}

		// we're done with the current cell, step into the next cell...
		is->i = 0;
		if( ++state.cellY > state.lastCellY )
		{
			++state.cellX;
			state.cellY = ( ( state.y - state.radius ) / map->cellSize() );
			is->cellId = ( state.cellX * map->gridHeight() + state.cellY );
		}
		else
		{
			++is->cellId;
		}
	}
	while( state.cellX <= state.lastCellX );


	is->valid = false;
	return NULL;
}

cUObject * MapObjects::nextIteration( void *state )
{
	IteratorState *is = reinterpret_cast<IteratorState *>( state );
	if( !is->valid )
		return NULL;

	switch( is->type )
	{
	case ITERATE_CELL:			return nextObjectInCell( is );
	case ITERATE_POINT:			return nextObjectAtPoint( is );
	case ITERATE_RECTANGLE:		return nextObjectInRectangle( is );
	case ITERATE_CIRCLE:		return nextObjectInCircle( is );
	default:					return NULL;
	}
}

inline cUObject * firstObjectInCell( IteratorState *is )
{
	is->i = 0;
	is->valid = true;
	return nextObjectInCell( is );
}

inline cUObject * firstObjectAtPoint( IteratorState *is )
{
	is->i = 0;
	is->valid = true;
	return nextObjectAtPoint( is );
}

inline cUObject * firstObjectInRectangle( IteratorState *is )
{
	MapObjectsGrid *map = is->map;
	RectangleIteratorState &state = is->state.rect;

	state.cellX = ( state.x1 / map->cellSize() );
	state.cellY = ( state.y1 / map->cellSize() );
	state.lastCellX = wpMin<UI16>( map->gridWidth() - 1, state.x2 / map->cellSize() );
	state.lastCellY = wpMin<UI16>( map->gridHeight() - 1, state.y2 / map->cellSize() );

	is->cellId = ( state.cellX * map->gridHeight() + state.cellY );
	is->i = 0;

	is->valid = true;
	return nextObjectInRectangle( is );
}

inline cUObject * firstObjectInCircle( IteratorState *is )
{
	MapObjectsGrid *map = is->map;
	CircleIteratorState &state = is->state.circle;

	state.cellX = ( ( state.x - state.radius ) / map->cellSize() );
	state.cellY = ( ( state.y - state.radius ) / map->cellSize() );
	state.lastCellX = wpMin<UI16>( map->gridWidth() - 1, ( state.x + state.radius ) / map->cellSize() );
	state.lastCellY = wpMin<UI16>( map->gridHeight() - 1, ( state.y + state.radius ) / map->cellSize() );

	is->cellId = ( state.cellX * map->gridHeight() + state.cellY );
	is->i = 0;

	is->valid = true;
	return nextObjectInCircle( is );
}

cUObject * MapObjects::firstIteration( void *state )
{
	// it's safe to call first() on invalid iterators
	if( !state )
		return NULL;

	IteratorState *is = reinterpret_cast<IteratorState *>( state );
	switch( is->type )
	{
	case ITERATE_CELL:			return firstObjectInCell( is );
	case ITERATE_POINT:			return firstObjectAtPoint( is );
	case ITERATE_RECTANGLE:		return firstObjectInRectangle( is );
	case ITERATE_CIRCLE:		return firstObjectInCircle( is );
	default:					return NULL;
	}
}

void MapObjects::releaseIterator( void *state )
{
	IteratorState *is = reinterpret_cast<IteratorState *>( state );
	is->map->releaseIterator( is );
}
