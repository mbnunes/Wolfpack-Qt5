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
#include "tilecache.h"
#include "speech.h"
#include "itemid.h"
#include "bounty.h"
#include "guildstones.h"
#include "regions.h"
#include "srvparams.h"
#include "skills.h"
#include "classes.h"
#include "mapstuff.h"
#include "network.h"
#include "network/uosocket.h"
#include "network/uorxpackets.h"
#include "network/uotxpackets.h"

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
void cDragItems::grabItem( cUOSocket *socket, cUORxDragItem *packet )
{
	// Get our character
	P_CHAR pChar = socket->player();
	if( !pChar )
		return;

	// Fetch the grab information
	UI16 amount = packet->amount();
	if( !amount )
		amount = 1;

	P_ITEM pItem = FindItemBySerial( packet->serial() );

	// If it's an invalid pointer we can't even bounce
	if( !pItem )
		return;

	// Are we already dragging an item ?
	// Bounce it and reject the move
	// (Logged out while dragging an item)
	if( socket->dragging() )
	{
		// TODO: Reason for dropping the item
		socket->bounceItem( socket->dragging(), BR_ALREADY_DRAGGING );
		return;
	}

	// Do we really want to let him break his meditation
	// When he picks up an item ?
	// Maybe a meditation check here ?!?
	pChar->disturbMed(); // Meditation

	P_CHAR itemOwner = GetPackOwner( pItem, 64 );

	// Try to pick something out of another characters posessions
	if( itemOwner && ( itemOwner != pChar ) && ( !pChar->Owns( itemOwner ) ) )
	{
		socket->bounceItem( pItem, BR_BELONGS_TO_SOMEONE_ELSE );
		return;
	}

	// Check if the user can grab the item
	if( !pChar->canPickUp( pItem ) )
	{
		socket->bounceItem( pItem, BR_CANNOT_PICK_THAT_UP );
		return;
	}

	// The user can't see the item
	// Basically thats impossible as the socket should deny moving the item
	// if it's not in line of sight but to prevent exploits
	/*if( !line_of_sight( socket->socket(), pChar->pos, pItem->pos, TREES_BUSHES|WALLS_CHIMNEYS|DOORS|ROOFING_SLANTED|FLOORS_FLAT_ROOFING|LAVA_WATER ) )
	{
		socket->sysMessage( "You can't see the item." );
		bounceItem( socket, pItem, true );
		return;
	}*/

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
			socket->sysMessage( "You lost some karma." );
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
		socket->soundEffect( 0x57, pItem );
	
	// If we're picking up a specific amount of what we got
	// Take that into account
	if( amount < pItem->amount() )
	{
		UI32 pickedAmount = QMIN( amount, pItem->amount() );

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
			splitItem->update();

			// If we're taking something out of a spawn-region it's spawning "flag" is removed isn't it?
			pItem->SetSpawnSerial( INVALID_SERIAL );
			pItem->setAmount( pickedAmount );
		}
	}
	
	//mapRegions->Remove( pItem );
	pItem->setContSerial( pChar->serial );
	pItem->SetMultiSerial( INVALID_SERIAL ); 
	pItem->setLayer( 0x1E );
}

// Tries to equip an item
// if that fails it tries to put the item in the users backpack
// if *that* fails it puts it at the characters feet
// That works for NPCs as well
void equipItem( P_CHAR wearer, P_ITEM item )
{
	tile_st tile = cTileCache::instance()->getTile( item->id() );

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
	wearer->giveItemBonus( item );
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
		for( UOXSOCKET s = 0; s < now; s++ )
			if( inrange2( s, pItem ) )
				soundeffect( s, 0x00, 0x42 );
	}
	else
		soundeffect( client->socket(), 0x00, 0x57 );
}

