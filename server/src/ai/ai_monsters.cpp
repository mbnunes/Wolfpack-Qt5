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
#include "../npc.h"
#include "../factory.h"
#include "../sectors.h"
#include "../player.h"
#include "../srvparams.h"
#include "../globals.h"
#include "../basics.h"
#include "../itemid.h"
#include "../items.h"

// library includes
#include <math.h>

void Monster_Aggressive::check()
{
	selectVictim();
	AbstractAI::check();
}

static AbstractAI* productCreator_MAL0()
{
	return new Monster_Aggressive_L0( NULL );
}

void Monster_Aggressive_L0::registerInFactory()
{
	AIFactory::instance()->registerType("Monster_Aggressive_L0", productCreator_MAL0);
}

void Monster_Aggressive_L0::selectVictim()
{
	// We must make sure, that the npc has a correct target.
	// If he already has one, we must check if it is still correct.
	// Else we search a new one.

	// If the npc is tamed, it cant attack
	if( m_npc->isTamed() )
	{
		m_currentVictim = NULL;
		return;
	}
	
	if( m_currentVictim )
	{
		// Check if the current target is valid, including:
		// - Target not dead.
		// - Target in attack range.
		if( m_currentVictim->isDead() )
			m_currentVictim = NULL;
		else if( !m_npc->inRange( m_currentVictim, SrvParams->attack_distance() ) )
			m_currentVictim = NULL;
	}
	
	if( !m_currentVictim )
	{
		// Get the first best character and attack it
		RegionIterator4Chars ri( m_npc->pos(), VISRANGE );
		for( ri.Begin(); !ri.atEnd(); ri++ )
		{
			P_CHAR pChar = ri.GetData();
			if( pChar && !pChar->free && pChar != m_npc && !pChar->isInvulnerable() && !pChar->isHidden() && !pChar->isInvisible() && !pChar->isDead() )
			{
				P_PLAYER pPlayer = dynamic_cast<P_PLAYER>(pChar);
				if( pPlayer && pPlayer->isGMorCounselor() )
					continue;

				m_currentVictim = pChar;
				break;
			}
		}

		// If we found a new target, let us attack it
		if( m_currentVictim )
			m_npc->fight( m_currentVictim );
	}

}

static AbstractAI* productCreator_MAL1()
{
	return new Monster_Aggressive_L1( NULL );
}

void Monster_Aggressive_L1::registerInFactory()
{
	AIFactory::instance()->registerType("Monster_Aggressive_L1", productCreator_MAL1);
}

void Monster_Aggressive_L1::selectVictim()
{
	// We must make sure, that the npc has a correct target.
	// If he already has one, we must check if it is still correct.
	// Else we search a new one.
	
	// If the npc is tamed, it cant attack
	if( m_npc->isTamed() )
	{
		m_currentVictim = NULL;
		return;
	}
	
	if( m_currentVictim )
	{
		// Check if the current target is valid, including:
		// - Target not dead.
		// - Target in attack range.
		if( m_currentVictim->isDead() )
			m_currentVictim = NULL;
		else if( !m_npc->inRange( m_currentVictim, SrvParams->attack_distance() ) )
			m_currentVictim = NULL;
	}
	
	int currentPriority = -0x00DDDDDD;
	P_CHAR newVictim = m_currentVictim;
	// we divide the negative priority of the current victim by two, 
	// so it cant be overwritten that easy.
	// This shall reduce "target hopping".
	if( m_currentVictim )
		currentPriority = (0 - m_currentVictim->dist( m_npc ) - m_currentVictim->hitpoints() - m_currentVictim->calcDefense(ALLBODYPARTS)) / 2;

	// Get the attackable char which has the highest priority.
	// The priority is calculated by *distance* and *strength*.
	RegionIterator4Chars ri( m_npc->pos(), VISRANGE );
	for( ri.Begin(); !ri.atEnd(); ri++ )
	{
		P_CHAR pChar = ri.GetData();
		if( pChar && !pChar->free && pChar != m_npc && !pChar->isInvulnerable() && !pChar->isHidden() && !pChar->isInvisible() && !pChar->isDead() )
		{
			P_PLAYER pPlayer = dynamic_cast<P_PLAYER>(pChar);

			int priority = 0 - pChar->dist( m_npc ) - pChar->hitpoints() - pChar->calcDefense( ALLBODYPARTS );
			if( priority > currentPriority )
			{
				newVictim = pChar;
				currentPriority = priority;
			}
		}
	}

	// If we found a new target, let us attack it
	if( (!m_currentVictim && newVictim) || (m_currentVictim && m_currentVictim != newVictim) )
	{
		m_currentVictim = newVictim;
		m_npc->fight( newVictim );
	}
}

