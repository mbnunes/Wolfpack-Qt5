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

static cNPC_AI* productCreator_MAL0()
{
	return new Monster_Aggressive_L0();
}

void Monster_Aggressive_L0::registerInFactory()
{
	AIFactory::instance()->registerType("Monster_Aggressive_L0", productCreator_MAL0);

	// register its states
	Monster_Aggr_L0_Wander::registerInFactory();
	Monster_Aggr_L0_Combat::registerInFactory();
}

Monster_Aggressive_L0::Monster_Aggressive_L0()
{ 
	currentState = new Monster_Aggr_L0_Wander( this, m_npc );
}

Monster_Aggressive_L0::Monster_Aggressive_L0( P_NPC currnpc )
{ 
	setNPC( currnpc ); 
	currentState = new Monster_Aggr_L0_Wander( this, m_npc );
}

void Monster_Aggressive_L0::eventHandler()
{
	currentState->nextState = currentState;
	if( !m_npc->isDead() )
	{
		P_CHAR pVictim = World::instance()->findChar( m_npc->combatTarget() );
		if( !pVictim  || pVictim->isDead() )
			currentState->won();
		else if( !pVictim->inRange( m_npc, VISRANGE ) )
			currentState->combatCancelled();

		P_CHAR pAttacker = World::instance()->findChar( m_npc->attackerSerial() );
		if( pAttacker && pAttacker->inRange( m_npc, VISRANGE ) )
			currentState->attacked();
		else
		{
			// get the first best character and attack it
			RegionIterator4Chars ri( m_npc->pos(), VISRANGE );
			for( ri.Begin(); !ri.atEnd(); ri++ )
			{
				P_CHAR pChar = ri.GetData();
				if( pChar && pChar != m_npc && !pChar->isInvulnerable() && !pChar->isHidden() && !pChar->isInvisible() && !pChar->isDead() )
				{
					P_PLAYER pPlayer = dynamic_cast<P_PLAYER>(pChar);
					if( pPlayer && pPlayer->isGMorCounselor() )
						continue;

					currentState->foundVictim( pChar );
					break;
				}
			}
		}

		updateState();
		currentState->execute();
	}
}

static AbstractState* productCreator_MAL0_Wander()
{
	return new Monster_Aggr_L0_Wander();
}

void Monster_Aggr_L0_Wander::registerInFactory()
{
	StateFactory::instance()->registerType("Monster_Aggr_L0_Wander", productCreator_MAL0_Wander);
}

void Monster_Aggr_L0_Wander::attacked()
{
	reattack();
	nextState = new Monster_Aggr_L0_Combat( m_interface, npc );
}

void Monster_Aggr_L0_Wander::foundVictim( P_CHAR pVictim )
{
	if( pVictim )
		npc->setCombatTarget( pVictim->serial() );
	attack();
	nextState = new Monster_Aggr_L0_Combat( m_interface, npc );
}

void Monster_Aggr_L0_Wander::execute()
{
	// wander freely
	wanderFreely();
}

static AbstractState* productCreator_MAL0_Combat()
{
	return new Monster_Aggr_L0_Combat();
}

void Monster_Aggr_L0_Combat::registerInFactory()
{
	StateFactory::instance()->registerType("Monster_Aggr_L0_Combat", productCreator_MAL0_Combat);
}

void Monster_Aggr_L0_Combat::won()
{
	npc->setAttackerSerial( INVALID_SERIAL );
	npc->setCombatTarget( INVALID_SERIAL );
	nextState = new Monster_Aggr_L0_Wander( m_interface, npc );
}

void Monster_Aggr_L0_Combat::combatCancelled()
{
	npc->setAttackerSerial( INVALID_SERIAL );
	npc->setCombatTarget( INVALID_SERIAL );
	nextState = new Monster_Aggr_L0_Wander( m_interface, npc );
}

void Monster_Aggr_L0_Combat::execute()
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


static cNPC_AI* productCreator_MAL1()
{
	return new Monster_Aggressive_L1();
}

void Monster_Aggressive_L1::registerInFactory()
{
	AIFactory::instance()->registerType("Monster_Aggressive_L1", productCreator_MAL1);

	// register its states
	// uses L0_Wander, registered above
	Monster_Aggr_L1_Wander::registerInFactory();
	Monster_Aggr_L1_Combat::registerInFactory();
	Monster_Aggr_L1_Flee::registerInFactory();
}

Monster_Aggressive_L1::Monster_Aggressive_L1()
{ 
	currentState = new Monster_Aggr_L1_Wander( this, m_npc );
}

Monster_Aggressive_L1::Monster_Aggressive_L1( P_NPC currnpc )
{ 
	setNPC( currnpc );  
	currentState = new Monster_Aggr_L1_Wander( this, m_npc );
}

