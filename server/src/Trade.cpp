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

#include "Trade.h"
#include "inlines.h"
#include "srvparams.h"
#include "typedefs.h"
#include "basechar.h"
#include "player.h"
#include "npc.h"
#include "world.h"
#include "console.h"
#include "network.h"
#include "items.h"
#include "network/uorxpackets.h"
#include "network/uotxpackets.h"
#include "network/uosocket.h"

// System Includes
#include <algorithm>

using namespace std;

namespace Trade
{

	struct MatchItemAndSerial : public std::binary_function<P_ITEM, SERIAL, bool>
	{
		bool operator()(P_ITEM pi, SERIAL serial) const
		{
			return pi->serial() == serial;
		}
	};

	void Trade::buyAction( cUOSocket *socket, cUORxBuy *packet )
	{
		P_PLAYER pChar = socket->player();
		P_NPC pVendor = dynamic_cast<P_NPC>( FindCharBySerial( packet->serial() ) );

		cUOTxClearBuy clearBuy;
		clearBuy.setSerial( pVendor->serial() );

		if( !pChar || !pVendor || pVendor->free || pChar->free )
		{
			socket->send( &clearBuy );
			return;
		}

		// Is the vendor in range
		if( !pVendor->inRange( pChar, 4 ) )
		{
			socket->sysMessage( tr( "You can't reach the vendor" ) );
			return;
		}

		// Check the LoS (later)

		P_ITEM pPack = pChar->getBackpack();
		if( !pPack )
		{
			socket->send( &clearBuy );
			return;
		}

		UINT32 itemCount = packet->itemCount();

		if( itemCount >= 256 )
		{
			socket->send( &clearBuy );
			return;
		}

		P_ITEM pStock = pVendor->GetItemOnLayer( 0x1A );
		cItem::ContainerContent sContent;

		if( pStock )
			sContent = pStock->content();

		P_ITEM pBought = pVendor->GetItemOnLayer( 0x1B );
		cItem::ContainerContent bContent;

		if( pBought )
			bContent = pBought->content();

		UINT32 totalValue = 0;
		UINT32 i;
		map< SERIAL, UINT16 > items;

		for( i = 0; i < itemCount; ++i )
		{
			P_ITEM pItem = FindItemBySerial( packet->iSerial( i ) );

			if( !pItem )
			{
				socket->sysMessage( tr( "Invalid item bought." ) );
				socket->send( &clearBuy );
				return;
			}

			UINT16 amount = packet->iAmount( i );
			UINT8 layer = packet->iLayer( i );

			// First check: is the item on the vendor in the specified layer
			if( layer == 0x1A )
			{
				if( find_if( sContent.begin(), sContent.end(), bind2nd(MatchItemAndSerial(), pItem->serial()) ) == sContent.end() )
				{
					socket->sysMessage( tr( "Invalid item bought." ) );
					socket->send( &clearBuy );
					return;
				}
			}
			else if( layer == 0x1B )
			{
				if( find_if( bContent.begin(), bContent.end(), bind2nd(MatchItemAndSerial(), pItem->serial() ) ) == bContent.end() )
				{
					socket->sysMessage( tr( "Invalid item bought." ) );
					socket->send( &clearBuy );
					return;
				}
			}
			else
			{
				socket->sysMessage( tr( "Invalid item bought." ) );
				socket->send( &clearBuy );
				return;
			}

			// Check amount
			if( pItem->restock() > pItem->amount() )
				pItem->setRestock( pItem->amount() );

			// Nothing of that item left
			if( pItem->restock() == 0 )
				continue;

			totalValue += amount * pItem->buyprice();

			if ( amount )
				items.insert( make_pair( pItem->serial(), amount ) );
		}

		if ( !items.size() )
		{
			socket->send( &clearBuy );
			pVendor->talk( 500190, 0, 0, false, 0xFFFF, pChar->socket() ); // Thou hast bought nothing!
			return;
		}

		// Get our total gold at once
		UINT32 bankGold = pChar->CountBankGold();
		UINT32 packGold = pChar->CountGold();
		bool fromBank;

		if (!pChar->isGM()) {
			if (packGold >= totalValue) {
				fromBank = false;
				pChar->getBackpack()->removeItems("eed", totalValue);
				socket->sendStatWindow();
			} else if (bankGold >= totalValue) {
				fromBank = true;
				pChar->getBankbox()->removeItems("eed", totalValue);
				socket->sendStatWindow();
			} else {
				pVendor->talk( 500192, 0, 0, false, 0xFFFF, pChar->socket() ); //Begging thy pardon, but thou casnt afford that.
				return;
			}
		}

		// Sanity checks all passed here
		for( map< SERIAL,  UINT16 >::iterator iter = items.begin(); iter != items.end(); ++iter )
		{
			P_ITEM pItem = FindItemBySerial( iter->first );
			UINT16 amount = QMIN( iter->second, pItem->restock() );

			pItem->setRestock( pItem->restock() - amount ); // Reduce the items in stock
			P_ITEM pSold;

			if( pItem->isPileable() )
			{
				pSold = pItem->dupe();
				pSold->setAmount( iter->second );
				pPack->addItem( pSold );
				pSold->update();
				pSold->sendTooltip(pChar->socket());
			}
			else
			{
				for( UINT16 j = 0; j < amount; ++j )
				{
					pSold = pItem->dupe();
					pSold->setAmount( 1 );
					pPack->addItem( pSold );
					pSold->update();
					pSold->sendTooltip(pChar->socket());
				}
			}

			//socket->sysMessage( tr( "You put the %1 into your pack" ).arg( pItem->getName() ) );
		}

		socket->send( &clearBuy );

		if ( fromBank )
			pVendor->talk( tr("The total of thy purchase is %1 gold, which has been withdrawn from your bank account.  My thanks for the patronage.").arg(totalValue), 0xFFFF, 0, false, pChar->socket() );
		else
			pVendor->talk( tr("The total of thy purchase is %1 gold.  My thanks for the patronage.").arg(totalValue), 0xFFFF, 0, false, pChar->socket() );

		pChar->socket()->soundEffect( 0x32 );
	}

