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

#include "player.h"
#include "persistentbroker.h"
#include "dbdriver.h"
#include "globals.h"
#include "chars.h"
#include "console.h"
#include "network.h"
#include "network/uosocket.h"
#include "network/uotxpackets.h"
#include "basics.h"
#include "srvparams.h"
#include "world.h"
#include "npc.h"
#include "wpdefmanager.h"
#include "corpse.h"
#include "sectors.h"
#include "tilecache.h"
#include "skills.h"
#include "pythonscript.h"
#include "log.h"
#include "scriptmanager.h"
#include "inlines.h"

cPlayer::cPlayer()
{
	account_			= NULL;
	logoutTime_			= 0;
	muteTime_			= 0;
	objectDelay_		= 0;
	additionalFlags_	= 0;
	trackingTime_		= 0;
	socket_				= NULL;
	inputMode_			= enNone;
	inputItem_			= INVALID_SERIAL;
	visualRange_		= VISRANGE;
	profile_			= (char*)0;
	fixedLightLevel_	= 0xFF;
}


cPlayer::cPlayer(const cPlayer& right)
{
}

cPlayer::~cPlayer()
{
}

cPlayer& cPlayer::operator=(const cPlayer& right)
{
	return *this;
}

static cUObject* productCreator()
{
	return new cPlayer;
}

void cPlayer::registerInFactory()
{
	QStringList fields, tables, conditions;
	buildSqlString( fields, tables, conditions ); // Build our SQL string
	QString sqlString = QString( "SELECT %1 FROM uobjectmap,%2 WHERE uobjectmap.type = 'cPlayer' AND %3" ).arg( fields.join( "," ) ).arg( tables.join( "," ) ).arg( conditions.join( " AND " ) );
	UObjectFactory::instance()->registerType( "cPlayer", productCreator );
	UObjectFactory::instance()->registerSqlQuery( "cPlayer", sqlString );
}

void cPlayer::buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions )
{
	cBaseChar::buildSqlString( fields, tables, conditions );
	fields.push_back( "players.account,players.additionalflags,players.visualrange" );
	fields.push_back( "players.profile,players.fixedlight" );
	tables.push_back( "players" );
	conditions.push_back( "uobjectmap.serial = players.serial" );
}

static void playerRegisterAfterLoading( P_PLAYER pc );

void cPlayer::load( char **result, UINT16 &offset )
{
	cBaseChar::load( result, offset );

	setAccount( Accounts::instance()->getRecord( result[offset++] ) );
	additionalFlags_ = atoi( result[offset++] );
	visualRange_ = atoi( result[offset++] );
	profile_ = result[offset++];
	fixedLightLevel_ = atoi( result[offset++] );

	playerRegisterAfterLoading( this );
	changed_ = false;
}

void cPlayer::save()
{
	if ( changed_ )
	{
		initSave;
		setTable( "players" );
		
		addField( "serial", serial() );

		if( account_ )
		{
			addStrField( "account", account_->login() );
		}
		else
		{
			addStrField( "account", QString::null );
		}

		addField( "additionalflags", additionalFlags_ );
		addField( "visualrange", visualRange_ );
		addStrField( "profile", profile_ );
		addField( "fixedlight", fixedLightLevel_ );

		addCondition( "serial", serial() );
		saveFields;
	}
	cBaseChar::save();
}

bool cPlayer::del()
{	
	if( !isPersistent )
		return false; // We didn't need to delete the object

	persistentBroker->addToDeleteQueue( "players", QString( "serial = '%1'" ).arg( serial() ) );
	changed_ = true;
	return cBaseChar::del();
}

static void playerRegisterAfterLoading( P_PLAYER pc )
{
	if (pc->account() == 0)
	{
		// We need to remove the equipment here.
		cBaseChar::ItemContainer container(pc->content());
		cBaseChar::ItemContainer::const_iterator it (container.begin());
		cBaseChar::ItemContainer::const_iterator end(container.end());
		for (; it != end; ++it )
		{
			P_ITEM pItem = *it;
			if( !pItem )
				continue;

			pItem->remove();
		}
		pc->del();
		return;
	} 
/*	else
	{
		pc->setBodyID(0x0190);
		Console::instance()->send("player: %s with bugged body-value detected, restored to male shape\n",pc->name().latin1());
	}*/
}

// Update flags etc.
void cPlayer::update( bool excludeself )
{
	cUOTxUpdatePlayer* updatePlayer = new cUOTxUpdatePlayer();
	updatePlayer->fromChar( this );

	for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		P_PLAYER pChar = mSock->player();

		if( pChar && pChar->socket() && pChar->inRange( this, pChar->visualRange() ) )
		{
			updatePlayer->setHighlight( notority( pChar ) );
			mSock->send( new cUOTxUpdatePlayer( *updatePlayer ) );
		}
	}

	delete updatePlayer;

	if( !excludeself && socket_ )
	{
		cUOTxDrawPlayer drawPlayer;
		drawPlayer.fromChar( this );
		socket_->send( &drawPlayer );
	}
}

// Resend the char to all sockets in range
void cPlayer::resend( bool clean, bool excludeself )
{
	if( socket_ && !excludeself )
		socket_->resendPlayer();

	cUOTxRemoveObject rObject;
	rObject.setSerial( serial() );

	cUOTxDrawChar drawChar;
	drawChar.fromChar( this );

	cUOSocket *mSock;

	for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		// Don't send such a packet to ourself
		if( mSock == socket_ )
		{
			sendTooltip( mSock );		
			continue;
		}


		P_PLAYER pChar = mSock->player();

		if( !pChar || !pChar->account() )
			continue;

		if( pChar->dist( this ) > pChar->visualRange() )
			continue;
        
		if( clean )
			mSock->send( &rObject );

		// We are logged out and the object can't see us.
		if( !socket_  && !pChar->account()->isAllShow() )
			continue;

		// We are hidden (or dead and not visible)
		if( ( isHidden() || ( isDead() && !isAtWar() ) ) && !pChar->isGMorCounselor() )
			continue;

		drawChar.setHighlight( notority( pChar ) );
		
		sendTooltip( mSock );
		mSock->send( &drawChar );
	}
}

