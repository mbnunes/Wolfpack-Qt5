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
#include "../network/uosocket.h"
#include "../speech.h"
#include "../targetrequests.h"
#include "../TmpEff.h"
#include "../srvparams.h"
#include "../globals.h"
#include "../sectors.h"
#include "../world.h"
#include "../basics.h"

// library includes
#include <math.h>

static AbstractAI* productCreator_HV()
{
	return new Human_Vendor(0);
}

void Human_Vendor::registerInFactory()
{
	AIFactory::instance()->registerType("Human_Vendor", productCreator_HV);
}

void Human_Vendor::onSpeechInput( P_PLAYER pTalker, const QString &comm )
{
	if( !pTalker->socket() )
		return;

	if( m_npc->inRange( pTalker, 4 ) && VendorChkName( m_npc, comm ) )
	{
		if( comm.contains( tr(" BUY") ) )
		{
			m_npc->vendorBuy( pTalker );
		}
		else if( comm.contains( tr(" SELL") ) )
		{
			m_npc->vendorSell( pTalker );
		}
	}
}

Human_Stablemaster::Human_Stablemaster( P_NPC npc ) : AbstractAI( npc )
{
	notorietyOverride_ = 1;
	m_actions.append( new Action_Wander( npc, this ) );
	m_actions.append( new Action_FleeAttacker( npc, this ) );
}

void Human_Stablemaster::init( P_NPC npc )
{
	AbstractAI::init( npc );
}

static AbstractAI* productCreator_HS()
{
	return new Human_Stablemaster( NULL );
}

void Human_Stablemaster::registerInFactory()
{
	AIFactory::instance()->registerType("Human_Stablemaster", productCreator_HS);
}

void Human_Stablemaster::onSpeechInput( P_PLAYER pTalker, const QString &message )
{
	if( !pTalker->socket() )
		return;

	if( m_npc->inRange( pTalker, 4 ) && ( VendorChkName( m_npc, message ) || message.contains( tr("STABLEMASTER") ) ) )
	{
		if( message.contains( tr(" STABLE") ) )
		{
			m_npc->talk( tr("Which pet do you want me to stable?") );
			pTalker->socket()->attachTarget( new cStableTarget( m_npc ) );
		}
		else if( message.contains( tr(" RELEASE") ) )
		{
			int gold = pTalker->CountBankGold() + pTalker->CountGold();
			P_ITEM pPack = m_npc->getBankbox();
			cItem::ContainerContent stableitems;
			if( pPack )
			{
				cItem::ContainerContent content = pPack->content();
				cItem::ContainerContent::const_iterator it( content.begin() );
				while( it != content.end() )
				{
					if( !(*it)->hasTag( "player" ) || !(*it)->hasTag( "pet" ) )
						continue;

					if( (*it) && (*it)->getTag( "player" ).toInt() == pTalker->serial() )
						stableitems.push_back( (*it) );
					++it;
				}
			}

			if( !stableitems.empty() )
			{
				cItem::ContainerContent::const_iterator it( stableitems.begin() );
				while( it != stableitems.end() )
				{
					if( (*it) )
					{
						P_NPC pPet = dynamic_cast<P_NPC>(World::instance()->findChar( (*it)->getTag( "pet" ).toInt() ));
						if( pPet )
						{
							pPet->free = false;
							pPet->moveTo( m_npc->pos() );
							pPet->resend();
						}
						(*it)->remove();
					}
					++it;
				}

				pPack->update();
				m_npc->talk( tr("Here's your pet back!") );
			}
		}
	}
}

void Human_Stablemaster::refreshStock()
{
}

void Human_Stablemaster::handleTargetInput( P_PLAYER player, cUORxTarget *target )
{
	if( !player )
		return;

	P_ITEM pPack = m_npc->getBankbox();
	if( !pPack )
		return;

	P_NPC pPet = dynamic_cast< P_NPC >(World::instance()->findChar( target->serial() ));
	if( !pPet )
	{
		m_npc->talk( tr("I cannot stable that!" ) );
		return;
	}

	if( pPet->owner() != player )
	{
		m_npc->talk( tr("This does not belong to you!" ) );
		return;
	}

	// we spawn a worldgem in the stablemasters bankbox for the pet
	// it does only hold the serial of it, the serial of the owner and the
	// number of refresh signals since begin of stabling
	// the pet becomes "free", which means, that it isnt in the world
	// but will still be saved.
	P_ITEM pGem = new cItem();
	pGem->Init( false );
	pGem->setTag( "player", cVariant( player->serial() ) );
	pGem->setTag( "pet", cVariant( pPet->serial() ) );
	pGem->setId( 0x1ea7 );
	pGem->setName( tr("petitem: %1").arg(pPet->name()) );
	pGem->setVisible( 2 ); // gm visible
	pPack->addItem( pGem );
	pGem->update();

	pPet->free = true;
	MapObjects::instance()->remove( pPet );
	pPet->removeFromView();

	m_npc->talk( tr("Say release to get your pet back!") );
}

static AbstractAI* productCreator_HGC()
{
	return new Human_Guard_Called( NULL );
}

void Human_Guard_Called::registerInFactory()
{
	AIFactory::instance()->registerType("Human_Guard_Called", productCreator_HGC);
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
	npc->setSummonTime( uiCurrentTime + MY_CLOCKS_PER_SEC * SrvParams->guardDispelTime() );
	npc->setSummoned(true);
	AbstractAI::init( npc );
}

