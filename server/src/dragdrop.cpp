//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
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
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

// dragdrop.cpp: implementation of dragging and dropping
// cut from wolfpack.cpp by Duke, 23.9.2000
//////////////////////////////////////////////////////////////////////

#include "debug.h"
#include "basics.h"
#include "wolfpack.h"
#include "SndPkg.h"
#include "speech.h"
#include "itemid.h"
#include "bounty.h"
#include "trigger.h"
#include "guildstones.h"
#include "regions.h"
#include "srvparams.h"
#include "classes.h"
#include "mapstuff.h"
#include "network.h"

#undef  DBGFILE
#define DBGFILE "dragdrop.cpp"

// Lord Binaries "Three Dimension" Drop-bug-fix
/* if (clientDimension[s]==3)
{
  // UO:3D clients send SOMETIMES two dragg packets for a single dragg action.
  // sometimes we HAVE to swallow it, sometimes it has to be interpreted
  // if UO:3D specific item loss problems are reported, this is probably the code to blame :)
  // LB
  P_ITEM pi = FindItemBySerial(pp->Iserial);

  if( (pp->TxLoc==-1) && (pp->TyLoc==-1) && (pp->Tserial==0)  && (EVILDRAGG[s]==1) )
  {
	  EVILDRAGG[s]=0;
	  return;
  }
  else if( (pp->TxLoc==-1) && (pp->TyLoc==-1) && (pp->Tserial==0)  && (EVILDRAGG[s]==0) )
  {
	  bounceItem( ps, pi );
	  return;
  }
  else if( ( (pp->TxLoc!=-1) && (pp->TyLoc!=-1) && ( pp->Tserial!=-1)) || ( (isItemSerial(pp->Iserial)) && (isItemSerial(pp->Tserial)) ) ) 
	  EVILDRAGG[s] = 1;
  else 
	  EVILDRAGG[s] = 0;
} */

// New Class implementation
void cDragItems::grabItem( P_CLIENT client )
{
	// Get our character
	P_CHAR pChar = client->player();
	if( pChar == NULL )
		return;

	// Fetch the grab information
	SERIAL iSerial = LongFromCharPtr( &buffer[ client->socket() ][ 1 ] );
	UI16 amount = ShortFromCharPtr( &buffer[ client->socket() ][ 5 ] );

	P_ITEM pItem = FindItemBySerial( iSerial );

	if( !pItem )
		return;

	// Are we already dragging an item ?
	// Bounce it and reject the move
	// (Logged out while dragging an item)
	if( client->dragging() )
	{
		bounceItem( client, client->dragging() );
		bounceItem( client, pItem, true );
		return;
	}

	// Do we really want to let him break his meditation
	// When he picks up an item ?
	// Maybe a meditation check here ?!?
	pChar->disturbMed( client->socket() ); // Meditation

	P_CHAR itemOwner = GetPackOwner( pItem, 64 );

	// Try to pick something out of another characters posessions
	if( itemOwner && ( itemOwner != pChar ) && ( !pChar->Owns( itemOwner ) ) )
	{
		client->sysMessage( QString( "You have to steal the %1 out of %2's posessions." ).arg( pItem->getName() ).arg( itemOwner->name.c_str() ) );
		bounceItem( client, pItem, true );
		return;
	}

	// Check if the user can grab the item
	if( !pChar->canPickUp( pItem ) )
	{
		client->sysMessage( "You cannot pick that up." );
		bounceItem( client, pItem, true );
		return;
	}

	// The user can't see the item
	// Basically thats impossible as the client should deny moving the item
	// if it's not in line of sight but to prevent exploits
	if( !line_of_sight( client->socket(), pChar->pos, pItem->pos, TREES_BUSHES|WALLS_CHIMNEYS|DOORS|ROOFING_SLANTED|FLOORS_FLAT_ROOFING|LAVA_WATER ) )
	{
		client->sysMessage( "You can't see the item." );
		bounceItem( client, pItem, true );
		return;
	}

	P_ITEM outmostCont = GetOutmostCont( pItem, 64 );  

	// If it's a trade-window, reset the ack-status
	if( outmostCont && ( outmostCont->contserial == pChar->serial ) && ( outmostCont->layer() == 0 ) && ( outmostCont->id() == 0x1E5E ) )
	{
		// Get the other sides tradewindow
		P_ITEM tradeWindow = FindItemBySerial( calcserial( outmostCont->moreb1(), outmostCont->moreb2(), outmostCont->moreb3(), outmostCont->moreb4() ) );

		// If one of the trade-windows has the ack-status reset it
		if( tradeWindow && ( tradeWindow->morez || outmostCont->morez ) )
		{
			tradeWindow->morez = 0;
			outmostCont->morez = 0;
			sendtradestatus( tradeWindow, outmostCont );
		}
	}

	// If the top-most container ( thats important ) is a corpse 
	// and looting is a crime, flag the character criminal.
	if( outmostCont && outmostCont->corpse() )
	{
		// For each item we take out we loose carma
		// if the corpse is innocent and not in our guild
		bool sameGuild = ( GuildCompare( pChar, FindCharBySerial( outmostCont->ownserial ) ) != 0 );

		if( ( outmostCont->more2 == 1 ) && !pChar->Owns( outmostCont ) && !sameGuild )
		{
			pChar->karma -= 5;
			criminal( pChar );
			client->sysMessage( "You lost some karma." );
		}
	}

	// Check if the item is too heavy
	//if( !pc_currchar->isGMorCounselor() )
	//{
	//} << Deactivated (DarkStorm)

	// ==== Grabbing the Item is allowed here ====
	
	// Remove eventual item-bonusses if we're unequipping something
	if( pItem->layer() > 0 ) 
	{
		P_CHAR wearer = FindCharBySerial( pItem->contserial );

		if( wearer )
			wearer->removeItemBonus( pItem );
	}

	// Send the user a pickup sound if we're picking it up
	// From a container/paperdoll
	if( !pItem->isInWorld() )
		soundeffect( client->socket(), 0x00, 0x57 );
	
	// If we're picking up a specific amount of what we got
	// Take that into account
	if( pItem->amount() > 1 )
	{
		UI32 pickedAmount = min( amount, pItem->amount() );

		// We only have to split if we're not taking it all
		if( pickedAmount != pItem->amount() )
		{
			P_ITEM splitItem = new cItem( *pItem ); // Create a new item to pick that up
			splitItem->SetSerial( cItemsManager::getInstance()->getUnusedSerial() );
			splitItem->setAmount( pItem->amount() - pickedAmount );
			splitItem->setContSerial( pItem->contserial );
			splitItem->SetOwnSerial( pItem->ownserial );
			splitItem->SetSpawnSerial( pItem->spawnserial );

			// He needs to see the new item
			RefreshItem( splitItem ); 

			// If we're taking something out of a spawn-region it's spawning "flag" is removed isn't it?
			pItem->SetSpawnSerial( INVALID_SERIAL );
			pItem->setAmount( pickedAmount );
		}
	}
	
	pItem->setContSerial( pChar->serial );
	pItem->SetMultiSerial( INVALID_SERIAL ); 
	pItem->setLayer( 0x1E );
	
	// It's in the equipment of another character
	if( itemOwner && ( itemOwner != pChar ) )
	{
		itemOwner->weight -= pItem->getWeight();
		statwindow( calcSocketFromChar( itemOwner ), itemOwner );
	}

	// If the item is in the bank or any sell-container it's NOT counted as char-weight
	bool inBank = ( outmostCont && 	( outmostCont->contserial == pChar->serial ) && ( outmostCont->layer() >= 0x1A ) );

	// Add the weight if:
	//  - Picked from ground
	//  - Picked out of another character
	//  - Picked out of our bank or any other non-visible container
	if( ( itemOwner != pChar ) || !inBank )
	{
		pChar->weight += pItem->getWeight();	
		statwindow( client->socket(), pChar );
	}
}

