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

#include "customtags.h"
#include "junk.h"
#include "chars.h"
#include "items.h"

bool cVariant::isChar( void )
{
	return (UI32)intvalue_ < 0x40000000 && FindCharBySerial( (UI32)intvalue_ );
}

bool cVariant::isItem( void )
{
	return (UI32)intvalue_ >= 0x40000000 && FindItemBySerial( (UI32)intvalue_ );
}

SI32 cVariant::asInt( void )		
{
	if( isValue() )
		return intvalue_;
	else if( isString() )
		return strvalue_.toInt();
	else
		return 0;
}

UI32 cVariant::asUInt( void )
{
	return (UI32)asInt();
}

QString	cVariant::asString( void )
{
	if( isValue() )
		return QString("%1").arg( intvalue_ );
	else if( isString() )
		return strvalue_;
	else
		return QString();
}

P_CHAR	cVariant::asChar( void )
{
	if( isChar() )
		return FindCharBySerial( (UI32)intvalue_ );
	else
		return NULL;
}

P_ITEM	cVariant::asItem( void )
{
	if( isItem() )
		return FindItemBySerial( (UI32)intvalue_ );
	else
		return NULL;
}

SERIAL	cVariant::asSerial( void ) 
{ 
	if( isChar() || isItem() )
		return (UI32)intvalue_;	
	else if( isString() && ( ( strvalue_.toUInt() < 0x40000000 && FindCharBySerial( strvalue_.toUInt() ) ) ||
							 ( strvalue_.toUInt() >= 0x40000000 && FindCharBySerial( strvalue_.toUInt() ) ) ) )
		return strvalue_.toUInt();
	else
		return INVALID_SERIAL;
}

cVariant& cVariant::operator=(SI32 data)
{
	this->intvalue_ = data;
	this->strvalue_ = (char*)0;
	return *this;
}

cVariant& cVariant::operator=(QString data)
{
	this->intvalue_ = 0;
	this->strvalue_ = data;
	return *this;
}

cVariant& cVariant::operator=(cChar* data)
{
	if( data != NULL )
		this->intvalue_ = (SI32)data->serial;
	else
		this->intvalue_ = INVALID_SERIAL;
	this->strvalue_ = (char*)0;
	return *this;
}

cVariant& cVariant::operator=(cItem* data)
{
	if( data != NULL )
		this->intvalue_ = (SI32)data->serial;
	else
		this->intvalue_ = INVALID_SERIAL;
	this->strvalue_ = (char*)0;
	return *this;
}
