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

#if !defined(__STRUCTS_H__)
#define __STRUCTS_H__

// Platform Specifics
#include "platform.h"

// System Headers
#include <string>
#include <map>


// Wolfpack Headers
#include "defines.h"
#include "typedefs.h"
#include "coord.h"

using namespace std;

//##ModelId=3C5D92BF02F9
struct move_st
{
	//##ModelId=3C5D92BF0335
	int effect[18];
};

//##ModelId=3C5D92BF038F
struct stat_st
{
	//##ModelId=3C5D92BF03AD
	int effect[17];
};

//##ModelId=3C5D92BF03DF
struct sound_st
{
	//##ModelId=3C5D92C00015
	int effect[2];
};

//##ModelId=3C5D92C0005B
struct path_st 
{
	//##ModelId=3C5D92C0007A
	unsigned short x;
	//##ModelId=3C5D92C0008D
	unsigned short y;
};

// REPSYS
//##ModelId=3C5D92C0017E
struct repsys_st
{
	//##ModelId=3C5D92C0019C
	long int		murderdecay;
	//##ModelId=3C5D92C001B0
	unsigned int	maxkills;
	//##ModelId=3C5D92C001BA
	int				crimtime;
};

//##ModelId=3C5D92C00296
struct resource_st
{
	//##ModelId=3C5D92C00319
	unsigned int		logs;
	//##ModelId=3C5D92C003AF
	unsigned long int	logtime;
	//##ModelId=3C5D92C10099
	unsigned long int	lograte;// AntiChrist
	//##ModelId=3C5D92C100AD
	unsigned int		logarea;
	//##ModelId=3C5D92C100CB
	int					logtrigger;// AntiChrist
	//##ModelId=3C5D92C100DF
	int					logstamina;// AntiChrist
	//##ModelId=3C5D92C100F3
	unsigned int		ore;
	//##ModelId=3C5D92C1011B
	unsigned long int	oretime;
	//##ModelId=3C5D92C1012F
	unsigned long int	orerate;// AntiChrist
	//##ModelId=3C5D92C10143
	unsigned int		orearea;
	//##ModelId=3C5D92C10157
	int					miningtrigger;// AntiChrist
	//##ModelId=3C5D92C101CF
	int					miningstamina;// AntiChrist
};

//##ModelId=3C5D92C10248
struct creat_st
{
	//##ModelId=3C5D92C102B6
	int basesound;
	//##ModelId=3C5D92C102CA
	unsigned char soundflag;
	//##ModelId=3C5D92C102D4
	unsigned char who_am_i;
	//##ModelId=3C5D92C102E8
	int icon;
};

#include "start_pack.h"
//##ModelId=3C5D92C103BA
struct versionrecord
{
	//##ModelId=3C5D92C103E3
	SI32 file;
	//##ModelId=3C5D92C20019
	SI32 block;
	//##ModelId=3C5D92C20037
	SI32 filepos;
	//##ModelId=3C5D92C2004B
	SI32 length;
	//##ModelId=3C5D92C20069
	SI32 unknown;
} PACK;
#include "end_pack.h"

#include "start_pack.h"
//##ModelId=3C5D92C200D6
struct staticrecord
{
	//##ModelId=3C5D92C20163
	short int itemid;
	// short int extra; // Unknown yet --Zippy unknown thus not used thus taking up mem.
	//##ModelId=3C5D92C201E5
	unsigned char xoff;
	//##ModelId=3C5D92C20203
	unsigned char yoff;
	//##ModelId=3C5D92C20217
	signed char zoff;
	//##ModelId=3C5D92C2022B
	unsigned char align;	// force word alignment by hand to avoid bus errors - fur
} PACK;
#include "end_pack.h"

// XYZZY
#include "start_pack.h"
//##ModelId=3C5D92C202C1
struct unitile_st
{
	//##ModelId=3C5D92C202E0
	signed char basez;
	//##ModelId=3C5D92C202F3
	unsigned char type; // 0=Terrain, 1=Item
	//##ModelId=3C5D92C20307
	unsigned short int id;
	//##ModelId=3C5D92C2031B
	unsigned char flag1;
	//##ModelId=3C5D92C20325
	unsigned char flag2;
	//##ModelId=3C5D92C20339
	unsigned char flag3;
	//##ModelId=3C5D92C20375
	unsigned char flag4;
	//##ModelId=3C5D92C20389
	signed char height;
	//##ModelId=3C5D92C203C6
	unsigned char weight;
} PACK;
#include "end_pack.h"

