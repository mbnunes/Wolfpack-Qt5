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
#include "world.h"
#include "itemid.h"
#include "items.h"
#include "srvparams.h"
#include "basics.h"
#include "globals.h"
#include "inlines.h"
#include "debug.h"
#include "player.h"
#include "network/uosocket.h"
#include "network/uorxpackets.h"
#include "speech.h"
#include "targetrequests.h"
#include "Trade.h"
#include "TmpEff.h"

#include <math.h>

static cNPC_AI* productCreator_HV()
{
	return new Human_Vendor();
}

void Human_Vendor::registerInFactory()
{
	AIFactory::instance()->registerType("Human_Vendor", productCreator_HV);

	// register its states
	Human_Vendor_Wander::registerInFactory();
	Human_Vendor_Combat::registerInFactory();
	Human_Vendor_Flee::registerInFactory();
//	Human_Vendor_BuyQuery::registerInFactory();
//	Human_Vendor_SellQuery::registerInFactory();
}

Human_Vendor::Human_Vendor()
{ 
	currentState = new Human_Vendor_Wander( this, m_npc );
}

Human_Vendor::Human_Vendor( P_NPC currnpc )
{ 
	setNPC( currnpc ); 
	currentState = new Human_Vendor_Wander( this, m_npc );
}

void Human_Vendor::eventHandler()
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

static AbstractState* productCreator_HV_Wander()
{
	return new Human_Vendor_Wander();
}

void Human_Vendor_Wander::registerInFactory()
{
	StateFactory::instance()->registerType("Human_Vendor_Wander", productCreator_HV_Wander);
}

void Human_Vendor_Wander::attacked()
{
	callGuards();
	reattack();
	nextState = new Human_Vendor_Combat( m_interface, npc );
}

void Human_Vendor_Wander::speechInput( P_PLAYER pTalker, const QString &message )
{
	if( !pTalker->socket() )
		return;

	if( npc->inRange( pTalker, 4 ) && ( VendorChkName( npc, message ) || message.contains( tr("VENDOR") ) ) )
	{
		if( message.contains( tr(" BUY") ) )
		{
			P_ITEM pContA = npc->GetItemOnLayer( cBaseChar::BuyRestockContainer );
			P_ITEM pContB = npc->GetItemOnLayer( cBaseChar::BuyNoRestockContainer );

			npc->turnTo( pTalker );

			if( !pContA && !pContB )
			{
				npc->talk( tr( "Sorry but i have no goods to sell" ) );
				return;
			}

			npc->talk( tr( "Take a look at my wares!" ) );
			pTalker->socket()->sendBuyWindow( npc );

//			nextState = new Human_Vendor_BuyQuery( m_interface, npc );
		}
		else if( message.contains( tr(" SELL") ) )
		{
			P_ITEM pContC = npc->GetItemOnLayer( cBaseChar::SellContainer );

			npc->turnTo( pTalker );

			if( !pContC )
			{
				npc->talk( tr( "Sorry, I cannot use any of your wares!" ) );
				return;
			}

			npc->talk( tr( "This could be of interest!" ) );
			pTalker->socket()->sendSellWindow( npc, pTalker );

//			nextState = new Human_Vendor_SellQuery( m_interface, npc );
		}
	}
}

void Human_Vendor_Wander::handleSelection( P_PLAYER pPlayer, cUORxBuy* packet )
{
	Trade::buyAction( pPlayer->socket(), packet );
//	nextState = new Human_Vendor_Wander( m_interface, npc );
}

void Human_Vendor_Wander::handleSelection( P_PLAYER pPlayer, cUORxSell* packet )
{
	Trade::sellAction( pPlayer->socket(), packet );
//	nextState = new Human_Vendor_Wander( m_interface, npc );
}

static AbstractState* productCreator_HV_Combat()
{
	return new Human_Vendor_Combat();
}

void Human_Vendor_Combat::registerInFactory()
{
	StateFactory::instance()->registerType("Human_Vendor_Combat", productCreator_HV_Combat);
}

void Human_Vendor_Combat::won()
{
	npc->setAttackerSerial( INVALID_SERIAL );
	npc->setCombatTarget( INVALID_SERIAL );
	nextState = new Human_Vendor_Wander( m_interface, npc );
}

void Human_Vendor_Combat::combatCancelled()
{
	npc->setAttackerSerial( INVALID_SERIAL );
	npc->setCombatTarget( INVALID_SERIAL );
	nextState = new Human_Vendor_Wander( m_interface, npc );
}