void Human_Guard_Called_Fight::execute()
{
	// talk only in about every 10th check
	switch( RandomNum( 0, 20 ) )
	{
		case 0:		m_npc->talk( tr( "Thou shalt regret thine actions, swine!" ), -1, 0, true );	break;
		case 1:		m_npc->talk( tr( "Death to all Evil!" ), -1, 0, true );						break;
	}

	m_npc->setSummonTime( uiCurrentTime + MY_CLOCKS_PER_SEC * SrvParams->guardDispelTime() );
	m_npc->setSummoned(true);

	// Fighting is handled within combat..
}

float Human_Guard_Called_Fight::preCondition() {
	P_CHAR pTarget = m_npc->attackTarget();

	if( !pTarget || pTarget->isDead() || pTarget->isInnocent() || pTarget->region() != m_npc->region() )
		return 0.0f;

	if( pTarget && m_npc->dist( pTarget ) < 2 )
		return 1.0f;
	else
		return 0.0f;
}

float Human_Guard_Called_Fight::postCondition()
{
	return 1.0f - preCondition();
}

void Human_Guard_Called_TeleToTarget::execute() {
	m_npc->setSummonTime(uiCurrentTime + MY_CLOCKS_PER_SEC * SrvParams->guardDispelTime());
	m_npc->setSummoned(true);

	// Teleports the guard towards the target
	P_CHAR pTarget = m_npc->attackTarget();
	if( pTarget )
	{
		m_npc->moveTo( pTarget->pos() );
		m_npc->soundEffect( 0x1FE );
		m_npc->effect( 0x372A, 0x09, 0x06 );

		m_npc->resend( false );
	}
}

float Human_Guard_Called_TeleToTarget::preCondition() {
	P_CHAR pTarget = m_npc->attackTarget();
	if( !pTarget || pTarget->isDead() || pTarget->isInnocent() || pTarget->region() != m_npc->region() )
		return 0.0f;

	if( pTarget && m_npc->dist( pTarget ) >= 2 )
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

float Human_Guard_Called_Disappear::preCondition() {
	P_CHAR pTarget = m_npc->attackTarget();
	if( !pTarget || pTarget->isDead() || pTarget->isInnocent() || pTarget->region() != m_npc->region() )
		return 1.0f;

	return 0.0f;
}

float Human_Guard_Called_Disappear::postCondition()
{
	return 1.0f - preCondition();
}

static AbstractAI* productCreator_HG()
{
	return new Human_Guard( NULL );
}

void Human_Guard::registerInFactory()
{
	AIFactory::instance()->registerType("Human_Guard", productCreator_HG);
}

Human_Guard::Human_Guard( P_NPC npc ) : AbstractAI( npc ), m_currentVictim( NULL )
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
	if( m_currentVictim )
	{
		// Check if the current target is valid, including:
		// - Target not dead.
		// - Target in attack range.
		// - Target not innocent.
		if( m_currentVictim->isDead() || m_currentVictim->isInnocent() )
			m_currentVictim = NULL;
		else if( !m_npc->inRange( m_currentVictim, SrvParams->attack_distance() ) )
			m_currentVictim = NULL;
	}

	if( !m_currentVictim )
	{
		// Get a criminal or murderer in range to attack it
		RegionIterator4Chars ri( m_npc->pos(), VISRANGE );
		for( ri.Begin(); !ri.atEnd(); ri++ )
		{
			P_CHAR pChar = ri.GetData();
			if( pChar && !pChar->free && pChar != m_npc && !pChar->isInvulnerable() && !pChar->isHidden() && !pChar->isInvisible() && !pChar->isDead() && !pChar->isInnocent() )
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

void Human_Guard_Fight::execute()
{
	// talk only in about every 10th check
	switch( RandomNum( 0, 20 ) )
	{
		case 0:		m_npc->talk( tr( "Thou shalt regret thine actions, swine!" ), -1, 0, true );	break;
		case 1:		m_npc->talk( tr( "Death to all Evil!" ), -1, 0, true );						break;
	}
}

float Human_Guard_Fight::preCondition()
{
	Human_Guard* pAI = dynamic_cast< Human_Guard* >(m_ai);
	P_CHAR pTarget = ( pAI ? pAI->currentVictim() : NULL );

	if( !pTarget || pTarget->isDead() || pTarget->isInnocent() )
		return 0.0f;

	if( pTarget && m_npc->dist( pTarget ) < 2 )
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
	Human_Guard* pAI = dynamic_cast< Human_Guard* >(m_ai);
	P_CHAR pTarget = ( pAI ? pAI->currentVictim() : NULL );


	// Make sure we are attacking the target
	// If there is no target, stop.
	m_npc->fight(pTarget);

	if( !pTarget )
		return;

	if( SrvParams->pathfind4Combat() )
		movePath( pTarget->pos() );
	else
		moveTo( pTarget->pos() );
}

float Human_Guard_MoveToTarget::preCondition()
{
	Human_Guard* pAI = dynamic_cast< Human_Guard* >(m_ai);
	P_CHAR pTarget = ( pAI ? pAI->currentVictim() : NULL );

	if( !pTarget || pTarget->isDead() || pTarget->isInnocent() )
		return 0.0f;

	if( pTarget && m_npc->dist( pTarget ) >= 2 )
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
	Human_Guard* pAI = dynamic_cast< Human_Guard* >(m_ai);
	P_CHAR pTarget = ( pAI ? pAI->currentVictim() : NULL );

	if( !pTarget || pTarget->isDead() || pTarget->isInnocent() )
		return 1.0f;

	return 0.0f;
}

float Human_Guard_Wander::postCondition()
{
	return 1.0f - preCondition();
}