void cPlayer::talk( const QString &message, UI16 color, UINT8 type, bool autospam, cUOSocket* socket )
{
	if( color == 0xFFFF )
		color = saycolor_;

	QString lang;

	if( socket_ )
		lang = socket_->lang();
	
	cUOTxUnicodeSpeech::eSpeechType speechType;

	switch( type )
	{
	case 0x01:		speechType = cUOTxUnicodeSpeech::Broadcast;		break;
	case 0x06:		speechType = cUOTxUnicodeSpeech::System;		break;
	case 0x09:		speechType = cUOTxUnicodeSpeech::Yell;			break;
	case 0x02:		speechType = cUOTxUnicodeSpeech::Emote;			break;
	case 0x08:		speechType = cUOTxUnicodeSpeech::Whisper;		break;
	case 0x0A:		speechType = cUOTxUnicodeSpeech::Spell;			break;
	default:		speechType = cUOTxUnicodeSpeech::Regular;		break;
	};

	cUOTxUnicodeSpeech* textSpeech = new cUOTxUnicodeSpeech();
	textSpeech->setSource( serial() );
	textSpeech->setModel( bodyID_ );
	textSpeech->setFont( 3 ); // Default Font
	textSpeech->setType( speechType );
	textSpeech->setLanguage( lang );
	textSpeech->setName( name() );
	textSpeech->setColor( color );
	textSpeech->setText( message );

	QString ghostSpeech;

	// Generate the ghost-speech *ONCE*
	if( isDead() )
	{
		for( UINT32 gI = 0; gI < message.length(); ++gI )
		{
			if( message.at( gI ) == " " )
				ghostSpeech.append( " " );
			else 
				ghostSpeech.append( ( RandomNum( 0, 1 ) == 0 ) ? "o" : "O" );
		}

	}

	if( socket )
	{
		// Take the dead-status into account
		if( isDead() )
			if( !socket->player()->isDead() && !socket->player()->isGMorCounselor() )
				textSpeech->setText( ghostSpeech );
			else
				textSpeech->setText( message );

		socket->send( textSpeech );
	}
	else
	{
		// Send to all clients in range
		for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
		{
				if( mSock->player() && ( mSock->player()->dist( this ) < 18 ) )
				{
					// Take the dead-status into account
					if( isDead() )
						if( !mSock->player()->isDead() && !mSock->player()->isGMorCounselor() )
							textSpeech->setText( ghostSpeech );
						else
							textSpeech->setText( message );

					mSock->send( new cUOTxUnicodeSpeech( *textSpeech ) );
				}
		}
		delete textSpeech;
	}
}

UINT8 cPlayer::notority( P_CHAR pChar ) // Gets the notority toward another char
{
	// 0x01 Blue, 0x02 Green, 0x03 Grey, 0x05 Orange, 0x06 Red
	UINT8 result;

	// Check for Guild status + Highlight
//	UINT8 guildStatus = GuildCompare( this, pChar );

	if( pChar->kills() > SrvParams->maxkills() )
		result = 0x06; // 6 = Red -> Murderer
	
//	else if( guildStatus == 1 )
//		result = 0x02; // 2 = Green -> Same Guild
	
//	else if( guildStatus == 2 )
//		result = 0x05; // 5 = Orange -> Enemy Guild

	else if( account_ )
	{
		if( isCriminal() )
			result = 0x03;
		else if( karma_ < -2000 ) 
			result = 0x06;
		else if( karma_ < 0 )
			result = 0x03;
		else
			result = 0x01;
	}
	else
	{
		// Everything else
		result = 0x03;
	}
	
	return result;
}

