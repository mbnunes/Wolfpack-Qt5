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

#if !defined( __MAPOBJECTS_H__ )
#define __MAPOBJECTS_H__

#include "coord.h"
#include "server.h"
#include "singleton.h"
#include <qptrvector.h>

class Coord;
class cItem;
class cMulti;
class cUObject;
class cBaseChar;
class MapObjects;
class MapObjectsGrid;

/*!
	Iterator returned by MapObjects::list*() operations.
	If you need to iterate many times over the same map region, you CAN reuse the iterator.
	All changes made to the map region will instantly affect the iterator.

	@warning Do not replicate iterator objects, as they have implicitly shared states.
 */
template<typename T = cUObject>
class MapObjectsIterator
{
public:
	//! Constructs an invalid iterator.
	inline MapObjectsIterator() : mState( NULL )
	{;}

	//! Copy constructor.
	inline MapObjectsIterator( const MapObjectsIterator &other ) : mState( other.mState )
	{
		incrementRefCount();
	}

	inline ~MapObjectsIterator()
	{
		decrementRefCount();
	}

	//! Rewinds the iterator and returns the first element (NULL if empty).
	inline T * first();

	//! Fetches the next element (NULL if there are no more elements).
	inline T * next();

	//! Copy operator.
	MapObjectsIterator & operator=( const MapObjectsIterator &other );

private:
	friend class MapObjects;

	inline MapObjectsIterator( void *state ) : mState( state )
	{
		incrementRefCount();
	}

	inline void incrementRefCount();
	inline void decrementRefCount();

private:
	void *mState;
};

typedef MapObjectsIterator<cItem> MapItemsIterator;
typedef MapObjectsIterator<cMulti> MapMultisIterator;
typedef MapObjectsIterator<cBaseChar> MapCharsIterator;

/*!
	Use MapObjects to efficiently locate objects within specific regions of the game maps.

	Supported lookups:
		- Objects at an exact coord.
		- Objects within a block (a cell of the internal grid structure).
		- Objects within a rectangle.
		- Objects within a circle (just as fast as the rectangle lookup).
 */
class MapObjects : public cComponent, public Singleton<MapObjects>
{
public:
	struct GridSet;

public:
	MapObjects();
	virtual ~MapObjects();

	// Loads default maps
	void load();

	/*!
		Adds a map, with the specified width and height, to the structure.
		@pre width and height MUST be multiples of 8.
	 */
	void addMap( UI08 map, UI16 width, UI16 height );

	//! Adds an UObject to the structure.
	void add( cUObject *object );

	/*!
		Updates an UObjects that's already in the structure.
		This is equivalent to removing&re-adding the UObject, but much faster.
		Must be called just before the object's position changes, with the new position.
	 */
	void update( cUObject *object, const Coord &newPos );

	/*!
		This must be called whenever a player's online status changes (as defined by
		isOnline(), which takes lingering into consideration).
		
		If 'online' is true, the player's status is considered to have changed from
		offline to online; or if it's false, from online to offline.
	 */
	void updateOnlineStatus( cPlayer *player, bool online );

	//! Removes an UObject from the structure.
	void remove( cUObject *object );

	//! Whether a map is valid.
	bool validMap( UI08 map )
	{
		return ( mMaps.size() > map && mMaps[map] );
	}

	MapItemsIterator listItemsAtCoord( UI08 map, UI16 x, UI16 y );
	MapItemsIterator listItemsInBlock( UI08 map, UI16 x, UI16 y );
	MapItemsIterator listItemsInRect( UI08 map, UI16 x1, UI16 y1, UI16 x2, UI16 y2 );
	MapItemsIterator listItemsInCircle( UI08 map, UI16 x, UI16 y, UI16 radius );

	inline MapItemsIterator listItemsAtCoord( const Coord &pos )
	{
		return listItemsAtCoord( pos.map, pos.x, pos.y );
	}

	inline MapItemsIterator listItemsInBlock( const Coord &pos )
	{
		return listItemsInBlock( pos.map, pos.x, pos.y );
	}

	inline MapItemsIterator listItemsInRect( const Coord &p1, const Coord &p2 )
	{
		return listItemsInRect( p1.map, p1.x, p1.y, p2.x, p2.y );
	}