void cDragItems::equipItem( cUOSocket *socket, cUORxWearItem *packet )
{
	P_ITEM pItem = FindItemBySerial( packet->serial() );
	P_CHAR pWearer = FindCharBySerial( packet->wearer() );

	if( !pItem || !pWearer )
		return;

	P_CHAR pChar = socket->player();

	// We're dead and can't do that
	if( pChar->dead )
	{
		socket->sysMessage( tr( "You are dead and can't do that." ) );
		socket->bounceItem( pItem, BR_NO_REASON );
		return;
	}

	// Our target is dead
	if( ( pWearer != pChar ) && pWearer->dead )
	{
		socket->sysMessage( tr( "You can't equip dead players." ) );
		socket->bounceItem( pItem, BR_NO_REASON );
		return;
	}

	// Get our tile-information
	tile_st pTile = cTileCache::instance()->getTile( pItem->id() );

	// Is the item wearable ? ( layer == 0 | equip-flag not set )
	// Multis are not wearable are they :o)
	if( pTile.layer == 0 || !( pTile.flag3 & 0x40 ) || pItem->isMulti() )
	{
		socket->sysMessage( tr( "This item cannot be equipped." ) );
		socket->bounceItem( pItem, BR_NO_REASON );
		return;
	}

	// Required Strength
	if( pItem->st > pWearer->st )
	{
		if( pWearer == pChar )
			socket->sysMessage( tr( "You cannot wear that item, you seem not strong enough" ) );
		else
			socket->sysMessage( tr( "This person can't wear that item, it seems not strong enough" ) );

		socket->bounceItem( pItem, BR_NO_REASON );
		return;
	}

	// Required Dexterity
	if( pItem->dx > pWearer->effDex() )
	{
		if( pWearer == pChar )
			socket->sysMessage( tr( "You cannot wear that item, you seem not agile enough" ) );
		else
			socket->sysMessage( tr( "This person can't wear that item, it seems not agile enough" ) );

		socket->bounceItem( pItem, BR_NO_REASON );
		return;
	}
	
	// Required Intelligence
	if( pItem->in > pWearer->in )
	{
		if( pWearer == pChar )
			socket->sysMessage( tr( "You cannot wear that item, you seem not smart enough" ) );
		else
			socket->sysMessage( tr( "This person can't wear that item, it seems not smart enough" ) );

		socket->bounceItem( pItem, BR_NO_REASON );
		return;
	}

	// Males can't wear female armor
	if( ( pChar->id() == 0x0190 ) && ( pItem->id() >= 0x1C00 ) && ( pItem->id() <= 0x1C0D ) )
	{
		socket->sysMessage( tr( "You cannot wear female armor." ) );
		socket->bounceItem( pItem, BR_NO_REASON );
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
				socket->sysMessage( tr( "You already have an item on that layer." ) );
				socket->bounceItem( pItem, BR_NO_REASON );
				return;
			}
		}
	}

	// At this point we're certain that we can wear the item
	pItem->setContSerial( packet->wearer() );
	pItem->setLayer( pTile.layer ); // Don't trust the user input on this one

	if( pTile.layer == 0x19 )
		pWearer->setOnHorse( true );

	// Apply the bonuses
	pWearer->giveItemBonus( pItem );

	// Show debug information if requested
	if( showlayer )
		clConsole.send( QString( "Item (%1) equipped on layer %2" ).arg( pItem->name() ).arg( pItem->layer() ) );

	// I don't think we need to remove the item
	// as it's only visible to the current char
	// And he looses contact anyway

	// Build our packets
	cUOTxCharEquipment wearItem;
	wearItem.fromItem( pItem );

	cUOTxSoundEffect soundEffect;
	soundEffect.setSound( 0x57 );
	soundEffect.setCoord( pWearer->pos );

	// Send to all sockets in range
	// ONLY the new equipped item and the sound-effect
	for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		if( mSock->player() && ( mSock->player()->pos.distance( pWearer->pos ) <= mSock->player()->VisRange ) );
		{
			mSock->send( &wearItem );
			mSock->send( &soundEffect );
		}
	}

	// Lord Binaries Glow stuff
	if( pItem->glow != INVALID_SERIAL )
	{
		pChar->removeHalo( pItem );
		pWearer->addHalo( pItem );
		pWearer->glowHalo( pItem );
	}
}