void cPlayer::kill()
{
	changed( TOOLTIP );
	changed_ = true;
	int ele;

	if( free )
		return;

	if( isDead() || isInvulnerable() )
		return;

	// Do this in the beginning
	setDead( true );
	hitpoints_ = 0;

	if( isPolymorphed() )
	{
		setBodyID( orgBodyID_ );
		setPolymorphed( false );
	}

	orgBodyID_ = bodyID_;
	setOrgSkin( skin() );
	setMurdererSerial( INVALID_SERIAL ); // Reset previous murderer serial # to zero

	QString murderer( "" );

	P_CHAR pAttacker = FindCharBySerial( attackerSerial_ );
	if( pAttacker )
	{
		pAttacker->setCombatTarget(INVALID_SERIAL);
		murderer = pAttacker->name();
	}

	// We do know our murderer here (or if there is none it's null)
	// So here it's time to kall onKilled && onKill
	// And give them a chance to return true
	// But take care. You would need to create the corpse etc. etc.
	// Which is *hard* work
	// TODO: Call onKilled/onKill events

	// Reputation system ( I dont like the idea of this loop )
	cCharIterator iter_char;
	P_CHAR pc_t;

	for( pc_t = iter_char.first(); pc_t; pc_t = iter_char.next() )
	{
		if( ( pc_t->swingTarget() == serial() || pc_t->combatTarget() == serial() ) && !pc_t->free )
		{
/*			if( pc_t->npcaitype() == 4 )
			{
				pc_t->setSummonTimer( ( uiCurrentTime + ( MY_CLOCKS_PER_SEC * 20 ) ) );
				pc_t->setNpcWander(2);
				pc_t->setNextMoveTime();
				pc_t->talk( tr( "Thou have suffered thy punishment, scoundrel." ), -1, 0, true );
			}*/

			pc_t->setCombatTarget( INVALID_SERIAL );
			pc_t->setNextHitTime(0);
			pc_t->setSwingTarget( INVALID_SERIAL );

			if( pc_t->attackerSerial() != INVALID_SERIAL )
			{
				P_CHAR pc_attacker = FindCharBySerial( pc_t->attackerSerial() );
				pc_attacker->setAttackFirst( false );
				pc_attacker->setAttackerSerial( INVALID_SERIAL );
			}

			pc_t->setAttackerSerial(INVALID_SERIAL);
			pc_t->setAttackFirst( false );

			if( ( pc_t->objectType() == enPlayer ) && !pc_t->inGuardedArea() )
			{
				P_PLAYER pp_t = dynamic_cast<P_PLAYER>(pc_t);
				pp_t->awardKarma( this, ( 0 - ( karma_ ) ) );
				pp_t->awardFame( fame_ );

				if( isInnocent() && pp_t->attackFirst() )
				{
					setMurdererSerial( pp_t->serial() );
					pp_t->kills_++;

					// Notify the user of reputation changes
					if( pp_t->socket() )
					{
						pp_t->socket()->sysMessage( tr( "You have killed %1 innocent people." ).arg( pp_t->kills_ ) );

						if( pp_t->kills_ >= SrvParams->maxkills() )
						{
							pp_t->socket()->sysMessage( tr( "You are now a murderer!" ) );
							
							// Set the Murder Decay Time
							pp_t->setMurdererTime( getNormalizedTime() + SrvParams->murderdecay() * MY_CLOCKS_PER_SEC );
						}
					}
				}
			}

			Log::instance()->print( LOG_NOTICE, QString( "%1 was killed by %2\n" ).arg( name() ).arg( pc_t->name() ) );

			if( pc_t->objectType() == enNPC && pc_t->isAtWar() )
			{
				dynamic_cast<P_NPC>(pc_t)->toggleCombat();
			}
		}
	}

	// Now for the corpse
	P_ITEM pi_backpack = getBackpack();
	
	unmount();

#pragma message(__FILE__ Reminder "Implement here tradewindow closing and disposal of it's cItem*")
	// Close here the trade window... we still not sure how this will work, so I took out
	//the old code
	ele = 0;

	// I would *NOT* do that but instead replace several *send* things
	// We have ->dead already so there shouldn't be any checks regarding
	// 0x192-0x193 to see if the char is dead or not
	if( orgBodyID_ == 0x0191 )
		setBodyID( 0x0193 );	// Male or Female
	else
		setBodyID( 0x0192 );

	playDeathSound();

	setSkin( 0x0000 ); // Undyed
	
	// Reset poison
	setPoisoned(0);
	setPoison(0);

	// Create our Corpse
	cCorpse *corpse = new cCorpse( true );

	const cElement *elem = DefManager->getDefinition( WPDT_ITEM, "2006" );
	
	if( elem )
		corpse->applyDefinition( elem );

	corpse->setName( tr( "corpse of %1" ).arg( name() ) );
	corpse->setColor( orgSkin() );

	// Check for the player hair/beard
	P_ITEM pHair = GetItemOnLayer( 11 );
	
	if( pHair )
	{
		corpse->setHairColor( pHair->color() );
		corpse->setHairStyle( pHair->id() );
	}

	P_ITEM pBeard = GetItemOnLayer( 16 );
	
	if( pBeard )
	{
		corpse->setBeardColor( pBeard->color() );
		corpse->setBeardStyle( pBeard->id() );
	}

	// Storing the player's notority
	// So a singleclick on the corpse
	// Will display the right color
	if( isInnocent() )
		corpse->setTag( "notority", cVariant( 1 ) );
	else if( isCriminal() )
		corpse->setTag( "notority", cVariant( 2 ) );
	else if( isMurderer() )
		corpse->setTag( "notority", cVariant( 3 ) );

    corpse->setOwner( this );

	corpse->setBodyId( orgBodyID_ );
	corpse->setTag( "human", cVariant( isHuman() ? 1 : 0 ) );
	corpse->setTag( "name", cVariant( name() ) );

	corpse->moveTo( pos() );

	corpse->setDirection( direction() );
	
	// Set the ownerserial to the player's
	corpse->SetOwnSerial( serial() );

	// stores the time and the murderer's name
	corpse->setMurderer( murderer );
	corpse->setMurderTime(uiCurrentTime);

	std::vector< P_ITEM > equipment;

	// Check the Equipment and Unequip if neccesary
	cBaseChar::ItemContainer::const_iterator iter;
	for ( iter = content_.begin(); iter != content_.end(); iter++ )
	{
		P_ITEM pi_j = iter.data();

		if( pi_j )
			equipment.push_back( pi_j );
	}

	for( std::vector< P_ITEM >::iterator iit = equipment.begin(); iit != equipment.end(); ++iit )
	{
		P_ITEM pi_j = *iit;

		// unequip trigger...
		if( pi_j->layer() != 0x0B && pi_j->layer() != 0x10 )
		{	// Let's check all items, except HAIRS and BEARD

			if( pi_j->type() == 1 && pi_j->layer() != 0x1A && pi_j->layer() != 0x1B && pi_j->layer() != 0x1C && pi_j->layer() != 0x1D )
			{   // if this is a pack but it's not a VendorContainer(like the buy container) or a bankbox
				cItem::ContainerContent container = pi_j->content();
				cItem::ContainerContent::const_iterator it2 = container.begin();
				for ( ; it2 != container.end(); ++it2 )
				{
					P_ITEM pi_k = *it2;

					if( !pi_k )
						continue;

					// put the item in the corpse only of we're sure it's not a newbie item or a spellbook
					if( !pi_k->newbie() && ( pi_k->type() != 9 ) )
					{					
						corpse->addItem( pi_k );
						
						// Ripper...so order/chaos shields disappear when on corpse backpack.
						if( pi_k->id() == 0x1BC3 || pi_k->id() == 0x1BC4 )
						{
							soundEffect( 0x01FE );
							this->effect( 0x372A, 0x09, 0x06 );
							pi_k->remove();
						}
					}
				}
			}
			// if it's a normal item but ( not newbie and not bank items )
			else if ( !pi_j->newbie() && pi_j->layer() != 0x1D )
			{
				if( pi_j != pi_backpack )
				{
					pi_j->removeFromView();
					corpse->addEquipment( pi_j->layer(), pi_j->serial() );
					corpse->addItem( pi_j );					
				}
			}
			else if( ( pi_j != pi_backpack ) && ( pi_j->layer() != 0x1D ) )
			{	
				// else if the item is newbie put it into char's backpack
				pi_j->removeFromView();
				pi_backpack->addItem( pi_j );
			}

			//if( ( pi_j->layer() == 0x15 ) && ( shop == 0 ) ) 
			//	pi_j->setLayer( 0x1A );
		}
	}

	corpse->update();

	cUOTxDeathAction dAction;
	dAction.setSerial( serial() );
	dAction.setCorpse( corpse->serial() );

	cUOTxRemoveObject rObject;
	rObject.setSerial( serial() );

	for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
		if( mSock->player() && mSock->player()->inRange( this, mSock->player()->visualRange() ) && ( mSock != socket_ ) )
		{
			mSock->send( &dAction );
			mSock->send( &rObject );
		}

	resend( false );

	P_ITEM pItem = cItem::createFromScript( "204e" );
	if( pItem )
	{
		this->addItem( cBaseChar::OuterTorso, pItem );
		pItem->update();
	}

	if( socket_ )
	{
		cUOTxCharDeath cDeath;
		socket_->send( &cDeath );
	}

	// trigger the event now
	onDeath();
}

