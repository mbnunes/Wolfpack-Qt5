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

#if !defined(__WALKING2_H__)
#define __WALKING2_H__

#include "wolfpack.h"

class cMovement
{

	// Variable/Type definitions
private:

	signed char z, dispz;
	
	// Function declarations

public:

	void Walking( P_CHAR pc, int dir, int seq );
	void CombatWalk( P_CHAR pc );
	bool CanCharWalk(P_CHAR pc, short int x, short int y, signed char &z);
	bool CanCharMove(P_CHAR pc, short int x, short int y, signed char &z, int dir);
	void NpcMovement( unsigned int currenttime, P_CHAR pc_i );
//	int validNPCMove(P_CHAR pc, short int x, short int y, signed char &z, int dir);

	int validNPCMove( short int x, short int y, signed char z, P_CHAR pc_s );
	
	int  calc_walk( P_CHAR pc, unsigned int x, unsigned int y, unsigned int oldx, unsigned int oldy, bool justask );
	bool calc_move( P_CHAR pc, short int x, short int y, signed char &z, int dir );

	// Static members
	static void getXYfromDir(int dir, int *x, int *y);

private:

	bool MoveHeightAdjustment( int MoveType, unitile_st *thisblock, int &ontype, signed int &nItemTop, signed int &nNewZ );
	bool isValidDirection(int dir);
	bool isFrozen(P_CHAR pc, UOXSOCKET socket, int sequence);
	bool isOverloaded(P_CHAR pc, UOXSOCKET socket, int sequence);

	bool CanGMWalk(unitile_st xyb);
	bool CanPlayerWalk(unitile_st xyb);
	bool CanNPCWalk(unitile_st xyb);
	bool CanFishWalk(unitile_st xyb);
	bool CanBirdWalk(unitile_st xyb);

	void FillXYBlockStuff(short int x, short int y, unitile_st *xyblock, int &xycount);
	void GetBlockingMap(SI16 x, SI16 y, unitile_st *xyblock, int &xycount);
	void GetBlockingStatics(SI16 x, SI16 y, unitile_st *xyblock, int &xycount);
	void GetBlockingDynamics(SI16 x, SI16 y, unitile_st *xyblock, int &xycount);

	short int Distance(short int sx, short int sy, short int dx, short int dy);
	short int Direction(short int sx, short int sy, short int dx, short int dy);

	short int CheckMovementType(P_CHAR pc);
	bool CheckForCharacterAtXY(P_CHAR pc);
	bool CheckForCharacterAtXYZ(P_CHAR pc, short int cx, short int cy, signed char cz);

	void NpcWalk( P_CHAR pc_i, int j, int type );
	unsigned short GetXfromDir( int dir, unsigned short x );
	unsigned short GetYfromDir( int dir, unsigned short y );
	void PathFind( P_CHAR pc, unsigned short gx, unsigned short gy );

	bool VerifySequence(P_CHAR pc, UOXSOCKET socket, int sequence) throw();
	bool CheckForRunning(P_CHAR pc, UOXSOCKET socket, int dir);
	bool CheckForStealth(P_CHAR pc, UOXSOCKET socket);
	bool CheckForHouseBan(P_CHAR pc, UOXSOCKET socket);
	void MoveCharForDirection(P_CHAR pc, int dir);
	void HandleRegionStuffAfterMove(P_CHAR pc, short int oldx, short int oldy);
	void SendWalkToPlayer(P_CHAR pc, UOXSOCKET socket, short int sequence);
	void SendWalkToOtherPlayers(P_CHAR pc, int dir, short int oldx, short int oldy);
	void OutputShoveMessage(P_CHAR pc, UOXSOCKET socket, short int oldx, short int oldy);
	void HandleItemCollision(P_CHAR pc, UOXSOCKET socket, bool amTurning);
	void HandleTeleporters(P_CHAR pc, UOXSOCKET socket, short int oldx, short int oldy);
	void HandleWeatherChanges(P_CHAR pc, UOXSOCKET socket);
	void HandleGlowItems(P_CHAR pc, UOXSOCKET socket);
	bool IsGMBody(P_CHAR pc);
	signed char CheckWalkable( P_CHAR pc, unitile_st *xyblock, int xycount );

	bool CrazyXYBlockStuff(P_CHAR pc, UOXSOCKET socket, short int oldx, short int oldy, int sequence);
	void FillXYBlockStuff(P_CHAR pc, unitile_st *xyblock, int &xycount, unsigned short oldx, unsigned short oldy );

	void deny(UOXSOCKET k, P_CHAR pc, int sequence);
};

#endif // __WALKING2_H__

