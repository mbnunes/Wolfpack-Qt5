//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001-2003 by holders identified in authors.txt
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

// Library includes
#include "qstring.h"
#include <vector>
#include <map>

// Forward Base Classes declaration

class cItem;
class cBaseChar;
class cNPC;
class cPlayer;
class cUObject;

// Typedefs

typedef int					GUILDID;
typedef unsigned int		TIMERVAL;
typedef int					UOXSOCKET;
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

struct gumpChoice_st 
{
	signed int button;
	std::vector< unsigned int > switches;
	std::map< unsigned short, QString > textentries;
};

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

/*
Food explanation:

Food items:
  - have type 14 to indicate that they are food..
  - have type2 in enFoodTypes to indicate the type of food..

NPCs:
  - have UI32 food variable that stores a bit flag for each food type.
  - food == 0 would mean no food, food == 0xFFFFFFFF would mean each.
  - hence, we have a maximum of 32 food types for future changes.
*/
enum enFoodTypes
{
	NOFOOD = 0,
	MEAT,			//  all uncooked meat items, also (non-player) body parts
	RAWFISH,		//  all uncooked fish items (fishsteaks, whole fish, big fish, small fish, magic fish)
	EGGS,			//  raw eggs
	CROPS,			//  all vegetable items
	FRUIT,			//  all fruits
	HAY,			//  sheaves of hay, ears of corn
	GRAIN,			//  corn
	COOKEDMEAT,		//  cooked meat
	COOKEDFISH,		//  cooked fish
	PASTRIES,		//  all bakery products
	LEATHER,		//  leather, leather items and hides
	METAL,			//  all metal items
	VEGETABLES,		//	vegetables
	OTHER,			//  all other foods (cheese i.e.)
	enNumberOfFood
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
	enGoToPosition,
	enCombatTarget,
	enNumberOfWanderTypes
};

#endif

