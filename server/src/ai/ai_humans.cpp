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
#include "../items.h"
#include "../network/uosocket.h"
#include "../speech.h"
#include "../targetrequests.h"
#include "../timers.h"
#include "../serverconfig.h"
#include "../console.h"

#include "../world.h"
#include "../basics.h"
#include "../inlines.h"
#include "../mapobjects.h"

// library includes
#include <math.h>

void Human_Vendor::registerInFactory()
{
#ifndef __VC6
	AIFactory::instance()->registerType( "Human_Vendor", productCreatorFunctor<Human_Vendor> );
#else
	AIFactory::instance()->registerType( "Human_Vendor", productCreatorFunctor_Human_Vendor );
#endif
}

void Human_Vendor::onSpeechInput( P_PLAYER pTalker, const QString& comm )
{
	if ( !pTalker->socket() )
		return;

	if ( m_npc->inRange( pTalker, 4 ) && VendorChkName( m_npc, comm ) )
	{
		if ( comm.contains( " BUY" ) )
		{
			m_npc->vendorBuy( pTalker );
		}
		else if ( comm.contains( " SELL" ) )
		{
			m_npc->vendorSell( pTalker );
		}
	}
}

Human_Stablemaster::Human_Stablemaster( P_NPC npc ) : Human_Vendor( npc )
{
	notorietyOverride_ = 1;
	m_actions.append( new Action_Wander( npc, this ) );
	m_actions.append( new Action_FleeAttacker( npc, this ) );
}

void Human_Stablemaster::init( P_NPC npc )
{
	AbstractAI::init( npc );
}

void Human_Stablemaster::registerInFactory()
{
#ifndef __VC6
	AIFactory::instance()->registerType( "Human_Stablemaster", productCreatorFunctor<Human_Stablemaster> );
#else
	AIFactory::instance()->registerType( "Human_Vendor", productCreatorFunctor_Human_Stablemaster );
#endif
}

void Human_Stablemaster::onSpeechInput( P_PLAYER pTalker, const QString& message )
{
	Human_Vendor::onSpeechInput( pTalker, message );
	if ( !pTalker->socket() )
		return;

	if ( m_npc->inRange( pTalker, 4 ) && ( VendorChkName( m_npc, message ) || message.contains( "STABLEMASTER" ) ) )
	{
		if ( message.contains( " STABLE" ) )
		{
			m_npc->talk( 1042558 ); /* I charge 30 gold per pet for a real week's stable time.
									*  I will withdraw it from thy bank account.
									*  Which animal wouldst thou like to stable here?
									*/
			pTalker->socket()->attachTarget( new cStableTarget( m_npc ) );
		}
		else if ( message.contains( " CLAIM" ) )
		{
			P_ITEM pPack = m_npc->getBankbox();
			
			QPtrList<cItem> stableitems;
			if ( pPack )
			{				
				for (ContainerIterator it(pPack); !it.atEnd(); ++it) {
					if ( !( *it )->hasTag( "player" ) || !( *it )->hasTag( "pet" ) )
						continue;

					if ( ( *it ) && ( uint )( *it )->getTag( "player" ).toInt() == pTalker->serial() )
						stableitems.append( ( *it ) );
				}
			}

			if ( !stableitems.isEmpty() )
			{
				for (P_ITEM pItem = stableitems.first(); pItem; pItem = stableitems.next()) {
					P_NPC pPet = dynamic_cast<P_NPC>( World::instance()->findChar( pItem->getTag( "pet" ).toInt() ) );
					if ( pPet ) {
						if (pTalker->pets().count() + pPet->controlSlots() > pTalker->maxControlSlots()) {
							m_npc->talk(1049612, pPet->name());
						} else {
							pPet->free = false;
							// we need this for db saves
							pPet->setStablemasterSerial( INVALID_SERIAL );
							pPet->setOwner( pTalker ); // This is important...
							pPet->moveTo( m_npc->pos() );
							pPet->resend();
						}
					}
					pItem->remove();
				}

				pPack->update();
				m_npc->talk( 1042559 ); // Here you go... and good day to you!;
			} else {
				m_npc->talk( 502671 ); // But I have no animals stabled with me at the moment!
			}
		}
	}
}

