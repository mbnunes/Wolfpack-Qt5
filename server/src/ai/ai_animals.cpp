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
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#include "ai.h"
#include "../npc.h"
#include "../player.h"
#include "../serverconfig.h"

#include "../basics.h"
#include "../mapobjects.h"
#include "../targetrequests.h"

// library includes
#include <math.h>

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