void cDragItems::dropItem( cUOSocket *socket, cUORxDropItem *packet )
{
	P_CHAR pChar = socket->player();

	if( !pChar )
		return;

	// Get the data
	SERIAL contId = packet->cont();

	Coord_cl dropPos = pChar->pos; // plane
	dropPos.x = packet->x();
	dropPos.y = packet->y();
	dropPos.z = packet->z();

	// Get possible containers
	P_ITEM pItem = FindItemBySerial( packet->serial() );
	
	if( !pItem )
		return;

	P_ITEM iCont = FindItemBySerial( packet->cont() );
	P_CHAR cCont = FindCharBySerial( packet->cont() );

	// >> SEE LORD BINARIES DROPFIXBUGFIXBUG <<

	// A completely invalid Drop packet
	if( !iCont && !cCont && ( dropPos.x == 0xFFFF ) && ( dropPos.y == 0xFFFF ) && ( (unsigned char)dropPos.z == 0xFF ) )
	{
		socket->bounceItem( pItem, BR_NO_REASON );
		return;
	}

	// Item dropped on Ground
	if( !iCont && !cCont )
		dropOnGround( socket, pItem, dropPos );

	// Item dropped on another item
	else if( iCont )
		dropOnItem( socket, pItem, iCont, dropPos );

	// Item dropped on char
	else if( cCont )
		dropOnChar( socket, pItem, cCont );

	// Handle the sound-effect
	if( pItem->id() == 0xEED )
		goldsfx( socket, pItem->amount(), true );
}

void cDragItems::dropOnChar( cUOSocket *socket, P_ITEM pItem, P_CHAR pOtherChar )
{
	// Three possibilities:
	// If we're dropping it on ourself: packintobackpack
	// If we're dropping it on some other player: trade-window
	// If we're dropping it on some NPC: checkBehaviours
	// If not handeled: Equip the item if the NPC is owned by us

	// To prevent bad effects remove it from the clients view first
	cUOTxRemoveObject rObject;
	rObject.setSerial( pItem->serial );
	socket->send( &rObject );


	P_CHAR pChar = socket->player();

	// Dropped on ourself
	if( pChar == pOtherChar )
	{
		pItem->toBackpack( pChar );
		return;
	}

	// Are we in range of our target
	if( !inrange1p( pChar, pOtherChar ) )
	{
		socket->bounceItem( pItem, BR_OUT_OF_REACH );
		return;
	}

	// Can wee see our target
	if( lineOfSight( pChar->pos, pOtherChar->pos, TREES_BUSHES|WALLS_CHIMNEYS|DOORS|ROOFING_SLANTED|FLOORS_FLAT_ROOFING|LAVA_WATER ) )
	{
		socket->bounceItem( pItem, BR_OUT_OF_SIGHT );
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

		//if( !tradeWindow )
		//	tradeWindow = Trade->tradestart( client->socket(), pOtherChar );
		socket->bounceItem( pItem, BR_NO_REASON );
		socket->sysMessage( "Trading is disabled" );
		return;

		pItem->setContSerial( tradeWindow->serial);
		pItem->pos.x = rand() % 60;
		pItem->pos.y = rand() % 60;
		pItem->pos.z = 9;
		pItem->setLayer( 0 );
		pItem->removeFromView( false );
		pItem->update();
		return;
	}

	// For our hirelings we have a special function
	/*if( pChar->Owns( pOtherChar ) )
	{
		dropOnPet( client, pItem, pOtherChar );
		return;
	}*/

	// Dropping based on AI Type
	/*switch( pOtherChar->npcaitype() )
	{
	case 4:
		dropOnGuard( client, pItem, pOtherChar );
		break;
	case 5:
		dropOnBeggar( client, pItem, pOtherChar );
		break;
	case 8:
		dropOnBanker( client, pItem, pOtherChar );
		break;
	case 19:
		dropOnBroker( client, pItem, pOtherChar );
		break;
	};

	// Try to train - works for any NPC
	if( pOtherChar->cantrain() )
		if( pChar->trainer() == pOtherChar->serial )
			dropOnTrainer( client, pItem, pOtherChar );
		else
			pOtherChar->talk( "You need to tell me what you want to learn first" );*/

	socket->sysMessage( "Dropping on other characters is disabled" );
	socket->bounceItem( pItem, BR_NO_REASON );
	return;
}

