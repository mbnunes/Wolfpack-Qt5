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

// wolfpack includes
#include "npc.h"
#include "network/uotxpackets.h"
#include "network/uosocket.h"
#include "network.h"
#include "player.h"

cNPC::cNPC()
{
	minDamage_			= 0;
	maxDamage_			= 0;
	nextMsgTime_		= 0;
	nextGuardCallTime_	= 0;
	nextBeggingTime_	= 0;
	nextMoveTime_		= 0;
	tamingMinSkill_		= 0;
	summonTime_			= 0;
	additionalFlags_	= 0;
	owner_				= NULL;
	carve_				= (char*)0;
	spawnregion_			= (char*)0;
	stablemasterSerial_	= INVALID_SERIAL;
	loot_				= (char*)0;
    guarding_			= NULL;
}

cNPC::cNPC(const cNPC& right)
{
}

cNPC::~cNPC()
{
}

cNPC& cNPC::operator=(const cNPC& right)
{
}

static cUObject* productCreator()
{
	return new cNPC;
}

void cNPC::buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions )
{
	cBaseChar::buildSqlString( fields, tables, conditions );
	fields.push_back( "npcs.mindamage,npcs.maxdamage,npcs.tamingminskill" );
	fields.push_back( "npcs.summontime,npcs.additionalflags,npcs.owner" );
	fields.push_back( "npcs.carve,npcs.spawnregion,npcs.stablemaster" );
	fields.push_back( "npcs.lootlist,npcs.guarding" );
	tables.push_back( "npcs" );
	conditions.push_back( "uobjectmap.serial = npcs.serial" );
}

static void npcRegisterAfterLoading( P_CHAR pc );

void cNPC::load( char **result, UINT16 &offset )
{
	cBaseChar::load( result, offset );
	SERIAL ser;

	minDamage_ = atoi( result[offset++] );
	maxDamage_ = atoi( result[offset++] );
	tamingMinSkill_ = atoi( result[offset++] );
	summonTime_ = atoi( result[offset++] ) + uiCurrentTime;
	if( summonTime_ )
		summonTime_ += uiCurrentTime;
	additionalFlags_ = atoi( result[offset++] );
	ser = atoi( result[offset++] );
	owner_ = dynamic_cast<P_PLAYER>(FindCharBySerial( ser ));
	carve_ = result[offset++];
	spawnregion_ = result[offset++];
	stablemasterSerial_ = atoi( result[offset++] );
	lootList_ = result[offset++];
	ser = atoi( result[offset++] );
	guarding_ = dynamic_cast<P_PLAYER>(FindCharBySerial( ser ));

	npcRegisterAfterLoading( this );
	changed_ = false;
}

void cNPC::save()
{
	if ( changed_ )
	{
		initSave;
		setTable( "npcs" );
		
		addField( "serial", serial() );
		addField( "mindamage", minDamage_);
		addField( "maxdamage", maxDamage_);
		addField( "tamingminskill", tamingMinSkill_);
		addField( "summontime", summonTime_ - uiCurrentTime );
		addField( "additionalflags", additionalFlags_ );
		addField( "owner", owner_ ? owner_->serial() : INVALID_SERIAL );
		addStrField( "carve", carve_);
		addStrField( "spawnregion", spawnregion_);
		addField( "stablemaster", stablemasterSerial_ );
		addStrField( "lootlist", lootList_);
		addField( "guarding", guarding_ ? guarding_->serial() : INVALID_SERIAL );
		
		addCondition( "serial", serial() );
		saveFields;
	}
	cBaseChar::save();
	changed_ = false;
}

bool cNPC::del()
{	
	if( !isPersistent )
		return false; // We didn't need to delete the object

	persistentBroker->addToDeleteQueue( "npcs", QString( "serial = '%1'" ).arg( serial() ) );
	changed( SAVE );
	return cBaseChar::del();
}