void Human_Stablemaster::refreshStock()
{
}

void Human_Stablemaster::handleTargetInput( P_PLAYER player, cUORxTarget* target )
{
	if ( !player )
		return;

	P_ITEM pPack = m_npc->getBankbox();
	if ( !pPack )
		return;

	if ( player->serial() == target->serial() )
	{
		m_npc->talk( 502672 ); // HA HA HA! Sorry, I am not an inn.
		return;
	}

	P_NPC pPet = dynamic_cast<P_NPC>( World::instance()->findChar( target->serial() ) );
	if ( !pPet )
	{
		m_npc->talk( 502672 ); // HA HA HA! Sorry, I am not an inn.
	}
	else if ( pPet->owner() != player )
	{
		m_npc->talk( 1042562 ); // You do not own that pet!
	}
	else if ( pPet->isHuman() )
	{
		m_npc->talk( 502672 ); // HA HA HA! Sorry, I am not an inn.
	}
	else if ( pPet->isAtWar() )
	{
		m_npc->talk( 1042564 ); // I'm sorry.  Your pet seems to be busy.
	}
	else 
	{
		if ( player->takeGold( 30, true ) == 30 )
		{
			// we spawn a worldgem in the stablemasters bankbox for the pet
			// it does only hold the serial of it, the serial of the owner and the
			// number of refresh signals since begin of stabling
			// the pet becomes "free", which means, that it isnt in the world
			// but will still be saved.
			P_ITEM pGem = new cItem();
			pGem->Init( true );
			pGem->setTag( "player", cVariant( player->serial() ) );
			pGem->setTag( "pet", cVariant( pPet->serial() ) );
			pGem->setId( 0x1ea7 );
			pGem->setName( tr( "petitem: %1" ).arg( pPet->name() ) );
			pGem->setVisible( 2 ); // gm visible
			pPack->addItem( pGem );
			pGem->update();

			//pPet->free = true;
			pPet->setStablemasterSerial( this->m_npc->serial() );
			pPet->setOwner(0); // Remove ownership from this player since it's stabled
			pPet->removeFromView();
			m_npc->talk( 502679 ); // Very well, thy pet is stabled. Thou mayst recover it by saying 'claim' to me. In one real world week, I shall sell it off if it is not claimed!
		}
		else
			m_npc->talk( 502677 ); // But thou hast not the funds in thy bank account!
	}
}

void Human_Guard_Called::registerInFactory()
{
#ifndef __VC6
	AIFactory::instance()->registerType( "Human_Guard_Called", productCreatorFunctor<Human_Guard_Called> );
#else
	AIFactory::instance()->registerType( "Human_Guard_Called", productCreatorFunctor_Human_Guard_Called );
#endif
}

Human_Guard_Called::Human_Guard_Called( P_NPC npc ) : AbstractAI( npc )
{
	notorietyOverride_ = 1;
	m_actions.append( new Human_Guard_Called_Fight( npc, this ) );
	m_actions.append( new Human_Guard_Called_TeleToTarget( npc, this ) );
	m_actions.append( new Human_Guard_Called_Disappear( npc, this ) );
}

void Human_Guard_Called::init( P_NPC npc )
{
	npc->setSummonTime( Server::instance()->time() + MY_CLOCKS_PER_SEC * Config::instance()->guardDispelTime() );
	npc->setSummoned( true );
	AbstractAI::init( npc );
}

void Human_Guard_Called_Fight::execute()
{
	// talk only in about every 10th check
	switch ( RandomNum( 0, 20 ) )
	{
	case 0:
		m_npc->talk( tr( "Thou shalt regret thine actions, swine!" ), 0xFFFF, 0, true );	break;
	case 1:
		m_npc->talk( tr( "Death to all Evil!" ), 0xFFFF, 0, true );						break;
	}

	m_npc->setSummonTime( Server::instance()->time() + MY_CLOCKS_PER_SEC * Config::instance()->guardDispelTime() );
	m_npc->setSummoned( true );

	// Fighting is handled within combat..
}