// Tries to equip an item
// if that fails it tries to put the item in the users backpack
// if *that* fails it puts it at the characters feet
// That works for NPCs as well
void equipItem( P_CHAR wearer, P_ITEM item )
{
	tile_st tile;

	Map->SeekTile( item->id(), &tile );

	// User cannot wear the item
	if( tile.layer == 0 )
	{
		if( online( wearer ) )
			sysmessage( calcSocketFromChar( wearer ), "You cannot wear that item." );

		item->toBackpack( wearer );
		return;
	}

	vector< SERIAL > equipment = contsp.getData( wearer->serial );	

	// If n item on the same layer is already equipped, unequip it
	for( UI08 i = 0; i < equipment.size(); i++ )
	{
		P_ITEM equip = FindItemBySerial( equipment[ i ] ); 
		
		// Unequip the item and free the layer that way
		if( equip && ( equip->layer() == tile.layer ) )
			equip->toBackpack( wearer );

		wearer->removeItemBonus( equip );
	}

	// *finally* equip the item
	item->setContSerial( wearer->serial );

	// Add the item bonuses
	wearer->st = (wearer->st + item->st2);
	wearer->chgDex( item->dx2 );
	wearer->in = (wearer->in + item->in2);
}

void cDragItems::bounceItem( P_CLIENT client, P_ITEM pItem, bool denyMove )
{
	// Reject the move of the item
	cBounceItem pBounce( denyMove );
	pBounce.send( client->socket() );

	// If the Client is *not* dragging the item we don't need to reset it to it's original location
	if( !client->dragging() )
		return;

	// Sends the item to the backpack of char (client)
	pItem->toBackpack( client->player() );

	// When we're dropping the item to the ground let's play a nice sound-effect
	// to all in-range sockets
	if( pItem->isInWorld() )
	{
		for( UOXSOCKET s; s < now; s++ )
			if( inrange2( s, pItem ) )
				soundeffect( s, 0x00, 0x42 );
	}
	else
		soundeffect( client->socket(), 0x00, 0x57 );
}

