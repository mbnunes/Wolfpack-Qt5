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
//	Wolfpack Homepage: http://www.wpdev.sf.net/
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

bool ishouse(int id1, int id2);

//Boats --Fucntions in Boats.cpp
void sendinrange(P_ITEM pi);
int dist(int,int,int);
unsigned int dist(Coord_cl&, Coord_cl&); // Distance between position a and b
P_ITEM findmulti(Coord_cl);
bool inmulti(Coord_cl, P_ITEM);
//End Boat functions

extern void init_creatures(void);

void TellScroll( char *menu_name, int player, long item_param );

void doubleclick(int s);
void dbl_click_character(UOXSOCKET s, SERIAL target_serial);
void singleclick(UOXSOCKET s);

//void walking(int s, int dir, int sequence);
void teleporters(CHARACTER s);
void read_in_teleport(void);
void npcwalk(CHARACTER i, int j, int type);
//void walking2(CHARACTER s);
void all_items(int s);
void savelog(const char *msg, char *logfile);
void explodeitem(int s, P_ITEM pi);
void monstergate(int s, int x);
//void npcMovement2(unsigned int, int);//Lag fix -- Zippy
//void npcMovement(unsigned int);
void Karma(int nCharID,int nKilledID, int nKarma);
void npctalkall_runic(int npc, char *txt,char antispam);
void Fame(int nCharID, int nFame);
void charstartup(int s);
void checkdumpdata(unsigned int currenttime); // This dumps data for Ridcully's UOXBot 0.02 (jluebbe@hannover.aball.de)
void killall(int s, int percent, char* sysmsg);

// functions in necro.cpp
int SpawnRandomMonster(int nCharID, char* cScript, char* cList, char* cNpcID);
int SpawnRandomItem(int nCharID,int nInPack, char* cScript, char* cList, char* cItemID);

// functions in fishing.cpp
int SpawnFishingMonster(int nCharID, char* cScript, char* cList, char* cNpcID);
int SpawnFishingItem(int nCharID,int nInPack, char* cScript, char* cList, char* cItemID);





//int checkweight2(int s);
int check_house_decay();
bool ishouse(P_ITEM);
void cleanup(int s);
P_ITEM packitem(int p);
void titletarget(int s);
int ishuman(int p);
void npcact(int s);
void objTeleporters(CHARACTER s);
void SkillVars();
void StartClasses();
void LoadCustomScripts();
void SetGlobalVars();
void BuildPointerArray();
void InitMultis();
void InitServerSettings();
void DeleteClasses();
void npcToggleCombat(int s);
int chardir(int a, int b);
UOXSOCKET calcSocketFromChar(CHARACTER i);
int calcCharFromSer(int ser1, int ser2, int ser3, int ser4);
int calcCharFromSer(int ser);
int calcSerFromChar(int ser);
int checkBoundingBox(int xPos, int yPos, int fx1, int fy1, int fz1, int fx2, int fy2);
int checkBoundingCircle(int xPos, int yPos, int fx1, int fy1, int fz1, int radius);
int unmounthorse(int s);
void telltime(int s);
void impaction(int s, int act);
int fielddir(int s, int x, int y, int z);
void npcattacktarget(int target2, int target);
void npcsimpleattacktarget(int target2, int target);
int RandomNum(int nLowNum, int nHighNum);
void enlist(int s, int listnum); // For enlisting in army

// Day and Night related prototypes
void doworldlight(void);
char indungeon(int s);
void setabovelight(unsigned char);

void tweakmenu(UOXSOCKET, SERIAL);
int validtelepos(int s);
void showcname (int s, int i, char b);
void addhere(int s, signed char z);
void whomenu(int s, int type);
void playermenu(int s, int type);
void gmmenu(int s, int m);
void scriptcommand (int s,  char *script1,  char *script2);
void endmessage(int x);

void goldsfx(int s, int goldtotal);

void gcollect();

//signed char mapheight(int x, int y);
//void seekland(int landnum, land_st *land);
void weather(int s, unsigned char bolt);
unsigned char npcinrange (int s, int i, int distance);  //check for horse distance...
//void xbanktarget(int s);
void openbank(int s, int i);
void openspecialbank(int s, int i);//AntiChrist
char inbankrange(int i);
int getamount(int s, short id);
void delequan(int s, short id, int amount, int *not_deleted = NULL);
void gettokennum(char * s, int num);
void setrandomname(int s, char * namelist);
void donewithcall(int s, int type);
void initque();
void choice(int s);
void mounthorse(UOXSOCKET s, CHARACTER x);
char *title1(P_CHAR pc);
char *title2(P_CHAR pc);
char *title3(P_CHAR pc);
char *complete_title(CHARACTER p);
void buyaction(int s);
void restock(int s);
void dooruse(int s, P_ITEM pi);
void readFullLine ();

