//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2004 by holders identified in authors.txt
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
#include "../player.h"
#include "../srvparams.h"
#include "../globals.h"
#include "../sectors.h"
#include "../basics.h"
#include "../targetrequests.h"
#include "../chars.h"

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

void AnimalAI::onSpeechInput( P_PLAYER pTalker, const QString &comm )
{
	if( m_npc->owner() != pTalker && !pTalker->isGM() )
		return;

	// too far away to hear us
	if( pTalker->dist( m_npc ) > 7 )
		return;
	
	if( comm.contains( " FOLLOW" ) )
	{
		if( comm.contains( " ME" ) )
		{

			m_npc->setWanderFollowTarget( pTalker );
			m_npc->setWanderType( enFollowTarget );
			m_npc->bark( cBaseChar::Bark_Attacking );
		}
		else
		{
			pTalker->socket()->attachTarget( new cFollowTarget( m_npc ) );
			// LEGACY: target( s, 0, 1, 0, 117, "Click on the target to follow." );
		}
	}
	else if( ( comm.contains( " KILL" ) ) || ( comm.contains( " ATTACK" ) ) )
	{
		if( m_npc->inGuardedArea() ) // Ripper..No pet attacking in town.
		{
			pTalker->message( tr( "You can't have pets attack in town!" ) );
		}
		//pPlayer->setGuarded( false );
		// >> LEGACY
		//addx[s]=pPet->serial();
		//target(s, 0, 1, 0, 118, "Select the target to attack.");//AntiChrist
	}
	else if( ( comm.contains( " FETCH" ) ) || ( comm.contains( " GET" ) ) )
	{
		//pPlayer->setGuarded(false);
		// >> LEGACY
		//addx[s]=pPet->serial();
		//target(s, 0, 1, 0, 124, "Click on the object to fetch.");
	}
	else if( comm.contains( " COME" ) )
	{
		//pPlayer->setGuarded( false );
//		pPet->setWanderFollowTarget( pPlayer->serial() );
//		pPet->setWanderType( enFollowTarget );
//		m_npc->setNextMoveTime();
//		pTalker->message( tr( "Your pet begins following you." ) );
	}
	else if( comm.contains( " GUARD" ) )
	{
		// LEGACY
		/*addx[s] = pPet->serial();	// the pet's serial
		addy[s] = 0;

		if( comm.find( " ME" ) != string::npos )
			addy[s]=1;	// indicates we already know whom to guard (for future use)
		
		// for now they still must click on themselves (Duke)
		target(s, 0, 1, 0, 120, "Click on the char to guard.");*/
	}
	else if( ( comm.contains( " STOP" ) ) || ( comm.contains(" STAY") ) )
	{
		m_npc->fight(0);
		m_npc->setWanderType( enHalt );
	}
	else if( comm.contains( " TRANSFER" ) )
	{
		//pPlayer->setGuarded( false );
		// >> LEGACY
		/*addx[s]=pPet->serial();
		target(s, 0, 1, 0, 119, "Select character to transfer your pet to.");*/
	}
	else if( comm.contains( " RELEASE" ) )
	{
		// Has it been summoned ? Let's dispel it
		if( m_npc->summonTime() > uiCurrentTime )
			m_npc->setSummonTime( uiCurrentTime );

		m_npc->setWanderType( enFreely );
		m_npc->setOwner( NULL );
		m_npc->setTamed( false );
		m_npc->emote( tr( "%1 appears to have decided that it is better off without a master" ).arg( m_npc->name() ) );
		if( SrvParams->tamedDisappear() )
		{
			m_npc->soundEffect( 0x01FE );
			cCharStuff::DeleteChar( m_npc );
		}
	}
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

	RegionIterator4Chars ri( m_npc->pos(), SrvParams->animalWildFleeRange() );
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

	RegionIterator4Chars ri( m_npc->pos(), SrvParams->animalWildFleeRange() );
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


