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
#include "network/uosocket.h"
#include "speech.h"
#include "targetrequests.h"
#include "TmpEff.h"
#include "srvparams.h"
#include "globals.h"
#include "sectors.h"

// library includes
#include <math.h>

static AbstractAI* productCreator_HV()
{
	return new Human_Vendor( NULL );
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
			P_ITEM pContA = m_npc->GetItemOnLayer( cBaseChar::BuyRestockContainer );
			P_ITEM pContB = m_npc->GetItemOnLayer( cBaseChar::BuyNoRestockContainer );

			m_npc->turnTo( pTalker );

			if( !pContA && !pContB )
			{
				m_npc->talk( tr( "Sorry but i have no goods to sell" ) );
				return;
			}

			m_npc->talk( tr( "Take a look at my wares!" ) );
			pTalker->socket()->sendBuyWindow( m_npc );
		}
		else if( comm.contains( tr(" SELL") ) )
		{
			P_ITEM pContC = m_npc->GetItemOnLayer( cBaseChar::SellContainer );

			m_npc->turnTo( pTalker );

			if( !pContC )
			{
				m_npc->talk( tr( "Sorry, I cannot use any of your wares!" ) );
				return;
			}

			m_npc->talk( tr( "This could be of interest!" ) );
			pTalker->socket()->sendSellWindow( m_npc, pTalker );
		}
	}
}

Human_Stablemaster::Human_Stablemaster( P_NPC npc ) : AbstractAI( npc ) 
{
	m_actions.append( new Action_Wander( npc, this ) );
	m_actions.append( new Action_FleeAttacker( npc, this ) );
	if( npc )
		TempEffects::instance()->insert( new cStablemasterRefreshTimer( npc, this, SrvParams->stablemasterRefreshTime() ) );
}

void Human_Stablemaster::init( P_NPC npc )
{
	AbstractAI::init( npc );
	TempEffects::instance()->insert( new cStablemasterRefreshTimer( npc, this, SrvParams->stablemasterRefreshTime() ) );
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
			int topay = 0;
			P_ITEM pPack = m_npc->getBackpack();
			cItem::ContainerContent stableitems;
			if( pPack )
			{
				cItem::ContainerContent content = pPack->content();
				cItem::ContainerContent::const_iterator it( content.begin() );
				while( it != content.end() )
				{
					if( (*it) && (*it)->id() == 0x1ea7 && (*it)->morey() == pTalker->serial() )
					{
						topay += (int)floor( (float)(*it)->morez() * SrvParams->stablemasterGoldPerRefresh() );
						stableitems.push_back( (*it) );
					}
					++it;
				}
			}
			if( !stableitems.empty() )
			{
				if( topay > gold )
				{
					m_npc->talk( tr("You do not possess enough gold. Come later if you have more!") );
					return;
				}
				cItem::ContainerContent::const_iterator it( stableitems.begin() );
				while( it != stableitems.end() )
				{
					if( (*it) )
					{
						P_NPC pPet = dynamic_cast<P_NPC>(World::instance()->findChar( (*it)->morex() ));
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
				if( topay > 0 )
				{
					pTalker->takeGold( topay, true );
					m_npc->talk( tr("Here you are! That costs %1 gold. Farewell!").arg( topay ) );
				}
				else
					m_npc->talk( tr("Here's your pet back!") );
			}
		}
	}
}

void Human_Stablemaster::refreshStock()
{
	// let's increase the refresh times of the gems in the 
	// stablemaster's backpack
	P_ITEM pPack = m_npc->getBackpack();
	if( pPack )
	{
		cItem::ContainerContent content = pPack->content();
		cItem::ContainerContent::const_iterator it( content.begin() );
		while( it != content.end() )
		{
			if( (*it) && (*it)->id() == 0x1ea7 )
			{
				(*it)->setMoreZ( (*it)->morez() + 1 );
			}
			++it;
		}
	}

	TempEffects::instance()->insert( new cStablemasterRefreshTimer( m_npc, this, SrvParams->stablemasterRefreshTime() ) );
}

void Human_Stablemaster::handleTargetInput( P_PLAYER player, cUORxTarget *target )
{
	if( !player )
		return;

	P_ITEM pPack = m_npc->getBackpack();
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

	// we spawn a worldgem in the stablemasters backpack for the pet
	// it does only hold the serial of it, the serial of the owner and the
	// number of refresh signals since begin of stabling
	// the pet becomes "free", which means, that it isnt in the world
	// but will still be saved.
	P_ITEM pGem = new cItem();
	pGem->Init( false );
	pGem->setMoreX( pPet->serial() );
	pGem->setMoreY( player->serial() );
	pGem->setMoreZ( 0 );
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