	void Trade::sellAction( cUOSocket *socket, cUORxSell *packet )
	{
		P_PLAYER pChar = socket->player();
		P_CHAR pVendor = FindCharBySerial( packet->serial() );

		cUOTxClearBuy clearBuy;
		clearBuy.setSerial( pVendor->serial() );

		if( !pChar || !pVendor || pVendor->free || pChar->free )
		{
			socket->send( &clearBuy );
			return;
		}

		// Is the vendor in range
		if( !pVendor->inRange( pChar, 4 ) )
		{
			socket->sysMessage( tr( "You can't reach the vendor" ) );
			return;
		}

		// Check the LoS (later)

		P_ITEM pPack = pChar->getBackpack();
		if( !pPack )
		{
			socket->send( &clearBuy );
			return;
		}

		UINT32 itemCount = packet->itemCount();

		if( itemCount >= 256 )
		{
			socket->send( &clearBuy );
			return;
		}

		P_ITEM pPurchase = pVendor->GetItemOnLayer( 0x1C );
		if( !pPurchase )
		{
			socket->sysMessage( tr( "Invalid item sold." ) );
			socket->send( &clearBuy );
			return;
		}
		cItem::ContainerContent sContent;
		cItem::ContainerContent::const_iterator it;

		UINT32 totalValue = 0;
		UINT32 i;
		map< SERIAL, UINT16 > items;

		for( i = 0; i < itemCount; ++i )
		{
			P_ITEM pItem = FindItemBySerial( packet->iSerial( i ) );

			if( !pItem )
			{
				socket->sysMessage( tr( "Invalid item sold." ) );
				socket->send( &clearBuy );
				return;
			}

			UINT16 amount = packet->iAmount( i );

			// First an equal item with higher amount must be in the vendors sell container!
			sContent = pPurchase->content();

			bool found = false;
			for( it = sContent.begin(); it != sContent.end(); ++it )
			{
				if( !(*it) )
					continue;

				if(		(*it)->id() == pItem->id() &&
						(*it)->color() == pItem->color() &&
	//					(*it)->amount() >= pItem->amount() &&
						(*it)->eventList() == pItem->eventList() )
				{
					found = true;
					break;
				}
			}

			if( !found )
			{
				socket->sysMessage( tr( "Invalid item sold." ) );
				socket->send( &clearBuy );
				return;
			}

			// now look for the item in the player's pack
			sContent = pPack->content();
			if( find_if( sContent.begin(), sContent.end(), bind2nd(MatchItemAndSerial(), pItem->serial()) ) == sContent.end() )
			{
				socket->sysMessage( tr( "Invalid item sold." ) );
				socket->send( &clearBuy );
				return;
			}

			totalValue += amount * pItem->sellprice();

			items.insert( make_pair( pItem->serial(), amount ) );
		}

		// Sanity checks all passed here
		P_ITEM pBought = pVendor->atLayer( cBaseChar::BuyNoRestockContainer );
		if( pBought )
		{
			for( map< SERIAL,  UINT16 >::iterator iter = items.begin(); iter != items.end(); ++iter )
			{
				P_ITEM pItem = FindItemBySerial( iter->first );
				UINT16 amount = iter->second;

				if( pItem->isPileable() )
				{
					P_ITEM pSold = pItem->dupe();
					pSold->setAmount( iter->second );
					pBought->addItem( pSold );
					pSold->update();
					if( pItem->amount() <= iter->second )
						pItem->remove();
					else
						pItem->setAmount( pItem->amount() - iter->second );
					pItem->update();
				}
				else
				{
					pPack->removeItem( pItem );
					pBought->addItem( pItem );
					pItem->update();
				}
			}
		}

		socket->send( &clearBuy );
		pVendor->talk( tr( "Thank you %1, here are your %2 gold" ).arg( pChar->name() ).arg( totalValue ) );

		pChar->giveGold( totalValue, false );
	}

