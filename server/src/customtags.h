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

#ifndef __CUSTOMTAGS_H
#define __CUSTOMTAGS_H

#include "platform.h"
#include "typedefs.h"
#include "defines.h"

// Library include
#include <qdom.h>
#include <map>
#include <vector>
#include "qstring.h"
#include "qstringlist.h"

// Forward Declarations
class cItem;
class cChar;

class cVariant
{
public:
	cVariant() { intvalue_ = 0; strvalue_ = (char*)0; }
	~cVariant() {;}

	bool		isValue( void )		{ return strvalue_.isNull(); }
	bool		isString( void )	{ return !strvalue_.isNull(); }
	bool		isChar( void );
	bool		isItem( void );
	bool		isSerial( void )	{ return isChar() || isItem(); }
	bool		isNull( void )		{ return strvalue_.isNull() && intvalue_ == 0; }

	SI32		asInt( void );
	UI32		asUInt( void );
	QString		asString( void );
	P_CHAR		asChar( void );
	P_ITEM		asItem( void );
	SERIAL		asSerial( void );

	cVariant& cVariant::operator=(SI32 data);
	cVariant& cVariant::operator=(QString data);
	cVariant& cVariant::operator=(P_CHAR data);
	cVariant& cVariant::operator=(P_ITEM data);

private:
	SI32		intvalue_;
	QString		strvalue_;
};

class cCustomTags
{
public:
	cCustomTags() {;}
	~cCustomTags() {;}

	cVariant	get( QString key ) { return this->tags_[ key ]; }
	void		set( QString key, cVariant value ) 
	{
		if( value.isNull() )
			this->tags_.erase( key );
		else
			this->tags_[ key ] = value; 
	}
	void		remove( QString key ) { this->tags_.erase( key ); }

	UI32		size( void ) { return this->tags_.size(); }

	QStringList getKeys( void )
	{
		QStringList keys_;
		std::map< QString, cVariant >::iterator it = this->tags_.begin();
		while( it != this->tags_.end() )
		{
			keys_.push_back( it->first );
			it++;
		}
		return keys_;
	}

	std::vector< cVariant > getValues( void )
	{
		std::vector< cVariant > values_;
		std::map< QString, cVariant >::iterator it = this->tags_.begin();
		while( it != this->tags_.end() )
		{
			values_.push_back( it->second );
			it++;
		}
		return values_;
	}

private:
	std::map< QString, cVariant > tags_;
};

#endif
