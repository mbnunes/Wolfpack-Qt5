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
//	Wolfpack Homepage: http://www.wpdev.sf.net/
//========================================================================================

// Platform Specifics
#include "platform.h"

// Wolfpack includes
#include "coord.h"
#include "uobject.h"
#include "iserialization.h"
#include "defines.h"

// Debug includes and defines
#undef  DBGFILE
#define DBGFILE "uobject.cpp"

//##ModelId=3C5D92B0021B
cUObject::cUObject()
{
	init();
}

//##ModelId=3C5D92B0021C
cUObject::cUObject( cUObject &src )
{
	this->serial = src.serial;
	this->multis = src.multis;
	this->name = src.name;
	this->free = src.free;
}

//##ModelId=3C5D92B0022F
cUObject::~cUObject()
{
}

//##ModelId=3C5D92B00207
void cUObject::init()
{
	this->serial = INVALID_SERIAL;
	this->multis = INVALID_SERIAL;
	this->free = false;
}


/*!
 * Provides persistence for instances of UOBject
 *
 * @param &archive : an ISerialization descendent.
 *
 * @return void  : none.
 */
//##ModelId=3C5D92B00239
void cUObject::Serialize(ISerialization &archive)
{
	if (archive.isReading())
	{
		archive.read("name", name);
		archive.read("serial", serial);
		archive.read("multis", multis);
		archive.read("pos.x", pos.x);
		archive.read("pos.y", pos.y);
		archive.read("pos.z", pos.z);
		archive.read("pos.map", pos.map);
		archive.read("pos.plane", pos.plane);
	}
	else if (archive.isWritting())
	{
		archive.write("name", name);
		archive.write("serial", serial);
		archive.write("multis", multis);
		archive.write("pos.x", pos.x);
		archive.write("pos.y", pos.y);
		archive.write("pos.z", pos.z);
		archive.write("pos.map", pos.map);
		archive.write("pos.plane", pos.plane);
		archive.doneWritting();
	}
}

//##ModelId=3C5D92B00257
inline string cUObject::objectID()
{
	return string("UOBJECT");
}