void Monster_Aggressive_L1::eventHandler()
{
	currentState->nextState = currentState;
	if( !m_npc->isDead() )
	{
		P_CHAR pVictim = World::instance()->findChar( m_npc->combatTarget() );
		if( !pVictim || pVictim->isDead() )
			currentState->won();
		else if( !pVictim->inRange( m_npc, VISRANGE ) )
			currentState->combatCancelled();

		if( m_npc->hitpoints() < (float)m_npc->criticalHealth() * 0.01f * m_npc->maxHitpoints() )
			currentState->hitpointsCritical();
		else
			currentState->hitpointsRestored();

		P_CHAR pAttacker = World::instance()->findChar( m_npc->attackerSerial() );
		if( pAttacker && pAttacker->inRange( m_npc, VISRANGE ) )
			currentState->attacked();
		else
		{
			// get the first best character and attack it
			RegionIterator4Chars ri( m_npc->pos(), VISRANGE );
			for( ri.Begin(); !ri.atEnd(); ri++ )
			{
				P_CHAR pChar = ri.GetData();
				if( pChar && pChar != m_npc && !pChar->isInvulnerable() && !pChar->isHidden() && !pChar->isInvisible() && !pChar->isDead() )
				{
					P_PLAYER pPlayer = dynamic_cast<P_PLAYER>(pChar);
					if( pPlayer && pPlayer->isGMorCounselor() )
						continue;

					currentState->foundVictim( pChar );
					break;
				}
			}
		}

		updateState();
		currentState->execute();
	}
}

static AbstractState* productCreator_MAL1_Wander()
{
	return new Monster_Aggr_L1_Wander();
}

void Monster_Aggr_L1_Wander::registerInFactory()
{
	StateFactory::instance()->registerType("Monster_Aggr_L1_Wander", productCreator_MAL1_Wander);
}

void Monster_Aggr_L1_Wander::attacked()
{
	reattack();
	nextState = new Monster_Aggr_L1_Combat( m_interface, npc );
}

void Monster_Aggr_L1_Wander::foundVictim( P_CHAR pVictim )
{
	if( pVictim )
		npc->setCombatTarget( pVictim->serial() );
	attack();
	nextState = new Monster_Aggr_L1_Combat( m_interface, npc );
}

void Monster_Aggr_L1_Wander::execute()
{
	// wander freely
	wanderFreely();
}

static AbstractState* productCreator_MAL1_Combat()
{
	return new Monster_Aggr_L1_Combat();
}

void Monster_Aggr_L1_Combat::registerInFactory()
{
	StateFactory::instance()->registerType("Monster_Aggr_L1_Combat", productCreator_MAL1_Combat);
}

void Monster_Aggr_L1_Combat::won()
{
	npc->setAttackerSerial( INVALID_SERIAL );
	npc->setCombatTarget( INVALID_SERIAL );
	nextState = new Monster_Aggr_L1_Wander( m_interface, npc );
}

void Monster_Aggr_L1_Combat::combatCancelled()
{
	npc->setAttackerSerial( INVALID_SERIAL );
	npc->setCombatTarget( INVALID_SERIAL );
	nextState = new Monster_Aggr_L1_Wander( m_interface, npc );
}

void Monster_Aggr_L1_Combat::hitpointsCritical()
{
	nextState = new Monster_Aggr_L1_Flee( m_interface, npc );
}

void Monster_Aggr_L1_Combat::execute()
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
		Coord_cl pos = npc->pos();
		if( SrvParams->pathfind4Combat() )
			movePath( pTarget->pos() );
		else
			moveTo( pTarget->pos() );

		// no move found
		if( pos == npc->pos() )
		{ // search new target
			RegionIterator4Chars ri( npc->pos(), VISRANGE );
			for( ri.Begin(); !ri.atEnd(); ri++ )
			{
				P_CHAR pChar = ri.GetData();
				if( pChar && pChar != npc && pChar != pTarget && !pChar->isInvulnerable() && !pChar->isHidden() && !pChar->isInvisible() && !pChar->isDead() )
				{
					P_PLAYER pPlayer = dynamic_cast<P_PLAYER>(pChar);
					if( pPlayer && pPlayer->isGMorCounselor() )
						continue;

					npc->setCombatTarget( pChar->serial() );
					break;
				}
			}
		}
	}
}

static AbstractState* productCreator_MAL1_Flee()
{
	return new Monster_Aggr_L1_Flee();
}

void Monster_Aggr_L1_Flee::registerInFactory()
{
	StateFactory::instance()->registerType("Monster_Aggr_L1_Flee", productCreator_MAL1_Flee);
}

void Monster_Aggr_L1_Flee::won()
{
	npc->setAttackerSerial( INVALID_SERIAL );
	npc->setCombatTarget( INVALID_SERIAL );
	nextState = new Monster_Aggr_L1_Wander( m_interface, npc );
}

void Monster_Aggr_L1_Flee::combatCancelled()
{
	npc->setAttackerSerial( INVALID_SERIAL );
	npc->setCombatTarget( INVALID_SERIAL );
	nextState = new Monster_Aggr_L1_Wander( m_interface, npc );
}

void Monster_Aggr_L1_Flee::hitpointsRestored()
{
	nextState = new Monster_Aggr_L1_Combat( m_interface, npc );
}

void Monster_Aggr_L1_Flee::execute()
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