static void npcRegisterAfterLoading( P_CHAR pc )
{
	if( pc->stablemasterSerial() == INVALID_SERIAL )
	{ 
		MapObjects::instance()->add(pc); 
	} 
	else
		stablesp.insert(pc->stablemasterSerial(), pc->serial());
}

// Update flags etc.
void cNPC::update( bool excludeself )
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
}

// Resend the char to all sockets in range
void cNPC::resend( bool clean, bool excludeself )
{
	// We are stabled and therefore we arent visible to others
	if( stablemasterSerial() != INVALID_SERIAL )
		return;

	cUOTxRemoveObject rObject;
	rObject.setSerial( serial() );

	cUOTxDrawChar drawChar;
	drawChar.fromChar( this );

	cUOSocket *mSock;

	for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		P_CHAR pChar = mSock->player();

		if( !pChar || !pChar->account() )
			continue;

		if( pChar->dist( this ) > pChar->VisRange() )
			continue;
        
		if( clean )
			mSock->send( &rObject );

		// We are hidden (or dead and not visible)
		if( ( isHidden() || ( dead_ && !war_ ) ) && !pChar->isGMorCounselor() )
			continue;

		drawChar.setHighlight( notority( pChar ) );
		
		sendTooltip( mSock );
		mSock->send( &drawChar );
	}
}

void cNPC::talk( const QString &message, UI16 color, UINT8 type, bool autospam, cUOSocket* socket )
{
	if( autospam )
	{
		if( nextMsgTime_ < uiCurrentTime )
			nextMsgTime_ = uiCurrentTime + MY_CLOCKS_PER_SEC*10;
		else 
			return;
	}

	if( color == 0xFFFF )
		color = saycolor_;

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
	textSpeech->setLanguage( QString() );
	textSpeech->setName( name() );
	textSpeech->setColor( color );
	textSpeech->setText( message );

	if( socket )
	{
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
					textSpeech->setText( message );
					mSock->send( new cUOTxUnicodeSpeech( *textSpeech ) );
				}
		}
		delete textSpeech;
	}
}

UINT8 cNPC::notority( P_CHAR pChar ) // Gets the notority toward another char
{
	// 0x01 Blue, 0x02 Green, 0x03 Grey, 0x05 Orange, 0x06 Red
	UINT8 result;

	// Check for Guild status + Highlight
	UINT8 guildStatus = GuildCompare( this, pChar );

//	if( npcaitype() == 0x02 )
//		return 0x06; // 6 = Red -> Monster

	if( pChar->kills() > SrvParams->maxkills() )
		result = 0x06; // 6 = Red -> Murderer
	
	else if( guildStatus == 1 )
		result = 0x02; // 2 = Green -> Same Guild
	
	else if( guildStatus == 2 )
		result = 0x05; // 5 = Orange -> Enemy Guild

	else
	{
		// Monsters are always bad
//		if( npcaitype_ == 4 )
//			return 0x01;
		
		if( isHuman() )
		{
			if( karma_ >= 0 )
				result = 0x01;
			else
				result = 0x06;
		}
		
		// Everything else
		else
			result = 0x03;
	}
	
	return result;
}

void cNPC::kill()
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
	setOrgSkin( skin() );
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

	corpse->setBodyId( xid_ );
	corpse->setMoreY( ishuman( this ) ); //is human??
	corpse->setCarve( carve() ); //store carve section
	corpse->setName2( name() );

	corpse->moveTo( pos() );

	corpse->setMore1(nType);
	corpse->setDirection( dir_ );
	corpse->startDecay();
	
	// stores the time and the murderer's name
	corpse->setMurderer( murderer );
	corpse->setMurderTime(uiCurrentTime);

	// create loot
	QStringList lootItemSections = DefManager->getList( loot_ );
	QStringList::const_iterator it = lootItemSections.begin();

	while( it != lootItemSections.end() )
	{
		P_ITEM pi_loot = Items->createScriptItem( (*it) );
		if( pi_loot )
			corpse->addItem( pi_loot );
		it++;
	}
	
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

	resend( true );

	cCharStuff::DeleteChar( this );
}

