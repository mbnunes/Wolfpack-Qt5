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
#include "trade.h"

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
	return new Monster_Aggr_L0_Wander();
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

	if( npc->inRange( pTalker, 4 ) && ( message.contains( pTalker->name().upper() ) || message.contains( "VENDOR" ) ) )
	{
		if( message.contains( "BUY" ) )
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
		else if( message.contains( "SELL" ) )
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
	Trade->buyaction( pPlayer->socket(), packet );
//	nextState = new Human_Vendor_Wander( m_interface, npc );
}

void Human_Vendor_Wander::handleSelection( P_PLAYER pPlayer, cUORxSell* packet )
{
#pragma note( "Implement handling of incoming cUORxSell packet!" )
//	nextState = new Human_Vendor_Wander( m_interface, npc );
}

void Human_Vendor_Wander::execute()
{
	// wander freely
	wanderFreely();
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

void Human_Vendor_Combat::execute()
{
	P_CHAR pTarget = World::instance()->findChar( npc->combatTarget() );
	if( !pTarget )
	{
		won();
		return;
	}

	UINT8 range = 1;
	if( npc->rightHandItem() && IsBowType( npc->rightHandItem()->id() ) )
		range = ARCHERY_RANGE;

	if( !npc->inRange( pTarget, range ) )
	{ // move towards the target
		if( SrvParams->pathfind4Combat() )
			movePath( pTarget->pos() );
		else
			moveTo( pTarget->pos() );
	}
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

void Human_Vendor_Flee::execute()
{
	if( !npc->hasPath() )
	{
		Coord_cl newPos = npc->pos();
		// find a valid spot in a circle of flee_radius fields to move to
		float rnddist = (float)RandomNum( 1, SrvParams->pathfindFleeRadius() );
		// now get a point on this circle around the npc
		float rndphi = (float)RandomNum( 0, 100 ) / 100.0f * 2.0f * 3.14159265358979323846f;
		newPos.x = newPos.x + (INT16)floor( cos( rndphi ) * rnddist );
		newPos.y = newPos.y + (INT16)floor( sin( rndphi ) * rnddist );

		// we use pathfinding for fleeing
		movePath( newPos );
	}
	else
		movePath();
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
	Trade->buyaction( pPlayer->socket(), packet );
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