	P_ITEM Trade::startTrade( P_CHAR pPlayer, P_CHAR pChar )
	{
	/*	if( !pChar || !pChar->socket() || !pPlayer || !pPlayer->socket() )
			return NULL;

		// Create a trade-container for both players
		// 0x2AF8 on Layer 0x1F
		SERIAL box1,box2;

		// One for our player
		P_ITEM tCont = cItem::createFromScript( "2af8" );
		tCont->setLayer( 0x1f );

	//	tCont->setContSerial( pPlayer->serial() );
		tCont->setOwner( pPlayer );
		tCont->tags.set( "tradepartner", cVariant( pChar->serial() ) );
		tCont->update( pPlayer->socket() );
		tCont->update( pChar->socket() );
		box1 = tCont->serial();

		// One for the tradepartner
		tCont = tCont->dupe();
		tCont->setLayer( 0x1f );
	//	tCont->setContSerial( pChar->serial() );
		tCont->setOwner( pChar );
		tCont->tags.set( "tradepartner", cVariant( pPlayer->serial() ) );
		tCont->update( pPlayer->socket() );
		tCont->update( pChar->socket() );
		box2 = tCont->serial();

		// Now send the both secure trading packets
		cUOTxTrade trade;

		// To us
		trade.setPartner( pChar->serial() );
		trade.setBox1( box1 );
		trade.setBox2( box2 );
		trade.setName( pChar->name.latin1() );
		pPlayer->socket()->send( &trade );

		// To the other
		trade.setPartner( pPlayer->serial() );
		trade.setBox1( box2 );
		trade.setBox2( box1 );
		trade.setName( pPlayer->name.latin1() );
		pChar->socket()->send( &trade );

		return FindItemBySerial( box1 );*/
		return 0;
	}