/*!
	If this character is in a guarded area, it checks the surroundings for criminals
	or murderers and spawns a guard if one is found.
*/
void cNPC::callGuards()
{
	if( antiguardstimer() < uiCurrentTime )
	{
		setAntiguardstimer( uiCurrentTime + (MY_CLOCKS_PER_SEC*10) );
	} else 
		return;

	if (!inGuardedArea() || !SrvParams->guardsActive() )
		return;

	// Is there a criminal around?
	RegionIterator4Chars ri(pos());
	for( ri.Begin(); !ri.atEnd(); ri++ )
	{
		P_CHAR pc = ri.GetData();
		if( pc )
		{
			if( !pc->dead() && !pc->isInnocent() && inRange( pc, 14 ) )
			{
				Combat::spawnGuard( pc, pc, pc->pos() );
			}
		}
	}
}

void cNPC::showName( cUOSocket *socket )
{
	if( !socket->player() )
		return;

	if( onSingleClick( socket->player() ) )
		return;

	QString charName = name();

	// apply titles
	if( SrvParams->showNpcTitles() && !title_.isEmpty() )
		charName.append( ", " + title_ );

	// Lord & Lady Title
	if( fame_ == 10000 )
		charName.prepend( ( id() == 0x191 ) ? tr( "Lady " ) : tr( "Lord " ) );

	// Are we squelched ?
	if( squelched() )
		charName.append( tr(" [squelched]" ) );

	// Append serial for GMs
	if( socket->player()->canSeeSerials() )
		charName.append( QString( " [0x%1]" ).arg( serial(), 4, 16 ) );

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

void cNPC::fight(P_CHAR other)
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
	
	if (this->isNpc())
	{
		if (!this->war_)
			toggleCombat();

		this->setNextMoveTime();
	}
}

void cNPC::soundEffect( UI16 soundId, bool hearAll )
{
	if( !hearAll )
		return;

	cUOTxSoundEffect pSoundEffect;
	pSoundEffect.setSound( soundId );
	pSoundEffect.setCoord( pos() );

	// Send the sound to all sockets in range
	for( cUOSocket *s = cNetwork::instance()->first(); s; s = cNetwork::instance()->next() )
		if( s->player() && s->player()->inRange( this, s->player()->VisRange() ) )
			s->send( &pSoundEffect );
}

void cNPC::giveGold( Q_UINT32 amount, bool inBank )
{
	P_ITEM pCont = getBackpack();

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
}

UINT32 cNPC::takeGold( UINT32 amount, bool useBank )
{
	P_ITEM pPack = getBackpack();

	UINT32 dAmount;

	if( pPack )
		dAmount = pPack->DeleteAmount( amount, 0xEED, 0 );

	return dAmount;
}

