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

#if !defined(__PROTOTYPES_H__)
#define __PROTOTYPES_H__


// Wolfpack Includes

#include "coord.h"

// Function Declarations
//
void endScrn() ;
void updatehtml();
void offlinehtml();

/*
//Boats --Fucntions in Boats.cpp
int dist(int,int,int);
unsigned int dist(Coord_cl&, Coord_cl&); // Distance between position a and b
//End Boat functions
*/
void sendinrange(P_ITEM pi);
P_ITEM findmulti(Coord_cl);
bool inmulti(Coord_cl, P_ITEM);

extern void init_creatures(void);

void TellScroll( char *menu_name, int player, long item_param );

void doubleclick(int s);
void dbl_click_character(UOXSOCKET s, SERIAL target_serial);
void singleclick(UOXSOCKET s);

//void walking(int s, int dir, int sequence);
void teleporters(P_CHAR pc_s);
void read_in_teleport(void);
void npcwalk(CHARACTER i, int j, int type);
//void walking2(CHARACTER s);
void all_items(int s);
void savelog(const char *msg, char *logfile);
void explodeitem(int s, P_ITEM pi);
void monstergate(P_CHAR pc_s, int x);
//void npcMovement2(unsigned int, int);//Lag fix -- Zippy
//void npcMovement(unsigned int);
void Karma(P_CHAR pc_toChange, P_CHAR pc_Killed, int nKarma);
void npctalkall_runic(P_CHAR npc, const char *txt,char antispam);
void Fame(P_CHAR pc_toChange, int nFame);
void charstartup(int s);
void checkdumpdata(unsigned int currenttime); // This dumps data for Ridcully's UOXBot 0.02 (jluebbe@hannover.aball.de)
void killall(int s, int percent, char* sysmsg);

// functions in fishing.cpp
int SpawnFishingMonster(int nCharID, char* cScript, char* cList, char* cNpcID);
int SpawnFishingItem(int nCharID,int nInPack, char* cScript, char* cList, char* cItemID);





//int checkweight2(int s);
int check_house_decay();
void cleanup(int s);
void titletarget(int s);
int ishuman(P_CHAR pc);
void npcact(int s);
//void objTeleporters(P_CHAR pc_s);
void SkillVars();
void StartClasses();
void LoadCustomScripts();
void SetGlobalVars();
void BuildPointerArray();
void InitMultis();
void InitServerSettings();
void DeleteClasses();
void npcToggleCombat(P_CHAR pc);
int chardir(P_CHAR a, P_CHAR b);
int unmounthorse(UOXSOCKET s);
void telltime(int s);
void impaction(int s, int act);
int fielddir(P_CHAR pc, int x, int y, int z);
void npcattacktarget(P_CHAR pc_target2, P_CHAR pc_target);
void npcsimpleattacktarget(P_CHAR pc_target2, P_CHAR pc_target);
int RandomNum(int nLowNum, int nHighNum);
void enlist(UOXSOCKET s, UI32 listnum); // For enlisting in army

// Day and Night related prototypes
void doworldlight(void);
char indungeon(P_CHAR pc);
void setabovelight(unsigned char);

void tweakmenu(UOXSOCKET, SERIAL);
int validtelepos(int s);
void showcname (UOXSOCKET s, P_CHAR pc_i, char b);
void addhere(int s, signed char z);
void whomenu(int s, int type);
void playermenu(int s, int type);
void scriptcommand (int s,  char *script1,  char *script2);
void endmessage(int x);

void goldsfx(int s, int goldtotal);

void gcollect();

//signed char mapheight(int x, int y);
//void seekland(int landnum, land_st *land);
void weather(int s, unsigned char bolt);
unsigned char npcinrange (UOXSOCKET s, P_CHAR i, int distance);  //check for horse distance...
int getamount(P_CHAR pc, short id);
void delequan(P_CHAR pc, short id, int amount, int *not_deleted = NULL);
void gettokennum(char * s, int num);
void donewithcall(int s, int type);
void initque();
void MenuChoice( UOXSOCKET Socket );
void mounthorse(UOXSOCKET s, P_CHAR pc_mount);
char *title1(P_CHAR pc);
char *title2(P_CHAR pc);
char *title3(P_CHAR pc);
char *complete_title(P_CHAR pc);
void buyaction(int s);
void restock(int s);
void dooruse(int s, P_ITEM pi);
void readFullLine ();

