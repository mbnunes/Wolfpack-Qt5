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
#include "npc.h"
#include "mapobjects.h"
#include "player.h"
#include "srvparams.h"
#include "globals.h"
#include "basics.h"
#include "walking.h"
#include "itemid.h"
#include "world.h"

// library includes
#include <math.h>
#include <vector>

struct stActionNode
{
	stActionNode( float fz, AbstractAction* ac ) : fuzzy( fz ), action( ac ) {}

	float			fuzzy;
	AbstractAction*	action;
};

struct ActionNodeComparePredicate : public std::binary_function<stActionNode, stActionNode, bool>
{
	bool operator()(const stActionNode &a, const stActionNode &b)
	{
		return a.fuzzy > b.fuzzy;
	}
};

void AbstractAI::check()
{
	// If we have no current action or our action cant be executed, we must get a new one
	if( !m_currentAction || ( m_currentAction && m_currentAction->preCondition() <= 0.0f ) )
	{
		std::vector< stActionNode > actions;
		std::vector< stActionNode >::iterator it;

		AbstractAction* action = NULL;
		for( action = m_actions.first(); action; action = m_actions.next() )
		{
			actions.push_back( stActionNode( action->preCondition(), action ) );
			clConsole.send( QString("%1\n").arg( action->preCondition() ) );
		}
		std::sort( actions.begin(), actions.end(), ActionNodeComparePredicate() );

		it = actions.begin();
		while( it != actions.end() && !m_currentAction )
		{
			if( (*it).fuzzy > 0.0f )
				m_currentAction = (*it).action;

			++it;
		}
	}

	// Now we should have a current action set, else do nothing!
	if( m_currentAction )
	{
		m_currentAction->execute();

		// We must check the postcondition now and set the current action to NULL
		// if the action is finished (when it returns >= 1.0f)!
		float rnd = RandomNum( 0, 1000 ) / 1000.0f;
		clConsole.send(QString("post:%1, rnd:%2").arg(m_currentAction->postCondition()).arg(rnd));
		if( m_currentAction->postCondition() >= rnd )
			m_currentAction = NULL;

	}
}

float Action_Wander::preCondition()
{
	/*
	 * Wandering has the following preconditions:
	 * - There is no character attacking us.
	 * - The wander type is not enHalt.
	 * - We are wandering towards a destination and aren't there yet.
	 * - We are following a char and aren't in follow range yet.
	 */

	if( m_npc->attackerSerial() != INVALID_SERIAL )
		return 0.0f;

	if( m_npc->wanderType() == enHalt )
		return 0.0f;

	if( m_npc->wanderType() == enDestination && m_npc->wanderDestination() == m_npc->pos() )
		return 0.0f;

	if( m_npc->wanderType() == enFollowTarget && m_npc->inRange( m_npc->wanderFollowTarget(), SrvParams->pathfindFollowRadius() ) )
		return 0.0f;

	return 1.0f;
}

float Action_Wander::postCondition()
{
	/*
	 * Wandering has the following postconditions:
	 * - The NPC is wandering freely/in a circle/in a rectangle, 
	 *   then we can abort after each step!
	 * - The NPC has reached its destination.
	 * - The NPC is within follow range.
	 */

	switch( m_npc->wanderType() )
	{
	case enFreely:
	case enCircle:
	case enRectangle:
		return 1.0f;

	case enDestination:
		if( m_npc->wanderDestination() == m_npc->pos() )
			return 1.0f;
		break;

	case enFollowTarget:
		if( m_npc->inRange( m_npc->wanderFollowTarget(), SrvParams->pathfindFollowRadius() ) )
			return 1.0f;
		break;

	};
	return 0.0f;
}

void Action_Wander::execute()
{
	switch( m_npc->wanderType() )
	{
	case enFreely:
	{
		UINT8 dir = m_npc->direction();
		if( RandomNum(0, 100) < 20 )
			dir = RandomNum( 0, 7 );

		m_npc->setDirection( dir );
		Movement::instance()->Walking( m_npc, dir, 0xFF );
		break;
	}
	case enRectangle:
	{
		// get any point out of the rectangle and calculate the direction to it
		UINT16 rndx = RandomNum( m_npc->wanderX1(), m_npc->wanderX2() );
		UINT16 rndy = RandomNum( m_npc->wanderY1(), m_npc->wanderY2() );
		
		UINT8 dir = chardirxyz( m_npc, rndx, rndy );
		m_npc->setDirection( dir );
		Movement::instance()->Walking( m_npc, dir, 0xFF );
		break;
	}
	case enCircle:
	{
		Coord_cl pos = m_npc->pos();
		pos.x = m_npc->wanderX1();
		pos.y = m_npc->wanderY1();
		// get any point within the circle and calculate the direction to it
		// first a random distance which can be max. the length of the radius
		float rnddist = (float)RandomNum( 1, m_npc->wanderRadius() );
		// now get a point on this circle around the m_npc
		float rndphi = (float)RandomNum( 0, 100 ) / 100.0f * 2.0f * 3.14159265358979323846f;
		pos.x = pos.x + (INT16)floor( cos( rndphi ) * rnddist );
		pos.y = pos.y + (INT16)floor( sin( rndphi ) * rnddist );

		UINT8 dir = chardirxyz( m_npc, pos.x, pos.y );
		m_npc->setDirection( dir );
		Movement::instance()->Walking( m_npc, dir, 0xFF );
		break;	
	}
	case enFollowTarget:
	{
		if( SrvParams->pathfind4Follow() )
		{
			P_CHAR pTarget = m_npc->wanderFollowTarget();
			if( pTarget )
			{
				movePath( pTarget->pos() );
			}
		}
		else
		{
			P_CHAR pTarget = m_npc->wanderFollowTarget();
			if( pTarget )
			{
				moveTo( pTarget->pos() );
			}
		}
		break;
	}
	case enDestination:
	{
		movePath( m_npc->wanderDestination() );
		break;
	}
	}
}