void who(int s);
void gms(int s);
void playmonstersound(int monster, int id1, int id2, int sfx);
void sellaction(int s);
void addgold(int s, int totgold);
int calcValue(P_ITEM pi, int value);
int calcGoodValue(int npcnum, P_ITEM pi, int value,int goodtype); // by Magius(CHE) for trade system
void StoreItemRandomValue(P_ITEM pi,int tmpreg); // by Magius(CHE) (2) for trade system

int tradestart(int s, int i);
void clearalltrades();
void trademsg(int s);
void dotrade(P_ITEM cont1, P_ITEM cont2);
void dosocketmidi(int s);
void wipe(int s);

void loadmetagm();
void xteleport(int s,int x);
void wornitems(UOXSOCKET s, CHARACTER j);
void RefreshItem(P_ITEM pi);
int MenuListGenerator();
void itemmenu(int s, int m);
void npcemoteall(int npc, char *txt,unsigned char antispam);
int findsection (unsigned char *s);
void read1 ();
void read2 ();
bool inVisRange(int x1, int y1, int x2, int y2);
int inrange1(UOXSOCKET a, UOXSOCKET b);
int inrange1p (CHARACTER a, CHARACTER b);
void deathstuff(int i);
bool online(CHARACTER c);
void loadserverscript(void);
void loadremote_admin();
void saveserverscript(void);
void loadserverdefaults(void);
int numbitsset( int number );
int whichbit( int number, int bit );
unsigned int chardist (CHARACTER a, CHARACTER b);
unsigned int itemdist(CHARACTER a, P_ITEM pi);

int GetBankCount( CHARACTER p, unsigned short itemid, unsigned short color = 0x0000 );
int DeleBankItem( CHARACTER p, unsigned short itemid, unsigned short color, int amt );

void getSextantCords(signed int x, signed int y, bool t2a, char *sextant);

void usehairdye(int s, int x);
void buildhouse(int s, int i);
//void buildhouse(int s, int i,char d_id1,char d_id2,char d_id3, char d_id4);
void deedhouse(UOXSOCKET s, int i); //crackerjack 8/9/99
void killkeys(SERIAL serial); // crackerjack 8/11/99
// house list functions - cj 8/12/99
int add_hlist(int c, int h, int t);
int del_hlist(int c, int h);
//
void house_speech(int s, char *talk);
void mtarget(int s, int a1, int a2, int a3, int a4, char b1, char b2, char *txt);

int addrandomcolor(int s, char *colorlist);


void lockpick(int s);


int getstatskillvalue(char *stringguy);
// for newbie stuff
int bestskill(CHARACTER p);
int nextbestskill(CHARACTER m, int bstskll);
void newbieitems(CHARACTER c);
void read3 ();
void read4 ();
void readscript ();

//For custom titles
void loadcustomtitle();
void loadskills();
void loadmenuprivs();


void advancementobjects(CHARACTER s, int x, int always);
void itemsfx(UOXSOCKET s, short item);
void bgsound(int s);
void splitline();
int hexnumber(int countx);
int makenumber(int countx);

// Dupois - fileArchive() prototypes
// Added Oct 20, 1998
void fileArchive(char *pFile2Archive_chars, char *pFile2Archive_items, char *pArchiveDir);
void ArchiveID(char archiveid[MAXARCHID]);
// End - Dupois

//Trigger routines
void triggerwitem(UOXSOCKET ts, P_ITEM pi, int ttype);  // trigger.cpp
void triggernpc(UOXSOCKET ts,int ti, int ttype);  // trigger.cpp --- Changed by Magius(CHE) §
int checkenvoke(char eid1, char eid2);  //trigger.scp

inline int calcserial(unsigned char a1,unsigned char a2,unsigned char a3,unsigned char a4) {return (static_cast<int>((a1<<24))|static_cast<int>((a2<<16)) | static_cast<int>((a3<<8)) | static_cast<int>(a4));}

// Pointer.cpp functions
// - set item in pointer array
void setptr(lookuptr_st *ptr, int item);
void removefromptr(lookuptr_st *ptr, int nItem);// - remove item from pointer array
int findbyserial(lookuptr_st *ptr, int nSerial, int nType);// - find item in a pointer array

//void possess(int s); //Not done
int line_of_sight(int s, Coord_cl&, Coord_cl&, int checkfor);

// Dupois message board prototype
// Proto for handling the different message type for message 0x71
void    MsgBoardEvent(int nSerial);

void doGmMoveEff(UOXSOCKET s); //gm movement effects
// Guildstone related functions
int chardirxyz(int a, int x, int y);	// direction from character a to char b


void batchcheck(int s);
void readw2();
void readw3();


//void selectspell2cast(int s, int num); //Socket, Spell Number
void criminal(int c);
void callguards( int p );
int recursestatcap(int chr); //Morrolan - stat/skill cap
void skillfreq(int chr, int skill); //Morrolan - stat/skill cap

#endif