	P_ITEM Trade::tradestart(cUOSocket* s, P_CHAR pc_i)
	{
	/*	P_CHAR pc_currchar = currchar[s];
		unsigned char msg[90];

		P_ITEM pi_bps = pc_currchar->getBackpack();
		P_ITEM pi_bpi = pc_i->getBackpack();
		UOXSOCKET s2 = calcSocketFromChar(pc_i);

		if (pi_bps == NULL) //LB
		{
			sysmessage(s, "Time to buy a backpack!");
			sysmessage(s2, "%s doesnt have a backpack!", pc_currchar->name.latin1());
			return 0;
		}
		if (pi_bpi == NULL)
		{
			sysmessage(s2, "Time to buy a backpack!");
			sysmessage(s, "%s doesnt have a backpack!", pc_i->name.latin1());
			return 0;
		}

		P_ITEM pi_ps = Items->SpawnItem(s2, pc_currchar, 1, "#", 0, 0x1E, 0x5E, 0, 0, 0);
		if(pi_ps == NULL)
			return 0;
		pi_ps->pos = Coord_cl(26, 0, 0);
	//	pi_ps->setContSerial(pc_currchar->serial());
		pi_ps->setLayer( 0 );
		pi_ps->setType( 1 );
		pi_ps->setDye(0);
	//	sendbpitem(s, pi_ps);
	//	if (s2 != INVALID_UOXSOCKET)
	//		sendbpitem(s2, pi_ps);

		P_ITEM pi_pi = Items->SpawnItem(s2,pc_i,1,"#",0,0x1E,0x5E,0,0,0);
		if (pi_pi == NULL)
			return 0;
		pi_pi->pos = Coord_cl(26, 0, 0);
	//	pi_pi->setContSerial(pc_i->serial());
		pi_pi->setLayer( 0 );
		pi_pi->setType( 1 );
		pi_pi->setDye(0);
	//	sendbpitem(s, pi_pi);
	//	if (s2 != INVALID_UOXSOCKET)
	//		sendbpitem(s2, pi_pi);

		pi_pi->setMoreb1( static_cast<unsigned char>((pi_ps->serial()&0xFF000000)>>24) );
		pi_pi->setMoreb2( static_cast<unsigned char>((pi_ps->serial()&0x00FF0000)>>16) );
		pi_pi->setMoreb3( static_cast<unsigned char>((pi_ps->serial()&0x0000FF00)>>8) );
		pi_pi->setMoreb4( static_cast<unsigned char>((pi_ps->serial()&0x000000FF)) );
		pi_ps->setMore1( static_cast<unsigned char>((pi_pi->serial()&0xFF000000)>>24) );
		pi_ps->setMore2( static_cast<unsigned char>((pi_pi->serial()&0x00FF0000)>>16) );
		pi_ps->setMore3( static_cast<unsigned char>((pi_pi->serial()&0x0000FF00)>>8) );
		pi_ps->setMore4( static_cast<unsigned char>((pi_pi->serial()&0x000000FF)) );
		pi_ps->setMoreZ(0);
		pi_pi->setMoreZ(0);

		msg[0] = 0x6F; // Header Byte
		msg[1] = 0; // Size
		msg[2] = 47; // Size
		msg[3] = 0; // Initiate
		LongToCharPtr(pc_i->serial(),msg+4);
		LongToCharPtr(pi_ps->serial(),msg+8);
		LongToCharPtr(pi_pi->serial(),msg+12);
		msg[16]=1;
		strcpy((char*)&(msg[17]), pc_i->name.latin1());
		Xsend(s, msg, 47);

		if (s2 != INVALID_UOXSOCKET)
		{
			msg[0]=0x6F; // Header Byte
			msg[1]=0;    // Size
			msg[2]=47;   // Size
			msg[3]=0;    // Initiate
			LongToCharPtr(pc_currchar->serial(),msg+4);
			LongToCharPtr(pi_pi->serial(),msg+8);
			LongToCharPtr(pi_ps->serial(),msg+12);
			msg[16]=1;
			strcpy((char*)&(msg[17]), pc_currchar->name.latin1());

			Xsend(s2, msg, 47);
		} */
		return 0;
	}

	void Trade::clearalltrades()
	{
	}

	void Trade::trademsg(int s)
	{
	}

	void Trade::dotrade(P_ITEM cont1, P_ITEM cont2)
	{
	}

}

