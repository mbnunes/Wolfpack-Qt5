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


cPlayer::cPlayer()
{
	account_			= NULL;
	logoutTime_			= 0;
	clientIdleTime_		= 0;
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
}

static cUObject* productCreator()
{
	return new cPlayer;
}

void cPlayer::buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions )
{
	cBaseChar::buildSqlString( fields, tables, conditions );
	fields.push_back( "players.account,players.additionalflags,players.visualrange" );
	fields.push_back( "players.profile,players.fixedlight );
	tables.push_back( "players" );
	conditions.push_back( "uobjectmap.serial = players.serial" );
}

static void characterRegisterAfterLoading( P_CHAR pc );

void cPlayer::load( char **result, UINT16 &offset )
{
	cBaseChar::load( result, offset );

	setAccount( Accounts::instance()->getRecord( result[offset++] ) );
	additionalFlags_ = atoi( result[offset++] );
	visualRange_ = atoi( result[offset++] );
	profile_ = atoi( result[offset++] );
	fixedlight_ = atoi( result[offset++] );

	characterRegisterAfterLoading( this );
	changed_ = false;
}

void cPlayer::save()
{
	if ( changed_ )
	{
		initSave;
		setTable( "characters" );
		
		addField( "serial", serial() );
		addStrField( "name", incognito() ? name() : orgname() );	
		addStrField( "title", title() );
		
		if( account_ )
			addStrField( "account", account_->login() );
		
		addField( "creationday", creationday_ );
		addField( "dir", dir_ );
		
		addField( "body", (incognito() || polymorph()) ? xid_ : id_ );
		addField( "xbody", xid_ );
		addField( "skin", incognito() ? xskin_ : skin_ );
		addField( "xskin", xskin_ );
		addField( "priv", priv );
		addField( "stablemaster", stablemaster_serial_ );
		
		addField( "allmove", priv2_);
		addField( "say", saycolor_);
		addField( "emote", emotecolor_);
		addField( "strength", st_);
		addField( "strength2", st2_);
		addField( "dexterity", dx);
		addField( "dexterity2", dx2);
		addField( "intelligence", in_);
		addField( "intelligence2", in2_);
		addField( "hitpoints", hp_);
		addField( "spawnregion", spawnregion_);
		addField( "stamina", stm_);
		addField( "mana", mn_);
		addField( "npc", npc_);
		addField( "shop", shop_);
		
		addField( "owner", owner_ ? owner_->serial() : INVALID_SERIAL );
		
		addField( "karma", karma_);
		addField( "fame", fame_);
		addField( "kills", kills_);
		addField( "deaths", deaths_);
		addField( "dead", dead_);
		addField( "fixedlight", fixedlight_);
		addField( "cantrain", cantrain_);
		addField( "def", def_);
		addField( "lodamage", lodamage_);
		addField( "hidamage", hidamage_);
		addField( "war", war_);
		addField( "npcwander", npcWander_);
		addField( "oldnpcwander", oldnpcWander_);
		addStrField( "carve", carve_);
		addField( "fx1", fx1_);
		addField( "fy1", fy1_);
		addField( "fz1", fz1_);
		addField( "fx2", fx2_);
		addField( "fy2", fy2_);
		addField( "hidden", hidden_);
		addField( "hunger", hunger_);
		addField( "npcaitype", npcaitype_);
		addField( "taming", taming_);
		unsigned int summtimer = summontimer_ - uiCurrentTime;
		addField( "summonremainingseconds", summtimer);
		addField( "poison", poison_);
		addField( "poisoned", poisoned_);
		addField( "fleeat", fleeat_);
		addField( "reattackat", reattackat_);
		addField( "split", split_);
		addField( "splitchance",	splitchnc_);
		addField( "murderrate", murderrate_);
		addStrField( "lootlist", loot_);
		addField( "food", food_);
		addStrField( "profile", profile_ );
		addField( "guarding", guarding_ ? guarding_->serial() : INVALID_SERIAL );
		addStrField( "destination", QString( "%1,%2,%3,%4" ).arg( ptarg_.x ).arg( ptarg_.y ).arg( ptarg_.z ).arg( ptarg_.map ) );
		addField( "sex", sex_ );
		
		addCondition( "serial", serial() );
		saveFields;
		
		QValueVector< stSkillValue >::const_iterator it;
		int i = 0;
		persistentBroker->lockTable("skills");
		for( it = skills.begin(); it != skills.end(); ++it )
		{
			clearFields;
			setTable( "skills" );
			addField( "serial", serial() );
			addField( "skill", i );
			addField( "value", (*it).value );
			addField( "locktype", (*it).lock );
			addField( "cap", (*it).cap );
			addCondition( "serial", serial() );
			addCondition( "skill", i );
			saveFields;
			++i;
		}
		persistentBroker->unlockTable("skills");
	}
	cBaseChar::save();
	changed_ = false;
}

bool cPlayer::del()
{	
	if( !isPersistent )
		return false; // We didn't need to delete the object

	persistentBroker->addToDeleteQueue( "characters", QString( "serial = '%1'" ).arg( serial() ) );
	persistentBroker->addToDeleteQueue( "skills", QString( "serial = '%1'" ).arg( serial() ) );
	changed( SAVE );
	return cBaseChar::del();
}

static void characterRegisterAfterLoading( P_CHAR pc )
{
	World::instance()->registerObject( pc );
	pc->setPriv2(pc->priv2() & 0xBF); // ???

	pc->setHidden( 0 );
	pc->setStealth( -1 );
	
	pc->setRegion( AllTerritories::instance()->region( pc->pos().x, pc->pos().y, pc->pos().map ) );
	
	pc->setAntispamtimer( 0 );   //LB - AntiSpam -
	pc->setAntiguardstimer( 0 ); //AntiChrist - AntiSpam for "GUARDS" call - to avoid (laggy) guards multi spawn
	
	if (pc->id() <= 0x3e1)
	{
		unsigned short k = pc->id();
		unsigned short c1 = pc->skin();
		unsigned short b = c1&0x4000;
		if ((b == 16384 && (k >=0x0190 && k<=0x03e1)) || c1==0x8000)
		{
			if (c1!=0xf000)
			{
				pc->setSkin( 0xF000 );
				pc->setXSkin( 0xF000 );
				clConsole.send("char/player: %s : %i correted problematic skin hue\n", pc->name().latin1(),pc->serial());
			}
		}
	} 
	else	// client crashing body --> delete if non player esle put only a warning on server screen
	{	// we dont want to delete that char, dont we ?
	
		if (pc->account() == 0)
		{
			cCharStuff::DeleteChar(pc);
			return;
		} 
		else
		{
			pc->setId(0x0190);
			clConsole.send("player: %s with bugged body-value detected, restored to male shape\n",pc->name().latin1());
		}
	}
	
	if( pc->stablemaster_serial() == INVALID_SERIAL )
	{ 
		MapObjects::instance()->add(pc); 
	} 
	else
		stablesp.insert(pc->stablemaster_serial(), pc->serial());
	
	UINT16 max_x = Map->mapTileWidth(pc->pos().map) * 8;
	UINT16 max_y = Map->mapTileHeight(pc->pos().map) * 8;

	// only > max_x and > max_y are invalid
	if( pc->pos().x >= max_x || pc->pos().y >= max_y )
	{
		cCharStuff::DeleteChar( pc );
		return;
	}	
}

// Update flags etc.
void cChar::update( bool excludeself )
{
	cUOTxUpdatePlayer* updatePlayer = new cUOTxUpdatePlayer();
	updatePlayer->fromChar( this );

	for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		if( socket_ == mSock && excludeself )
			continue;

		P_CHAR pChar = mSock->player();

		if( pChar && pChar->socket() && pChar->inRange( this, pChar->VisRange() ) )
		{
			updatePlayer->setHighlight( notority( pChar ) );
			mSock->send( new cUOTxUpdatePlayer( *updatePlayer ) );
		}
	}
	delete updatePlayer;
}

