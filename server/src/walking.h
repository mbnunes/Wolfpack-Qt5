/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
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

#if !defined(__WALKING2_H__)
#define __WALKING2_H__

#include "typedefs.h"
#include "singleton.h"

#include <qglobal.h>

struct unitile_st
{
	signed char basez;
	unsigned char type; // 0=Terrain, 1=Item
	unsigned short int id;
	unsigned char flag1;
	unsigned char flag2;
	unsigned char flag3;
	unsigned char flag4;
	signed char height;
	unsigned char weight;
}/* PACK*/;

bool mayWalk( P_CHAR pChar, Coord& pos );

class cMovement
{
private:
	signed char z;

public:
	bool Walking( P_CHAR pChar, Q_UINT8 dir, Q_UINT8 sequence );
	void CombatWalk( P_CHAR pc );
	void NpcMovement( unsigned int currenttime, P_NPC pc_i );
	bool canLandMonsterMoveHere( Coord& ) const;
	bool CheckForCharacterAtXYZ( P_CHAR pc, const Coord& pos );
	Coord calcCoordFromDir( Q_UINT8 dir, const Coord& oldCoords );
private:
	bool consumeStamina( P_PLAYER pChar, bool running );
	bool verifySequence( cUOSocket* socket, Q_UINT8 sequence ) throw();
	void checkStealth( P_CHAR );
	void sendWalkToOther( P_PLAYER pChar, P_CHAR pWalker, const Coord& oldpos );

	void handleItemCollision( P_CHAR pChar );
	void handleTeleporters( P_CHAR pc, const Coord& oldpos );
};

typedef Singleton<cMovement> Movement;

#endif // __WALKING2_H__