void who(int s);
void gms(int s);
void playmonstersound(P_CHAR monster, unsigned short id, int sfx);
void sellaction(int s);
void addgold(UOXSOCKET s, int totgold);
int calcValue(P_ITEM pi, int value);
int calcGoodValue(P_CHAR npcnum, P_ITEM pi, int value,int goodtype); // by Magius(CHE) for trade system
void StoreItemRandomValue(P_ITEM pi,QString tmpreg); // by Magius(CHE) (2) for trade system

void clearalltrades();
void trademsg(int s);
void dotrade(P_ITEM cont1, P_ITEM cont2);
void dosocketmidi(int s);
void wipe(int s);

void xteleport(int s,int x);
void wornitems(UOXSOCKET s, P_CHAR pc);
void RefreshItem(P_ITEM pi);
int MenuListGenerator();
void ShowMenu( UOXSOCKET s, UI16 Menu );
void npcemoteall(P_CHAR npc, char *txt,unsigned char antispam);
int findsection (unsigned char *s);
void read1 ();
void read2 ();
bool inVisRange(int x1, int y1, int x2, int y2);
int inrange1(UOXSOCKET a, UOXSOCKET b);
void deathstuff(P_CHAR pc_player);
bool online(P_CHAR pc);
void loadremote_admin();
int numbitsset( int number );
int whichbit( int number, int bit );
unsigned int chardist (P_CHAR a, P_CHAR b);
unsigned int itemdist(P_CHAR pc, P_ITEM pi);

int GetBankCount( P_CHAR pc, unsigned short itemid, unsigned short color = 0x0000 );
int DeleBankItem( P_CHAR pc, unsigned short itemid, unsigned short color, int amt );

void getSextantCords(signed int x, signed int y, bool t2a, char *sextant);

void usehairdye(int s, P_ITEM piDye);
//
void house_speech(int s, string& talk);
void mtarget(int s, int a1, int a2, int a3, int a4, char b1, char b2, char *txt);

void lockpick(int s);


short getstatskillvalue(char *stringguy);
// for newbie stuff
int bestskill(P_CHAR pc);
int nextbestskill(P_CHAR pc, int bstskll);
void newbieitems(UOXSOCKET s, P_CHAR pc);
void read3 ();
void read4 ();
void readscript ();

//For custom titles
void loadcustomtitle();
void loadskills();

//void advancementobjects(P_CHAR pc_s, int x, int always);
void itemsfx(UOXSOCKET s, short item);
void bgsound(P_CHAR pc);
void splitline();
int hexnumber(int countx);
int makenumber(int countx);

// Dupois - fileArchive() prototypes
// Added Oct 20, 1998
void fileArchive(char *pFile2Archive_chars, char *pFile2Archive_items, char *pArchiveDir);
// End - Dupois

//Trigger routines
void triggerwitem(UOXSOCKET ts, P_ITEM pi, int ttype);  // trigger.cpp
int checkenvoke(char eid1, char eid2);  //trigger.scp

inline int calcserial(unsigned char a1,unsigned char a2,unsigned char a3,unsigned char a4) {return (static_cast<int>((a1<<24))|static_cast<int>((a2<<16)) | static_cast<int>((a3<<8)) | static_cast<int>(a4));}

//void possess(int s); //Not done
int line_of_sight(int s, const Coord_cl&, const Coord_cl&, int checkfor);

// Dupois message board prototype
// Proto for handling the different message type for message 0x71
void    MsgBoardEvent(int nSerial);

void doGmMoveEff(UOXSOCKET s); //gm movement effects
// Guildstone related functions
int chardirxyz(P_CHAR pc, int x, int y);	// direction from character a to char b


void batchcheck(int s);
void readw2();
void readw3();


//void selectspell2cast(int s, int num); //Socket, Spell Number
void criminal(P_CHAR pc);
void callguards( P_CHAR pc_player );
int recursestatcap(int chr); //Morrolan - stat/skill cap
void skillfreq(int chr, int skill); //Morrolan - stat/skill cap

#endif

