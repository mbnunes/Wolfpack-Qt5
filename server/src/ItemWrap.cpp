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
//	Wolfpack Homepage: http://www.wpdev.com/
//========================================================================================

//////////////////////////////////
// name:	ItemWrap.cpp
// purpose:	implementation of the ItemWrap class
//			intended to control the access to the items[] by redefining 'items' as follows:
//				extern cItemWrap iwrap;
//				#define items (iwrap.prepare(__LINE__, DBGFILE))
//			Thus an access like 'item[i].name' will be mapped to realitems[i].name, with
//			a chance of detecting a bad index value and logging the line & file where it came from.
//			Because bad index values have to be directed to some place *within* the array, all items are loaded '1 up',
//			leaving items[0] free (to take all the damage). And items[i] is mapped to realitems[i+1]
// history:	init by Duke, 30.9.2000
//			added offset feature Duke, 5.10.2000
//			added suppressing empty wsc messages Duke 11.4.2001
//

#include "wolfpack.h"

#undef  DBGFILE
#define DBGFILE "ItemWrap.cpp"

cItemWrap::cItemWrap(){}

cItemWrap::~cItemWrap(){}

cItem& cItemWrap::operator[](long inx)
{
	if (inx<0 || inx>=itemcount)
	{
		if (!(itemcount==0 && inx==0))	// suppress 4 messages when starting the server with NO worldfiles
		{
			char tmp[222];
			sprintf(tmp,"bad item index %i in file %s line %i itemcount %i",inx, srcfile, srcline,itemcount);
			LogError(tmp);
		}
		return realitems[0];	// VERY bad for item 0, but it keeps the server alive ;-)
	}
	else
	{
		return realitems[inx+I_W_O_1];	// plus 1 offset
	}
}

cItemWrap& cItemWrap::prepare(int line, char* fn)
{
	srcline = line;
	srcfile = fn;
	return *this;
}
