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

	void Walking( CHARACTER s, int dir, int seq );
	void CombatWalk( int s );
	bool CanCharWalk(CHARACTER c, short int x, short int y, signed char &z);
	bool CanCharMove(CHARACTER c, short int x, short int y, signed char &z, int dir);
	void NpcMovement( unsigned int currenttime, int i );
	int validNPCMove(CHARACTER c, short int x, short int y, signed char &z, int dir);

	int validNPCMove( short int x, short int y, signed char z, CHARACTER s );
	
	int  calc_walk( CHARACTER c, unsigned int x, unsigned int y, unsigned int oldx, unsigned int oldy, bool justask );
	bool calc_move( CHARACTER c, short int x, short int y, signed char &z, int dir );

private:

	bool MoveHeightAdjustment( int MoveType, unitile_st *thisblock, int &ontype, signed int &nItemTop, signed int &nNewZ );
	bool isValidDirection(int dir);
	bool isFrozen(CHARACTER c, UOXSOCKET socket, int sequence);
	bool isOverloaded(CHARACTER c, UOXSOCKET socket, int sequence);

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

	short int CheckMovementType(CHARACTER c);
	bool CheckForCharacterAtXY(CHARACTER c);
	bool CheckForCharacterAtXYZ(CHARACTER c, short int cx, short int cy, signed char cz);

	void NpcWalk( CHARACTER i, int j, int type );
	unsigned short GetXfromDir( int dir, unsigned short x );
	unsigned short GetYfromDir( int dir, unsigned short y );
	void PathFind( CHARACTER c, unsigned short gx, unsigned short gy );

	bool VerifySequence(CHARACTER c, UOXSOCKET socket, int sequence);
	bool CheckForRunning(CHARACTER c, UOXSOCKET socket, int dir);
	bool CheckForStealth(CHARACTER c, UOXSOCKET socket);
	bool CheckForHouseBan(CHARACTER c, UOXSOCKET socket);
	void MoveCharForDirection(CHARACTER c, int dir);
	void HandleRegionStuffAfterMove(CHARACTER c, short int oldx, short int oldy);
	void SendWalkToPlayer(P_CHAR pc, UOXSOCKET socket, short int sequence);
	void SendWalkToOtherPlayers(P_CHAR pc, int dir, short int oldx, short int oldy);
	void OutputShoveMessage(P_CHAR pc, UOXSOCKET socket, short int oldx, short int oldy);
	void HandleItemCollision(CHARACTER c, UOXSOCKET socket, bool amTurning);
	void HandleTeleporters(P_CHAR pc, UOXSOCKET socket, short int oldx, short int oldy);
	void HandleWeatherChanges(P_CHAR pc, UOXSOCKET socket);
	void HandleGlowItems(P_CHAR pc, UOXSOCKET socket);
	bool IsGMBody(CHARACTER c);
	signed char CheckWalkable( CHARACTER c, unitile_st *xyblock, int xycount );

	bool CrazyXYBlockStuff(CHARACTER c, UOXSOCKET socket, short int oldx, short int oldy, int sequence);
	void FillXYBlockStuff(CHARACTER c, unitile_st *xyblock, int &xycount, unsigned short oldx, unsigned short oldy );

	void deny(int k, int s, int sequence);
};

#endif // __WALKING2_H__