void cPlayer::turnTo( cUObject *object )
{
	cBaseChar::turnTo( object->pos() );
}

void cPlayer::turnTo( const Coord_cl &pos )
{
	cBaseChar::turnTo( pos );
}

P_NPC cPlayer::unmount()
{
	P_ITEM pi = atLayer(Mount);
	if( pi && !pi->free )
	{
		P_NPC pMount = dynamic_cast<P_NPC>( FindCharBySerial( pi->getTag( "pet" ).toInt() ) );
		if( pMount )
		{
			pMount->setWanderType( enHalt );
			pMount->setWanderX1( pi->pos().x );
			pMount->setWanderY1( pi->pos().y );
			pMount->setWanderRadius( pi->pos().z );
			pMount->setBodyID( pi->getTag( "body" ).toInt() );
			pMount->setDirection( direction() );
			pMount->setStrength( pi->getTag( "strength" ).toInt() );
			pMount->setDexterity( pi->getTag( "dexterity" ).toInt() );
			pMount->setIntelligence( pi->getTag( "intelligence" ).toInt() );
			pMount->setHitpoints( pi->hp() );
			pMount->setFame( pi->lodamage() );
			pMount->setKarma( pi->hidamage() );
			pMount->setPoisoned( pi->poisoned() );
			pMount->setSummonTime( pi->decaytime() );
			
			pMount->moveTo( pos() );
			pMount->resend( false );
		}
		pi->remove();
		removeItem( Mount );
		resend( false );
		return pMount;
	}
	return NULL;
}

void cPlayer::mount( P_NPC pMount )
{	
	if( !pMount )
		return;

	cUOSocket* socket = this->socket();
	if( !inRange( pMount, 2 ) && !isGM() )
	{
		if( socket )
			socket->sysMessage( tr("You are too far away to mount!") );
		return;
	}

	if( pMount->owner() == this || isGM() )
	{
		unmount();

		P_ITEM pMountItem = new cItem;
		pMountItem->Init();
		pMountItem->setId( 0x915 );
		pMountItem->setName( pMount->name() );
		pMountItem->setColor( pMount->skin() );

		if( !pMountItem )
			return;

		switch( static_cast< unsigned short >(pMount->bodyID() & 0x00FF) )
		{
			case 0xC8: pMountItem->setId(0x3E9F); break; // Horse
			case 0xE2: pMountItem->setId(0x3EA0); break; // Horse
			case 0xE4: pMountItem->setId(0x3EA1); break; // Horse
			case 0xCC: pMountItem->setId(0x3EA2); break; // Horse
			case 0xD2: pMountItem->setId(0x3EA3); break; // Desert Ostard
			case 0xDA: pMountItem->setId(0x3EA4); break; // Frenzied Ostard
			case 0xDB: pMountItem->setId(0x3EA5); break; // Forest Ostard
			case 0xDC: pMountItem->setId(0x3EA6); break; // LLama
			case 0x34: pMountItem->setId(0x3E9F); break; // Brown Horse
			case 0x4E: pMountItem->setId(0x3EA0); break; // Grey Horse
			case 0x50: pMountItem->setId(0x3EA1); break; // Tan Horse
			case 0x74: pMountItem->setId(0x3EB5); break; // Nightmare
			case 0x75: pMountItem->setId(0x3EA8); break; // Silver Steed
			case 0x72: pMountItem->setId(0x3EA9); break; // Dark Steed
			case 0x7A: pMountItem->setId(0x3EB4); break; // Unicorn
			case 0x84: pMountItem->setId(0x3EAD); break; // Kirin
			case 0x73: pMountItem->setId(0x3EAA); break; // Etheral
			case 0x76: pMountItem->setId(0x3EB2); break; // War Horse-Brit
			case 0x77: pMountItem->setId(0x3EB1); break; // War Horse-Mage Council
			case 0x78: pMountItem->setId(0x3EAF); break; // War Horse-Minax
			case 0x79: pMountItem->setId(0x3EB0); break; // War Horse-Shadowlord
			case 0xAA: pMountItem->setId(0x3EAB); break; // Etheral LLama
			case 0x3A: pMountItem->setId(0x3EA4); break; // Forest Ostard
			case 0x39: pMountItem->setId(0x3EA3); break; // Desert Ostard
			case 0x3B: pMountItem->setId(0x3EA5); break; // Frenzied Ostard
			case 0x90: pMountItem->setId(0x3EB3); break; // Seahorse
			case 0xAB: pMountItem->setId(0x3EAC); break; // Etheral Ostard
			case 0xBB: pMountItem->setId(0x3EB8); break; // Ridgeback
			case 0x17: pMountItem->setId(0x3EBC); break; // giant beetle
			case 0x19: pMountItem->setId(0x3EBB); break; // skeletal mount
			case 0x1a: pMountItem->setId(0x3EBD); break; // swamp dragon
			case 0x1f: pMountItem->setId(0x3EBE); break; // armor dragon
		}
		
		this->addItem( cBaseChar::Mount, pMountItem );
		Coord_cl position = pMountItem->pos();
		position.x = pMount->wanderX1();
		position.y = pMount->wanderY1();
		position.z = pMount->wanderRadius();
		pMountItem->setPos( position );
		
		pMountItem->setTag( "pet", cVariant( pMount->serial() ) );
		pMountItem->setTag( "body", cVariant( pMount->bodyID() ) );

		pMountItem->setTag( "wanderType",	(int)pMount->wanderType() );
		pMountItem->setTag( "strength",		pMount->strength() );
		pMountItem->setTag( "dexterity",	pMount->dexterity() );
		pMountItem->setTag( "intelligence", pMount->intelligence() );

		pMountItem->setHp( pMount->hitpoints() );
		pMountItem->setLodamage( pMount->fame() );
		pMountItem->setHidamage( pMount->karma() );
		pMountItem->setPoisoned( pMount->poisoned() );
		if (pMount->summonTime() != 0)
			pMountItem->setDecayTime(pMount->summonTime());

		pMountItem->update();

		// if this is a gm lets tame the animal in the process
		if( isGM() )
		{
			pMount->setOwner( this );
		}
		
		// remove it from screen!
		pMount->removeFromView( true );
		
		pMount->setBodyID(0);
		MapObjects::instance()->remove( pMount );
		pMount->setPos( Coord_cl( 0, 0, 0 ) );
		
		pMount->setAtWar( false );
		pMount->setAttackerSerial(INVALID_SERIAL);
	}
	else
		socket->sysMessage( tr("You dont own that creature.") );
}