void cNPC::attackTarget( P_CHAR defender )
{
	if( this == defender || !defender || dead() || defender->dead() ) 
		return;

	playmonstersound( this, id_, SND_STARTATTACK );
	unsigned int cdist=0 ;

	P_CHAR target = FindCharBySerial( defender->targ() );
	if( target )
		cdist = defender->dist( target );
	else 
		cdist = 30;

	if( cdist > defender->dist( this ) )
	{
		defender->setAttacker(serial());
		defender->setAttackFirst();
	}

	target = FindCharBySerial( targ_ );
	if( target )
		cdist = this->dist( target );
	else 
		cdist = 30;

	if( ( cdist > defender->dist( this ) ) &&
		( !(npcaitype() == 4) || target ) )
	{
		targ_ = defender->serial();
		attacker_ = defender->serial();
		resetAttackFirst();
	}

	unhide();
	disturbMed();
	
	if( defender->isNpc() )
	{
		if( !( defender->war() ) )
			defender->toggleCombat();
		defender->setNextMoveTime();
	}
	
	if( npcaitype_ != 4 )
	{
		if ( !war_ )
			toggleCombat();

		setNextMoveTime();
	}

	// Check if the defender has pets defending him
	CharContainer guards = defender->guardedby();

	for( CharContainer::const_iterator iter = guards.begin(); iter != guards.end(); ++iter )
	{
		P_CHAR pPet = *iter;

		if( pPet->targ() == INVALID_SERIAL && pPet->inRange( this, SrvParams->attack_distance() ) ) // is it on screen?
		{
			pPet->fight( this );

			// Show the You see XXX attacking YYY messages
			QString message = tr( "*You see %1 attacking %2*" ).arg( pPet->name() ).arg( name() );
			for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
				if( mSock->player() && mSock != socket_ && mSock->player()->inRange( pPet, mSock->player()->VisRange() ) )
					mSock->showSpeech( pPet, message, 0x26, 3, cUOTxUnicodeSpeech::Emote );
			
			if( socket_ )
				socket_->showSpeech( pPet, tr( "*You see %1 attacking you*" ).arg( pPet->name() ), 0x26, 3, cUOTxUnicodeSpeech::Emote );
		}
	}

	// Send a message to the defender
	if( defender->objectType() == enPlayer )
	{
		P_PLAYER pc = dynamic_cast<P_PLAYER>(defender);
		if( pc && pc->socket() )
		{
			QString message = tr( "You see %1 attacking you!" ).arg( name() );
			pc->socket()->showSpeech( this, message, 0x26, 3, cUOTxUnicodeSpeech::Emote );
		}
	}

	QString emote = tr( "You see %1 attacking %2" ).arg( name() ).arg( defender->name() );

	cUOSocket *mSock = 0;
	for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		if( mSock->player() && mSock->player() != this && mSock->player() != defender && mSock->player()->inRange( this, mSock->player()->VisRange() ) )
		{
			mSock->showSpeech( this, emote, 0x26, 3, cUOTxUnicodeSpeech::Emote );
		}
	}
}

void cNPC::toggleCombat()
{
	war_ = !war_;
	Movement::instance()->CombatWalk( this );
}

