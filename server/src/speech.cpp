//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2004 by holders identified in authors.txt
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

// Platform Includes
#include "platform.h"

// Wolfpack Includes
#include "accounts.h"
#include "speech.h"

#include "sectors.h"
#include "uotime.h"
#include "srvparams.h"
#include "network.h"
#include "territories.h"
#include "network/uosocket.h"
#include "pagesystem.h"
#include "skills.h"
#include "house.h"
#include "boats.h"
#include "pythonscript.h"
#include "itemid.h"
#include "player.h"
#include "npc.h"
#include "chars.h"
#include "ai/ai.h"
#include "world.h"
#include "inlines.h"
#include "console.h"

// Library Includes
#include <qdatetime.h>
#include <qfile.h>
#include <vector>

using namespace std;

#undef  DBGFILE
#define DBGFILE "speech.cpp"

bool InputSpeech( cUOSocket *socket, P_PLAYER pChar, const QString &speech )
{
	if( pChar->inputMode() == cPlayer::enNone )
		return false;

	P_ITEM pItem = FindItemBySerial( pChar->inputItem() );

	if( !pItem )
		return false;

	bool ok;
	INT32 num = speech.toInt( &ok ); // Generally try to convert it
	QString notification;

	switch (pChar->inputMode())
	{
	// Pricing an item - PlayerVendors
	case cPlayer::enPricing:
		if (ok)
		{
//			pItem->setPrice( num );
//			socket->sysMessage( tr( "This item's price has been set to %1." ).arg( num ) );
			socket->sysMessage( "Ops, sorry not implemented" );
		}
		else
			socket->sysMessage( tr( "You have to enter a numeric price" ) );

		pChar->setInputMode(cPlayer::enDescription);
		socket->sysMessage( tr( "Enter a description for this item." ) );
		break;

	// Describing an item
	case cPlayer::enDescription:
		socket->sysMessage( "Description is not used anywhere :( not implemented right now" );
		socket->sysMessage( tr( "This item is now described as %1." ).arg( speech ) );
		pChar->setInputMode(cPlayer::enNone);
		pChar->setInputItem(INVALID_SERIAL);
		break;

	// Renaming ourself
	case cPlayer::enNameDeed: 
		pChar->setName( speech );
		socket->sysMessage( tr( "Your new name is: %1" ).arg( speech ) );
		pChar->setInputMode(cPlayer::enNone);
		pChar->setInputItem(INVALID_SERIAL);
		break;

	// Renaming a house sign
	case cPlayer::enHouseSign:
		pItem->setName( speech ); 
		socket->sysMessage( tr( "Your house has been renamed to: %1" ).arg( speech ) );
		pChar->setInputMode(cPlayer::enNone);
		pChar->setInputItem(INVALID_SERIAL);
		break;

	// Paging a GM
	case cPlayer::enPageGM:
		{
			cPage* pPage = new cPage( pChar->serial(), PT_GM, speech, pChar->pos() );
			cPagesManager::getInstance()->push_back( pPage );
			notification = tr( "GM Page from %1: %2" ).arg( pChar->name() ).arg( speech );
			
			for ( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next())
				if( mSock->player() && mSock->player()->isGM() )
					mSock->sysMessage( notification );
				
				if( cNetwork::instance()->count() > 0 )
					socket->sysMessage( tr( "Available Game Masters have been notified of your request." ) );
				else
					socket->sysMessage( tr( "There was no Game Master available, page queued." ) );
				
				pChar->setInputMode(cPlayer::enNone);
		}
		break;
		
	// Paging a Counselor
	case cPlayer::enPageCouns:
		{
			cPage* pPage = new cPage( pChar->serial(), PT_COUNSELOR, speech, pChar->pos() );
			cPagesManager::getInstance()->push_back( pPage );
			notification = tr( "Counselor Page from %1: %2" ).arg( pChar->name() ).arg( speech );
			
			for ( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next())
				if( mSock->player() && (socket->player()->isCounselor() || socket->player()->isGM()) )
					mSock->sysMessage( notification );
				
				if( cNetwork::instance()->count() > 0 )
					socket->sysMessage( tr( "Available Counselors have been notified of your request." ) );
				else
					socket->sysMessage( tr( "There was no Counselor available, page queued." ) );
				
				pChar->setInputMode(cPlayer::enNone);
		}
		break;

	default:
		break;	// do nothing
	}
	
	return true;
}