void Action_Wander::moveTo( const Coord_cl &pos )
{
	// simply move towards the target
	UINT8 dir = chardirxyz( m_npc, pos.x, pos.y );
	Coord_cl newPos = Movement::instance()->calcCoordFromDir( dir, m_npc->pos() );
	if( !mayWalk( m_npc, newPos ) )
	{
		if( dir == 7 )
			dir = 0;
		else
			dir++;

		newPos = Movement::instance()->calcCoordFromDir( dir, m_npc->pos() );
		if( !mayWalk( m_npc, newPos ) )
		{
			if( dir == 0 )
				dir = 6;
			else if( dir == 1 )
				dir = 7;
			else
				dir = dir - 2;

			newPos = Movement::instance()->calcCoordFromDir( dir, m_npc->pos() );
			if( !mayWalk( m_npc, newPos ) )
			{
				return;
			}
		}
	}

	m_npc->setDirection( dir );
	Movement::instance()->Walking( m_npc, dir, 0xFF );
}

void Action_Wander::movePath( const Coord_cl &pos )
{
	if( ( !waitForPathCalculation && !m_npc->hasPath() ) || pos != m_npc->pathDestination() )
	{
		UINT8 range = 1;
		if( m_npc->rightHandItem() && IsBowType( m_npc->rightHandItem()->id() ) )
			range = ARCHERY_RANGE;

		m_npc->findPath( pos, range == 1 ? 1.5f : (float)range );
		// dont return here!
	}
	else
	{
		waitForPathCalculation--;
		moveTo( pos );
		return;
	}

	if( m_npc->hasPath() )
	{
		waitForPathCalculation = 0;
		Coord_cl nextmove = m_npc->nextMove();
		UINT8 dir = chardirxyz( m_npc, nextmove.x, nextmove.y );
		m_npc->setDirection( dir );
		Movement::instance()->Walking( m_npc, dir, 0xFF );
		m_npc->popMove();
		return;
	}
	else
	{
		waitForPathCalculation = 3;
		moveTo( pos );
		return;
	}
}

float Action_FleeAttacker::preCondition()
{
	/*
	 * Fleeing from an attacker has the following preconditions:
	 * - There is a character attacking us.
	 * - The attacking character has died.
	 * - The attacking character is within flee range.
	 * - The hitpoints are below the critical value.
	 *
	 * Fuzzy: The nearer we get to the critical health, the chance
	 *	      increases to flee.
	 */

	P_CHAR pAttacker = World::instance()->findChar( m_npc->attackerSerial() );
	if( !pAttacker || pAttacker->isDead() || !m_npc->inRange( pAttacker, SrvParams->pathfindFleeRadius() ) )
		return 0.0f;

	if( m_npc->hitpoints() < m_npc->criticalHealth() )
		return 0.0f;

	float healthmod = (float)(m_npc->maxHitpoints() - m_npc->hitpoints()) /
						(float)(m_npc->maxHitpoints() - m_npc->criticalHealth());
	return healthmod;
}

float Action_FleeAttacker::postCondition()
{
	/*
	 * Fleeing from an attacker has the following postconditions:
	 * - The character isn't attacking us anymore.
	 * - The attacker has died.
	 * - The attacker is not within flee range.
	 * - The hitpoints are restored.
	 *
	 * Fuzzy: The farer we are from the critical health line,
	 *        the higher is the chance to end the flee action.
	 */

	P_CHAR pAttacker = World::instance()->findChar( m_npc->attackerSerial() );
	if( !pAttacker || pAttacker->isDead() || !m_npc->inRange( pAttacker, SrvParams->pathfindFleeRadius() ) )
		return 1.0f;

	if( m_npc->hitpoints() == m_npc->maxHitpoints() )
		return 1.0f;

	float healthmod = (float)(m_npc->hitpoints() - m_npc->criticalHealth()) /
						(float)(m_npc->maxHitpoints() - m_npc->criticalHealth());
	return healthmod;
}

void Action_FleeAttacker::execute()
{
	if( !m_npc->hasPath() )
	{
		Coord_cl newPos = m_npc->pos();
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
		movePath( m_npc->pathDestination() );
}

