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

#if !defined(__BASEREGION_H__)
#define __BASEREGION_H__

#include "definable.h"
#include "platform.h"
#include "coord.h"
#include "typedefs.h"
#include "wpdefmanager.h"

// Library includes
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluevector.h>
#include <qmap.h>

// Forward Definitions

class cBaseRegion : public cDefinable
{
public:
	struct rect_st
	{
		UI16 x1;
		UI16 x2;
		UI16 y1;
		UI16 y2;
		UI08 map;
	};

	cBaseRegion(): parent_( 0 ) {;}

	cBaseRegion( const cElement *tag, cBaseRegion* pParent )
	{
		this->init();
		this->name_ = tag->getAttribute( "id" );
		this->applyDefinition( tag );
		this->parent_ = pParent;
	}

	virtual ~cBaseRegion()
	{
		QValueVector< cBaseRegion* >::iterator it = this->subregions_.begin();
		while( it != this->subregions_.end() )
		{
			delete (*it);
			++it;
		}
	}

	void	init( void )
	{
		name_ = "the wilderness";
	}

	bool	contains( UI16 posx, UI16 posy, UI08 map ) const
	{
		QValueVector< rect_st >::const_iterator it = this->rectangles_.begin();

		while( it != this->rectangles_.end() )
		{
			if( ( ( posx >= (*it).x1 && posx <= (*it).x2 )   ||
				  ( posx >= (*it).x2 && posx <= (*it).x2 ) ) &&
				( ( posy >= (*it).y1 && posy <= (*it).y2 )   ||
				  ( posy >= (*it).y2 && posy <= (*it).y1 ) ) &&
				  map == (*it).map )
				  return true;
			++it;
		}
		return false;
	}

	cBaseRegion*				region( const QString& regName )
	{
		if( this->name_ == regName )
			return this;
		else
		{
			QValueVector< cBaseRegion* >::const_iterator it = this->subregions_.begin();
			while( it != this->subregions_.end() )
			{
				cBaseRegion* currRegion = 0;
				if( *it != 0 )
					currRegion = (*it)->region( regName );
				if( currRegion != 0 )
					return currRegion;
				++it;
			}
		}
		return 0;
	}

	cBaseRegion*				region( UI16 posx, UI16 posy, UI08 map )
	{
		cBaseRegion* foundRegion = 0;
		if( this->contains( posx, posy, map ) )
			foundRegion = this;
		else
			return 0;

		QValueVector< cBaseRegion* >::iterator it = this->subregions_.begin();
		while( it != this->subregions_.end() )
		{
			cBaseRegion* currRegion = 0;
			if( *it != 0 )
				currRegion = (*it)->region( posx, posy, map );
			if( currRegion != 0 )
				foundRegion = currRegion;
			++it;
		}
		return foundRegion;
	}

	UI32						count( void ) const
	{
		UI32 result = 1;
		QValueVector< cBaseRegion* >::const_iterator it(this->subregions_.begin());
		while( it != this->subregions_.end() )
		{
			if( *it != NULL )
				result += (*it)->count();
			++it;
		}
		return result;
	}

protected:
	virtual void processNode( const cElement *Tag )
	{
		QString TagName = Tag->name();
		QString Value = Tag->value();

		// <rectangle x1="0" x2="1000" y1="0" y2="500" />
		if( TagName == "rectangle"  )
		{
			rect_st toinsert_;
			toinsert_.x1  = Tag->getAttribute( "x1" ).toUShort();
			toinsert_.x2  = Tag->getAttribute( "x2" ).toUShort();
			toinsert_.y1  = Tag->getAttribute( "y1" ).toUShort();
			toinsert_.y2  = Tag->getAttribute( "y2" ).toUShort();
			toinsert_.map = Tag->getAttribute( "map").toUShort();
			this->rectangles_.push_back( toinsert_ );
		}
		else if( TagName == "region" && Tag->hasAttribute( "id" ) )
			this->subregions_.push_back( new cBaseRegion( Tag, this ) );
	}

protected:
	QString							name_;			// name of the region (section's name)
	QValueVector< rect_st >			rectangles_;	// vector of rectangles
	QValueVector< cBaseRegion* >	subregions_;	// list of region object references of included regions
	cBaseRegion*					parent_;		// the region directly above this region
public:
	// Only getters, no setters
	cBaseRegion *parent() const { return parent_; }
	QValueVector< cBaseRegion* > &children() { return subregions_; }
	QValueVector< rect_st > &rectangles() { return rectangles_; }
};

class cAllBaseRegions
{
public:
	virtual ~cAllBaseRegions()
	{
		QMap<uint, cBaseRegion*>::const_iterator it( topregions.begin() );
		for ( ; it != topregions.end(); ++it )
			delete it.data();
	}

	virtual void load( void ) = 0;

	void reload( void )
	{
		QMap<uint, cBaseRegion*>::iterator it( topregions.begin() );
		for ( ; it != topregions.end(); ++it )
			delete it.data();

		this->load();
	}

	cBaseRegion*	region( const QString& regName )
	{
		QMap<uint, cBaseRegion*>::const_iterator it( topregions.begin() );
		for ( ; it != topregions.end(); ++it )
		{
			cBaseRegion* result = it.data()->region( regName );
			if ( result )
				return result;
		}
		return 0;
	}

	cBaseRegion*	region( UI16 posx, UI16 posy, UI08 map )
	{
		QMap<uint, cBaseRegion*>::const_iterator it( topregions.find(map) );
		if ( it != topregions.end() )
			return it.data()->region( posx, posy, map );
		else
			return 0;
	}

	UI32			count( void ) const
	{
		UI32 i = 0;
		QMap<uint, cBaseRegion*>::const_iterator it( topregions.begin() );
		for ( ; it != topregions.end(); ++it )
			i += it.data()->count();
		return i;
	}

protected:
	QMap<uint, cBaseRegion*> topregions;
};

#endif