	inline MapItemsIterator listItemsInCircle( const Coord &center, UI16 radius )
	{
		return listItemsInCircle( center.map, center.x, center.y, radius );
	}

	MapMultisIterator listMultisAtCoord( UI08 map, UI16 x, UI16 y );
	MapMultisIterator listMultisInBlock( UI08 map, UI16 x, UI16 y );
	MapMultisIterator listMultisInRect( UI08 map, UI16 x1, UI16 y1, UI16 x2, UI16 y2 );
	MapMultisIterator listMultisInCircle( UI08 map, UI16 x, UI16 y, UI16 radius );

	inline MapMultisIterator listMultisAtCoord( const Coord &pos )
	{
		return listMultisAtCoord( pos.map, pos.x, pos.y );
	}

	inline MapMultisIterator listMultisInBlock( const Coord &pos )
	{
		return listMultisInBlock( pos.map, pos.x, pos.y );
	}

	inline MapMultisIterator listMultisInRect( const Coord &p1, const Coord &p2 )
	{
		return listMultisInRect( p1.map, p1.x, p1.y, p2.x, p2.y );
	}

	inline MapMultisIterator listMultisInCircle( const Coord &center, UI16 radius )
	{
		return listMultisInCircle( center.map, center.x, center.y, radius );
	}

	// Use 'offline' to list EITHER the online or offline characters
	MapCharsIterator listCharsAtCoord( UI08 map, UI16 x, UI16 y, bool offline = false );
	MapCharsIterator listCharsInBlock( UI08 map, UI16 x, UI16 y, bool offline = false );
	MapCharsIterator listCharsInRect( UI08 map, UI16 x1, UI16 y1, UI16 x2, UI16 y2, bool offline = false );
	MapCharsIterator listCharsInCircle( UI08 map, UI16 x, UI16 y, UI16 radius, bool offline = false );

	inline MapCharsIterator listCharsAtCoord( const Coord &pos, bool offline = false )
	{
		return listCharsAtCoord( pos.map, pos.x, pos.y, offline );
	}

	inline MapCharsIterator listCharsInBlock( const Coord &pos, bool offline = false )
	{
		return listCharsInBlock( pos.map, pos.x, pos.y, offline );
	}

	inline MapCharsIterator listCharsInRect( const Coord &p1, const Coord &p2, bool offline = false )
	{
		return listCharsInRect( p1.map, p1.x, p1.y, p2.x, p2.y, offline );
	}

	inline MapCharsIterator listCharsInCircle( const Coord &center, UI16 radius, bool offline = false )
	{
		return listCharsInCircle( center.map, center.x, center.y, radius, offline );
	}

private:
	friend class MapObjectsIterator<cUObject>;
	friend class MapObjectsIterator<cItem>;
	friend class MapObjectsIterator<cMulti>;
	friend class MapObjectsIterator<cBaseChar>;

	static cUObject * firstIteration( void *state );
	static cUObject * nextIteration( void *state );
	static void releaseIterator( void *state );

private:
	QPtrVector<GridSet> mMaps;
};


template<typename T>
inline T * MapObjectsIterator<T>::first()
{
	return reinterpret_cast<T *>( MapObjects::firstIteration( mState ) );
}

template<typename T>
inline T * MapObjectsIterator<T>::next()
{
	return reinterpret_cast<T *>( MapObjects::nextIteration( mState ) );
}

template<typename T>
inline void MapObjectsIterator<T>::incrementRefCount()
{
	// first UI16 in state holds our refcount
	if( mState )
		++( *reinterpret_cast<UI08 *>( mState ) );
}

template<typename T>
inline void MapObjectsIterator<T>::decrementRefCount()
{
	// if refcount reaches zero, release the iterator
	if( mState && --( *reinterpret_cast<UI08 *>( mState ) ) == 0 )
		MapObjects::releaseIterator( mState );
}

template<typename T>
MapObjectsIterator<T> & MapObjectsIterator<T>::operator=( const MapObjectsIterator &other )
{
	if( mState != other.mState )
	{
		// increment other's refcount
		if( other.mState )
			++( *reinterpret_cast<UI08 *>( other.mState ) );

		decrementRefCount();
		mState = other.mState;
	}

	return *this;
}

#endif
