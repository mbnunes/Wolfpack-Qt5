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
 * Wolfpack Homepage: http://wpdev.sf.net/
 */

#include "ai.h"
#include "../combat.h"
#include "../npc.h"
#include "../factory.h"
#include "../sectors.h"
#include "../player.h"
#include "../serverconfig.h"
#include "../basics.h"
#include "../items.h"

// library includes
#include <math.h>

// Is this an invalid target?
bool invalidTarget(P_NPC npc, P_CHAR victim, int dist = -1) {
	if (victim->isInvulnerable() || victim->isDead()) {
		return true;
	}

	if (npc->owner() == victim) {
		return true;
	}

	if (dist == -1) {
		dist = npc->dist(victim);
	}

	if (dist > Config::instance()->attack_distance()) {
		return true;
	}

	if (!npc->canSee(victim)) {
		return true;
	}

	return false;
}

// Is this a valid target?
bool validTarget(P_NPC npc, P_CHAR victim, int dist = -1) {
	if (invalidTarget(npc, victim, dist)) {
		return false;
	}

	if (!npc->lineOfSight(victim)) {
		return false;
	}

	return true;
}

// Find the best target for this NPC
P_CHAR findBestTarget(P_NPC npc) {
	unsigned int distance = ~0;
	P_CHAR target = 0;

	// Search for targets in our list of current targets first
	QPtrList<cFightInfo> fights = npc->fights();
	for (cFightInfo *info = fights.first(); info; info = fights.next()) {
		P_CHAR victim = info->victim();
		if (victim == npc) {
			victim = info->attacker();
		}

		// We don't already attack the target, right?
		// If we're tamed we only choose this target if
		// it's fighting us.
		if (victim != target && (!npc->isTamed() || victim->attackTarget() == npc)) {
			// See if it's a target we want
			unsigned int dist = npc->dist(victim);		
			if (dist < distance && validTarget(npc, victim, dist)) {
				target = victim;
				distance = dist;
			}
		}
	}

	// If we're not tamed, we attack other players as well.
	if (!npc->isTamed()) {
		RegionIterator4Chars ri(npc->pos(), VISRANGE);
		for ( ri.Begin(); !ri.atEnd(); ri++ ) {
			// We limit ourself to players here
			P_PLAYER victim= dynamic_cast<P_PLAYER>( ri.GetData() );

			// We don't already attack the target, right?
			if (victim && victim != target) {
				// See if it's a target we want
				unsigned int dist = npc->dist(victim);
				if (dist < distance && validTarget(npc, victim, dist)) {
					target = victim;
					distance = dist;
				}
			}
		}
	}

	return target;
}

void Monster_Aggressive::check()
{
	// Our current victim
	P_CHAR m_currentVictim = World::instance()->findChar(m_currentVictimSer);
	if (!m_currentVictim) {
		m_currentVictim = INVALID_SERIAL;
	}

	if (m_currentVictim && invalidTarget(m_npc, m_currentVictim)) {
		m_currentVictim = 0;
		m_currentVictim = INVALID_SERIAL;
		m_npc->fight(0);
	}

	
	if (nextVictimCheck < Server::instance()->time()) {
		// Don't switch if we can hit it...
		if (!m_currentVictim || m_currentVictim->dist(m_npc) > 1) {
			P_CHAR target = findBestTarget(m_npc);

			if (target) {
				m_currentVictim = target;
				m_currentVictimSer = target->serial();
				m_npc->fight(target);
			}
		}

		nextVictimCheck = Server::instance()->time() + 1500;
	}

	AbstractAI::check();
}

static AbstractAI* productCreator_MAL0()
{
	return new Monster_Aggressive_L0( NULL );
}

void Monster_Aggressive_L0::registerInFactory()
{
	AIFactory::instance()->registerType( "Monster_Aggressive_L0", productCreator_MAL0 );
}

void Monster_Aggressive_L0::selectVictim()
{
}

static AbstractAI* productCreator_MB()
{
	return new Monster_Berserk( NULL );
}

void Monster_Berserk::registerInFactory()
{
	AIFactory::instance()->registerType( "Monster_Berserk", productCreator_MB );
}

void Monster_Berserk::selectVictim()
{
}

static AbstractAI* productCreator_MAL1()
{
	return new Monster_Aggressive_L1( NULL );
}

void Monster_Aggressive_L1::registerInFactory()
{
	AIFactory::instance()->registerType( "Monster_Aggressive_L1", productCreator_MAL1 );
}

void Monster_Aggressive_L1::selectVictim()
{
}

float Monster_Aggr_Wander::preCondition()
{
	/*
	 * Wandering has the following preconditions:
	 * - Same as Action_Wander
	 * - No target has been set.
	 */

	Monster_Aggressive* pAI = dynamic_cast<Monster_Aggressive*>( m_ai );
	if ( pAI && pAI->currentVictim() )
		return 0.0f;

	return Action_Wander::preCondition();
}