bool StableSpeech( cUOSocket *socket, P_CHAR pPlayer, P_CHAR pMaster, const QString &speech )
{
/* redo with python..
	// is it a stablemaster ?
	if( pMaster->npc_type() != 1 )
		return false;

	if( !speech.contains( "STABLE" ) )
		return false;
      
	/////////////////////////////////////////////////////////////////////
	//// so far we have a stablemaster! lets see if the caller has a pet
	//// if so check if the pets name is in the commandstring
	//// if not return
    ///////////////////////////////////////////////////////////////////
	
	bool found = false;
	P_CHAR p_pet = NULL;
	RegionIterator4Chars ri( pPlayer->pos() );
	for (ri.Begin(); !ri.atEnd(); ri++)
	{
		p_pet = ri.GetData();
		if (pPlayer->dist(p_pet) <= 8)
		{
			if (p_pet->owner() == pPlayer && p_pet->stablemaster_serial()==INVALID_SERIAL) //owner of the pet ? and not already stabled ?
			{
				QString pntmp = p_pet->name();
				if (speech.contains(pntmp, false))
				{
					found=true;
					break;
				}
			}
		}
	}

	if (!found) 
	{ 
		pMaster->talk( tr( "Which pet?" ) );
		return 1;
	} 

	/////////////////////////////////////////////////////////////
	/// now we have a pet and stablemaster -> time to STABLE :-)
    ////////////////////////////////////////////////////////////
	
    // set stablesp && pets stablemaster serial
	// remove it from screen!
	p_pet->removeFromView(); // Remove it from view of all sockets
	p_pet->setWar(false);
	p_pet->setAttacker(INVALID_SERIAL);

	pPlayer->setWar(false);
	pPlayer->setTarg( INVALID_SERIAL );

	MapObjects::instance()->remove( p_pet );
	p_pet->setStablemaster_serial( pMaster->serial() );

	// set timer for fee calculation
    p_pet->setTime_unused(0);
	p_pet->setTimeused_last( getNormalizedTime() );

	stablesp.insert( pMaster->serial(), p_pet->serial() );

	pMaster->talk( tr( "Your pet is now stabled, say retrieve or claim %1 to claim your pet" ).arg( p_pet->name() ) );
	*/
	return false;
}

bool UnStableSpeech( cUOSocket *socket, P_CHAR pPlayer, P_CHAR pMaster, const QString &comm )
{
/* redo with python
	// is it a stablemaster ?
	if( pMaster->npc_type() != 1 )	
		return false;

	if( !comm.contains( "CLAIM" ) && !comm.contains( "RETRIEVE" ) )
	
		return false;

	/////////////////////////////////////////////////////////////////////
	//// so far we have a stablemaster! lets see if the stablemaster has a pet
	//// stabled the petowner owns
	//// if not return
    ///////////////////////////////////////////////////////////////////
	P_CHAR pPet = NULL;
	vector<SERIAL> pets = stablesp.getData(pMaster->serial());

	for( UINT32 i = 0; i < pets.size(); ++i )
	{
		pPet = FindCharBySerial( pets[i] );
		
		if( pPet )
		{
			if( pPet->owner() != pPlayer || !comm.contains( pPet->name(), false ) )
				pPet = NULL;
			else
				break;
		}
	}

	if( !pPet ) 
	{ 
		pMaster->talk( tr( "Sorry, I can't return that pet." ) );
		return true;
	} 

	/////////////////////////////////////////////////////////////
	/// now we have the claimed pet and stablemaster -> time to UNSTABLE :-)
    ////////////////////////////////////////////////////////////

	/// calc fee
	// (fee per 10 minutes) * number of 10 minute blocks
	float f_fee = ( ( pPet->time_unused() ) / 600.0f ) * SrvParams->stablingFee();
	int fee = ( (int) f_fee ) + 5; // 5 basefee

	pMaster->talk( tr( "That's %1 gold pieces" ).arg( fee ) );

	/////////// check if customer can pay ! ///////////////    
	if( pPlayer->CountGold() < fee )
	{
		pMaster->talk( tr( "You can't afford the fee to claim your pet. Come back when you have enough gold." ) );
		return true;
	}
	
	delequan( pPlayer, 0x0EED, fee, NULL );
		
	// remove from hash table
	stablesp.remove( pMaster->serial(), pPet->serial() );
	pPet->setStablemaster_serial( INVALID_SERIAL ); // actual unstabling
	pPet->setTimeused_last(getNormalizedTime());
	pPet->setTime_unused(0);

	MapObjects::instance()->remove( pPet );
	MapObjects::instance()->add( pPet );
	pPet->resend( false ); // Resend
		
	pMaster->talk( tr( "Here's your pet. Treat it well." ) );
*/
	return false;
}