// Resend the char to all sockets in range
void cChar::resend( bool clean, bool excludeself )
{
	if( socket_ && !excludeself )
		socket_->resendPlayer();

	// We are stabled and therefore we arent visible to others
	if( stablemaster_serial() != INVALID_SERIAL )
		return;

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


		P_CHAR pChar = mSock->player();

		if( !pChar || !pChar->account() )
			continue;

		if( pChar->dist( this ) > pChar->VisRange() )
			continue;
        
		if( clean )
			mSock->send( &rObject );

		// We are logged out and the object can't see us.
		if( !isNpc() && !socket_  && !pChar->account()->isAllShow() )
			continue;

		// We are hidden (or dead and not visible)
		if( ( isHidden() || ( dead_ && !war_ ) ) && !pChar->isGMorCounselor() )
			continue;

		drawChar.setHighlight( notority( pChar ) );
		
		sendTooltip( mSock );
		mSock->send( &drawChar );
	}
}

void cChar::talk( const QString &message, UI16 color, UINT8 type, bool autospam, cUOSocket* socket )
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
	textSpeech->setModel( id() );
	textSpeech->setFont( 3 ); // Default Font
	textSpeech->setType( speechType );
	textSpeech->setLanguage( lang );
	textSpeech->setName( name() );
	textSpeech->setColor( color );
	textSpeech->setText( message );

	QString ghostSpeech;

	// Generate the ghost-speech *ONCE*
	if( dead_ )
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
		if( dead_ )
			if( !socket->player()->dead() && !socket->player()->isGMorCounselor() )
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
					if( dead_ )
						if( !mSock->player()->dead() && !mSock->player()->isGMorCounselor() )
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
	UINT8 guildStatus = GuildCompare( this, pChar );

	if( pChar->kills() > SrvParams->maxkills() )
		result = 0x06; // 6 = Red -> Murderer
	
	else if( guildStatus == 1 )
		result = 0x02; // 2 = Green -> Same Guild
	
	else if( guildStatus == 2 )
		result = 0x05; // 5 = Orange -> Enemy Guild

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
	changed( SAVE|TOOLTIP );
	int ele;
	int nType=0;

	if( free )
		return;

	if( dead_ || isInvul() )
		return;

	// Do this in the beginning
	dead_ = true; // Dead
	hp_ = 0; // With no hp left

	if( isPolymorphed() )
	{
		setId( xid_ );
		setPolymorphed( false );
	}

	xid_ = id();
	setXSkin( skin() );
	setMurdererSer( INVALID_SERIAL ); // Reset previous murderer serial # to zero

	QString murderer( "" );

	P_CHAR pAttacker = FindCharBySerial( attacker_ );
	if( pAttacker )
	{
		pAttacker->setTarg(INVALID_SERIAL);
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
		if( ( pc_t->swingtarg() == serial() || pc_t->targ() == serial() ) && !pc_t->free )
		{
			if( pc_t->npcaitype() == 4 )
			{
				pc_t->setSummonTimer( ( uiCurrentTime + ( MY_CLOCKS_PER_SEC * 20 ) ) );
				pc_t->setNpcWander(2);
				pc_t->setNextMoveTime();
				pc_t->talk( tr( "Thou have suffered thy punishment, scoundrel." ), -1, 0, true );
			}

			pc_t->setTarg( INVALID_SERIAL );
			pc_t->setTimeOut(0);
			pc_t->setSwingTarg( INVALID_SERIAL );

			if( pc_t->attacker() != INVALID_SERIAL )
			{
				P_CHAR pc_attacker = FindCharBySerial( pc_t->attacker() );
				pc_attacker->resetAttackFirst();
				pc_attacker->setAttacker( INVALID_SERIAL );
			}

			pc_t->setAttacker(INVALID_SERIAL);
			pc_t->resetAttackFirst();

			if( pc_t->isPlayer() && !pc_t->inGuardedArea() )
			{
				Karma( pc_t, this, ( 0 - ( karma_ ) ) );
				Fame( pc_t, fame_ );

				if( ( isPlayer() ) && ( pc_t->isPlayer() ) ) //Player vs Player
				{
					if( isInnocent() && GuildCompare( pc_t, this ) == 0 && pc_t->attackfirst() )
					{
						// Ask the victim if they want to place a bounty on the murderer (need gump to be added to
						// BountyAskViction() routine to make this a little nicer ) - no time right now
						// BountyAskVictim( this->serial(), pc_t->serial() );
						setMurdererSer( pc_t->serial() );
						pc_t->kills_++;

						// Notify the user of reputation changes
						if( pc_t->socket() )
						{
							pc_t->socket()->sysMessage( tr( "You have killed %1 innocent people." ).arg( pc_t->kills_ ) );

							if( pc_t->kills_ >= SrvParams->maxkills() )
								pc_t->socket()->sysMessage( tr( "You are now a murderer!" ) );
						}

						setcharflag( pc_t );
					}

					if( SrvParams->pvpLog() )
					{
						sprintf((char*)temp,"%s was killed by %s!\n", name().latin1(),pc_t->name().latin1());
						savelog((char*)temp,"PvP.log");
					}
				}
			}


			if( pc_t->isNpc() && pc_t->war() )
				pc_t->toggleCombat();

		}
	}

	// Now for the corpse
	P_ITEM pi_backpack = getBackpack();
	
	unmount();

