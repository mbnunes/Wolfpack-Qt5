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

// cCharWrap.h: interface for the cCharWrap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CCHARWRAP_H__121F974E_57F4_4B7D_BD1D_10DE434ED863__INCLUDED_)
#define AFX_CCHARWRAP_H__121F974E_57F4_4B7D_BD1D_10DE434ED863__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class cCharWrap  
{
private:
	int	srcline;
	char *srcfile;
public:
	cCharWrap();
	virtual ~cCharWrap();
	cCharWrap& prepare(int line, char* fn); 
	cChar& operator [](long inx);

};

extern cChar *realchars;
extern cCharWrap cwrap;

#define C_W_O_1	1	// character wrapping offset. 1 or 0. All chars are loaded into the array at index+1, leaving char[0] free !

#if C_W_O_1		// character wrapping active
#define chars (cwrap.prepare(__LINE__, DBGFILE))
#else
#define chars realchars
#endif

#endif // !defined(AFX_CCHARWRAP_H__121F974E_57F4_4B7D_BD1D_10DE434ED863__INCLUDED_)