void cDragItems::equipItem( P_CLIENT client )
{
	// Get the packet information
	SERIAL itemId = LongFromCharPtr( &buffer[ client->socket() ][ 1 ] );
	SERIAL playerId = LongFromCharPtr( &buffer[ client->socket() ][ 6 ] );

	P_ITEM pItem = FindItemBySerial( itemId );
	P_CHAR pWearer = FindCharBySerial( playerId );

	if( !pItem || !pWearer )
		return;

	P_CHAR pChar = client->player();

	// We're dead and can't do that
	if( pChar->dead )
	{
		client->sysMessage( "You are dead and can't do that." );
		bounceItem( client, pItem );
		return;
	}

	// Our target is dead
	if( ( pWearer != pChar ) && pWearer->dead )
	{
		client->sysMessage( "You can't equip dead players." );
		bounceItem( client, pItem );
		return;
	}

	// Get our tile-information
	tile_st pTile;
	Map->SeekTile( pItem->id(), &pTile );

	// Is the item wearable ? ( layer == 0 | equip-flag not set )
	// Multis are not wearable are they :o)
	if( pTile.layer == 0 || !( pTile.flag3 & 0x40 ) || pItem->isMulti() )
	{
		client->sysMessage( "This item cannot be equipped." );
		bounceItem( client, pItem );
		return;
	}

	// Required Strength
	if( pItem->st > pWearer->st )
	{
		if( pWearer == pChar )
			client->sysMessage( "You cannot wear that item, you seem not strong enough" );
		else
			client->sysMessage( "This person can't wear that armor, it seems not strong enough" );

		bounceItem( client, pItem );
		return;
	}

	// Required Dexterity
	if( pItem->dx > pWearer->effDex() )
	{
		if( pWearer == pChar )
			client->sysMessage( "You cannot wear that item, you seem not agile enough" );
		else
			client->sysMessage( "This person can't wear that armor, it seems not agile enough" );

		bounceItem( client, pItem );
		return;
	}
	
	// Required Intelligence
	if( pItem->in > pWearer->in )
	{
		if( pWearer == pChar )
			client->sysMessage( "You cannot wear that item, you seem not smart enough" );
		else
			client->sysMessage( "This person can't wear that armor, it seems not smart enough" );

		bounceItem( client, pItem );
		return;
	}

	// Males can't wear female armor
	if( ( pChar->id() == 0x0190 ) && ( pItem->id() >= 0x1C00 ) && ( pItem->id() <= 0x1C0D ) )
	{
		client->sysMessage( "You cannot wear female armor." );
		bounceItem( client, pItem );
		return;
	}

	// Needs a check (!)
	// Checks for equipment on the same layer
	// If there is any it tries to unequip it
	// If that fails it cancels
	UI08 layer = pItem->layer();
	vector< SERIAL > equipment = contsp.getData( pWearer->serial );

	for( UI32 i = 0; i < equipment.size(); i++ )
	{
		P_ITEM pEquip = FindItemBySerial( equipment[ i ] );

		if( pEquip )
			continue;

		// We found an item which is on the same layer (!)
		// Unequip it if we can 
		bool twoHanded = false;

		if( pEquip->twohanded() && ( layer == 1 || layer == 2 ) )
			twoHanded = true;

		if( pItem->twohanded() && ( pEquip->layer() == 1 || pEquip->layer() == 2 ) )
			twoHanded = true;
			
		if( ( pEquip->layer() == layer ) || twoHanded )
		{
			if( pChar->canPickUp( pEquip ) ) // we're equipping so we do the check
				pEquip->toBackpack( pWearer );

			// If it's still on the char: cancel equipment
			if( pEquip->contserial == pWearer->serial )
			{
				client->sysMessage( "You already have an item on that layer." );
				bounceItem( client, pItem );
				return;
			}
		}
	}

	// At this point we're certain that we can wear the item
	pItem->setContSerial( playerId );
	pItem->setLayer( pTile.layer ); // Don't trust the user input on this one

	// Handle the weight if the item is leaving our "body"
	if( pWearer != pChar )
	{
		pChar->weight -= pItem->getWeight();
		pWearer->weight += pItem->getWeight();

		// Update the status-windows
		statwindow( client->socket(), pChar );
		statwindow( calcSocketFromChar( pWearer ), pWearer );
	}

	if( pTile.layer == 0x19 )
		pWearer->setOnHorse( true );

	// Apply the bonuses
	pWearer->st += pItem->st2;
	pWearer->chgDex( pItem->dx2 );
	pWearer->in += pItem->in2;

	// Show debug information if requested
	if( showlayer )
		clConsole.send( QString( "Item (%1) equipped on layer %2" ).arg( pItem->name() ).arg( pItem->layer() ) );

	// I don't think we need to remove the item
	// as it's only visible to the current char
	// And he looses contact anyway
	// SndRemoveitem( pi->serial );

	// Build our packets
	cWornItems wearItem( pWearer->serial, pItem->serial, pItem->layer(), pItem->id(), pItem->color() );
	cSoundEffect soundEffect( 0x57, pWearer->pos );

	// Send to all sockets in range
	// ONLY the new equipped item and the sound-effect
	for( UOXSOCKET s = 0; s < now; s++ )
		if( perm[s] && inrange1p( pWearer, currchar[s] ) )
		{
			soundEffect.send( s );
			wearItem.send( s );
		}

	// Lord Binaries Glow stuff
	if( pItem->glow != INVALID_SERIAL )
	{
		pChar->removeHalo( pItem );
		pWearer->addHalo( pItem );
		pWearer->glowHalo( pItem );
	}
}

