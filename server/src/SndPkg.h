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

// Forward class Declaration
class QString;
class cUObject;

// Wolfpack includes 
#include "defines.h"
#include "structs.h"
#include "typedefs.h"		// UOXSOCKET and others

void SndAttackOK(UOXSOCKET s, int serial);
void SndDyevat(UOXSOCKET s, int serial, short id);
void SndUpdscroll(UOXSOCKET s, short txtlen, char* txt);
void SndRemoveitem(int serial);
void SndShopgumpopen(UOXSOCKET s, int serial);

void soundeffect2(P_CHAR pc, unsigned char a, unsigned char b);
void soundeffect4(P_ITEM pi, UOXSOCKET s, unsigned char a, unsigned char b);
void soundeffect5(UOXSOCKET s, unsigned char a, unsigned char b);
void sysbroadcast( const QString& txt ); // System broadcast in bold text
void tileeffect(int x, int y, int z, char eff1, char eff2, char speed, char loop);
void chardel (UOXSOCKET s); // Deletion of character
void teleport(P_CHAR pc); // Teleports character to its current set coordinates
void teleport2(P_CHAR pc); // used for /RESEND only - Morrolan, so people can find their corpses
void npctalk_runic(int s, P_CHAR pc_npc, const char *txt,char antispam); // NPC speech
void npcemote(int s, P_CHAR pc_npc, const char *txt, char antispam); // NPC speech

void dolight(int s, char level);

void PlayDeathSound( P_CHAR pc );
void sysmessage(UOXSOCKET s, char *txt, ...); // System message (In lower left corner);
void sysmessage(UOXSOCKET s, short color, char *txt, ...);
void sysmessage(UOXSOCKET s, const QString& txt);
void sysmessage(UOXSOCKET s, short color, const QString&);

#endif
