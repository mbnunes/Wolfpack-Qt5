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
	P_ITEM pBought = pVendor->getItemOnLayer( 0x1B );

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
		Q_UINT8 layer = pItem->getOutmostItem()->layer();

		// First check: is the item on the vendor in the specified layer
		if (layer == cBaseChar::BuyRestockContainer) {
			amount = QMIN(pItem->restock(), amount);
		} else if (layer == cBaseChar::BuyNoRestockContainer) {
			// Not enough of that item is left
			amount = QMIN(pItem->amount(), amount);
		} else {
			socket->sysMessage( tr( "Invalid item bought." ) );
			socket->send( &clearBuy );
			return;
		}

		if (amount) {
			totalValue += amount * pItem->buyprice();
			items.insert( make_pair( pItem->serial(), amount ) );
		}
	}

	if ( !items.size() ) {
		socket->send( &clearBuy );
		pVendor->talk( 500190, 0, 0, false, 0xFFFF, pChar->socket() ); // Thou hast bought nothing!
		return;
	}

	// Get our total gold at once
	Q_UINT32 bankGold = pChar->countBankGold();
	Q_UINT32 packGold = pChar->countGold();
	bool fromBank = false;

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
		Q_UINT16 amount = iter->second; // we already checked if there is enough in stock
		P_ITEM pSold;

		if ( pItem->isPileable() )
		{
			pSold = pItem->dupe();
			pSold->removeTag( "restock" ); // Remove the restock tag;
			pSold->removeTag( "buy_time" );
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
				pSold->removeTag( "buy_time" );
				pSold->setAmount( 1 );
				pPack->addItem( pSold );
				pSold->update();
				pSold->sendTooltip( pChar->socket() );
			}
		}

		P_ITEM pCont = (P_ITEM)pItem->container();
		if (pCont->layer() == cBaseChar::BuyRestockContainer) {
			pItem->setRestock( pItem->restock() - amount ); // Reduce the items in stock
		} else {
			pItem->reduceAmount( amount );			
		}
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
		bool found = false;
		for (ContainerIterator it(pPurchase); !it.atEnd(); ++it)
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
		if (pItem->getOutmostChar() != pChar) {
			socket->sysMessage( tr( "Invalid item sold." ) );
			socket->send( &clearBuy );
			return;
		}

		unsigned int sellprice = pItem->getSellPrice(pVendor);

		if (!sellprice || !pItem->buyprice()) {
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
			Q_UINT16 amount = QMIN(pItem->amount(), iter->second);

			// If we can find something to stack with that is already in the vendors 
			// no restock container, increase the amount of that item instead. 
			// otherwise use the oldsytle handling.
			/*if (pItem->isPileable()) {
				bool processed = false;
				cItem::ContainerContent packContent = pBought->content();
				cItem::ContainerContent::const_iterator cit;
				for (cit = packContent.begin(); cit != packContent.end(); ++cit) {
					P_ITEM pOtherItem = *cit;
					if (pItem->canStack(pOtherItem) && pOtherItem->amount() < 65000) {
						unsigned int realAmount = QMIN(65000 - pOtherItem->amount(), amount);

						pOtherItem->setAmount(pOtherItem->amount() + realAmount);
						pOtherItem->setTag("buy_time", Server::instance()->time());						

						if ( pItem->amount() <= realAmount ) {
							pItem->remove();
						} else {
							pItem->setAmount( pItem->amount() - realAmount );
							pItem->update();
						}

						if (realAmount == amount) {
							processed = true;
							break;
						} else {
							amount -= realAmount;
						}						
					}
				}

				if (processed) {
					continue; // Next item
				}
			}*/

			if ( pItem->isPileable() && amount < pItem->amount() )
			{
				P_ITEM pSold = pItem->dupe();
				pSold->removeTag( "restock" );
				pSold->setAmount( amount );
				pSold->setTag("buy_time", Server::instance()->time());
				pBought->addItem( pSold );
				pSold->update();

				if ( pItem->amount() <= amount ) {
					pItem->remove();
				} else {
					pItem->setAmount( pItem->amount() - amount );
					pItem->update();
				}
			}
			else
			{
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