bool ShieldSpeech( cUOSocket *socket, P_CHAR pPlayer, P_CHAR pGuard, const QString& comm )
{
/*	// lets be close to talk :)
	if( pPlayer->dist( pGuard ) > 3 )	
		return false;

	cGuildStone *pStone = pPlayer->getGuildstone();

	// if they say chaos shield
	if( ( pGuard->npcaitype() == 6 ) && ( comm.contains( "CHAOS SHIELD" ) ) )
	{	
		// The user needs to be in a chaos guild in order to get a shield
		if( !pStone || ( pStone->guildType != cGuildStone::chaos ) )
		{
			pGuard->talk( tr( "You must be in a chaos guild to get a shield!" ) );
			return true;
		}

		// We will only give out once shield per player
		if( pPlayer->CountItems( 0x1BC3 ) > 0 )
		{
			pGuard->talk( "You already possess a shield!" );
			return true;
		}

		// lets give them a new chaos shield.
		P_ITEM pShield = cItem::createFromScript( "28" );
		pPlayer->getBackpack()->addItem( pShield );
		
		socket->sysMessage( tr( "You put the chaos shield into your backpack" ) );
		pGuard->talk( tr( "Hi fellow guild member, here is your new chaos shield." ) );
		return true;
	}
	// He wants an order shield
	else if( ( pGuard->npcaitype() == 7 ) && ( comm.contains( "ORDER SHIELD" ) ) )
	{	
		// The user needs to be in a order guild in order to get a shield
		if( !pStone || ( pStone->guildType != cGuildStone::order ) )
		{
			pGuard->talk( tr( "You must be in an order guild to get a shield!" ) );
			return true;
		}

		// We will only give out once shield per player
		// BAD: Player may have stored his shield in his bankbox
		if( pPlayer->CountItems( 0x1BC4 ) > 0 )
		{
			pGuard->talk( "You already possess a shield!" );
			return true;
		}

		// lets give them a new order shield.
		P_ITEM pShield = cItem::createFromScript( "29" );
		pPlayer->getBackpack()->addItem( pShield );
		
		socket->sysMessage( tr( "You put the order shield into your backpack" ) );
		pGuard->talk( tr( "Hi fellow guild member, here is your new order shield." ) );
		return true;
	}
*/
	return false;
}

// All this Stuff should be scripted
bool QuestionSpeech( cUOSocket *socket, P_PLAYER pPlayer, P_NPC pChar, const QString& comm )
{
	if( !pChar->isHuman() || pPlayer->dist( pChar ) > 3 )
		return false;
	
	// Tell the questioner our name
	if( comm.contains( "NAME" ) )
	{
		pChar->talk( tr( "Hello, my name is %1." ).arg( pChar->name() ) );
		return true;
	}
	
    // say time and the npChar gives the time.
	if( comm.contains( "TIME" ) )
	{
		pChar->talk( tr( "It is now %1 hours and %2 minutes.").arg(UoTime::instance()->hour()).arg(UoTime::instance()->minute()));
		return true;
	}	

	if( comm.contains( "LOCATION" ) )
	{
		cTerritory* Region = pPlayer->region();
		
		if( Region )
			pChar->talk( tr( "You are in %1" ).arg( Region->name() ) );
		else 
			pChar->talk( tr( "You are in the wilderness" ) );
				
		return true;
	}

	// We couldn't handle the speech
	return false;
}

