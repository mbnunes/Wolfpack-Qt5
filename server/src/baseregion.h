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
#include "qfile.h"

#include <vector>
#include <map>

class cBaseRegion : public cDefinable
{
public:
	virtual ~cBaseRegion() = 0 {;}

	void	Init( void )
	{
		name_ = QString();
	}

	bool	contains( UI16 posx, UI16 posy )
	{
		for( UI32 i = 0; i < this->x1_.size(); i++ )
		{
			if( ( ( posx >= x1_[i] && posx <= x2_[i] ) ||
				( posx >= x2_[i] && posx <= x2_[i] ) ) &&
				( ( posy >= y1_[i] && posy <= y2_[i] ) ||
				( posy >= y2_[i] && posy <= y2_[i] ) ) )
			{
				return true;
			}
		}
		return false;
	}

protected:
	virtual void processNode( const QDomElement &Tag )
	{
		QString TagName = Tag.nodeName();
		QString Value = this->getNodeValue( Tag );
	
		// <rectangle x1="0" x2="1000" y1="0" y2="500" />
		if( TagName == "rectangle"  )
		{
			this->x1_.push_back( Tag.attribute( "x1" ).toUShort() );
			this->x2_.push_back( Tag.attribute( "x2" ).toUShort() );
			this->y1_.push_back( Tag.attribute( "y1" ).toUShort() );
			this->y2_.push_back( Tag.attribute( "y2" ).toUShort() );
		}
	}

protected:
	QString					name_;			// name of the region (section's name)
	std::vector< UI16 >		x1_;			// vector of Top left X
	std::vector< UI16 >		x2_;			// vector of Bottom right x
	std::vector< UI16 >		y1_;			// vector of Top left y
	std::vector< UI16 >		y2_;			// vector of Bottom right y
	QStringList				subregions_;		// list of region identifiers of included regions by this one
};

class cAllBaseRegions : public std::map< QString, cBaseRegion* >
{
public:
	virtual ~cAllBaseRegions() = 0 {;}

	virtual void Load( void ) = 0;

	void reload( void )
	{
		iterator it = this->begin();
		while( it != this->end() )
		{
			delete it->second; // delete the objects from the stack!
			it++;
		}

		this->Load();
	}

	cBaseRegion*	region( QString regName )
	{
		iterator it = this->find( regName );
		if( it != this->end() )
			return it->second;
		else
			return NULL;
	}

};

#endif