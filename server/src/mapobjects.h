//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
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
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

#if !defined(__MAPOBJECTS_H__)
#define __MAPOBJECTS_H__

// Platform specifics
#include "platform.h"

// Library Includes
#include <vector>
#include "qmap.h"

// Forward class definition
class cUObject;
class RegionIterator4Chars;
class RegionIterator4Items;

// wolfpack includes
#include "typedefs.h"

class cQuadNode
{
public:
	cQuadNode( UI16 x, UI16 y, cQuadNode* parent_ = NULL );
	~cQuadNode();

	enum enQuadrants { northeast = 0, northwest, southwest, southeast, enNumberOfChilds };

	UI16	x()	const { return x_; }
	UI16	y()	const { return y_; }
	UI32	distance( UI16 srcx, UI16 srcy );
	enQuadrants	compare( UI16 srcx, UI16 srcy );
	bool	inRange( UI16 srcx, UI16 srcy, UI32 distance );
	bool	overlap( UI16 left, UI16 right, UI16 top, UI16 bottom, UI16 srcx, UI16 srcy, UI32 distance );

	void	setParent( cQuadNode* node )	{ parent_ = node; }

	cQuadNode* parent()			{ return parent_; }

	cQuadNode* childs[enNumberOfChilds];


	void	search( UI16 left, UI16 right, UI16 top, UI16 bottom, UI16 srcx, UI16 srcy, UI32 distance, std::vector< SERIAL > &serials );

	bool	contains( SERIAL serial );
	void	add( UI16 srcx, UI16 srcy, SERIAL serial );
	void	remove( UI16 srcx, UI16 srcy, SERIAL serial );
	void	pushdown( cQuadNode* node );

	std::vector< SERIAL >	objectserials;
private:
	UI16 x_;
	UI16 y_;
	cQuadNode*	parent_;
};

class cMapObjects
{
public:
	cMapObjects();
	~cMapObjects();

	static cMapObjects *getInstance( void ) { return &instance; }

	friend class RegionIterator4Chars;
	friend class RegionIterator4Items;

	void	add( cUObject* object );
	void	remove( cUObject* object );

protected:
	void	search( const Coord_cl &pos, UI32 distance, std::vector< SERIAL > &serials );

protected:
	QMap< UI08, cQuadNode* > rootmap_;
	static cMapObjects instance;
};

class RegionIterator4Chars
{
public:
	RegionIterator4Chars( const Coord_cl &pos, UI32 distance = 18 ); 
	void		Begin( void );
	bool		atEnd( void ) const;
	P_CHAR		GetData( void );
	void		reset( const Coord_cl &pos, UI32 distance = 18 )
	{
		position_ = pos;
		distance_ = distance;
		serials.clear();
		cMapObjects::getInstance()->search( pos, distance, this->serials );
		Begin();
	}

	// Operators
	RegionIterator4Chars& operator++( int );
	RegionIterator4Chars& operator=( const Coord_cl &pos );
protected:
	std::vector< SERIAL > serials;
	std::vector< SERIAL >::iterator	currentIterator;

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
	void		reset( const Coord_cl &pos, UI32 distance = 18 )
	{
		position_ = pos;
		distance_ = distance;
		serials.clear();
		cMapObjects::getInstance()->search( pos, distance, this->serials );
		Begin();
	}

	// Operators
	RegionIterator4Items& operator++( int );
	RegionIterator4Items& operator=( const Coord_cl &pos );
protected:
	std::vector< SERIAL > serials;
	std::vector< SERIAL >::iterator	currentIterator;

	Coord_cl	position_;
	UI32		distance_;
};

#endif