void cDragItems::dropOnGround( cUOSocket *socket, P_ITEM pItem, const Coord_cl &pos )
{
	P_CHAR pChar = socket->player();

	// Check if the destination is in line of sight
	if( !lineOfSight( pChar->pos, pos, WALLS_CHIMNEYS|DOORS|LAVA_WATER ) )
	{
		socket->bounceItem( pItem, BR_OUT_OF_SIGHT );
		return;
	}

	if( !pChar->canPickUp( pItem ) )
	{
		socket->bounceItem( pItem, BR_CANNOT_PICK_THAT_UP );
		return;
	}

	pItem->setContSerial( INVALID_SERIAL );
	pItem->moveTo( pos );	
	pItem->setLayer( 0 );
	pItem->update();

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

void cDragItems::dropOnItem( cUOSocket *socket, P_ITEM pItem, P_ITEM pCont, const Coord_cl &dropPos )
{
	P_CHAR pChar = socket->player();
	
	if( pItem->isMulti() )
	{
		socket->sysMessage( tr( "You cannot put houses in containers" ) );
		Items->DeleItem( pItem );
		cUOTxBounceItem bounce;
		bounce.setReason( BR_NO_REASON );
		socket->send( &bounce );
		return;
	}
	
	// If the target belongs to another character 
	// It needs to be our vendor or else it's denied
	P_CHAR packOwner = GetPackOwner( pCont );

	if( ( packOwner ) && ( packOwner != pChar ) )
	{
		// For each item someone puts into there 
		// He needs to do a snoop-check
		if( pChar->canSnoop() )
		{
			if( !Skills->CheckSkill( pChar, SNOOPING, 0, 1000 ) )
			{

				socket->sysMessage( tr( "You fail to put that into %1's pack" ).arg( packOwner->name.c_str() ) );
				socket->bounceItem( pItem, BR_NO_REASON );
				return;
			}
		}

		if( !packOwner->isNpc() || ( packOwner->npcaitype() != 17 ) || !pChar->Owns( packOwner ) )
		{
			socket->sysMessage( "You cannot put that into the belongings of another player" );
			socket->bounceItem( pItem, BR_NO_REASON );
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
	
	if( !pChar->canPickUp( pItem ) )
	{
		socket->bounceItem( pItem, BR_CANNOT_PICK_THAT_UP );
		return;
	}

	// Trash can
	if( pCont->type()==87 )
	{
		Items->DeleItem( pItem );
		socket->sysMessage( tr( "As you let go of the item it disappears." ) );
		return;
	}

	// Spell Book
	if( pCont->type() == 9 )
	{
		SI16 spellId = Magic->calcSpellId( pItem->id() );

		if( spellId < 0 )
		{
			socket->sysMessage( tr( "You can only put scrolls into a spellbook" ) );
			socket->bounceItem( pItem, BR_NO_REASON );
			return;
		}		

		if( Magic->hasSpell( pCont, spellId )  )
		{
			socket->sysMessage( tr( "That spellbook already contains this spell" ) );
			socket->bounceItem( pItem, BR_NO_REASON );
			return;
		}
	}

	// We drop something on the belongings of one of our playervendors
	if( ( packOwner != NULL ) && ( packOwner->npcaitype() == 17 ) && pChar->Owns( packOwner ) )
	{
		socket->sysMessage( tr( "You drop something into your playervendor (unimplemented)" ) );
		socket->bounceItem( pItem, BR_NO_REASON );
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
	*/

	// We may also drop into *any* locked chest
	// So we can have post-boxes ;o)
	// Spellbooks are containers for us as well
	if( pCont->type() == 9 || pCont->type() == 1 || pCont->type() == 8 || pCont->type() == 63 || pCont->type() == 65 || pCont->type() == 66 )
	{
		// If we're dropping it onto the closed container
		if( dropPos.distance( pCont->pos ) == 0 )
			pCont->AddItem( pItem );
		else
			pCont->AddItem( pItem, dropPos.x, dropPos.y );

		// Dropped on another Container/in another Container
		pChar->soundEffect( 0x57 );
		return;
	}
	// Item matching needs to be extended !!! at least Color! (for certain types)
	else if ( pCont->isPileable() && pItem->isPileable() && ( pCont->id() == pItem->id() ) )
	{
		if( pCont->amount() + pItem->amount() <= 65535 )
		{
			pCont->setAmount( pCont->amount() + pItem->amount() );
			
			Items->DeleItem( pItem );

			pCont->update(); // Need to update the amount
			return;
		}
		// We have to *keep* our current item
		else
		{
			pCont->setAmount( 65535 ); // Max out the amount
			pCont->update();

			// The delta between 65535 and pCont->amount() sub our Amount is the
			// new amount
			pItem->setAmount( pItem->amount() - ( 65535 - pCont->amount() ) );
		}
	}

	// We dropped the item NOT on a container
	// And were *un*able to stack it (!)
	// >> Set it to the location of the item we dropped it on and stack it up by 2
	pItem->moveTo( pCont->pos );
	pItem->pos.z += 2; // Increase z by 2
	pItem->setLayer( 0 );
	pItem->setContSerial( pCont->contserial );
	pItem->update();

	// This needs to be checked
	// It annoyingly shows the spellbook
	// whenever you add a scroll
	// << could it be that addItemToContainer is enough?? >>
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
		pPet->resend( false );
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
	//goldsfx( client->socket(), pVictim->questBountyReward() );
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
			pBeggar->resend( false );
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

void cDragItems::dropOnBroker( P_CLIENT client, P_ITEM pItem, P_CHAR pBroker )
{
	// For House and Boat deeds we should pay back 75% of the value
	if( pItem->id() == 0x14EF )
	{
		if( !pItem->value )
		{
			pBroker->talk( "I can only accept deeds with value!" );
			bounceItem( client, pItem );
			return;
		}

		Q_UINT32 nValue = static_cast< Q_UINT32 >( 0.75 * pItem->value );
		client->player()->giveGold( nValue, true );
		Items->DeleItem( pItem );
		pBroker->talk( QString( "Here you have your %1 gold, %2" ).arg( nValue ).arg( client->player()->name.c_str() ) );
		return;
	}

	bounceItem( client, pItem );
}

void cDragItems::dropOnBanker( P_CLIENT client, P_ITEM pItem, P_CHAR pBanker )
{
	P_CHAR pChar = client->player();

	// No cheque ? >> Put into bank
	if( ( pItem->id() != 0x14F0 ) && ( pItem->type() != 1000 ) )
	{
		P_ITEM bankBox = pChar->getBankBox();

		if( bankBox )
			bankBox->AddItem( pItem );
		else
			bounceItem( client, pItem );

		pBanker->talk( QString( "The %1 is now in thy bank box" ).arg( pItem->getName() ) );
		return;
	}

	// No Value ?!
	if( !pItem->value )
	{
		pBanker->talk( "This cheque does not have any value!" );
		bounceItem( client, pItem );
		return;
	}

	pChar->giveGold( pItem->value, true );
	pBanker->talk( QString( "%1 I have cashed thy cheque and deposited %2 gold." ).arg( pChar->name.c_str() ).arg( pItem->amount() ) );

	pItem->ReduceAmount();
	//if( pItem->ReduceAmount() > 0 )
	//	socket->bounce( pItem, BR_NO_REASON );
}

void cDragItems::dropOnTrainer( P_CLIENT client, P_ITEM pItem, P_CHAR pTrainer )
{
	P_CHAR pChar = client->player();

	if( pItem->id() != 0xEED )
	{
		pTrainer->talk( "You need to give me gold if you want me to train you!" );
		bounceItem( client, pItem );
		return;
	}

	pTrainer->talk( "I thank thee for thy payment. That should give thee a good start on thy way. Farewell!" );

	Q_UINT8 skill = pTrainer->trainingplayerin();
	Q_INT32 skillSum = pChar->getSkillSum();
	Q_INT32 skillDelta = pTrainer->getTeachingDelta( pChar, skill, skillSum );

	//goldsfx( client->socket(), pItem->amount() );

	if( pItem->amount() > skillDelta )
	{
		pItem->ReduceAmount( skillDelta );
		bounceItem( client, pItem );
	}
	else
	{
		skillDelta = pItem->amount();
		Items->DeleItem( pItem );
	}
	
	pChar->setBaseSkill( skill, pChar->baseSkill( skill ) + skillDelta );
	Skills->updateSkillLevel( pChar, skill );
	updateskill( client->socket(), skill );

	// we will not reset the trainer id here because he may want to give him more money
}
