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


#include "walking2.h"
#include "wolfpack.h"
#include "debug.h"

#undef  DBGFILE
#define DBGFILE "SrvParms.cpp"


// DEFINES (Some of these should probably be moved to typedefs.h in the future)

// These are defines that I'll use. I have a history of working with properties, so that's why
// I'm using custom definitions here versus what may be defined in the other includes.
// NOTE: P = Property, P_C = Property for Characters, P_PF = Property for Pathfinding
//       P_M = Property for Movement

#define P_C_PRIV_GM			0x01	// GM Privilege Bit
#define P_C_PRIV_COUNSELOR	0x80	// Counselor Privilege Bit

#define P_C_PRIV2_FROZEN	0x02	// Character Frozen Bit

#define P_C_IS_GM_BODY		0x01	// Bits for different movement types
#define P_C_IS_PLAYER		0x02
#define P_C_IS_BIRD			0x20
#define P_C_IS_NPC			0x40
#define P_C_IS_FISH			0x80

// Maximum Search Depth: Iterations to calculate
#define P_PF_MSD		5
// Maximum Return Value: Number of steps to return (Replaces PATHNUM)
// NOTE: P_PF_MRV CANNOT EXCEED THE VALUE OF PATHNUM FOR THE TIME BEING
#define P_PF_MRV		2
// Maximum Influence Range: Tiles to left/right of path to account for
#define P_PF_MIR		5
// Maximum Blocked Range: MIR to use if character is stuck
#define P_PF_MBR		10
// Minimum Flee Distance: MFD
#define P_PF_MFD		15

// Ok, I played with this some, and in some places in T2A, the max height that I should be allowed
// to climb is actually around 14. So, I'm gonna use a different var up here, and if you
// don't agree with me, just give it the value MaxZstep [9] (defined in typedefs.h)

#define P_M_MAX_Z_CLIMB		14
#define P_M_MAX_Z_INFLUENCE	15
#define P_M_MAX_Z_FALL		15

// These are the debugging defines

// These are the errors that we should want to see. RARELY results in any console spam unless
// someone is trying to use an exploit or is sending invalid data directly to the server.
#define DEBUG_WALK_ERROR	1

#define DEBUG_NPCWALK		0
#define DEBUG_WALK			0
#define DEBUG_PATHFIND		0

//#define DBGFILE "walking2.cpp"

// Variable Declarations

// CLEAN CODE END

// Ok, as a part of the cleanup effort of the code, I'm seperating everything into sections...


// Thyme 2000.09.21
// This is my attempt to rewriting the walking code. I'm going to take the code and documentation
// that others before me have used and incorporate my little (big?) fixes and comments.

// Many thanks to all of the previous contributors, and I hope these changes help out.
// Lord Binary
// Morrolan
// Anti-Christ
// fur            : 1999.10.27 - rewrite of walking.cpp with documentation!
//                : 1999.10.27 - ripped apart walking into smaller functions
// Tauriel        : 1999.03.06 - For all of the region stuff
// knoxos         : 2000.08.?? - For finally making use of the flags, and height blocking


// Now, off to the races. If you see a function somewhere, and don't know what it is, a
// description of what the function does will be located right before it in the code... with
// the logic, if possible.

/*
** Walking() This function is called whenever we get a message from the client
** to walk/run somewhere.   It is also called by the NPC movement functions in this
** class to make the NPCs move.  The arguments are fairly fixed because we don't
** have a lot of control about what the client gives us.
**
** CHARACTER s - Obviously the character index of the character trying to move.
**
** dir - Which direction the character is trying to move. The first nibble holds
** the cardinal direction.      If the bit 0x80 is set, it means the character is
** running instead of walking.  
**              0: // North
**              1: // Northeast
**              2: // East
**              3: // Southeast
**              4: // South
**              5: // Southwest
**              6: // West
**              7: // Northwest
**
** sequence - This is what point in the walking sequence we are at, this seems to
**            roll over once it hits 256
**
*/

void cMovement::Walking(P_CHAR pc, int dir, int sequence)
{
	// Here it used to check if dir was -1 and return. We need to make sure that we
	// don't have any unexpected values, otherwise how can we eliminate dir as a potential
	// walking bug. If value is invalid, don't allow the walk. -1 falls in that range.
	
	// sometimes the NPC movement code comes up with -1, for example, if we are following someone
	// and we are directly on top of them

	if ( ! isValidDirection(dir) )
	{
#if DEBUG_WALK_ERROR
		printf("%s (cMovement::Walking) caught bad direction = %s %d 0x%x\n", DBGFILE, pc->name, dir, dir);
#endif
		pc->pathnum += PATHNUM;
		return;
	}

    UOXSOCKET socket = calcSocketFromChar(DEREF_P_CHAR(pc));
    
    if (!VerifySequence(pc, socket, sequence))
        return;

	// If checking for weight is more expensive, shouldn't we check for frozen first?
	if ( isFrozen(pc, socket, sequence) )
		return;

    if ( isOverloaded(pc, socket, sequence) )
        return;
    
	// save our original location before we even think about moving
	const short int oldx = pc->pos.x;
	const short int oldy = pc->pos.y;
	const signed char oldz = pc->pos.z;
	
	// this if assumes that chars[s].dir has no high-bits just lets make sure of it
	// assert((pc->dir & 0xFFF0) == 0);
	// this assertion is failing, so either my assumption about it is wrong or there
	// is a bugaboo
	
	// see if we have stopped to turn or if we are moving
	const bool amTurning = ((dir&0x07) != pc->dir);
	if (!amTurning)
	{
		if (!CheckForRunning(pc, socket, dir))
			return;
		
		if (!CheckForStealth(pc, socket))
			return;
		
		/* this is already done in the cNetwork method that calls this, so its redundant here
        ** i'm leaving this in because it might make more sense to have it here because we can
        * call it only in the case of actual movement
        if (pc->med) //Morrolan - Meditation
        {
            pc->med=0; 
            sysmessage(c, "You break your concentration.");
        }
		*/
		
		// if this was an NPC lets reset their move timer
		// this seems to be an usual place within this function to reset this
		// i guess they can turn a whole lot this way
		// Thyme: Already reset in NPCMovement (which calls this function, and NPCWalk)
		//if (pc->npc) 
		//{
		//	pc->npcmovetime=(unsigned int)(uiCurrentTime+(double)(NPCSPEED*CLOCKS_PER_SEC)); //reset move timer
		//}

		signed char myz = illegal_z;
		short int myx = GetXfromDir(dir, pc->pos.x);
		short int myy = GetYfromDir(dir, pc->pos.y);
		if ( ! CanCharMove(pc, pc->pos.x, pc->pos.y, myz, dir) )
		{
#if DEBUG_WALK
			printf("%s (cMovement::Walking) Character Walk Failed for %s\n", DBGFILE, pc->name);
			printf("%s (cMovement::Walking) sx (%d) sy (%d) sz (%d)\n", DBGFILE, pc->pos.x, pc->pos.y, pc->pos.z);
			printf("%s (cMovement::Walking) dx (%d) dy (%d) dz (%d)\n", DBGFILE, myx, myy, myz);
#endif
			if ( socket != INVALID_UOXSOCKET )
				deny(socket, pc, sequence);
			if ( pc->isNpc() )
					pc->pathnum += P_PF_MRV;
			return;
		}
		dispz = z = myz;

#if DEBUG_WALK
		printf("%s (cMovement::Walking) Character Walk Passed for %s\n", DBGFILE, pc->name);
		printf("%s (cMovement::Walking) sx (%d) sy (%d) sz (%d)\n", DBGFILE, pc->pos.x, pc->pos.y, pc->pos.z);
		printf("%s (cMovement::Walking) dx (%d) dy (%d) dz (%d)\n", DBGFILE, myx, myy, myz);
#endif

		if ( pc->isNpc() && CheckForCharacterAtXYZ(pc, myx, myy, myz) )
		{
			pc->pathnum += P_PF_MRV;
			return;
		}

		MoveCharForDirection(pc, dir);
		
		// i actually moved this for now after the z =  illegal_z, in the end of CrazyXYBlockStuff()
		// can't see how that would hurt anything
		if (!CheckForHouseBan(pc, socket))
			return;
		
		/*
		** OK AT THIS POINT IT IS NOW OFFICIALLY A LEGAL MOVE TO MAKE, LETS GO FOR IT!
		**
		** That means any bugs concerning if a move was legal must be before this point!
		*/

		// i moved this down after we are certain we are moving
//		if( server_data.footSteps )
//  			playTileSound( socket );
		
		// since we actually moved, update the regions code
		HandleRegionStuffAfterMove(pc, oldx, oldy);            
	}
	else
	{
		//printf("Player is turning in the same spot.\n");
	}
	
	// do all of the following regardless of whether turning or moving i guess
	
	// set the player direction to contain only the cardinal direction bits
	pc->dir = (dir&0x07);
	
	SendWalkToPlayer(pc, socket, sequence);
	cRegion::RegionIterator4Chars ri(pc->pos);
	for (ri.Begin(); ri.GetData() != ri.End(); ri++)
	{
		P_CHAR pc_vis = ri.GetData();
		if (pc_vis != NULL)
		{
			int distance=chardist(DEREF_P_CHAR(pc_vis), DEREF_P_CHAR(pc));
			if(distance<=Races[pc_vis->race]->VisRange)
				SendWalkToOtherPlayers(pc_vis, dir, oldx, oldy);
		}
	}
	
	OutputShoveMessage(pc, socket, oldx, oldy);
	
	// keep on checking this even if we just turned, because if you are taking damage
	// for standing here, lets keep on dishing it out. if we pass whether we actually
	// moved or not we can optimize things some
	HandleItemCollision(pc, socket, amTurning);
	
	// i'm going ahead and optimizing this, if you haven't really moved, should be
	// no need to check for teleporters and the weather shouldn't change
	if (!amTurning)
	{
		HandleTeleporters(pc, socket, oldx, oldy);
        
		HandleWeatherChanges(pc, socket);
	}
	
	// i'm afraid i don't know what this does really, do you need to do it when turning??
	HandleGlowItems(pc, socket);
	
	// would have already collided, right??
	if (!amTurning && pc->isPlayer())
		Magic->GateCollision(DEREF_P_CHAR(pc));
	
	// again, don't know if we need to check when turning or not
	if( !amTurning )
		checkregion(DEREF_P_CHAR(pc)); // doesn't change physical coords, so no point in making a change
	//if (socket==-1) printf("checkregion called for %s region#: %i region-name:%s \n",pc->name,pc->region,region[pc->region].name);

}

