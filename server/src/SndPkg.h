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

////////////////////////
// SndPkg.h: interface for SndPkg.cpp
//

#if !defined(__SNDPKG_H__)
#define __SNDPKG_H__

// Platform specifics
#include "platform.h"

// System includes

#include <iostream>

using namespace std;

// Forward class Declaration
class QString;

// Wolfpack includes 
#include "defines.h"
#include "structs.h"
#include "typedefs.h"		// UOXSOCKET and others

// New Style Packet classes:
class cPacket {
protected:
	QByteArray data;
public:
	cPacket( void ) {};
	virtual ~cPacket( void ) {};

	virtual void send( UOXSOCKET socket );
};

class cVariablePacket: public cPacket {
public:
	virtual void send( UOXSOCKET socket );
};

// Class for sending a container gump
class cPDrawContainer: public cPacket
{
protected:
	UI16 gumpId;
	SERIAL serial;
public:
	cPDrawContainer( UI16 gumpId, SERIAL serial );
};

class cPContainerItems: public cVariablePacket
{
public:
	cPContainerItems( void );

	virtual void addItem( P_ITEM item );
	virtual void addItem( SERIAL serial, UI16 model, UI16 amount, UI16 x, UI16 y, SERIAL contserial, UI16 hue = 0 );
};

void SndAttackOK(UOXSOCKET s, int serial);
void SndDyevat(UOXSOCKET s, int serial, short id);
void SndUpdscroll(UOXSOCKET s, short txtlen, char* txt);
void SndRemoveitem(int serial);
void SndShopgumpopen(UOXSOCKET s, int serial);

void soundeffect(int s, unsigned char a, unsigned char b); // Play sound effect for player
void soundeffect2(P_CHAR pc, short sound);
void soundeffect2(P_CHAR pc, unsigned char a, unsigned char b);
void soundeffect3(P_ITEM pi, short sound);
void soundeffect4(P_ITEM pi, UOXSOCKET s, unsigned char a, unsigned char b);
void soundeffect5(UOXSOCKET s, unsigned char a, unsigned char b);
void action(int s, int x); // Character does a certain action
void npcaction(P_CHAR pc_npc, int x); // NPC character does a certain action
void sysbroadcast(const char *txt); // System broadcast in bold text
void itemmessage(UOXSOCKET s, char *txt, int serial, short color=0x0000);
void wearIt(const UOXSOCKET s, const P_ITEM pi);
void backpack(UOXSOCKET s, SERIAL serial); // Send Backpack (with items);
void backpack2(int s, SERIAL serial); // Send corpse stuff
void sendbpitem(UOXSOCKET s, P_ITEM pi); // Update single item in backpack
void tileeffect(int x, int y, int z, char eff1, char eff2, char speed, char loop);
void senditem(UOXSOCKET s, P_ITEM pi); // Send items (on ground);
void senditem_lsd(UOXSOCKET s, P_ITEM pi,char color1, char color2, int x, int y, signed char z);
void sendperson_lsd(UOXSOCKET s, P_CHAR pc, char color1, char color2);
void chardel (UOXSOCKET s); // Deletion of character
void textflags (UOXSOCKET s, P_CHAR pc, char *name);
void teleport(P_CHAR pc); // Teleports character to its current set coordinates
void teleport2(P_CHAR pc); // used for /RESEND only - Morrolan, so people can find their corpses
void updatechar(P_CHAR pc); // If character status has been changed (Polymorph);, resend him
void target(UOXSOCKET s, int a1, int a2, int a3, int a4, char *txt); // Send targetting cursor to client
void skillwindow(int s); // Opens the skills list, updated for client 1.26.2b by LB
void updatestats(P_CHAR pc, char x);
void statwindow(int s, P_CHAR pc); // Opens the status window
void updates(UOXSOCKET s); // Update Window
void tips(int s, int i); // Tip of the day window
void weblaunch(int s, char *txt); // Direct client to a web page
void broadcast(int s); // GM Broadcast (Done if a GM yells something);
void itemtalk(int s, P_ITEM pi, char *txt); // Item "speech"
void npctalk(int s, cChar* pNpc, const char *txt,char antispam); // NPC speech
void npctalkall(cChar* pNpc, const char *txt,char antispam); // NPC speech to all in range.
void npctalk_runic(int s, P_CHAR pc_npc, const char *txt,char antispam); // NPC speech
void npcemote(int s, P_CHAR pc_npc, char *txt, char antispam); // NPC speech


void staticeffect (P_CHAR pc_player, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, bool UO3DonlyEffekt=false, stat_st *str=NULL, bool skip_old=false );
void movingeffect(P_CHAR pc_source, P_CHAR pc_dest, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode, bool UO3DonlyEffekt=false, move_st *str=NULL, bool skip_old=false);
void bolteffect(P_CHAR pc_player, bool UO3DonlyEffekt=false, bool skip_old=false);
void staticeffect2(P_ITEM pi, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode, bool UO3DonlyEffekt=false, stat_st *str=NULL, bool skip_old=false);

void staticeffect3(UI16 x, UI16 y, SI08 z, unsigned char eff1, unsigned char eff2, char speed, char loop, char explode);
void movingeffect3(P_CHAR pc_source, unsigned short x, unsigned short y, signed char z, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode);
void movingeffect3(P_CHAR pc_source, P_CHAR pc_dest, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode,unsigned char unk1,unsigned char unk2,unsigned char ajust,unsigned char type);
void movingeffect2(P_CHAR pc_source, P_ITEM dest, unsigned char eff1, unsigned char eff2, unsigned char speed, unsigned char loop, unsigned char explode);
void bolteffect2(P_CHAR pc_player,char a1,char a2);	// experimenatal, lb

void staticeffectUO3D(P_CHAR pc_cs, stat_st *sta);
void movingeffectUO3D(P_CHAR pc_cs, P_CHAR pc_cd, move_st *sta);
void bolteffectUO3D(P_CHAR player);
void itemeffectUO3D(P_ITEM pi, stat_st *sta);

void dolight(int s, char level);
void updateskill(int s, int skillnum); // updated for client 1.26.2b by LB
void deathaction(P_CHAR pc, P_ITEM pi_x); // Character does a certain action
void deathmenu(int s); // Character sees death menu
void impowncreate(int s, P_CHAR pc, int z); //socket, player to send
void sendshopinfo(int s, P_CHAR pc, P_ITEM pi);
int sellstuff(UOXSOCKET s, P_CHAR pc);
void playmidi(int s, char num1, char num2);
void sendtradestatus(P_ITEM cont1, P_ITEM cont2);
void endtrade(SERIAL);
void tellmessage(int i, int s, const char *txt);
void PlayDeathSound( P_CHAR pc );
void sysmessage(UOXSOCKET s, char *txt, ...); // System message (In lower left corner);
void sysmessage(UOXSOCKET s, short color, char *txt, ...);
void sysmessage(UOXSOCKET s, const QString& txt);
void sysmessage(UOXSOCKET s, short color, const QString&);

#endif
