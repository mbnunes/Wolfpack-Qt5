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

// library includes
#include <qvaluelist.h>
#include <math.h>

// wolfpack includes
#include "npc.h"
#include "network/uotxpackets.h"
#include "network/uosocket.h"
#include "network.h"
#include "player.h"
#include "globals.h"
#include "world.h"
#include "persistentbroker.h"
#include "dbdriver.h"
#include "sectors.h"
#include "srvparams.h"
#include "corpse.h"
#include "wpdefmanager.h"
#include "chars.h"
#include "combat.h"
#include "walking.h"
#include "skills.h"
#include "ai.h"

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
	stablemasterSerial_	= INVALID_SERIAL;
	wanderType_			= stWanderType();
	aiid_				= "Monster_Aggressive_L1";
	ai_					= new Monster_Aggressive_L1( this );
	aiCheckInterval_	= (UINT16)floor(SrvParams->checkAITime() * 1000.0f);
	aiCheckTime_		= uiCurrentTime + (float)aiCheckInterval_ * 0.001f * MY_CLOCKS_PER_SEC;
	criticalHealth_		= 10; // 10% !
	spellsLow_			= 0;
	spellsHigh_			= 0;
}

cNPC::cNPC(const cNPC& right)
{
}

cNPC::~cNPC()
{
	delete ai_;
}

cNPC& cNPC::operator=(const cNPC& right)
{
	return *this;
}

static cUObject* productCreator()
{
	return new cNPC;
}

void cNPC::registerInFactory()
{
	QStringList fields, tables, conditions;
	buildSqlString( fields, tables, conditions ); // Build our SQL string
	QString sqlString = QString( "SELECT %1 FROM uobjectmap,%2 WHERE uobjectmap.type = 'cNPC' AND %3" ).arg( fields.join( "," ) ).arg( tables.join( "," ) ).arg( conditions.join( " AND " ) );
	UObjectFactory::instance()->registerType( "cNPC", productCreator );
	UObjectFactory::instance()->registerSqlQuery( "cNPC", sqlString );
}

void cNPC::buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions )
{
	cBaseChar::buildSqlString( fields, tables, conditions );
	fields.push_back( "npcs.mindamage,npcs.maxdamage,npcs.tamingminskill" );
	fields.push_back( "npcs.summontime,npcs.additionalflags,npcs.owner" );
	fields.push_back( "npcs.carve,npcs.spawnregion,npcs.stablemaster" );
	fields.push_back( "npcs.lootlist,npcs.ai,npcs.wandertype" );
	fields.push_back( "npcs.wanderx1,npcs.wanderx2,npcs.wandery1,npcs.wandery2" );
	fields.push_back( "npcs.wanderradius,npcs.fleeat,npcs.spellslow,npcs.spellshigh" );
	tables.push_back( "npcs" );
	conditions.push_back( "uobjectmap.serial = npcs.serial" );
}

static void npcRegisterAfterLoading( P_NPC pc );

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
	setAI( result[offset++] );
	setWanderType( (enWanderTypes)atoi( result[offset++] ) );
	setWanderX1( atoi( result[offset++] ) );
	setWanderX2( atoi( result[offset++] ) );
	setWanderY1( atoi( result[offset++] ) );
	setWanderY2( atoi( result[offset++] ) );
	setWanderRadius( atoi( result[offset++] ) );
	setCriticalHealth( atoi( result[offset++] ) );
	spellsLow_ = atoi( result[offset++] );
	spellsHigh_ = atoi( result[offset++] );

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
		addField( "summontime", summonTime_ ? summonTime_ - uiCurrentTime : 0 );
		addField( "additionalflags", additionalFlags_ );
		addField( "owner", owner_ ? owner_->serial() : INVALID_SERIAL );
		addStrField( "carve", carve_);
		addStrField( "spawnregion", spawnregion_);
		addField( "stablemaster", stablemasterSerial_ );
		addStrField( "lootlist", lootList_);
		addStrField( "ai", aiid_ );
		addField( "wandertype", (UINT8)wanderType() );
		addField( "wanderx1", wanderX1() );
		addField( "wanderx2", wanderX2() );
		addField( "wandery1", wanderY1() );
		addField( "wandery2", wanderY2() );
		addField( "wanderradius", wanderRadius() );
		addField( "fleeat", criticalHealth() );
		addField( "spellslow", spellsLow_ );
		addField( "spellshigh", spellsHigh_ );
		
		addCondition( "serial", serial() );
		saveFields;
	}
	cBaseChar::save();
}

