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

QString hex2dec( const QString& value );
void showPaperdoll( cUOSocket *socket, P_CHAR pTarget, bool hotkey );
void savelog(const char *msg, char *logfile);
void Karma(P_CHAR pc_toChange, P_CHAR pc_Killed, int nKarma);
void Fame(P_CHAR pc_toChange, int nFame);

int fielddir(P_CHAR pc, int x, int y, int z);

void endmessage(int x);

void goldsfx( cUOSocket *socket, UINT16 amount, bool hearall );

int getamount(P_CHAR pc, short id);
void delequan(P_CHAR pc, short id, int amount, int *not_deleted = NULL);
void playmonstersound(P_CHAR monster, unsigned short id, int sfx);

bool inVisRange(int x1, int y1, int x2, int y2);

int DeleBankItem( P_PLAYER pc, unsigned short itemid, unsigned short color, int amt );
void getSextantCords(signed int x, signed int y, bool t2a, char *sextant);
int lineOfSight( const Coord_cl&, const Coord_cl&, int checkfor );
void reloadScripts();

// Guildstone related functions
int chardirxyz(P_CHAR pc, int x, int y);	// direction from character a to char b
bool inrange1p (PC_CHAR pca, P_CHAR pcb);
//unsigned char tempeffect(P_CHAR pc_source, P_CHAR pc_dest, int num, unsigned char more1, unsigned char more2, unsigned char more3,short dur=-1);
//unsigned char tempeffect2(P_CHAR source, P_ITEM piDest, int num, unsigned char more1, unsigned char more2, unsigned char more3);
void setcharflag(P_CHAR pc);

#endif

