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

#include "trade.h"
#include "inlines.h"
#include "serverconfig.h"
#include "typedefs.h"
#include "basechar.h"
#include "player.h"
#include "npc.h"
#include "world.h"
#include "console.h"
#include "network/network.h"
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
	bool operator()( P_ITEM pi, SERIAL serial ) const
	{
		return pi->serial() == serial;
	}
};

void Trade::buyAction( cUOSocket* socket, cUORxBuy* packet )
{
	cUOTxClearBuy clearBuy;
	clearBuy.setSerial( packet->serial() );

	P_PLAYER pChar = socket->player();

	if ( !pChar || pChar->free ) {
		socket->send( &clearBuy );
		return;
	}

	P_NPC pVendor = dynamic_cast<P_NPC>( FindCharBySerial( packet->serial() ) );

	if (!pVendor || pVendor->free) {
		socket->send( &clearBuy );
		return;
	}

	// Is the vendor in range
	if ( !pVendor->inRange( pChar, 4 ) )
	{
		socket->sysMessage( tr( "You can't reach the vendor" ) );
		return;
	}

	// Check the LoS (later)

	P_ITEM pPack = pChar->getBackpack();
	if ( !pPack )
	{
		socket->send( &clearBuy );
		return;
	}

	Q_UINT32 itemCount = packet->itemCount();

	if ( itemCount >= 256 )
	{
		socket->send( &clearBuy );
		return;
	}

	P_ITEM pStock = pVendor->getItemOnLayer( 0x1A );
	cItem::ContainerContent sContent;

	if ( pStock )
		sContent = pStock->content();

	P_ITEM pBought = pVendor->getItemOnLayer( 0x1B );
	cItem::ContainerContent bContent;

	if ( pBought )
		bContent = pBought->content();

	Q_UINT32 totalValue = 0;
	Q_UINT32 i;
	map<SERIAL, Q_UINT16> items;

	for ( i = 0; i < itemCount; ++i )
	{
		P_ITEM pItem = FindItemBySerial( packet->iSerial( i ) );

		if ( !pItem )
		{
			socket->sysMessage( tr( "Invalid item bought." ) );
			socket->send( &clearBuy );
			return;
		}

		Q_UINT16 amount = packet->iAmount( i );
		Q_UINT8 layer = packet->iLayer( i );

		// First check: is the item on the vendor in the specified layer
		if ( layer == 0x1A )
		{
			if ( find_if( sContent.begin(), sContent.end(), bind2nd( MatchItemAndSerial(), pItem->serial() ) ) == sContent.end() )
			{
				socket->sysMessage( tr( "Invalid item bought." ) );
				socket->send( &clearBuy );
				return;
			}
		}
		else if ( layer == 0x1B )
		{
			if ( find_if( bContent.begin(), bContent.end(), bind2nd( MatchItemAndSerial(), pItem->serial() ) ) == bContent.end() )
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
		if ( pItem->restock() > pItem->amount() )
			pItem->setRestock( pItem->amount() );

		// Nothing of that item left
		if ( pItem->restock() == 0 )
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
	Q_UINT32 bankGold = pChar->countBankGold();
	Q_UINT32 packGold = pChar->countGold();
	bool fromBank;

	if ( !pChar->isGM() )
	{
		if ( packGold >= totalValue )
		{
			fromBank = false;
			pChar->getBackpack()->removeItems( "eed", totalValue );
			socket->sendStatWindow();
		}
		else if ( bankGold >= totalValue )
		{
			fromBank = true;
			pChar->getBankbox()->removeItems( "eed", totalValue );
			socket->sendStatWindow();
		}
		else
		{
			pVendor->talk( 500192, 0, 0, false, 0xFFFF, pChar->socket() ); //Begging thy pardon, but thou casnt afford that.
			return;
		}
	}

	// Sanity checks all passed here
	for ( map<SERIAL, Q_UINT16>::iterator iter = items.begin(); iter != items.end(); ++iter )
	{
		P_ITEM pItem = FindItemBySerial( iter->first );
		Q_UINT16 amount = QMIN( iter->second, pItem->restock() );

		pItem->setRestock( pItem->restock() - amount ); // Reduce the items in stock
		P_ITEM pSold;

		if ( pItem->isPileable() )
		{
			pSold = pItem->dupe();
			pSold->removeTag( "restock" ); // Remove the restock tag;
			pSold->setAmount( iter->second );
			pPack->addItem( pSold );
			pSold->update();
			pSold->sendTooltip( pChar->socket() );
		}
		else
		{
			for ( Q_UINT16 j = 0; j < amount; ++j )
			{
				pSold = pItem->dupe();
				pSold->removeTag( "restock" ); // Remove the restock tag;
				pSold->setAmount( 1 );
				pPack->addItem( pSold );
				pSold->update();
				pSold->sendTooltip( pChar->socket() );
			}
		}

		//socket->sysMessage( tr( "You put the %1 into your pack" ).arg( pItem->getName() ) );
	}

	socket->send( &clearBuy );

	if ( fromBank )
		pVendor->talk( tr( "The total of thy purchase is %1 gold, which has been withdrawn from your bank account.  My thanks for the patronage." ).arg( totalValue ), 0xFFFF, 0, false, pChar->socket() );
	else
		pVendor->talk( tr( "The total of thy purchase is %1 gold.  My thanks for the patronage." ).arg( totalValue ), 0xFFFF, 0, false, pChar->socket() );

	pChar->socket()->soundEffect( 0x32 );
}

void Trade::sellAction( cUOSocket* socket, cUORxSell* packet )
{
	P_PLAYER pChar = socket->player();
	P_CHAR pVendor = FindCharBySerial( packet->serial() );

	cUOTxClearBuy clearBuy;
	clearBuy.setSerial( packet->serial() );

	if ( !pChar || !pVendor || pVendor->free || pChar->free )
	{
		socket->send( &clearBuy );
		return;
	}

	// Is the vendor in range
	if ( !pVendor->inRange( pChar, 4 ) )
	{
		socket->sysMessage( tr( "You can't reach the vendor" ) );
		return;
	}

	// Check the LoS (later)

	P_ITEM pPack = pChar->getBackpack();
	if ( !pPack )
	{
		socket->send( &clearBuy );
		return;
	}

	Q_UINT32 itemCount = packet->itemCount();

	if ( itemCount >= 256 )
	{
		socket->send( &clearBuy );
		return;
	}

	P_ITEM pPurchase = pVendor->getItemOnLayer( 0x1C );
	if ( !pPurchase )
	{
		socket->sysMessage( tr( "Invalid item sold." ) );
		socket->send( &clearBuy );
		return;
	}
	cItem::ContainerContent sContent;
	cItem::ContainerContent::const_iterator it;

	Q_UINT32 totalValue = 0;
	Q_UINT32 i;
	map<SERIAL, Q_UINT16> items;

	for ( i = 0; i < itemCount; ++i )
	{
		P_ITEM pItem = FindItemBySerial( packet->iSerial( i ) );

		if ( !pItem )
		{
			socket->sysMessage( tr( "Invalid item sold." ) );
			socket->send( &clearBuy );
			return;
		}

		Q_UINT16 amount = packet->iAmount( i );

		// First an equal item with higher amount must be in the vendors sell container!
		sContent = pPurchase->content();

		bool found = false;
		for ( it = sContent.begin(); it != sContent.end(); ++it )
		{
			if ( !( *it ) )
				continue;

			if ( ( *it )->baseid() == pItem->baseid() && ( *it )->scriptList() == pItem->scriptList() )
			{
				found = true;
				break;
			}
		}

		if ( !found )
		{
			socket->sysMessage( tr( "Invalid item sold." ) );
			socket->send( &clearBuy );
			return;
		}

		// now look for the item in the player's pack
		if (pItem->container() != pPack) {
			socket->sysMessage( tr( "Invalid item sold." ) );
			socket->send( &clearBuy );
			return;
		}

		unsigned int sellprice = pItem->getSellPrice(pVendor);

		if (!sellprice) {
			socket->sysMessage( tr( "Invalid item sold." ) );
			socket->send( &clearBuy );
			return;
		}

		totalValue += amount * sellprice;

		items.insert( make_pair( pItem->serial(), amount ) );
	}

	// Sanity checks all passed here
	P_ITEM pBought = pVendor->atLayer( cBaseChar::BuyNoRestockContainer );
	if ( pBought )
	{
		for ( map<SERIAL, Q_UINT16>::iterator iter = items.begin(); iter != items.end(); ++iter )
		{
			P_ITEM pItem = FindItemBySerial( iter->first );
			Q_UINT16 amount = iter->second;

			if ( pItem->isPileable() )
			{
				P_ITEM pSold = pItem->dupe();
				pSold->removeTag( "restock" );
				pSold->setAmount( iter->second );
				pBought->addItem( pSold );
				pSold->update();
				if ( pItem->amount() <= iter->second )
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
	socket->sendStatWindow(); // Update Gold
}

}
