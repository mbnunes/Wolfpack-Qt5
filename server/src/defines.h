/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2006 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In addition to that license, if you are running this program or modified
 * versions of it on a public system you HAVE TO make the complete source of
 * the version used by you available or provide people with a location to
 * download it.
 *
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#if !defined(__DEFINES_H__)
#define __DEFINES_H__

//o---------------------------------------------------------------------------o
// FIXMEs / TODOs / NOTE macros
//o---------------------------------------------------------------------------o
#define _QUOTE(x) # x
#define QUOTE(x) _QUOTE(x)
#define __FILE__LINE__ __FILE__ "(" QUOTE(__LINE__) ") : "
#define todo( x )  message( __FILE__LINE__" TODO :   " #x "\n" )
#define fixme( x )  message( __FILE__LINE__" FIXME:   " #x "\n" )
#define note( x )  message( __FILE__LINE__" NOTE :   " #x "\n" )

#define Reminder  __FILE__LINE__ ":Remind: "

#define MaxZstep 5

#define ILLEGAL_Z	127

#define VISRANGE 18 // Visibility for normal items
#define BUILDRANGE 21 // Visibility for castles and keeps

// List of skill numbers (For later implementation)
enum eSkills
{
	ALCHEMY				= 0,
	ANATOMY,
	ANIMALLORE,
	ITEMID,
	ARMSLORE,
	PARRYING,
	BEGGING,
	BLACKSMITHING,
	BOWCRAFT,
	PEACEMAKING,
	CAMPING,
	CARPENTRY,
	CARTOGRAPHY,
	COOKING,
	DETECTINGHIDDEN,
	DISCORDANCE,
	EVALUATINGINTEL,
	HEALING,
	FISHING,
	FORENSICS,
	HERDING,
	HIDING,
	PROVOCATION,
	INSCRIPTION,
	LOCKPICKING,
	MAGERY,
	MAGICRESISTANCE,
	TACTICS,
	SNOOPING,
	MUSICIANSHIP,
	POISONING,
	ARCHERY,
	SPIRITSPEAK,
	STEALING,
	TAILORING,
	TAMING,
	TASTEID,
	TINKERING,
	TRACKING,
	VETERINARY,
	SWORDSMANSHIP,
	MACEFIGHTING,
	FENCING,
	WRESTLING,
	LUMBERJACKING,
	MINING,
	MEDITATION,
	STEALTH,
	REMOVETRAPS,
	NECROMANCY,
	FOCUS,
	CHIVALRY,
	BUSHIDO,
	NINJITSU,
	SPELLWEAVING,
	ALLSKILLS
};

#define DEFAULTWEBPAGE "http://www.wpdev.org/"

#define MY_CLOCKS_PER_SEC 1000
#define INVALID_SERIAL 0

#define ARCHERY_RANGE 10

#endif
///////////////////////  End Defines
