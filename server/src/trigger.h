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

#if !defined(__TRIGGER_H__)
#define __TRIGGER_H__

//Platform specifics

#include "platform.h"

//system includes
#include <errno.h>

//Forward class
class cTrigger;


//Wolfpack includes

#include "wolfpack.h"
#include "SndPkg.h"
#include "debug.h"

class cTrigger
{
public:
	void triggerwitem(UOXSOCKET const ts, P_ITEM pi, int ttype);
    void triggernpc(UOXSOCKET ts, int ti, int ttype);
	int  checkenvoke(char eid1, char eid2);
};

extern cTrigger	 *Trig;



#endif

