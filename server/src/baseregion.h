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

#if !defined(__BASEREGION_H__)
#define __BASEREGION_H__ 

#include "definable.h"
#include "platform.h"
#include "coord.h"
#include "typedefs.h"

// Library includes
#include "qstring.h"
#include "qstringlist.h"
#include "qdom.h"

#include <vector>

// Forward Definitions

class cBaseRegion : public cDefinable
{
public:
	cBaseRegion(): parent_( 0 ) {;}

	cBaseRegion( const QDomElement& Tag, cBaseRegion* pParent )
	{
		this->init();
		this->name_ = Tag.attribute( "id" );
		this->applyDefinition( Tag );
		this->parent_ = pParent;
	}

	virtual ~cBaseRegion()
	{
		std::vector< cBaseRegion* >::iterator it = this->subregions_.begin();
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

	bool	contains( UI16 posx, UI16 posy ) const
	{
		std::vector< rect_st >::const_iterator it = this->rectangles_.begin();
		while( it != this->rectangles_.end() )
		{
			if( ( ( posx >= (*it).x1 && posx <= (*it).x2 )   ||
				  ( posx >= (*it).x2 && posx <= (*it).x2 ) ) &&
				( ( posy >= (*it).y1 && posy <= (*it).y2 )   ||
				  ( posy >= (*it).y2 && posy <= (*it).y1 ) ) )
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
			std::vector< cBaseRegion* >::iterator it = this->subregions_.begin();
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

	cBaseRegion*				region( UI16 posx, UI16 posy )
	{
		cBaseRegion* foundRegion = 0;
		if( this->contains( posx, posy ) )
			foundRegion = this;
		else
			return 0;
		
		std::vector< cBaseRegion* >::iterator it = this->subregions_.begin();
		while( it != this->subregions_.end() )
		{
			cBaseRegion* currRegion = 0;
			if( *it != 0 )
				currRegion = (*it)->region( posx, posy );
			if( currRegion != 0 )
				foundRegion = currRegion;
			++it;
		}
		return foundRegion;
	}

	UI32						count( void ) const
	{
		UI32 result = 1;
		std::vector< cBaseRegion* >::const_iterator it(this->subregions_.begin());
		while( it != this->subregions_.end() )
		{
			if( *it != NULL )
				result += (*it)->count();
			++it;
		}
		return result;
	}

	// Only getters, no setters
	cBaseRegion *parent() const { return parent_; }
	std::vector< cBaseRegion* > children() const { return subregions_; }
protected:
	virtual void processNode( const QDomElement &Tag )
	{
		QString TagName = Tag.nodeName();
		QString Value = this->getNodeValue( Tag );
	
		// <rectangle x1="0" x2="1000" y1="0" y2="500" />
		if( TagName == "rectangle"  )
		{
			rect_st toinsert_;
			toinsert_.x1 = Tag.attribute( "x1" ).toUShort();
			toinsert_.x2 = Tag.attribute( "x2" ).toUShort();
			toinsert_.y1 = Tag.attribute( "y1" ).toUShort();
			toinsert_.y2 = Tag.attribute( "y2" ).toUShort();
			this->rectangles_.push_back( toinsert_ );
		}

		else if( TagName == "region" && Tag.attributes().contains( "id" ) )
			this->subregions_.push_back( new cBaseRegion( Tag, this ) );
	}

protected:
	struct rect_st 
	{
		UI16 x1;
		UI16 x2;
		UI16 y1;
		UI16 y2;
	};

	QString							name_;			// name of the region (section's name)
	std::vector< rect_st >			rectangles_;	// vector of rectangles
	std::vector< cBaseRegion* >		subregions_;	// list of region object references of included regions
	cBaseRegion*					parent_;		// the region directly above this region
};

class cAllBaseRegions
{
public:
	virtual ~cAllBaseRegions() {}

	virtual void load( void ) = 0;

	void reload( void )
	{
		if( this->topregion_ )
			delete this->topregion_;

		this->load();
	}

	cBaseRegion*	region( const QString& regName )
	{
		if( topregion_ )
			return topregion_->region( regName );
		else
			return 0;
	}

	cBaseRegion*	region( UI16 posx, UI16 posy )
	{
		if( topregion_ )
			return topregion_->region( posx, posy );
		else
			return 0;
	}

	UI32			count( void ) const
	{
		if( topregion_ )
			return topregion_->count();
		else
			return 0;
	}
protected:
	cBaseRegion* topregion_;
};

#endif