bool cPlayer::isGM() const
{
	return account() && ( account()->acl() == "admin" || account()->acl() == "gm" ) && account()->isStaff();
} 

bool cPlayer::isCounselor() const 
{
	return account() && ( account()->acl() == "counselor" )  && account()->isStaff();
}
 
bool cPlayer::isGMorCounselor() const	
{
	return account() && ( account()->acl() == "admin" || account()->acl() == "gm" || account()->acl() == "counselor" ) && account()->isStaff();
} 

void cPlayer::showName( cUOSocket *socket )
{
	if( !socket->player() )
		return;

	QString charName = name();

	// Lord & Lady Title
	if( fame_ == 10000 )
		charName.prepend( gender_ ? tr( "Lady " ) : tr( "Lord " ) );

	// Are we squelched ?
	if( isMuted() )
		charName.append( tr(" [muted]" ) );

	// Append serial for GMs
	if( socket->player()->showSerials() )
		charName.append( QString( " [0x%1]" ).arg( serial(), 4, 16 ) );

	// Append offline flag
	if( !socket_ )
		charName.append( tr(" [offline]") );

	// Invulnerability
	if( isInvulnerable() )
		charName.append( tr(" [invul]") );

	// Frozen
	if( isFrozen() )
		charName.append( tr(" [frozen]") );

	// Guarded
	if( guardedby_.size() > 0 )
		charName.append( tr(" [guarded]") );

	// Guarding
	if( isTamed() && guarding_ )
		charName.append( tr(" [guarding]") );

	// Tamed
	if( isTamed() )
		charName.append( tr(" [tamed]") );

	// WarMode ?
	if( isAtWar() )
		charName.append( tr(" [war mode]") );

	// Criminal ?
	if( ( criminalTime_ > uiCurrentTime ) && ( kills_ < SrvParams->maxkills() ) )
		charName.append( tr(" [criminal]") );

	// Murderer
	if( kills_ >= SrvParams->maxkills() )
		charName.append( tr(" [murderer]") );

	Q_UINT16 speechColor;

	// 0x01 Blue, 0x02 Green, 0x03 Grey, 0x05 Orange, 0x06 Red
	switch( notority( socket->player() ) )
	{		
		case 0x01:	speechColor = 0x5A;		break; //blue
		case 0x02:	speechColor = 0x43;		break; //green
		case 0x03:	speechColor = 0x3B2;	break; //grey
		case 0x05:	speechColor = 0x30;		break; //orange
		case 0x06:	speechColor = 0x26;		break; //red
		default:	speechColor = 0x3B2;	break; // grey		
	}

	// Show it to the socket
	socket->showSpeech( this, charName, speechColor, 3, cUOTxUnicodeSpeech::System );
}

/*!
	Make someone criminal.
*/
void cPlayer::makeCriminal()
{
	if( isGMorCounselor() )
		return;

	//not grey, not red	
	if( !this->isCriminal() && !this->isMurderer() )
	{ 
		 this->setCriminalTime((SrvParams->crimtime()*MY_CLOCKS_PER_SEC)+uiCurrentTime);

		 if( socket_ )
			 socket_->sysMessage( tr( "You are now a criminal!" ) );

		 changed_ = true;
	}
}

void cPlayer::fight(P_CHAR other)
{
	// I am already fighting this character.
	if( isAtWar() && combatTarget() == other->serial() )
		return;

	// Store the current Warmode
	bool oldwar = isAtWar();

	this->combatTarget_ = other->serial();
	this->unhide();
	this->disturbMed();	// Meditation
	this->attackerSerial_ = other->serial();
	this->setAtWar( true );
	
	if( socket_ )
	{
		// Send warmode status
		cUOTxWarmode warmode;
		warmode.setStatus( true );
		socket_->send( &warmode );

		// Send Attack target
		cUOTxAttackResponse attack;
		attack.setSerial( other->serial() );
		socket_->send( &attack );

		// Resend the Character (a changed warmode results in not walking but really just updating)
		if( oldwar != isAtWar() )
			update( true );
	}
}

void cPlayer::disturbMed()
{
	if( isMeditating() ) //Meditation
	{
		this->setMeditating( false );

		if( socket_ )
			socket_->sysMessage( tr( "You loose your concentration" ) );
	}
}

P_ITEM cPlayer::getBankBox( void )
{
	P_ITEM pi = atLayer( BankBox );
	
	if ( pi )
		return pi;

	pi = new cItem;
	pi->Init();
	pi->setId( 0x9ab );
	pi->SetOwnSerial( this->serial() );
	pi->setType( 1 );
	pi->setName( tr( "%1's bank box" ).arg( name() ) );
	addItem( BankBox, pi, true, true );

	return pi;
}

int cPlayer::CountBankGold()
{
	P_ITEM pi = getBankBox(); //we want gold bankbox.
	return pi->CountItems( 0x0EED );
}

bool cPlayer::canPickUp( cItem* pi )
{
	if( !pi )
	{
		Console::instance()->log( LOG_ERROR, "cChar::canPickUp() - bad parm" );
		return false;
	}

	if( account_ && account_->isAllMove() )
		return true;

	if( ( pi->isOwnerMovable() || pi->isLockedDown() ) && !this->Owns( pi ) )	// owner movable or locked down ?
		return false;

	tile_st tile = TileCache::instance()->getTile( pi->id() );
	if( pi->isGMMovable() || ( tile.weight == 255 && !pi->isAllMovable() ) )
		return false;

	return true;
}

