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

// MapObjects class, replaces cRegion stuff
// - sereg, 09/13/02

#include "mapobjects.h"
#include "debug.h"
#include "maps.h"
#include "uobject.h"
#include "junk.h"
#include "globals.h"

#include <math.h>
#include <algorithm>

using namespace std;

#undef  DBGFILE
#define DBGFILE "mapobjects.cpp"


/*
 * Internal Classes
*/

/*!
	\internal
	\class cQuadNode mapobjects.cpp

	\brief The cQuadNode class implements a QuadTree node.

	cQuadNode is a QuadTree node. QuadTrees are trees with 4
	childs. It's used by Wolfpack to easily index objects
	according to it's map positions, relatively to each other.
*/

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


	void	search( UI16 left, UI16 right, UI16 top, UI16 bottom, UI16 srcx, UI16 srcy, UI32 distance, std::vector< cMapObjects::type > &serials );

	bool	contains( cMapObjects::type serial );
	void	add( UI16 srcx, UI16 srcy, cMapObjects::type serial );
	bool	remove( UI16 srcx, UI16 srcy, cMapObjects::type serial );
	void	pushdown( cQuadNode* node );

	std::vector< cMapObjects::type >	objectserials;
private:
	UI16 x_;
	UI16 y_;
	cQuadNode*	parent_;
};


/*****************************************************************************
  cQuadNode member functions
 *****************************************************************************/

cQuadNode::cQuadNode( UI16 x, UI16 y, cQuadNode* parent ) : x_( x ), y_( y ), parent_( parent )
{
	for( int i = 0; i < enNumberOfChilds; ++i )
		childs[i] = 0;
}

cQuadNode::~cQuadNode()
{
	for( int i = 0; i < enNumberOfChilds; ++i )
		delete childs[i];
}

UI32 cQuadNode::distance( UI16 srcx, UI16 srcy )
{
	RF64 rfResult = sqrt(static_cast<RF64>((x_ - srcx)*(x_ - srcx) + (y_ - srcy)*(y_ - srcy)));
	return static_cast<UI32>(floor(rfResult)); // truncate
}

cQuadNode::enQuadrants cQuadNode::compare( UI16 srcx, UI16 srcy )
{
	if( srcx <= x_ )
	{
		if( srcy <= y_ )
			return southwest;
		else
			return northwest;
	}
	else
	{
		if( srcy <= y_ )
			return southeast;
		else
			return northeast;
	}
}

bool cQuadNode::inRange( UI16 srcx, UI16 srcy, UI32 distance )
{
	return ( this->distance( srcx, srcy ) <= distance );
}

bool cQuadNode::overlap( UI16 left, UI16 right, UI16 bottom, UI16 top, UI16 srcx, UI16 srcy, UI32 distance )
{
	return ( left <= (srcx + distance) && right >= (srcx - distance) &&
			 bottom <= (srcy + distance) && top >= (srcy - distance) );
}

void cQuadNode::search( UI16 left, UI16 right, UI16 bottom, UI16 top, UI16 srcx, UI16 srcy, UI32 distance, std::vector< cMapObjects::type > &serials )
{
	if( inRange( srcx, srcy, distance ) )
		serials.insert( serials.end(), objectserials.begin(), objectserials.end() );

	if( childs[ northeast ] && childs[ northeast ]->overlap( x_, right, y_, top, srcx, srcy, distance ) )
		childs[ northeast ]->search( x_, right, y_, top, srcx, srcy, distance, serials );

	if( childs[ northwest ] && childs[ northwest ]->overlap( left, x_, y_, top, srcx, srcy, distance ) )
		childs[ northwest ]->search( left, x_, y_, top, srcx, srcy, distance, serials );

	if( childs[ southwest ] && childs[ southwest ]->overlap( left, x_, bottom, y_, srcx, srcy, distance ) )
		childs[ southwest ]->search( left, x_, bottom, y_, srcx, srcy, distance, serials );

	if( childs[ southeast ] && childs[ southeast ]->overlap( x_, right, bottom, y_, srcx, srcy, distance ) )
		childs[ southeast ]->search( x_, right, bottom, y_, srcx, srcy, distance, serials );
}