void cDragItems::dropItem( P_CLIENT client )
{
	P_CHAR pChar = client->player();

	// Get the data
	SERIAL itemId = LongFromCharPtr( &buffer[ client->socket() ][ 1 ] );
	SERIAL contId = LongFromCharPtr( &buffer[ client->socket() ][ 10 ] );

	Coord_cl dropPos = pChar->pos; // plane+map
	dropPos.x = ShortFromCharPtr( &buffer[ client->socket() ][ 5 ] );
	dropPos.y = ShortFromCharPtr( &buffer[ client->socket() ][ 7 ] );
	dropPos.z = buffer[ client->socket() ][ 9 ];

	// Get possible containers
	P_ITEM pItem = FindItemBySerial( itemId );
	
	if( !pItem )
		return;

	P_ITEM iCont = FindItemBySerial( contId );
	P_CHAR cCont = FindCharBySerial( contId );

	/* >> SEE LORD BINARIES DROPFIXBUGFIXBUG << */

	// A completely invalid Drop packet
	if( !iCont && !cCont && ( dropPos.x == 0xFFFF ) && ( dropPos.y == 0xFFFF ) && ( dropPos.z == 0xFF ) )
	{
		bounceItem( client, pItem );
		return;
	}

	// Item dropped on Ground
	if( !iCont && !cCont )
		dropOnGround( client, pItem, dropPos );
	// Item dropped on another item
	else if( iCont )
		dropOnItem( client, pItem, iCont, dropPos );
	// Item dropped on char
	else if( cCont )
		dropOnChar( client, pItem, cCont );
}

void cDragItems::dropOnChar( P_CLIENT client, P_ITEM pItem, P_CHAR pOtherChar )
{
	// Three possibilities:
	// If we're dropping it on ourself: packintobackpack
	// If we're dropping it on some other player: trade-window
	// If we're dropping it on some NPC: checkBehaviours
	// If not handeled: Equip the item if the NPC is owned by us
	
	P_CHAR pChar = client->player();

	// Dropped on ourself
	if( pChar == pOtherChar )
	{
		pItem->setLayer( 0 );
		pItem->setContSerial( INVALID_SERIAL );
		pItem->toBackpack( pChar );
		return;
	}

	// Are we in range of our target
	if( !inrange1p( pChar, pOtherChar ) )
	{
		client->sysMessage( "You are too far away from that character." );
		bounceItem( client, pItem );
		return;
	}

	// Can wee see our target
	if( !line_of_sight( client->socket(), pChar->pos, pOtherChar->pos, TREES_BUSHES|WALLS_CHIMNEYS|DOORS|ROOFING_SLANTED|FLOORS_FLAT_ROOFING|LAVA_WATER ) )
	{
		client->sysMessage( "You can't see this character" );
		bounceItem( client, pItem );
		return;
	}

	// Open a secure trading window
	if( !pOtherChar->isNpc() && online( pOtherChar ) )
	{
		// Check if we're already trading, 
		// if not create a new window
		vector< SERIAL > equipment = contsp.getData( pChar->serial );
		P_ITEM tradeWindow = NULL;

		for( UI16 i = 0; i < equipment.size(); i++ )
		{
			P_ITEM pEquip = FindItemBySerial( equipment[ i ] );
			
			// Is it a trade-window ?
			if( ( pEquip->layer() == 0 ) && ( pEquip->id() == 0x1E5E ) )
			{
				P_ITEM tradeWindow = FindItemBySerial( calcserial( pEquip->moreb1(), pEquip->moreb2(), pEquip->moreb3(), pEquip->moreb4() ) );
				if( tradeWindow && ( tradeWindow->contserial == pOtherChar->serial ) )
				{
					tradeWindow = pEquip;
					break;
				}
			}
		}

		if( !tradeWindow )
			tradeWindow = Trade->tradestart( client->socket(), pOtherChar );

		pItem->setContSerial( tradeWindow->serial);
		pItem->pos.x = rand() % 60;
		pItem->pos.y = rand() % 60;
		pItem->pos.z = 9;
		pItem->setLayer( 0 );
		SndRemoveitem( pItem->serial );
		RefreshItem( pItem );
		return;
	}

	// For our hirelings we have a special function
	if( pChar->Owns( pOtherChar ) )
	{
		dropOnPet( client, pItem, pOtherChar );
		return;
	}

	// Dropping based on AI Type
	switch( pOtherChar->npcaitype() )
	{
	case 4:
		dropOnGuard( client, pItem, pOtherChar );
		break;
	case 5:
		dropOnBeggar( client, pItem, pOtherChar );
		break;
	/*case 8:
		dropOnBanker( client, pItem, pOtherChar );
		break;*/
	/*case 19:
		dropOnBroker( client, pItem, pOtherChar );
		break;*/
	};

	// Try to train - works for any NPC
	if( pOtherChar->cantrain() )
		if( pChar->trainer() == pOtherChar->serial )
			dropOnTrainer( client, pItem, pOtherChar );
		else
			pOtherChar->talk( "You need to tell me what you want to learn first" );

	bounceItem( client, pItem );
	return;
}