bool cNPC::del()
{	
	if( !isPersistent )
		return false; // We didn't need to delete the object

	persistentBroker->addToDeleteQueue( "npcs", QString( "serial = '%1'" ).arg( serial() ) );
	changed_ = true;
	return cBaseChar::del();
}

static void npcRegisterAfterLoading( P_NPC pc )
{
	if( pc->stablemasterSerial() == INVALID_SERIAL )
	{ 
		MapObjects::instance()->add(pc); 
	} 
	else
		stablesp.insert(pc->stablemasterSerial(), pc->serial());
}

void cNPC::setOwner(P_PLAYER data, bool nochecks)
{
	// We CANT be our own owner
	if( data && ( data->serial() == this->serial() ) )
		return;

	if( !nochecks && owner_ )
	{
		owner_->removePet( this, true );
		setTamed( false );
	}

	owner_ = data;
	changed( TOOLTIP );
	changed_ = true;

	if( !nochecks && owner_ )
	{
		owner_->addPet( this, true );
		setTamed( false );
	}
}

void cNPC::setNextMoveTime()
{
	if( isTamed() )
		setNextMoveTime( uiCurrentTime + MY_CLOCKS_PER_SEC * SrvParams->tamedNpcMoveTime() );
	else
		setNextMoveTime( uiCurrentTime + MY_CLOCKS_PER_SEC * SrvParams->npcMoveTime() );
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
		P_PLAYER pChar = mSock->player();

		if( !pChar )
			continue;

		if( pChar->dist( this ) > pChar->visualRange() )
			continue;
        
		if( clean )
			mSock->send( &rObject );

		// We are hidden (or dead and not visible)
		if( ( isHidden() || ( isDead() && !isAtWar() ) ) && !pChar->isGMorCounselor() )
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
	textSpeech->setModel( bodyID_ );
	textSpeech->setFont( 3 ); // Default Font
	textSpeech->setType( speechType );
	textSpeech->setLanguage( "" );
	textSpeech->setName( name() );
	textSpeech->setColor( color );
	textSpeech->setText( message );

	if( socket )
	{
		socket->send( textSpeech );
	}
	else
	{
		// Send to all clients in range
		for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
		{
				if( mSock->player() && ( mSock->player()->dist( this ) < 18 ) )
				{
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
//	UINT8 guildStatus = GuildCompare( this, pChar );

//	if( npcaitype() == 0x02 )
//		return 0x06; // 6 = Red -> Monster

	if( pChar->kills() > SrvParams->maxkills() )
		result = 0x06; // 6 = Red -> Murderer
	
//	else if( guildStatus == 1 )
//		result = 0x02; // 2 = Green -> Same Guild
	
//	else if( guildStatus == 2 )
//		result = 0x05; // 5 = Orange -> Enemy Guild

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
	changed( TOOLTIP );
	changed_ = true;
	int ele;

	if( free )
		return;

	if( isDead() || isInvulnerable() )
		return;

	// Do this in the beginning
	setDead( true );
	hitpoints_ = 0; // With no hp left

	if( isPolymorphed() )
	{
		setBodyID( orgBodyID_ );
		setPolymorphed( false );
	}

	orgBodyID_ = bodyID();
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

			if( pc_t->objectType() == enPlayer && !pc_t->inGuardedArea() )
			{
				pc_t->awardKarma( this, ( 0 - ( karma_ ) ) );
				pc_t->awardFame( fame_ );
			}

			if( pc_t->objectType() == enNPC && pc_t->isAtWar() )
				dynamic_cast<P_NPC>(pc_t)->toggleCombat();

		}
	}

	// Now for the corpse
	P_ITEM pi_backpack = getBackpack();
	
#pragma note("Implement here tradewindow closing and disposal of it's cItem*")
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

	corpse->setBodyId( orgBodyID_ );
	corpse->setCarve( carve() ); //store carve section
	corpse->tags().set( "human", cVariant( isHuman() ? 1 : 0 ) );
	corpse->tags().set( "name", cVariant( name() ) );

	corpse->moveTo( pos() );
	corpse->setDirection( direction() );
	
	// stores the time and the murderer's name
	corpse->setMurderer( murderer );
	corpse->setMurderTime(uiCurrentTime);

	// create loot
	QStringList lootItemSections = DefManager->getList( lootList_ );
	QStringList::const_iterator it = lootItemSections.begin();

	while( it != lootItemSections.end() )
	{
		P_ITEM pi_loot = cItem::createFromScript( (*it) );
		if( pi_loot )
			corpse->addItem( pi_loot );
		it++;
	}
	
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

	cUOTxDeathAction dAction;
	dAction.setSerial( serial() );
	dAction.setCorpse( corpse->serial() );

	cUOTxRemoveObject rObject;
	rObject.setSerial( serial() );

	for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
		if( mSock->player() && mSock->player()->inRange( this, mSock->player()->visualRange() ) )
		{
			if( SrvParams->showDeathAnim() )
				mSock->send( &dAction );

			mSock->send( &rObject );
		}
	
	corpse->update();

	resend( true );

	// trigger the event
	onDeath();

	cCharStuff::DeleteChar( this );
}

/*!
	If this character is in a guarded area, it checks the surroundings for criminals
	or murderers and spawns a guard if one is found.
*/
void cNPC::callGuards()
{
	if( nextGuardCallTime() < uiCurrentTime )
	{
		setNextGuardCallTime( uiCurrentTime + (MY_CLOCKS_PER_SEC*10) );
	} else 
		return;

	cBaseChar::callGuards();
}

void cNPC::applyDefinition( const cElement *sectionNode )
{
	cBaseChar::applyDefinition( sectionNode );

	// Let's try to assume some unspecified values
	if ( this->strength() && !this->hitpoints() ) // we don't want to instantly die, right?
	{
		if ( !this->maxHitpoints() )
			setMaxHitpoints( strength() );
		setHitpoints( maxHitpoints() );
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
		charName.prepend( ( gender_ ) ? tr( "Lady " ) : tr( "Lord " ) );

	// Append serial for GMs
	if( socket->player()->showSerials() )
		charName.append( QString( " [0x%1]" ).arg( serial(), 4, 16 ) );

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
	if( ( criminalTime() > uiCurrentTime ) && ( kills_ < SrvParams->maxkills() ) )
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
	if( isAtWar() && combatTarget_ == other->serial() )
		return;

	// Store the current Warmode
	bool oldwar = isAtWar();

	this->combatTarget_ = other->serial();
	this->unhide();
	this->attackerSerial_ = other->serial();
	this->setAtWar( true );
	
	if( !isAtWar() )
		toggleCombat();

	this->setNextMoveTime();
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
		if( s->player() && s->player()->inRange( this, s->player()->visualRange() ) )
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

	UINT32 dAmount = 0;

	if( pPack )
		dAmount = pPack->DeleteAmount( amount, 0xEED, 0 );

	return dAmount;
}

void cNPC::attackTarget( P_CHAR defender )
{
	if( this == defender || !defender || isDead() || defender->isDead() ) 
		return;

	playmonstersound( this, bodyID_, SND_STARTATTACK );
	unsigned int cdist=0 ;

	P_CHAR target = FindCharBySerial( defender->combatTarget() );
	if( target )
		cdist = defender->dist( target );
	else 
		cdist = 30;

	if( cdist > defender->dist( this ) )
	{
		defender->setAttackerSerial(serial());
		defender->setAttackFirst( true );
	}

	target = FindCharBySerial( combatTarget_ );
	if( target )
		cdist = this->dist( target );
	else 
		cdist = 30;

	if( ( cdist > defender->dist( this ) ) &&
		( target ) )
	{
		combatTarget_ = defender->serial();
		attackerSerial_ = defender->serial();
		setAttackFirst( false );
	}

	unhide();
	
	P_NPC pNPC = dynamic_cast<P_NPC>(defender);
	if( pNPC )
	{
		if( !( pNPC->isAtWar() ) )
			pNPC->toggleCombat();
		pNPC->setNextMoveTime();
	}
	
/*	if( npcaitype_ != 4 )
	{
		if ( !war_ )
			toggleCombat();

		setNextMoveTime();
	}*/

	// Check if the defender has pets defending him
	CharContainer guards = defender->guardedby();

	for( CharContainer::const_iterator iter = guards.begin(); iter != guards.end(); ++iter )
	{
		P_NPC pPet = dynamic_cast<P_NPC>(*iter);

		if( pPet && pPet->combatTarget() == INVALID_SERIAL && pPet->inRange( this, SrvParams->attack_distance() ) ) // is it on screen?
		{
			pPet->fight( this );

			// Show the You see XXX attacking YYY messages
			QString message = tr( "*You see %1 attacking %2*" ).arg( pPet->name() ).arg( name() );
			for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
				if( mSock->player() && mSock->player()->inRange( pPet, mSock->player()->visualRange() ) )
					mSock->showSpeech( pPet, message, 0x26, 3, cUOTxUnicodeSpeech::Emote );
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
		if( mSock->player() && mSock->player()->serial() != serial() && mSock->player() != defender && mSock->player()->inRange( this, mSock->player()->visualRange() ) )
		{
			mSock->showSpeech( this, emote, 0x26, 3, cUOTxUnicodeSpeech::Emote );
		}
	}
}

void cNPC::toggleCombat()
{
	setAtWar( !isAtWar() );
	Movement::instance()->CombatWalk( this );
}

void cNPC::processNode( const cElement *Tag )
{
	changed_ = true;
	QString TagName = Tag->name();
	QString Value = Tag->getValue();
	QDomNodeList ChildTags;

	//<attack min=".." max= "" />
	//<attack>10</attack>
	if( TagName == "attack" )
	{
		if( Tag->hasAttribute("min") && Tag->hasAttribute("max") )
		{
			minDamage_ = hex2dec( Tag->getAttribute("min") ).toInt();
			maxDamage_ = hex2dec( Tag->getAttribute("max") ).toInt();
		}
		else
		{
			minDamage_ = Value.toInt();
			maxDamage_ = minDamage_;
		}
	}

	//<npcwander type="rectangle" x1="-10" x2="12" y1="5" y2="7" />
	//<......... type="rect" ... />
	//<......... type="3" ... />
	//<......... type="circle" radius="10" />
	//<......... type="2" ... />
	//<......... type="free" (or "1") />
	//<......... type="none" (or "0") />
	else if( TagName == "npcwander" )
	{
		if( Tag->hasAttribute("type") )
		{
			QString wanderType = Tag->getAttribute("type");
			if( wanderType == "rectangle" || wanderType == "rect" || wanderType == "3" )
				if( Tag->hasAttribute("x1") &&
					Tag->hasAttribute("x2") &&
					Tag->hasAttribute("y1") &&
					Tag->hasAttribute("y2") )
				{
					wanderType_ = stWanderType( pos().x + Tag->getAttribute("x1").toInt(),
						pos().x + Tag->getAttribute("x2").toInt(),
						pos().y + Tag->getAttribute("y1").toInt(),
						pos().y + Tag->getAttribute("y2").toInt() );
				}
			else if( wanderType == "circle" || wanderType == "4" )
			{
				wanderType_ = stWanderType( pos().x, pos().y, 5 );
				if( Tag->hasAttribute("radius") )
					setWanderRadius( Tag->getAttribute("radius").toInt() );
			}
			else if( wanderType == "free" || wanderType == "2" )
				wanderType_ = stWanderType( enFreely );
			else
				wanderType_ = stWanderType();
		}
	}

	//<shopkeeper>
	//	<sellable>...handled like item-<contains>-section...</sellable>
	//	<buyable>...see above...</buyable>
	//	<restockable>...see above...</restockable>
	//</shopkeeper>
	else if( TagName == "shopkeeper" )
	{
		makeShop();

		for( unsigned int i = 0; i < Tag->childCount(); ++i )
		{
			const cElement *currNode = Tag->getChild( i );
			
			if( !currNode->childCount() )
				continue;
			
			unsigned char contlayer = 0;
			if( currNode->name() == "restockable" )
				contlayer = BuyRestockContainer;
			else if( currNode->name() == "buyable" )
				contlayer = BuyNoRestockContainer;
			else if( currNode->name() == "sellable" )
				contlayer = SellContainer;
			else 
				continue;
				
			P_ITEM contItem = this->GetItemOnLayer( contlayer );
			if( contItem != NULL )
				contItem->processContainerNode( currNode );
		}		
	}

	else if( TagName == "inherit" )
	{
		QString inheritID;
		if( Tag->hasAttribute( "id" ) )
			inheritID = Tag->getAttribute( "id" );
		else
			inheritID = Value;

		const cElement *element = DefManager->getDefinition( WPDT_NPC, inheritID );
		if( element )
			applyDefinition( element );
	}

	else 
		cBaseChar::processNode( Tag );

}

// Simple setting and getting of properties for scripts and the set command.
stError *cNPC::setProperty( const QString &name, const cVariant &value )
{
	changed( TOOLTIP );
	changed_ = true;
	SET_INT_PROPERTY( "nextmsgtime", nextMsgTime_ )
	else SET_INT_PROPERTY( "antispamtimer", nextMsgTime_ )
	else SET_INT_PROPERTY( "nextguardcalltime", nextGuardCallTime_ )
	else SET_INT_PROPERTY( "antiguardstimer", nextGuardCallTime_ )
	else SET_STR_PROPERTY( "carve", carve_ )
	else SET_STR_PROPERTY( "spawnregion", spawnregion_ )
	else SET_INT_PROPERTY( "stablemaster", stablemasterSerial_ )
	else SET_STR_PROPERTY( "lootlist", lootList_ )
	else SET_STR_PROPERTY( "loot", lootList_ )
	else SET_INT_PROPERTY( "maxdamage", maxDamage_ )
	else SET_INT_PROPERTY( "mindamage", minDamage_ )
	else SET_INT_PROPERTY( "hidamage", maxDamage_ )
	else SET_INT_PROPERTY( "lodamage", minDamage_ )
	else SET_INT_PROPERTY( "karma", karma_ )
	else SET_INT_PROPERTY( "fame", fame_ )
	else SET_INT_PROPERTY( "nextmovetime", nextMoveTime_ )
	else SET_INT_PROPERTY( "npcmovetime", nextMoveTime_ )
	else if( name == "wandertype" )
	{
		setWanderType( (enWanderTypes)value.toInt() );
		return 0;
	}
	else if( name == "fx1" || name == "wanderx1" )
	{
		setWanderX1( value.toInt() );
		return 0;
	}
	else if( name == "fx2" || name == "wanderx2" )
	{
		setWanderX2( value.toInt() );
		return 0;
	}
	else if( name == "fy1" || name == "wandery1" )
	{
		setWanderY1( value.toInt() );
		return 0;
	}
	else if( name == "fy2" || name == "wandery2" )
	{
		setWanderY2( value.toInt() );
		return 0;
	}
	else if( name == "fz1" || name == "wanderradius" )
	{
		setWanderRadius( value.toInt() );
		return 0;
	}
	else SET_INT_PROPERTY( "totame", tamingMinSkill_ )
	else SET_INT_PROPERTY( "summontime", summonTime_) 
	else SET_INT_PROPERTY( "summontimer", summonTime_) 
	else if( name == "owner" )
	{
		P_PLAYER pOwner = dynamic_cast<P_PLAYER>(value.toChar());
		setOwner( pOwner );
		return 0;
	}

	// skill.
	else if( name.left( 6 ) == "skill." )
	{
		QString skill = name.right( name.length() - 6 );
		INT16 skillId = Skills->findSkillByDef( skill );

		if( skillId != -1 )
		{
			setSkillValue( skillId, value.toInt() );
			return 0;
		}
	}
	else if( name == "ai" )
	{
		setAI( value.toString() );
		return 0;
	}
	else SET_INT_PROPERTY( "fleeat", criticalHealth_ )
	else SET_INT_PROPERTY( "criticalhealth", criticalHealth_ )
	else SET_INT_PROPERTY( "spellslow", spellsLow_ )
	else SET_INT_PROPERTY( "spellshigh", spellsHigh_ )
	else if( name == "spell" )
	{
		UINT8 spell = name.right( name.length() - 6 ).toShort();

		setSpell( spell, value.toInt() );
		return 0;
	}
	
	return cBaseChar::setProperty( name, value );
}

stError *cNPC::getProperty( const QString &name, cVariant &value ) const
{
	GET_PROPERTY( "nextmsgtime", (int)nextMsgTime_ )
	else GET_PROPERTY( "antispamtimer", (int)nextMsgTime_ )
	else GET_PROPERTY( "nextguardcalltime", (int)nextGuardCallTime_ )
	else GET_PROPERTY( "antiguardstimer", (int)nextGuardCallTime_ )
	else GET_PROPERTY( "carve", carve_ )
	else GET_PROPERTY( "spawnregion", spawnregion_ )
	else GET_PROPERTY( "stablemaster", FindCharBySerial( stablemasterSerial_ ) )
	else GET_PROPERTY( "lootlist", lootList_ )
	else GET_PROPERTY( "loot", lootList_ )
	else GET_PROPERTY( "maxdamage", maxDamage_ )
	else GET_PROPERTY( "mindamage", minDamage_ )
	else GET_PROPERTY( "hidamage", maxDamage_ )
	else GET_PROPERTY( "lodamage", minDamage_ )
	else GET_PROPERTY( "npc", true )
	else GET_PROPERTY( "nextmovetime", (int)nextMoveTime_ )
	else GET_PROPERTY( "npcmovetime", (int)nextMoveTime_ )
	else GET_PROPERTY( "wandertype", (int)wanderType() )
	else GET_PROPERTY( "wanderx1", wanderX1() )
	else GET_PROPERTY( "fx1", wanderX1() )
	else GET_PROPERTY( "wanderx2", wanderX2() )
	else GET_PROPERTY( "fx2", wanderX2() )
	else GET_PROPERTY( "wandery1", wanderY1() )
	else GET_PROPERTY( "fy1", wanderY1() )
	else GET_PROPERTY( "wandery2", wanderY2() )
	else GET_PROPERTY( "fy2", wanderY2() )
	else GET_PROPERTY( "wanderradius", wanderRadius() )
	else GET_PROPERTY( "fz1", wanderRadius() )
	else GET_PROPERTY( "totame", tamingMinSkill_ )
	else GET_PROPERTY( "summontime", (int)summonTime_) 
	else GET_PROPERTY( "summontimer", (int)summonTime_) 
	else GET_PROPERTY( "owner", owner_ )
	else GET_PROPERTY( "ai", aiid_ )
	else GET_PROPERTY( "fleeat", criticalHealth_ )
	else GET_PROPERTY( "criticalhealth", criticalHealth_ )
	else GET_PROPERTY( "spellslow", (int)spellsLow_ )
	else GET_PROPERTY( "spellshigh", (int)spellsHigh_ )
	else if( "name" == "spell" )
	{
		UINT8 spell = name.right( name.length() - 6 ).toShort();
		
		value = cVariant( hasSpell( spell ) );
		return 0;
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
void cNPC::findPath( const Coord_cl &goal, float sufficient_cost /* = 0.0f */ )
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

void cNPC::setAI( const QString &data )
{
	QString tmp = QStringList::split( ",", data )[0];
	aiid_ = tmp;
	
	if( ai_ )
		delete ai_;

	ai_ = NULL;

	AbstractAI* ai = AIFactory::instance()->createObject( tmp );
	if( !ai ) 
		return;

	ScriptAI* sai = dynamic_cast< ScriptAI* >( ai );
	if( sai )
	{
		sai->setName( tmp );
	}
	ai->init( this );

	setAI( ai );
}

void cNPC::makeShop()
{
	P_ITEM currCont = GetItemOnLayer( BuyRestockContainer );
	if( !currCont )
	{
		currCont = new cItem;
		currCont->Init();
		currCont->setId( 0xE75 );
		currCont->setOwner( this );
		currCont->setType( 1 );		
		addItem( BuyRestockContainer, currCont );
		currCont->update();
	}

	currCont = GetItemOnLayer( BuyNoRestockContainer );
	if( !currCont )
	{
		currCont = new cItem;
		currCont->Init();
		currCont->setId( 0xE75 );
		currCont->setOwner( this );
		currCont->setType( 1 );		
		addItem( BuyNoRestockContainer, currCont );
		currCont->update();
	}

	currCont = GetItemOnLayer( SellContainer );
	if( !currCont )
	{
		currCont = new cItem;
		currCont->Init();
		currCont->setId( 0xE75 );
		currCont->setOwner( this );
		currCont->setType( 1 );		
		addItem( SellContainer, currCont );
		currCont->update();
	}
}

void cNPC::awardKarma( P_CHAR pKilled, short amount )
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
}

void cNPC::awardFame( short amount )
{
	int nCurFame, nChange=0;

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
	}
	else
	{
		nChange = ( amount - nCurFame ) / 75;
		setFame( nCurFame+nChange );
	}
}