bool cQuadNode::contains( cMapObjects::type serial )
{
	return binary_search( objectserials.begin(), objectserials.end(), serial );
}

void cQuadNode::pushdown( cQuadNode* node )
{
	if( node )
	{
		register int i = 0;
		while( i < enNumberOfChilds )
		{
			cQuadNode* currchild = node->childs[i];
			if( currchild )
			{
				node->childs[i] = NULL;
				pushdown( currchild );
			}
			++i;
		}

		if( x_ == node->x() && y_ == node->y() )
		{
			qWarning( "Warning: two nodes on same positions found!\n" );
			delete node;
			return;
		}

		enQuadrants child = compare( node->x(), node->y() );
		if( childs[ child ] )
			childs[ child ]->pushdown( node );
		else
		{
			childs[ child ] = node;
			node->setParent( this );
		}
	}
}

void cQuadNode::add( UI16 srcx, UI16 srcy, cMapObjects::type serial )
{
	if( srcx == x_ && srcy == y_ )
	{
		if( !contains( serial ) )
		{
			objectserials.push_back( serial );
			sort( objectserials.begin(), objectserials.end() );
			return;
		}
		else
			return;
	}
	else
	{
		enQuadrants child = compare( srcx, srcy );
		if( !childs[ child ] )
			childs[ child ] = new cQuadNode( srcx, srcy, this );
		childs[ child ]->add( srcx, srcy, serial );
	}
}

bool cQuadNode::remove( UI16 srcx, UI16 srcy, cMapObjects::type serial )
{
	if( srcx == x_ && srcy == y_ )
	{
		// delete serial 
		std::vector< cMapObjects::type >::iterator it = find( objectserials.begin(), objectserials.end(), serial );
		if( it != objectserials.end() )
			objectserials.erase( it );
		if( objectserials.empty() )
		{
			// delete node
			if( parent_ )
			{
				register int pchild = 0;
				while( pchild < enNumberOfChilds )
				{
					if( parent_->childs[ pchild ] == this )
						break;
					++pchild;
				}
				if( pchild == enNumberOfChilds )
					return true;

				register int child = 0;
				while( child < enNumberOfChilds )
				{
					if( childs[ child ] )
						break;
					++child;
				}
				if( child == enNumberOfChilds )
				{
					parent_->childs[ pchild ] = NULL;
					return true;
				}
				
				parent_->childs[ pchild ] = childs[ child ];
				childs[ child ]->setParent( parent_ );
				++child;
				while( child < enNumberOfChilds )
				{
					if( childs[ child ] )
						parent_->childs[ pchild ]->pushdown( childs[ child ] );
					++child;
				}
				child = 0;
				while( child < enNumberOfChilds )
				{
					childs[ child ] = NULL;
					++child;
				}
				return true;
			}
		}
		return false;
	}

	enQuadrants child = compare( srcx, srcy );
	cQuadNode* currchild = childs[ child ];
	if( currchild && currchild->remove( srcx, srcy, serial ) )
	{
		delete currchild;
	}
	return false;
}

/*****************************************************************************
  cMapObjects member functions
 *****************************************************************************/

cMapObjects::cMapObjects()
{
//	rootmap_.insert( 0, new cQuadNode( 3072, 2048 ) ); // unnecessary, see add()
}

cMapObjects::~cMapObjects()
{
	QMap< UI08, cQuadNode* >::iterator it = rootmap_.begin();
	while( it != rootmap_.end() )
	{
		delete it.data();
		++it;
	}
}

void cMapObjects::search( const Coord_cl &pos, UI32 distance, std::vector< cMapObjects::type > &serials )
{
	QMap< UI08, cQuadNode* >::iterator nit = rootmap_.find( pos.map );
	if( nit != rootmap_.end() )
		nit.data()->search( 0,  Map->mapTileWidth( pos.map ) * 8, 0, Map->mapTileHeight( pos.map ) * 8, pos.x, pos.y, distance, serials );
	else
		return;
}

