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

#if !defined(__DEFINES_H__)
#define __DEFINES_H__

#define MAXLOOPS 66666
#define MaxZstep 5

#if defined(__borland__)
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif


#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#endif
#if defined(__unix__)
typedef unsigned char BYTE ;
#define SOCKET_ERROR -1
//#ifndef max
//#define max(a,b)            (((a) > (b)) ? (a) : (b))
//#endif


//#ifndef min
//#define min(a,b)            (((a) < (b)) ? (a) : (b))
//#endif

extern "C" {
char *strlwr(char *);
char *strupr(char *);
};
extern time_t  oldtime, newtime;
void Writeslot(char *lpszMessage);
#define SLPMULTI 1000
#define Sleep(sec) usleep(sec * 1000)

#else
#include <winsock2.h>
extern WSADATA wsaData;
extern WORD wVersionRequested;
extern long int oldtime, newtime;
void Writeslot(LPSTR lpszMessage);

#endif
//#ifndef MSG_NOSIGNAL
//#define MSG_NOSIGNAL 0
//#endif

#define MAX_GUILDTYPE 2	// Code to support the Chaos/order system
#define MAXIMUM 128 // Maximum open connections to server
#define MAXCLIENT MAXIMUM+1
#define MAXSERV 5 // Maximum servers in login listing
#define MAXSTART 15 // Maximum starting locations

////////////////////////////////////////////////////////////////
/// DO NOT TOUCH THIS VALUES !!!!!!!
/// Unless you are a SERVER AND(!!!) CLIENT networking guru
////////////////////////////////////////////////////////////////

#define MAXBUFFER_ASYNCH 2560   // Maximum# of bytes for old asynchronous UOX3 packetsend buffer
                                // If buffer exceeded this max they are send in bursts of this sizes.
                                // *NO* correlation to packets. 
                                // has to be < MAXBUFFER_REAL
                                // only used for PacketSendStyle UOX3 in server.scp
                                // 2560 is traditional UOX value, unchanged for years ...

#define MAXBUFFER_REAL   17000  // that's the real buffersize correlating to the client packet buffer
                                // it's 64kb, but you have to consider compression !!! >32 isn't safe.
                                // didn't use 32kb because too much memory (4MB versus 2MB of 17kb)
                                // and I've never seen any packet >10kb on OSI servers.

#define MAXBUFFER_SYNCH  512    // Used for synchronous packetSendStyle SMART.
                                // If buffer exceeded this max all FULL packets of the buffer are send.
                                // keep this small (<1024, >64)

#define MAXPACKETS_SYNCH 10     // Used for synchronous packetSendStyle SMART
                                // if packet sin buffer exceed this max all FULL packets of the buffer are send
                                // usefiull values: 5-25
 
                                // remark: synchronous packetSendStyle OSI doesn't use those both.

//////////////////////////////////

#define PSS_UOX3      0x0
#define PSS_OSI       0x1
#define PSS_SMARTWOLF 0x2

#define MAXPAGES 75 // Maximum number of pages in the GM queue
#define VISRANGE 18 // Visibility for normal items
#define BUILDRANGE 31 // Visibility for castles and keeps
#define MAP0CACHE 300
#define PATHNUM 2


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
#define ENTICEMENT 15
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
#define REMOVETRAPS 48  // lb, have to be extacly the numbers the client sends !!


#define ALLSKILLS 49 // #skills+1

#define STR 50 // #skills+2
#define DEX 51 // #skills+3
#define INT 52 // #skills+4
#define FAME 53 // #skills+5
#define KARMA 54 // #skills+6

#define SKILLS 55 //#skills+7

#define TRUESKILLS 49 //#skills+1

#define DOORTYPES 17
#define WEIGHT_PER_STR 4
#define MAXGATES 100
#define POLYMORPHMENUOFFSET 8196 // polymorph spell menu offset - AntiChrist (9/99)
#define MAKEMENUOFFSET 2048
#define TRACKINGMENUOFFSET 4096 // menu offset

#define NORTH 1
#define NORTHEAST 2
#define EAST 3
#define SOUTHEAST 4
#define SOUTH 5
#define SOUTHWEST 6
#define WEST 7
#define NORTHWEST 8

// for spinning wheel -vagrant
#define YARN 0
#define THREAD 1

// Dupois - for fileArchive()
// Added Oct 20, 1998
#define MAXARCHID 14

// Line Of Sight
#define ITEM_TYPE_CHOICES 6
#define TREES_BUSHES 1 // Trees and other large vegetaion in the way
#define WALLS_CHIMNEYS 2  // Walls, chimineys, ovens, etc... in the way
#define DOORS 4 // Doors in the way
#define ROOFING_SLANTED 8  // So can't tele onto slanted roofs, basically
#define FLOORS_FLAT_ROOFING 16  //  For attacking between floors
#define LAVA_WATER 32  // Don't know what all to use this for yet

// House List types
#define H_FRIEND	1	// friend of the house
#define H_BAN		2	// banned from house

///////////////////////  End Defines


//#define HASHMAX 149111 // lord binary, hashmax must be a prime for maximum performce (see Knuth, Sedgewick et al).
// 149111 is far too high for small systems :( keep in mind how much additional memory will be allocated
// for each number: nearly 1k! So with 149111 we have 140 MB of memory for the hashtable (Duke, 17.9.01)
#define HASHMAX 4973 // lord binary, hashmax must be a prime for maximum performce (see Knuth, Sedgewick et al).

// Comment about HASHMAX:
// The size of the hash array is very important. In order to get good performance, you should 
// use a suitably large prime number. Suitable means equal to or larger than the maximum expected 
// number of items/chars. 





// DasRaetsels' stuff down, don't touch ;)

#define DEFAULTWEBPAGE "http://www.wpdev.com/"  //this URL doesn't exist, changed to www.wpdev.com ;)

#define MY_CLOCKS_PER_SEC 1000
#define INVALID_SERIAL -1
#define INVALID_UOXSOCKET -1

#endif
///////////////////////  End Defines