float Monster_Aggr_MoveToTarget::preCondition()
{
	/*
	 * Moving to the target has the following preconditions:
	 * - A target has been set.
	 * - The NPC is not in combat range.
	 *
	 * Here we take the fuzzy logic into account.
	 * If the npc is injured, the chance of fighting will decrease.
	 */

	Monster_Aggressive* pAI = dynamic_cast<Monster_Aggressive*>( m_ai );
	if ( !pAI || !pAI->currentVictim() )
		return 0.0f;

	Q_UINT8 range = 1;
	P_ITEM weapon = m_npc->getWeapon();

	if ( weapon )
	{
		if ( weapon->hasTag( "range" ) ) {
			range = weapon->getTag( "range" ).toInt();
		} else if ( weapon->basedef() ) {
			range = weapon->basedef()->getIntProperty( "range", 1 );
		}
	}

	if ( m_npc->inRange( pAI->currentVictim(), range ) )
		return 0.0f;

	// 1.0 = Full Health, 0.0 = Dead
	float diff = 1.0 - QMAX( 0, ( m_npc->maxHitpoints() - m_npc->hitpoints() ) / ( float ) m_npc->maxHitpoints() );

	if ( diff <= m_npc->criticalHealth() / 100.0 )
	{
		return 0.0;
	}

	return 1.0;
}

float Monster_Aggr_MoveToTarget::postCondition()
{
	/*
	 * Moving to the target has the following postconditions:
	 * - The target is not set anymore.
	 * - The NPC is within fight range.
	 * - The NPC is not injured above the critical line.
	 */

	Monster_Aggressive* pAI = dynamic_cast<Monster_Aggressive*>( m_ai );
	if ( !pAI || !pAI->currentVictim() )
		return 1.0f;

	Q_UINT8 range = 1;
	P_ITEM weapon = m_npc->getWeapon();
	if ( weapon )
	{
		if ( weapon->hasTag( "range" ) ) {
			range = weapon->getTag( "range" ).toInt();
		} else if ( weapon->basedef() ) {
			range = weapon->basedef()->getIntProperty( "range", 1 );
		}
	}

	if ( m_npc->inRange( pAI->currentVictim(), range ) )
		return 1.0f;

	// 1.0 = Full Health, 0.0 = Dead
	float diff = 1.0 - QMAX( 0, ( m_npc->maxHitpoints() - m_npc->hitpoints() ) / ( float ) m_npc->maxHitpoints() );

	if ( diff <= m_npc->criticalHealth() / 100.0 )
	{
		return 1.0;
	}

	return 0.0;
}

void Monster_Aggr_MoveToTarget::execute()
{
	Monster_Aggressive* pAI = dynamic_cast<Monster_Aggressive*>( m_ai );

	if ( !pAI || !pAI->currentVictim() )
		return;

	// Even if the victim is zero, thats correct.
	if ( pAI && !m_npc->attackTarget() )
	{
		m_npc->fight( pAI->currentVictim() );
	}

	if ( Config::instance()->pathfind4Combat() )
		movePath( pAI->currentVictim()->pos() );
	else
		moveTo( pAI->currentVictim()->pos() );
}

float Monster_Aggr_Fight::preCondition()
{
	/*
	 * Fighting the target has the following preconditions:
	 * - A target has been set.
	 * - The target is not dead.
	 * - The NPC is in combat range.
	 *
	 * Here we take the fuzzy logic into account.
	 * If the npc is injured, the chance of fighting will decrease.
	 */

	Monster_Aggressive* pAI = dynamic_cast<Monster_Aggressive*>( m_ai );

	if ( !pAI || !pAI->currentVictim() || pAI->currentVictim()->isDead() )
		return 0.0f;

	Q_UINT8 range = 1;
	P_ITEM weapon = m_npc->getWeapon();
	if ( weapon )
	{
		if ( weapon->hasTag( "range" ) ) {
			range = weapon->getTag( "range" ).toInt();
		} else if ( weapon->basedef() ) {
			range = weapon->basedef()->getIntProperty( "range", 1 );
		}
	}

	if ( !m_npc->inRange( pAI->currentVictim(), range ) )
		return 0.0f;

	// 1.0 = Full Health, 0.0 = Dead
	float diff = 1.0 - QMAX( 0, ( m_npc->maxHitpoints() - m_npc->hitpoints() ) / ( float ) m_npc->maxHitpoints() );

	if ( diff <= m_npc->criticalHealth() / 100.0 )
	{
		return 0.0;
	}

	return 1.0;
}

float Monster_Aggr_Fight::postCondition()
{
	/*
	 * Fighting the target has the following postconditions:
	 * - The target is not set anymore.
	 * - The NPC is not within fight range.
	 * - The target is dead.
	 * - The NPC is injured above the criticial line.
	 */

	Monster_Aggressive* pAI = dynamic_cast<Monster_Aggressive*>( m_ai );
	if ( !pAI || !pAI->currentVictim() || pAI->currentVictim()->isDead() )
		return 1.0f;

	Q_UINT8 range = 1;
	P_ITEM weapon = m_npc->getWeapon();
	if ( weapon )
	{
		if ( weapon->hasTag( "range" ) ) {
			range = weapon->getTag( "range" ).toInt();
		} else if ( weapon->basedef() ) {
			range = weapon->basedef()->getIntProperty( "range", 1 );
		}
	}

	if ( !m_npc->inRange( pAI->currentVictim(), range ) )
	{
#if defined(AIDEBUG)
		m_npc->talk( "[COMBAT: Not In Range]" );
#endif
		return 1.0f;
	}

	// 1.0 = Full Health, 0.0 = Dead
	float diff = 1.0 - QMAX( 0, ( m_npc->maxHitpoints() - m_npc->hitpoints() ) / ( float ) m_npc->maxHitpoints() );

	if ( diff <= m_npc->criticalHealth() / 100.0 )
	{
		return 1.0;
	}

	return 0.0;
}

void Monster_Aggr_Fight::execute()
{
	Monster_Aggressive* ai = dynamic_cast<Monster_Aggressive*>( m_ai );

	if ( !m_npc->attackTarget() )
	{
		m_npc->fight( ai->currentVictim() );
	}
}
