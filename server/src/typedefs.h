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


#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

// Forward Base Classes declaration

class cItem;
class cChar;

// Typedefs

//##ModelId=3C5D92B0026B
typedef int					GUILDID;
//##ModelId=3C5D92B00289
typedef unsigned int		TIMERVAL;
//##ModelId=3C5D92B002BB
typedef int					UOXSOCKET;
//##ModelId=3C5D92B002E3
typedef int					PLAYER;
//##ModelId=3C5D92B0030B
typedef int					CHARACTER;
//##ModelId=3C5D92B00333
typedef unsigned char		LIGHTLEVEL;
//##ModelId=3C5D92B0035B
typedef unsigned char		SECONDS;
//##ModelId=3C5D92B0037A
typedef int					ITEM;
//##ModelId=3C5D92B003A2
typedef int					SERIAL;
//##ModelId=3C5D92B003CA
typedef unsigned int		RACE;

//##ModelId=3C5D92B1000A
typedef	cItem *				P_ITEM;
//##ModelId=3C5D92B10032
typedef const cItem *		PC_ITEM;
//##ModelId=3C5D92B1005A
typedef cChar *				P_CHAR;
//##ModelId=3C5D92B10082
typedef const cChar *		PC_CHAR;


#endif

