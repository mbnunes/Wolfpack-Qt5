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

#ifndef __WOLFPACK_H
#define __WOLFPACK_H
// Platform specifics

#include "platform.h"

//system includes

#include <iostream>

#if defined (__unix__)
#include <termios.h>
#include <fcntl.h>
#else

//#include <winsock2.h>
//#include <winbase.h>
#include <dos.h>
#include <io.h>
#include <climits>
#include <conio.h>
#include <vector>

#endif

using namespace std;

// Forward class declarations


// Wolfpack includes



#include "wpconsole.h"
#include "netsys.h"
#include "typedefs.h"
#include "verinfo.h"
#include "scriptc.h"
//#include "worldmain.h"
#include "boats.h"    // Boats
#include "msgboard.h" // Bulletin Boards
#include "hCache.h"   // MUL Cache stuff.
//#include "hFile.h"  // New File IO stuff //Currently unused
#include "SrvParms.h" // Bounty System
#include "accounts.h" // Account System.
#include "cweather.h" // Weather System
#include "house.h"    // Housing System
#include "races.h"    // Race System
#include "defines.h"

#include "structs.h"

#include "globals.h"
#include "itemsmgr.h"
#include "charsmgr.h"

#include "prototypes.h"

#include "junk.h"

#include "inlines.h"
#endif // __WOLFPACK_H

