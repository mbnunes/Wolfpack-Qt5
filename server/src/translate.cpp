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

//////////////////////////////////////////////////////////////////////
// file:	translate.cpp
// purpose:	translates strings to another language
//			used by emus that are based on Wolfpack
// remarks:	this is a VERY simple implementation. The primary goal was
//			to provide a way to avoid translations throughout the sources
//			(allowing for easier merges) and to avoid things like
//			sysmessage(s, MSG47362); // resource-approach
//			Searching the array of strings sequentially will be a bit slow :(
//			but let's see how slow it really is. There are several ways to
//			make it faster ;)
// history:	by Duke, 22.9.01
//
 
#include <string>
#include "platform.h"

#include "oem.h"
#ifdef OEM

struct str_pair		// a pair of two strings
{
	char* english;
	char* foreign;
};

static const struct str_pair trtab[] =	// translation table
{
	{"You aren't skilled enough to make anything with what you have.",
	 "rien ne vas plus"},
	{"You fail your attempt at contacting the netherworld.",
	 NULL},		// NULL for foreign string should give original (english) text
	{"You must hide first.",
	 "O sole mio!"}
};

char* translate(char* txt)
{
	const struct str_pair * pp;

	for (pp=trtab; pp < trtab+(sizeof(trtab)/sizeof(str_pair));pp++)
	{
		if (!strcmp(pp->english,txt))
		{
			if (!pp->foreign)
				break;
			return pp->foreign;
		}
	}
	return txt;	// no translation found 
}
#endif

