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

// dragdrop.h: interface for the dragdrop class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRAGDROP_H__AB350791_EF80_4406_ADE8_FF309B0849AD__INCLUDED_)
#define AFX_DRAGDROP_H__AB350791_EF80_4406_ADE8_FF309B0849AD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
class cDragdrop;

class cDragdrop
{
public:
	void get_item(P_CLIENT ps);			// Client grabs an item
    void wear_item(P_CLIENT ps);			// Item is dropped on paperdoll
    void drop_item(P_CLIENT ps);	// Item is dropped on ground, char or item
};

extern cDragdrop    *Drag;

#endif // !defined(AFX_DRAGDROP_H__AB350791_EF80_4406_ADE8_FF309B0849AD__INCLUDED_)
