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

#if !defined(__WALKING2_H__)
#define __WALKING2_H__

#include "wolfpack.h"

class cUOSocket;

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

bool mayWalk( P_CHAR pChar, Coord_cl &pos );

class cMovement
{
private:
	signed char z, dispz;

public:
	void Walking( P_CHAR pChar, Q_UINT8 dir, Q_UINT8 sequence );
	void CombatWalk( P_CHAR pc );
	void NpcMovement( unsigned int currenttime, P_CHAR pc_i );
	bool canLandMonsterMoveHere( const Coord_cl& ) const;
private:
	inline bool isValidDirection( Q_UINT8 dir );

	bool CanGMWalk(unitile_st xyb);
	bool CanPlayerWalk(unitile_st xyb);
	bool CanNPCWalk(unitile_st xyb);
	bool CanFishWalk(unitile_st xyb);
	bool CanBirdWalk(unitile_st xyb);

	void FillXYBlockStuff(short int x, short int y, unitile_st *xyblock, int &xycount);
	void GetBlockingMap(const Coord_cl, unitile_st *xyblock, int &xycount);
	void GetBlockingStatics(const Coord_cl, unitile_st *xyblock, int &xycount);
	void GetBlockingDynamics(const Coord_cl, unitile_st *xyblock, int &xycount);

	short int Direction(short int sx, short int sy, short int dx, short int dy);

	short int CheckMovementType(P_CHAR pc);
	bool CheckForCharacterAtXYZ(P_CHAR pc, const Coord_cl &pos );

	void randomNpcWalk( P_CHAR pChar, Q_UINT8 dir, Q_UINT8 type );
	Coord_cl calcCoordFromDir( Q_UINT8 dir, const Coord_cl& oldCoords );
	void PathFind( P_CHAR pc, unsigned short gx, unsigned short gy );

	bool consumeStamina( cUOSocket *socket, P_CHAR pChar, bool running );
	bool checkObstacles( cUOSocket *socket, P_CHAR pChar, const Coord_cl &newPos, bool running );
	bool verifySequence( cUOSocket *socket, Q_UINT8 sequence ) throw();
	void checkRunning( cUOSocket*, P_CHAR, Q_UINT8 );
	void checkStealth( P_CHAR );
	void sendWalkToOther( P_CHAR pChar, P_CHAR pWalker, const Coord_cl& oldpos );

	void handleItemCollision( P_CHAR pChar );
	void outputShoveMessage( P_CHAR pChar, cUOSocket *socket, const Coord_cl& oldpos );
	void HandleTeleporters(P_CHAR pc, const Coord_cl& oldpos);
	void HandleWeatherChanges(P_CHAR pc, UOXSOCKET socket);
	
	void FillXYBlockStuff(P_CHAR pc, unitile_st *xyblock, int &xycount, unsigned short oldx, unsigned short oldy );

	static bool checkBoundingBox(const Coord_cl pos, int fx1, int fy1, int fx2, int fy2);
	static bool checkBoundingCircle(const Coord_cl pos, int fx1, int fy1, int radius);

};

typedef SingletonHolder<cMovement> Movement;

#endif // __WALKING2_H__