void cNPC::processNode( const QDomElement &Tag )
{
	changed( SAVE );
	QString TagName = Tag.nodeName();
	QString Value = this->getNodeValue( Tag );
	QDomNodeList ChildTags;

	//<carve>3</carve>
	if( TagName == "carve" ) 
		this->setCarve( Value );

	//<cantrain />
	else if( TagName == "cantrain" )
		this->setCantrain( true );

	//<attack min=".." max= "" />
	//<attack>10</attack>
	else if( TagName == "attack" )
	{
		if( Tag.hasAttribute("min") && Tag.hasAttribute("max") )
		{
			lodamage_ = hex2dec( Tag.attribute("min") ).toInt();
			hidamage_ = hex2dec( Tag.attribute("max") ).toInt();
		}
		else
		{
			lodamage_ = Value.toInt();
			hidamage_ = lodamage_;
		}
	}

	//<fleeat>10</fleeat>
	else if( TagName == "fleeat" )
		this->setFleeat( Value.toShort() );

	//<hidamage>10</hidamage>
	else if( TagName == "hidamage" )
		this->hidamage_ = Value.toInt();

	//<loot>lootlist</loot>
	else if( TagName == "loot" )
	{
		this->setLootList( Value );
	}

	//<lodamage>10</lodamage>
	else if( TagName == "lodamage" )
		this->lodamage_ = Value.toInt();

	//<notrain />
	else if( TagName == "notrain" )
		this->setCantrain( false );

	//<npcwander type="rectangle" x1="-10" x2="12" y1="5" y2="7" />
	//<......... type="rect" ... />
	//<......... type="3" ... />
	//<......... type="circle" radius="10" />
	//<......... type="2" ... />
	//<......... type="free" (or "1") />
	//<......... type="none" (or "0") />
	else if( TagName == "npcwander" )
	{
		if( Tag.attributes().contains("type") )
		{
			QString wanderType = Tag.attribute("type");
			if( wanderType == "rectangle" || wanderType == "rect" || wanderType == "3" )
				if( Tag.attributes().contains("x1") &&
					Tag.attributes().contains("x2") &&
					Tag.attributes().contains("y1") &&
					Tag.attributes().contains("y2") )
				{
					this->npcWander_ = 3;
					this->fx1_ = this->pos().x + Tag.attribute("x1").toInt();
					this->fx2_ = this->pos().x + Tag.attribute("x2").toInt();
					this->fy1_ = this->pos().y + Tag.attribute("y1").toInt();
					this->fy2_ = this->pos().y + Tag.attribute("y2").toInt();
					this->fz1_ = -1 ;
				}
			else if( wanderType == "circle" || wanderType == "4" )
			{
				this->npcWander_ = 4;
				this->fx1_ =  this->pos().x;
				this->fy1_ = this->pos().y;
				if( Tag.attributes().contains("radius") )
					this->fx2_ =  Tag.attribute("radius").toInt();
				else
					this->fx2_ = 5;
			}
			else if( wanderType == "free" || wanderType == "2" )
				this->npcWander_ = 2;
			else
				this->npcWander_ = 0; //default
		}
	}

	//<ai>2</ai>
	else if( TagName == "ai" )
		this->setNpcAIType( Value.toInt() );

	//<reattackat>40</reattackat>
	else if( TagName == "reattackat" )
		this->setReattackat( Value.toShort() );

	//<shopkeeper>
	//	<sellable>...handled like item-<contains>-section...</sellable>
	//	<buyable>...see above...</buyable>
	//	<restockable>...see above...</restockable>
	//</shopkeeper>
	else if( TagName == "shopkeeper" )
	{
		makeShop();
		QDomNode childNode = Tag.firstChild();
		while( !childNode.isNull() )
		{
			QDomElement currNode = childNode.toElement();
			
			if( !currNode.hasChildNodes() )
			{
				childNode = childNode.nextSibling();
				continue;
			}

			unsigned char contlayer = 0;
			if( currNode.nodeName() == "restockable" )
				contlayer = 0x1A;
			else if( currNode.nodeName() == "buyable" )
				contlayer = 0x1B;
			else if( currNode.nodeName() == "sellable" )
				contlayer = 0x1C;
			else 
			{
				childNode = childNode.nextSibling();
				continue;
			}
				
			P_ITEM contItem = this->GetItemOnLayer( contlayer );
			if( contItem != NULL )
				contItem->processContainerNode( currNode );
			childNode = childNode.nextSibling();
		}
		
		restock();
	}
		
	//<split>1</split>
	else if( TagName == "split" )
		this->setSplit( Value.toUShort() );

	//<splitchance>10</splitchance>
	else if( TagName == "splitchance" )
		this->setSplitchnc( Value.toUShort() );

	//<totame>115</totame>
	else if( TagName == "totame" )
		this->taming_ = Value.toInt();

	else
		cBaseChar::processNode( Tag );

}

// Simple setting and getting of properties for scripts and the set command.
stError *cNPC::setProperty( const QString &name, const cVariant &value )
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

stError *cNPC::getProperty( const QString &name, cVariant &value ) const
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

Coord_cl cNPC::nextMove()
{
	Coord_cl ret;
	if( !path_.empty() )
	{
		ret = path_.front();
	}
	else
		ret = Coord_cl( 0xFFFF, 0xFFFF, 0xFF, 0 );

	return ret;
}

void cNPC::pushMove( UI16 x, UI16 y, SI08 z )
{
	path_.push_back( Coord_cl( x, y, z, 0 ) );
}

void cNPC::pushMove( const Coord_cl &move )
{
	path_.push_back( move );
}