void cPlayer::soundEffect( UI16 soundId, bool hearAll )
{
	cUOTxSoundEffect pSoundEffect;
	pSoundEffect.setSound( soundId );
	pSoundEffect.setCoord( pos() );

	if( !hearAll )
	{
		if( socket_ )
			socket_->send( &pSoundEffect );
	}
	else 
	{
		// Send the sound to all sockets in range
		for( cUOSocket *s = cNetwork::instance()->first(); s; s = cNetwork::instance()->next() )
			if( s->player() && s->player()->inRange( this, s->player()->visualRange() ) )
				s->send( &pSoundEffect );
	}
}

void cPlayer::giveGold( Q_UINT32 amount, bool inBank )
{
	P_ITEM pCont = NULL;
    if( !inBank )
		pCont = getBackpack();
	else
		pCont = getBankBox();

	if( !pCont )
		return;

	// Begin Spawning
	Q_UINT32 total = amount;

	while( total > 0 )
	{
		P_ITEM pile = cItem::createFromScript( "eed" );
		pile->setAmount( QMIN( total, static_cast<Q_UINT32>(65535) ) );
		pCont->addItem( pile );
		total -= pile->amount();
	}

	goldSound( amount, false );
}

UINT32 cPlayer::takeGold( UINT32 amount, bool useBank )
{
	P_ITEM pPack = getBackpack();

	UINT32 dAmount = 0;

	if( pPack )
		dAmount = pPack->DeleteAmount( amount, 0xEED, 0 );

	if( ( dAmount < amount ) && useBank )
	{
		P_ITEM pBank = getBankBox();

		if( pBank )
			dAmount += pBank->DeleteAmount( (amount-dAmount), 0xEED, 0 );
	}

	goldSound( amount, false );

	return dAmount;
}

void cPlayer::message( const QString &message, UI16 color )
{
	if( socket_ )
		socket_->showSpeech( this, message, color, 3 );
}

/*!
	Queries if the character is an online player
*/
bool cPlayer::online() const
{
	return socket_ && socket_->state() == cUOSocket::InGame;
}

void cPlayer::giveNewbieItems( Q_UINT8 skill ) 
{
	const cElement *startItems = DefManager->getDefinition( WPDT_STARTITEMS, ( skill == 0xFF ) ? QString("default") : Skills->getSkillDef( skill ).lower() );

	// No Items defined
	if( !startItems )
	{
		startItems = DefManager->getDefinition( WPDT_STARTITEMS, "default" );
		if( !startItems )
			return;
	}

	applyStartItemDefinition( startItems );
}

void cPlayer::applyStartItemDefinition( const cElement *element )
{
	for( unsigned int i = 0; i < element->childCount(); ++i )
	{
		const cElement *node = element->getChild( i );

		// Apply another startitem definition
		if( node->name() == "inherit" )
		{
			const cElement* inheritNode = DefManager->getDefinition( WPDT_STARTITEMS, node->getAttribute( "id" ) );
			
			if( inheritNode )			
			{
				applyStartItemDefinition( inheritNode );
			}
		}

		// Add gold to the players backpack
		else if( node->name() == "gold" )
		{
			giveGold( node->getValue().toUInt() );
		}

		// Item related nodes
		else
		{
			P_ITEM pItem = 0;
			const QString &id = node->getAttribute( "id" );

			if( id != QString::null )
			{		
				pItem = cItem::createFromScript( id );
			}
			else
			{		
				const QString &list = node->getAttribute( "list" );
	
				if( list != QString::null )
				{
					pItem = cItem::createFromList( list );
				}
			}

			if( !pItem )
			{
				Console::instance()->log( LOG_ERROR, QString( "Invalid item tag without id or list in startitem definition '%1'" ).arg( element->getAttribute( "id" ) ) );
			}
			else
			{
				pItem->applyDefinition( node );
				pItem->setNewbie( true );

				if( node->name() == "item"  )
				{
					pItem->toBackpack( this );
				}
				else if( node->name() == "bankitem" )
				{
					getBackpack()->addItem( pItem );
				}
				else if( node->name() == "equipment" )
				{
					unsigned char layer = pItem->layer();
					pItem->setLayer( 0 );

					if( !layer )
					{
						tile_st tile = TileCache::instance()->getTile( pItem->id() );
						layer = tile.layer;
					}

					if( layer )
					{
						addItem( static_cast<cBaseChar::enLayer>( layer ), pItem );
					}
					else
					{
						Console::instance()->log( LOG_ERROR, QString( "Trying to equip invalid item (%1) in startitem definition '%2'" ).arg( pItem->id(), 0, 16 ).arg( element->getAttribute( "id" ) ) );
					}
				}
				else
				{
					pItem->remove();
					Console::instance()->log( LOG_ERROR, QString( "Unrecognized startitem tag '%1' in definition '%2'.").arg( node->name(), element->getAttribute( "id" ) ) );
				}
			}
		}
	}
}

bool cPlayer::checkSkill( UI16 skill, SI32 min, SI32 max, bool advance )
{
	if( isDead() )
	{
		if( socket_ )
			socket_->clilocMessage( 0x7A12C ); // You cannot use skills while dead.
		return false;
	}

	bool success = cBaseChar::checkSkill( skill, min, max, advance );

	// We can only advance when doing things which aren't below our ability
	if( skillValue( skill ) < max )
	{
		if( advance && Skills->advanceSkill( this, skill, min, max, success ) )
		{
			if( socket_ )
				socket_->sendSkill( skill );
		}
	}

	return success;
}

void cPlayer::addPet( P_NPC pPet, bool noOwnerChange )
{
	if( !pPet )
		return;

	// It may be the follower of someone else already, so 
	// check that...
	if( pPet->owner() && pPet->owner() != this )
		pPet->owner()->removePet( pPet, true );

	pPet->setOwner( this, true );
	pPet->setTamed( true );

	// Check if it already is our follower
	CharContainer::iterator it = std::find(pets_.begin(), pets_.end(), pPet);
	if ( it != pets_.end() )
		return;

	pets_.push_back( pPet );
}

