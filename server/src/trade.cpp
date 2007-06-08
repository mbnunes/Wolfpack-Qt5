/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2007 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
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

void buyAction( cUOSocket* socket, cUORxBuy* packet )
{
	cUOTxClearBuy clearBuy;
	clearBuy.setSerial( packet->serial() );

	P_PLAYER pChar = socket->player();

	if ( !pChar || pChar->free )
	{
		socket->send( &clearBuy );
		return;
	}

	P_NPC pVendor = dynamic_cast<P_NPC>( FindCharBySerial( packet->serial() ) );

	if ( !pVendor || pVendor->free )
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

	quint32 itemCount = packet->itemCount();

	if ( itemCount >= 256 )
	{
		socket->send( &clearBuy );
		return;
	}

	P_ITEM pStock = pVendor->getItemOnLayer( 0x1A );
	P_ITEM pBought = pVendor->getItemOnLayer( 0x1B );
	Q_UNUSED( pStock );
	Q_UNUSED( pBought );

	quint32 totalValue = 0;
	quint32 i;
	map<SERIAL, quint16> items;

	for ( i = 0; i < itemCount; ++i )
	{
		P_ITEM pItem = FindItemBySerial( packet->iSerial( i ) );

		if ( !pItem )
		{
			socket->sysMessage( tr( "Invalid item bought." ) );
			socket->send( &clearBuy );
			return;
		}

		quint16 amount = packet->iAmount( i );
		quint8 layer = pItem->getOutmostItem()->layer();

		// First check: is the item on the vendor in the specified layer
		if ( layer == cBaseChar::BuyRestockContainer )
		{
			amount = wpMin<quint16>( pItem->restock(), amount );
		}
		else if ( layer == cBaseChar::BuyNoRestockContainer )
		{
			// Not enough of that item is left
			amount = wpMin<quint16>( pItem->amount(), amount );
		}
		else
		{
			socket->sysMessage( tr( "Invalid item bought." ) );
			socket->send( &clearBuy );
			return;
		}

		// Lets introduce onBuy Events here
		int secAmount;
		
		secAmount = pItem->onBuy( pVendor, pChar, amount );
		if (secAmount < 0)
			return;
		else
			amount = secAmount;

		secAmount = pChar->onBuy( pVendor, pItem, amount );
		if (secAmount < 0)
			return;
		else
			amount = secAmount;
		
		secAmount = pVendor->onBuy( pChar, pItem, amount );
		if (secAmount < 0)
			return;
		else
			amount = secAmount;

		if ( amount )
		{
			totalValue += amount * pItem->getBuyPrice( pVendor, pChar );
			items.insert( make_pair( pItem->serial(), amount ) );
		}
	}

	if ( !items.size() )
	{
		socket->send( &clearBuy );
		pVendor->talk( 500190, 0, 0, false, 0xFFFF, pChar->socket() ); // Thou hast bought nothing!
		return;
	}

	int fromWhere = 0; // 0 is Default to Pack, 1 is to Bank and 2 for both

	if ( !pChar->isGM() )
	{
		// Oh yeah, the new monetary
		if (Config::instance()->usenewmonetary()) {
			// Pay just from Pack
			if (Config::instance()->payfrompackonly()) {

				// Get the BackPack
				P_ITEM backpack = pChar->getBackpack(); //My BackPack

				// Lets Assign Region
				cTerritory* Region = pVendor->region();

				// Lets Assign the IDs
				QString idfirst = Config::instance()->defaultFirstCoin();
				QString idsecond = Config::instance()->defaultSecondCoin();
				QString idthird = Config::instance()->defaultThirdCoin();

				// Lets try to find the IDs ahn?
				if ( Region )
				{
					idfirst = Region->firstcoin();
					idsecond = Region->secondcoin();
					idthird = Region->thirdcoin();
				}

				// Get our total gold at once
				quint32 packFirst = backpack->countBaseItems( idfirst );
				quint32 packSecond = backpack->countBaseItems( idsecond );
				quint32 packThird = backpack->countBaseItems( idthird );

				// Use Reversed Monetary?
				if (Config::instance()->usereversedvaluable()) {
					if ( packThird >= totalValue )
					{
						pChar->getBackpack()->removeItem( idthird, totalValue );
						socket->sendStatWindow();
					}
					else if ( ( packThird + packSecond*10 ) >= totalValue)
					{
						// Oh! We Cant Pay just with Silver
						if (totalValue % 10)
						{
							if (packSecond*10 >= totalValue)
							{
								pChar->getBackpack()->removeItem( idsecond, ( totalValue/10 ) + 1 );
								// Creating Money Back
								P_ITEM pTroco = cItem::createFromScript( idthird );
								pTroco->setAmount( 10 - (totalValue % 10) );
								pTroco->toBackpack( pChar );
							}
							else
							{
								pChar->getBackpack()->removeItem( idsecond, packSecond );
								pChar->getBackpack()->removeItem( idthird, totalValue - ( packSecond * 10 ) );
							}
						}
						// Oh! We can pay maybe just with Silver
						else
						{
							if (packSecond*10 >= totalValue)
							{
								pChar->getBackpack()->removeItem( idsecond, totalValue/10 );
							}
							else
							{
								pChar->getBackpack()->removeItem( idsecond, packSecond );
								pChar->getBackpack()->removeItem( idthird, totalValue - ( packSecond * 10 ) );
							}
						}
						// Update Gold
						socket->sendStatWindow();
					}
					else if ( ( packThird + packSecond*10 + packFirst*100 ) >= totalValue)
					{
						// Oh! We cant pay just with Gold
						if (totalValue % 100)
						{
							if (packFirst*100 >= totalValue)
							{
								pChar->getBackpack()->removeItem( idfirst, ( totalValue/100 ) + 1 );
								// Creating Money Back
								if ( (totalValue % 100) < 90 )
								{
									// First, Silver
									P_ITEM pTroco = cItem::createFromScript( idsecond );
									pTroco->setAmount( ( 100 - (totalValue % 100) ) / 10 );
									pTroco->toBackpack( pChar );
									// Now, Copper
									pTroco = cItem::createFromScript( idthird );
									pTroco->setAmount( 10 - (totalValue % 10) );
									pTroco->toBackpack( pChar );
								}
								else
								{
									P_ITEM pTroco = cItem::createFromScript( idthird );
									pTroco->setAmount( 10 - (totalValue % 10) );
									backpack->addItem( pTroco );
								}
							}
							else
							{
								pChar->getBackpack()->removeItem( idfirst, packFirst );
								// The rest of Money
								if (totalValue % 10)
								{
									if (packSecond*10 >= ( totalValue - packFirst * 100) )
									{
										pChar->getBackpack()->removeItem( idsecond, ((totalValue - packFirst * 100)/10) + 1 );
										
										// Money Back
										P_ITEM pTroco = cItem::createFromScript( idthird );
										pTroco->setAmount( 10 - (totalValue % 10) );
										backpack->addItem( pTroco );
									}
									else
									{
										pChar->getBackpack()->removeItem( idsecond, packSecond );
										pChar->getBackpack()->removeItem( idthird, totalValue - ( packSecond * 10 ) - ( packFirst * 100 ) );
									}
								}
								else
								{
									if (packSecond*10 >= ( totalValue - packFirst * 100) )
									{
										pChar->getBackpack()->removeItem( idsecond, ((totalValue - packFirst * 100)/10) );
									}
									else
									{
										pChar->getBackpack()->removeItem( idsecond, packSecond );
										pChar->getBackpack()->removeItem( idthird, totalValue - ( packSecond * 10 ) - ( packFirst * 100 ) );
									}
								}
							}
						}
						// Oh! We can pay maybe just with Gold
						else
						{
							if (packFirst*100 >= totalValue)
							{
								pChar->getBackpack()->removeItem( idfirst, totalValue/100 );
							}
							else
							{
								pChar->getBackpack()->removeItem( idfirst, packFirst );
								// The rest of Money
								if (packSecond*10 >= ( totalValue - packFirst * 100) )
								{
									pChar->getBackpack()->removeItem( idsecond, ((totalValue - packFirst * 100)/10) );
								}
								else
								{
									pChar->getBackpack()->removeItem( idsecond, packSecond );
									pChar->getBackpack()->removeItem( idthird, totalValue - ( packSecond * 10 ) - ( packFirst * 100 ) );
								}
							}
						}
						// Update Gold
						socket->sendStatWindow();
					}
					else
					{
						pVendor->talk( 500192, 0, 0, false, 0xFFFF, pChar->socket() ); //Begging thy pardon, but thou casnt afford that.
						return;
					}
				}
				// No, Use Normal Base
				else
				{
					if ( packFirst >= totalValue )
					{
						pChar->getBackpack()->removeItem( idfirst, totalValue );
						socket->sendStatWindow();
					}
					else if ( (packFirst + packSecond/10) >= totalValue )
					{
						pChar->getBackpack()->removeItem( idfirst, packFirst );
						pChar->getBackpack()->removeItem( idsecond, ((totalValue - packFirst)*10) );
						socket->sendStatWindow();
					}
					else if ( (packFirst + packSecond/10 + packThird/100) >= totalValue )
					{
						pChar->getBackpack()->removeItem( idfirst, packFirst );
						pChar->getBackpack()->removeItem( idsecond, packSecond );
						pChar->getBackpack()->removeItem( idthird, ( (totalValue * 100) - (packFirst * 100) ) - (packSecond * 10) );
						socket->sendStatWindow();
					}
					else
					{
						pVendor->talk( 500192, 0, 0, false, 0xFFFF, pChar->socket() ); //Begging thy pardon, but thou casnt afford that.
						return;
					}
				}
				// End of Reversed / Normal Base for New Monetary
			}
			// No, We Can Pay from Pack or Bank
			else
			{
				// Lets Assign Region
				cTerritory* Region = pVendor->region();

				// Lets Assign the IDs
				QString idfirst = "eed";
				QString idsecond = "ef0";
				QString idthird = "eea";

				// Lets try to find the IDs ahn?
				if ( Region )
				{
					idfirst = Region->firstcoin();
					idsecond = Region->secondcoin();
					idthird = Region->thirdcoin();
				}

				// Get the BankBox
				P_ITEM bank = pChar->getBankbox(); //My BankBox
				// Get the BackPack
				P_ITEM backpack = pChar->getBackpack(); //My BackPack

				// Get our total gold at once
				quint32 bankFirst = bank->countBaseItems( idfirst );
				quint32 bankSecond = bank->countBaseItems( idsecond );
				quint32 bankThird = bank->countBaseItems( idthird );

				quint32 packFirst = backpack->countBaseItems( idfirst );
				quint32 packSecond = backpack->countBaseItems( idsecond );
				quint32 packThird = backpack->countBaseItems( idthird );

				// Lets go... First Money Section
				if ( packFirst >= totalValue )
				{
					pChar->getBackpack()->removeItem( idfirst, totalValue );
					socket->sendStatWindow();
				}
				else if ( bankFirst >= totalValue )
				{
					fromWhere = 1;
					pChar->getBankbox()->removeItem( idfirst, totalValue );
					socket->sendStatWindow();
				}
				else if ( (bankFirst + packFirst) >= totalValue )
				{
					fromWhere = 2;
					// From Pack the Max
					pChar->getBackpack()->removeItem( idfirst, packFirst );
					// From Bank the rest
					pChar->getBankbox()->removeItem( idfirst, (totalValue - packFirst) );
					socket->sendStatWindow();
				}
				// First + Second Money Session
				else if ( (packFirst + packSecond/10) >= totalValue )
				{
					pChar->getBackpack()->removeItem( idfirst, packFirst );
					pChar->getBackpack()->removeItem( idsecond, ((totalValue - packFirst)*10) );
					socket->sendStatWindow();
				}
				else if ( (bankFirst + bankSecond/10) >= totalValue )
				{
					fromWhere = 1;
					pChar->getBankbox()->removeItem( idfirst, bankFirst );
					pChar->getBankbox()->removeItem( idsecond, ((totalValue - bankFirst)*10) );
					socket->sendStatWindow();
				}
				else if ( (bankFirst + packFirst + packSecond/10) >= totalValue )
				{
					fromWhere = 2;
					// From Pack the Max
					pChar->getBackpack()->removeItem( idfirst, packFirst );
					// From Bank more things
					pChar->getBankbox()->removeItem( idfirst, bankFirst );
					// And now, from Pack the Silver
					pChar->getBackpack()->removeItem( idsecond, ((totalValue - packFirst - bankFirst)*10) );
					// Socket
					socket->sendStatWindow();
				}
				else if ( (bankFirst + packFirst + packSecond/10 + bankSecond/10) >= totalValue )
				{
					fromWhere = 2;
					// From Pack the Max
					pChar->getBackpack()->removeItem( idfirst, packFirst );
					// From Bank more things
					pChar->getBankbox()->removeItem( idfirst, bankFirst );
					// Now, from Pack the Silver
					pChar->getBackpack()->removeItem( idsecond, packSecond );
					// and finally, from Bank
					pChar->getBankbox()->removeItem( idsecond, ((totalValue - packFirst - bankFirst - (packSecond/10))*10) );
					// Socket
					socket->sendStatWindow();
				}
				// First + Second + Third Money session (The Pain)
				else if ( (packFirst + packSecond/10 + packThird/100) >= totalValue )
				{
					pChar->getBackpack()->removeItem( idfirst, packFirst );
					pChar->getBackpack()->removeItem( idsecond, packSecond );
					pChar->getBackpack()->removeItem( idthird, ( (totalValue * 100) - (packFirst * 100) ) - (packSecond * 10) );
					socket->sendStatWindow();
				}
				else if ( (bankFirst + bankSecond/10 + bankThird/100) >= totalValue )
				{
					pChar->getBankbox()->removeItem( idfirst, bankFirst );
					pChar->getBankbox()->removeItem( idsecond, bankSecond );
					pChar->getBankbox()->removeItem( idthird, ( (totalValue * 100) - (bankFirst * 100) ) - (bankSecond * 10) );
					socket->sendStatWindow();
				}
				else if ( (bankFirst + packFirst + packSecond/10 + bankSecond/10 + packThird/100) >= totalValue )
				{
					fromWhere = 2;
					// From Pack the Max
					pChar->getBackpack()->removeItem( idfirst, packFirst );
					// From Bank more things
					pChar->getBankbox()->removeItem( idfirst, bankFirst );
					// From Pack, the Silver
					pChar->getBackpack()->removeItem( idsecond, packSecond );
					// From Bank, the Silver Too
					pChar->getBankbox()->removeItem( idsecond, bankSecond );
					// And, now, from the Pack, the Copper
					pChar->getBackpack()->removeItem( idthird, ( (totalValue * 100) - (packFirst * 100) - (bankFirst * 100) ) - (packSecond * 10) - (bankSecond * 10) );
					// Socket
					socket->sendStatWindow();
				}
				else if ( (bankFirst + packFirst + packSecond/10 + bankSecond/10 + packThird/100 + bankThird/100) >= totalValue )
				{
					fromWhere = 2;
					// From Pack the Max
					pChar->getBackpack()->removeItem( idfirst, packFirst );
					// From Bank more things
					pChar->getBankbox()->removeItem( idfirst, bankFirst );
					// From Pack, the Silver
					pChar->getBackpack()->removeItem( idsecond, packSecond );
					// From Bank, the Silver Too
					pChar->getBankbox()->removeItem( idsecond, bankSecond );
					// From Pack, all the Copper
					pChar->getBackpack()->removeItem( idthird, packThird );
					// And, now, from the Bank, the Copper
					pChar->getBankbox()->removeItem( idthird, ( (totalValue * 100) - (packFirst * 100) - (bankFirst * 100) ) - (packSecond * 10) - (bankSecond * 10) - packThird );
					// Socket
					socket->sendStatWindow();
				}
				else
				{
					pVendor->talk( 500192, 0, 0, false, 0xFFFF, pChar->socket() ); //Begging thy pardon, but thou casnt afford that.
					return;
				}
			}
			// End of Pay From Pack / Pay from Pack and Bank
		}
		// Oh no, the normal way (No New Monetary)
		else
		{
			// Pay just from Pack?
			if (Config::instance()->payfrompackonly()) {

				// Get our total gold at once
				quint32 packGold = pChar->countGold();

				if ( packGold >= totalValue )
				{
					pChar->getBackpack()->removeItem( "eed", totalValue );
					socket->sendStatWindow();
				}
				else
				{
					pVendor->talk( 500192, 0, 0, false, 0xFFFF, pChar->socket() ); //Begging thy pardon, but thou casnt afford that.
					return;
				}
			}
			// No, Pay from Pack and Bank
			else
			{
				// Get our total gold at once
				quint32 bankGold = pChar->countBankGold();
				quint32 packGold = pChar->countGold();

				if ( packGold >= totalValue )
				{
					pChar->getBackpack()->removeItem( "eed", totalValue );
					socket->sendStatWindow();
				}
				else if ( bankGold >= totalValue )
				{
					fromWhere = 1;
					pChar->getBankbox()->removeItem( "eed", totalValue );
					socket->sendStatWindow();
				}
				else if ( (bankGold + packGold) >= totalValue )
				{
					fromWhere = 2;
					// From Pack the Max
					pChar->getBackpack()->removeItem( "eed", packGold );
					// From Bank the rest
					pChar->getBankbox()->removeItem( "eed", (totalValue - packGold) );
					socket->sendStatWindow();
				}
				else
				{
					pVendor->talk( 500192, 0, 0, false, 0xFFFF, pChar->socket() ); //Begging thy pardon, but thou casnt afford that.
					return;
				}
			}
			// End of "Pay from Pack only or from pack and from bank?"
		}
		// End of "New or Normal Monetary System"
	}
	// End of "Char is a GM or Not?"

	// Sanity checks all passed here
	for ( map<SERIAL, quint16>::iterator iter = items.begin(); iter != items.end(); ++iter )
	{
		P_ITEM pItem = FindItemBySerial( iter->first );
		quint16 amount = iter->second; // we already checked if there is enough in stock
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
			for ( quint16 j = 0; j < amount; ++j )
			{
				pSold = pItem->dupe(1);
				pSold->removeTag( "restock" ); // Remove the restock tag;
				pSold->removeTag( "buy_time" );
				pSold->setAmount( 1 );
				pPack->addItem( pSold );
				pSold->update();
				pSold->sendTooltip( pChar->socket() );
			}
		}

		P_ITEM pCont = ( P_ITEM ) pItem->container();
		if ( pCont->layer() == cBaseChar::BuyRestockContainer )
		{
			pItem->setRestock( pItem->restock() - amount ); // Reduce the items in stock
		}
		else
		{
			pItem->reduceAmount( amount );
		}
	}

	//resend gold and weight
	socket->sendStatWindow();
	socket->send( &clearBuy );

	if ( fromWhere == 1 )
		pVendor->talk( tr( "The total of thy purchase is %1 gold, which has been withdrawn from your bank account. My thanks for the patronage." ).arg( totalValue ), 0xFFFF, 0, false, pChar->socket() );
	else if ( fromWhere == 0 )
		pVendor->talk( tr( "The total of thy purchase is %1 gold. My thanks for the patronage." ).arg( totalValue ), 0xFFFF, 0, false, pChar->socket() );
	else
		pVendor->talk( tr( "The total of thy purchase is %1 gold, which has been withdrawn from your bag and from your bank account. My thanks for the patronage." ).arg( totalValue ), 0xFFFF, 0, false, pChar->socket() );

	pChar->socket()->soundEffect( 0x32 );
}

