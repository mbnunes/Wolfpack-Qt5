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
#if !defined(__COORD_H__)
#define __COORD_H__

// Platform specifics
#include "platform.h"


// System Includes
#include <algorithm>
#include <cmath>
using namespace std;


// Third Party includes



//Forward class declaration

class Coord_cl;

//Wolfpack Includes

#include "wpconsole.h"

// Any specific structures


// Externals we need
extern WPConsole_cl clConsole;

// typedefs


//Class definitions

class Coord_cl
{
public:
	Coord_cl(void);
	Coord_cl(const Coord_cl& clCoord);
	Coord_cl(UI16, UI16, SI08, UI08 uiMap = 0, UI08 uiPlane = 0);
	// Operators
	Coord_cl& operator= (const Coord_cl& clCoord);
	bool operator==(const Coord_cl&);
	bool operator!=(const Coord_cl&);
	Coord_cl operator+(const Coord_cl& src) const;
	UI32 distance( const Coord_cl &src) const;
	static UI32 distance ( const Coord_cl &a, const Coord_cl &b );

	/*!
		Displays an effect staying at this position.
	*/
	void effect( UI16 id, UI08 speed = 10, UI08 duration = 5, UI16 hue = 0, UI16 renderMode = 0 );
	
	/*!
		Displays a lightning bolt at this position.
	*/
	void lightning( UI08 speed = 10, UI08 duration = 5, UI16 hue = 0, UI16 renderMode = 0 );

public:
	UI16 	x;
	UI16 	y;
	SI08 	z;
	UI08    map;
};

#endif // __COORD_H__