void cDragItems::dropOnGround( P_CLIENT client, P_ITEM pItem, const Coord_cl &pos )
{
	P_CHAR pChar = client->player();

	// Check if the destination is in line of sight
	if( !line_of_sight( client->socket(), pChar->pos, pos, TREES_BUSHES|WALLS_CHIMNEYS|DOORS|ROOFING_SLANTED|FLOORS_FLAT_ROOFING|LAVA_WATER ) )
	{
		client->sysMessage( "You cannot see the target." );
		bounceItem( client, pItem );
		return;
	}

	if( !pChar->canPickUp( pItem ) )
	{
		bounceItem( client, pItem );
		return;
	}

	pItem->setContSerial( INVALID_SERIAL );
	pItem->moveTo( pos );	
	pItem->setLayer( 0 );
	RefreshItem( pItem ); // Send it to all clients in range

	pChar->weight -= pItem->getWeight();
	statwindow( client->socket(), pChar ); // Update our weight-stats

	if( pItem->glow != INVALID_SERIAL )
	{
		pChar->removeHalo( pItem );
		pChar->glowHalo( pItem );
	}

	// Multi handling (Hm i don't like that...)
	if( pChar->multis > 0 )
	{
		P_ITEM pMulti = FindItemBySerial( pChar->multis );
		if( pMulti != NULL )
		{
			pMulti = findmulti( pItem->pos );
			if( pItem != NULL )
				pItem->SetMultiSerial( pMulti->serial );
		}
	}
}

void cDragItems::dropOnItem( P_CLIENT client, P_ITEM pItem, P_ITEM pCont, const Coord_cl &dropPos )
{
	P_CHAR pChar = client->player();
	
	if( pItem->isMulti() )
	{
		client->sysMessage( "You cannot put houses in containers" );
		bounceItem( client, pItem );
		return;
	}
	
	// If the target belongs to another character 
	// It needs to be our vendor or else it's denied
	P_CHAR packOwner = GetPackOwner( pCont );

	if( ( packOwner != NULL ) && ( packOwner != pChar ) )
	{
		// For each item someone puts into there 
		// He needs to do a snoop-check
		if( pChar->canSnoop() )
		{
			if( !Skills->CheckSkill( pChar, SNOOPING, 0, 1000 ) )
			{

				client->sysMessage( QString( "You fail to put that into %1's pack" ).arg( packOwner->name.c_str() ) );
				bounceItem( client, pItem );
				return;
			}
		}

		if( !packOwner->isNpc() || ( packOwner->npcaitype() != 17 ) || !pChar->Owns( packOwner ) )
		{
			client->sysMessage( "You cannot put that into the belongings of another player" );
			bounceItem( client, pItem );
			return;
		}
	}

	// If we put the item into a trade-window
	// Reset the trade-status for both players
	if( pCont->layer() == 0 && pCont->id() == 0x1E5E &&	pChar->Wears( pCont ) )
	{
		// Trade window???
		P_ITEM tradeWindow = FindItemBySerial( calcserial( pCont->moreb1(), pCont->moreb2(), pCont->moreb3(), pCont->moreb4() ) );

		// If it *IS* a trade-window, replace the status
		if( tradeWindow && ( pCont->morez || tradeWindow->morez ) )
			{
				tradeWindow->morez = 0;
				pCont->morez = 0;
				sendtradestatus( tradeWindow, pCont );
			}
	}
	
	//
	// AntiChrist - Special Bank Stuff
	//
	// if morey == 123  - gold only bank
	// and morex == 1
	//
	if( SrvParams->useSpecialBank() )
	{
		if( pCont->morey == 123 && pCont->morex == 1 && pCont->type() == 1 )
		{
			// Only gold is allowed in this bank
			if( pItem->id() == 0xEED )
			{ 
				goldsfx( client->socket(), 2 );
			} 
			else // bonce back
			{
				client->sysMessage( "You can only put gold in this bank box!" );
				bounceItem( client, pItem );
				return;
			}
		}
	}
	
	if( !pChar->canPickUp( pItem ) )
	{
		bounceItem( client, pItem );
		return;
	}

	// Trash can
	if( pCont->type()==87 )
	{
		Items->DeleItem( pItem );
		client->sysMessage( "As you let go of the item it disappears." );
		return;
	}

	// Spell Book
	if( pCont->type() == 9 )
	{
		UI08 spellId = Magic->calcSpellId( pItem->id() );

		if( spellId < 0 )
		{
			client->sysMessage( "You can only put scrolls into a spellbook" );
			bounceItem( client, pItem );
			return;
		}		

		if( Magic->hasSpell( pCont, spellId )  )
		{
			client->sysMessage( "That spellbook already contains this spell" );
			bounceItem( client, pItem );
			return;
		}
	}

	// We drop something on the belongings of one of our playervendors
	if( ( packOwner != NULL ) && ( packOwner->npcaitype() == 17 ) && pChar->Owns( packOwner ) )
	{
		client->sysMessage( "You drop something into your playervendor" );
		bounceItem( client, pItem );
		return;
	}

	// Playervendors (chest equipped by the vendor - opened to the client)
	/*if( !( pCont->pileable() && pItem->pileable() && pCont->id() == pItem->id() || ( pCont->type() != 1 && pCont->type() != 9 ) ) )
	{
		P_CHAR pc_j = GetPackOwner(pCont);
		if (pc_j != NULL)
		{
			if (pc_j->npcaitype() == 17 && pc_j->isNpc() && pChar->Owns(pc_j))
			{
				pChar->inputitem = pItem->serial;
				pChar->inputmode = cChar::enPricing;
				sysmessage(s, "Set a price for this item.");
			}
		}

		short xx=pp->TxLoc;
		short yy=pp->TyLoc;

		pCont->AddItem(pItem,xx,yy);
	
		soundeffect2( pChar, 0x57 );
		statwindow(s, pChar);
	}
	
	else*/
	
	// We may also drop into *any* locked chest
	// So we can have post-boxes ;o)
	// Spellbooks are containers for us as well
	if( pCont->type() == 9 || pCont->type() == 1 || pCont->type() == 8 || pCont->type() == 63 || pCont->type() == 65 || pCont->type() == 66 )
	{
		pItem->setContSerial( pCont->serial );
		pItem->setLayer( 0 ); // Remove it from our drag-layer

		// Huh ? - Make that random will you!
		pItem->pos = dropPos;
		
		SndRemoveitem( pItem->serial );
		RefreshItem( pItem );
		
		// Dropped on another Container/in another Container
		soundeffect2( pChar, 0x57 );

		return;
	}
	// Item matching needs to be extended !!! at least Color! (for certain types)
	else if ( pCont->isPileable() && pItem->isPileable() && ( pCont->id() == pItem->id() ) )
	{
		if( pCont->amount() + pItem->amount() <= 65535 )
		{
			pCont->setAmount( pCont->amount() + pItem->amount() );
			Items->DeleItem( pItem );

			RefreshItem( pCont ); // Need to update the amount
			return;
		}
		// We have to *keep* our current item
		else
		{
			pCont->setAmount( 65535 ); // Max out the amount
			RefreshItem( pCont );

			// The delta between 65535 and pCont->amount() sub our Amount is the
			// new amount
			pItem->setAmount( pItem->amount() - ( 65535 - pCont->amount() ) );
		}
	}

	// We dropped the item NOT on a container
	// And were *un*able to stack it (!)
	// >> Set it to the location of the item we dropped it on and stack it up by 1
	pItem->moveTo( pCont->pos );
	pItem->pos.z++; // Increase z by 1
	pItem->pos.y++; // To get it visualized do that with y as well
	pItem->setLayer( 0 );
	pItem->setContSerial( pCont->contserial );
	RefreshItem( pItem );
				
	// This needs to be checked
	// It annoyingly shows the spellbook
	// whenever you add a scroll
	if( pCont->type() == 9 )
		Magic->openSpellBook( pChar, pCont );

	// Glowing Objects moved between chars
	if( pItem->glow != INVALID_SERIAL )
	{
		pChar->removeHalo( pItem );
				
		if( packOwner != NULL )
		{
			packOwner->addHalo(pItem);
			packOwner->glowHalo(pItem);
		}
	}
}