float Monster_Aggr_Wander::preCondition()
{
	/*
	 * Wandering has the following preconditions:
	 * - Same as Action_Wander
	 * - No target has been set.
	 */

	Monster_Aggressive* pAI = dynamic_cast< Monster_Aggressive* >( m_ai );
	if( pAI && pAI->currentVictim() )
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

	Monster_Aggressive* pAI = dynamic_cast< Monster_Aggressive* >( m_ai );
	if( !pAI || !pAI->currentVictim() )
		return 0.0f;
	
	UINT8 range = 1;
	if( m_npc->rightHandItem() && IsBowType( m_npc->rightHandItem()->id() ) )
		range = ARCHERY_RANGE;

	if( m_npc->inRange( pAI->currentVictim(), range ) )
		return 0.0f;

	float healthmod = (float)m_npc->hitpoints() / ((float)m_npc->criticalHealth()/100.0f * (float)m_npc->maxHitpoints());
	return healthmod;
}

float Monster_Aggr_MoveToTarget::postCondition()
{
	/*
	 * Moving to the target has the following postconditions:
	 * - The target is not set anymore.
	 * - The NPC is within fight range.
	 * - The NPC is not injured above the critical line.
	 */

	Monster_Aggressive* pAI = dynamic_cast< Monster_Aggressive* >( m_ai );
	if( !pAI || !pAI->currentVictim() )
		return 1.0f;

	UINT8 range = 1;
	if( m_npc->rightHandItem() && IsBowType( m_npc->rightHandItem()->id() ) )
		range = ARCHERY_RANGE;

	if( m_npc->inRange( pAI->currentVictim(), range ) )
		return 1.0f;

	float healthmod = (float)(m_npc->maxHitpoints() - m_npc->hitpoints()) /
						(float)(m_npc->maxHitpoints() - ((float)m_npc->criticalHealth()/100.0f * (float)m_npc->maxHitpoints()));
	return healthmod;
}

void Monster_Aggr_MoveToTarget::execute()
{
	Monster_Aggressive* pAI = dynamic_cast< Monster_Aggressive* >( m_ai );

	// Even if the victim is zero, thats correct.
	if (pAI) {
		m_npc->fight(pAI->currentVictim());
	}

	if( !pAI || !pAI->currentVictim() )
		return;

	if( SrvParams->pathfind4Combat() )
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

	Monster_Aggressive* pAI = dynamic_cast< Monster_Aggressive* >( m_ai );

	if (!pAI || !pAI->currentVictim() || pAI->currentVictim()->isDead())
		return 0.0f;
	
	UINT8 range = 1;
	if( m_npc->rightHandItem() && IsBowType( m_npc->rightHandItem()->id() ) )
		range = ARCHERY_RANGE;

	if( !m_npc->inRange( pAI->currentVictim(), range ) )
		return 0.0f;

	float healthmod = (float)m_npc->hitpoints() / ((float)m_npc->criticalHealth()/100.0f * (float)m_npc->maxHitpoints());
	return healthmod;
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

	Monster_Aggressive* pAI = dynamic_cast< Monster_Aggressive* >( m_ai );
	if( !pAI || !pAI->currentVictim() || pAI->currentVictim()->isDead() )
		return 1.0f;
	
	UINT8 range = 1;
	if( m_npc->rightHandItem() && IsBowType( m_npc->rightHandItem()->id() ) )
		range = ARCHERY_RANGE;

	if( !m_npc->inRange( pAI->currentVictim(), range ) )
		return 1.0f;

	float healthmod = (float)(m_npc->maxHitpoints() - m_npc->hitpoints()) /
						(float)(m_npc->maxHitpoints() - ((float)m_npc->criticalHealth()/100.0f * (float)m_npc->maxHitpoints()));
	return healthmod;
}

void Monster_Aggr_Fight::execute()
{
	Monster_Aggressive* pAI = dynamic_cast< Monster_Aggressive* >( m_ai );
	if (pAI) {
		m_npc->fight(pAI->currentVictim());
	}
}
