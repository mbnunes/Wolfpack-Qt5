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

#include "player.h"
#include "persistentbroker.h"
#include "dbdriver.h"
#include "globals.h"
#include "chars.h"
#include "guilds.h"
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
#include "combat.h"
#include "tilecache.h"
#include "guilds.h"
#include "skills.h"
#include "pythonscript.h"
#include "log.h"
#include "scriptmanager.h"
#include "inlines.h"

cPlayer::cPlayer()
{
	account_			= NULL;
	logoutTime_			= 0;
	objectDelay_		= 0;
	additionalFlags_	= 0;
	trackingTime_		= 0;
	socket_				= NULL;
	inputMode_			= enNone;
	inputItem_			= INVALID_SERIAL;
	visualRange_		= VISRANGE;
	profile_			= (char*)0;
	fixedLightLevel_	= 0xFF;
	party_				= 0;
	guild_				= 0;
	strengthLock_		= 0;
	dexterityLock_		= 0;
	intelligenceLock_	= 0;
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
	fields.push_back( "players.strlock,players.dexlock,players.intlock");
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
	strengthLock_ = atoi(result[offset++]);
	dexterityLock_ = atoi(result[offset++]);
	intelligenceLock_ = atoi(result[offset++]);

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
		addField( "strlock", strengthLock_ );
		addField( "dexlock", dexterityLock_ );
		addField( "intlock", intelligenceLock_ );

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
void cPlayer::update(bool excludeself) {
	cUOTxUpdatePlayer update;
	update.fromChar(this);

	for (cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next()) {
		if (socket != socket_ && socket->canSee(this)) {
			update.setHighlight(notoriety(socket->player()));
			socket->send(&update);
		}
	}

	if (!excludeself && socket_) {
		//socket_->updatePlayer();
		socket_->updatePlayer();
	}
}

// Resend the char to all sockets in range
void cPlayer::resend( bool clean, bool excludeself )
{
	if (socket_ && !excludeself) {
		socket_->resendPlayer();
	}

	cUOTxRemoveObject remove;
	remove.setSerial(serial());

	cUOTxDrawChar drawChar;
	drawChar.fromChar(this);

	for (cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next()) {
		// Don't send such a packet to ourself
		if (socket == socket_) {
			sendTooltip(socket);
		} else {
			if (socket->canSee(this)) {
				drawChar.setHighlight(notoriety(socket->player()));
				socket->send(&drawChar);
				sendTooltip(socket);
	            
				// Send equipment tooltips to other players as well
				for (ItemContainer::const_iterator it = content_.begin(); it != content_.end(); ++it) {
					it.data()->sendTooltip(socket);
				}
			} else if (clean) {
				socket->send(&remove);
			}
		}
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

UINT8 cPlayer::notoriety( P_CHAR pChar ) // Gets the notoriety toward another char
{
	// Player is incognito
	if (isIncognito()) {
		return 0x03;
	}

	// 0x01 Blue, 0x02 Green, 0x03 Grey, 0x05 Orange, 0x06 Red, 0x07 Yellow
	UINT8 result;

	if (isInvulnerable()) {
		return 7;
	}

	// Guilds override kills
	if (guild_ && pChar != this) {
		P_PLAYER player = dynamic_cast<P_PLAYER>(pChar);

		if (player && player->guild_) {
			// Same Guild => Green
			if (player->guild_ == guild_) {
				return 0x02;
			}
			// TODO: Enemy Guilds, Allied Guilds
		}
	}

	if (pChar->kills() > SrvParams->maxkills()) {
		result = 0x06; // 6 = Red -> Murderer

	} else if(account_) {
		if( isCriminal() )
			result = 0x03;
		else if( karma_ < -2000 )
			result = 0x06;
		else if( karma_ < 0 )
			result = 0x03;
		else
			result = 0x01;
	} else {
		// Everything else
		result = 0x03;
	}

	return result;
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
	if (pi && !pi->free) {
		P_NPC pMount = dynamic_cast<P_NPC>( FindCharBySerial( pi->getTag( "pet" ).toInt() ) );
		if (pMount && !pMount->free) {
			pMount->setWanderType(enHalt);
			pMount->setStablemasterSerial(INVALID_SERIAL);
			pMount->moveTo(pos());
			pMount->setDirection(dir_);
			pMount->resend(false);
			pMount->bark(Bark_Idle);
		}
		pi->remove();
		resend(false);
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
		pMountItem->setId(0x915);
		pMountItem->setColor(pMount->skin());

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
		pMountItem->setTag("pet", cVariant(pMount->serial()));
		pMountItem->update();

		// if this is a gm lets tame the animal in the process
		if (isGM()) {
			pMount->setOwner( this );
		}

		// remove it from screen!
		pMount->bark(Bark_Idle);
		pMount->removeFromView(false);
		pMount->fight(0);
		pMount->setStablemasterSerial(serial_);
		MapObjects::instance()->remove(pMount);		
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
	if( !socket->player() ) {
		return;
	}

	QString charName = name();

	// Lord & Lady Title
	if( fame_ == 10000 )
		charName.prepend( gender_ ? tr( "Lady " ) : tr( "Lord " ) );

	QString affix = "";

	if (guild_ && !guild_->abbreviation().isEmpty()) {
		affix.append(QString(" [%1]").arg(guild_->abbreviation()));
	}

	// Append serial for GMs
	if( socket->player()->showSerials() )
		affix.append( QString( "[0x%1]" ).arg( serial(), 4, 16 ) );

	// Append offline flag
	if( !socket_ && !logoutTime_ )
		affix.append( tr(" [offline]") );

	// Guarded
	if( guardedby_.size() > 0 )
		affix.append( tr(" [guarded]") );

	// Guarding
	if( isTamed() && guarding_ )
		affix.append( tr(" [guarding]") );

	Q_UINT16 speechColor;

	// 0x01 Blue, 0x02 Green, 0x03 Grey, 0x05 Orange, 0x06 Red
	switch( notoriety( socket->player() ) )
	{
		case 0x01:	speechColor = 0x59;		break; //blue
		case 0x02:	speechColor = 0x3F;		break; //green
		case 0x03:	speechColor = 0x3B2;	break; //grey
		case 0x05:	speechColor = 0x90;		break; //orange
		case 0x06:	speechColor = 0x22;		break; //red
		default:	speechColor = 0x3B2;	break; // grey
	}

	if (isInvulnerable()) {
		speechColor = 0x35;
	}

	// Show it to the socket
	// socket->showSpeech( this, charName, speechColor, 3, cUOTxUnicodeSpeech::System );
	// Names are presented in ASCII speech, Guild titles are not
	socket->clilocMessage(1050045, " \t" + charName + "\t " + affix, speechColor, 3, this, true);
}

/*!
	Make someone criminal.
*/
void cPlayer::makeCriminal() {
	if (!isGMorCounselor()) {
		// Murderers dont become criminal, they already are!
		if (!this->isMurderer()) {
			// Notify us if we're not already a criminal
			if (socket_ && !isCriminal()) {
				socket_->clilocMessage(500167);
			}

			setCriminalTime(uiCurrentTime + SrvParams->crimtime() * MY_CLOCKS_PER_SEC);
			changed_ = true;
		}
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

	if( !pi )
	{
		pi = cItem::createFromScript( "9ab" );
		pi->setOwner( this );
		pi->setType( 1 );
		pi->setName( tr( "%1's bank box" ).arg( name() ) );
		addItem( BankBox, pi, true, true );
	}

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

/*!
	Reduces the specified \a amount of gold from the user
	and returns the amount reduced. If it was successfull, it will return
	the same value passed on \a amount parameter.
*/
UINT32 cPlayer::takeGold( UINT32 amount, bool useBank )
{
	P_ITEM pPack = getBackpack();

	UINT32 dAmount = 0;

	if( pPack )
		dAmount = pPack->DeleteAmount( amount, 0xEED, 0 );

	if( ( dAmount > 0 ) && useBank )
	{
		P_ITEM pBank = getBankBox();

		if( pBank )
			dAmount += pBank->DeleteAmount( (amount-dAmount), 0xEED, 0 );
	}

	goldSound( amount, false );

	return amount - dAmount;
}

bool cPlayer::inWorld() {
	return socket_ != 0;
}

void cPlayer::giveNewbieItems( Q_UINT8 skill )
{
	const cElement *startItems = DefManager->getDefinition( WPDT_STARTITEMS, ( skill == 0xFF ) ? QString("default") : Skills->getSkillDef( skill ).lower() );

	// No Items defined
	if( !startItems )
		return;

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
			giveGold( node->value().toUInt() );
		}

		// Item related nodes
		else
		{
			P_ITEM pItem = 0;
			const QString &id = node->getAttribute("id");

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
					pItem->setLayer(0);

					if( !layer )
					{
						tile_st tile = TileCache::instance()->getTile( pItem->id() );
						layer = tile.layer;
					}

					if( layer )
					{
						// Check if there is sth there already.
						// Could happen due to inherit.
						P_ITEM existing = atLayer(static_cast<cBaseChar::enLayer>(layer));
						if ( existing )
							existing->remove();
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
					Console::instance()->log( LOG_ERROR, QString( "Unrecognized startitem tag '%1' in definition '%2'.").arg( node->name() ).arg( element->getAttribute( "id" ) ) );
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

bool cPlayer::onTradeStart( P_PLAYER partner, P_ITEM firstitem )
{
	cPythonScript *global = ScriptManager::instance()->getGlobalHook( EVENT_TRADESTART );
	bool result = false;

	if( scriptChain || global )
	{
		PyObject *args = Py_BuildValue( "(O&O&O&)", PyGetCharObject, this, PyGetCharObject, partner, PyGetItemObject, firstitem );

		result = cPythonScript::callChainedEventHandler( EVENT_TRADESTART, scriptChain, args );

		if( !result && global )
			result = global->callEventHandler( EVENT_TRADESTART, args );

		Py_DECREF( args );
	}

	return result;
}

bool cPlayer::onTrade( unsigned int type, unsigned int buttonstate, SERIAL itemserial )
{
	cPythonScript *global = ScriptManager::instance()->getGlobalHook( EVENT_TRADE );
	bool result = false;

	if( scriptChain || global )
	{
		PyObject *args = Py_BuildValue( "(O&iii)", PyGetCharObject, this, type, buttonstate, itemserial );

		result = cPythonScript::callChainedEventHandler( EVENT_TRADE, scriptChain, args );

		if( !result && global )
			result = global->callEventHandler( EVENT_TRADE, args );

		Py_DECREF( args );
	}

	return result;
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
	else SET_INT_PROPERTY( "fixedlight", fixedLightLevel_ )
	else if( name == "inputmode" )
	{
		inputMode_ = (enInputMode)value.toInt();
		return 0;
	}
	else SET_INT_PROPERTY( "inputitem", inputItem_ )
	else SET_INT_PROPERTY( "objectdelay", objectDelay_ )
	else SET_INT_PROPERTY( "visrange", visualRange_ )
	else SET_STR_PROPERTY( "profile", profile_ )

	else SET_INT_PROPERTY( "strengthlock", strengthLock_ )
	else SET_INT_PROPERTY( "dexteritylock", dexterityLock_ )
	else SET_INT_PROPERTY( "intelligencelock", intelligenceLock_ )
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
	else GET_PROPERTY( "npc", false )
	else GET_PROPERTY( "fixedlight", fixedLightLevel_ )
	else GET_PROPERTY( "inputmode", inputMode_ )
	else GET_PROPERTY( "inputitem", FindItemBySerial( inputItem_ ) )
	else GET_PROPERTY( "objectdelay", (int)objectDelay_ )
	else GET_PROPERTY( "visrange", visualRange_ )
	else GET_PROPERTY( "profile", profile_ )
	else GET_PROPERTY( "strengthlock", strengthLock_ )
	else GET_PROPERTY( "dexteritylock", dexterityLock_ )
	else GET_PROPERTY( "intelligencelock", intelligenceLock_ )
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

	uint message = 1019051;

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

	UINT32 message = 0xF8CB3;

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

bool cPlayer::canSeeChar(P_CHAR character) {
	if (character != this) {
		if (!character || character->free) {
			return false;
		}

		// Check distance
		if (pos_.distance(character->pos()) > visualRange()) {
			return false;
		}

		P_PLAYER player = dynamic_cast<P_PLAYER>(character);

		// By default we are mor privileged than our target if we are a gm
		bool privileged = isGM();

		// Disconnected players are invisible unless allShow is active for the current account
		if (player) {
			if (!player->socket() && !player->logoutTime() && (!account_ || !account_->isAllShow())) {
				return false;
			}

			if (privileged) {
				// Determine if we are more privileged than the target
				if (player->account()) {
					if (!account_ || player->account()->rank() > account_->rank()) {
						privileged = false;
					}
				}
			}
			
			if (party_ && party_ == player->party()) {
				privileged = true;
			}
		}

		// Hidden and invisible characters are invisible unless we are more privileged than them
		if (character->isInvisible() || character->isHidden()) {
			if (!privileged) {
				return false;
			}
		}

		// Dead characters are invisible unless we have more than 100.0% spirit speak 
		// or are privileged...
		if (character->isDead()) {
			// Only NPCs with spiritspeak >= 1000 can see dead people
			// or if the AI overrides it
			if (!character->isAtWar() && skillValue(SPIRITSPEAK) < 1000) {
				if (!privileged) {
					return false;
				}
			}
		}

		// Check if the target is a npc and currently stabled
		P_NPC npc = dynamic_cast<P_NPC>(character);
		if (npc && npc->stablemasterSerial() != INVALID_SERIAL) {
			return false;
		}
	}

	return true;
}

bool cPlayer::canSeeItem(P_ITEM item) {
	if (!item) {
		return false;
	}

	if (item->visible() == 2) {
		if (!isGM()) {
			return false;
		}
	} else if (item->visible() == 1 && item->owner() != this) {
		if (!isGM()) {
			return false;
		}
	}

	// Check for container
	if (item->container()) {
		P_ITEM container = dynamic_cast<P_ITEM>(item->container());

		if (container) {
			return canSeeItem(container);
		} else {
			P_CHAR character = dynamic_cast<P_CHAR>(item->container());
			return canSeeChar(character);
		}		
	} else {
		if (pos_.distance(item->pos()) > visualRange()) {
				return false;
		}
	}

	return true;
}

PyObject *cPlayer::getPyObject() {
	return PyGetCharObject(this);
}

const char *cPlayer::className() const {
	return "player";
}

bool cPlayer::send(cUOPacket *packet) {
	if (socket_) {
		socket_->send(packet);
		return true;
	}
	return false;
}

bool cPlayer::message(const QString &message, unsigned short color, cUObject *source, unsigned short font, unsigned char mode) {
	if (socket_) {
		if (!source) {
			socket_->showSpeech(this, message, color, font, mode);
		} else {
			socket_->showSpeech(source, message, color, font, mode);
		}
		return true;
	} else {
		return false;
	}
}

bool cPlayer::sysmessage(const QString &message, unsigned short color, unsigned short font) {
	if (socket_) {
		socket_->sysMessage(message, color, font);
		return true;
	}
	return false;
}

bool cPlayer::sysmessage(unsigned int message, const QString &params, unsigned short color, unsigned short font) {
	if (socket_) {
		socket_->clilocMessage(message, params, color, font);
		return true;
	}
	return false;
}

cBaseChar::FightStatus cPlayer::fight(P_CHAR enemy) {
	FightStatus status = cBaseChar::fight(enemy);
	
	// We have player dependant actions if the fight started
	if (status != FightDenied) {
		// Notoriety handling only if the fight started
		if (status == FightStarted) {
			cFightInfo *fight = findFight(enemy);
		
			if (fight && fight->attacker() == this && !fight->legitimate()) {
				makeCriminal();
			}
		}

		// Always stop meditating
		disturbMed();
	}

	return status;
}

void cPlayer::createTooltip(cUOTxTooltipList &tooltip, cPlayer *player) {
	cUObject::createTooltip(tooltip, player);

	QString affix = " ";

	// Append the (frozen) tag
	if (isFrozen()) {
		affix = " (frozen)";
	}

	// Don't miss lord and lady titles
	if (fame_ >= 1000) {
		if (gender()) {
			tooltip.addLine(1050045, tr(" \tLady %1\t%2").arg(name_).arg(affix));
		} else {
			tooltip.addLine(1050045, tr(" \tLord %1\t%2").arg(name_).arg(affix));
		}
	} else {
		tooltip.addLine(1050045, QString(" \t%1\t%2").arg(name_).arg(affix));
	}

	// Append guild title and name
	if (guild_) {
		cGuild::MemberInfo *info = guild_->getMemberInfo(this);

		if (info) {
			if (!info->guildTitle().isEmpty()) {
				tooltip.addLine(1042971, QString("%1, %2").arg(info->guildTitle()).arg(guild_->name()));
			} else {
				tooltip.addLine(1042971, guild_->name());
			}
		}
	}

	onShowTooltip(player, &tooltip);
}
