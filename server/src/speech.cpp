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

// Platform Includes
#include "platform.h"

// Wolfpack Includes
#include "accounts.h"
#include "speech.h"
#include "worldmain.h"
#include "trigger.h"
#include "guildstones.h"
#include "regions.h"
#include "srvparams.h"
#include "network.h"
#include "classes.h"
#include "territories.h"
#include "scriptc.h"
#include "network/uosocket.h"

// Library Includes
#include "qdatetime.h"

#undef  DBGFILE
#define DBGFILE "speech.cpp"

bool InputSpeech( cUOSocket *socket, cChar* pChar, const QString &speech )
{
	if( pChar->inputmode == cChar::enNone )
		return false;

	P_ITEM pItem = FindItemBySerial( pChar->inputitem );

	if( !pItem )
		return false;

	bool ok;
	INT32 num = speech.toInt( &ok ); // Generally try to convert it
	QString notification;

	switch (pChar->inputmode)
	{
	// Pricing an item - PlayerVendors
	case cChar::enPricing:
		if (ok)
		{
			pItem->value = num;
			socket->sysMessage( tr( "This item's price has been set to %1." ).arg( num ) );
		}
		else
			socket->sysMessage( tr( "You have to enter a numeric price" ) );

		pChar->inputmode = cChar::enDescription;
		socket->sysMessage( tr( "Enter a description for this item." ) );
		break;

	// Describing an item
	case cChar::enDescription:
		pItem->desc = speech.latin1();
		socket->sysMessage( tr( "This item is now described as %1." ).arg( speech ) );
		pChar->inputmode = cChar::enNone;
		pChar->inputitem = INVALID_SERIAL;
		break;

	// Renaming a rune
	case cChar::enRenameRune:
		pItem->setName( tr( "Rune to: %1" ).arg( speech ) );
		socket->sysMessage( tr( "Rune renamed to: Rune to: %1" ).arg( speech ) );
		pChar->inputmode = cChar::enNone;
		pChar->inputitem = INVALID_SERIAL;
		break;

	// Renaming ourself
	case cChar::enNameDeed: 
		pChar->name = speech.latin1();
		socket->sysMessage( tr( "Your new name is: %1" ).arg( speech ) );
		pChar->inputmode = cChar::enNone;
		pChar->inputitem = INVALID_SERIAL;
		break;

	// Renaming a house sign
	case cChar::enHouseSign:
		pItem->setName( speech ); 
		socket->sysMessage( tr( "Your house has been renamed to: %1" ).arg( speech ) );
		pChar->inputmode = cChar::enNone;
		pChar->inputitem=INVALID_SERIAL;
		break;

	// Paging a GM
	case cChar::enPageGM:
		{
			gmpages[ pChar->playercallnum() ].reason = speech;
			notification = tr( "GM Page from %1: %2" ).arg( pChar->name.c_str() ).arg( speech );
			
			for ( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next())
				if( mSock->player() && mSock->player()->isGM() )
					mSock->sysMessage( notification );
				
				if( cNetwork::instance()->count() > 0 )
					socket->sysMessage( tr( "Available Game Masters have been notified of your request." ) );
				else
					socket->sysMessage( tr( "There was no Game Master available, page queued." ) );
				
				pChar->inputmode = cChar::enNone;
		}
		break;
		
	// Paging a Counselor
	case cChar::enPageCouns:
		{
			counspages[ pChar->playercallnum() ].reason = speech;
			notification = tr( "Counselor Page from %1: %2" ).arg( pChar->name.c_str() ).arg( speech );
			
			for ( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next())
				if( mSock->player() && socket->player()->isCounselor() )
					mSock->sysMessage( notification );
				
				if( cNetwork::instance()->count() > 0 )
					socket->sysMessage( tr( "Available Counselors have been notified of your request." ) );
				else
					socket->sysMessage( tr( "There was no Counselor available, page queued." ) );
				
				pChar->inputmode = cChar::enNone;
		}
		break;

	default:
		break;	// do nothing
	}
	
	return true;
}

