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

#if !defined(__PROTOTYPES_H__)
#define __PROTOTYPES_H__


// Wolfpack Includes

// Forward class declarations

class QString;
class cUOSocket;
class Coord_cl;

// Function Declarations
//
void endScrn() ;
void updatehtml();
void offlinehtml();

QString hex2dec( const QString& value );
extern void init_creatures(void);
void dbl_click_item(cUOSocket* socket, SERIAL target_serial);
void showPaperdoll( cUOSocket *socket, P_CHAR pTarget, bool hotkey );
void savelog(const char *msg, char *logfile);
void explodeitem(int s, P_ITEM pi);
void Karma(P_CHAR pc_toChange, P_CHAR pc_Killed, int nKarma);
void Fame(P_CHAR pc_toChange, int nFame);

int check_house_decay();
int ishuman(P_CHAR pc);
void StartClasses();
void SetGlobalVars();
void InitMultis();
void InitServerSettings();
void DeleteClasses();
int chardir(P_CHAR a, P_CHAR b);
int fielddir(P_CHAR pc, int x, int y, int z);

void endmessage(int x);

void goldsfx( cUOSocket *socket, UINT16 amount, bool hearall );

int getamount(P_CHAR pc, short id);
void delequan(P_CHAR pc, short id, int amount, int *not_deleted = NULL);
void dooruse( cUOSocket*, P_ITEM );
void playmonstersound(P_CHAR monster, unsigned short id, int sfx);
void sellaction(int s);
void addgold(cUOSocket* s, int totgold);
int calcValue(P_ITEM pi, int value);
int calcGoodValue(P_CHAR npcnum, P_ITEM pi, int value,int goodtype); // by Magius(CHE) for trade system
void StoreItemRandomValue(P_ITEM pi,QString tmpreg); // by Magius(CHE) (2) for trade system

void clearalltrades();
void trademsg(int s);
void dotrade(P_ITEM cont1, P_ITEM cont2);

bool inVisRange(int x1, int y1, int x2, int y2);
bool online(P_CHAR pc);

int DeleBankItem( P_CHAR pc, unsigned short itemid, unsigned short color, int amt );
void getSextantCords(signed int x, signed int y, bool t2a, char *sextant);
void bgsound(P_CHAR pc);
int hexnumber(int countx);
int makenumber(int countx);
inline int calcserial(unsigned char a1,unsigned char a2,unsigned char a3,unsigned char a4) {return (static_cast<int>((a1<<24))|static_cast<int>((a2<<16)) | static_cast<int>((a3<<8)) | static_cast<int>(a4));}
int lineOfSight( const Coord_cl&, const Coord_cl&, int checkfor );

// Guildstone related functions
int chardirxyz(P_CHAR pc, int x, int y);	// direction from character a to char b
void callguards( P_CHAR pc_player );
bool inrange1p (PC_CHAR pca, P_CHAR pcb);
unsigned char tempeffect(P_CHAR pc_source, P_CHAR pc_dest, int num, unsigned char more1, unsigned char more2, unsigned char more3,short dur=-1);
unsigned char tempeffect2(P_CHAR source, P_ITEM piDest, int num, unsigned char more1, unsigned char more2, unsigned char more3);
void setcharflag(P_CHAR pc);
P_ITEM FindItemBySerial(int serial);
P_CHAR FindCharBySerial(int serial);

#endif