void cMapObjects::add( cUObject* object )
{
	QMap< UI08, cQuadNode* >::iterator it = rootmap_.find( object->pos.map );
	if( it != rootmap_.end() )
		it.data()->add( object->pos.x, object->pos.y, object->serial );
	else
	{
		// root node in center of map.
		cQuadNode* root = new cQuadNode( Map->mapTileWidth( object->pos.map ) * 4, Map->mapTileHeight( object->pos.map ) * 4 );
		rootmap_.insert( object->pos.map, root );
		root->add( object->pos.x, object->pos.y, object->serial );
	}
}

void cMapObjects::remove( cUObject* object )
{
	QMap< UI08, cQuadNode* >::iterator it = rootmap_.find( object->pos.map );
	if( it != rootmap_.end() )
	{
		it.data()->remove( object->pos.x, object->pos.y, object->serial );
	}
}

/*****************************************************************************
  RegionIterator4Items member functions
 *****************************************************************************/

RegionIterator4Items::RegionIterator4Items( const Coord_cl &pos, UI32 distance )
{
	position_ = pos;
	distance_ = distance;
	MapObjects::instance()->search( pos, distance, this->serials );
}

RegionIterator4Items& RegionIterator4Items::operator=( const Coord_cl &pos )
{
	position_ = pos;
	serials.clear();
	MapObjects::instance()->search( pos, distance_, this->serials );
	Begin();
	return *this;
}

void RegionIterator4Items::Begin()
{
	currentIterator = serials.begin();
	while( currentIterator != serials.end() && !isItemSerial( *currentIterator ) )
		++currentIterator;
}

bool RegionIterator4Items::atEnd() const
{
	return currentIterator == serials.end();
}

void RegionIterator4Items::reset( const Coord_cl &pos, UI32 distance )
{
	position_ = pos;
	distance_ = distance;
	serials.clear();
	MapObjects::instance()->search( pos, distance, this->serials );
	Begin();
}

RegionIterator4Items& RegionIterator4Items::operator++ ( int )
{
	++currentIterator;
	while( currentIterator != serials.end() && !isItemSerial( *currentIterator ) )
		++currentIterator;
	return *this;
}

P_ITEM RegionIterator4Items::GetData()
{
	P_ITEM pi = NULL;
	while( !(pi = FindItemBySerial( *currentIterator )) && !atEnd() )
		(*this)++;
	return pi;
}

/*****************************************************************************
  RegionIterator4Chars member functions
 *****************************************************************************/

RegionIterator4Chars::RegionIterator4Chars( const Coord_cl &pos, UI32 distance )
{
	position_ = pos;
	distance_ = distance;
	MapObjects::instance()->search( pos, distance, this->serials );
}

RegionIterator4Chars& RegionIterator4Chars::operator=( const Coord_cl &pos )
{
	position_ = pos;
	serials.clear();
	MapObjects::instance()->search( pos, distance_, this->serials );
	Begin();
	return *this;
}

void RegionIterator4Chars::Begin()
{
	currentIterator = serials.begin();
	while( currentIterator != serials.end() && !isCharSerial( *currentIterator ) )
		++currentIterator;
}

bool RegionIterator4Chars::atEnd() const
{
	return currentIterator == serials.end();
}

void RegionIterator4Chars::reset( const Coord_cl &pos, UI32 distance )
{
	position_ = pos;
	distance_ = distance;
	serials.clear();
	MapObjects::instance()->search( pos, distance, this->serials );
	Begin();
}


RegionIterator4Chars& RegionIterator4Chars::operator++ ( int )
{
	++currentIterator;
	while( currentIterator != serials.end() && !isCharSerial( *currentIterator ) )
		++currentIterator;
	return *this;
}

P_CHAR RegionIterator4Chars::GetData()
{
	P_CHAR pc = NULL;
	while( !(pc = FindCharBySerial( *currentIterator )) && !atEnd() )
		(*this)++;
	return pc;
}