float Human_Guard_Called_Fight::preCondition()
{
	P_CHAR pTarget = m_npc->attackTarget();

	if ( !pTarget || pTarget->isDead() || pTarget->isInnocent() || pTarget->region() != m_npc->region() )
		return 0.0f;

	if ( pTarget && m_npc->dist( pTarget ) < 2 )
		return 1.0f;
	else
		return 0.0f;
}

float Human_Guard_Called_Fight::postCondition()
{
	return 1.0f - preCondition();
}

void Human_Guard_Called_TeleToTarget::execute()
{
	m_npc->setSummonTime( Server::instance()->time() + MY_CLOCKS_PER_SEC * Config::instance()->guardDispelTime() );
	m_npc->setSummoned( true );

	// Teleports the guard towards the target
	P_CHAR pTarget = m_npc->attackTarget();
	if ( pTarget )
	{
		m_npc->moveTo( pTarget->pos() );
		m_npc->soundEffect( 0x1FE );
		m_npc->effect( 0x372A, 0x09, 0x06 );

		m_npc->resend( false );
		pTarget->kill( m_npc );
	}
}

float Human_Guard_Called_TeleToTarget::preCondition()
{
	P_CHAR pTarget = m_npc->attackTarget();
	if ( !pTarget || pTarget->isDead() || pTarget->isInnocent() || pTarget->region() != m_npc->region() )
		return 0.0f;

	if ( pTarget && m_npc->dist( pTarget ) >= 2 )
		return 1.0f;
	else
		return 0.0f;
}

float Human_Guard_Called_TeleToTarget::postCondition()
{
	return 1.0f - preCondition();
}

void Human_Guard_Called_Disappear::execute()
{
	// nothing to do
}

float Human_Guard_Called_Disappear::preCondition()
{
	P_CHAR pTarget = m_npc->attackTarget();
	if ( !pTarget || pTarget->isDead() || pTarget->isInnocent() || pTarget->region() != m_npc->region() )
		return 1.0f;

	return 0.0f;
}

float Human_Guard_Called_Disappear::postCondition()
{
	return 1.0f - preCondition();
}

void Human_Guard::registerInFactory()
{
#ifndef __VC6
	AIFactory::instance()->registerType( "Human_Guard", productCreatorFunctor<Human_Guard> );
#else
	AIFactory::instance()->registerType( "Human_Guard", productCreatorFunctor_Human_Guard );
#endif
}

Human_Guard::Human_Guard( P_NPC npc ) : AbstractAI( npc ), m_currentVictimSer( INVALID_SERIAL )
{
	notorietyOverride_ = 1;
	m_actions.append( new Human_Guard_Wander( npc, this ) );
	m_actions.append( new Human_Guard_MoveToTarget( npc, this ) );
	m_actions.append( new Human_Guard_Fight( npc, this ) );
}

void Human_Guard::check()
{
	selectVictim();
	AbstractAI::check();
}

