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

// ItemWrap.h: interface for the ItemWrap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ITEMWRAP_H__86003DCB_E86A_4006_ACA9_4A22D455C661__INCLUDED_)
#define AFX_ITEMWRAP_H__86003DCB_E86A_4006_ACA9_4A22D455C661__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class cItemWrap  
{
private:
	int	srcline;
	char *srcfile;
public:
	cItemWrap();
	virtual ~cItemWrap();
	cItemWrap& prepare(int line, char* fn); 
	cItem& operator [](long inx);

};

extern cItem *realitems;
extern cItemWrap iwrap;

#define I_W_O_1	1	// item wrapping offset. 1 or 0. All items are loaded into the array at index+1, leaving items[0] free !

#if I_W_O_1		// item wrapping active
#define items (iwrap.prepare(__LINE__, DBGFILE))
#else
#define items realitems
#endif

#endif // !defined(AFX_ITEMWRAP_H__86003DCB_E86A_4006_ACA9_4A22D455C661__INCLUDED_)
