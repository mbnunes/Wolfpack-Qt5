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

#if !defined(__WALKING2_H__)
#define __WALKING2_H__

#include "wolfpack.h"

//##ModelId=3C5D92AD0036
class cMovement
{

	// Variable/Type definitions
private:

	signed char z, dispz;
	
	// Function declarations

public:

	//##ModelId=3C5D92AD0054
	void Walking( P_CHAR pc, int dir, int seq );
	//##ModelId=3C5D92AD0072
	void CombatWalk( P_CHAR pc );
	//##ModelId=3C5D92AD0086
	bool CanCharWalk(P_CHAR pc, short int x, short int y, signed char &z);
	//##ModelId=3C5D92AD00AF
	bool CanCharMove(P_CHAR pc, short int x, short int y, signed char &z, int dir);
	//##ModelId=3C5D92AD00EA
	void NpcMovement( unsigned int currenttime, P_CHAR pc_i );
//	int validNPCMove(P_CHAR pc, short int x, short int y, signed char &z, int dir);

	//##ModelId=3C5D92AD0130
	int validNPCMove( short int x, short int y, signed char z, P_CHAR pc_s );
	
	//##ModelId=3C5D92AD0158
	int  calc_walk( P_CHAR pc, unsigned int x, unsigned int y, unsigned int oldx, unsigned int oldy, bool justask );
	//##ModelId=3C5D92AD018B
	bool calc_move( P_CHAR pc, short int x, short int y, signed char &z, int dir );

	// Static members
	//##ModelId=3C5D92AD01BE
	static void getXYfromDir(int dir, int *x, int *y);

private:

	//##ModelId=3C5D92AD01E6
	bool MoveHeightAdjustment( int MoveType, unitile_st *thisblock, int &ontype, signed int &nItemTop, signed int &nNewZ );
	//##ModelId=3C5D92AD0221
	bool isValidDirection(int dir);
	//##ModelId=3C5D92AD0235
	bool isFrozen(P_CHAR pc, UOXSOCKET socket, int sequence);
	//##ModelId=3C5D92AD025D
	bool isOverloaded(P_CHAR pc, UOXSOCKET socket, int sequence);

	//##ModelId=3C5D92AD0285
	bool CanGMWalk(unitile_st xyb);
	//##ModelId=3C5D92AD02A3
	bool CanPlayerWalk(unitile_st xyb);
	//##ModelId=3C5D92AD02C1
	bool CanNPCWalk(unitile_st xyb);
	//##ModelId=3C5D92AD02E9
	bool CanFishWalk(unitile_st xyb);
	//##ModelId=3C5D92AD0307
	bool CanBirdWalk(unitile_st xyb);

	//##ModelId=3C5D92AD0325
	void FillXYBlockStuff(short int x, short int y, unitile_st *xyblock, int &xycount);
	//##ModelId=3C5D92AD0361
	void GetBlockingMap(SI16 x, SI16 y, unitile_st *xyblock, int &xycount);
	//##ModelId=3C5D92AD0389
	void GetBlockingStatics(SI16 x, SI16 y, unitile_st *xyblock, int &xycount);
	//##ModelId=3C5D92AD03BB
	void GetBlockingDynamics(SI16 x, SI16 y, unitile_st *xyblock, int &xycount);

	//##ModelId=3C5D92AE0010
	short int Distance(short int sx, short int sy, short int dx, short int dy);
	//##ModelId=3C5D92AE0088
	short int Direction(short int sx, short int sy, short int dx, short int dy);

	//##ModelId=3C5D92AE00C4
	short int CheckMovementType(P_CHAR pc);
	//##ModelId=3C5D92AE00D8
	bool CheckForCharacterAtXY(P_CHAR pc);
	//##ModelId=3C5D92AE010A
	bool CheckForCharacterAtXYZ(P_CHAR pc, short int cx, short int cy, signed char cz);

	//##ModelId=3C5D92AE0132
	void NpcWalk( P_CHAR pc_i, int j, int type );
	//##ModelId=3C5D92AE015A
	unsigned short GetXfromDir( int dir, unsigned short x );
	//##ModelId=3C5D92AE018C
	unsigned short GetYfromDir( int dir, unsigned short y );
	//##ModelId=3C5D92AE01AA
	void PathFind( P_CHAR pc, unsigned short gx, unsigned short gy );

	//##ModelId=3C5D92AE01D2
	bool VerifySequence(P_CHAR pc, UOXSOCKET socket, int sequence) throw();
	//##ModelId=3C5D92AE01F0
	bool CheckForRunning(P_CHAR pc, UOXSOCKET socket, int dir);
	//##ModelId=3C5D92AE0218
	bool CheckForStealth(P_CHAR pc, UOXSOCKET socket);
	//##ModelId=3C5D92AE0240
	bool CheckForHouseBan(P_CHAR pc, UOXSOCKET socket);
	//##ModelId=3C5D92AE029A
	void MoveCharForDirection(P_CHAR pc, int dir);
	//##ModelId=3C5D92AE02D6
	void HandleRegionStuffAfterMove(P_CHAR pc, short int oldx, short int oldy);
	//##ModelId=3C5D92AE02FE
	void SendWalkToPlayer(P_CHAR pc, UOXSOCKET socket, short int sequence);
	//##ModelId=3C5D92AE0327
	void SendWalkToOtherPlayers(P_CHAR pc, int dir, short int oldx, short int oldy);
	//##ModelId=3C5D92AE0350
	void OutputShoveMessage(P_CHAR pc, UOXSOCKET socket, short int oldx, short int oldy);
	//##ModelId=3C5D92AE03A0
	void HandleItemCollision(P_CHAR pc, UOXSOCKET socket, bool amTurning);
	//##ModelId=3C5D92AE03C7
	void HandleTeleporters(P_CHAR pc, UOXSOCKET socket, short int oldx, short int oldy);
	//##ModelId=3C5D92AF0039
	void HandleWeatherChanges(P_CHAR pc, UOXSOCKET socket);
	//##ModelId=3C5D92AF0057
	void HandleGlowItems(P_CHAR pc, UOXSOCKET socket);
	//##ModelId=3C5D92AF0075
	bool IsGMBody(P_CHAR pc);
	//##ModelId=3C5D92AF0093
	signed char CheckWalkable( P_CHAR pc, unitile_st *xyblock, int xycount );

	//##ModelId=3C5D92AF00BB
	bool CrazyXYBlockStuff(P_CHAR pc, UOXSOCKET socket, short int oldx, short int oldy, int sequence);
	//##ModelId=3C5D92AF013D
	void FillXYBlockStuff(P_CHAR pc, unitile_st *xyblock, int &xycount, unsigned short oldx, unsigned short oldy );

	//##ModelId=3C5D92AF016F
	void deny(UOXSOCKET k, P_CHAR pc, int sequence);
};

#endif // __WALKING2_H__

