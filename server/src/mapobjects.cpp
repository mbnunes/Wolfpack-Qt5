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

#include "wolfpack.h"
#include "mapobjects.h"
#include "debug.h"
#include "mapstuff.h"

#undef  DBGFILE
#define DBGFILE "mapobjects.cpp"

cQuadNode::cQuadNode( UI16 x, UI16 y, cQuadNode* parent ) : x_( x ), y_( y ), parent_( parent )
{
	objectserials.clear();
	for( register int i = 0; i < enNumberOfChilds; ++i )
		childs[i] = NULL;
}

cQuadNode::~cQuadNode()
{
	for( register int i = 0; i < enNumberOfChilds; ++i )
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

void cQuadNode::search( UI16 left, UI16 right, UI16 bottom, UI16 top, UI16 srcx, UI16 srcy, UI32 distance, std::vector< SERIAL > &serials )
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

bool cQuadNode::contains( SERIAL serial )
{
	return binary_search( objectserials.begin(), objectserials.end(), serial );
}

void cQuadNode::pushdown( cQuadNode* node )
{
	if( node )
	{
		if( x_ == node->x() && y_ == node->y() )
		{
			clConsole.send( tr("Warning: two nodes on same positions found!\n") );
			delete node;
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

void cQuadNode::add( UI16 srcx, UI16 srcy, SERIAL serial )
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

void cQuadNode::remove( UI16 srcx, UI16 srcy, SERIAL serial )
{
	if( srcx == x_ && srcy == y_ )
	{
		// delete serial 
		std::vector< SERIAL >::iterator it = find( objectserials.begin(), objectserials.end(), serial );
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
					return;

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
					delete this;
					return;
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

				delete this;
				return;
			}
		}
		return;
	}

	enQuadrants child = compare( srcx, srcy );
	if( childs[ child ] )
		childs[ child ]->remove( srcx, srcy, serial );
}

cMapObjects::cMapObjects()
{
	rootmap_.insert( 0, new cQuadNode( 3072, 2048 ) );
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

void cMapObjects::search( const Coord_cl &pos, UI32 distance, std::vector< SERIAL > &serials )
{
	QMap< UI08, cQuadNode* >::iterator nit = rootmap_.find( pos.map );
	if( nit != rootmap_.end() )
		nit.data()->search( 0, 6144, 0, 4096, pos.x, pos.y, distance, serials );
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
		cQuadNode* root = new cQuadNode( object->pos.x, object->pos.y );
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

RegionIterator4Items::RegionIterator4Items( const Coord_cl &pos, UI32 distance )
{
	position_ = pos;
	distance_ = distance;
	cMapObjects::getInstance()->search( pos, distance, this->serials );
}

RegionIterator4Items& RegionIterator4Items::operator=( const Coord_cl &pos )
{
	position_ = pos;
	serials.clear();
	cMapObjects::getInstance()->search( pos, distance_, this->serials );
	Begin();
	return *this;
}

void RegionIterator4Items::Begin()
{
	currentIterator = serials.begin();
	while( currentIterator != serials.end() && !isItemSerial( *currentIterator ) )
		currentIterator++;
}

bool RegionIterator4Items::atEnd() const
{
	return currentIterator == serials.end();
}

RegionIterator4Items& RegionIterator4Items::operator++ ( int )
{
	currentIterator++;
	while( currentIterator != serials.end() && !isItemSerial( *currentIterator ) )
		currentIterator++;
	return *this;
}

P_ITEM RegionIterator4Items::GetData()
{
	P_ITEM pi = NULL;
	while( !(pi = FindItemBySerial( *currentIterator )) && !atEnd() )
		(*this)++;
	return pi;
}

RegionIterator4Chars::RegionIterator4Chars( const Coord_cl &pos, UI32 distance )
{
	position_ = pos;
	distance_ = distance;
	cMapObjects::getInstance()->search( pos, distance, this->serials );
}

RegionIterator4Chars& RegionIterator4Chars::operator=( const Coord_cl &pos )
{
	position_ = pos;
	serials.clear();
	cMapObjects::getInstance()->search( pos, distance_, this->serials );
	Begin();
	return *this;
}

void RegionIterator4Chars::Begin()
{
	currentIterator = serials.begin();
	while( currentIterator != serials.end() && !isCharSerial( *currentIterator ) )
		currentIterator++;
}

bool RegionIterator4Chars::atEnd() const
{
	return currentIterator == serials.end();
}

RegionIterator4Chars& RegionIterator4Chars::operator++ ( int )
{
	currentIterator++;
	while( currentIterator != serials.end() && !isCharSerial( *currentIterator ) )
		currentIterator++;
	return *this;
}

P_CHAR RegionIterator4Chars::GetData()
{
	P_CHAR pc = NULL;
	while( !(pc = FindCharBySerial( *currentIterator )) && !atEnd() )
		(*this)++;
	return pc;
}

// Singleton
cMapObjects cMapObjects::instance;