bool BankerSpeech( cUOSocket *socket, P_PLAYER pPlayer, P_NPC pBanker, const QString& comm )
{
	// Needs to be a banker
/*	if( pBanker->npcaitype() != 8 )
		return false;*/

	if ( !pBanker->ai() || pBanker->ai()->name() != "Banker")
		return false;

	if( pPlayer->dist(pBanker) > 6 )
		return false;

	if( comm.contains( "BANK" ) )
	{
		pBanker->turnTo( pPlayer );		
		socket->sendContainer( pPlayer->getBankbox() );
		return true;
	}

	if( ( comm.contains( "BALANCE" ) ) | ( comm.contains( "WITHDRAW" ) ) || ( comm.contains( "CHECK" ) ) )
	{
	    //BankerAI->DoAI( s, pBanker, comm );
	    return true;
	}

    return false;
}

bool TrainerSpeech( cUOSocket *socket, P_CHAR pPlayer, P_NPC pTrainer, const QString& comm ) 
{
/*	if( pPlayer->dist( pTrainer ) > 3 || !pTrainer->isHuman() )
		return false;

	if( !comm.contains( "TRAIN" ) && !comm.contains( "TEACH" ) && !comm.contains( "LEARN" ) )
		return false;

	if( !pTrainer->cantrain() )
	{
		pTrainer->talk( tr( "I am sorry, but I have nothing to teach thee" ) );
		return true;
	}

	INT32 i, skill = -1;
	
	// this is to prevent errors when a player says "train <skill>" then don't pay the npc
	pPlayer->setTrainer( INVALID_SERIAL ); 

	for( i = 0; i < ALLSKILLS; ++i )
		if( comm.contains( Skills->getSkillName( i ), false ) )
		{
			skill = i;
			break;
		}

	if( skill != -1 )
	{
		if( pTrainer->skillValue( skill ) <= 10 || !pTrainer->cantrain() )
		{
			pTrainer->talk( "I am sorry but I cannot train thee in that skill." );
			return true;
		}

		QString skillName = Skills->getSkillName( skill );
		skillName = skillName.lower();

		QString message = tr( "Thou wishest to learn of %1" ).arg( skillName );

		// This should be configureable
		if( pPlayer->skillValue(skill) >= 250 )
			message.append( tr( " I can teach thee no more than thou already knowest!" ) );
		else
		{
			UINT32 sum = pPlayer->getSkillSum();

			// The user knows too much
			if( sum >= SrvParams->skillcap() * 10 )
				message.append( tr( " I can teach thee no more. Thou already knowest too much!" ) );

			else
			{
				int delta = pTrainer->getTeachingDelta( pPlayer, skill, sum );
				int perc = ( pPlayer->skillValue( skill ) + delta ) / 10;
				
				message.append( tr( " Very well I, can train thee up to the level of %i percent for %i gold. Pay for less and I shall teach thee less." ).arg( perc ).arg( delta ) );

				pPlayer->setTrainer( pTrainer->serial() );
				pTrainer->setTrainingplayerin( skill );
			}
		}

		pTrainer->talk( message );
		return true;
	}

	// Didn't ask to be trained in a specific skill
	pTrainer->setTrainingplayerin( 0xFF );
	QStringList skillList;

	for( i = 0; i < ALLSKILLS; ++i )
	{
		if( pTrainer->skillValue( i ) >= 10 && pPlayer->skillValue( i ) < 250 )
		{
			QString skillName = Skills->getSkillName( i );
			skillList.push_back( skillName );
		}
	}

	// skills and a trainer ?
	if( skillList.count() > 0 ) 
		pTrainer->talk( tr( "I can teach thee the following skills: %1." ).arg( skillList.join( ", " ) ) );
	else
		pTrainer->talk( tr( "I am sorry, but I have nothing to teach thee" ) );
*/
	return false;
}

//PlayerVendors
void PlVGetgold( cUOSocket *socket, P_CHAR pPlayer, P_CHAR pVendor )
{
/*
	if( pVendor->owner() != pPlayer )
	{
		pVendor->talk( tr( "I don't work for you!" ) );
		return;
	}

	if( pVendor->holdg() <= 0 )
	{
		pVendor->talk( tr( "I have no gold waiting for you." ) );
		pVendor->setHoldg( 0 );
		return;
	}

	// He keeps 10% of all earnings
	UINT32 goldKeeping = pVendor->holdg();
	UINT32 goldSalary = (UINT32)( 0.1 * goldKeeping );
	goldKeeping -= goldSalary;

	// Hand him the gold
	if( goldKeeping > 0 )
		pPlayer->giveGold( goldKeeping );

	pVendor->talk( tr( "Today's purchases total %1 gold. I am keeping %2 gold for my self. Here is the remaining %3 gold. Have a nice day." ).arg( goldKeeping + goldSalary ).arg( goldSalary ).arg( goldKeeping ) );
	pVendor->setHoldg( 0 );
*/
}

