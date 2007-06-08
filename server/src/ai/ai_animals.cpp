/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2007 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#include "ai.h"
#include "../npc.h"
#include "../player.h"
#include "../serverconfig.h"
#include "../mapobjects.h"
#include "../combat.h"


void Animal_Wild::registerInFactory()
{
#ifndef __VC6
	AIFactory::instance()->registerType( "Animal_Wild", productCreatorFunctor<Animal_Wild> );
	AIFactory::instance()->registerType( "Animal_Aggressive", productCreatorFunctor<Animal_Wild> );
#else
	AIFactory::instance()->registerType( "Animal_Wild", productCreatorFunctor_Animal_Wild );
	AIFactory::instance()->registerType( "Animal_Aggressive", productCreatorFunctor_Animal_Wild );
#endif
}

void Animal_Domestic::registerInFactory()
{
#ifndef __VC6
	AIFactory::instance()->registerType( "Animal_Domestic", productCreatorFunctor<Animal_Domestic> );
#else
	AIFactory::instance()->registerType( "Animal_Domestic", productCreatorFunctor_Animal_Domestic );
#endif
}

void Animal_Predator::registerInFactory()
{
#ifndef __VC6
	AIFactory::instance()->registerType( "Animal_Predator", productCreatorFunctor<Animal_Predator> );
#else
	AIFactory::instance()->registerType( "Animal_Predator", productCreatorFunctor_Animal_Predator );
#endif
}

void AnimalAI::onSpeechInput( P_PLAYER pTalker, const QString& comm )
{
	Q_UNUSED( pTalker );
	Q_UNUSED( comm );
	return;
}

float Animal_Wild_Flee::preCondition()
{
	return 0.0f;

	/*
	 * Fleeing from an approaching player has the following preconditions:
	 * - There is a player within flight range.
	 * - There is no character attacking us.
	 * - Our owner is not in range.
	 *
	 */

	if ( m_npc->attackTarget() )
		return 0.0f;

	MapCharsIterator ri = MapObjects::instance()->listCharsInCircle( m_npc->pos(), Config::instance()->animalWildFleeRange() );
	for ( P_CHAR pChar = ri.first(); pChar; pChar = ri.next() )
	{
		P_PLAYER pPlayer = dynamic_cast<P_PLAYER>( pChar );
		if ( pPlayer && !pPlayer->free && !pPlayer->isGMorCounselor() && !pPlayer->isHidden() && !pPlayer->isInvisible() )
		{
			pFleeFromSer = pPlayer->serial();
		}
		if ( pPlayer && m_npc->owner() == pPlayer )
			return 0.0f;
	}

	if ( pFleeFromSer != INVALID_SERIAL )
		return 1.0f;

	return 0.0f;
}

float Animal_Wild_Flee::postCondition()
{
	/*
	 * Fleeing from an approaching player has the following postconditions:
	 * - There is no character in flight range.
	 * - There is an character attacking us.
	 * - Our owner has come in range.
	 *
	 */

	if ( m_npc->attackTarget() )
		return 1.0f;

	bool found = false;
	MapCharsIterator ri = MapObjects::instance()->listCharsInCircle( m_npc->pos(), Config::instance()->animalWildFleeRange() );
	for ( P_CHAR pChar = ri.first(); pChar; pChar = ri.next() )
	{
		P_PLAYER pPlayer = dynamic_cast<P_PLAYER>( pChar );
		if ( pPlayer && !pPlayer->free && !pPlayer->isGMorCounselor() && !pPlayer->isHidden() && !pPlayer->isInvisible() )
			found = true;

		if ( pPlayer && m_npc->owner() == pPlayer )
			return 1.0f;
	}

	if ( found )
		return 0.0f;

	return 1.0f;
}

void Animal_Predator::check()
{
	// Our current victim
	P_CHAR m_currentVictim = World::instance()->findChar( m_currentVictimSer );
	if ( !m_currentVictim )
	{
		m_currentVictimSer = INVALID_SERIAL;
	}

	if ( m_currentVictim && invalidTarget( m_currentVictim ) )
	{
		m_currentVictim = 0;
		m_currentVictimSer = INVALID_SERIAL;
		m_npc->fight( 0 );
	}

	if ( nextVictimCheck < Server::instance()->time() )
	{
		// Don't switch if we can hit it...
		if ( !m_currentVictim || m_currentVictim->dist( m_npc ) > 1 )
		{
			P_CHAR target = findPrey();
			if ( target )
			{
				m_currentVictim = target;
				m_currentVictimSer = target->serial();
				m_npc->fight( target );
			}
		}

		nextVictimCheck = Server::instance()->time() + 1500;
	}

	AbstractAI::check();
}

// Find prey for this predator
P_CHAR Animal_Predator::findPrey()
{
	unsigned int distance = ~0;
	P_CHAR target = 0;

	if ( !m_npc )
	{
		return target;
	}

	// Search for targets in our list of current targets first
	QList<cFightInfo*> fights( m_npc->fights() );
	foreach ( cFightInfo* info, fights )
	{
		P_CHAR victim = info->victim();
		if ( victim == m_npc )
		{
			victim = info->attacker();
		}

		// We don't already attack the target, right?
		if ( victim != target )
		{
			// See if it's a target we want
			unsigned int dist = m_npc->dist( victim );
			if ( dist < distance && validTarget( victim, dist, false ) )
			{
				target = victim;
				distance = dist;
			}
		}
	}

	// If we are tamed or there is nothing we like we do not hunt
	if ( m_npc->isTamed() || prey.isEmpty() )
	{	
		return target;
	}

	// Get all chars around us
	MapCharsIterator ri = MapObjects::instance()->listCharsInCircle( m_npc->pos(), VISRANGE );
	for ( P_CHAR pChar = ri.first(); pChar; pChar = ri.next() )
	{
		P_PLAYER victim = dynamic_cast<P_PLAYER>( pChar );
		P_NPC npcVictim = dynamic_cast<P_NPC>( pChar );

		// We don't already attack the target, right?
		if ( victim && victim != target )
		{
			// do we eat players?
			if ( huntPlayers )
			{
				// See if it's a target we want
				unsigned int dist = m_npc->dist( victim );
				if ( dist < distance && validTarget( victim, dist, true ) )
				{
					target = victim;
					distance = dist;
				}
			}
		}
		else if ( npcVictim && npcVictim->owner() && npcVictim != target )
		{
			// do we eat their pets?
			if ( huntPlayers )
			{
				// See if it's a target we want
				unsigned int dist = m_npc->dist( npcVictim );
				if ( dist < distance && validTarget( npcVictim, dist, true ) )
				{
					target = npcVictim;
					distance = dist;
				}
			}
		}
		else if ( npcVictim && npcVictim != target )
		{
			// See if it's a target we want
			unsigned int dist = m_npc->dist( npcVictim );
			// check prey (stringlist) last, because this is the most expensive
			if ( dist < distance && validTarget( npcVictim, dist, true ) && prey.contains( npcVictim->baseid() ) )
			{
				target = npcVictim;
				distance = dist;
			}
		}
	}

	return target;
}

void Animal_Predator::initPrey()
{
	if ( m_npc )
	{
		// build a list of baseids of chars we hunt
		QString preyProp = m_npc->basedef()->getStrProperty( "prey" );
		prey = preyProp.split( ",", QString::SkipEmptyParts );
		
		// player get special handling to lower costs
		if ( prey.contains( "player" ) )
		{
			huntPlayers = true;
			prey.removeAll( "player" );
		}
		else
		{
			huntPlayers = false;
		}
	}
}