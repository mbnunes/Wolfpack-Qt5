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

#if !defined(__DEFINES_H__)
#define __DEFINES_H__

#define MAXLOOPS 250000
#define MaxZstep 5

#if defined(__unix__)
#define BYTE unsigned char
#define SOCKET_ERROR -1

extern "C" {
char *strlwr(char *);
char *strupr(char *);
};
#include <sys/types.h>
extern time_t  oldtime, newtime;
//#define Sleep(sec) usleep(sec * 1000)

#else
extern long int oldtime, newtime;

#endif
//#ifndef MSG_NOSIGNAL
//#define MSG_NOSIGNAL 0
//#endif

#define MAX_GUILDTYPE 2	// Code to support the Chaos/order system
#define MAXSERV 5 // Maximum servers in login listing
//#define MAXSTART 15 // Maximum starting locations

#define MAXPAGES 75 // Maximum number of pages in the GM queue
#define VISRANGE 18 // Visibility for normal items
#define BUILDRANGE 31 // Visibility for castles and keeps

#define XYMAX 256 // Maximum items Wolfpack can handle on one X/Y square
//#define MAXEFFECTS MAXCHARS/10
#define MAXLAYERS 50 // Maximum number of Layers in paperdolls (still not sure how many)
#define ITEMMENUOFFSET 256
#define CMAX 40 // Maximum parameters in one line (Only for memory reasons)
#define VERFILE_MAP 0x00
#define VERFILE_STAIDX 0x01
#define VERFILE_STATICS 0x02
#define VERFILE_ARTIDX 0x03
#define VERFILE_ART 0x04
#define VERFILE_ANIMIDX 0x05
#define VERFILE_ANIM 0x06
#define VERFILE_SOUNDIDX 0x07
#define VERFILE_SOUND 0x08
#define VERFILE_TEXIDX 0x09
#define VERFILE_TEXMAPS 0x0A
#define VERFILE_GUMPIDX 0x0B
#define VERFILE_GUMPART 0x0C
#define VERFILE_MULTIIDX 0x0D
#define VERFILE_MULTI 0x0E
#define VERFILE_SKILLSIDX 0x0F
#define VERFILE_SKILLS 0x10
#define VERFILE_TILEDATA 0x1E
#define VERFILE_ANIMDATA 0x1F
#define TILEDATA_TILES 0x68800
#define SPIRITSPEAKTIMER 30 // spiritspeak lasts at least 30 seconds, additional time is calculated by Skill & INT
//////////////////


// List of monster sounds
#define SND_STARTATTACK 0
#define SND_IDLE 1
#define SND_ATTACK 2
#define SND_DEFEND 3
#define SND_DIE 4

// List of skill numbers (For later implementation)
#define ALCHEMY 0
#define ANATOMY 1
#define ANIMALLORE 2
#define ITEMID 3
#define ARMSLORE 4
#define PARRYING 5
#define BEGGING 6
#define BLACKSMITHING 7
#define BOWCRAFT 8
#define PEACEMAKING 9
#define CAMPING 10
#define CARPENTRY 11
#define CARTOGRAPHY 12
#define COOKING 13
#define DETECTINGHIDDEN 14
#define DISCORDANCE 15
#define EVALUATINGINTEL 16
#define HEALING 17
#define FISHING 18
#define FORENSICS 19
#define HERDING 20
#define HIDING 21
#define PROVOCATION 22
#define INSCRIPTION 23
#define LOCKPICKING 24
#define MAGERY 25
#define MAGICRESISTANCE 26
#define TACTICS 27
#define SNOOPING 28
#define MUSICIANSHIP 29
#define POISONING 30
#define ARCHERY 31
#define SPIRITSPEAK 32
#define STEALING 33
#define TAILORING 34
#define TAMING 35
#define TASTEID 36
#define TINKERING 37
#define TRACKING 38
#define VETERINARY 39
#define SWORDSMANSHIP 40
#define MACEFIGHTING 41
#define FENCING 42
#define WRESTLING 43
#define LUMBERJACKING 44
#define MINING 45

#define MEDITATION 46
#define STEALTH 47
#define REMOVETRAPS 48

#define NECROMANCY 49
#define FOCUS 50
#define CHIVALRY 51

#define ALLSKILLS 52 // # skills + 1

#define DOORTYPES 17
#define WEIGHT_PER_STR 4
#define MAXGATES 100

//Stats
#define MANA 1
#define STAMINA 2
#define HITPOINTS 3
#define DEXTERITY 4
#define INTELLIGENCE 5
#define STRENGTH 6


//Requirements
#define MANACOST 1
#define REAGENTCONST 2
#define GLB_REQUIREMENT 3

//Damage by
#define SPELL 1
#define COLD 2
#define DISPEL 3
#define ENERGY 4
#define FIRE 5
#define HITCHANCE 6
#define LOWERATTACK 7
#define LOWERDEFENCE 8	   
// ... here we need to add every spell also

// Enhancements
#define GOLD 1             // Gold increase
#define SWINGSPEED 2	   // Swing speed increase
#define POTIONS 3		   // Enhance potions
#define SELFREPAIR 4	   // Self repair
#define DEFENCECHANCE 5	   // Defence chance
#define CASTSPEED 6		   // Faster casting
#define CASTRECOVERY 7	   // Faster cast recovery

#define REGEN 1		//Regenerations
#define BONUS 2		//Stats bonuses
#define DAMAGE 3	//Damage ( cold, fire, energy etc )
#define ENH 4		//Enhancements, for example: Gold increase
#define HIT 5		//Attack and defence modifiers
#define REQ 6		//Smaller requirements
#define RESIST 7	//Resists to various types of damage
#define REFLECT 8	//Reflecting damage
 
enum enDirection
{
	North = 1, NorthEast, East, SouthEast, South, SouthWest, West, NorthWest
};

// for spinning wheel -vagrant
#define YARN 0
#define THREAD 1

// Line Of Sight
#define ITEM_TYPE_CHOICES 6
#define TREES_BUSHES 1 // Trees and other large vegetaion in the way
#define WALLS_CHIMNEYS 2  // Walls, chimineys, ovens, etc... in the way
#define DOORS 4 // Doors in the way
#define ROOFING_SLANTED 8  // So can't tele onto slanted roofs, basically
#define FLOORS_FLAT_ROOFING 16  //  For attacking between floors
#define LAVA_WATER 32  // Don't know what all to use this for yet

///////////////////////  End Defines


#define DEFAULTWEBPAGE "http://www.wpdev.org/"  //this URL doesn't exist, changed to www.wpdev.org ;)

#define MAXTRACKINGTARGETS 20   // maximum number of trackable targets

#define MY_CLOCKS_PER_SEC 1000
#define INVALID_SERIAL -1
#define INVALID_UOXSOCKET -1

#define CONN_MAIN 1
#define CONN_SECOND 2

// Maximum Search Steps: Iterations to calculate
#endif
///////////////////////  End Defines