void cPlayer::removePet( P_NPC pPet, bool noOwnerChange )
{
	if( !pPet )
		return;

	CharContainer::iterator it = std::find(pets_.begin(), pets_.end(), pPet);
	if ( it != pets_.end() )
		pets_.erase(it);

	if( !noOwnerChange )
	{
		pPet->setOwner( NULL, true );
		pPet->setTamed( false );
	}
}

bool cPlayer::onPickup( P_ITEM pItem )
{
	bool result = false;
	
	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&O&", PyGetCharObject, this, PyGetItemObject, pItem );

		result = cPythonScript::callChainedEventHandler( EVENT_PICKUP, scriptChain, args );

		Py_DECREF( args );
	}

	return result;
}

bool cPlayer::onLogin( void )
{
	cPythonScript *global = ScriptManager::instance()->getGlobalHook( EVENT_LOGIN );
	bool result = false;
	
	if( scriptChain || global )
	{
		PyObject *args = Py_BuildValue( "(O&)", PyGetCharObject, this );

		result = cPythonScript::callChainedEventHandler( EVENT_LOGIN, scriptChain, args );

		if( !result && global )
			result = global->callEventHandler( EVENT_LOGIN, args );

		Py_DECREF( args );
	}

	return result;
}

bool cPlayer::onCastSpell( unsigned int spell )
{
	cPythonScript *global = ScriptManager::instance()->getGlobalHook( EVENT_CASTSPELL );
	bool result = false;
	
	if( scriptChain || global )
	{
		PyObject *args = Py_BuildValue( "O&i", PyGetCharObject, this, spell );

		result = cPythonScript::callChainedEventHandler( EVENT_CASTSPELL, scriptChain, args );

		if( !result && global )
			result = global->callEventHandler( EVENT_CASTSPELL, args );

		Py_DECREF( args );
	}

	return result;
}

bool cPlayer::onLogout( void )
{
	cPythonScript *global = ScriptManager::instance()->getGlobalHook( EVENT_LOGOUT );
	bool result = false;
	
	if( scriptChain || global )
	{
		PyObject *args = Py_BuildValue( "(O&)", PyGetCharObject, this );

		result = cPythonScript::callChainedEventHandler( EVENT_LOGOUT, scriptChain, args );

		if( !result && global )
			result = global->callEventHandler( EVENT_LOGOUT, args );

		Py_DECREF( args );
	}

	return result;
}

// The character wants help
bool cPlayer::onHelp( void )
{
	cPythonScript *global = ScriptManager::instance()->getGlobalHook( EVENT_HELP );
	bool result = false;
	
	if( scriptChain || global )
	{
		PyObject *args = Py_BuildValue( "(O&)", PyGetCharObject, this );

		result = cPythonScript::callChainedEventHandler( EVENT_HELP, scriptChain, args );

		if( !result && global )
			result = global->callEventHandler( EVENT_HELP, args );

		Py_DECREF( args );
	}

	return result;
}

// The character wants to chat
bool cPlayer::onChat( void )
{
	cPythonScript *global = ScriptManager::instance()->getGlobalHook( EVENT_CHAT );
	bool result = false;
	
	if( scriptChain || global )
	{
		PyObject *args = Py_BuildValue( "(O&)", PyGetCharObject, this );

		result = cPythonScript::callChainedEventHandler( EVENT_CHAT, scriptChain, args );

		if( !result && global )
			result = global->callEventHandler( EVENT_CHAT, args );

		Py_DECREF( args );
	}

	return result;
}

bool cPlayer::onShowContext( cUObject *object )
{
	bool result = false;
	
	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&O&", PyGetCharObject, this, PyGetObjectObject, object );
		result = cPythonScript::callChainedEventHandler( EVENT_SHOWCONTEXTMENU, scriptChain, args );
		Py_DECREF( args );
	}

	return result;
}

bool cPlayer::onUse( P_ITEM pItem )
{
	bool result = false;
	
	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&O&", PyGetCharObject, this, PyGetItemObject, pItem );
		result = cPythonScript::callChainedEventHandler( EVENT_USE, scriptChain, args );
		Py_DECREF( args );
	}

	return result;
}

void cPlayer::processNode( const cElement *Tag )
{
	return;
}

QPtrList< cMakeSection > cPlayer::lastSelections( cMakeMenu* basemenu )
{ 
	QMap< cMakeMenu*, QPtrList< cMakeSection > >::iterator it = lastSelections_.find( basemenu );
	if( it != lastSelections_.end() )
		return it.data();
	else
		return QPtrList< cMakeSection >();
}

cMakeSection* cPlayer::lastSection( cMakeMenu* basemenu )
{
	QMap< cMakeMenu*, QPtrList< cMakeSection > >::iterator it = lastSelections_.find( basemenu );
	QPtrList< cMakeSection > lastsections_;
	if( it != lastSelections_.end() )
		 lastsections_ = it.data();
	else 
		return 0;

	if( lastsections_.count() > 0 )
		return lastsections_.at(0);
	else
		return 0;
}

void cPlayer::setLastSection( cMakeMenu* basemenu, cMakeSection* data )
{
	QMap< cMakeMenu*, QPtrList< cMakeSection > >::iterator mit = lastSelections_.find( basemenu );
	QPtrList< cMakeSection > lastsections_;
	//		lastsections_.setAutoDelete( true ); NEVER DELETE THE SECTIONS :) THEY ARE DELETED WITH THEIR MAKEMENU PARENTS
	if( mit != lastSelections_.end() )
		lastsections_ = mit.data();
	else
	{
		lastsections_.append( data );
		lastSelections_.insert( basemenu, lastsections_ );
		return;
	}
	
	QPtrListIterator< cMakeSection > it( lastsections_ );
	while( it.current() )
	{
		if( data == it.current() )
			return;
		++it;
	}
	lastsections_.prepend( data );
	while( lastsections_.count() > 10 )
		lastsections_.removeLast();
	
	mit.data() = lastsections_;
	return;
}

void cPlayer::clearLastSelections( void )
{
	lastSelections_.clear();
}