// Function      : cMovement::isValidDirection()
// Written by    : Unknown
// Revised by    : Thyme
// Revision Date : 2000.09.21
// Purpose       : Check if a given direction is valid
// Method        : Return true on the below values:
//
// Direction   Walking Value   Running Value
// North          0 0x00         128 0x80
// Northeast      1 0x01         129 0x81
// East           2 0x02         130 0x82
// Southeast      3 0x03         131 0x83
// South          4 0x04         132 0x84
// Southwest      5 0x05         133 0x85
// West           6 0x06         134 0x86
// Northwest      7 0x07         135 0x87

bool cMovement::isValidDirection(int dir)
{
	return ( dir == ( dir & 0x87 ) );
}

// Function      : cMovement::isFrozen()
// Written by    : Unknown
// Revised by    : Thyme
// Revision Date : 2000.09.21
// Purpose       : Check if a character is frozen or casting a spell
// Method        : Because of the way the source uses the frozen flag, I decided to change
// something in how this works. If the character is casting a spell (chars.casting is true)
// OR if they're frozen (chars.priv2 & FROZEN_BIT) then they can't walk. Why? If a player/npc
// has their frozen bit set, and they cast a spell, they will lose their frozen bit at the
// end of the spell cast. With this new check, we don't even need to set the frozen bit when
// casting a spell!

bool cMovement::isFrozen(P_CHAR pc, UOXSOCKET socket, int sequence)
{

	if ( pc->casting )
	{
		if ( socket != INVALID_UOXSOCKET )
		{
			sysmessage(socket, "You cannot move while casting.");
			deny(socket, pc, sequence);  
		}
#if DEBUG_WALK
		printf("%s (cMovement::isFrozen) casting char %s\n", DBGFILE, pc->name);
#endif
		return true;
	}
	if ( pc->priv2 & P_C_PRIV2_FROZEN )
	{
		if (socket != INVALID_UOXSOCKET)
		{
			sysmessage(socket, "You are frozen and cannot move.");
			deny(socket, pc, sequence);  
		}
#if DEBUG_WALK
		printf("%s (cMovement::isFrozen) frozen char %s\n", DBGFILE, pc->name);
#endif
		return true;
	} 

	return false;

}

// Thyme 07/28/00

// return TRUE is character is overloaded (with weight)
// return FALSE otherwise

// CheckForWeight was confusing...

// Old code called check weight first then checked socket... I changed it the other way.
// Why, well odds are (I may be wrong) if you don't have a socket, you're an NPC and if you
// have one, you're a character. We said in the first line that we didn't want to restrict
// based upon NPC, so if you're an NPC, the socket/checkweight will never be called anyway.

// Rewrote to deny the client... We'll see if it works.

bool cMovement::isOverloaded(P_CHAR pc, UOXSOCKET socket, int sequence)
{
	// Who are we going to check for weight restrictions?
	if ( !pc->dead &&							// If they're not dead
		 !pc->isNpc() &&							// they're not an npc
		 !pc->isGMorCounselor())			// they're not a GM
	{
		// Can probably put this in the above check, but I'll keep it here for now.
		if ( socket != INVALID_UOXSOCKET )
		{
			if (!Weight->CheckWeight(DEREF_P_CHAR(pc), socket) || (pc->stm<3))
			{
				sysmessage(socket, "You are too fatigued to move, you are carrying %d stones.", pc->weight);
				deny(socket, pc, sequence);
#if DEBUG_WALK
				printf("%s (cMovement::Walking) overloaded char %s\n", DBGFILE, pc->name);
#endif
				return true;
			}
		}
	}
	return false;
}

// Thyme 07/28/00

// Here's how I'm going to use it for now.  Movement Type may be used for races, that's why
// I put it as an integer.  Here are the values I'm going to use:
// GM Body  0x01
// Player   0x02
// Bird     0x20 (basically, a fish and an NPC, so I could use 0xc0, but I don't wanna)
// NPC      0x40
// Fish     0x80 (So they can swim!)
// I left a gap between Player and NPC because someone may want to implement race
// restrictions... 

short int cMovement::CheckMovementType(P_CHAR pc)
{
	// Am I a GM Body?
	if ( IsGMBody(pc) )
		return P_C_IS_GM_BODY;

	// Am I a player?
	if (pc->isPlayer())
		return P_C_IS_PLAYER;

	// Change this to a flag in NPC.scp

	short int retval = P_C_IS_NPC;
	switch ( pc->id() )
	{
	case 0x0010 : // Water Elemental
		retval = P_C_IS_FISH;
		break;
	case 0x005F : // Kraken
	case 0x0096 : // Dolphin
	case 0x0097 : // Sea Serpent
		retval = P_C_IS_FISH;
		break;
	default : // Regular NPC
		break;
	}
	return retval;
}