// Item was dropped on a pet
void cDragItems::dropOnPet( P_CLIENT client, P_ITEM pItem, P_CHAR pPet )
{
	// Feed our pets
	if( ( pPet->hunger() >= 6 ) || pItem->type() != 14 )
	{
		client->sysMessage( "It doesn't seem to want your item" );
		bounceItem( client, pItem );
		return;
	}

	// We have three different eating-sounds (I don't like the idea as they sound too human)
	pPet->soundEffect( 0x3A + RandomNum( 1, 3 ) );

	// If you want to poison a pet... Why not
	if( pItem->poisoned && pPet->poisoned() < pItem->poisoned )
	{
		pPet->soundEffect( 0x246 );
		pPet->setPoisoned( pItem->poisoned );
		
		// a lev.1 poison takes effect after 40 secs, a deadly pois.(lev.4) takes 40/4 secs - AntiChrist
		pPet->setPoisontime( uiCurrentTime + ( MY_CLOCKS_PER_SEC * ( 40 / pPet->poisoned() ) ) );
		
		//wear off starts after poison takes effect - AntiChrist
		pPet->setPoisonwearofftime(pPet->poisontime() + ( MY_CLOCKS_PER_SEC * SrvParams->poisonTimer() ) );
		
		// Refresh the health-bar of our target
		impowncreate( client->socket(), pPet, 1 );
	}

	// *You see Snowwhite eating some poisoned apples*
	// Color: 0x0026
	QString emote = QString( "*You see %1 eating %2*" ).arg( pPet->name.c_str() ).arg( pItem->getName() );
	pPet->emote( emote );

	// We try to feed it more than it needs
	if( pPet->hunger() + pItem->amount() > 6 )
	{
		pItem->setAmount( pItem->amount() - ( 6 - pPet->hunger() ) );
		pPet->setHunger( 6 );

		// Pack the rest into his backpack
		bounceItem( client, pItem );
		return;
	}

	pPet->setHunger( pPet->hunger() + pItem->amount() );
	Items->DeleItem( pItem );
}