#include "start_pack.h"
//##ModelId=3C5D92C30060
struct st_multiidx
{
	//##ModelId=3C5D92C30089
	SI32 start;
	//##ModelId=3C5D92C3009D
	SI32 length;
	//##ModelId=3C5D92C300BB
	SI32 unknown;
} PACK;
#include "end_pack.h"

#include "start_pack.h"
//##ModelId=3C5D92C301D2
struct st_multi
{
	//##ModelId=3C5D92C3025F
	SI32 visible;  // this needs to be first so it is word aligned to avoid bus errors - fur
	//##ModelId=3C5D92C3027C
	short int tile;
	//##ModelId=3C5D92C302A5
	signed short int x;
	//##ModelId=3C5D92C302EB
	signed short int y;
	//##ModelId=3C5D92C30313
	signed char z;
	//##ModelId=3C5D92C30345
	signed char empty;
} PACK;
#include "end_pack.h"

//##ModelId=3C5D92C40057
struct location_st
{
	//##ModelId=3C5D92C40075
	int x1;
	//##ModelId=3C5D92C40089
	int y1;
	//##ModelId=3C5D92C400E3
	int x2;
	//##ModelId=3C5D92C40129
	int y2;
	// char region;
	//##ModelId=3C5D92C4013D
	unsigned char region;
};

//##ModelId=3C5D92C401B6
struct logout_st// Instalog
{
	//##ModelId=3C5D92C401DE
	unsigned int x1;
	//##ModelId=3C5D92C401F2
	unsigned int y1;
	//##ModelId=3C5D92C401FC
	unsigned int x2;
	//##ModelId=3C5D92C40210
	unsigned int y2;
};

//##ModelId=3C5D92C403BE
struct region_st
{
	//##ModelId=3C5D92C403DD
	char name[50];
	//##ModelId=3C5D92C50008
	int midilist;
	//##ModelId=3C5D92C5004F
	unsigned char priv; // 0x01 guarded, 0x02, mark allowed, 0x04, gate allowed, 0x08, recall
	// 0x10 raining, 0x20, snowing, 0x40 magic damage reduced to 0
	//##ModelId=3C5D92C50063
	char guardowner[50];
	//##ModelId=3C5D92C5009F
	unsigned char snowchance;
	//##ModelId=3C5D92C500B3
	unsigned char rainchance;
	//##ModelId=3C5D92C500BD
	int guardnum[10];
	//##ModelId=3C5D92C500EF
	int goodsell[256]; // Magius(CHE)
	//##ModelId=3C5D92C50103
	int goodbuy[256]; // Magius(CHE)
	//##ModelId=3C5D92C50117
	int goodrnd1[256]; // Magius(CHE) (2)
	//##ModelId=3C5D92C5012B
	int goodrnd2[256]; // Magius(CHE) (2)
	//##ModelId=3C5D92C5013F
	int RaceOwner; // Skyfires Race stuff
};

//##ModelId=3C5D92C502B1
struct spawnregion_st// Regionspawns
{
	//##ModelId=3C5D92C502D9
	unsigned char name[512];// Any Name to show up when this region is spawned [512]
	//##ModelId=3C5D92C502EE
	int npclists[512];// NPC lists [512]
	//##ModelId=3C5D92C50302
	int npcs[512];// Individual npcs [512]
	//##ModelId=3C5D92C50316
	int itemlists[512];// item Lists [512]
	//##ModelId=3C5D92C50320
	int totalitemlists;// Total number of item lists
	//##ModelId=3C5D92C50334
	int totalnpcs;// Total Number of indiviual npcs
	//##ModelId=3C5D92C50348
	int totalnpclists;// Number of NPC lists
	//##ModelId=3C5D92C5035C
	int max;// Max amount of spawned characters
	//##ModelId=3C5D92C50366
	int current;// Current amount of spawned chars
	//##ModelId=3C5D92C5037A
	int mintime;// Minimum spawn time
	//##ModelId=3C5D92C5038E
	int maxtime;// Maximum spawn time
	//##ModelId=3C5D92C503A2
	int nexttime;// Nextspawn time for this region
	//##ModelId=3C5D92C503B6
	int x1;// Top left X
	//##ModelId=3C5D92C503C0
	int x2;// Bottom right x
	//##ModelId=3C5D92C503D4
	int y1;// Top left y
	//##ModelId=3C5D92C60000
	int y2;// Bottom right y
};