bool VendorChkName( P_CHAR pVendor, const QString& comm )
{
	if( ( comm.contains( "VENDOR" ) ) || ( comm.contains( "SHOPKEEPER" ) ) )
		return true;

	if( comm.contains( pVendor->name(), false ) )
		return true;
	else
		return false;
}

// Handles house commands from friends of the house.
// msg must already be capitalized
void HouseSpeech( cUOSocket *socket, P_CHAR pPlayer, const QString& msg )
{
	Q_UNUSED(socket);
	Q_UNUSED(msg);
	// Not inside a multi
	if( pPlayer->multis() == INVALID_SERIAL )
		return; 

	P_ITEM pMulti = FindItemBySerial( pPlayer->multis() );

	if( !pMulti )
	{
		Console::instance()->send( tr( "Player %1 [0x%2] has bad multi serial [0x%1]" ).arg( pPlayer->name() ).arg( pPlayer->serial(), 8, 16 ).arg( pPlayer->multis() ) );
		pPlayer->setMultis( INVALID_SERIAL );
		return;
	}

	if ( pMulti && IsHouse( pMulti->id() ) )
	{
		cHouse* pHouse = dynamic_cast< cHouse* >( pMulti );

		// Only the owner or a friend of the house can control it
		if( !( pPlayer->Owns( pHouse ) || pHouse->isFriend( pPlayer ) ) )
			return;
	}
	else
		return;

	// >> LEGACY
	/*if(msg.contains("I BAN THEE")) 
	{ // house ban
		addid1[s] = pMulti->serial()>>24;
		addid2[s] = pMulti->serial()>>16;
		addid3[s] = pMulti->serial()>>8;
		addid4[s] = pMulti->serial()%256;
		target(s, 0, 1, 0, 229, "Select person to ban from house.");
	}
	else if(msg.contains("REMOVE THYSELF")) 
	{ // kick out of house
		addid1[s] = pMulti->serial()>>24;
		addid2[s] = pMulti->serial()>>16;
		addid3[s] = pMulti->serial()>>8;
		addid4[s] = pMulti->serial()%256;
		target(s, 0, 1, 0, 228, "Select person to eject from house.");
	}
	else if (msg.contains("I WISH TO LOCK THIS DOWN")) 
	{ // lock down code AB/LB
         target(s, 0, 1, 0, 232, "Select item to lock down");
	}
	else if (msg.contains("I WISH TO RELEASE THIS")) 
	{ // lock down code AB/LB
          target(s, 0, 1, 0, 233, "Select item to release");
	}
	else if (msg.contains("I WISH TO SECURE THIS")) 
	{ // lock down code AB/LB
		target(s, 0, 1, 0, 234, "Select item to secure"); 
	}*/
}