void cDragItems::dropOnGuard( P_CLIENT client, P_ITEM pItem, P_CHAR pGuard )
{
	// Only heads for bountys are accepted
	if( !pItem->name().contains( "the head of" ) || !pItem->owner())
	{
		pGuard->talk( "Bring that to a merchant if you want to sell it!" );
		client->sysMessage( "I do not want that, citizen!" );
		bounceItem( client, pItem );
		return;
	}

	P_CHAR pVictim = pItem->owner();

	if( pVictim->questBountyReward() <= 0 )
	{
		pGuard->talk( "You can not claim a prize for innocent citizens!. You are lucky I don't strike you down where you stand!" );
		bounceItem( client, pItem );
		return;
	}

	if( pVictim == client->player() )
	{
		pGuard->talk( "You can not claim that prize scoundrel. You are lucky I don't strike you down where you stand!" );
		Items->DeleItem( pItem ); // The guard wont give the head back...
		return;
	}

	addgold( client->socket(), pVictim->questBountyReward() );
	goldsfx( client->socket(), pVictim->questBountyReward() );
	Bounty->BountyDelete( pVictim->serial );
	
	// Thank them for their work
	pGuard->talk( QString( "Excellent work! You have brought us the head of %1. Here is your reward of %2 gold coins." ).arg( pVictim->name.c_str() ).arg( pVictim->questBountyReward() ) );

	client->player()->karma += 100;
}

void cDragItems::dropOnBeggar( P_CLIENT client, P_ITEM pItem, P_CHAR pBeggar )
{
	if( ( pBeggar->hunger() < 6 ) && pItem->type() == 14 )
	{
		pBeggar->talk( "*cough* Thank thee!" );
		pBeggar->soundEffect( 0x3A + RandomNum( 1, 3 ) );

		// If you want to poison a pet... Why not
		if( pItem->poisoned && pBeggar->poisoned() < pItem->poisoned )
		{
			pBeggar->soundEffect( 0x246 );
			pBeggar->setPoisoned( pItem->poisoned );
			
			// a lev.1 poison takes effect after 40 secs, a deadly pois.(lev.4) takes 40/4 secs - AntiChrist
			pBeggar->setPoisontime( uiCurrentTime + ( MY_CLOCKS_PER_SEC * ( 40 / pBeggar->poisoned() ) ) );
			
			//wear off starts after poison takes effect - AntiChrist
			pBeggar->setPoisonwearofftime( pBeggar->poisontime() + ( MY_CLOCKS_PER_SEC * SrvParams->poisonTimer() ) );
			
			// Refresh the health-bar of our target
			impowncreate( client->socket(), pBeggar, 1 );
		}


		// *You see Snowwhite eating some poisoned apples*
		// Color: 0x0026
		QString emote = QString( "*You see %1 eating %2*" ).arg( pBeggar->name.c_str() ).arg( pItem->getName() );
		pBeggar->emote( emote );

		// We try to feed it more than it needs
		if( pBeggar->hunger() + pItem->amount() > 6 )
		{
			client->player()->karma += ( 6 - pBeggar->hunger() ) * 10;

			pItem->setAmount( pItem->amount() - ( 6 - pBeggar->hunger() ) );
			pBeggar->setHunger( 6 );

			// Pack the rest into his backpack
			bounceItem( client, pItem );
			return;
		}

		pBeggar->setHunger( pBeggar->hunger() + pItem->amount() );
		client->player()->karma += pItem->amount() * 10;

		Items->DeleItem( pItem );
		return;
	}

	// No Food? Then it has to be Gold
	if( pItem->id() != 0xEED )
	{
		pBeggar->talk( "Sorry, but i can only use gold." );
		bounceItem( client, pItem );
		return;
	}

	pBeggar->talk( QString( "Thank you %1 for the %2 gold!" ).arg( client->player()->name.c_str() ).arg( pItem->amount() ) );
	client->sysMessage( "You have gained some karma!" );
	
	if( pItem->amount() <= 100 )
		client->player()->karma += 10;
	else
		client->player()->karma += 50;
	
	Items->DeleItem( pItem );
}

void cDragItems::dropOnBanker( P_CLIENT client, P_ITEM pItem, P_CHAR pBanker )
{
	bounceItem( client, pItem );
}

void cDragItems::dropOnBroker( P_CLIENT client, P_ITEM pItem, P_CHAR pBroker )
{
	bounceItem( client, pItem );
}

void cDragItems::dropOnTrainer( P_CLIENT client, P_ITEM pItem, P_CHAR pTrainer )
{
	bounceItem( client, pItem );
}