void cNPC::popMove( void )
{
	path_.pop_front();
}

void cNPC::clearPath( void )
{
	path_.clear();
}

bool cNPC::hasPath( void )
{
	return !path_.empty();
}

Coord_cl cNPC::pathDestination( void ) const
{
	if( path_.empty() )
		return Coord_cl( 0xFFFF, 0xFFFF, 0xFF, 0 );
	else
		return path_.back();
}

/*!
	A* pathfinding algorithm implementation. consult me (sereg) for changes!
	I've used this paper as a base: http://www.wpdev.org/docs/informed-search.pdf
	A*>>
*/

/*!
	We'll need to build a list of pathnodes. Each pathnode contains
	coordinates, iteration step, cost and a pointer to its predecessor.
*/
class pathnode_cl 
{
public:
	pathnode_cl( UI16 x_, UI16 y_, SI08 z_, UI16 step_, float cost_ ) : prev( NULL ), x( x_ ), y( y_ ), z( z_ ), step( step_ ), cost( cost_ ) {}
	pathnode_cl() : prev( NULL ) {}
	UI16	x;
	UI16	y;
	SI08	z;
	UI16	step;
	float	cost;

	pathnode_cl *prev;
};

/*!
	Compare predicate for pathnodes using cost and step
	Two times because we need more weight for the costs.
*/
struct pathnode_comparePredicate : public std::binary_function<pathnode_cl, pathnode_cl, bool>
{
	bool operator()(const pathnode_cl *a, const pathnode_cl *b)
	{
		return (a->step + 2 * a->cost) > (b->step + 2 * b->cost);
	}
};

/*!
	Compare predicate for pathnodes using coordinates
*/
struct pathnode_coordComparePredicate : public std::binary_function<pathnode_cl, pathnode_cl, bool>
{
	bool operator()(const pathnode_cl *a, const pathnode_cl *b)
	{
		return ( a->x * a->x + a->y * a->y + a->z * a->z ) <
				( b->x * b->x + b->y * b->y + b->z * b->z );
	}
};

/*!
	Heuristic function for A*
	We use simple 3-dim. euclid distance: d = sqrt( (x1-x2)² + (y1-y2)² + (z1-z2)² )
*/
float cNPC::pathHeuristic( const Coord_cl &source, const Coord_cl &destination )
{
	return (float)( sqrt( pow( source.x - destination.x, 2 ) + pow( source.y - destination.y, 2 ) + pow( (source.z - destination.z) / 5.0f, 2 ) ) );
}

