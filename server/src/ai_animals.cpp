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
#include "player.h"

/*
#include "world.h"
#include "network/uosocket.h"
#include "inlines.h"
#include "debug.h"
#include "TmpEff.h"
#include "srvparams.h"
#include "chars.h"

#define WILD_ANIMALS_FLEE_TIME_ON_TAME	30

void PetCommand( P_PLAYER pPlayer, P_NPC pPet, const QString& comm )
{
	if( pPet->owner() != pPlayer && !pPlayer->isGM() )
		return;

	// too far away to hear us
	if( pPlayer->dist( pPet ) > 7 )
		return;
	
	if( comm.contains( " FOLLOW" ) )
	{
		if( comm.contains( " ME" ) )
		{

//			pPet->setWanderFollowTarget( pPlayer->serial() );
//			pPet->setWanderType( enFollowTarget );
			playmonstersound( pPet, pPet->bodyID(), SND_STARTATTACK );
		}
		else
		{
			// LEGACY: target( s, 0, 1, 0, 117, "Click on the target to follow." );
		}
	}
	else if( ( comm.contains( " KILL" ) ) || ( comm.contains( " ATTACK" ) ) )
	{
		if( pPet->inGuardedArea() ) // Ripper..No pet attacking in town.
		{
			pPlayer->message( tr( "You can't have pets attack in town!" ) );
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
		pPet->setNextMoveTime();
		pPlayer->message( tr( "Your pet begins following you." ) );
	}
	else if( comm.contains( " GUARD" ) )
	{
		// LEGACY
		/*addx[s] = pPet->serial();	// the pet's serial
		addy[s] = 0;

		if( comm.find( " ME" ) != string::npos )
			addy[s]=1;	// indicates we already know whom to guard (for future use)
		
		// for now they still must click on themselves (Duke)
		target(s, 0, 1, 0, 120, "Click on the char to guard.");*//*
	}
	else if( ( comm.contains( " STOP" ) ) || ( comm.contains(" STAY") ) )
	{
		//pPlayer->setGuarded( false );
//		pPet->setWanderFollowTarget( INVALID_SERIAL );
		pPet->setCombatTarget( INVALID_SERIAL );

		if (pPet->isAtWar()) 
			pPet->toggleCombat();

		pPet->setWanderType( enHalt );
	}
	else if( comm.contains( " TRANSFER" ) )
	{
		//pPlayer->setGuarded( false );
		// >> LEGACY
		/*addx[s]=pPet->serial();
		target(s, 0, 1, 0, 119, "Select character to transfer your pet to.");*//*
	}
	else if( comm.contains( " RELEASE" ) )
	{
		//pPlayer->setGuarded( false );

		// Has it been summoned ? Let's dispel it
		if( pPet->summonTime() > uiCurrentTime )
			pPet->setSummonTime( uiCurrentTime );

//		pPet->setWanderFollowTarget( INVALID_SERIAL );
		pPet->setWanderType( enFreely );
		pPet->setOwner( NULL );
		pPet->setTamed( false );
		pPet->emote( tr( "%1 appears to have decided that it is better off without a master" ).arg( pPet->name() ) );
		if( SrvParams->tamedDisappear() ==1 )
		{
			pPet->soundEffect( 0x01FE );
			cCharStuff::DeleteChar( pPet );
		}
	}
}

static cNPC_AI* productCreator_AW()
{
	return new Animal_Wild();
}

void Animal_Wild::registerInFactory()
{
	AIFactory::instance()->registerType("Animal_Wild", productCreator_AW);

	// register its states
	Animal_Wild_Wander::registerInFactory();
	Animal_Wild_Combat::registerInFactory();
	Animal_Wild_Flee::registerInFactory();
}

Animal_Wild::Animal_Wild()
{ 
	currentState = new Animal_Wild_Wander( this, m_npc );
}

Animal_Wild::Animal_Wild( P_NPC currnpc )
{ 
	setNPC( currnpc ); 
	currentState = new Animal_Wild_Wander( this, m_npc );
}

void Animal_Wild::eventHandler()
{
	currentState->nextState = currentState;
	if( !m_npc->isDead() )
	{
		P_CHAR pVictim = World::instance()->findChar( m_npc->combatTarget() );
		if( !pVictim  || pVictim->isDead() )
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

		updateState();
		currentState->execute();
	}
}

static AbstractState* productCreator_AW_Wander()
{
	return new Animal_Wild_Wander();
}

void Animal_Wild_Wander::registerInFactory()
{
	StateFactory::instance()->registerType("Animal_Wild_Wander", productCreator_AW_Wander);
}

void Animal_Wild_Wander::attacked()
{
	reattack();
	nextState = new Animal_Wild_Combat( m_interface, npc );
}

void Animal_Wild_Wander::speechInput( P_PLAYER pTalker, const QString &message )
{
	if( !pTalker->socket() )
		return;

	if( npc->isTamed() && npc->owner() == pTalker && npc->inRange( pTalker, 4 ) && ( message.contains( npc->name() ) || message.contains( tr("ALL") ) ) )
	{
#pragma note("TODO: tamed animals speech handling here")
	}
}

void Animal_Wild_Wander::tameAttempt()
{
	TempEffects::instance()->insert( new cFleeReset( npc, WILD_ANIMALS_FLEE_TIME_ON_TAME ) ); 
	nextState = new Animal_Wild_Flee( m_interface, npc, true );
}

void Animal_Wild_Wander::execute()
{
	AbstractState_Wander::execute();
}

static AbstractState* productCreator_AW_Combat()
{
	return new Animal_Wild_Combat();
}

void Animal_Wild_Combat::registerInFactory()
{
	StateFactory::instance()->registerType("Animal_Wild_Combat", productCreator_AW_Combat);
}

void Animal_Wild_Combat::won()
{
	npc->setAttackerSerial( INVALID_SERIAL );
	npc->setCombatTarget( INVALID_SERIAL );
	nextState = new Animal_Wild_Wander( m_interface, npc );
}

void Animal_Wild_Combat::combatCancelled()
{
	npc->setAttackerSerial( INVALID_SERIAL );
	npc->setCombatTarget( INVALID_SERIAL );
	nextState = new Animal_Wild_Wander( m_interface, npc );
}

void Animal_Wild_Combat::hitpointsCritical()
{
	nextState = new Animal_Wild_Flee( m_interface, npc );
}

static AbstractState* productCreator_AW_Flee()
{
	return new Animal_Wild_Flee();
}

void Animal_Wild_Flee::registerInFactory()
{
	StateFactory::instance()->registerType("Animal_Wild_Flee", productCreator_AW_Flee);
}

void Animal_Wild_Flee::won()
{
	if( !m_fleeingDueTame )
	{
		npc->setAttackerSerial( INVALID_SERIAL );
		npc->setCombatTarget( INVALID_SERIAL );
		nextState = new Animal_Wild_Wander( m_interface, npc );
	}
}

void Animal_Wild_Flee::combatCancelled()
{
	if( !m_fleeingDueTame )
	{
		npc->setAttackerSerial( INVALID_SERIAL );
		npc->setCombatTarget( INVALID_SERIAL );
		nextState = new Animal_Wild_Wander( m_interface, npc );
	}
}

void Animal_Wild_Flee::hitpointsRestored()
{
	if( !m_fleeingDueTame )
	{
		nextState = new Animal_Wild_Combat( m_interface, npc );
	}
}

void Animal_Wild_Flee::ceaseFlee()
{
	nextState = new Animal_Wild_Wander( m_interface, npc );
}

*/