#pragma note("Implement here tradewindow closing and disposal of it's cItem*")
	// Close here the trade window... we still not sure how this will work, so I took out
	//the old code
	ele = 0;

	// I would *NOT* do that but instead replace several *send* things
	// We have ->dead already so there shouldn't be any checks regarding
	// 0x192-0x193 to see if the char is dead or not
	if( xid_ == 0x0191 )
		setId( 0x0193 );	// Male or Female
	else
		setId( 0x0192 );

	playDeathSound();

	setSkin( 0x0000 ); // Undyed
	
	// Reset poison
	setPoisoned(0);
	setPoison(0);

	// Create our Corpse
	cCorpse *corpse = new cCorpse( true );

	const QDomElement *elem = DefManager->getSection( WPDT_ITEM, "2006" );
	
	if( elem && !elem->isNull() )
		corpse->applyDefinition( (*elem) );

	corpse->setName( tr( "corpse of %1" ).arg( name() ) );
	corpse->setColor( xskin() );

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
		corpse->setMore2(1);
	else if( isCriminal() )
		corpse->setMore2(2);
	else if( isMurderer() )
		corpse->setMore2(3);

    corpse->setOwner( this );

	corpse->setBodyId( xid_ );
	corpse->setMoreY( ishuman( this ) ); //is human??
	corpse->setCarve( carve() ); //store carve section
	corpse->setName2( name() );

	corpse->moveTo( pos() );

	corpse->setMore1(nType);
	corpse->setDirection( dir_ );
	corpse->startDecay();
	
	// Set the ownerserial to the player's
	corpse->SetOwnSerial(serial());
	// This is.... stupid...
	corpse->setMore4( char( SrvParams->playercorpsedecaymultiplier()&0xff ) ); // how many times longer for the player's corpse to decay

	// stores the time and the murderer's name
	corpse->setMurderer( murderer );
	corpse->setMurderTime(uiCurrentTime);

	std::vector< P_ITEM > equipment;

	// Check the Equipment and Unequip if neccesary
	ContainerContent::const_iterator iter;
	for ( iter = content_.begin(); iter != content_.end(); iter++ )
	{
		P_ITEM pi_j = iter.data();

		if( pi_j )
			equipment.push_back( pi_j );
	}

	for( std::vector< P_ITEM >::iterator it = equipment.begin(); it != equipment.end(); ++it )
	{
		P_ITEM pi_j = *it;

		if( !pi_j->newbie() )
			removeItemBonus( pi_j );

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
							Items->DeleItem( pi_k );
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

	cUOTxDeathAction dAction;
	dAction.setSerial( serial() );
	dAction.setCorpse( corpse->serial() );

	cUOTxRemoveObject rObject;
	rObject.setSerial( serial() );

	for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
		if( mSock->player() && mSock->player()->inRange( this, mSock->player()->VisRange() ) && ( mSock != socket_ ) )
		{
			if( SrvParams->showDeathAnim() )
				mSock->send( &dAction );

			mSock->send( &rObject );
		}
	
	corpse->update();

#pragma note( "Deathshroud has to be defined as 204e in the scripts" )
	P_ITEM pItem = Items->createScriptItem( "204e" );
	if( pItem )
	{
		this->addItem( cChar::OuterTorso, pItem );
		pItem->update();
	}

	resend( true );

	if( socket_ )
	{
		cUOTxCharDeath cDeath;
		socket_->send( &cDeath );
	}
}

void cPlayer::turnTo( const Coord_cl &pos )
{
	cBaseChar::turnTo( pos );

	// Send a different packet for ourself
	if( socket_ )
	{
		socket_->resendPlayer( true );

		/*cUOTxForceWalk forceWalk;
		forceWalk.setDirection( dir_ );
		socket_->send( &forceWalk );*/
	}
}