//##ModelId=3C5D92C600B4
struct skill_st
{
	//##ModelId=3C5D92C600DD
	int st;
	//##ModelId=3C5D92C600F0
	int dx;
	//##ModelId=3C5D92C60104
	int in;
	//##ModelId=3C5D92C6012C
	int advance_index;
	//##ModelId=3C5D92C60140
	char madeword[50]; // Added by Magius(CHE)
};

//##ModelId=3C5D92C6019A
struct advance_st
{
	//##ModelId=3C5D92C601BA
	unsigned char skill;
	//##ModelId=3C5D92C601CD
	int base;
	//##ModelId=3C5D92C601E1
	int success;
	//##ModelId=3C5D92C601F5
	int failure;
};

//##ModelId=3C5D92C6030D
struct make_st
{
	//##ModelId=3C5D92C6032B
	int has;
	//##ModelId=3C5D92C6033F
	int has2;
	//##ModelId=3C5D92C60353
	int needs;
	//##ModelId=3C5D92C60367
	int minskill;
	//##ModelId=3C5D92C60371
	int maxskill;
	//##ModelId=3C5D92C6038F
	unsigned short newcolor; // This color is setted if coloring>-1 // Magius(CHE) §
	//##ModelId=3C5D92C60399
	int coloring; // Color modification activated by trigger token!  // Magius(CHE) §
	//##ModelId=3C5D92C603AD
	short Mat1id;		// id of material 1 used to make item
	//##ModelId=3C5D92C603C1
	short Mat1color;	// color of material 1 used to make item
	//##ModelId=3C5D92C603CB
	short Mat2id;		// id of material 2 used to make item
	//##ModelId=3C5D92C70001
	short Mat2color;	// color of material 2 used to make item
	//##ModelId=3C5D92C70033
	int minrank; // value of minum rank level of the item to create! - Magius(CHE)
	//##ModelId=3C5D92C70065
	int maxrank; // value of maximum rank level of the item to create! - Magius(CHE)
	//##ModelId=3C5D92C7006F
	int number; // Store Script Number used to Rank System by Magius(CHE)
};

//##ModelId=3C5D92C700DE
struct gmpage_st
{
	//##ModelId=3C5D92C7012F
	string reason;
	//##ModelId=3C5D92C70143
	SERIAL serial;
	//##ModelId=3C5D92C70160
	char timeofcall[9];
	//##ModelId=3C5D92C7017F
	string name;
	//##ModelId=3C5D92C70192
	int handled;
};

//##ModelId=3C5D92C701EC
struct jail_st
{
	//##ModelId=3C5D92C7020B
	Coord_cl oldpos;
	//##ModelId=3C5D92C70229
	Coord_cl pos;
	//##ModelId=3C5D92C7023C
	bool occupied;
};

//##ModelId=3C5D92C702A0
struct tracking_st
{
	//##ModelId=3C5D92C702BF
	unsigned int baserange;
	//##ModelId=3C5D92C702DC
	unsigned int maxtargets;
	//##ModelId=3C5D92C702E6
	unsigned int basetimer;
	//##ModelId=3C5D92C702FA
	unsigned int redisplaytime;
};

//##ModelId=3C5D92C7035F
struct begging_st
{
	//##ModelId=3C5D92C7037D
	int timer;
	//##ModelId=3C5D92C70391
	unsigned int range;
	//##ModelId=3C5D92C703A5
	char text[3][256];
};

//##ModelId=3C5D92C800B7
struct fishing_st
{
	//##ModelId=3C5D92C800D6
	unsigned int basetime;
	//##ModelId=3C5D92C800E9
	unsigned int randomtime;
};

//##ModelId=3C5D92C801CF
struct spiritspeak_st
{
	//##ModelId=3C5D92C801EE
	unsigned int spiritspeaktimer;
};

//##ModelId=3C5D92C8028E
struct speed_st // Lag Fix
{
	//##ModelId=3C5D92C802FC
	int nice;
	//##ModelId=3C5D92C8034C
	float itemtime;
	//##ModelId=3C5D92C80388
	unsigned int srtime;
	//##ModelId=3C5D92C8039C
	float npctime;
	//##ModelId=3C5D92C803B0
	float tamednpctime;// AntiChrist
	//##ModelId=3C5D92C803BA
	float npcfollowtime;// Ripper
	//##ModelId=3C5D92C803D8
	float npcaitime;
	//##ModelId=3C5D92C90004
	unsigned int checkmem;
};