void Human_Vendor_Combat::hitpointsCritical()
{
	// call again
	callGuards();
	nextState = new Human_Vendor_Flee( m_interface, npc );
}

static AbstractState* productCreator_HV_Flee()
{
	return new Human_Vendor_Flee();
}

void Human_Vendor_Flee::registerInFactory()
{
	StateFactory::instance()->registerType("Human_Vendor_Flee", productCreator_HV_Flee);
}

void Human_Vendor_Flee::won()
{
	npc->setAttackerSerial( INVALID_SERIAL );
	npc->setCombatTarget( INVALID_SERIAL );
	nextState = new Human_Vendor_Wander( m_interface, npc );
}

void Human_Vendor_Flee::combatCancelled()
{
	npc->setAttackerSerial( INVALID_SERIAL );
	npc->setCombatTarget( INVALID_SERIAL );
	nextState = new Human_Vendor_Wander( m_interface, npc );
}

void Human_Vendor_Flee::hitpointsRestored()
{
	nextState = new Human_Vendor_Combat( m_interface, npc );
}

/*
static AbstractState* productCreator_HV_BuyQuery()
{
	return new Human_Vendor_BuyQuery();
}

void Human_Vendor_BuyQuery::registerInFactory()
{
	StateFactory::instance()->registerType("Human_Vendor_BuyQuery", productCreator_HV_BuyQuery);
}

void Human_Vendor_BuyQuery::handleSelection( P_PLAYER pPlayer, cUORxBuy* packet )
{
	Trade::buyaction( pPlayer->socket(), packet );
	nextState = new Human_Vendor_Wander( m_interface, npc );
}

void Human_Vendor_BuyQuery::selectionCancelled()
{
	npc->talk( tr( "As you wish. Farewell!" ) );
	nextState = new Human_Vendor_Wander( m_interface, npc );
}

void Human_Vendor_BuyQuery::selectionTimeOut()
{
	npc->talk( tr( "I do not have time the whole day for a single trade! Come later, when you know what you want!" ) );
	nextState = new Human_Vendor_Wander( m_interface, npc );
}

static AbstractState* productCreator_HV_SellQuery()
{
	return new Human_Vendor_SellQuery();
}

void Human_Vendor_SellQuery::registerInFactory()
{
	StateFactory::instance()->registerType("Human_Vendor_SellQuery", productCreator_HV_SellQuery);
}

void Human_Vendor_SellQuery::handleSelection( P_PLAYER pPlayer, cUORxBuy* packet )
{
#pragma note( "TODO: do item transaction here" )
	nextState = new Human_Vendor_Wander( m_interface, npc );
}

void Human_Vendor_SellQuery::selectionCancelled()
{
	npc->talk( tr( "As you wish. Farewell!" ) );
	nextState = new Human_Vendor_Wander( m_interface, npc );
}

void Human_Vendor_SellQuery::selectionTimeOut()
{
	npc->talk( tr( "I do not have time the whole day for a single trade! Come later, when you know what you want!" ) );
	nextState = new Human_Vendor_Wander( m_interface, npc );
}
*/

static cNPC_AI* productCreator_HS()
{
	return new Human_Vendor();
}

void Human_Stablemaster::registerInFactory()
{
	AIFactory::instance()->registerType("Human_Stablemaster", productCreator_HS);

	// register its states
	Human_Stablemaster_Wander::registerInFactory();
	Human_Stablemaster_Combat::registerInFactory();
	Human_Stablemaster_Flee::registerInFactory();
//	Human_Vendor_BuyQuery::registerInFactory();
//	Human_Vendor_SellQuery::registerInFactory();
}

Human_Stablemaster::Human_Stablemaster()
{ 
	currentState = new Human_Stablemaster_Wander( this, m_npc );

}

Human_Stablemaster::Human_Stablemaster( P_NPC currnpc )
{ 
	setNPC( currnpc ); 
	currentState = new Human_Stablemaster_Wander( this, m_npc );
}

void Human_Stablemaster::eventHandler()
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

static AbstractState* productCreator_HS_Wander()
{
	return new Human_Stablemaster_Wander();
}

void Human_Stablemaster_Wander::registerInFactory()
{
	StateFactory::instance()->registerType("Human_Stablemaster_Wander", productCreator_HS_Wander);
}

void Human_Stablemaster_Wander::attacked()
{
	callGuards();
	reattack();
	nextState = new Human_Stablemaster_Combat( m_interface, npc );
}

