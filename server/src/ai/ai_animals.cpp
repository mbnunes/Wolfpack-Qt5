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
#include "../npc.h"
#include "../player.h"
#include "../config.h"

#include "../sectors.h"
#include "../basics.h"
#include "../targetrequests.h"

// library includes
#include <math.h>

static AbstractAI* productCreator_AW()
{
	return new Animal_Wild( NULL );
}

void Animal_Wild::registerInFactory()
{
	AIFactory::instance()->registerType("Animal_Wild", productCreator_AW);
}

static AbstractAI* productCreator_AD()
{
	return new Animal_Domestic( NULL );
}

void Animal_Domestic::registerInFactory()
{
	AIFactory::instance()->registerType("Animal_Domestic", productCreator_AD);
}

void AnimalAI::onSpeechInput(P_PLAYER pTalker, const QString &comm) {
	return;
}

float Animal_Wild_Flee::preCondition()
{
	/*
	 * Fleeing from an approaching player has the following preconditions:
	 * - There is a player within flight range.
	 * - There is no character attacking us.
	 * - Our owner is not in range.
	 *
	 */

	if( m_npc->attackTarget() )
		return 0.0f;

	RegionIterator4Chars ri( m_npc->pos(), Config::instance()->animalWildFleeRange() );
	for(ri.Begin(); !ri.atEnd(); ri++)
	{
		P_PLAYER pPlayer = dynamic_cast<P_PLAYER>(ri.GetData());
		if( pPlayer && !pPlayer->free && !pPlayer->isGMorCounselor() && !pPlayer->isHidden() && !pPlayer->isInvisible() )
		{
			pFleeFrom = pPlayer;
		}
		if( pPlayer && m_npc->owner() == pPlayer )
			return 0.0f;

	}
	if( pFleeFrom )
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

	if( m_npc->attackTarget() )
		return 1.0f;

	RegionIterator4Chars ri( m_npc->pos(), Config::instance()->animalWildFleeRange() );
	bool found = false;
	for(ri.Begin(); !ri.atEnd(); ri++)
	{
		P_PLAYER pPlayer = dynamic_cast<P_PLAYER>(ri.GetData());
		if( pPlayer && !pPlayer->free && !pPlayer->isGMorCounselor() && !pPlayer->isHidden() && !pPlayer->isInvisible() )
			found = true;

		if( pPlayer && m_npc->owner() == pPlayer )
			return 1.0f;
	}

	if( found )
		return 0.0f;

	return 1.0f;
}


