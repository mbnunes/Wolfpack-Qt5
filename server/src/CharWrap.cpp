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
// name:	CharWrap.cpp
// purpose:	implementation of the cCharWrap class
//			intended to control the access to the chars[] by redefining 'chars' as follows:
//				extern cCharWrap cwrap;
//				#define chars (cwrap.prepare(__LINE__, DBGFILE))
//			Thus an access like 'chars[i].name' will be mapped to realchars[i].name, with
//			a chance of detecting a bad index value and logging the line & file where it came from.
//			Because bad idex values have to be directed to some place *within* the array, all chars are loaded '1 up',
//			leaving char[0] free (to take all the damage). And chars[i] is mapped to realchar[i+1]
// history:	init by Duke, 5.10.2000
//

#include "wolfpack.h"
#include "CharWrap.h"

#undef  DBGFILE 
#define DBGFILE "CharWrap.cpp"

cCharWrap::cCharWrap(){}

cCharWrap::~cCharWrap(){}

cChar& cCharWrap::operator[](long inx)
{
	if (inx<0 || inx>=charcount)
	{
		char tmp[222];
		sprintf(tmp,"bad char index %i in file %s line %i",inx, srcfile, srcline);
		LogError(tmp);
		return realchars[0];	// VERY bad for char 0, but it keeps the server alive ;-)
	}
	else
	{
		return realchars[inx+C_W_O_1];	// plus wrapping offset
	}
}

// remember line & file of the access and return an object that the operator overload can act on.
cCharWrap& cCharWrap::prepare(int line, char* fn)
{
	srcline = line;
	srcfile = fn;
	return *this;
}