void Human_Guard::selectVictim()
{
	P_CHAR m_currentVictim = World::instance()->findChar(m_currentVictimSer);

	if (!m_currentVictim) {
		m_currentVictimSer = INVALID_SERIAL;
	}

	if ( m_currentVictim )
	{
		// Check if the current target is valid, including:
		// - Target not dead.
		// - Target in attack range.
		// - Target not innocent.
		if ( m_currentVictim->isDead() || m_currentVictim->isInnocent() ) {
			m_currentVictim = NULL;
			m_currentVictimSer = INVALID_SERIAL;
		} else if ( !m_npc->inRange( m_currentVictim, Config::instance()->attack_distance() ) ) {
			m_currentVictim = NULL;
			m_currentVictimSer = INVALID_SERIAL;
		}
	}

	if ( !m_currentVictim )
	{
		// Get a criminal or murderer in range to attack it
		MapCharsIterator ri = MapObjects::instance()->listCharsInCircle( m_npc->pos(), VISRANGE );
		for ( P_CHAR pChar = ri.first(); pChar; pChar = ri.next() )
		{
			if ( pChar && !pChar->free && pChar != m_npc && !pChar->isInvulnerable() && !pChar->isHidden() && !pChar->isInvisible() && !pChar->isDead() )
			{
				// If its a NPC... special handling
				P_NPC pNpc = dynamic_cast<P_NPC>(pChar);

				// If the character has a checkvictim processing function, use that instead
				if (m_npc->canHandleEvent(EVENT_CHECKVICTIM)) {
					if (pNpc && pNpc->owner()) {
						pChar = pNpc->owner();
					}

					PyObject *args = Py_BuildValue("(NNi)", m_npc->getPyObject(), pChar->getPyObject(), pChar->dist(m_npc));
					bool result = m_npc->callEventHandler(EVENT_CHECKVICTIM, args);
					Py_DECREF(args);

					if (!result) {
						continue;
					}
				} else {
					// NPCs owned by innocent players aren't attacked
					if (pNpc) {
						if (pNpc->isTamed() && pNpc->owner()) {
							if ((pNpc->owner()->isInnocent() || pNpc->owner()->isGMorCounselor())) {
								continue;
							}
						} else {
							// Check for the AI, guards only attack other npcs if they
							// are monsters.
							Monster_Aggressive *npcai = dynamic_cast<Monster_Aggressive*>(pNpc->ai());
							if (!npcai) {
								continue;
							}
						}
					} else {
						// Innocent players aren't attacked
						P_PLAYER pPlayer = dynamic_cast<P_PLAYER>( pChar );
						if ( pPlayer && (pPlayer->isInnocent() || pPlayer->isGMorCounselor()) )
							continue;
					}
				}

				m_currentVictim = pChar;
				m_currentVictimSer = pChar->serial();
				break;
			}
		}

		// If we found a new target, let us attack it
		if ( m_currentVictim )
			m_npc->fight( m_currentVictim );
	}
}

void Human_Guard_Fight::execute()
{
	// talk only in about every 10th check
	switch ( RandomNum( 0, 20 ) )
	{
	case 0:
		m_npc->talk( tr( "Thou shalt regret thine actions, swine!" ), 0xFFFF, 0, true );	break;
	case 1:
		m_npc->talk( tr( "Death to all Evil!" ), 0xFFFF, 0, true );						break;
	default:
		break;
	}
}

float Human_Guard_Fight::preCondition()
{
	Human_Guard* pAI = dynamic_cast<Human_Guard*>( m_ai );
	P_CHAR pTarget = ( pAI ? pAI->currentVictim() : NULL );

	if ( !pTarget || pTarget->isDead() || pTarget->isHidden() || pTarget->isInvisible() )
		return 0.0f;

	if ( pTarget && m_npc->dist( pTarget ) < 2 )
		return 1.0f;
	else
		return 0.0f;
}

float Human_Guard_Fight::postCondition()
{
	return 1.0f - preCondition();
}

void Human_Guard_MoveToTarget::execute()
{
	Human_Guard* pAI = dynamic_cast<Human_Guard*>( m_ai );
	P_CHAR pTarget = ( pAI ? pAI->currentVictim() : NULL );


	// Make sure we are attacking the target
	// If there is no target, stop.
	m_npc->fight( pTarget );

	if ( !pTarget )
		return;

	// Guards always run
	bool run = true;

	if ( Config::instance()->pathfind4Combat() )
		movePath( pTarget->pos(), run );
	else
		moveTo( pTarget->pos(), run );
}

float Human_Guard_MoveToTarget::preCondition()
{
	Human_Guard* pAI = dynamic_cast<Human_Guard*>( m_ai );
	P_CHAR pTarget = ( pAI ? pAI->currentVictim() : NULL );

	if ( !pTarget || pTarget->isDead() || pTarget->isHidden() || pTarget->isInvisible() )
		return 0.0f;

	if ( pTarget && m_npc->dist( pTarget ) >= 2 )
		return 1.0f;
	else
		return 0.0f;
}

float Human_Guard_MoveToTarget::postCondition()
{
	return 1.0f - preCondition();
}

float Human_Guard_Wander::preCondition()
{
	Human_Guard* pAI = dynamic_cast<Human_Guard*>( m_ai );
	P_CHAR pTarget = ( pAI ? pAI->currentVictim() : NULL );

	if ( !pTarget || pTarget->isDead() || pTarget->isHidden() || pTarget->isInvisible() )
		return 1.0f;

	return 0.0f;
}

float Human_Guard_Wander::postCondition()
{
	return 1.0f - preCondition();
}