/////////////////
// name:	response
// purpose:	tries to get a response from an npc standing around
// history:	heavily revamped/rewritten by Duke, Oct 2001
// remark:	The new logic tries to minimize the # of strstr() calls by *first* checking
//			what kind of npcs are standing around and then checking only those keywords
//			that they might be interested in.
//			This is especially usefull in crowded places.
bool cSpeech::response( cUOSocket *socket, P_PLAYER pPlayer, const QString& comm, QValueVector< UINT16 > &keywords )
{
	if (!pPlayer->socket() || pPlayer->isDead()) {
		return false;
	}

	QString speechUpr = comm.upper();

	RegionIterator4Chars ri( pPlayer->pos() );
	for( ri.Begin(); !ri.atEnd(); ri++ )
	{
		P_NPC pNpc = dynamic_cast<P_NPC>(ri.GetData());

		// We will only process NPCs here
		if( !pNpc )
			continue;

		// at least they should be on the screen
		if( pPlayer->dist( pNpc ) > 16 )
			continue;

		cPythonScript **events = pNpc->getEvents();

		if (events) {
			PyObject *pkeywords = PyList_New(keywords.size());

			// Set Items
			for( unsigned int i = 0; i < keywords.size(); ++i )
				PyList_SetItem( pkeywords, i, PyInt_FromLong( keywords[i] ) );

			PyObject *args = Py_BuildValue("(O&O&uO)", PyGetCharObject, pNpc, PyGetCharObject, pPlayer, comm.ucs2(), pkeywords);

			bool result = cPythonScript::callChainedEventHandler(EVENT_SPEECH, events, args);

			Py_DECREF( args );
			Py_DECREF( pkeywords );

			if( result )
				return true;
		}

		if( pNpc->ai() )
		{
			pNpc->ai()->onSpeechInput( pPlayer, speechUpr );
		}

		if( BankerSpeech( socket, pPlayer, pNpc, speechUpr ) )
			return true;

		if( StableSpeech( socket, pPlayer, pNpc, speechUpr ) )
			return true;
		
		if( UnStableSpeech( socket, pPlayer, pNpc, speechUpr ) )
			return true;
		
		if( ShieldSpeech( socket, pPlayer, pNpc, speechUpr ) )
			return true;
		
		if( QuestionSpeech( socket, pPlayer, pNpc, speechUpr ) )
			return true;
		
		if( TrainerSpeech( socket, pPlayer, pNpc, speechUpr ) )
			return true;	
	}
	
	return false;
}

void cSpeech::talking( P_PLAYER pChar, const QString &lang, const QString &speech, QValueVector< UINT16 > &keywords, UINT16 color, UINT16 font, UINT8 type ) // PC speech
{	
	// handle things like renaming or describing an item
	if( !pChar->socket() )
		return;

	cUOSocket *socket = pChar->socket();

	if (InputSpeech(socket, pChar, speech))	
		return;

	pChar->unhide();
		
	// Check for Bogus Color
	if (!isNormalColor(color))
		color = 0x2;

	if (type == 0 || type == 2)
		pChar->setSaycolor( color );

	if (pChar->onTalk( type, color, font, speech, lang))
		return;

	if ((type == 0x09) && (pChar->mayBroadcast()))
	{
		pChar->talk( speech, color, type );
		return;
	}

	pChar->talk(speech, color, type);
		
	QString speechUpr = speech.upper();
	if( response( socket, pChar, speech, keywords ) )
		return;  // Vendor responded already
	
	// 0x0007 -> Speech-id for "Guards"
	for( QValueVector< UINT16 >::const_iterator iter = keywords.begin(); iter != keywords.end(); ++iter )
	{
		UINT16 keyword = *iter;

		if( keyword == 0x07 )
			pChar->callGuards();
	}
	
	// well,i had a strange problem with duplicate speech input
	// its quite easy to understand....
	// the former loop searched for the tiller man and when it
	// was found, the speechInput method of that boat was called.
	// in this method the tiller had been removed from the mapregion
	// and appended to the end of the cell vector... hence, the
	// tiller was found twice...
	// therefore we produce a QPtrList of cBoat* pointers and 
	// then go through it for applying speech --- sereg
	RegionIterator4Items rj( pChar->pos() );
	QPtrList< cBoat >	pboats;
	for( rj.Begin(); !rj.atEnd(); rj++ )
	{
		P_ITEM pi = rj.GetData();

		if( !pi )
			continue;

		if( pi->type() == 117 && pi->getTag( "tiller" ).toInt() == 1 )
		{
			cBoat* pBoat = dynamic_cast< cBoat* >(FindItemBySerial( pi->getTag("boatserial").toInt() ));
			if( pBoat )
				pboats.append( pBoat );
		}
	}
	QPtrListIterator< cBoat >	pit( pboats );
	while( pit.current() )
	{
		pit.current()->speechInput( socket, speechUpr );
		++pit;
	}

	// this makes it so npcs do not respond to isDead people - HEALERS ??
	if( pChar->isDead() )
		return;
	
/*	P_CHAR pc = NULL; ???
	P_CHAR pNpc = NULL;
	RegionIterator4Chars ri( pChar->pos() );
	for( ri.Begin(); !ri.atEnd(); ri++ )
	{	
		pc = ri.GetData();
		if (!pc->isSameAs( pChar ) 
			&& pc->isNpc()
			&& pc->dist( pChar ) <= 2)
		{
			pNpc = pc;
			break;
		}
	}
	*/
}
