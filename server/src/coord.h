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

// Defines we need

#define COORD_ALL_PLANES 255
#define COORD_HIDDEN_PLANE 254

// Any specific structures


// Externals we need
extern WPConsole_cl clConsole;

// typedefs


//Class definitions

//##ModelId=3C5D92F6001D
class Coord_cl
{
public:

	//##ModelId=3C5D92F60045
	Coord_cl(void);
	//##ModelId=3C5D92F60050
	Coord_cl(const Coord_cl& clCoord) ;
	//##ModelId=3C5D92F60063
	Coord_cl(UI16, UI16, SI08, UI08 uiMap = 0, UI08 uiPlane = 0);
	// Operators
	//##ModelId=3C5D92F6008B
	Coord_cl& operator= (const Coord_cl& clCoord) ;
	//##ModelId=3C5D92F6009F
	bool operator==(const Coord_cl&);
	//##ModelId=3C5D92F600B3
	Coord_cl operator+(const Coord_cl& src) const;

	//##ModelId=3C5D92F600C7
	UI32 distance(Coord_cl &src);

public:

	//##ModelId=3C5D92F600E6
	UI16 	x;
	//##ModelId=3C5D92F60104
	UI16 	y;
	//##ModelId=3C5D92F60122
	SI08 	z;
	//##ModelId=3C5D92F60154
	UI08    map;
	//##ModelId=3C5D92F60172
	UI08    plane;

};

#endif // __COORD_H__