/*!
	The algorithm..
	currently works in x,y,z direction. no idea how to implement map jumping yet.
*/
void cNPC::findPath( const Coord_cl &goal, float sufficient_cost = 0.0f )
{
	if( path_.size() > 0 )
		path_.clear();

	if( pos_.map != goal.map )
		return;

	/*
		For A* we use a priority queue to store the unexamined path nodes 
		in a way, that the node with lowest cost lies always at the beginning.
		I'll use std::vector combined with the stl heap functions for it.
	*/
	std::vector< pathnode_cl* >	unvisited_nodes;
	std::vector< pathnode_cl* >	visited_nodes;
	std::make_heap( unvisited_nodes.begin(), unvisited_nodes.end(), pathnode_comparePredicate() );

	/*
		We also need a vector for the calculated list.
		And a temporary vector of neighbours.
	*/
	std::vector< pathnode_cl* >	allnodes;
	std::vector< Coord_cl >		neighbours;
	std::vector< pathnode_cl* >::iterator	pit;
	std::vector< Coord_cl >::iterator		nit;

	/*
		So let's start :)
		The initial thing we'll have to do is to push the current char position
		into the priority queue.
	*/
	pathnode_cl *baseNode = new pathnode_cl( pos_.x, pos_.y, pos_.z, 0, pathHeuristic( pos_, goal ) );
	unvisited_nodes.push_back( baseNode );
	// pushing the heap isnt needed here..
	allnodes.push_back( baseNode );

	/*
		Each iteration of the following loop will take the first element
		out of the priority queue and calculate the neighbour nodes 
		(sourrounding coordinates). The nodes have prev pointers which let
		us get the whole path in the end.

		All neighbours that are reachable will be pushed into the priority
		queue.

		If no neighbour is reachable we ran into a dead end. Nothing will
		be done in this iteration, because we'll get a better node out of the
		priority queue next iteration.
	*/

	pathnode_cl *currentNode = NULL;
	pathnode_cl *newNode = NULL;
	pathnode_cl *prevNode = NULL;
	int iterations = 0;
	while( !unvisited_nodes.empty() )
	{
		// get the first element of the queue
		bool visited;
		do
		{
			visited = false;
			currentNode = *unvisited_nodes.begin();

			// remove it from the queue
			std::pop_heap( unvisited_nodes.begin(), unvisited_nodes.end(), pathnode_comparePredicate() );
			unvisited_nodes.pop_back();

			// dont calculate loops !

			visited = std::binary_search( visited_nodes.begin(), visited_nodes.end(), currentNode, pathnode_coordComparePredicate() );

			// steps > step depth 
		} while( ( visited || currentNode->step > SrvParams->pathfindMaxSteps() ) && !unvisited_nodes.empty() );

		if( iterations > SrvParams->pathfindMaxIterations() || currentNode->step > SrvParams->pathfindMaxSteps() )
		{
			// lets set the pointer invalid if we have an invalid path
			currentNode = NULL;
			break;
		}

		// we reached the goal
		if( currentNode->cost <= sufficient_cost )
			break;

		// the neighbours can be all surrounding x-y-coordinates
		neighbours.clear();
		int i = 0;
		int j = 0;
		Coord_cl pos;
		for( i = -1; i <= 1; ++i )
		{
			for( j = -1; j <= 1; ++j )
			{
				if( i != 0 || j != 0 )
					neighbours.push_back( Coord_cl( currentNode->x + i, currentNode->y + j, currentNode->z, pos_.map ) );
			}
		}

		// check walkability and set z offsets for neighbours
		nit = neighbours.begin();
		while( nit != neighbours.end() )
		{
			pos = *nit;
			// this should change z coordinates also if there are stairs
			if( mayWalk( this, pos ) && !Movement::instance()->CheckForCharacterAtXYZ( this, pos ) )
			{
				// push a path node into the priority queue
				newNode = new pathnode_cl( pos.x, pos.y, pos.z, currentNode->step + 1, pathHeuristic( pos, goal ) );
				newNode->prev = currentNode;
				unvisited_nodes.push_back( newNode );
				allnodes.push_back( newNode );
				std::push_heap( unvisited_nodes.begin(), unvisited_nodes.end(), pathnode_comparePredicate() );
			}
			++nit;
		}

		visited_nodes.push_back( currentNode );
		std::sort( visited_nodes.begin(), visited_nodes.end(), pathnode_coordComparePredicate() );
		++iterations;
	}

	// finally lets set the char's path
	if( currentNode )
	{
		path_.clear();
		while( currentNode->prev )
		{
			path_.push_front( Coord_cl( currentNode->x, currentNode->y, currentNode->z, pos_.map ) );
			currentNode = currentNode->prev;
		}
	}

	/* debug...
	clConsole.send( QString( "Pathfinding: %1 iterations\n" ).arg( iterations ) );
	std::deque< Coord_cl >::const_iterator it = path_.begin();
	while( it != path_.end() )
	{
		clConsole.send( QString( "%1,%2\n" ).arg( (*it).x ).arg( (*it).y ) );
		++it;
	}
	*/

	// lets free the memory assigned to the nodes...
	pit = allnodes.begin();
	while( pit != allnodes.end() )
	{
		delete *pit;
		++pit;
	}
}

/*!
	<<A*
*/


