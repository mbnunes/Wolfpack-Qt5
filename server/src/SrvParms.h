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

#ifndef __Srv_Params
#define __Srv_Params

void loadserverdefaults(void);
void loadserverscript(void);
void loadclientsallowed(void);


void saveserverscript(void);
#define SrvParms (&server_data)	// in preparation of making sever_data a class.Use SrvParms->xxx instead of server_dat.xxx (Duke)


#define STABLOCKCACHESIZE 1000
#define NPCSPEED .3	// was .2  5 steps every second

#define ECLIPSETIMER .5

#define DECAYTIMER 300  // 5 minute decay rate

struct server_st
{
	unsigned int eclipsemode;
	unsigned int eclipsetimer;
	char archivepath[256];
	char specialbanktrigger[50]; //special bank trigger - AntiChrist
	int usespecialbank; //special bank - AntiChrist
	char msgboardpath[256];   // Dupois - Added April 4, 1999 for msgboard.cpp - path to store BB* files (ie C:\POSTS\)
	int  msgpostaccess;       // Dupois - Added April 4, 1999 for msgboard.cpp - who gets to post messages (0=GM only, 1 = everyone)
	int  msgpostremove;       // Dupois - Added April 4, 1999 for msgboard.cpp - who gets to remove post messages (0=GM only, 1 = everyone)
	int  msgretention;        // Dupois - Added April 4, 1999 for msgboard.cpp - how long to keep posts before deleting them automatically (in days - default 30)
};


#endif
