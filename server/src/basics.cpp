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

////////////////////////
// name:	basics.cpp
// purpose:	implementation of basic functions (that don't need wolfpack definitions)
//
//

#include "basics.h"
#include "qstringlist.h"
#include "coord.h"

// Library Includes
#include "qstring.h"


int RandomNum(int nLowNum, int nHighNum)
{
	if (nHighNum - nLowNum + 1)
		return ((rand() % (nHighNum - nLowNum + 1)) + nLowNum);
	else
		return nLowNum;
}

int rollDice( QString dicePattern ) // roll dices d&d style
{
	// dicePattern looks like "xdy+z"
	// which equals RandomNum(x,y)+z

	int doffset = dicePattern.find("d"), poffset = dicePattern.find("+");
	int x = dicePattern.left( doffset ).toInt();
	int z = dicePattern.right( dicePattern.length()-1 - poffset ).toInt();
	int y = dicePattern.mid( doffset+1, poffset-doffset-1 ).toInt();

	return RandomNum(x,y)+z;
}

int str2num( string s, int base)
{
	return str2num((char*)s.c_str(),base) ;
}

int str2num(char *s, int base) // Convert string to integer
{
	char* dummy; // ignore the stop pointer
	return strtol(s, &dummy, base);
}

bool parseCoordinates( const QString &input, Coord_cl &coord )
{
	QStringList coords = QStringList::split( ",", input );
	
	// We at least need x, y, z
	if( coords.size() < 3 )
		return false;

	bool ok = false;

	UINT16 x = coords[0].toULong( &ok );
	if( !ok )
		return false;

	UINT16 y = coords[1].toULong( &ok );
	if( !ok )
		return false;

	INT8 z = coords[2].toShort( &ok );
	if( !ok )
		return false;

	UINT8 plane = coord.plane; // Current by default

	if( coords.size() > 3 )
	{
		plane = coords[3].toUShort( &ok );

		if( !ok )
			return false;
	}

	// They are 100% valid now, so let's move! 
	// TODO: Add Mapbounds check here
	coord.x = x;
	coord.y = y;
	coord.z = z;
	coord.plane = plane;

	return true;
}
