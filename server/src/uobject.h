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

#if !defined (__UOBJECT_H__)
#define __UOBJECT_H__

#include "platform.h"
#include "typedefs.h"
#include "coord.h"

// System includes
#include <string>

// Forward class declarations
class ISerialization;
class Coord_cl;

//##ModelId=3C5D92B0015D
class cUObject
{
// Data Members
public:
	//##ModelId=3C5D92B00186
	SERIAL serial;
	//##ModelId=3C5D92B001A4
	SERIAL multis;
	//##ModelId=3C5D92B001B7
	bool free;

	//##ModelId=3C5D92B001D6
	std::string name;
	//##ModelId=3C5D92B001F4
	Coord_cl pos;
// Methods
protected:
	//##ModelId=3C5D92B00207
	void init();
public:
	//##ModelId=3C5D92B0021B
	cUObject();
	//##ModelId=3C5D92B0021C
	cUObject( cUObject& ); // Copy constructor
	//##ModelId=3C5D92B0022F
	virtual ~cUObject() = 0;
	//##ModelId=3C5D92B00239
	virtual void Serialize(ISerialization &archive);
	//##ModelId=3C5D92B00257
	virtual std::string objectID();
};

#endif // __UOBJECT_H__