//##ModelId=3C5D92C90068
struct title_st // For custom titles
{
	//##ModelId=3C5D92C90086
	char fame[50];
	//##ModelId=3C5D92C9009A
	char skill[50];
	//##ModelId=3C5D92C900AE
	char prowess[50];
	//##ModelId=3C5D92C900CC
	char other[50];
};

//##ModelId=3C5D92C90181
struct reag_st
{
	//##ModelId=3C5D92C9019F
	int ginseng;
	//##ModelId=3C5D92C901B3
	int moss;
	//##ModelId=3C5D92C901C7
	int drake;
	//##ModelId=3C5D92C901D1
	int pearl;
	//##ModelId=3C5D92C901E5
	int silk;
	//##ModelId=3C5D92C901F9
	int ash;
	//##ModelId=3C5D92C9020D
	int shade;
	//##ModelId=3C5D92C90221
	int garlic;
};

// AntiChrist (9/99)
//##ModelId=3C5D92C90307
struct spell_st
{
	//##ModelId=3C5D92C9032F
	int enabled;		// spell enabled?
	//##ModelId=3C5D92C90343
	int circle;		// circle number
	//##ModelId=3C5D92C90375
	int mana;			// mana requirements
	//##ModelId=3C5D92C90389
	int loskill;		// low magery skill req.
	//##ModelId=3C5D92C9039E
	int hiskill;		// high magery skill req.
	//##ModelId=3C5D92C903A8
	int sclo;			// low magery skill req. if using scroll
	//##ModelId=3C5D92C903BC
	int schi;			// high magery skill req. if using scroll
	//##ModelId=3C5D92C903D0
	char mantra[25];	// words of power
	//##ModelId=3C5D92C903E4
	int action;		// character action
	//##ModelId=3C5D92CA0010
	int delay;			// spell delay
	//##ModelId=3C5D92CA0025
	reag_st reagents;	// reagents req.
	//##ModelId=3C5D92CA004C
	char strToSay[100];// string visualized with targ. system
	//##ModelId=3C5D92CA0056
	int reflect;		// 1=spell reflectable, 0=spell not reflectable
	//##ModelId=3C5D92CA0074
	unsigned char runic;
};

//##ModelId=3C5D92CA00C4
struct tele_locations_st 
{
	Coord_cl destination, origem;
};

//##ModelId=3C5D92CA01BE
struct ServerList_st
{
	//##ModelId=3C5D92CA01E7
	UI16 uiIndex;
	//##ModelId=3C5D92CA0205
	string sServer;
	//##ModelId=3C5D92CA0223
	UI08 uiFull;
	//##ModelId=3C5D92CA0237
	UI08 uiTime;
	//##ModelId=3C5D92CA0255
	string sIP;
	//##ModelId=3C5D92CA0274
	UI16 uiPort;
};

#include "start_pack.h"
//##ModelId=3C5D92CA036D
struct tile_st
{
	//##ModelId=3C5D92CA0396
	SI32 unknown1;  // longs must go at top to avoid bus errors - fur
	//##ModelId=3C5D92CA03B4
	SI32 animation;
	//##ModelId=3C5D92CA03C7
	unsigned char flag1;
	//##ModelId=3C5D92CA03DB
	unsigned char flag2;
	//##ModelId=3C5D92CB0007
	unsigned char flag3;
	//##ModelId=3C5D92CB0025
	unsigned char flag4;
	//##ModelId=3C5D92CB0039
	unsigned char weight;
	//##ModelId=3C5D92CB0057
	signed char layer;
	//##ModelId=3C5D92CB006B
	signed char unknown2;
	//##ModelId=3C5D92CB007F
	signed char unknown3;
	//##ModelId=3C5D92CB00BB
	signed char height;
	//##ModelId=3C5D92CB00CF
	signed char name[23];	// manually padded to long to avoid bus errors - fur | There is no negative letter.
} PACK ;
#include "end_pack.h"

//##ModelId=3C5D92CB01A2
struct land_st
{
	//##ModelId=3C5D92CB01C0
	char flag1;
	//##ModelId=3C5D92CB01D4
	char flag2;
	//##ModelId=3C5D92CB01E8
	char flag3;
	//##ModelId=3C5D92CB01FC
	char flag4;
	//##ModelId=3C5D92CB021A
	char unknown1;
	//##ModelId=3C5D92CB0224
	char unknown2;
	//##ModelId=3C5D92CB0238
	char name[20];
};

//##ModelId=3C5D92CB0300
struct map_st
{
	//##ModelId=3C5D92CB031E
	short int id;
	//##ModelId=3C5D92CB0332
	signed char z;
};

#endif