void sellAction( cUOSocket* socket, cUORxSell* packet )
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

	quint32 itemCount = packet->itemCount();

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

	quint32 totalValue = 0;
	quint32 i;
	map<SERIAL, quint16> items;

	for ( i = 0; i < itemCount; ++i )
	{
		P_ITEM pItem = FindItemBySerial( packet->iSerial( i ) );

		if ( !pItem )
		{
			socket->sysMessage( tr( "Invalid item sold." ) );
			socket->send( &clearBuy );
			return;
		}

		quint16 amount = packet->iAmount( i );

		// First an equal item with higher amount must be in the vendors sell container!
		bool found = false;
		for ( ContainerIterator it( pPurchase ); !it.atEnd(); ++it )
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
		if ( pItem->getOutmostChar() != pChar )
		{
			socket->sysMessage( tr( "Invalid item sold." ) );
			socket->send( &clearBuy );
			return;
		}

		unsigned int sellprice = pItem->getSellPrice( pVendor, pChar );

		if ( !sellprice || !pItem->getBuyPrice( pVendor, pChar ) )
		{
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
		for ( map<SERIAL, quint16>::iterator iter = items.begin(); iter != items.end(); ++iter )
		{
			P_ITEM pItem = FindItemBySerial( iter->first );
			quint16 amount = wpMin<quint16>( pItem->amount(), iter->second );

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
				pSold->setTag( "buy_time", Server::instance()->time() );
				pBought->addItem( pSold );
				pSold->update();

				if ( pItem->amount() <= amount )
				{
					pItem->removeFromView();
					pItem->remove();
				}
				else
				{
					pItem->setAmount( pItem->amount() - amount );
					pItem->update();
				}
			}
			else
			{
				pItem->removeFromView();
				pBought->addItem( pItem );
			}
		}
	}

	socket->send( &clearBuy );
	pVendor->talk( tr( "Thank you %1, here are your %2 gold" ).arg( pChar->name() ).arg( totalValue ) );

	// New Monetary?
	if (Config::instance()->usenewmonetary())
	{

		// Lets Assign Region
		cTerritory* Region = pVendor->region();

		// Lets Assign the IDs
		QString idfirst = Config::instance()->defaultFirstCoin();
		QString idsecond = Config::instance()->defaultSecondCoin();
		QString idthird = Config::instance()->defaultThirdCoin();

		// Lets try to find the IDs ahn?
		if ( Region )
		{
			idfirst = Region->firstcoin();
			idsecond = Region->secondcoin();
			idthird = Region->thirdcoin();
		}

		// Use Reversed Monetary?
		if (Config::instance()->usereversedvaluable()) {
			if ( totalValue < 10)
			{
				P_ITEM pMoney = cItem::createFromScript( idthird );
				pMoney->setAmount( totalValue );
				pMoney->toBackpack( pChar );
			}
			else if ( totalValue < 100)
			{
				// Silver First
				P_ITEM pMoney = cItem::createFromScript( idsecond );
				pMoney->setAmount( totalValue / 10 );
				pMoney->toBackpack( pChar );
				// Now, Copper
				if ( totalValue % 10 )
				{
					P_ITEM pMoney = cItem::createFromScript( idthird );
					pMoney->setAmount( totalValue % 10 );
					pMoney->toBackpack( pChar );
				}
			}
			else
			{
				// Gold First
				P_ITEM pMoney = cItem::createFromScript( idfirst );
				pMoney->setAmount( totalValue / 100 );
				pMoney->toBackpack( pChar );
				// Now, Silver
				if ( totalValue % 100 )
				{
					P_ITEM pMoney = cItem::createFromScript( idsecond );
					pMoney->setAmount( ( totalValue % 100 ) / 10 );
					pMoney->toBackpack( pChar );
				}
				// Finally Copper
				if ( totalValue % 10 )
				{
					P_ITEM pMoney = cItem::createFromScript( idthird );
					pMoney->setAmount( totalValue % 10 );
					pMoney->toBackpack( pChar );
				}
			}
		}
		// No... use normal Base
		else
		{
			P_ITEM pMoney = cItem::createFromScript( idfirst );
			pMoney->setAmount( totalValue );
			pMoney->toBackpack( pChar );
		}
		// Sound
		pChar->goldSound( totalValue, false );
	}
	// No, old Monetary
	else
	{
		pChar->giveGold( totalValue, false );
	}


	socket->sendStatWindow(); // Update Gold
}
}
