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


// Include files

#include "coord.h"
#include "debug.h"
#include "network/uotxpackets.h"
#include "network/uosocket.h"

// System Includes
#include <math.h>

#undef  DBGFILE
#define DBGFILE "coord.cpp"

Coord_cl::Coord_cl(void)
{
	x = y = z = map = 0;
}

Coord_cl::Coord_cl(const Coord_cl& clCoord)
{
	(*this) = clCoord;
}

Coord_cl::Coord_cl(UI16 uiX, UI16 uiY, SI08 siZ, UI08 uiMap, UI08)
{
	x = uiX;
	y = uiY;
	z = siZ;
	map = uiMap;
	//plane = uiPlane;
}


/*!
 * Calculates the distance between 2 coordinates. If they are in different planes and maps, the
 * return value is infinite (0xFFFFFFFF). The exception is if one of the planes is COORD_ALL_PLANES
 *
 * @param &src : Coordinate to compare
 *
 * @return UI32  : distance result
 */
UI32 Coord_cl::distance( const Coord_cl &src) const
{
	UI32 uiResult = ~0; // Initialize with *infinite*
	if ( map == src.map )
	{
		RF64 rfResult = sqrt(static_cast<RF64>((x - src.x)*(x - src.x) + (y - src.y)*(y - src.y)/* + (z - src.z)*(z - src.z)*/));
		//		RF64 rfResult = sqrt(static_cast<RF64>((x * src.x + y * src.y + z * src.z)));
		uiResult = static_cast<UI32>(floor(rfResult)); // truncate
	}
	return uiResult;
}

UI32 Coord_cl::distance ( const Coord_cl &a, const Coord_cl &b )
{
	return a.distance( b );
}

// Operators
Coord_cl& Coord_cl::operator=(const Coord_cl& clCoord)
{
	x = clCoord.x;
	y = clCoord.y;
	z = clCoord.z;
	map = clCoord.map;
	return (*this);
}

bool Coord_cl::operator== (const Coord_cl& src)
{
	return (x == src.x && y == src.y && z == src.z && map == src.map);
}

bool Coord_cl::operator!= (const Coord_cl& src)
{
	return !(x == src.x && y == src.y && z == src.z && map == src.map);
}

Coord_cl Coord_cl::operator+ (const Coord_cl& src) const
{
	return Coord_cl(this->x + src.x, this->y + src.y, this->z + src.z);
}

void Coord_cl::lightning( UINT8 speed, UINT8 duration, UINT16 hue, UINT16 renderMode )
{
	cUOTxEffect effect;
	effect.setType( ET_LIGHTNING );
	effect.setSourcePos( (*this) );
    effect.setSpeed( speed );
	effect.setDuration( duration );
	effect.setHue( hue );
	effect.setRenderMode( renderMode );

	cUOSocket *mSock = 0;
	for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		if( mSock->player() && ( mSock->player()->pos.distance( (*this) ) <= mSock->player()->VisRange() ) )
			mSock->send( &effect );
	}
}

void Coord_cl::effect( UINT16 id, UINT8 speed, UINT8 duration, UINT16 hue, UINT16 renderMode )
{
	cUOTxEffect effect;
	effect.setType( ET_STAYSOURCEPOS );
	effect.setSourcePos( (*this) );
	effect.setId( id );
    effect.setSpeed( speed );
	effect.setDuration( duration );
	effect.setHue( hue );
	effect.setRenderMode( renderMode );

	cUOSocket *mSock = 0;
	for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		if( mSock->player() && ( mSock->player()->pos.distance( (*this) ) <= mSock->player()->VisRange() ) )
			mSock->send( &effect );
	}
}