/*
static bool DeedDroppedOnBroker(P_CLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	UOXSOCKET s=ps->GetSocket();
	P_CHAR pc_currchar = ps->getPlayer();
	P_CHAR target = FindCharBySerial(pp->Tserial);
	P_ITEM bankbox = pc_currchar->GetBankBox();
	int value = static_cast<int>(pi->value/1.25);
	int total = value;
	
	if ((pi->morex >= 1 && pi->morex <= 14) || (pi->morex >= 16 && pi->morex <= 17) || (pi->morex >= 26 && pi->morex <= 32))
	{
		while ( value > 65000)
		{
			const P_ITEM pi_gold = Items->SpawnItem(s, pc_currchar, 65000, "#", 1, 0x0E, 0xED, 0, 0, 0);
		    if(pi_gold == NULL) return false;
			bankbox->AddItem(pi_gold);
			value -= 65000;
		}
		const P_ITEM pi_gold = Items->SpawnItem(s, pc_currchar, value, "#", 1, 0x0E, 0xED, 0, 0, 0);
	    if(pi_gold == NULL) return false;
		bankbox->AddItem( pi_gold );
		Items->DeleItem( pi ); // deed is consumed.
		sprintf((char*)temp,"%s thank you for your patronage, I have deposited %i gold into your bank account.",pc_currchar->name.c_str(), total);
		npctalk(s,target,(char*)temp,0);
	    statwindow(s, pc_currchar);
		return true;
	}
    else
	{
		  sprintf((char*)temp,"Sorry %s i can only accept house deeds.",pc_currchar->name.c_str());
		  npctalk(s,target,(char*)temp,0);
		  bounceItem( ps, pi );
	}
	return true;
}

static bool ItemDroppedOnBanker(P_CLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	UOXSOCKET s=ps->GetSocket();
	P_CHAR pc_currchar = ps->getPlayer();
	P_CHAR target = FindCharBySerial(pp->Tserial);
	P_ITEM bankbox = pc_currchar->GetBankBox();
	int amt = pi->amount();
	int value = pi->value;
	
	if (pi->id() == 0x14F0 && pi->type() == 1000)
	{
		while ( pi->value > 65000)
		{
			const P_ITEM pi_gold = Items->SpawnItem(s, pc_currchar, 65000, "#", 1, 0x0E, 0xED, 0, 0, 0);
		    if(pi_gold == NULL) return false;
			bankbox->AddItem(pi_gold);
			pi->value -= 65000;
		}
		const P_ITEM pi_gold = Items->SpawnItem(s, pc_currchar, pi->value, "#", 1, 0x0E, 0xED, 0, 0, 0);
	    if(pi_gold == NULL) return false;
		bankbox->AddItem( pi_gold );
		Items->DeleItem( pi ); // Check is consumed.
		sprintf((char*)temp,"%s I have cashed your check and deposited %i gold.",pc_currchar->name.c_str(), value);
		npctalk(s,target,(char*)temp,0);
	    statwindow(s, pc_currchar);
		return true;
	}
    else if (pi->id() == 0x0EED)
	{
		sprintf((char*)temp,"%s you have deposited %i gold.",pc_currchar->name.c_str(), amt);
		npctalk(s,target,(char*)temp,0);
		bankbox->AddItem(pi);
	    statwindow(s, pc_currchar);
		return true;
	}
    else
	{
		  sprintf((char*)temp,"Sorry %s i can only deposit gold",pc_currchar->name.c_str());
		  npctalk(s,target,(char*)temp,0);
		  bounceItem( ps, pi );
	}
	return true;
}

static bool ItemDroppedOnTrainer(P_CLIENT ps, PKGx08 *pp, P_ITEM pi)
{
	UOXSOCKET s = ps->GetSocket();
	P_CHAR pc_currchar = ps->getPlayer();
	P_CHAR pc_t = FindCharBySerial(pp->Tserial);

	if( pi->id() ==0x0EED )
	{ // They gave the NPC gold
		char sk=pc_t->trainingplayerin();
		npctalk(s, pc_t, "I thank thee for thy payment. That should give thee a good start on thy way. Farewell!",0);

		int sum = pc_currchar->getSkillSum();
		int delta = pc_t->getTeachingDelta(pc_currchar, sk, sum);

		if(pi->amount()>delta) // Paid too much
		{
			pi->setAmount( pi->amount()  - delta );
			bounceItem( ps, pi );
		}
		else
		{
			if(pi->amount() < delta)		// Gave less gold
				delta = pi->amount();		// so adjust skillgain
			Items->DeleItem(pi);
		}
		pc_currchar->setBaseSkill(sk, pc_currchar->baseSkill(sk) + delta);
		Skills->updateSkillLevel(pc_currchar, sk);
		updateskill(s,sk);

		pc_currchar->setTrainer(INVALID_SERIAL);
		pc_t->setTrainingplayerin('\xFF');
		itemsfx(s, pi->id());//AntiChrist - do the gold sound
		return true;
	}
	else // Did not give gold
	{
		npctalk(s, pc_t, "I am sorry, but I can only accept gold.",0);
		bounceItem( ps, pi );
	}
	return true;
}

//This crazy training stuff done by Anthracks (fred1117@tiac.net)
if(pc_currchar->trainer() != pTC->serial)
{
	npctalk(s, pTC, "Thank thee kindly, but I have done nothing to warrant a gift.",0);
	bounceItem( ps, pi );
	return true;
}
else // The player is training from this NPC
{
	ItemDroppedOnTrainer( ps, pp, pi);
	return true;
}
*/