P_NPC cPlayer::unmount()
{
	P_ITEM pi = atLayer(Mount);
	if( pi && !pi->free)
	{
		P_NPC pMount = dynamic_cast<P_NPC>(FindCharBySerial( pi->morex() ));
		if( pMount )
		{
			pMount->setFx1( pi->pos().x );
			pMount->setFy1( pi->pos().y );
			pMount->setFz1( pi->pos().z );
			pMount->setId( pi->morey() );
			pMount->setNpcWander( 0 ); // Stay where you are & turn where we are facing
			pMount->setDir( dir_ );
			pMount->setSt( pi->moreb2() );
			pMount->setDex( pi->moreb3() );
			pMount->setIn( pi->moreb4() );
			pMount->setFx2( pi->att() );
			pMount->setFy2( pi->def() );
			pMount->setHp( pi->hp() );
			pMount->setFame( pi->lodamage() );
			pMount->setKarma( pi->hidamage() );
			pMount->setPoisoned( pi->poisoned() );
			pMount->setSummonTimer( pi->decaytime() );
			
			pMount->moveTo( pos() );
			pMount->resend( false );
		}
		Items->DeleItem( pi );
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

	if( pMount->owner_ == this || isGM() )
	{
		unmount();

		P_ITEM pMountItem = new cItem;
		pMountItem->Init();
		pMountItem->setId( 0x915 );
		pMountItem->setName( pMount->name() );
		pMountItem->setColor( pMount->skin() );

		if( !pMountItem )
			return;

		switch( static_cast< unsigned short >(pMount->id() & 0x00FF) )
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
		
		this->addItem( cChar::Mount, pMountItem );
		Coord_cl position = pMountItem->pos();
		position.x = pMount->fx1();
		position.y = pMount->fy1();
		position.z = pMount->fz1();
		pMountItem->setPos( position );
		
		pMountItem->setMoreX(pMount->serial());
		pMountItem->setMoreY(pMount->id());

		pMountItem->setMoreb1( pMount->npcWander() );
		pMountItem->setMoreb2( pMount->st() );
		pMountItem->setMoreb3( pMount->realDex() );
		pMountItem->setMoreb4( pMount->in() );
		pMountItem->setAtt( pMount->fx2() );
		pMountItem->setDef( pMount->fy2() );
		pMountItem->setHp( pMount->hp() );
		pMountItem->setLodamage( pMount->fame() );
		pMountItem->setHidamage( pMount->karma() );
		pMountItem->setPoisoned( pMount->poisoned() );
		if (pMount->summontimer() != 0)
			pMountItem->setDecayTime(pMount->summontimer());

		pMountItem->update();

		// if this is a gm lets tame the animal in the process
		if( isGM() )
		{
			pMount->setOwner( this );
			pMount->setNpcAIType( 0 );
		}
		
		// remove it from screen!
		pMount->removeFromView( true );
		
		pMount->setId(0);
		MapObjects::instance()->remove( pMount );
		pMount->setPos( Coord_cl(0, 0, 0) );
		
		pMount->setWar( false );
		pMount->setAttacker(INVALID_SERIAL);
	}
	else
		socket->sysMessage( tr("You dont own that creature.") );
}

bool cPlayer::isGM() const
{
	return account() && ( account()->acl() == "admin" || account()->acl() == "gm" );
} 

bool cPlayer::isCounselor() const 
{
	return account() && account()->acl() == "counselor";
}
 
bool cPlayer::isGMorCounselor() const	
{
	return account() && ( account()->acl() == "admin" || account()->acl() == "gm" || account()->acl() == "counselor" );
} 

void cPlayer::showName( cUOSocket *socket )
{
	if( !socket->player() )
		return;

	if( onSingleClick( socket->player() ) )
		return;

	QString charName = name();

	// Lord & Lady Title
	if( fame_ == 10000 )
		charName.prepend( ( id() == 0x191 ) ? tr( "Lady " ) : tr( "Lord " ) );

	// Are we squelched ?
	if( squelched() )
		charName.append( tr(" [squelched]" ) );

	// Append serial for GMs
	if( socket->player()->canSeeSerials() )
		charName.append( QString( " [0x%1]" ).arg( serial(), 4, 16 ) );

	// Append offline flag
	if( !socket_ )
		charName.append( tr(" [offline]") );

	// Invulnerability
	if( isInvul() )
		charName.append( tr(" [invul]") );

	// Frozen
	if( isFrozen() )
		charName.append( tr(" [frozen]") );

	// Guarded
	if( guardedby_.size() > 0 )
		charName.append( tr(" [guarded]") );

	// Guarding
	if( tamed() && npcaitype_ == 32 && guarding_ )
		charName.append( tr(" [guarding]") );

	// Tamed
	if( tamed() && npcaitype_ != 17 )
		charName.append( tr(" [tamed]") );

	// WarMode ?
	if( war_ )
		charName.append( tr(" [war mode]") );

	// Criminal ?
	if( crimflag() && ( kills_ < SrvParams->maxkills() ) )
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

		 changed( SAVE );
	}
}

void cPlayer::fight(P_CHAR other)
{
	// I am already fighting this character.
	if( war() && targ() == other->serial() )
		return;

	// Store the current Warmode
	bool oldwar = war_;

	this->targ_ = other->serial();
	this->unhide();
	this->disturbMed();	// Meditation
	this->attacker_ = other->serial();
	this->setWar( true );
	
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
		if( oldwar != war_ )
			update( true );
	}
}

void cPlayer::disturbMed()
{
	if( med() ) //Meditation
	{
		this->setMed( false );

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
	pi->SetOwnSerial(this->serial());
	pi->setMoreX(1);
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
		LogCritical("cChar::canPickUp() - bad parm");
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
			if( s->player() && s->player()->inRange( this, s->player()->VisRange() ) )
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
		P_ITEM pile = new cItem;
		pile->Init();
		pile->setId( 0xEED );
		pile->setAmount( QMIN( total, static_cast<Q_UINT32>(65535) ) );
		pCont->addItem( pile );
		total -= pile->amount();
	}

	if( socket_ )
		goldsfx( socket_, amount, false );
}

UINT32 cPlayer::takeGold( UINT32 amount, bool useBank )
{
	P_ITEM pPack = getBackpack();

	UINT32 dAmount;

	if( pPack )
		dAmount = pPack->DeleteAmount( amount, 0xEED, 0 );

	if( ( dAmount < amount ) && useBank )
	{
		P_ITEM pBank = getBankBox();

		if( pBank )
			dAmount += pBank->DeleteAmount( (amount-dAmount), 0xEED, 0 );
	}

	if( socket_ )
		goldsfx( socket_, dAmount, false );

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
	const QDomElement *startItems = DefManager->getSection( WPDT_STARTITEMS, ( skill == 0xFF ) ? QString("default") : Skills->getSkillDef( skill ).lower() );

	// No Items defined
	if( !startItems || startItems->isNull() )
	{
		startItems = DefManager->getSection( WPDT_STARTITEMS, "default" );
		if( !startItems || startItems->isNull() )
			return;
	}

	applyStartItemDefinition( *startItems );
}