bool StableSpeech( cUOSocket *socket, P_CHAR pPlayer, P_CHAR pMaster, const QString &speech )
{
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
	cRegion::RegionIterator4Chars ri( pPlayer->pos );
	for (ri.Begin(); !ri.atEnd(); ri++)
	{
		p_pet = ri.GetData();
		if (pPlayer->dist(p_pet) <= 8)
		{
			if (pPlayer->Owns(p_pet) && p_pet->stablemaster_serial()==INVALID_SERIAL) //owner of the pet ? and not already stabled ?
			{
				QString pntmp = p_pet->name.c_str();
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
	p_pet->war = false;
	p_pet->attacker = INVALID_SERIAL;

	pPlayer->war = false;
	pPlayer->targ = INVALID_SERIAL;

	mapRegions->Remove( p_pet );
	p_pet->setStablemaster_serial( pMaster->serial );

	// set timer for fee calculation
    p_pet->setTime_unused(0);
	p_pet->setTimeused_last( getNormalizedTime() );

	stablesp.insert( pMaster->serial, p_pet->serial );

	pMaster->talk( tr( "Your pet is now stabled, say retrieve or claim %1 to claim your pet" ).arg( p_pet->name.c_str() ) );
	return true;
}

bool UnStableSpeech( cUOSocket *socket, P_CHAR pPlayer, P_CHAR pMaster, const QString &comm )
{
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
	vector<SERIAL> pets = stablesp.getData(pMaster->serial);

	for( UINT32 i = 0; i < pets.size(); ++i )
	{
		pPet = FindCharBySerial( pets[i] );
		
		if( pPet )
		{
			if( !pPlayer->Owns( pPet ) || !comm.contains( pPet->name.c_str(), false ) )
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
	stablesp.remove( pMaster->serial, pPet->serial );
	pPet->setStablemaster_serial( INVALID_SERIAL ); // actual unstabling
	pPet->setTimeused_last(getNormalizedTime());
	pPet->setTime_unused(0);

	mapRegions->Remove( pPet );
	mapRegions->Add( pPet );
	pPet->resend( false ); // Resend
		
	pMaster->talk( tr( "Here's your pet. Treat it well." ) );
	return true;
}

bool ShieldSpeech( cUOSocket *socket, P_CHAR pPlayer, P_CHAR pGuard, const QString& comm )
{
	// lets be close to talk :)
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
		P_ITEM pShield = Items->createScriptItem( "28" );
		pPlayer->getBackpack()->AddItem( pShield );
		
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
		P_ITEM pShield = Items->createScriptItem( "29" );
		pPlayer->getBackpack()->AddItem( pShield );
		
		socket->sysMessage( tr( "You put the order shield into your backpack" ) );
		pGuard->talk( tr( "Hi fellow guild member, here is your new order shield." ) );
		return true;
	}

	return false;
}

// All this Stuff should be scripted
bool QuestionSpeech( cUOSocket *socket, P_CHAR pPlayer, P_CHAR pChar, const QString& comm )
{
	if( pChar->npcaitype()==2 || !pChar->isHuman() || pPlayer->dist( pChar ) > 3 )
		return false;
	
	// Tell the questioner our name
	if( comm.contains( "NAME" ) )
	{
		pChar->talk( tr( "Hello, my name is %1." ).arg( pChar->name.c_str() ) );
		return true;
	}
	
    // say time and the npChar gives the time.
	if( comm.contains( "TIME" ) )
	{
		pChar->talk( tr( "It is now %1" ).arg( uoTime.toString() ) );
		return true;
	}	

	if( comm.contains( "LOCATION" ) )
	{
		cTerritory* Region = cAllTerritories::getInstance()->region( pPlayer->region );
		
		if( Region )
			pChar->talk( tr( "You are in %1" ).arg( Region->name() ) );
		else 
			pChar->talk( tr( "You are in the wilderness" ) );
				
		return true;
	}

	// We couldn't handle the speech
	return false;
}

// arghl
bool TriggerSpeech( cUOSocket *socket, P_CHAR pPlayer, P_CHAR pChar, const QString& comm )
{
	if( pPlayer->dist( pChar ) < 5 )
		return false;

	if( !pPlayer->trigger() || pPlayer->trigword().isEmpty() || !comm.contains( pChar->trigword(), false )  )
		return false;

	if( pChar->disabled() > 0 && pChar->disabled() > uiCurrentTime )
	{
		pChar->talk( tr( "I'm a little busy now! Leave me be!" ) );
		return false;
	}

	Trig->triggernpc( calcSocketFromChar( pPlayer ), pChar, 1 ); // LEGACY
	return true;
}

bool EscortSpeech( cUOSocket *socket, P_CHAR pPlayer, P_CHAR pEscortee, const QString& comm )
{
	// not close enough / not an escortee
	if (pPlayer->dist(pEscortee) > 1 || pEscortee->questType() != ESCORTQUEST )
		return false;

	bool onRoute = false;
	
	if( comm.contains( "I WILL TAKE THEE" ) )
	{
		if ( pEscortee->ftarg == INVALID_SERIAL )
		{
			pEscortee->ftarg = pPlayer->serial;		// Set the NPC to follow the PC
			pEscortee->npcWander = 1;			// Set the NPC to wander freely
			pEscortee->setNpcAIType( 0 );           // Set AI to 0
			
			// Set the expire time if nobody excepts the quest
			pEscortee->summontimer = ( uiCurrentTime + ( MY_CLOCKS_PER_SEC * SrvParams->escortactiveexpire() ) );
			
			// Send out the rant about accepting the escort
			pEscortee->talk( tr( "Lead on! Payment shall be made when we arrive at %1." ).arg( pEscortee->questDestRegion() ) ),
			
			MsgBoardQuestEscortRemovePost( pEscortee );	// Remove post from message board
			return true;
		}
		else
		{
			// If the current NPC already has an ftarg then respond to query for quest
			onRoute = true;
		}
	}
		
	// DESTINATION
	// If this is a request to find out where a NPC wants to go and the PC is within range of the NPC and the NPC is waiting for an ESCORT
	if( ( comm.contains( "DESTINATION" ) ) || onRoute )
	{
		// Send out the rant about accepting the escort
		if ( pEscortee->ftarg == pPlayer->serial )
			pEscortee->talk( tr( "Lead on to %1. I shall pay thee when we arrive." ).arg( pEscortee->questDestRegion() ) );
		
		// If nobody has been accepted for the quest yet
		else if( pEscortee->ftarg == INVALID_SERIAL )  
			pEscortee->talk( tr( "I am seeking an escort to %1. Wilt thou take me there?" ).arg( pEscortee->questDestRegion() ) );

		// The must be enroute
		else 
		{
			// Send out a message saying we are already being escorted
			pPlayer = FindCharBySerial( pEscortee->ftarg );
			pEscortee->talk( tr( "I being escorted to %1 by %2." ).arg( pEscortee->questDestRegion() ).arg( pPlayer->name.c_str() ) );
		}
		return true;
	}
	return false;
}

bool BankerSpeech( cUOSocket *socket, P_CHAR pPlayer, P_CHAR pBanker, const QString& comm )
{
	// Needs to be a banker
	if( pBanker->npcaitype() != 8 )
		return false;

	if( pPlayer->dist(pBanker) > 6 )
		return false;

	if( comm.contains( "BANK" ) )
	{
		socket->sendContainer( pPlayer->getBankBox() );
		return true;
	}

	if( ( comm.contains( "BALANCE" ) ) | ( comm.contains( "WITHDRAW" ) ) || ( comm.contains( "CHECK" ) ) )
	{
	    //BankerAI->DoAI( s, pBanker, comm );
	    return true;
	}

    return false;
}

bool TrainerSpeech( cUOSocket *socket, P_CHAR pPlayer, P_CHAR pTrainer, const QString& comm ) 
{
	if( pPlayer->dist( pTrainer ) > 3 || !pTrainer->isHuman() )
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
		if( comm.contains( skillname[i], false ) )
		{
			skill = i;
			break;
		}

	if( skill != -1 )
	{
		if( pTrainer->baseSkill( skill ) <= 10 || !pTrainer->cantrain() )
		{
			pTrainer->talk( "I am sorry but I cannot train thee in that skill." );
			return true;
		}

		QString skillName = skillname[ skill ];
		skillName = skillName.lower();

		QString message = tr( "Thou wishest to learn of %1" ).arg( skillName );

		// This should be configureable
		if( pPlayer->baseSkill(skill) >= 250 )
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
				int perc = ( pPlayer->baseSkill( skill ) + delta ) / 10;
				
				message.append( tr( " Very well I, can train thee up to the level of %i percent for %i gold. Pay for less and I shall teach thee less." ).arg( perc ).arg( delta ) );

				pPlayer->setTrainer( pTrainer->serial );
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
		if( pTrainer->baseSkill( i ) >= 10 && pPlayer->baseSkill( i ) < 250 )
		{
			QString skillName = skillname[i];
			skillList.push_back( skillName.lower() );
		}
	}

	// skills and a trainer ?
	if( skillList.count() > 0 ) 
		pTrainer->talk( tr( "I can teach thee the following skills: %1." ).arg( skillList.join( ", " ) ) );
	else
		pTrainer->talk( tr( "I am sorry, but I have nothing to teach thee" ) );

	return true;
}

bool PetCommand( cUOSocket *socket, P_CHAR pPlayer, P_CHAR pPet, const QString& comm )
{
	if( !pPlayer->Owns( pPet ) || pPlayer->isGM() )
		return false;

	// player vendor
	if( pPet->npcaitype() == 17 )
		return false;
	
	// too far away to hear us
	if( pPlayer->dist( pPet ) > 7 )
		return false;
	
	QString petname = pPet->name.c_str();
	bool bAllCommand = false;

	if( !comm.contains( petname, false ) )
		if( comm.contains( "ALL", false ) )
			bAllCommand = true;
		else
			return false;
	
	bool bReturn = false;
	
	if( comm.contains( " FOLLOW" ) )
	{
		pPlayer->setGuarded( false );

		if( comm.contains( " ME" ) )
		{
			pPet->ftarg = pPlayer->serial;
			pPet->npcWander = 1;
			playmonstersound( pPet, pPet->id(), SND_STARTATTACK );
		}
		else
		{
			// LEGACY: target( s, 0, 1, 0, 117, "Click on the target to follow." );
		}

		bReturn = true;
	}
	else if( ( comm.contains( " KILL" ) ) || ( comm.contains( " ATTACK" ) ) )
	{
		if( pPet->inGuardedArea() ) // Ripper..No pet attacking in town.
		{
			pPlayer->message( tr( "You can't have pets attack in town!" ) );
			return false;
		}

		pPlayer->setGuarded( false );
		// >> LEGACY
		//addx[s]=pPet->serial;
		//target(s, 0, 1, 0, 118, "Select the target to attack.");//AntiChrist
		bReturn = true;
	}
	else if( ( comm.contains( " FETCH" ) ) || ( comm.contains( " GET" ) ) )
	{
		pPlayer->setGuarded(false);
		// >> LEGACY
		//addx[s]=pPet->serial;
		//target(s, 0, 1, 0, 124, "Click on the object to fetch.");
		bReturn = true;
	}
	else if( comm.contains( " COME" ) )
	{
		pPlayer->setGuarded(false);
		pPet->ftarg = pPlayer->serial;
		pPet->npcWander = 1;
		pPet->setNextMoveTime();
		pPlayer->message( tr( "Your pet begins following you." ) );
		bReturn = true;
	}
	else if( comm.contains( " GUARD" ) )
	{
		// LEGACY
		/*addx[s] = pPet->serial;	// the pet's serial
		addy[s] = 0;

		if( comm.find( " ME" ) != string::npos )
			addy[s]=1;	// indicates we already know whom to guard (for future use)
		
		// for now they still must click on themselves (Duke)
		target(s, 0, 1, 0, 120, "Click on the char to guard.");*/
		bReturn = true;
	}
	else if( ( comm.contains( " STOP" ) ) || ( comm.contains(" STAY") ) )
	{
		pPlayer->setGuarded( false );
		pPet->ftarg = INVALID_SERIAL;
		pPet->targ = INVALID_SERIAL;
		if (pPet->war) 
			npcToggleCombat( pPet );
		pPet->npcWander=0;
		bReturn = true;
	}
	else if( comm.contains( " TRANSFER" ) )
	{
		pPlayer->setGuarded( false );
		// >> LEGACY
		/*addx[s]=pPet->serial;
		target(s, 0, 1, 0, 119, "Select character to transfer your pet to.");*/
		bReturn = true;
	}
	else if( comm.contains( " RELEASE" ) )
	{
		pPlayer->setGuarded( false );

		// Has it been summoned ? Let's dispel it
		if( pPet->summontimer )
			pPet->summontimer = uiCurrentTime;

		pPet->ftarg = INVALID_SERIAL;
		pPet->npcWander = 2;
		pPet->SetOwnSerial( -1 );
		pPet->setTamed( false );
		pPet->emote( tr( "%1 appears to have decided that it is better off without a master" ).arg( pPet->name.c_str() ) );
		if( SrvParams->tamedDisappear() ==1 )
		{
			soundeffect2( pPet, 0x01FE );
			Npcs->DeleteChar( pPet );
		}
		bReturn = true;
	}

	// give other pets opotunity to process command
	if ( bReturn && bAllCommand )
		return false; 
	else
		return bReturn;
}

//PlayerVendors
void PlVGetgold( cUOSocket *socket, P_CHAR pPlayer, P_CHAR pVendor )
{
	if( !pPlayer->Owns( pVendor ) )
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
}

bool VendorChkName( P_CHAR pVendor, const QString& comm )
{
	if( ( comm.contains( "VENDOR" ) ) || ( comm.contains( "SHOPKEEPER" ) ) )
		return true;

	if( comm.contains( pVendor->name.c_str(), false ) )
		return true;
	else
		return false;
}

bool PlayerVendorSpeech( cUOSocket *socket, P_CHAR pPlayer, P_CHAR pVendor, const QString &comm )
{
	if( pVendor->npcaitype() != 17 )
	     return false;

	if( pPlayer->dist( pVendor ) > 4 )
		return false;

	if( !VendorChkName( pVendor, comm ) )
		return false;

	if( ( comm.contains( " BROWSE" ) ) || ( comm.contains( " VIEW" ) ) || ( comm.contains( " LOOK" ) ) )
	{
		pVendor->talk( tr( "Take a look at my goods." ) );
		if( pPlayer->socket() )
			pPlayer->socket()->sendContainer( pVendor->getBackpack() );
	    return true;
	}
	
	if( ( comm.contains( " BUY" ) ) || ( comm.contains( " PURCHASE" ) ) )
	{
		// >> LEGACY
		/*addx[s]=pVendor->serial;
		npctalk(s,pVendor,"What would you like to buy?",0);
		target(s,0,1,0,224," ");*/
		return true;
	}

	if( !pPlayer->Owns( pVendor ) )
		return false;

	if( ( comm.contains( " COLLECT" ) ) || ( comm.contains( " GOLD" ) ) || ( comm.contains( " GET" ) ) )
	{
		PlVGetgold( socket, pPlayer, pVendor);
		return true;
	}

	if( comm.contains( "PACKUP" ) )
	{
		P_ITEM pDeed = Items->SpawnItem( pPlayer, 1, "employment deed", 0, 0x14F0, 0, 1 );
		if( pDeed )
		{
			pDeed->setType( 217 );
			pDeed->value = 2000;
			RefreshItem( pDeed );
			Npcs->DeleteChar( pVendor );
			socket->sysMessage( tr( "Packed up vendor %1." ).arg( pVendor->name.c_str() ) );
			return true;
		}
	}
	return false;
}

bool VendorSpeech( cUOSocket *socket, P_CHAR pPlayer, P_CHAR pVendor, const QString& comm )
{
	if( pVendor->npcaitype() == 17 )
		return false;

	if( !pVendor->shop )
		return false;

	if( pPlayer->dist( pVendor ) > 4 )
		return false;

	if( !VendorChkName( pVendor, comm ) )
		return false;

	// TODO: Rip this code out and
	// use python instead
	if( comm.contains( " BUY" ) )
	{
		// 0x1A = Normal Vendor items
		// 0x1B = Items Players sold to the vendor
		P_ITEM pItem = pVendor->GetItemOnLayer( 0x1A );

		pVendor->turnTo( pPlayer );

		if( !pItem )
		{
			pVendor->talk( tr( "Sorry but i have no goods to sell" ) );
			return true;
		}

		pVendor->talk( tr( "Take a look at my wares!" ) );
		socket->sendBuyWindow( pVendor );
		return true;
	}

	if( comm.contains( " SELL" ) )
	{
		// LEGACY
		//sellstuff(s, pVendor);
		return true;
	}

	return false;
}

// Handles house commands from friends of the house.
// msg must already be capitalized
void HouseSpeech( cUOSocket *socket, P_CHAR pPlayer, const QString& msg )
{
	// Not inside a multi
	if( pPlayer->multis == INVALID_SERIAL )
		return; 

	P_ITEM pMulti = FindItemBySerial( pPlayer->multis );

	if( !pMulti )
	{
		clConsole.send( tr( "Player %1 [0x%2] has bad multi serial [0x%1]" ).arg( pPlayer->name.c_str() ).arg( pPlayer->serial, 8, 16 ).arg( pPlayer->multis ) );
		pPlayer->multis = INVALID_SERIAL;
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
		addid1[s] = pMulti->serial>>24;
		addid2[s] = pMulti->serial>>16;
		addid3[s] = pMulti->serial>>8;
		addid4[s] = pMulti->serial%256;
		target(s, 0, 1, 0, 229, "Select person to ban from house.");
	}
	else if(msg.contains("REMOVE THYSELF")) 
	{ // kick out of house
		addid1[s] = pMulti->serial>>24;
		addid2[s] = pMulti->serial>>16;
		addid3[s] = pMulti->serial>>8;
		addid4[s] = pMulti->serial%256;
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
bool cSpeech::response( cUOSocket *socket, P_CHAR pPlayer, const QString& comm )
{
    // What the heck is that. It's a COMMAND and then we can use 'remove anyway !!
	/*if ((comm.contains("#EMPTY")) && online(currchar[s]) && !pPlayer->dead && pPlayer->isGM())
	{ // restricted to GMs for now. It's too powerful (Duke, 5.6.2001)
		target(s, 0, 1, 0, 71, "Select container to empty:");
		return 1;
	}*/

    if( !pPlayer->socket() || pPlayer->dead )
		return false;

	cRegion::RegionIterator4Chars ri( pPlayer->pos );
	for( ri.Begin(); !ri.atEnd(); ri++ )
	{
		P_CHAR pNpc = ri.GetData();

		if( pPlayer == pNpc )
			continue;

		// We will only process NPCs here
		if( !pNpc->isNpc() )
			continue;

		// at least they should be on the screen
		if( pPlayer->dist( pNpc ) > 16 )
			continue;
		
		if ( pNpc->onTalkToNPC( pPlayer, comm ) )
			return true;

		if( StableSpeech( socket, pPlayer, pNpc, comm ) )
			return true;
		
		if( UnStableSpeech( socket, pPlayer, pNpc, comm ) )
			return true;
		
		if( ShieldSpeech( socket, pPlayer, pNpc, comm ) )
			return true;
		
		if( QuestionSpeech( socket, pPlayer, pNpc, comm ) )
			return true;
		
		if( TriggerSpeech( socket, pPlayer, pNpc, comm ) )
			return true;
		
		if( EscortSpeech( socket, pPlayer, pNpc, comm ) )
			return true;
		
		if( BankerSpeech( socket, pPlayer, pNpc, comm ) )
			return true;
		
		if( TrainerSpeech( socket, pPlayer, pNpc, comm ) )
			return true;
		
		if( PetCommand( socket, pPlayer, pNpc, comm ) )
			return true;

		if( PlayerVendorSpeech( socket, pPlayer, pNpc, comm ) )
			return true;
		
		if( VendorSpeech( socket, pPlayer, pNpc, comm ) )
			return true;
	}
	
	return false;
}

void cSpeech::talking( P_CHAR pChar, const QString &speech, UINT16 color, UINT8 type ) // PC speech
{	
	// handle things like renaming or describing an item
	if( !pChar->socket() )
		return;

	cUOSocket *socket = pChar->socket();

	if( InputSpeech( socket, pChar, speech ) )	
		return;

	// not allowed to talk
	if( pChar->squelched() )
	{
		socket->sysMessage( tr( "You re squelched and cannot talk" ) );
		return;
	}

	pChar->unhide();
		
	if( ( type == 0x09 ) && ( pChar->canBroadcast() ) )
	{
		pChar->talk( speech, color, type );
		return;
	}

	if( type == 0 || type == 2)
		pChar->saycolor = color;

	if( SrvParams->speechLog() )
	{
		QFile lFile( "speech.log" );
		
		if( lFile.open( IO_Append ) )
		{
			QString logMessage( "[%1] %2: %3 [%4, 0x%5]" );
			logMessage = logMessage.arg( getRealTimeString().c_str() ).arg( pChar->name.c_str() ).arg( speech ).arg( pChar->account()->login() ).arg( pChar->serial, 8, 16 );
			lFile.writeBlock( logMessage.latin1(), logMessage.length() );
			lFile.close();
		}
	}

	pChar->talk( speech, color, type );
	QString speechUpr = speech.upper();

	// >> LEGACY
	/*if( speechUpr == "I RESIGN FROM MY GUILD" )
	{
		GuildResign(s);
	}*/
	
	if( response( socket, pChar, speechUpr ) )
		return;  // Vendor responded already
	
	// >> LEGACY
	if( speech.contains( "guards", false ) )
		callguards( pChar );

	cRegion::RegionIterator4Items rj( pChar->pos );
	for( rj.Begin(); !rj.atEnd(); rj++ )
	{
		P_ITEM pi = rj.GetData();

		if( !pi )
			continue;

		if( pi->type() == 117 && pi->tags.get( "tiller" ).toInt() == 1 )
		{
			cBoat* pBoat = dynamic_cast< cBoat* >(FindItemBySerial( pi->tags.get("boatserial").toUInt() ));
			// >> LEGACY
			//if( pBoat )
				//pBoat->speechInput( s, SpeechUpr );
		}
	}

	// >> LEGACY
	// house_speech( s, SpeechUpr );

	// this makes it so npcs do not respond to dead people - HEALERS ??
	if( pChar->dead )
		return;
	
	cChar* pc = NULL;
	cChar* pNpc = NULL;
	cRegion::RegionIterator4Chars ri( pChar->pos );
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

	if( pNpc && pNpc->speech )
	{
		Script *pScp=i_scripts[speech_script];
		if (!pScp->Open())
			return;
		char sect[512];
		sprintf(sect, "SPEECH %i", pNpc->speech);
		if (!pScp->find(sect)) 
		{
			pScp->Close();
			return;
		}
		int match = 0;
		strcpy(sect, "NO DEFAULT TEXT DEFINED");
		unsigned long loopexit = 0;
		do
		{
			pScp->NextLineSplitted();
			if (script1[0] != '}')
			{
				if (!(strcmp("DEFAULT", (char*)script1)))
				{
					strcpy(sect, (char*)script2);
				}
				if (!(strcmp("ON", (char*)script1)))
				{
					char scpUpr[500];
					strcpy(scpUpr,script2);
					strupr(scpUpr);
					if( speechUpr.contains( scpUpr ) )
						match=1;
				}
				if (!(strcmp("SAY", (char*)script1)))
				{
					if (match == 1)
					{
						pNpc->talk( script2 );
						match = 2;
					}
				}
				
				if (!(strcmp("TRG", (char*)script1))) // Added by Magius(CHE) §
				{							  
					if (match == 1)
					{
						pNpc->setTrigger( str2num(script2) );
						scpMark m=pScp->Suspend();
						
						// >> LEGACY
						// Trig->triggernpc(s, pNpc, 1);
						
						pScp->Resume(m);
						strcpy((char*)script1, "DUMMY");
						
						match = 2;
					}
				}
			}
		}
		while (script1[0] != '}'  && (++loopexit < MAXLOOPS));

		if (match == 0)
			pNpc->talk( sect );
		
		pScp->Close();
	}
}
