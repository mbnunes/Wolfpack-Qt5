//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
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

#include "ai.h"
#include "world.h"
#include "defines.h"
#include "npc.h"
#include "basechar.h"
#include "player.h"
#include "mapobjects.h"
#include "network.h"
#include "network/uosocket.h"
#include "inlines.h"
#include "walking.h"
#include "prototypes.h"
#include "itemid.h"
#include "coord.h"
#include "srvparams.h"

// library includes
#include <math.h>

cNPC_AI::~cNPC_AI()
{
	delete currentState;
}

void cNPC_AI::setNPC( P_NPC npc )
{
	m_npc = npc;
	if( currentState )
	{
		currentState->npc = npc;
		if( currentState->nextState )
			currentState->nextState->npc = npc;
	}
}

void cNPC_AI::updateState()
{
	if( currentState->nextState != currentState )
	{
		AbstractState* temp = currentState->nextState;
		delete currentState;
		currentState = temp;
		currentState->init();
	}
}

void cNPC_AI::updateState( AbstractState* newState )
{
	if( newState )
	{
		if( currentState )
			delete currentState;
		currentState = newState;
		currentState->setInterface( this );
		currentState->npc = m_npc;
		currentState->init();
	}
}

void Actions::reattack()
{
	P_CHAR pAttacker = World::instance()->findChar( npc->attackerSerial() );
	if( pAttacker )
	{
		npc->fight( pAttacker );
		npc->emote( tr( "*You see %1 attacking %2*" ).arg( npc->name() ).arg( pAttacker->name() ) );
	}
}

void Actions::attack()
{
	P_CHAR pVictim = World::instance()->findChar( npc->combatTarget() );
	if( pVictim )
	{
		npc->fight( pVictim );
		npc->emote( tr( "*You see %1 attacking %2*" ).arg( npc->name() ).arg( pVictim->name() ) );
	}
}

void Actions::wanderFreely()
{
	switch( npc->wanderType() )
	{
	case enHalt:
		// do nothing
		break;
	case enFreely:
	{
		UINT8 dir = npc->direction();
		if( RandomNum(0, 100) < 20 )
			dir = RandomNum( 0, 7 );

		npc->setDirection( dir );
		Movement::instance()->Walking( npc, dir, 0xFF );
		break;
	}
	case enRectangle:
	{
		// get any point out of the rectangle and calculate the direction to it
		UINT16 rndx = RandomNum( npc->wanderX1(), npc->wanderX2() );
		UINT16 rndy = RandomNum( npc->wanderY1(), npc->wanderY2() );
		
		UINT8 dir = chardirxyz( npc, rndx, rndy );
		npc->setDirection( dir );
		Movement::instance()->Walking( npc, dir, 0xFF );
		break;
	}
	case enCircle:
	{
		Coord_cl pos = npc->pos();
		pos.x = npc->wanderX1();
		pos.y = npc->wanderY1();
		// get any point within the circle and calculate the direction to it
		// first a random distance which can be max. the length of the radius
		float rnddist = (float)RandomNum( 1, npc->wanderRadius() );
		// now get a point on this circle around the npc
		float rndphi = (float)RandomNum( 0, 100 ) / 100.0f * 2.0f * 3.14159265358979323846f;
		pos.x = pos.x + (INT16)floor( cos( rndphi ) * rnddist );
		pos.y = pos.y + (INT16)floor( sin( rndphi ) * rnddist );

		UINT8 dir = chardirxyz( npc, pos.x, pos.y );
		npc->setDirection( dir );
		Movement::instance()->Walking( npc, dir, 0xFF );
		break;	
	}
	};
}

void Actions::moveTo( const Coord_cl &pos )
{
	// simply move towards the target
	UINT8 dir = chardirxyz( npc, pos.x, pos.y );
	Coord_cl newPos = Movement::instance()->calcCoordFromDir( dir, npc->pos() );
	if( !mayWalk( npc, newPos ) )
	{
		if( dir == 7 )
			dir = 0;
		else
			dir++;

		newPos = Movement::instance()->calcCoordFromDir( dir, npc->pos() );
		if( !mayWalk( npc, newPos ) )
		{
			if( dir == 0 )
				dir = 6;
			else if( dir == 1 )
				dir = 7;
			else
				dir = dir - 2;

			newPos = Movement::instance()->calcCoordFromDir( dir, npc->pos() );
			if( !mayWalk( npc, newPos ) )
			{
				return;
			}
		}
	}

	npc->setDirection( dir );
	Movement::instance()->Walking( npc, dir, 0xFF );
}

void Actions::movePath( const Coord_cl &pos )
{
	if( !waitForPathCalculation )
	{
		UINT8 range = 1;
		if( npc->rightHandItem() && IsBowType( npc->rightHandItem()->id() ) )
			range = ARCHERY_RANGE;

		npc->findPath( pos, range == 1 ? 1.5f : (float)range );
		// dont return here!
	}
	else
	{
		waitForPathCalculation--;
		moveTo( pos );
		return;
	}

	if( npc->hasPath() )
	{
		waitForPathCalculation = 0;
		Coord_cl nextmove = npc->nextMove();
		UINT8 dir = chardirxyz( npc, nextmove.x, nextmove.y );
		npc->setDirection( dir );
		Movement::instance()->Walking( npc, dir, 0xFF );
		npc->popMove();
		return;
	}
	else
	{
		waitForPathCalculation = 3;
		moveTo( pos );
		return;
	}
}

/*
Precondition: npc has path!
*/
void Actions::movePath()
{
	Coord_cl nextmove = npc->nextMove();
	UINT8 dir = chardirxyz( npc, nextmove.x, nextmove.y );
	npc->setDirection( dir );
	Movement::instance()->Walking( npc, dir, 0xFF );
	npc->popMove();
	return;
}

void Actions::callGuards()
{
	npc->talk( tr("Guards! Help me!") );
	npc->callGuards();
}

void AbstractState_Wander::execute()
{
	// wander freely
	wanderFreely();
}

void AbstractState_Combat::execute()
{
	P_CHAR pTarget = World::instance()->findChar( npc->combatTarget() );
	if( !pTarget )
	{
		won();
		return;
	}

	UINT8 range = 1;
	if( npc->rightHandItem() && IsBowType( npc->rightHandItem()->id() ) )
		range = ARCHERY_RANGE;

	if( !npc->inRange( pTarget, range ) )
	{ // move towards the target
		if( SrvParams->pathfind4Combat() )
			movePath( pTarget->pos() );
		else
			moveTo( pTarget->pos() );
	}
}

void AbstractState_Flee::execute()
{
	if( !npc->hasPath() )
	{
		Coord_cl newPos = npc->pos();
		// find a valid spot in a circle of flee_radius fields to move to
		float rnddist = (float)RandomNum( 1, SrvParams->pathfindFleeRadius() );
		// now get a point on this circle around the npc
		float rndphi = (float)RandomNum( 0, 100 ) / 100.0f * 2.0f * 3.14159265358979323846f;
		newPos.x = newPos.x + (INT16)floor( cos( rndphi ) * rnddist );
		newPos.y = newPos.y + (INT16)floor( sin( rndphi ) * rnddist );

		// we use pathfinding for fleeing
		movePath( newPos );
	}
	else
		movePath();
}