void cPlayer::applyStartItemDefinition( const QDomElement &Tag )
{
	QDomNode childNode = Tag.firstChild();

	while( !childNode.isNull() )
	{
		QDomElement node = childNode.toElement();
		if( !node.isNull() )
		{
			if( node.nodeName() == "item" )
			{
				P_ITEM pItem = NULL;
				const QDomElement *DefSection = DefManager->getSection( WPDT_ITEM, node.attribute( "id" ) );
				if( DefSection && !DefSection->isNull() )
				{
					// books wont work without this
					pItem = Items->createScriptItem( node.attribute("id") );
				}
				else
				{
					pItem = new cItem;
					pItem->Init( true );
				}

				if( pItem )
				{
					pItem->applyDefinition( node );
					pItem->setNewbie( true ); // make it newbie

					if( pItem->id() <= 1 )
						Items->DeleItem( pItem );
					else
					{
						// Put it into the backpack
						P_ITEM backpack = getBackpack();
						if( backpack )
							backpack->addItem( pItem );
						else
							Items->DeleItem( pItem );
					}
				}
			}
			else if( node.nodeName() == "bankitem" )
			{
				P_ITEM pItem = NULL;
				const QDomElement *DefSection = DefManager->getSection( WPDT_ITEM, node.attribute( "id" ) );
				if( DefSection && !DefSection->isNull() )
				{
					// books wont work without this
					pItem = Items->createScriptItem( node.attribute("id") );
				}
				else
				{
					pItem = new cItem;
					pItem->Init( true );
				}

				if( pItem )
				{
					pItem->applyDefinition( node );
					pItem->setNewbie( true ); // make it newbie

					if( pItem->id() <= 1 )
						Items->DeleItem( pItem );
					else
					{
						// Put it into the bankbox
						P_ITEM bankbox = getBankBox();
						if( bankbox )
							bankbox->addItem( pItem );
						else
							Items->DeleItem( pItem );
					}
				}
			}
			else if( node.nodeName() == "equipment" )
			{
				P_ITEM pItem = NULL;
				const QDomElement *DefSection = DefManager->getSection( WPDT_ITEM, node.attribute( "id" ) );
				if( DefSection && !DefSection->isNull() )
				{
					// books wont work without this
					pItem = Items->createScriptItem( node.attribute("id") );
				}
				else
				{
					pItem = new cItem;
					pItem->Init( true );
				}

				if( pItem )
				{
					pItem->applyDefinition( node );
					pItem->setNewbie( true ); // make it newbie

					UINT16 mLayer = pItem->layer();
					pItem->setLayer( 0 );
					if( !mLayer )
					{
						tile_st tile = TileCache::instance()->getTile( pItem->id() );
						mLayer = tile.layer;
					}

					if( pItem->id() <= 1 || !mLayer )
						Items->DeleItem( pItem );
					else
					{
						// Put it onto the char
						this->addItem( static_cast<cChar::enLayer>( mLayer ), pItem );
						giveItemBonus( pItem );
					}
				}
			}
			else if( node.nodeName() == "gold" )
			{
				giveGold( node.text().toUInt() );
			}
			else if( node.nodeName() == "inherit" )
			{
				const QDomElement* inheritNode = DefManager->getSection( WPDT_STARTITEMS, node.attribute("id") );
				if( inheritNode && !inheritNode->isNull() )
					applyStartItemDefinition( *inheritNode );
			}
		}
		childNode = childNode.nextSibling();
	}
}

bool cPlayer::checkSkill( UI16 skill, SI32 min, SI32 max, bool advance )
{
	if( dead_ )
	{
		if( socket_ )
			socket_->clilocMessage( 0x7A12C ); // You cannot use skills while dead.
		return false;
	}

	bool success = cBaseChar::checkSkill( skill, min, max, advance );

	// We can only advance when doing things which aren't below our ability
	if( skillValue( skill ) < max )
	{
		if( advance && Skills->advanceSkill( this, skill, success ) )
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
		pPet->owner()->removeFollower( pPet, true );

	pPet->setOwnerOnly( this );

	// Check if it already is our follower
	Followers::iterator it = std::find(followers_.begin(), followers_.end(), pPet);
	if ( it != followers_.end() )
		return;

	followers_.push_back( pPet );
}

void cPlayer::removePet( P_NPC pPet, bool noOwnerChange )
{
	if( !pPet )
		return;

	Followers::iterator it = std::find(followers_.begin(), followers_.end(), pPet);
	if ( it != followers_.end() )
		followers_.erase(it);

	if( !noOwnerChange )
	{
		pPet->setOwnerOnly( NULL );
		pPet->setTamed( false );
	}
}

bool cPlayer::onPickup( P_ITEM pItem )
{
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onPickup( this, pItem ) )
			return true;

	// Try to process the hooks then
	QValueVector< WPDefaultScript* > hooks;
	QValueVector< WPDefaultScript* >::const_iterator it;

	hooks = ScriptManager->getGlobalHooks( OBJECT_CHAR, EVENT_PICKUP );
	for( it = hooks.begin(); it != hooks.end(); ++it )
		if( (*it)->onPickup( this, pItem ) )
			return true;

	return false;
}

bool cPlayer::onLogin( void )
{
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onLogin( this ) )
			return true;

	// Try to process the hooks then
	QValueVector< WPDefaultScript* > hooks;
	QValueVector< WPDefaultScript* >::const_iterator it;

	hooks = ScriptManager->getGlobalHooks( OBJECT_CHAR, EVENT_LOGIN );
	for( it = hooks.begin(); it != hooks.end(); ++it )
		(*it)->onLogin( this );

	return false;
}

bool cPlayer::onLogout( void )
{
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onLogout( this ) )
			return true;

	// Try to process the hooks then
	QValueVector< WPDefaultScript* > hooks;
	QValueVector< WPDefaultScript* >::const_iterator it;

	hooks = ScriptManager->getGlobalHooks( OBJECT_CHAR, EVENT_LOGOUT );
	for( it = hooks.begin(); it != hooks.end(); ++it )
		(*it)->onLogout( this );

	return false;
}

// The character wants help
bool cPlayer::onHelp( void )
{
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onHelp( this ) )
			return true;

	// Try to process the hooks then
	QValueVector< WPDefaultScript* > hooks;
	QValueVector< WPDefaultScript* >::const_iterator it;

	hooks = ScriptManager->getGlobalHooks( OBJECT_CHAR, EVENT_HELP );
	for( it = hooks.begin(); it != hooks.end(); ++it )
		if( (*it)->onHelp( this ) )
			return true;

	return false;
}