void Human_Stablemaster_Wander::init()
{
	TempEffects::instance()->insert( new cAIRefreshTimer( npc, 60 ) );
}

void Human_Stablemaster_Wander::refresh()
{
//	cItem::ContainerContent 
#pragma note("TODO: gem refreshing" )

	TempEffects::instance()->insert( new cAIRefreshTimer( npc, 60 ) );
}

void Human_Stablemaster_Wander::speechInput( P_PLAYER pTalker, const QString &message )
{
	if( !pTalker->socket() )
		return;

	if( npc->inRange( pTalker, 4 ) && ( VendorChkName( npc, message ) || message.contains( tr(" VENDOR") ) ) )
	{
		if( message.contains( tr(" STABLE") ) )
		{
			npc->talk( tr("Which pet do you want me to stable?") );
			pTalker->socket()->attachTarget( new cStableTarget( npc ) );
		}
		else if( message.contains( tr(" RELEASE") ) )
		{
			int gold = pTalker->CountBankGold() + pTalker->CountGold();
#pragma note("TODO: pet releasing")
		}
	}
}

void Human_Stablemaster_Wander::targetCursorInput( cUOSocket *socket, cUORxTarget *target )
{
	if( !socket || !socket->player() )
		return;

	P_ITEM pPack = npc->getBackpack();
	if( !pPack )
		return;

	P_NPC pPet = dynamic_cast< P_NPC >(World::instance()->findChar( target->serial() ));
	if( !pPet )
	{
		npc->talk( tr("I cannot stable that!" ) );
		return;
	}

	if( pPet->owner() != socket->player() )
	{
		npc->talk( tr("This does not belong to you!" ) );
		return;
	}

	// we spawn a worldgem in the stablemasters backpack for the pet
	// it does only hold the serial of it, the serial of the owner and the
	// number of refresh signals since begin of stabling
	// the pet becomes "free", which means, that it isnt in the world
	// but will still be saved.
	P_ITEM pGem = new cItem();
	pGem->Init( false );
	pGem->setMoreX( pPet->serial() );
	pGem->setMoreY( socket->player()->serial() );
	pGem->setMoreZ( 0 );
	pGem->setId( 0x1ea7 );
	pGem->setName( tr("petitem: %1").arg(pPet->name()) );
	pGem->setName2( socket->player()->name() );
	pGem->setVisible( 2 ); // gm visible
	pGem->setContainer( pPack );
	pGem->update();

	npc->talk( tr("Say release to get your pet back!") );
}

static AbstractState* productCreator_HS_Combat()
{
	return new Human_Stablemaster_Combat();
}

void Human_Stablemaster_Combat::registerInFactory()
{
	StateFactory::instance()->registerType("Human_Stablemaster_Combat", productCreator_HS_Combat);
}

void Human_Stablemaster_Combat::won()
{
	npc->setAttackerSerial( INVALID_SERIAL );
	npc->setCombatTarget( INVALID_SERIAL );
	nextState = new Human_Stablemaster_Wander( m_interface, npc );
}

void Human_Stablemaster_Combat::combatCancelled()
{
	npc->setAttackerSerial( INVALID_SERIAL );
	npc->setCombatTarget( INVALID_SERIAL );
	nextState = new Human_Stablemaster_Wander( m_interface, npc );
}

void Human_Stablemaster_Combat::hitpointsCritical()
{
	// call again
	callGuards();
	nextState = new Human_Stablemaster_Flee( m_interface, npc );
}

static AbstractState* productCreator_HS_Flee()
{
	return new Human_Stablemaster_Flee();
}

void Human_Stablemaster_Flee::registerInFactory()
{
	StateFactory::instance()->registerType("Human_Stablemaster_Flee", productCreator_HS_Flee);
}

void Human_Stablemaster_Flee::won()
{
	npc->setAttackerSerial( INVALID_SERIAL );
	npc->setCombatTarget( INVALID_SERIAL );
	nextState = new Human_Stablemaster_Wander( m_interface, npc );
}

void Human_Stablemaster_Flee::combatCancelled()
{
	npc->setAttackerSerial( INVALID_SERIAL );
	npc->setCombatTarget( INVALID_SERIAL );
	nextState = new Human_Stablemaster_Wander( m_interface, npc );
}

void Human_Stablemaster_Flee::hitpointsRestored()
{
	nextState = new Human_Stablemaster_Combat( m_interface, npc );
}

