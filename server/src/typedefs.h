//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2004 by holders identified in authors.txt
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
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================


#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

// Forward Base Classes declaration

class cItem;
class cBaseChar;
class cNPC;
class cPlayer;
class cUObject;
class cUOSocket;
class Coord_cl;
class cParty;
class cUOPacket;

// Typedefs

typedef int					GUILDID;
typedef unsigned int		TIMERVAL;
typedef int					PLAYER;
typedef int					CHARACTER;
typedef unsigned char		LIGHTLEVEL;
typedef unsigned char		SECONDS;
typedef int					ITEM;
typedef int					SERIAL;
typedef	cItem *				P_ITEM;
typedef const cItem *		PC_ITEM;
typedef cBaseChar *			P_CHAR;
typedef const cBaseChar *	PC_CHAR;
typedef cNPC *				P_NPC;
typedef const cNPC *		PC_NPC;
typedef cPlayer *			P_PLAYER;
typedef const cPlayer *		PC_PLAYER;
typedef cUObject *			P_OBJECT;


enum WPPAGE_TYPE
{
	PT_GM = 0,
	PT_COUNSELOR
};

enum enBodyParts
{
	ALLBODYPARTS = 0,
	LEGS,
	BODY,
	ARMS,
	HANDS,
	NECK,
	HEAD,
	DEADLY
};

enum enCharTypes
{
	enNPC = 0,		// Non Player Characters (cNPC)
	enPlayer,		// Player Characters (cPlayer)
	enNumberOfCharTypes
};

enum enWanderTypes
{
	enHalt = 0,
	enFreely,
	enRectangle,
	enCircle,
	enFollowTarget,
	enDestination,
	enNumberOfWanderTypes
};

enum eDamageType
{
	DAMAGE_PHYSICAL = 0,
	DAMAGE_MAGICAL = 1,
	DAMAGE_GODLY = 2,
	DAMAGE_HUNGER = 3
};

enum enServerState { STARTUP = 0, RUNNING, SCRIPTRELOAD, SHUTDOWN };

#endif