// The character wants to chat
bool cPlayer::onChat( void )
{
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onChat( this ) )
			return true;

	// Try to process the hooks then
	QValueVector< WPDefaultScript* > hooks;
	QValueVector< WPDefaultScript* >::const_iterator it;

	hooks = ScriptManager->getGlobalHooks( OBJECT_CHAR, EVENT_CHAT );
	for( it = hooks.begin(); it != hooks.end(); ++it )
		if( (*it)->onChat( this ) )
			return true;

	return false;
}

bool cPlayer::onShowContext( cUObject *object )
{
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onShowContextMenu( (P_CHAR)this, object ) )
			return true;

	return false;
}

bool cPlayer::onShowTooltip( P_CHAR sender, cUOTxTooltipList* tooltip )
{

	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onShowToolTip( sender, this, tooltip  ) )
			return true;

	// Try to process the hooks then
	QValueVector< WPDefaultScript* > hooks;
	QValueVector< WPDefaultScript* >::const_iterator it;

	hooks = ScriptManager->getGlobalHooks( OBJECT_CHAR, EVENT_SHOWTOOLTIP );
	for( it = hooks.begin(); it != hooks.end(); ++it )
		if( (*it)->onShowToolTip( sender, this, tooltip ) ) 
			return true;

	return false;
}

void cPlayer::processNode( const QDomElement &Tag )
{
	return;
}

QPtrList< cMakeSection > cPlayer::lastSelections( cMakeMenu* basemenu )
{ 
	QMap< cMakeMenu*, QPtrList< cMakeSection > >::iterator it = lastselections_.find( basemenu );
	if( it != lastselections_.end() )
		return it.data();
	else
		return QPtrList< cMakeSection >();
}