void cPlayer::setStamina( INT16 data, bool notify /* = true */ )
{
	bool update = false;
	if ( data != stamina() && notify )
		update = true;
	cBaseChar::setStamina( data );
	if ( update && socket() )
		socket()->updateStamina( this );

}

// Simple setting and getting of properties for scripts and the set command.
stError *cPlayer::setProperty( const QString &name, const cVariant &value )
{
	changed( TOOLTIP );
	changed_ = true;
	if( name == "account" )
	{
		setAccount( Accounts::instance()->getRecord( value.toString() ) );
		return 0;
	}
	else SET_INT_PROPERTY( "logouttime", logoutTime_ )
	else if( name == "muted" )
	{
		setMuted( value.toInt() );
		return 0;
	}
	else SET_INT_PROPERTY( "mutetime", muteTime_ )
	else SET_INT_PROPERTY( "lightbonus", fixedLightLevel_ )
	else if( name == "inputmode" )
	{
		inputMode_ = (enInputMode)value.toInt();
		return 0;
	}
	else SET_INT_PROPERTY( "inputitem", inputItem_ )
	else SET_INT_PROPERTY( "objectdelay", objectDelay_ )
	else SET_INT_PROPERTY( "visrange", visualRange_ )
	else SET_STR_PROPERTY( "profile", profile_ )

	// skill.
	else if( name.left( 6 ) == "skill." )
	{
		QString skill = name.right( name.length() - 6 );
		INT16 skillId = Skills->findSkillByDef( skill );

		if( skillId != -1 )
		{
			setSkillValue( skillId, value.toInt() );
			if( socket_ )
				socket_->sendSkill( skillId );
			return 0;
		}
	}
	
	return cBaseChar::setProperty( name, value );
}

stError *cPlayer::getProperty( const QString &name, cVariant &value ) const
{
	GET_PROPERTY( "account", ( account_ != 0 ) ? account_->login() : QString( "" ) )
	else GET_PROPERTY( "logouttime", (int)logoutTime_ )
	else GET_PROPERTY( "muted", isMuted() )
	else GET_PROPERTY( "mutetime", (int)muteTime_ )
	else GET_PROPERTY( "npc", false )
	else GET_PROPERTY( "lightbonus", fixedLightLevel_ )
	else GET_PROPERTY( "inputmode", inputMode_ )
	else GET_PROPERTY( "inputitem", FindItemBySerial( inputItem_ ) )
	else GET_PROPERTY( "objectdelay", (int)objectDelay_ )
	else GET_PROPERTY( "visrange", visualRange_ )
	else GET_PROPERTY( "profile", profile_ )

	return cBaseChar::getProperty( name, value );
}

void cPlayer::awardFame( short amount )
{
	int nCurFame, nChange=0;
	bool gain = false;

	setFame( QMIN( 10000, fame() ) );

	nCurFame = fame();

	// We already have more than that.
	if( nCurFame > amount )
		return;

	// Loose Fame when we died
	if( isDead() )
	{
		// Fame / 25 is our loss
		nChange = nCurFame / 25;
		setFame( QMAX( 0, nCurFame - nChange ) );
		setDeaths( deaths() + 1 );
		gain = false;
	}
	else
	{
		nChange = ( amount - nCurFame ) / 75;
		setFame( nCurFame+nChange );
		gain = true;
	}

	// Nothing changed or we can't recieve the message
	if( !nChange || !socket() )
		return;

	uint message;

	if( nChange <= 25 )
	{
		if( gain )
			message = 1019051; //"You have gained a little fame."
		else
			message = 1019055; //"You have lost a little fame."
	}
	else if( nChange <= 75 )
	{
		if( gain )
			message = 1019052; //"You have gained some fame."
		else
			message = 1019056; //"You have lost some fame."
	}
	else if( nChange <= 100 )
	{
		if( gain )
			message = 1019053; // You have gained a good amount of fame.
		else
			message = 1019057; // You have lost a good amount of fame.
	}
	else if( nChange > 100 )
	{
		if( gain )
			message = 1019054; //"You have gained alot of fame."
		else
			message = 1019058; //"You have lost alot of fame."
	}

	socket()->clilocMessage( message );
}

void cPlayer::awardKarma( P_CHAR pKilled, short amount )
{
	int nCurKarma = 0, nChange = 0, nEffect = 0;

	nCurKarma = karma();

	if( nCurKarma < amount && amount > 0 )
	{
		nChange=((amount-nCurKarma)/75);
		setKarma(nCurKarma+nChange);
		nEffect=1;
	}

	if( ( nCurKarma > amount ) && ( !pKilled ) )
	{
		nChange = ( ( nCurKarma - amount ) / 50 );
		setKarma( nCurKarma - nChange );
		nEffect = 0;
	}
	else if( ( nCurKarma > amount ) && ( pKilled->karma() > 0 ) )
	{
		nChange= ( ( nCurKarma - amount ) / 50 );
		setKarma( nCurKarma - nChange );
		nEffect=0;
	}

	// Cap at 10000 or -10000
	if( karma_ > 10000 )
		karma_ = 10000;
	else if( karma_ < -10000 )
		karma_ = -10000;

	if( !nChange || !socket() )
		return;

	UINT32 message;

	if( nChange <= 25 )
	{
		if( nEffect )
			message = 0xF8CB3; // You have gained a little karma.
		else
			message = 0xF8CB7; // You have lost a little karma.
	}
	else if( nChange <= 75 )
	{
		if( nEffect )
			message = 0xF8CB4; // You have gained some karma.
		else
			message = 0xF8CB8; // You have lost some karma.
	}
	else if( nChange <= 100 )
	{
		if( nEffect )
			message = 0xF8CB5; // You have gained a good amount of karma.
		else
			message = 0xF8CB9; // You have lost a good amount of karma.
	}
	else if( nChange > 100 )
	{
		if( nEffect )
			message = 0xF8CB6; // You have gained a lot of karma.
		else
			message = 0xF8CBA; // You have lost a lot of karma.
	}

	socket()->clilocMessage( message );
}

void cPlayer::log( eLogLevel loglevel, const QString &string )
{
	Log::instance()->print( loglevel, socket_, string );
}

void cPlayer::log( const QString &string )
{
	log( LOG_NOTICE, string );
}