bool cMovement::CheckForCharacterAtXYZ(P_CHAR pc, short int cx, short int cy, signed char cz)
{
	unsigned int StartGrid=mapRegions->StartGrid(cx, cy);
	unsigned int increment=0, checkgrid, a;
	for (checkgrid=StartGrid+(increment*mapRegions->GetColSize());increment<3;increment++, checkgrid=StartGrid+(increment*mapRegions->GetColSize()))
	{
		for (a=0;a<3;a++)
		{
			vector<SERIAL> vecEntries = mapRegions->GetCellEntries(checkgrid+a);
			for ( unsigned int k = 0; k < vecEntries.size(); k++)
			{
				P_CHAR pc_i = FindCharBySerial(vecEntries[k]);
				if (pc_i != NULL)
				{
					if (pc_i != pc && (online(DEREF_P_CHAR(pc_i)) || pc_i->isNpc()))
					{
						// x=x,y=y, and distance btw z's <= MAX STEP
						if ((pc_i->pos.x == cx) && (pc_i->pos.y == cy) && (abs(pc_i->pos.z-cz) <= P_M_MAX_Z_CLIMB))
						{
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

// check if GM Body

bool cMovement::CanGMWalk(unitile_st xyb)
{
	unsigned short int blockid = xyb.id;

	tile_st newTile;
	Map->SeekTile( blockid, &newTile );

	if ( Map->IsRoofOrFloorTile(&newTile) )
		return true;
	
	if ( xyb.type == 0 )
		return true;

	if ( xyb.flag1 & 0x40 )
		return true;

	if ( xyb.flag2 & 0x16 )
		return true;

	if ( xyb.flag4 & 0x20 )
		return true;

	if ( Map->IsTileWet(blockid) )
		return true;

// Can they walk/swim on water tiles?
	if ( blockid > 0x00A7 && blockid < 0x00AC )
		return true;
	if ( blockid > 0x1796 && blockid < 0x179D )
		return true;
	if ( blockid > 0x346D && blockid < 0x3486 )
		return true;
	if ( blockid > 0x3493 && blockid < 0x34AC )
		return true;
	if ( blockid > 0x34B7 && blockid < 0x34CB )
		return true;

	// Can they walk/swim on water ripples and splashes?
	if ( blockid > 0x34D0 && blockid < 0x34D6 )
		return true;
	if ( blockid > 0x352C && blockid < 0x3531 )
		return true;

	// Can they walk/swim on whirlpools?
	if ( blockid > 0x348F && blockid < 0x3494 )
		return true;
	if ( blockid > 0x34B4 && blockid < 0x34B8 )
		return true;

	// Can they walk/swim on/up waterfalls?
	if ( blockid > 0x34EC && blockid < 0x3529 )
		return true;

	//Can they walk/swim on the coastlines?
	if ( blockid > 0x179C && blockid < 0x17B3 )
		return true;
	if ( blockid == 0x1796 )
		return true;

	return false;
}

bool cMovement::CanNPCWalk(unitile_st xyb)
{
	unsigned short int blockid = xyb.id;

	tile_st newTile;
	Map->SeekTile( blockid, &newTile );

	if ( Map->IsRoofOrFloorTile(&newTile) )
		return true;
	
	if ( xyb.type == 0 )
		return true;

	if ( xyb.flag2 & 0x06 )
		return true;

	return false;
}

bool cMovement::CanPlayerWalk(unitile_st xyb)
{
	unsigned short int blockid = xyb.id;

	tile_st newTile;
	Map->SeekTile( blockid, &newTile );

	if ( Map->IsRoofOrFloorTile(&newTile) )
		return true;
	
	if ( xyb.type == 0 )
		return true;

	if ( xyb.flag2 & 0x06 )
		return true;

	return false;
}


bool cMovement::CanFishWalk(unitile_st xyb)
{
	unsigned short int blockid = xyb.id;
	
	if ( Map->IsTileWet(blockid) )
		return true;

	// Can they walk/swim on water tiles?
	if ( blockid > 0x00A7 && blockid < 0x00AC )
		return true;
	if ( blockid > 0x1796 && blockid < 0x179D )
		return true;
	if ( blockid > 0x346D && blockid < 0x3486 )
		return true;
	if ( blockid > 0x3493 && blockid < 0x34AC )
		return true;
	if ( blockid > 0x34B7 && blockid < 0x34CB )
		return true;

	// Can they walk/swim on water ripples and splashes?
	if ( blockid > 0x34D0 && blockid < 0x34D6 )
		return true;
	if ( blockid > 0x352C && blockid < 0x3531 )
		return true;

	// Can they walk/swim on whirlpools?
//	if ( blockid > 0x348F && blockid < 0x3494 )
//		return true;
//	if ( blockid > 0x34B4 && blockid < 0x34B8 )
//		return true;

	// Can they walk/swim on/up waterfalls?
	if ( blockid > 0x34EC && blockid < 0x3529 )
		return true;

	// Can they walk/swim on the coastlines?
	//if ( blockid > 0x179C && blockid < 0x17B3 )
		//return true;
	//if ( blockid == 0x1796 )
		//return true;

	return false;
}

// needs testing... not totally accurate, but something to hold place.

bool cMovement::CanBirdWalk(unitile_st xyb)
{
//	unsigned short int blockid = xyb.id;

	return ( CanNPCWalk(xyb) || CanFishWalk(xyb) );
}

// All of the stuff below this point is old, unmodified code

// if we have a valid socket, see if we need to deny the movement request because of
// something to do with the walk sequence being out of sync.
bool cMovement::VerifySequence(P_CHAR pc, UOXSOCKET socket, int sequence) throw()
{
    if (socket != INVALID_UOXSOCKET)
    {
        if ((walksequence[socket] + 1 != sequence) && (sequence != 256))
        {
            deny(socket, pc, sequence);  
            return false;
        }
    }
    return true;
}

bool cMovement::CheckForRunning(P_CHAR pc, UOXSOCKET socket, int dir)
// New need for return
// returns true if updatechar required, or false if not
// PARAM WARNING: unreferenced paramater socket
{
	// if we are running
	if (dir&0x80)
	{ //AntiChrist -- if running
		// if we are using stealth
		if (pc->stealth!=-1) { //AntiChrist - Stealth - stop hiding if player runs
			pc->stealth=-1;
			pc->hidden=0;
			updatechar(DEREF_P_CHAR(pc));
		}


//Don't regenerate stamina while running
		pc->regen2=uiCurrentTime+(server_data.staminarate*CLOCKS_PER_SEC);
		pc->running++;
		// if all these things
		if(!pc->dead && !pc->onhorse && pc->running>(server_data.runningstaminasteps)*2)
		{
			//The *2 it's because i noticed that a step(animation) correspond to 2 walking calls
			pc->running=0;
			pc->stm--;
			updatestats(pc,2);
		}
		if( pc->war && pc->targ != INVALID_SERIAL )
		{
			pc->timeout=uiCurrentTime+CLOCKS_PER_SEC*2;
		}

	} else {
		pc->running=0;
	}                                           
	return true;
}

bool cMovement::CheckForStealth(P_CHAR pc, UOXSOCKET socket)
// PARAM WARNING: unreferenced paramater socket
{
	if ((pc->hidden)&&(!(pc->priv2&8)))
	{
		if(pc->stealth!=-1)
		{ //AntiChrist - Stealth
			pc->stealth++;
			if(pc->stealth>((server_data.maxstealthsteps*pc->skill[STEALTH])/1000))
			{
				pc->stealth=-1;
				pc->hidden=0;
				updatechar( DEREF_P_CHAR(pc) );
			}
		}
		else
		{
			pc->hidden=0;
			updatechar( DEREF_P_CHAR(pc) );
		}
	}
	return true;
}

// see if a player has tried to move into a house they were banned from it
bool cMovement::CheckForHouseBan(P_CHAR pc, UOXSOCKET socket)
{
    if ( pc->isPlayer() ) // this code is also called from npcs-walking code, so only check for players to cut down lag!
    {
        	walksequence[socket] = -1;
		int h=HouseManager->GetHouseNum(pc);
		if(h>=0)
		{
			int i=House[h]->FindBan(pc);
			if(i>=0)
			{
				pc->pos.x = House[h]->pos2.x+1;
                pc->pos.y = House[h]->pos2.y+1;
                teleport(DEREF_P_CHAR(pc));
                if (socket!=INVALID_UOXSOCKET)
				{
					sysmessage(socket, "You are banned from that location.");
					walksequence[socket] = -1;
				}
			}
		}
    } 
    return true;
}

// Thyme 2000.09.21
// I already made sure I could move there (even the crazy XY block stuff) so this IS a valid move. Just move the
// directions. Oh, and since I we already have the GetX/YfromDir functions (and we need those) why don't we just
// use them here?

void cMovement::MoveCharForDirection(P_CHAR pc, int dir)
{
	pc->pos.x = GetXfromDir(dir, pc->pos.x);
	pc->pos.y = GetYfromDir(dir, pc->pos.y);
}


// Split up of FillXYBlockStuff


void cMovement::GetBlockingMap(SI16 x, SI16 y, unitile_st *xyblock, int &xycount)
{
	int mapid = 0;
	signed char mapz = Map->MapElevation(x,y);  //Map->AverageMapElevation(x, y, mapid);
	if (mapz != illegal_z)
	{
		land_st land;
		Map->SeekLand(mapid, &land);
		
		xyblock[xycount].type=0;
		xyblock[xycount].basez = mapz;
		xyblock[xycount].id = mapid;
		xyblock[xycount].flag1=land.flag1;
		xyblock[xycount].flag2=land.flag2;
		xyblock[xycount].flag3=land.flag3;
		xyblock[xycount].flag4=land.flag4;
		xyblock[xycount].height=0;
		xyblock[xycount].weight=255;
		xycount++;
	}
}


void cMovement::GetBlockingStatics(SI16 x, SI16 y, unitile_st *xyblock, int &xycount)
{
	MapStaticIterator msi(x, y);
	staticrecord *stat;
 	while (stat = msi.Next())
	{
		//printf("staticr[X] type=%d, id=%d\n", 2, stat->itemid);
		tile_st tile;
		msi.GetTile(&tile);
		xyblock[xycount].type=2;
		xyblock[xycount].basez=stat->zoff;
		xyblock[xycount].id=stat->itemid;
		xyblock[xycount].flag1=tile.flag1;
		xyblock[xycount].flag2=tile.flag2;
		xyblock[xycount].flag3=tile.flag3;
		xyblock[xycount].flag4=tile.flag4;
		xyblock[xycount].height=tile.height;
		xyblock[xycount].weight=255;
		xycount++;
	}
}

void cMovement::GetBlockingDynamics(SI16 x, SI16 y, unitile_st *xyblock, int &xycount)
{
	Coord_cl position(x, y, 0);
	cRegion::RegionIterator4Items ri(position);
	for (ri.Begin(); ri.GetData() != ri.End(); ri++)
	{
		P_ITEM mapitem = ri.GetData();
		if (mapitem != NULL)
		{
			if (mapitem->id1<0x40)
			{
				if ((mapitem->pos.x == x) && (mapitem->pos.y == y))
				{
					tile_st tile;
					Map->SeekTile(mapitem->id(), &tile);
					xyblock[xycount].type=1;
					xyblock[xycount].basez=mapitem->pos.z;
					xyblock[xycount].id=mapitem->id();
					xyblock[xycount].flag1=tile.flag1;
					xyblock[xycount].flag2=tile.flag2;
					xyblock[xycount].flag3=tile.flag3;
					xyblock[xycount].flag4=tile.flag4;
					xyblock[xycount].height=tile.height;
					xyblock[xycount].weight=tile.weight;
					xycount++;
				}
			}
			else if (
				(abs(mapitem->pos.x- x)<=BUILDRANGE)&&
				(abs(mapitem->pos.y- y)<=BUILDRANGE)
				)
			{
				UOXFile *mfile = NULL;
				SI32 length = 0;		// should be SI32, not long
				Map->SeekMulti(mapitem->id()-0x4000, &mfile, &length);
				length=length/MultiRecordSize;
				if (length == -1 || length>=17000000)//Too big... bug fix hopefully (Abaddon 13 Sept 1999)
					//              if (length == -1)
				{
					printf("walking() - Bad length in multi file. Avoiding stall.\n");
					length = 0;
				}
				for (int j=0;j<length;j++)
				{
					st_multi multi;
					mfile->get_st_multi(&multi);
					if (multi.visible && (mapitem->pos.x+multi.x == x) && (mapitem->pos.y+multi.y == y))
					{
						tile_st tile;
						Map->SeekTile(multi.tile, &tile);
						xyblock[xycount].type=2;
						xyblock[xycount].basez = multi.z + mapitem->pos.z;
						xyblock[xycount].id=multi.tile;
						xyblock[xycount].flag1=tile.flag1;
						xyblock[xycount].flag2=tile.flag2;
						xyblock[xycount].flag3=tile.flag3;
						xyblock[xycount].flag4=tile.flag4;
						xyblock[xycount].height=tile.height;
						xyblock[xycount].weight=255;
						xycount++;
					}
				}
			}
		}
	}
} //- end of itemcount for loop

// checkout everything we might need to take into account and fill it into the xyblock array
void cMovement::FillXYBlockStuff(short int x, short int y, unitile_st *xyblock, int &xycount)
{

	GetBlockingMap(x, y, xyblock, xycount);
	GetBlockingStatics(x, y, xyblock, xycount);
	GetBlockingDynamics(x, y, xyblock, xycount);

}


// so we are going to move, lets update the regions
// FYI, Items equal to or greater than 1000000 are considered characters...
void cMovement::HandleRegionStuffAfterMove(P_CHAR pc, short int oldx, short int oldy)
{
	// save where we were moving to
	const short int nowx = pc->pos.x;
	const short int nowy = pc->pos.y;

	// i'm trying a new optimization here, if we end up in the same map cell
	// as we started, i'm sure there's no real reason to remove and readd back
	// to the same spot..
	if (mapRegions->GetCell(oldx, oldy) != mapRegions->GetCell(nowx, nowy))
	{
		// restore our original location and remove ourself
		pc->pos.x = oldx;
		pc->pos.y = oldy;
		mapRegions->Remove(pc);
		// we have to remove it with OLD x,y ... LB, very important, and he is right!

		// restore the new location and add ourselves
		pc->pos.x = nowx;
		pc->pos.y = nowy;
	
		mapRegions->Add(pc);

		// i suspect the new weather code probably needs something here, so if
		// you walk from one region to another it can adjust the weather, but i
		// could be wrong
	}
#if DEBUG_WALKING
	else
	{
		//printf("Guess what? I didn't change regions.\n");
	}
#endif

	// i'm moving this to the end because the regions shouldn't care what the z was
	pc->dispz = dispz;
	pc->pos.z = z;
}


// actually send the walk command back to the player and increment the sequence
void cMovement::SendWalkToPlayer(P_CHAR pc, UOXSOCKET socket, short int sequence)
{
	if (socket!=INVALID_UOXSOCKET)
	{
		char walkok[4]="\x22\x00\x01";
		walkok[1]=buffer[socket][2];
		walkok[2]=0x41;
		if (pc->hidden)
			walkok[2]=0x00;
		Network->xSend(socket, walkok, 3, 0);

		walksequence[socket] = sequence;
		if (walksequence[socket] == 255)
			walksequence[socket] = 0;
	}
}

// send out our movement to all other players who can see us move
void cMovement::SendWalkToOtherPlayers(P_CHAR pc, int dir, short int oldx, short int oldy)
{
	// lets cache these vars in advance
	const int visibleRange = Races[pc->race]->VisRange;//Races->getVisRange( pc->race );
	const int newx=pc->pos.x;
	const int newy=pc->pos.y;

	for (int i=0;i<now;i++)
	{
		// lets see, its much cheaper to call perm[i] first so i'm reordering this
		if ((perm[i]) && (inrange1p(DEREF_P_CHAR(pc), DEREF_P_CHAR(currchar[i]))))
		{
			/*if (
				(((abs(newx-chars[currchar[i]].pos.x)==visibleRange )||(abs(newy-chars[currchar[i]].pos.y)== visibleRange )) &&
				((abs(oldx-chars[currchar[i]].pos.x)>visibleRange )||(abs(oldy-chars[currchar[i]].pos.y)>visibleRange ))) ||
				((abs(newx-chars[currchar[i]].pos.x)==visibleRange )&&(abs(newy-chars[currchar[i]].pos.y)==visibleRange ))
				)*/
			if ((abs(newx-currchar[i]->pos.x)<Races[pc->race]->VisRange) && (abs(newy-currchar[i]->pos.y)<Races[pc->race]->VisRange))
			{
				impowncreate(i, DEREF_P_CHAR(pc), 1);
			}
			else
				//    if ((abs(newx-chars[currchar[i]].pos.x)<VISRANGE)||(abs(newy-chars[currchar[i]].pos.y)<VISRANGE))
			{
				P_CHAR pc_check = currchar[i];
				LongToCharPtr(pc->serial, &extmove[1]);
				ShortToCharPtr(pc->id(), &extmove[5]);
				extmove[7]=pc->pos.x>>8;
				extmove[8]=pc->pos.x%256;
				extmove[9]=pc->pos.y>>8;
				extmove[10]=pc->pos.y%256;
				extmove[11]=pc->dispz;
				extmove[12]=dir;
				//     extmove[12]=chars[currchar[c]].dir&0x7F;
				//     extmove[12]=buffer[c][1];
				ShortToCharPtr(pc->skin, &extmove[13]);
				if( pc->isNpc() /*&& pc->runs*/ && pc->war ) // Ripper 10-2-99 makes npcs run in war mode or follow :) (Ab mod, scriptable)
					extmove[12]=dir|0x80;
				if( pc->isNpc() && (pc->ftarg != INVALID_SERIAL))
					extmove[12]=dir|0x80;
				if (pc->war) extmove[15]=0x40; else extmove[15]=0x00;
				if (pc->hidden) extmove[15]=extmove[15]|0x80;
				if( pc->dead && !pc->war ) extmove[15] = extmove[15]|0x80; // Ripper
				if(pc->poisoned) extmove[15]=extmove[15]|0x04; //AntiChrist -- thnx to SpaceDog
				//if (pc->npcaitype==0x02) extmove[16]=6; else extmove[16]=1;
				int guild, race;
				//chars[i].flag=0x04;       // everyone should be blue on default
				guild=Guilds->Compare( DEREF_P_CHAR(pc), DEREF_P_CHAR(currchar[i]) );
				race = Races.CheckRelation(pc,pc_check);
				if( pc->kills > repsys.maxkills ) extmove[16]=6;
				else if (guild==1 || race==1)//Same guild (Green)
					extmove[16]=2;
				else if (guild==2 || race==2) // Enemy guild.. set to orange
					extmove[16]=5;
				else
				{
					switch(pc->flag)
					{//1=blue 2=green 5=orange 6=Red 7=Transparent(Like skin 66 77a)
					case 0x01: extmove[16]=6; break;// If a bad, show as red. 
					case 0x04: extmove[16]=1; break;// If a good, show as blue.
					case 0x08: extmove[16]=2; break; //green (guilds)
					case 0x10: extmove[16]=5; break;//orange (guilds)
					default:extmove[16]=3; break;//grey 
					}
				}
				if (currchar[i] != pc ) // fix from homey (NPCs will display right)
					Network->xSend(i, extmove, 17, 0);
			}
		}
	}
}

// see if we should mention that we shove something out of the way
void cMovement::OutputShoveMessage(P_CHAR pc, UOXSOCKET socket, short int oldx, short int oldy)
{
	if (socket!=INVALID_UOXSOCKET)
	{
		// lets cache these vars in advance
		const int visibleRange = Races[pc->race]->VisRange; //Races->getVisRange( pc->race );

		const int newx=pc->pos.x;
		const int newy=pc->pos.y;

		cRegion::RegionIterator4Chars ri(pc->pos);
		for (ri.Begin(); ri.GetData() != ri.End(); ri++)
		{
			P_CHAR mapchar = ri.GetData();
			if (mapchar != NULL)
			{
#if DEBUG
				printf("DEBUG: Mapchar %i [%i]\n",mapchar,mapitem);
#endif
				//Let GMs see logged out players
				if ( online(DEREF_P_CHAR(mapchar)) || mapchar->isNpc() || pc->isGM())
				{
					if (
						(((abs(newx-mapchar->pos.x)== visibleRange )||(abs(newy-mapchar->pos.y)== visibleRange )) &&
						((abs(oldx-mapchar->pos.x) > visibleRange )||(abs(oldy-mapchar->pos.y)> visibleRange ))) ||
						((abs(newx-mapchar->pos.x)== visibleRange )&&(abs(newy-mapchar->pos.y)== visibleRange ))
						)
					{
						impowncreate(socket, DEREF_P_CHAR(mapchar), 1);
					}
				}
				if (oldx == newx && oldy == newy)	// just turning ?
				continue;						// no multiple shoving
				if (!(
				pc->id()==0x03DB ||
				pc->id()==0x0192 ||
				pc->id()==0x0193 ||
				pc->isGMorCounselor()
				))
				{
					if (mapchar != pc && (online(DEREF_P_CHAR(mapchar)) || mapchar->isNpc()))
					{
						if (mapchar->pos.x == pc->pos.x && mapchar->pos.y == pc->pos.y && mapchar->pos.z == pc->pos.z)
						{
							if (mapchar->isHidden() && !mapchar->dead && !mapchar->isInvul() && !mapchar->isGM())
							{
								sprintf(temp, "Being perfectly rested, you shoved something invisible out of the way.", mapchar->name);
								if (socket!=INVALID_UOXSOCKET) sysmessage(socket, temp);
							    pc->stm = max(pc->stm-4, 0);
								updatestats(pc, 2);  // arm code
							}
						    else if (!mapchar->isHidden() && !mapchar->dead && (!(mapchar->isInvul())) &&(!(mapchar->isGM()))) // ripper..GMs and ghosts dont get shoved.)
							{
								sprintf(temp, "Being perfectly rested, you shove %s out of the way.", mapchar->name);
								if (socket!=INVALID_UOXSOCKET) sysmessage(socket, temp);
								pc->stm = max(pc->stm-4, 0);
								updatestats(pc, 2);  // arm code
							}
						    else if(!mapchar->isGMorCounselor() && !mapchar->isInvul())//A normal player (No priv1(Not a gm))
							{
								if (socket != INVALID_UOXSOCKET) sysmessage(socket, "Being perfectly rested, you shove something invisible out of the way.");
								pc->stm=max(pc->stm-4, 0);
								updatestats(pc, 2);  // arm code
							}
						}
					}
				}
			}
		}
	}
}

// handle item collisions, make items that appear on the edge of our sight because
// visible, buildings when they get in range, and if the character steps on something
// that might cause damage

// Umm... we need to split this up...

void cMovement::HandleItemCollision(P_CHAR pc, UOXSOCKET socket, bool amTurning)
{
	// apparently we don't want NPCs to be affected by any of this stuff,
	// i'm not sure i agree with that yet
	// It's not the fact that we don't want them to be affected by it, it's just the fact that this also updates
	// the display of the char... So what we need to do is handle the collision stuff for everyone, and only send
	// the stuff to actual characters.

	if (socket == INVALID_UOXSOCKET)
		return;

	// lets cache these vars in advance
	const int visibleRange = Races[pc->race]->VisRange;//Races->getVisRange( pc->race );
	const short int newx = pc->pos.x;
	const short int newy = pc->pos.y;
	const short int oldx = GetXfromDir(pc->dir + 4, newx);
	const short int oldy = GetYfromDir(pc->dir + 4, newy);

	
	// - Tauriel's region stuff 3/6/99
	const int StartGrid = mapRegions->StartGrid(newx, newy);

	int checkgrid = 0;
	for (int increment = 0; increment < 3; increment++)
	{
		checkgrid = StartGrid + (increment * mapRegions->GetColSize());
		for (int i=0;i<3;i++)
		{
			P_ITEM mapitem = NULL;
			vector<SERIAL> vecEntries = mapRegions->GetCellEntries(checkgrid+i);
			for ( unsigned int k = 0; k < vecEntries.size(); k++)
			{
				mapitem = FindItemBySerial(vecEntries[k]);
				if (mapitem != NULL)
				{
#if DEBUG_WALKING
				  //printf("Checking against Item %s, ID1:%d, ID2:%d\n", mapitem->name, mapitem->id1, mapitem->id2);
#endif
					// split out the x,y,z check so we can use else ifs for faster item id checking
					if ((mapitem->id1==0x39 && (mapitem->id2==0x96 || mapitem->id2==0x8C)))
					{//Fire Field
// Thyme 2000.09.15
// At the request of Abaddon
// Thyme BEGIN
//						if ((mapitem->pos.x == newx) && (mapitem->pos.y == newy) && (mapitem->pos.z==pc->pos.z))
						if ( ( mapitem->pos.x == newx ) &&
							 ( mapitem->pos.y == newy ) &&
							 ( pc->pos.z >= mapitem->pos.z ) &&
							 ( pc->pos.z <= ( mapitem->pos.z + 5 ) ) )
// Thyme END
						{
							if (!Magic->CheckResist(-1, DEREF_P_CHAR(pc), 4))
							{                                               
								Magic->MagicDamage(pc, mapitem->morex/300);
							}
							soundeffect2(DEREF_P_CHAR(pc), 2, 8);
						}
					}
					
					if ((mapitem->id1==0x39 && (mapitem->id2==0x15 || mapitem->id2==0x20)))
					{//Poison field
						if ((mapitem->pos.x == newx) && (mapitem->pos.y == newy) && (mapitem->pos.z==pc->pos.z))
						{
							if (!Magic->CheckResist(-1, DEREF_P_CHAR(pc), 5))
							{                                               
								Magic->PoisonDamage(DEREF_P_CHAR(pc),1);
							}
							soundeffect2(DEREF_P_CHAR(pc), 2, 8);
						}
					}
					
					else if ((mapitem->id1==0x39 && (mapitem->id2==0x79 || mapitem->id2==0x67)))
					{//Para Field
						if ((mapitem->pos.x == newx) && (mapitem->pos.y == newy) && (mapitem->pos.z==pc->pos.z))
						{
							if (!Magic->CheckResist(-1, DEREF_P_CHAR(pc), 6))
							{
								tempeffect(DEREF_P_CHAR(pc), DEREF_P_CHAR(pc), 1, 0, 0, 0);
							}
							soundeffect2(DEREF_P_CHAR(pc), 0x02, 0x04);
						}
					}
					else if (mapitem->id1<0x40)
					{
						// look for item triggers, this was moved from CrazyXYBlockStuff()
						if ((mapitem->pos.x== newx) && (mapitem->pos.y == newy))
						{
							if (mapitem->trigger!=0)
							{
								if ((mapitem->trigtype==1)&&(!pc->dead))
								{
									if (!mapitem->disabled)
									{
										Trig->triggerwitem(socket, mapitem, 1);  //When player steps on a trigger
									}
									else // see if disabled trigger can be re-enabled
									{
										if( ( mapitem->disabled != 0 ) && ( ( mapitem->disabled <= uiCurrentTime ) || (overflow)))
										{
											mapitem->disabled = 0;	// re-enable it
											Trig->triggerwitem( socket, mapitem, 1 );
										}
									}
								}
							}
						}
					}

					// always check for new items if we actually moved
					if (!amTurning)
					{
						// is the item a building on the BUILDRANGE?
						if ((mapitem->id1==0x40)&&(mapitem->id2>=0x7C)&&(mapitem->id2<=0x7E))
						{
							if ((abs(newx-mapitem->pos.x)==BUILDRANGE)||(abs(newy-mapitem->pos.y)==BUILDRANGE))
							{
								senditem(socket, mapitem);
							}
						}
						// otherwise if the item has just now become visible, inform the client about it
						else
						{
// Thyme
// Code Addition for Ab
// PLUS reduction in senditems for out of range objects!
// Stuff commented out is original code between BEGIN and END
// Thyme BEGIN
							signed int oldd = Distance(oldx, oldy, mapitem->pos.x, mapitem->pos.y);
							signed int newd = Distance(newx, newy, mapitem->pos.x, mapitem->pos.y);
							
//							if ((abs(newx-mapitem->pos.x) == visibleRange ) || (abs(newy-mapitem->pos.y) == visibleRange ))
							if (newd == visibleRange)
							{
								if( ( !mapitem->visible ) || ( ( mapitem->visible ) && ( currchar[socket]->isGM() ) ) )// we're a GM, or not hidden
									senditem(socket, mapitem);
							}
							if ( ( oldd == visibleRange ) && ( newd == ( visibleRange + 1 ) ) )
							{
								// item out of range for trigger
							}
// Thyme END
						}
					}
				}
			} // while (mapitem != NULL);
#pragma note("This while is strange, need some further investigation")
		}
	}
}

void cMovement::HandleTeleporters(P_CHAR pc, UOXSOCKET socket, short int oldx, short int oldy)
// PARAM WARNING: unreferenced paramater socket
{
	// now this is one wacky optimization. if we haven't moved don't do this
	// well, we wouldn't be in Walking() if we weren't trying to move!
	if ((pc->pos.x!=oldx)||(pc->pos.y!=oldy))
	{
		//    /*if (!(pc->dead))*/ objTeleporters(DEREF_P_CHAR(pc)); //morrolan
		if ( pc->isPlayer())
			objTeleporters( DEREF_P_CHAR(pc) );   // ripper
		teleporters( DEREF_P_CHAR(pc) );
	}
}


/********* start of LB's no rain & snow in buildings stuff ***********/
void cMovement::HandleWeatherChanges(P_CHAR pc, UOXSOCKET socket)
{
	if (pc->isPlayer() && online(DEREF_P_CHAR(pc))) // check for being in buildings (for weather) only for PC's
	{
		// ok, this is already a bug, because the new weather stuff doesn't use this global
		// for the weather.
		if (wtype!=0) // check only neccasairy if it rains or snows ...
		{
			int inDungeon = indungeon(DEREF_P_CHAR(pc)); // dung-check
			bool i = Map->IsUnderRoof(pc->pos.x, pc->pos.y, pc->pos.z); // static check
			// dynamics-check
			int x = Map->DynamicElevation(pc->pos.x, pc->pos.y, pc->pos.z);
			if (x!=illegal_z)
				if (Boats->GetBoat(pc) != NULL)
					x=illegal_z; // check for dynamic buildings except boats
			if (x==1)
				x = illegal_z; // 1 seems to be the multi-borders
			
			//printf("x: %i\n",x);
			// ah hah! this was a bug waiting to happen if not already, we have overloaded the use of the
			// variable k, which used to hold the socket
//			int k = noweather[socket];    
			if (inDungeon || i || x!= illegal_z )
				noweather[socket] = 1;
			else
				noweather[socket] = 0; // no rain & snow in static buildings+dungeons;
//			if (k - noweather[c] !=0)
//				weather(socket, 0); // if outside-inside changes resend weather ...
		}
	}
}

void cMovement::HandleGlowItems(P_CHAR pc, UOXSOCKET socket)
// PARAM WARNING: unreferenced paramater socket
{
	// i guess things only glow if you are online, i dunno what that means        
	if( online( DEREF_P_CHAR(pc) ))
	{
		vector<SERIAL> vecGlowItems = glowsp.getData(pc->serial);
		for( unsigned int ci = 0; ci < vecGlowItems.size(); ci++ )
		{
			P_ITEM pi = FindItemBySerial(vecGlowItems[ci]);
			if( pi != NULL )
			{
				if( !pi->free )
				{
					pc->glowHalo(pi);
				}
			}
		}
	}
}

// return whether someone is a GM Body
bool cMovement::IsGMBody(P_CHAR pc)
{
	if (
		((pc->isGM())) || // I got GM privs
        ((pc->id1==0x03)&&(pc->id2==0xDB)) ||//Gm
        ((pc->id1==0x01)&&(pc->id2==0x92)) ||//Ghosts
        ((pc->id1==0x01)&&(pc->id2==0x93))
        ) 
        return true;
    return false;
}


void cMovement::CombatWalk(P_CHAR pc) // Only for switching to combat mode
{
    for (unsigned int i=0;i<now;i++)
    {
		// moved perm[i] first since its much faster
        if ((perm[i]) && (inrange1p(DEREF_P_CHAR(pc), DEREF_P_CHAR(currchar[i]))))
        {
			P_CHAR pc_check = currchar[i];
            extmove[1] = pc->ser1;
            extmove[2] = pc->ser2;
            extmove[3] = pc->ser3;
            extmove[4] = pc->ser4;
            extmove[5] = pc->id1;
            extmove[6] = pc->id2;
            extmove[7] = (unsigned char)(pc->pos.x>>8);
            extmove[8] = (unsigned char)(pc->pos.x%256);
            extmove[9] = (unsigned char)(pc->pos.y>>8);
            extmove[10] = (unsigned char)(pc->pos.y%256);
            extmove[11] = pc->dispz;
            extmove[12] = (unsigned char)(pc->dir&0x7F);
            
			ShortToCharPtr(pc->skin, &extmove[13]);
            
            
            if (pc->war) extmove[15]=0x40; else extmove[15]=0x00;
            if (pc->hidden) extmove[15]=extmove[15]|0x80;
            if (pc->poisoned) extmove[15]=extmove[15]|0x04; //AntiChrist -- thnx to SpaceDog
            const int guild = Guilds->Compare( DEREF_P_CHAR(pc), DEREF_P_CHAR(currchar[i]) );
            const int race = Races.CheckRelation(pc,pc_check);
            if (pc->kills > repsys.maxkills ) extmove[16]=6; // ripper
            //if (pc->npcaitype==0x02) extmove[16]=6; else extmove[16]=1;
            //chars[i].flag=0x04;       // everyone should be blue on default
            else if (guild==1  || race==1)//Same guild (Green)
                extmove[16]=2;
            else if (guild==2 || race==2) // Enemy guild.. set to orange
                extmove[16]=5;
            //                  else if( !chars[i].npc && ( chars[i].priv&1 || chars[i].priv&80 ) )
            //                          extmove[16] = 7;
            else {
                switch(pc->flag)
                {//1=blue 2=green 5=orange 6=Red 7=Transparent(Like skin 66 77a)
                case 0x01: extmove[16]=6; break;// If a bad, show as red. 
                case 0x04: extmove[16]=1; break;// If a good, show as blue.
                case 0x08: extmove[16]=2; break; //green (guilds)
                case 0x10: extmove[16]=5; break;//orange (guilds)
                default:extmove[16]=3; break;//grey 
                }
            }
            
            if (!pc->war)
            {
                //                              pc->attacker=INVALID_SERIAL;
                pc->targ = INVALID_SERIAL;
            }
            Network->xSend(i, extmove, 17, 0);
        }
    }
}


void cMovement::NpcWalk(P_CHAR pc_i, int j, int type)   //type is npcwalk mode (0 for normal, 1 for box, 2 for circle)
{
	// sometimes the NPC movement code comes up with -1, for example, if we are following someone
	// and we are directly on top of them
	if (-1 == j) return;

    const short int x = pc_i->pos.x;
    const short int y = pc_i->pos.y;
//    const signed char z = pc_i->pos.z;

    // if we are walking in an area, and the area is not properly defined, just don't bother with the area anymore
    if( ((1 == type) && ( pc_i->fx1 == -1 || pc_i->fx2 == -1 || pc_i->fy1 == -1 || pc_i->fy2 == -1 ) ) ||
        ((2 == type) && ( pc_i->fx1 == -1 || pc_i->fx2 == -1 || pc_i->fy1 == -1)))
        // circle's don't use fy2, so don't require them! fur 10/30/1999
    {
        //printf("Rect/circle error!\n" );
        pc_i->npcWander = 2; // Wander freely from now on
        type = 0;
    }

    //Bug Fix -- Zippy
    if (pc_i->priv2&2)
		return;//Frozen - Don't send them al the way to walking to check this, just do it here.

// Thyme New Stuff 2000.09.21

//	if ( ( pc_i->dir & 0x07 ) == ( j & 0x07 ) )
//	{
		short int newx = GetXfromDir(j, x);
		short int newy = GetYfromDir(j, y);
// Take out the validNPCMove here... Not needed...If I fail, I don't try to walk somewhere else, so why should I
// check it twice? Just walk! Normal walking code will do the rest.
		if (
		    (!type)||
		    ((type==1)&&(checkBoundingBox(newx, newy, pc_i->fx1, pc_i->fy1, pc_i->fz1, pc_i->fx2, pc_i->fy2)))||
		    ((type==2)&&(checkBoundingCircle(newx, newy, pc_i->fx1, pc_i->fy1, pc_i->fz1, pc_i->fx2)))
		   )
			Walking(pc_i, j & 0x07, 256); // arm code
//	}

// need to add diagonal move checks to CanCharWalk...

// Thyme don't see the point in this... same as above
// else if (j<8)
//		Walking(i, j, 256);
}

// Function      : cMovement::GetYfromDir
// Written by    : Unknown
// Revised by    : Thyme
// Revision Date : 2000.09.15
// Purpose       : Return the new y from given dir

unsigned short cMovement::GetYfromDir(int dir, unsigned short y)
{

	switch ( dir & 0x07 )
	{
	case 0x00 :
	case 0x01 :
	case 0x07 :
		y--; break;
	case 0x03 :
	case 0x04 :
	case 0x05 :
		y++; break;
	}

    return y;

}

// Function      : cMovement::GetXfromDir
// Written by    : Unknown
// Revised by    : Thyme
// Revision Date : 2000.09.15
// Purpose       : Return the new x from given dir

unsigned short cMovement::GetXfromDir(int dir, unsigned short x)
{

   	switch ( dir & 0x07 )
	{
	case 0x01 :
	case 0x02 :
	case 0x03 :
		x++; break;
	case 0x05 :
	case 0x06 :
	case 0x07 :
		x--; break;
	}

    return x;

}


// Ok, I'm going to babble here, but here's my thinking process...
// Max Heuristic is 5 (for now) and I'm not concerned about walls... I'll add that later
// Easiest way I think would be for recursive call for now... Will change later if need be
// pathfind will call itself with new stuff... as long as the distance get's shorter
// We have to take into consideration if the NPC is going to a point or to a character
// if we don't want NPCs to walk over each other, this has to be known, because the NPC
// that's walking will never reach destination if it's another character.
// We must pass, that way if we get chardist=1 then we're ok.  We are basically searching
// for the shortest path, which is always a diagonal line, followed by lateral to target
// (barring obstacles) On calculation, for the FIRST step, we need to know if a character
// is there or not, then after that no biggie because if so we can just recalc the path if
// something is blocking. If we don't check on that first step, NPCs will get stuck behind horses
// and stuff... Kinda exploitable if I'm on a horse attacking, then step off and behind to hide
// while I heal. The first thing we need to do when walking is determine if i'm blocked... then
// if I'm an NPC, recalculate my path and step... I'm also gonna take out the path structure
// in chars_st... all we need is to hold the directions, not the x and y... Hopefully this will
// save memory.


void cMovement::PathFind(P_CHAR pc, unsigned short gx, unsigned short gy)
{

	// Make sure this is a valid character before proceeding
	if ( pc == NULL ) return;

	// Make sure the character has taken used all of their previously saved steps
	if ( pc->pathnum < P_PF_MRV ) return;
    

	// Thyme 2000.09.21
	// initial rewrite of pathfinding...

//	const signed char z = pc->pos.z;
	signed int newx, newy;
	signed char newz;
	signed int oldx = pc->pos.x;
	signed int oldy = pc->pos.y;
	path_st newpath[P_PF_MIR];
	pc->pathnum=0;

	for ( int pn = 0 ; pn < P_PF_MRV ; pn++ )
	{
		newpath[pn].x = newpath[pn].y = 0;
		int pf_neg = ( ( rand() % 2 ) ? 1 : -1 );
		int pf_dir = Direction(oldx, oldy, gx, gy);
		for ( int i = 0 ; i < 8 ; i++ )
		{
			pf_neg *= -1;
			pf_dir += ( i * pf_neg );
			newx = GetXfromDir(pf_dir, oldx);
			newy = GetYfromDir(pf_dir, oldy);
			if (CanCharMove(pc, oldx, oldy, newz, pf_dir))
			{
				if ( ( pn < P_PF_MRV ) && CheckForCharacterAtXYZ(pc, newx, newy, newz) )
					continue;

				newpath[pn].x = oldx = newx;
				newpath[pn].y = oldy = newy;
				break;
			}
		}
		if ( ( newpath[pn].x == 0 ) && ( newpath[pn].y == 0 ) )
		{
			pc->pathnum = P_PF_MRV;
			break;
#if DEBUG_PATHFIND
printf("Character stuck!\n");
#endif
		}
	}

	for ( int i = 0 ; i < P_PF_MRV ; i++ )
	{
		pc->path[i].x = newpath[i].x;
		pc->path[i].y = newpath[i].y;
#if DEBUG_PATHFIND
		printf("PFDump: %s - %i) %ix, %iy\n",pc->name, i+1, pc->path[i].pos.x, pc->path[i].pos.y);
#endif
	}

}


//NEW NPCMOVEMENT ZIPPY CODE STARTS HERE -- AntiChrist meging codes --
void cMovement::NpcMovement(unsigned int currenttime, P_CHAR pc_i)//Lag fix
{
    register int k;
    
	int j = rand() % 40;

    int dnpctime=0;
    if (pc_i->isNpc() && (pc_i->npcmovetime<=currenttime||(overflow)))
    {
#if DEBUG_NPCWALK
		printf("ENTER (%s): %d AI %d WAR %d J\n", pc_i->name, pc_i->npcWander, pc_i->war, j);
#endif
		if (pc_i->war && pc_i->npcWander != 5)
        {
            P_CHAR pc_attacker = FindCharBySerial(pc_i->attacker);
            if (pc_attacker != NULL)
            {
                if ( chardist(DEREF_P_CHAR(pc_i), DEREF_P_CHAR(pc_attacker)) > 1 /* || chardir(i, l)!=chars[i].dir // by Thyme: causes problems, will fix */)
                {
                    if ( online( DEREF_P_CHAR(pc_attacker) ) || pc_attacker->isNpc() )
                    {
						PathFind(pc_i, pc_attacker->pos.x, pc_attacker->pos.y);
                        j = chardirxyz(DEREF_P_CHAR(pc_i), pc_i->path[pc_i->pathnum].x, pc_i->path[pc_i->pathnum].y);
                        if ( ( pc_i->dir & 0x07 ) == ( j & 0x07 ) ) pc_i->pathnum++;
                        Walking(pc_i, j, 256);
                    }
                }
				else
				{ // if I'm within distance, clear my path... for attacking only.
					pc_i->pathnum += P_PF_MRV;
				}
	        }
        } // end of if l!=-1
        else
        {
            switch(pc_i->npcWander)
            {
            case 0: // No movement
                break;
            case 1: // Follow the follow target
				{
					P_CHAR pc_target = FindCharBySerial(pc_i->ftarg);
	                if (pc_target == NULL) return;
		            if ( online(DEREF_P_CHAR(pc_target)) || pc_target->isNpc() )
			        {
				        if ( chardist(DEREF_P_CHAR(pc_i), DEREF_P_CHAR(pc_target)) > 1 /* || chardir(i, k)!=chars[i].dir // by THyme: causes problems, will fix */)
					    {
						    PathFind(pc_i, pc_target->pos.x, pc_target->pos.y);
	                        j=chardirxyz(DEREF_P_CHAR(pc_i), pc_i->path[pc_i->pathnum].x, pc_i->path[pc_i->pathnum].y);
		                    pc_i->pathnum++;
			                Walking(pc_i,j,256);
				        }
						// Dupois - Added April 4, 1999
						// Has the Escortee reached the destination ??
						if( ( !pc_target->dead ) && ( pc_i->questDestRegion == pc_i->region ) )
						{
							// Pay the Escortee and free the NPC
							MsgBoardQuestEscortArrive( DEREF_P_CHAR(pc_i), calcSocketFromChar( DEREF_P_CHAR(pc_target) ) );
						}
					// End - Dupois
	                }
				}
                break;
            case 2: // Wander freely, avoiding obstacles.
                if (j<8 || j>32) dnpctime=5;
                if (j>7 && j<33) // Let's move in the same direction lots of the time.  Looks nicer.
                    j=pc_i->dir;
                NpcWalk(pc_i,j,0);
                break;
            case 3: // Wander freely, within a defined box
                if (j<8 || j>32) dnpctime=5;
                if (j>7 && j<33) // Let's move in the same direction lots of the time.  Looks nicer.
                    j=pc_i->dir;
                
                NpcWalk(pc_i,j,1);
                break;
            case 4: // Wander freely, within a defined circle
                if (j<8 || j>32) dnpctime=5;
                if (j>7 && j<33) // Let's move in the same direction lots of the time.  Looks nicer.
                    j=pc_i->dir;
                NpcWalk(pc_i,j,2);
                break;
            case 5: //FLEE!!!!!!
				{
					P_CHAR pc_k = FindCharBySerial(pc_i->targ);
					if (pc_k == NULL) return;
					
					if ( chardist(DEREF_P_CHAR(pc_i), DEREF_P_CHAR(pc_k)) < P_PF_MFD )
					{
						// calculate a x,y to flee towards
						int mydist = P_PF_MFD - chardist(DEREF_P_CHAR(pc_i), DEREF_P_CHAR(pc_k)) + 1;
						j=chardirxyz(DEREF_P_CHAR(pc_i), pc_k->pos.x, pc_k->pos.y);
						short int myx = GetXfromDir(j, pc_i->pos.x);
						short int myy = GetYfromDir(j, pc_i->pos.y);
						
						short int xfactor = 0;
						short int yfactor = 0;
						
						if ( myx != pc_i->pos.x )
							if ( myx < pc_i->pos.x )
								xfactor = -1;
							else
								xfactor = 1;
							
							if ( myy != pc_i->pos.y )
								if ( myy < pc_i->pos.y )
									yfactor = -1;
								else
									yfactor = 1;
								
								myx += ( xfactor * mydist );
								myy += ( yfactor * mydist );
								
								// now, got myx, myy... lets go.
								
								PathFind(pc_i, myx, myy);
								j=chardirxyz(DEREF_P_CHAR(pc_i), pc_i->path[pc_i->pathnum].x, pc_i->path[pc_i->pathnum].y);
								pc_i->pathnum++;
								Walking(pc_i,j,256);
					}
					else
					{ // wander freely... don't just stop because I'm out of range.
						j=rand()%40;
						if (j<8 || j>32) dnpctime=5;
						if (j>7 && j<33) // Let's move in the same direction lots of the time.  Looks nicer.
							j=pc_i->dir;
						NpcWalk(pc_i,j,0);
					}
				}
				break;
            default:
                break;
                //printf("ERROR: Fallout of switch statement without default [%i]. walking.cpp, npcMovement2()\n",chars[i].npcWander); //Morrolan
			} // break; //Morrolan unnecessary ?
		}
		pc_i->npcmovetime=(unsigned int)(currenttime+double((NPCSPEED*CLOCKS_PER_SEC)/5)); //reset move timer
        //pc_i->npcmovetime=(unsigned int)(currenttime+double(NPCSPEED*CLOCKS_PER_SEC*(1+dnpctime))); //reset move timer
    }
}


// This is my attempt at the writing a more effective pathfinding algorithm/sequence

// Function      : cMovement::Distance
// Written by    : Thyme
// Revision Date : 2000.09.08
// Purpose       : Calculate the shortest walkable distance between two points.
// Method        : The methodology behind this is pretty simple actually. The shortest distance
// between two walkable points would be to walk a diagonal line until sx=dx or sy=dy
// and then follow the line until the goal is reached. Since a diagonal step is
// the same distance as a lateral step, there's no need to use the Pythagorean theorem
// in the calculation. This calculation does not take into account any blocking objects.
// It will be used as a heuristic in determining priority of movement.

short int cMovement::Distance(short int sx, short int sy, short int dx, short int dy)
{
	return ( ( abs(sx-dx) < abs(sy-dy) ) ? abs(sy-dy) : abs(sx-dx) );
}

// Function      : cMovement::CanCharWalk()
// Written by    : Dupois
// Revised by    : Thyme
// Revision Date : 2000.09.17
// Purpose       : Check if a character can walk to a specified x,y location
// Method        : Modified the old CheckWalkable function so that it can be utilized throughout
// the walking code. Ever wonder why NPCs can walk through walls and stuff in combat mode? This
// is the fix, plus more.

short int cMovement::Direction(short int sx, short int sy, short int dx, short int dy)
{
	
	int dir,xdif,ydif;
	
	xdif = dx - sx;
	ydif = dy - sy;
	
	if ((xdif==0)&&(ydif<0)) dir=0;
	else if ((xdif>0)&&(ydif<0)) dir=1;
	else if ((xdif>0)&&(ydif==0)) dir=2;
	else if ((xdif>0)&&(ydif>0)) dir=3;
	else if ((xdif==0)&&(ydif>0)) dir=4;
	else if ((xdif<0)&&(ydif>0)) dir=5;
	else if ((xdif<0)&&(ydif==0)) dir=6;
	else if ((xdif<0)&&(ydif<0)) dir=7;
	else dir=-1;
	
	return dir;
}

bool cMovement::CanCharWalk(P_CHAR pc, short int x, short int y, signed char &z)
{

	const signed char oldZ = pc->pos.z;
	signed char nNewZ = illegal_z;
	short int MoveType = CheckMovementType(pc);
	bool blocked = false;

	for ( int cnt = 0; cnt < 3 ; cnt++ )
	{
		int xycount = 0;
		unitile_st xyblock[XYMAX];

		switch (cnt)
		{
		case 0:
			GetBlockingMap(x, y, xyblock, xycount);
			break;
		case 1:
			GetBlockingStatics(x, y, xyblock, xycount);
			break;
		case 2:
			GetBlockingDynamics(x, y, xyblock, xycount);
			break;
		default:
#if DEBUG_WALK_ERROR
			printf("IMPOSSIBLE!\n");
#endif
			break;
		}

// Insert Knoxos code
// Thyme: Shortened up and modified to my needs
	// knoxos
	// Work our way through the blockables item array
		int i;
		for(  i = 0; i < xycount; i++ )
		{
		    unitile_st *thisblock = &xyblock[i]; // this is a easy/little tricky, to save a little calculation
		                                     // since the [i] is calclated several times below
			                                 // if it doesn't help, it doesn't hurt either.
			signed char nItemTop = thisblock->basez + thisblock->height; // Calculate the items total height

	    // check if the creature is floating on a static (keeping Z or falling)
			if ( ( nItemTop >= nNewZ ) &&
				 ( ( ( nItemTop <= oldZ ) && ( abs(oldZ - nItemTop) <= P_M_MAX_Z_FALL ) ) ||
				 ( ( nItemTop >= oldZ ) && ( nItemTop < oldZ + P_M_MAX_Z_CLIMB ) ) ) )
			{
				if ( ( MoveType & P_C_IS_GM_BODY ) && ( CanGMWalk(xyblock[i]) ) )
					nNewZ = nItemTop;
				if ( ( MoveType & P_C_IS_PLAYER ) && ( CanPlayerWalk(xyblock[i]) ) )
					nNewZ = nItemTop;
				if ( ( MoveType & P_C_IS_FISH ) && ( CanFishWalk(xyblock[i]) ) )
					nNewZ = nItemTop;
				if ( ( MoveType & P_C_IS_NPC ) && ( CanNPCWalk(xyblock[i]) ) )
					nNewZ = nItemTop;
				if ( ( MoveType & P_C_IS_BIRD ) && ( CanBirdWalk(xyblock[i]) ) )
					nNewZ = nItemTop;
			}
		}

#if DEBUG_WALK
		printf( "CheckWalkable calculate Z=%s %d\n", pc->name, nNewZ );
#endif

		// now the new Z-cordinate of creature is known, 
		// check if it hits it's head against something (blocking in other words)
		for(i = 0; i < xycount; i++)
		{
			unitile_st *thisblock = &xyblock[i]; 
			signed char nItemTop = thisblock->basez + thisblock->height; // Calculate the items total height
			if ((nItemTop >= nNewZ) && (thisblock->basez <= nNewZ + P_M_MAX_Z_INFLUENCE))
			{ // in effact radius?
				if ( MoveType & P_C_IS_GM_BODY )
					continue;
				if ( ( MoveType & P_C_IS_FISH ) && ( CanFishWalk(xyblock[i]) ) )
					continue;
				if ( ( MoveType & P_C_IS_BIRD ) && ( CanBirdWalk(xyblock[i]) ) )
					continue;
				if ( thisblock->flag1 & 0x40 )
				{   // blocking
					nNewZ = illegal_z;
#if DEBUG_WALK
			printf( "CheckWalkable blocked due to tile=%d at height=%d.\n", xyblock[i].id, xyblock[i].basez);
#endif
					blocked = true;
					break;
				}
			}
    // knoxos: MAX_ITEM_Z_INFLUENCE is nice, but not truely correct,
    //         since the creature height should be the effect radius, if you are i.e.
    //         polymorphed to a "slime", you could go through things you normally 
    //         wouldn't get under. (Just leaves the question what happens if you
    //         unpolymorph in a place where you can't fit, lucky there are no
    //         such gaps or tunnels in Britannia). 
    //         (Well UO isn't ment to really think in 3d)
    // Thyme : He's right... Should be based of character's height.
		}

		if (blocked)
			break;
	
	}
// end knoxos code
	z = nNewZ;

#if DEBUG_WALK
	printf("CanCharWalk: %dx %dy %dz\n", x, y, z);
#endif

	if ( nNewZ == illegal_z )
		return false;
	else
		return true;
}

// Function      : cMovement::CanCharMove()
// Written by    : Thyme
// Revision Date : 2000.09.17
// Purpose       : Check if a character can walk to a from x,y to dir direction
// Method        : This handles the funky diagonal moves.

bool cMovement::CanCharMove(P_CHAR pc, short int x, short int y, signed char &z, int dir)
{
	z = illegal_z;

	if ( ( dir & 0x07 ) % 2 )
	{ // check three ways.
		if ( ! CanCharWalk(pc, GetXfromDir(dir - 1, x), GetYfromDir(dir - 1, y), z) )
			return false;
		if ( ! CanCharWalk(pc, GetXfromDir(dir + 1, x), GetYfromDir(dir + 1, y), z) )
			return false;
	}

	return CanCharWalk(pc, GetXfromDir(dir, x), GetYfromDir(dir, y), z);
}


void cMovement::deny( UOXSOCKET k, P_CHAR pc, int sequence )
{
	char walkdeny[9] = "\x21\x00\x01\x02\x01\x02\x00\x01";
	
	walkdeny[1] = sequence;
	walkdeny[2] = pc->pos.x>>8;
	walkdeny[3] = pc->pos.x%256;
	walkdeny[4] = pc->pos.y>>8;
	walkdeny[5] = pc->pos.y%256;
	walkdeny[6] = pc->dir;
	walkdeny[7] = pc->dispz;
	Network->xSend( k, walkdeny, 8, 0 );
	walksequence[k] = -1;
}


//o-------------------------------------------------------------o
//| Function : calcTileHeight
//| Author   : knoxos
//o-------------------------------------------------------------o
//| Description : 
//|   Out of some strange reason the tile height seems
//|   to be an absolute value of a two's complement of 
//|   the last four bit. Don't know if it has a special
//|   meaning if the tile is height is "negative"
//|
//|   (stairs in despise blocking bug)
//| Arguments :
//|   int h   orignial height as safed in mul file
//|
//| Return code:
//|   The absoulte value of the two's complement if the
//|   the value was "negative"
//o-------------------------------------------------------------o

#define MAX_ITEM_Z_INFLUENCE 10 // Any item above this height is discarded as being too far away to effect the char
#define MAX_Z_LEVITATE 10			// Maximum total height to reach a tile marked as 'LEVITATABLE'
												// Items with a mark as climbable have no height limit


inline signed int higher( signed int a, signed int b )
{
	if( a < b )
		return b;
	else
		return a;
}
inline signed int LOWER( signed int a, signed int b )
{
	if( a < b )
		return a;
	else
		return b;
}
inline unsigned int turn_clock_wise( unsigned int dir ) throw()
{
	unsigned int t = (dir - 1) & 7;
	return (dir & 0x80) ? ( t | 0x80) : t;
}

/*inline unsigned int turn_counter_clock_wise( unsigned int dir )
{
	return ( dir & 0x80 ) | ( ( dir - 1 ) & 7 );*/

inline int calcTileHeight( int h ) throw()
{
  ///return ((h & 0x8) ? (((h & 0xF) ^ 0xF) + 1) : h & 0xF);
	//return (h & 0x7);
	//return ((h & 0x8) ? (((h & 0xF) ^ 0xF) + 1) : h & 0xF);
	return ((h & 0x8) ? ((h & 0xF) >> 1) : h & 0xF);
} 


/********************************************************
  Function: cMovement::CheckWalkable

  Description: (knoxos)
    Rewritten checkwalk-function, it calculates new z-position
    for a walking creature (PC or NPC) walks, and checks if 
    movement is blocked.

    This function takes a little more calculation time, than the
    last one, since it walks two times through the static-tile set.
    However at least this one is (more) correct, and these VERy guys 
    now hit their noses on the walls.

    In principle it is the same as the World-kernel in UMelange.
   
  Parameters:  
    CHARACTER c           Character's index in chars[]
	int x, y			  new cords.
	int oldx, oldy		  old cords.
	bool justask		  don't make any changes, the func. is just asked
						  "what if"..

  Return code:
    new z-value        if not blocked
    invalid_z == -128, if walk is blocked

********************************************************/
int cMovement::calc_walk(P_CHAR pc, unsigned int x, unsigned int y, unsigned int oldx, unsigned int oldy, bool justask )
{
	const signed int oldz = pc->pos.z;
	bool may_levitate = pc->may_levitate;
	bool on_ladder = false;
//	bool climbing = false;
	signed int newz = illegal_z;
//	short int MoveType = CheckMovementType( pc );
	bool blocked = false;
	int ontype = 0;

	int xycount = 0;
	unitile_st xyblock[XYMAX];
	GetBlockingMap( x, y, xyblock, xycount );
	GetBlockingStatics( x, y, xyblock, xycount );
	GetBlockingDynamics( x, y, xyblock, xycount );

	int i;
	// first calculate newZ value
	for( i = 0; i < xycount; i++ )
	{
		unitile_st *thisblock = &xyblock[i]; // this is a easy/little tricky, to save a little calculation
		                                 // since the [i] is calclated several times below
			                             // if it doesn't help, it doesn't hurt either.
		signed int nItemTop = thisblock->basez + ((xyblock[i].type == 0) ? xyblock[i].height : calcTileHeight(xyblock[i].height)); // Calculate the items total height

		// check if the creature is floating on a static (keeping Z or falling)
		if( ( nItemTop >= newz ) && ( nItemTop <= oldz ) )
		{
			if( thisblock->flag2 & 0x02 )
			{ // walkable tile
				newz = nItemTop;
				ontype = thisblock->type;
				if( thisblock->flag4 == 0x80 ) { // if it was ladder the char is allowed to `levitate´ next move
					on_ladder = true;
				}
				continue;
			}
		}

		// So now comes next step, levitation :o)
		// you can gain Z to a limited amount if yo uwere climbing on last move on a ladder
		if( ( nItemTop >= newz ) && ( may_levitate ) && ( nItemTop <= oldz + MAX_Z_LEVITATE ) &&
			( thisblock->flag2 & 0x02 )
			)
		{
			ontype = thisblock->type;
			newz = nItemTop;
			if( thisblock->flag4 == 0x80 ) { // if it was ladder the char is allowed to `levitate´ next move
				on_ladder = true;
			}
		}
		// check if the creature is climbing on a climbable Z
		// (gaining Z through stairs, ladders, etc)
		// This form has no height limit, and the tile bottom must start lower or
		// equal current height + levitateable limit
		if( ( nItemTop >= newz ) && ( thisblock->basez <= oldz + MAX_Z_LEVITATE ) )
		{
			if( (thisblock->flag2 & 0x04) || ( thisblock->type == 0 ) || // Climbable tile, map tiles are also climbable
			( (thisblock->flag1 == 0) && (thisblock->flag2 == 0x22) ) || // These are a special kind of tiles where OSI forgot 
																		 // to set the climbable flag
			( (nItemTop >= oldz && nItemTop <= oldz + 3) && (thisblock->flag2 & 0x02) )		 // Allow to climb a height of 1 even if the climbable flag is not set
																		 // so you can walkupon mushrooms, grasses or so with height of 1
																		 // if it is a walkable tile of course
			)
			{                 
				ontype = thisblock->type;
				newz = nItemTop;
				if( thisblock->flag4 == 0x80 ) 	{ // if it was ladder the char is allowed to `levitate´ next move
					on_ladder = true;
				}
			}
		}
	}

#if DEBUG_WALKING
		ConOut( "CheckWalkable calculate Z=%d\n", newz );
#endif
        int item_influence = higher( newz + MAX_ITEM_Z_INFLUENCE, oldz );
		// also take care to look on all tiles the creature has fallen through
		// (npc's walking on ocean bug)
		// now the new Z-cordinate of creature is known, 
		// check if it hits it's head against something (blocking in other words)
		bool isGM = IsGMBody( pc );
		for(i = 0; i < xycount; i++)
		{
			unitile_st *thisblock = &xyblock[i]; 
			signed int nItemTop = thisblock->basez + ((xyblock[i].type == 0) ? xyblock[i].height : calcTileHeight(xyblock[i].height)); // Calculate the items total height
			unsigned char flag1 = thisblock->flag1;
			unsigned char flag2 = thisblock->flag2;
			unsigned char flag4 = thisblock->flag4;
		// yeah,yeah,  this if has grown more ugly than the devil hismelf...
		if( ( (flag1 & 0x40) ||                                                  // a normal blocking tile      
			((flag2 & 0x02) && (nItemTop > newz))                                // staircases don't have blocking set, so very guy's could walk into them without this check.
			) &&                                                                 //   but one can walk upon them!   
			!( ( isGM || pc->dead ) && ((flag2 & 0x10) || (flag4 & 0x20))   // ghosts can walk through doors
			) 
			) {                                                                    // blocking
				if ((nItemTop > newz) && (thisblock->basez <= item_influence ) ||
				((nItemTop == newz) && (ontype == 0))
				) 
					{ // in effact radius?
                        newz = illegal_z;
#if DEBUG_WALKING
						ConOut( "CheckWalkable blocked due to tile=%d at height=%d.\n", xyblock[i].id, xyblock[i].basez );
#endif
						blocked = true;
                        break;
                    }
			}
    // knoxos: MAX_ITEM_Z_INFLUENCE is nice, but not truely correct,
    //         since the creature height should be the effect radius, if you are i.e.
    //         polymorphed to a "slime", you could go through things you normally 
    //         wouldn't get under. (Just leaves the question what happens if you
    //         unpolymorph in a place where you can't fit, lucky there are no
    //         such gaps or tunnels in Britannia). 
    //         (Well UO isn't ment to really think in 3d)
		}
		//}
// end knoxos code
#if DEBUG_WALK
	clConsole.send("CanCharWalk: %dx %dy %dz\n", x, y, z);
#endif
	if( (newz > illegal_z) && (!justask)) {
		// save information if we have climbed on last move.
		pc->may_levitate = on_ladder;
	}
	return newz;
}

// knox, reinserted it since some other files access it,
//       100% sure this is wrong, however the smaller ill.
int cMovement::validNPCMove( short int x, short int y, signed char z, CHARACTER s )
{
	const int getcell=mapRegions->GetCell(x,y);

	P_CHAR pc_s = MAKE_CHARREF_LRV(s, 0);

    pc_s->blocked++;
	vector<SERIAL> vecEntries = mapRegions->GetCellEntries(getcell);
    for ( unsigned int k = 0; k < vecEntries.size(); k++)
    {
		P_ITEM mapitem = FindItemBySerial(vecEntries[k]);
        if (mapitem != NULL)
        {
		    tile_st tile;
            Map->SeekTile(mapitem->id(), &tile);
            if (mapitem->pos.x==x && mapitem->pos.y==y && mapitem->pos.z+tile.height>z+1 && mapitem->pos.z<z+MaxZstep)
            {
                // bugfix found by JustMichael, moved by crackerjack
                // 8/2/99 makes code run faster too - one less loop :)
                if (mapitem->id()==0x3946 || mapitem->id()==0x3956) return 0;
                if (mapitem->id1<=2 || (mapitem->id()>=0x0300 && mapitem->id()<=0x03E2)) return 0;
                if (mapitem->id()>0x0854 && mapitem->id()<0x0866) return 0;
                
                if (mapitem->type==12)
                {
                    if (pc_s->isNpc() && (strlen(pc_s->title) > 0 || pc_s->npcaitype != 0))
                    {                            
                        // clConsole.send("doors!!!\n");
                        dooruse(-1, mapitem);
                        
                    }                                   
                    pc_s->blocked=0;
                    return 0;
                }
                
            }
        }
    }

	// experimental check for bad spawning/walking places, not optimized in any way (Duke, 3.9.01)
	int mapid = 0;
	signed char mapz = Map->MapElevation(x, y);	// just to get the map-ID
	if (mapz != illegal_z)
	{
		if ((mapid >= 0x25A && mapid <= 0x261) ||	// cave wall
			(mapid >= 0x266 && mapid <= 0x26D) ||	// cave wall
			(mapid >= 0x2BC && mapid <= 0x2CB) )	// cave wall
			return 0;
		if ( mapid >= 0x0A8 && mapid <= 0x0AB) 	// water (ocean ?)
			return 0;
//		land_st land;
//		Map->SeekLand(mapid, &land);
	}
		
    // see if the map says its ok to move here
    if (Map->CanMonsterMoveHere(x, y, z))
    {
		pc_s->blocked = 0;
		return 1;
    }
    return 0;
}


// Static Members
void cMovement::getXYfromDir(int dir, int *x, int *y)
{
	switch(dir&0x07)
	{
	case 0: (*y)--;				break;
	case 1: (*x)++; (*y)--;		break;
	case 2: (*x)++;				break;
	case 3: (*x)++; (*y)++;		break;
	case 4: (*y)++;				break;
	case 5: (*x)--; (*y)++;		break;
	case 6: (*x)--;				break;
	case 7: (*x)--; (*y)--;		break;
	}
}