cMakeSection* cPlayer::lastSection( cMakeMenu* basemenu )
{
	QMap< cMakeMenu*, QPtrList< cMakeSection > >::iterator it = lastselections_.find( basemenu );
	QPtrList< cMakeSection > lastsections_;
	if( it != lastselections_.end() )
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
	QMap< cMakeMenu*, QPtrList< cMakeSection > >::iterator mit = lastselections_.find( basemenu );
	QPtrList< cMakeSection > lastsections_;
	//		lastsections_.setAutoDelete( true ); NEVER DELETE THE SECTIONS :) THEY ARE DELETED WITH THEIR MAKEMENU PARENTS
	if( mit != lastselections_.end() )
		lastsections_ = mit.data();
	else
	{
		lastsections_.append( data );
		lastselections_.insert( basemenu, lastsections_ );
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
	lastselections_.clear();
}

// Simple setting and getting of properties for scripts and the set command.
stError *cPlayer::setProperty( const QString &name, const cVariant &value )
{
	changed( SAVE|TOOLTIP );
	SET_STR_PROPERTY( "orgname", orgname_ )
	else SET_STR_PROPERTY( "title", title_ )
	else if( name == "account" )
	{
		setAccount( Accounts::instance()->getRecord( value.toString() ) );
		npc_ = ( account() == 0 );
		return 0;
	}
	else SET_INT_PROPERTY( "incognito", incognito_ )
	else SET_INT_PROPERTY( "polymorph", polymorph_ )
	else if( name == "haircolor" )
	{
		bool ok;
		INT32 data = value.toInt( &ok );
		if( !ok )
			PROPERTY_ERROR( -2, "Integer expected" )
		setHairColor( data );
		return 0;
	}
	else if( name == "hairstyle" )
	{
		bool ok;
		INT32 data = value.toInt( &ok );
		if( !ok )
			PROPERTY_ERROR( -2, "Integer expected" )
		setHairStyle( data );
		return 0;
	}
	else if( name == "beardcolor" )
	{
		bool ok;
		INT32 data = value.toInt( &ok );
		if( !ok )
			PROPERTY_ERROR( -2, "Integer expected" )
		setBeardColor( data );
		return 0;
	}
	else if( name == "beardstyle" )
	{
		bool ok;
		INT32 data = value.toInt( &ok );
		if( !ok )
			PROPERTY_ERROR( -2, "Integer expected" )
		setBeardStyle( data );
		return 0;
	}
	else SET_INT_PROPERTY( "skin", skin_ )
	else SET_INT_PROPERTY( "xskin", xskin_ )
	else SET_INT_PROPERTY( "creationday", creationday_ )
	else SET_INT_PROPERTY( "stealth", stealth_ )
	else SET_INT_PROPERTY( "running", running_ )
	else SET_INT_PROPERTY( "logout", logout_ )
	else SET_INT_PROPERTY( "clientidletime", clientidletime_ )
	else SET_INT_PROPERTY( "swingtarget", swingtarg_ )
	else SET_INT_PROPERTY( "tamed", tamed_ )
	else SET_INT_PROPERTY( "antispamtimer", antispamtimer_ )
	else SET_INT_PROPERTY( "antiguardstimer", antiguardstimer_ )
	else SET_CHAR_PROPERTY( "guarding", guarding_ )
	else SET_STR_PROPERTY( "carve", carve_ )
	else SET_INT_PROPERTY( "murderer", murdererSer_ )
	else SET_STR_PROPERTY( "spawnregion", spawnregion_ )
	else SET_INT_PROPERTY( "stablemaster", stablemaster_serial_ )
	else SET_INT_PROPERTY( "casting", casting_ )
	else SET_INT_PROPERTY( "hidden", hidden_ )
	else SET_INT_PROPERTY( "attackfirst", attackfirst_ )
	else SET_INT_PROPERTY( "hunger", hunger_ )
	else SET_INT_PROPERTY( "hungertime", hungertime_ )
	else SET_INT_PROPERTY( "npcaitype", npcaitype_ )
	else SET_INT_PROPERTY( "poison", poison_ )
	else SET_INT_PROPERTY( "poisoned", poisoned_ )
	else SET_INT_PROPERTY( "poisontime", poisontime_ )
	else SET_INT_PROPERTY( "poisonwearofftime", poisonwearofftime_ )
	else SET_INT_PROPERTY( "fleeat", fleeat_ )
	else SET_INT_PROPERTY( "reattackat", reattackat_ )
	else SET_INT_PROPERTY( "split", split_ )
	else SET_INT_PROPERTY( "splitchance", splitchnc_ )
	else SET_INT_PROPERTY( "ra", ra_ )
	else SET_INT_PROPERTY( "trainer", trainer_ )
	else SET_INT_PROPERTY( "trainingplayerin", trainingplayerin_ )
	else SET_INT_PROPERTY( "cantrain", cantrain_ )
/*	else if( name == "guildstone" )
	{
		P_ITEM pItem = value.toItem();
		// Remove from Current Guild
		if( !pItem )
		{
			cGuildStone *pGuild = dynamic_cast< cGuildStone* >( FindItemBySerial( guildstone_ ) );
			if( pGuild )
				pGuild->removeMember( this );
			guildstone_ = INVALID_SERIAL;
			return 0;
		}
		else if( pItem->serial() != guildstone_ )
		{
			cGuildStone *pGuild = dynamic_cast< cGuildStone* >( FindItemBySerial( guildstone_ ) );
			if( pGuild )
				pGuild->removeMember( this );
			guildstone_ = pItem->serial();
			return 0;
		}
		else
			return 0;
	}*/
	else SET_INT_PROPERTY( "flag", flag_ )
	else SET_INT_PROPERTY( "murderrate", murderrate_ )
	else SET_INT_PROPERTY( "crimflag", crimflag_ )
	else SET_INT_PROPERTY( "squelched", squelched_ )
	else SET_INT_PROPERTY( "mutetime", mutetime_ )
	else SET_INT_PROPERTY( "meditating", med_ )
	else SET_INT_PROPERTY( "weight", weight_ )
	else if( name == "stones" )
	{
		weight_ = value.toInt() * 10;
		return 0;
	}
	else SET_STR_PROPERTY( "lootlist", loot_ )
	else SET_INT_PROPERTY( "saycolor", saycolor_ )
	else SET_INT_PROPERTY( "emotecolor", emotecolor_ )
	else SET_INT_PROPERTY( "strength", st_ )
	else SET_INT_PROPERTY( "dexterity", dx )
	else SET_INT_PROPERTY( "intelligence", in_ )
	else SET_INT_PROPERTY( "strength2", st2_ )
	else SET_INT_PROPERTY( "dexterity2", dx2 )
	else SET_INT_PROPERTY( "intelligence2", in2_ )
	else SET_INT_PROPERTY( "direction", dir_ )
	else SET_INT_PROPERTY( "xid", xid_ )
	else SET_INT_PROPERTY( "priv", priv )
	else SET_INT_PROPERTY( "priv2", priv2_ )
	else SET_INT_PROPERTY( "health", hp_ )
	else SET_INT_PROPERTY( "stamina", stm_ )
	else SET_INT_PROPERTY( "mana", mn_ )
	else SET_INT_PROPERTY( "hidamage", hidamage_ )
	else SET_INT_PROPERTY( "lodamage", lodamage_ )
	else SET_INT_PROPERTY( "npc", npc_ )
	else SET_INT_PROPERTY( "shop", shop_ )
	else SET_INT_PROPERTY( "karma", karma_ )
	else SET_INT_PROPERTY( "fame", fame_ )
	else SET_INT_PROPERTY( "kills", kills_ )
	else SET_INT_PROPERTY( "deaths", deaths_ )
	else SET_INT_PROPERTY( "dead", dead_ )
	else SET_INT_PROPERTY( "lightbonus", fixedlight_ )
	else SET_INT_PROPERTY( "defense", def_ )
	else SET_INT_PROPERTY( "war", war_ )
	else SET_INT_PROPERTY( "target", targ_ )
	else SET_INT_PROPERTY( "nextswing", timeout_ )
	else SET_INT_PROPERTY( "regenhealth", regen_ )
	else SET_INT_PROPERTY( "regenstamina", regen2_ )
	else SET_INT_PROPERTY( "regenmana", regen3_ )
	else if( name == "inputmode" )
	{
		inputmode_ = (enInputMode)value.toInt();
		return 0;
	}
	else SET_INT_PROPERTY( "inputitem", inputitem_ )
	else SET_INT_PROPERTY( "attacker", attacker_ )
	else SET_INT_PROPERTY( "npcmovetime", npcmovetime_ )
	else SET_INT_PROPERTY( "npcwander", npcWander_ )
	else SET_INT_PROPERTY( "oldnpcwander", oldnpcWander_ )
	else SET_INT_PROPERTY( "following", ftarg_ )
	else if( name == "destination" )
	{
		ptarg_ = value.toCoord();
		return 0;
	}
	
	SET_INT_PROPERTY( "fx1", fx1_ )
	else SET_INT_PROPERTY( "fx2", fx2_ )
	else SET_INT_PROPERTY( "fy1", fy1_ )
	else SET_INT_PROPERTY( "fy2", fy2_ )
	else SET_INT_PROPERTY( "fz1", fz1_ )
	else SET_INT_PROPERTY( "skilldelay", skilldelay_ )
	else SET_INT_PROPERTY( "objectdelay", objectdelay_ )
	else SET_INT_PROPERTY( "totame", taming_ )
	else SET_INT_PROPERTY( "summontimer", summontimer_) 
	else SET_INT_PROPERTY( "visrange", VisRange_ )
	else SET_INT_PROPERTY( "food", food_ )
	else SET_CHAR_PROPERTY( "owner", owner_ )
	else SET_STR_PROPERTY( "profile", profile_ )
	else SET_INT_PROPERTY( "sex", sex_ )
	else SET_INT_PROPERTY( "id", id_ )

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
	GET_PROPERTY( "orgname", orgname_ )
	GET_PROPERTY( "title", title_ )
	GET_PROPERTY( "account", ( account_ != 0 ) ? account_->login() : QString( "" ) )
	GET_PROPERTY( "incognito", incognito_ )
	GET_PROPERTY( "polymorph", polymorph_ )
	GET_PROPERTY( "skin", skin_ )
	GET_PROPERTY( "xskin", xskin_ )
	GET_PROPERTY( "creationday", (int)creationday_ )
	GET_PROPERTY( "stealth", stealth_ )
	GET_PROPERTY( "running", (int)running_ )
	GET_PROPERTY( "logout", (int)logout_ )
	GET_PROPERTY( "clientidletime", (int)clientidletime_ )
	GET_PROPERTY( "swingtarget", FindCharBySerial( swingtarg_ ) )
	GET_PROPERTY( "tamed", tamed_ )
	GET_PROPERTY( "antispamtimer", (int)antispamtimer_ )
	GET_PROPERTY( "antiguardstimer", (int)antiguardstimer_ )
	GET_PROPERTY( "guarding", guarding_ )
	GET_PROPERTY( "carve", carve_ )
	GET_PROPERTY( "murderer", FindCharBySerial( murdererSer_ ) )
	GET_PROPERTY( "spawnregion", spawnregion_ )
	GET_PROPERTY( "stablemaster", FindCharBySerial( stablemaster_serial_ ) )
	GET_PROPERTY( "casting", casting_ )
	GET_PROPERTY( "hidden", hidden_ )
	GET_PROPERTY( "attackfirst", attackfirst_ )
	GET_PROPERTY( "hunger", hunger_ )
	GET_PROPERTY( "hungertime", (int)hungertime_ )
	GET_PROPERTY( "npcaitype", npcaitype_ )
	GET_PROPERTY( "poison", poison_ )
	GET_PROPERTY( "poisoned", (int)poisoned_ )
	GET_PROPERTY( "poisontime", (int)poisontime_ )
	GET_PROPERTY( "poisonwearofftime", (int)poisonwearofftime_ )
	GET_PROPERTY( "fleeat", fleeat_ )
	GET_PROPERTY( "reattackat", reattackat_ )
	GET_PROPERTY( "split", split_ )
	GET_PROPERTY( "splitchance", splitchnc_ )
	GET_PROPERTY( "ra", ra_ )
	GET_PROPERTY( "trainer", FindCharBySerial( trainer_ ) )
	GET_PROPERTY( "trainingplayerin", trainingplayerin_ )
	GET_PROPERTY( "cantrain", cantrain_ )
	GET_PROPERTY( "flag", flag_ )
	GET_PROPERTY( "murderrate", (int)murderrate_ )
	GET_PROPERTY( "crimflag", crimflag_ )
	GET_PROPERTY( "squelched", squelched_ )
	GET_PROPERTY( "mutetime", (int)mutetime_ )
	GET_PROPERTY( "meditating", med_ )
	GET_PROPERTY( "weight", weight_ )
	GET_PROPERTY( "stones", weight_ / 10 )
	GET_PROPERTY( "lootlist", loot_ )
	GET_PROPERTY( "saycolor", saycolor_ )
	GET_PROPERTY( "emotecolor", emotecolor_ )
	GET_PROPERTY( "strength", st_ )
	GET_PROPERTY( "dexterity", dx )
	GET_PROPERTY( "intelligence", in_ )
	GET_PROPERTY( "strength2", st2_ )
	GET_PROPERTY( "dexterity2", dx2 )
	GET_PROPERTY( "intelligence2", in2_ )
	GET_PROPERTY( "direction", dir_ )
	GET_PROPERTY( "xid", xid_ )
	GET_PROPERTY( "priv", priv )
	GET_PROPERTY( "priv2", priv2_ )
	GET_PROPERTY( "health", hp_ )
	GET_PROPERTY( "stamina", stm_ )
	GET_PROPERTY( "mana", mn_ )
	GET_PROPERTY( "hidamage", hidamage_ )
	GET_PROPERTY( "lodamage", lodamage_ )
	GET_PROPERTY( "npc", npc_ )
	GET_PROPERTY( "shop", shop_ )
	GET_PROPERTY( "karma", karma_ )
	GET_PROPERTY( "fame", fame_ )
	GET_PROPERTY( "kills", (int)kills_ )
	GET_PROPERTY( "deaths", (int)deaths_ )
	GET_PROPERTY( "dead", dead_ )
	GET_PROPERTY( "lightbonus", fixedlight_ )
	GET_PROPERTY( "defense", (int)def_ )
	GET_PROPERTY( "war", war_ )
	GET_PROPERTY( "target", FindCharBySerial( targ_ ) )
	GET_PROPERTY( "nextswing", (int)timeout_ )
	GET_PROPERTY( "regenhealth", (int)regen_ )
	GET_PROPERTY( "regenstamina", (int)regen2_ )
	GET_PROPERTY( "regenmana", (int)regen3_ )
	GET_PROPERTY( "inputmode", inputmode_ )
	GET_PROPERTY( "inputitem", FindItemBySerial( inputitem_ ) )
	GET_PROPERTY( "attacker", FindCharBySerial( attacker_ ) )
	GET_PROPERTY( "npcmovetime", (int)npcmovetime_ )
	GET_PROPERTY( "npcwander", npcWander_ )
	GET_PROPERTY( "oldnpcwander", oldnpcWander_ )
	GET_PROPERTY( "following", FindCharBySerial( ftarg_ ) )
	GET_PROPERTY( "destination", ptarg_ )
	GET_PROPERTY( "fx1", fx1_ )
	GET_PROPERTY( "fx2", fx2_ )
	GET_PROPERTY( "fy1", fy1_ )
	GET_PROPERTY( "fy2", fy2_ )
	GET_PROPERTY( "fz1", fz1_ )
	GET_PROPERTY( "region", ( region_ != 0 ) ? region_->name() : QString( "" ) )
	GET_PROPERTY( "skilldelay", (int)skilldelay_ )
	GET_PROPERTY( "objectdelay", (int)objectdelay_ )
	GET_PROPERTY( "totame", taming_ )
	GET_PROPERTY( "summontimer", (int)summontimer_) 
	GET_PROPERTY( "visrange", VisRange_ )
	GET_PROPERTY( "food", (int)food_ )
	GET_PROPERTY( "owner", owner_ )
	GET_PROPERTY( "profile", profile_ )
	GET_PROPERTY( "sex", sex_ )
	GET_PROPERTY( "id", id_ )

	// skill.
	if( name.left( 6 ) == "skill." )
	{
		QString skill = name.right( name.length() - 6 );
		INT16 skillId = Skills->findSkillByDef( skill );

		if( skillId != -1 )
		{
			value = cVariant( this->skillValue( skillId ) );
			return 0;
		}
	}

	return cBaseChar::getProperty( name, value );
}

