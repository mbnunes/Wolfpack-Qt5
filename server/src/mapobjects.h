//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001-2003 by holders identified in authors.txt
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================

#if !defined(__MAPOBJECTS_H__)
#define __MAPOBJECTS_H__

// Platform specifics
#include "platform.h"

// Library Includes
#include <vector>
#include <qmap.h>

// wolfpack includes
#include "typedefs.h"
#include "singleton.h"
#include "coord.h"

// Forward class definition
class cUObject;
class RegionIterator4Chars;
class RegionIterator4Items;
class cQuadNode;

class cMapObjects
{
public:
	
	typedef SERIAL type;

	cMapObjects();
	~cMapObjects();

	friend class RegionIterator4Chars;
	friend class RegionIterator4Items;

	void	add( cUObject* object );
	void	remove( cUObject* object );

protected:
	void	search( const Coord_cl &pos, UI32 distance, std::vector<type> &serials );

protected:
	QMap< UI08, cQuadNode* > rootmap_;
};

typedef SingletonHolder<cMapObjects> MapObjects;

class RegionIterator4Chars
{
public:
	RegionIterator4Chars( const Coord_cl &pos, UI32 distance = 18 ); 
	void		Begin( void );
	bool		atEnd( void ) const;
	P_CHAR		GetData( void );
	void		reset( const Coord_cl &pos, UI32 distance = 18 );

	// Operators
	RegionIterator4Chars& operator++( int );
	RegionIterator4Chars& operator=( const Coord_cl &pos );
protected:
	std::vector< cMapObjects::type > serials;
	std::vector< cMapObjects::type >::iterator	currentIterator;

	Coord_cl	position_;
	UI32		distance_;
};

class RegionIterator4Items
{
public:
	RegionIterator4Items( const Coord_cl &pos, UI32 distance = 18 ); 
	void		Begin( void );
	bool		atEnd( void ) const;
	P_ITEM		GetData( void );
	void		reset( const Coord_cl &pos, UI32 distance = 18 );

	// Operators
	RegionIterator4Items& operator++( int );
	RegionIterator4Items& operator=( const Coord_cl &pos );
protected:
	std::vector< cMapObjects::type > serials;
	std::vector< cMapObjects::type >::iterator	currentIterator;

	Coord_cl	position_;
	UI32		distance_;
};

#endif

