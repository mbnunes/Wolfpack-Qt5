/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: http://wpdev.sf.net/
 */

// wolfpack includes
#include "accounts.h"
#include "basechar.h"
#include "basedef.h"
#include "corpse.h"
#include "npc.h"
#include "party.h"
#include "player.h"
#include "world.h"
#include "persistentbroker.h"
#include "dbdriver.h"
#include "console.h"
#include "muls/maps.h"
#include "inlines.h"
#include "mapobjects.h"
#include "multi.h"
#include "network/uosocket.h"
#include "network/uotxpackets.h"
#include "network/network.h"
#include "combat.h"
#include "items.h"
#include "basics.h"
#include "muls/tilecache.h"
#include "pythonscript.h"
#include "scriptmanager.h"
#include "log.h"
#include "skills.h"
#include "definitions.h"
#include "serverconfig.h"

cBaseChar::cBaseChar()
{
	fights_.setAutoDelete( false );
	basedef_ = 0;
	stepsTaken_ = 0;
	lastMovement_ = 0;
	attackTarget_ = 0;
	nextSwing_ = 0;
	body_ = 0x190;
	orgBody_ = 0x190;
	gender_ = 0;
	orgSkin_ = 0;
	propertyFlags_ = 0;
	weight_ = 0;
	dexterity_ = 0;
	dexterityMod_ = 0;
	maxStamina_ = 0;
	stamina_ = 0;
	strength_ = 0;
	strengthMod_ = 0;
	maxHitpoints_ = 0;
	hitpoints_ = 0;
	intelligence_ = 0;
	intelligenceMod_ = 0;
	maxMana_ = 0;
	mana_ = 0;
	karma_ = 0;
	fame_ = 0;
	kills_ = 0;
	deaths_ = 0;
	hunger_ = 6;
	hungerTime_ = 0;
	flag_ = 0x02;
	emoteColor_ = 0x23;
	creationDate_ = QDateTime::currentDateTime();
	stealthedSteps_ = 0;
	running_ = false;
	murdererTime_ = 0;
	criminalTime_ = 0;
	skillDelay_ = 0;
	poison_ = -1;
	title_ = "";
	cUObject::name_ = "Man";
	orgName_ = "Man";
	skin_ = 0;
	region_ = NULL;
	saycolor_ = 0x1700;
	murdererSerial_ = INVALID_SERIAL;
	guarding_ = NULL;
	cUObject::pos_ = Coord( 100, 100, 0, 0 );
	setDead( false );  // we want to live ;)

	saycolor_ = 600;
	hitpointsBonus_ = 0;
	staminaBonus_ = 0;
	manaBonus_ = 0;
	strengthCap_ = 125;
	dexterityCap_ = 125;
	intelligenceCap_ = 125;
	statCap_ = Config::instance()->statcap();
	skills_.resize( ALLSKILLS );
	regenHitpointsTime_ = 0;
	regenStaminaTime_ = 0;
	regenManaTime_ = 0;
}

cBaseChar::cBaseChar( const cBaseChar& right ) : cUObject(right)
{
}

cBaseChar::~cBaseChar()
{
}

cBaseChar& cBaseChar::operator=( const cBaseChar& /*right*/ )
{
	return *this;
}

void cBaseChar::buildSqlString( const char *objectid, QStringList& fields, QStringList& tables, QStringList& conditions )
{
	cUObject::buildSqlString( objectid, fields, tables, conditions );
	fields.push_back( "characters.name,characters.title,characters.creationdate" );
	fields.push_back( "characters.body,characters.orgbody,characters.skin" );
	fields.push_back( "characters.orgskin,characters.saycolor" );
	fields.push_back( "characters.emotecolor,characters.strength,characters.strengthmod,characters.dexterity" );
	fields.push_back( "characters.dexteritymod,characters.intelligence,characters.intelligencemod" );
	fields.push_back( "characters.maxhitpoints,characters.hitpoints,characters.maxstamina,characters.stamina" );
	fields.push_back( "characters.maxmana,characters.mana" );
	fields.push_back( "characters.karma,characters.fame" );
	fields.push_back( "characters.kills,characters.deaths" );
	fields.push_back( "characters.hunger" );
	fields.push_back( "characters.poison" );
	fields.push_back( "characters.murderertime,characters.criminaltime" );
	fields.push_back( "characters.gender,characters.propertyflags" );
	fields.push_back( "characters.murderer" );
	fields.push_back( "characters.guarding" );
	fields.push_back( "characters.hitpointsbonus,characters.staminabonus,characters.manabonus" );
	fields.push_back( "characters.strcap,characters.dexcap,characters.intcap,characters.statcap" );
	fields.push_back( "characters.baseid,characters.direction" );
	tables.push_back( "characters" );
	conditions.push_back( "uobjectmap.serial = characters.serial" );
}

static void characterRegisterAfterLoading( P_CHAR pc );

void cBaseChar::load( cBufferedReader& reader, unsigned int version )
{
	cUObject::load( reader, version );

	orgName_ = reader.readUtf8();
	title_ = reader.readUtf8();
	creationDate_ = QDateTime::fromString( reader.readUtf8(), Qt::ISODate );
	body_ = reader.readShort();
	orgBody_ = reader.readShort();
	skin_ = reader.readShort();
	orgSkin_ = reader.readShort();
	saycolor_ = reader.readShort();
	emoteColor_ = reader.readShort();
	strength_ = reader.readShort();
	strengthMod_ = reader.readShort();
	dexterity_ = reader.readShort();
	dexterityMod_ = reader.readShort();
	intelligence_ = reader.readShort();
	intelligenceMod_ = reader.readShort();
	maxHitpoints_ = reader.readShort();
	hitpoints_ = reader.readShort();
	maxStamina_ = reader.readShort();
	stamina_ = reader.readShort();
	maxMana_ = reader.readShort();
	mana_ = reader.readShort();
	karma_ = reader.readShort();
	fame_ = reader.readShort();
	kills_ = reader.readShort();
	deaths_ = reader.readShort();
	hunger_ = reader.readByte();
	poison_ = reader.readByte();
	murdererTime_ = reader.readInt();
	if ( murdererTime_ )
	{
		murdererTime_ += Server::instance()->time();
	}
	criminalTime_ = reader.readInt();
	if ( criminalTime_ )
	{
		criminalTime_ += Server::instance()->time();
	}
	gender_ = reader.readByte();
	propertyFlags_ = reader.readInt();
	murdererSerial_ = reader.readInt();
	guarding_ = reinterpret_cast<P_CHAR>( static_cast<size_t>(reader.readInt()) ); // PostProcess
	hitpointsBonus_ = reader.readShort();
	staminaBonus_ = reader.readShort();
	manaBonus_ = reader.readShort();
	strengthCap_ = reader.readByte();
	dexterityCap_ = reader.readByte();
	intelligenceCap_ = reader.readByte();
	statCap_ = reader.readByte();
	basedef_ = CharBaseDefs::instance()->get( reader.readAscii() );
	direction_ = reader.readByte();

	// Load Skills
	unsigned int count = ALLSKILLS;
	for ( unsigned int s = 0; s < count; ++s )
	{
		// Read value, cap, lock
		setSkillValue( s, reader.readShort() );
		setSkillCap( s, reader.readShort() );
		setSkillLock( s, reader.readByte() );
	}
}

void cBaseChar::postload( unsigned int /*version*/ )
{
	// Resolve the guarding_ value.
	SERIAL guarding = static_cast<SERIAL>( reinterpret_cast<size_t>( guarding_ ) );
	guarding_ = 0;
	setGuarding( World::instance()->findChar( guarding ) );
}

void cBaseChar::save( cBufferedWriter& writer, unsigned int version )
{
	cUObject::save( writer, version );

	writer.writeUtf8( orgName_ );
	writer.writeUtf8( title_ );
	writer.writeUtf8( creationDate_.toString( Qt::ISODate ) );
	writer.writeShort( body_ );
	writer.writeShort( orgBody_ );
	writer.writeShort( skin_ );
	writer.writeShort( orgSkin_ );
	writer.writeShort( saycolor_ );
	writer.writeShort( emoteColor_ );
	writer.writeShort( strength_ );
	writer.writeShort( strengthMod_ );
	writer.writeShort( dexterity_ );
	writer.writeShort( dexterityMod_ );
	writer.writeShort( intelligence_ );
	writer.writeShort( intelligenceMod_ );
	writer.writeShort( maxHitpoints_ );
	writer.writeShort( hitpoints_ );
	writer.writeShort( maxStamina_ );
	writer.writeShort( stamina_ );
	writer.writeShort( maxMana_ );
	writer.writeShort( mana_ );
	writer.writeShort( karma_ );
	writer.writeShort( fame_ );
	writer.writeShort( kills_ );
	writer.writeShort( deaths_ );
	writer.writeByte( hunger_ );
	writer.writeByte( poison_ );
	writer.writeInt( murdererTime_ ? murdererTime_ - Server::instance()->time() : 0 );
	writer.writeInt( criminalTime_ ? criminalTime_ - Server::instance()->time() : 0 );
	writer.writeByte( gender_ );
	writer.writeInt( propertyFlags_ );
	writer.writeInt( murdererSerial_ );
	writer.writeInt( guarding_ ? guarding_->serial() : INVALID_SERIAL );
	writer.writeShort( hitpointsBonus_ );
	writer.writeShort( staminaBonus_ );
	writer.writeShort( manaBonus_ );
	writer.writeByte( strengthCap_ );
	writer.writeByte( dexterityCap_ );
	writer.writeByte( intelligenceCap_ );
	writer.writeByte( statCap_ );
	writer.writeAscii( baseid() );
	writer.writeByte( direction_ );

	// Load Skills
	unsigned int count = ALLSKILLS;
	for ( unsigned int s = 0; s < count; ++s )
	{
		// Read value, cap, lock
		writer.writeShort( skillValue( s ) );
		writer.writeShort( skillCap( s ) );
		writer.writeByte( skillLock( s ) );
	}
}

void cBaseChar::load( char** result, Q_UINT16& offset )
{
	cUObject::load( result, offset );

	// Broken Serial?
	if ( !isCharSerial( serial() ) )
		throw QString( "Character has invalid char serial: 0x%1" ).arg( serial(), 0, 16 );

	SERIAL ser;

	orgName_ = result[offset++];
	title_ = QString::fromUtf8(result[offset++]);
	creationDate_ = QDateTime::fromString( result[offset++], Qt::ISODate );
	body_ = atoi( result[offset++] );
	orgBody_ = atoi( result[offset++] );
	skin_ = atoi( result[offset++] );
	orgSkin_ = atoi( result[offset++] );
	saycolor_ = atoi( result[offset++] );
	emoteColor_ = atoi( result[offset++] );
	strength_ = atoi( result[offset++] );
	strengthMod_ = atoi( result[offset++] );
	dexterity_ = atoi( result[offset++] );
	dexterityMod_ = atoi( result[offset++] );
	intelligence_ = atoi( result[offset++] );
	intelligenceMod_ = atoi( result[offset++] );
	maxHitpoints_ = atoi( result[offset++] );
	hitpoints_ = atoi( result[offset++] );
	maxStamina_ = atoi( result[offset++] );
	stamina_ = atoi( result[offset++] );
	maxMana_ = atoi( result[offset++] );
	mana_ = atoi( result[offset++] );
	karma_ = atoi( result[offset++] );
	fame_ = atoi( result[offset++] );
	kills_ = atoi( result[offset++] );
	deaths_ = atoi( result[offset++] );
	hunger_ = atoi( result[offset++] );
	poison_ = ( char ) atoi( result[offset++] );
	murdererTime_ = atoi( result[offset++] );
	if (murdererTime_ != 0) {
		 murdererTime_ += Server::instance()->time();
	}
	criminalTime_ = atoi( result[offset++] );
	if (criminalTime_ != 0) {
		 criminalTime_ += Server::instance()->time();
	}
	gender_ = atoi( result[offset++] );
	propertyFlags_ = atoi( result[offset++] );
	murdererSerial_ = atoi( result[offset++] );
	ser = atoi( result[offset++] );
	guarding_ = dynamic_cast<P_PLAYER>( FindCharBySerial( ser ) );
	hitpointsBonus_ = atoi( result[offset++] );
	staminaBonus_ = atoi( result[offset++] );
	manaBonus_ = atoi( result[offset++] );
	strengthCap_ = atoi( result[offset++] );
	dexterityCap_ = atoi( result[offset++] );
	intelligenceCap_ = atoi( result[offset++] );
	statCap_ = atoi( result[offset++] );
	basedef_ = CharBaseDefs::instance()->get( result[offset++] );
	direction_ = atoi( result[offset++] );

	// Query the Skills for this character
	QString sql = "SELECT skill,value,locktype,cap FROM skills WHERE serial = '" + QString::number( serial() ) + "'";

	cDBResult res = PersistentBroker::instance()->query( sql );
	if ( !res.isValid() )
		throw PersistentBroker::instance()->lastError();

	// Fetch row-by-row
	while ( res.fetchrow() )
	{
		// row[0] = skill
		// row[1] = value
		// row[2] = locktype
		// row[3] = cap (unused!)
		Q_UINT16 skill = res.getInt( 0 );
		Q_UINT16 value = res.getInt( 1 );
		Q_UINT8 lockType = res.getInt( 2 );
		Q_UINT16 cap = res.getInt( 3 );

		if ( lockType > 2 )
			lockType = 0;

		stSkillValue skValue;
		skValue.value = value;
		skValue.lock = lockType;
		skValue.cap = cap;
		skValue.changed = false;

		skills_[skill] = skValue;
	}

	res.free();

	characterRegisterAfterLoading( this );
	changed_ = false;
}

void cBaseChar::save( cBufferedWriter& writer )
{
	cUObject::save( writer );

	// Save equipment
	ItemContainer::iterator it = content_.begin();
	for ( ; it != content_.end(); ++it )
	{
		it.data()->save( writer );
	}
}

void cBaseChar::save()
{
	if ( changed_ )
	{
		initSave;
		setTable( "characters" );

		addField( "serial", serial() );
		addStrField( "name", orgName_ );
		addStrField( "title", title_ );
		addStrField( "creationdate", creationDate_.toString( Qt::ISODate ) );
		addField( "body", body_ );
		addField( "orgbody", orgBody_ );
		addField( "skin", skin_ );
		addField( "orgskin", orgSkin_ );
		addField( "saycolor", saycolor_ );
		addField( "emotecolor", emoteColor_ );
		addField( "strength", strength_ );
		addField( "strengthmod", strengthMod_ );
		addField( "dexterity", dexterity_ );
		addField( "dexteritymod", dexterityMod_ );
		addField( "intelligence", intelligence_ );
		addField( "intelligencemod", intelligenceMod_ );
		addField( "maxhitpoints", maxHitpoints_ );
		addField( "hitpoints", hitpoints_ );
		addField( "maxstamina", maxStamina_ );
		addField( "stamina", stamina_ );
		addField( "maxmana", maxMana_ );
		addField( "mana", mana_ );
		addField( "karma", karma_ );
		addField( "fame", fame_ );
		addField( "kills", kills_ );
		addField( "deaths", deaths_ );
		addField( "hunger", hunger_ );
		addField( "poison", poison_ );
		addField( "murderertime", murdererTime_ ? murdererTime_ - Server::instance()->time() : 0 );
		addField( "criminaltime", criminalTime_ ? criminalTime_ - Server::instance()->time() : 0 );
		addField( "gender", gender_ );
		addField( "propertyflags", propertyFlags_ );
		addField( "murderer", murdererSerial_ );
		addField( "guarding", guarding_ ? guarding_->serial() : INVALID_SERIAL );
		addField( "hitpointsbonus", hitpointsBonus_ );
		addField( "staminabonus", staminaBonus_ );
		addField( "manabonus", manaBonus_ );
		addField( "strcap", strengthCap_ );
		addField( "dexcap", dexterityCap_ );
		addField( "intcap", intelligenceCap_ );
		addField( "statcap", statCap_ );
		addStrField( "baseid", baseid() );
		addField( "direction", direction_ );
		addCondition( "serial", serial() );
		saveFields;
	}

	QValueVector<stSkillValue>::iterator it;
	PersistentBroker::instance()->lockTable( "skills" );
	int i = 0;
	QCString query( 256 ); // 256 byte should be enough
	for ( it = skills_.begin(); it != skills_.end(); ++it, ++i )
	{
		if ( ( *it ).changed )
		{
			query.sprintf( "REPLACE INTO skills VALUES(%u,%u,%u,%u,%u);", serial_, i, ( *it ).value, ( *it ).lock, ( *it ).cap );
			PersistentBroker::instance()->executeQuery( query.data() );
			( *it ).changed = false;
		}
	}
	PersistentBroker::instance()->unlockTable( "skills" );
	cUObject::save();
}

bool cBaseChar::del()
{
	if ( !isPersistent )
		return false; // We didn't need to delete the object

	PersistentBroker::instance()->addToDeleteQueue( "characters", QString( "serial = '%1'" ).arg( serial() ) );
	PersistentBroker::instance()->addToDeleteQueue( "skills", QString( "serial = '%1'" ).arg( serial() ) );
	changed_ = true;
	return cUObject::del();
}

static void characterRegisterAfterLoading( P_CHAR pc )
{
	World::instance()->registerObject( pc );
}

bool cBaseChar::isMurderer() const
{
	return murdererTime_ > Server::instance()->time();
}

bool cBaseChar::isCriminal() const
{
	return criminalTime_ > Server::instance()->time();
}

// Send the changed health-bar to all sockets in range
void cBaseChar::updateHealth()
{
	MapCharsIterator cIter = MapObjects::instance()->listCharsInCircle( pos(), 18 );
	for( P_CHAR pChar = cIter.first(); pChar; pChar = cIter.next() )
	{
		P_PLAYER pPlayer = dynamic_cast<P_PLAYER>( pChar );

		// Send only if target can see us
		if ( !pPlayer || !pPlayer->socket() || !pPlayer->inRange( this, pPlayer->visualRange() ) || ( isHidden() && !pPlayer->isGM() && this != pPlayer ) )
			continue;

		pPlayer->socket()->updateHealth( this );
	}
}

void cBaseChar::action( unsigned char id, unsigned char speed, bool reverse )
{
	if( isAntiBlink() )
	{
		if( id == ANIM_CAST_DIR )
			id = ANIM_CAST_AREA;
	}

	bool mounted = atLayer( Mount ) != 0;

	// Bow + Area Cast
	if ( mounted )
	{
		switch ( id )
		{
		case ANIM_WALK_UNARM:
		case ANIM_WALK_ARM:
		case ANIM_WALK_WAR:
			id = ANIM_HORSE_RIDE_SLOW;
			break;
		case ANIM_RUN_UNARM:
		case ANIM_RUN_ARMED:
			id = ANIM_HORSE_RIDE_FAST;
			break;
		case ANIM_FIDGET1:
		case ANIM_FIDGET_YAWN:
		case ANIM_GET_HIT:
		case ANIM_BLOCK:
			id = ANIM_HORSE_SLAP;
			break;
		case ANIM_ATTACK_1H_WIDE:
		case ANIM_ATTACK_1H_JAB:
		case ANIM_ATTACK_1H_DOWN:
		case ANIM_CAST_DIR:
		case ANIM_ATTACK_UNARM:
		case ANIM_ATTACK_2H_JAB:
		case ANIM_ATTACK_2H_WIDE:
		case ANIM_ATTACK_2H_DOWN:
			id = ANIM_HORSE_ATTACK;
			break;
		case ANIM_CAST_AREA:
		case ANIM_ATTACK_BOW:
			id = ANIM_HORSE_ATTACK_BOW;
			break;
		case ANIM_ATTACK_XBOW:
		case ANIM_BOW:
		case ANIM_SALUTE:
		case ANIM_EAT:
			id = ANIM_HORSE_ATTACK_XBOW;
			break;
		case ANIM_STAND:
		case ANIM_STAND_WAR_1H:
		case ANIM_STAND_WAR_2H:
		default:
			id = ANIM_HORSE_STAND;
			break;
		}
	}

	if ( body_ < 0x190 && body_ >= 0xc8 )
	{
		// Animal specific translation
		switch ( id )
		{
		case ANIM_WALK_UNARM:
		case ANIM_WALK_ARM:
		case ANIM_WALK_WAR:
			id = ANIM_ANI_WALK;
			break;
		case ANIM_RUN_UNARM:
		case ANIM_RUN_ARMED:
			id = ANIM_ANI_RUN;
			break;
		case ANIM_STAND:
		case ANIM_STAND_WAR_1H:
		case ANIM_STAND_WAR_2H:
		default:
			id = ANIM_ANI_STAND;
			break;
		case ANIM_FIDGET1:
			id = ANIM_ANI_FIDGET1;
			break;
		case ANIM_FIDGET_YAWN:
			id = ANIM_ANI_FIDGET2;
			break;
		case ANIM_CAST_DIR:
			id = ANIM_ANI_ATTACK1;
			break;
		case ANIM_CAST_AREA:
			id = ANIM_ANI_EAT;
			break;
		case ANIM_GET_HIT:
			id = ANIM_ANI_GETHIT;
			break;
		case ANIM_ATTACK_1H_WIDE:
		case ANIM_ATTACK_1H_JAB:
		case ANIM_ATTACK_1H_DOWN:
		case ANIM_ATTACK_2H_DOWN:
		case ANIM_ATTACK_2H_JAB:
		case ANIM_ATTACK_2H_WIDE:
		case ANIM_ATTACK_BOW:
		case ANIM_ATTACK_XBOW:
		case ANIM_ATTACK_UNARM:
			switch ( RandomNum( 0, 1 ) )
			{
			case 0:
				id = ANIM_ANI_ATTACK1;
				break;
			case 1:
				id = ANIM_ANI_ATTACK2;
				break;
			}
			break;
		case ANIM_DIE_BACK:
			id = ANIM_ANI_DIE1;
			break;
		case ANIM_DIE_FORWARD:
			id = ANIM_ANI_DIE2;
			break;
		case ANIM_BLOCK:
		case ANIM_BOW:
		case ANIM_SALUTE:
			id = ANIM_ANI_SLEEP;
			break;
		case ANIM_EAT:
			id = ANIM_ANI_EAT;
			break;
		}
	}

	cUOTxAction action;
	action.setAction( id );
	action.setSerial( serial() );
	action.setDirection( direction() );
	action.setRepeat( 1 );
	action.setRepeatFlag( 0 );
	action.setSpeed( speed );
	if ( reverse )
	{
		action.setBackwards( 1 );
	}

	for ( cUOSocket*socket = Network::instance()->first(); socket; socket = Network::instance()->next() )
	{
		if ( socket->player() && socket->player()->inRange( this, socket->player()->visualRange() ) && ( !isHidden() || socket->player()->isGM() ) )
			socket->send( &action );
	}
}

P_ITEM cBaseChar::getWeapon() const
{
	// Check if we have something on our right hand
	P_ITEM rightHand = rightHandItem();
	if ( rightHand && rightHand->type() >= 1001 && rightHand->type() <= 1007 )
		return rightHand;

	// Check for two-handed weapons
	P_ITEM leftHand = leftHandItem();
	if ( leftHand && leftHand->type() >= 1001 && leftHand->type() <= 1007 )
		return leftHand;

	return 0;
}

P_ITEM cBaseChar::getShield() const
{
	P_ITEM leftHand = leftHandItem();
	if ( leftHand && leftHand->isShield() )
		return leftHand;
	return NULL;
}

void cBaseChar::setHairColor( Q_UINT16 d )
{
	changed_ = true;
	cItem* pHair = getItemOnLayer( 11 );
	if ( pHair )
		pHair->setColor( d );
	pHair->update();
}

void cBaseChar::setHairStyle( Q_UINT16 d )
{
	if ( !isHair( d ) )
		return;
	changed_ = true;
	cItem* pHair = getItemOnLayer( 11 );
	if ( pHair )
	{
		pHair->setId( d );
	}
	else
	{
		pHair = new cItem;
		pHair->Init();

		pHair->setDye( 1 );
		pHair->setNewbie( true );
		pHair->setId( d );
		addItem( cBaseChar::Hair, pHair );
	}
	pHair->update();
}

void cBaseChar::setBeardColor( Q_UINT16 d )
{
	changed_ = true;
	cItem* pBeard = getItemOnLayer( 16 );
	if ( pBeard )
		pBeard->setColor( d );
	pBeard->update();
}

void cBaseChar::setBeardStyle( Q_UINT16 d )
{
	if ( !isBeard( d ) )
		return;
	changed_ = true;
	cItem* pBeard = getItemOnLayer( 16 );
	if ( pBeard )
		pBeard->setId( d );
	else
	{
		pBeard = new cItem;
		pBeard->Init();

		pBeard->setDye( 1 );
		pBeard->setNewbie( true );
		pBeard->setId( d );
		addItem( cBaseChar::FacialHair, pBeard );
	}
	pBeard->update();
}


// This should check soon if we are standing above our
// corpse and if so, merge with our corpse instead of
// just resurrecting
bool cBaseChar::resurrect( cUObject* source )
{
	if ( !isDead() )
		return false;

	if ( onResurrect( source ) )
		return false;

	cCorpse* corpse = 0;

	// See if there's his corpse at his feet
	MapItemsIterator iter = MapObjects::instance()->listItemsAtCoord( pos() );
	for ( P_ITEM item = iter.first(); item; item = iter.next() )
	{
		corpse = dynamic_cast<cCorpse *>( item );

		if ( !corpse || corpse->owner() != this )
		{
			corpse = 0;
		}
		else
		{
			break;
		}
	}

	if ( corpse && corpse->direction() != direction() )
	{
		setDirection( corpse->direction() );
		update();
	}

	changed( TOOLTIP );
	changed_ = true;
	awardFame( 0 );
	soundEffect( 0x0214 );
	setBody( orgBody_ );
	setSkin( orgSkin_ );	
	setDead( false );
	hitpoints_ = wpMax<short>( 1, static_cast<short>( 0.1 * maxHitpoints_ ) );
	stamina_ = static_cast<short>( 0.5 * maxStamina_ );
	mana_ = static_cast<short>( 0.5 * maxMana_ );
	fight( 0 );
	P_ITEM backpack = getBackpack(); // Make sure he has a backpack

	// Delete what the user wears on layer 0x16 (Should be death shroud)
	P_ITEM pRobe = atLayer( OuterTorso );

	if ( pRobe )
	{
		pRobe->remove();
	}

	if ( !corpse )
	{
		pRobe = cItem::createFromScript( "1f03" );

		if ( pRobe )
		{
			pRobe->setColor( 0 );
			pRobe->setHp( 1 );
			pRobe->setMaxhp( 1 );
			this->addItem( cBaseChar::OuterTorso, pRobe );
			pRobe->update();
		}

		resend( false );
	}
	else
	{
		// Move all items from the corpse to the backpack and then look for
		// previous equipment
		for (ContainerIterator it(corpse); !it.atEnd(); ++it)
		{
			backpack->addItem( *it, false );
			( *it )->update();
		}

		for ( unsigned char layer = SingleHandedWeapon; layer < Mount; layer++ )
		{
			if ( layer != Backpack && layer != Hair && layer != FacialHair )
			{
				P_ITEM item = atLayer( ( enLayer ) layer );

				if ( item )
				{
					backpack->addItem( item );
					item->update();
				}

				SERIAL equipment = corpse->getEquipment( layer );
				item = World::instance()->findItem( equipment );

				if ( item && item->container() == backpack )
				{
					addItem( ( enLayer ) layer, item );
					item->update();
				}
			}
		}

		unsigned char action = 0x15;
		if ( corpse->direction() & 0x80 )
		{
			action = 0x16;
		}

		corpse->remove();

		removeFromView(false); // The skin changed
		resend(false);

		// Let him "stand up"
		this->action( action, 2, true );
	}
	return true;
}

void cBaseChar::turnTo( const Coord& pos )
{
	Q_INT16 xdif = ( Q_INT16 ) ( pos.x - this->pos().x );
	Q_INT16 ydif = ( Q_INT16 ) ( pos.y - this->pos().y );
	Q_UINT8 nDir;

	if ( xdif == 0 && ydif < 0 )
		nDir = 0;
	else if ( xdif > 0 && ydif < 0 )
		nDir = 1;
	else if ( xdif > 0 && ydif == 0 )
		nDir = 2;
	else if ( xdif > 0 && ydif > 0 )
		nDir = 3;
	else if ( xdif == 0 && ydif > 0 )
		nDir = 4;
	else if ( xdif <0 && ydif> 0 )
		nDir = 5;
	else if ( xdif < 0 && ydif == 0 )
		nDir = 6;
	else if ( xdif < 0 && ydif < 0 )
		nDir = 7;
	else
		return;

	if ( nDir != direction() )
	{
		setDirection( nDir );
		update();
	}
}

void cBaseChar::turnTo( cUObject* object )
{
	turnTo( object->pos() );
}

/*!
  Wears the given item and sends an update to those in range.
*/
void cBaseChar::wear( P_ITEM pi )
{
	Q_UINT8 layer = pi->layer();

	if ( !pi->container() )
	{
		pi->setLayer( 0 );
		if ( !layer )
		{
			tile_st tile = TileCache::instance()->getTile( pi->id() );
			layer = tile.layer;
		}
	}

	if ( !layer )
		return;

	this->addItem( static_cast<cBaseChar::enLayer>( layer ), pi );

	cUOTxCharEquipment packet;
	packet.setWearer( this->serial() );
	packet.setSerial( pi->serial() );
	packet.fromItem( pi );
	for ( cUOSocket*socket = Network::instance()->first(); socket != 0; socket = Network::instance()->next() )
		if ( socket->player() && socket->player()->inRange( this, socket->player()->visualRange() ) )
			socket->send( &packet );
}

void cBaseChar::unhide()
{
	setStealthedSteps( 0 );

	if ( isHidden() && !isInvisible() )
	{
		setHidden( false );
		resend( false ); // They cant see us anyway
		sysmessage( 500814 );
	}
}

int cBaseChar::countItems( short ID, short col )
{
	// Dont you think it's better to search the char's equipment as well?
	Q_UINT32 number = 0;
	ItemContainer::const_iterator it(content_.begin());
	ItemContainer::const_iterator end(content_.end());

	for ( ; it != end; ++it )
	{
		P_ITEM pItem = *it;

		if ( !pItem )
			continue;

		if ( ( pItem->id() == ID ) && ( pItem->color() == col ) )
			++number;
	}

	P_ITEM pi = getBackpack();

	if ( pi )
		number = pi->countItems( ID, col );
	return number;
}

int cBaseChar::countGold()
{
	return countItems( 0x0EED );
}

P_ITEM cBaseChar::getItemOnLayer( unsigned char layer )
{
	return atLayer( static_cast<enLayer>( layer ) );
}

P_ITEM cBaseChar::getBankbox()
{
	P_ITEM pi = atLayer( BankBox );

	if ( !pi )
	{
		pi = cItem::createFromScript( "9ab" );
		pi->setOwner( this );
		addItem( BankBox, pi, true, false );
	}

	return pi;
}

P_ITEM cBaseChar::getBackpack()
{
	P_ITEM backpack = atLayer( Backpack );

	// None found so create one
	if ( !backpack )
	{
		backpack = cItem::createFromScript( "e75" );
		backpack->setOwner( this );
		addItem( Backpack, backpack );
		backpack->update();
	}

	return backpack;
}

void cBaseChar::setSerial( const SERIAL ser )
{
	// This is not allowed
	if ( ser == INVALID_SERIAL || ser == serial_ )
		return;

	// is the new serial already occupied?
	P_CHAR other = World::instance()->findChar(ser);
	if (other && other != this) {
		Console::instance()->log(LOG_ERROR, tr("Trying to change the serial of char 0x%1 to the already occupied serial 0x%2.\n").arg(serial_, 0, 16).arg(ser, 0, 16));
		return;
	}

	if ( this->serial() != INVALID_SERIAL )
		World::instance()->unregisterObject( this->serial() );

	cUObject::setSerial( ser );

	World::instance()->registerObject( this );
}

bool cBaseChar::wears( P_ITEM pi )
{
	return ( this == pi->container() );
}

unsigned int cBaseChar::getSkillSum() const
{
	unsigned int sum = 0;

	QValueVector<stSkillValue>::const_iterator it = skills_.begin();
	for ( ; it != skills_.end(); ++it )
		sum += ( *it ).value;

	return sum;		// this *includes* the decimal digit ie. xxx.y
}

void cBaseChar::Init( bool createSerial )
{
	changed( TOOLTIP );
	changed_ = true;
	cUObject::setSerial( INVALID_SERIAL );

	if ( createSerial )
		this->setSerial( World::instance()->findCharSerial() );
}

bool cBaseChar::inGuardedArea()
{
	cTerritory* Region = Territories::instance()->region( this->pos().x, this->pos().y, this->pos().map );
	if ( Region )
		return Region->isGuarded();
	else
		return false;
}

void cBaseChar::emote( const QString& emote, UI16 color )
{
	if ( color == 0xFFFF )
		color = emoteColor_;

	cUOTxUnicodeSpeech textSpeech;
	textSpeech.setSource( serial() );
	textSpeech.setModel( body_ );
	textSpeech.setFont( 3 ); // Default Font
	textSpeech.setType( cUOTxUnicodeSpeech::Emote );
	textSpeech.setName( name() );
	textSpeech.setColor( color );
	textSpeech.setText( emote );

	for ( cUOSocket*mSock = Network::instance()->first(); mSock; mSock = Network::instance()->next() )
		if ( mSock->player() && mSock->player()->inRange( this, mSock->player()->visualRange() ) )
			mSock->send( &textSpeech );
}

bool cBaseChar::checkSkill( UI16 skill, SI32 min, SI32 max, bool advance )
{
	bool success = false;

	/* Returns false if you don't have the skill required. */
	if ( skillValue( skill ) < min )
		return false;

	// How far is the players skill above the required minimum.
	int charrange = abs( skillValue( skill ) - min );

	if ( charrange < 0 )
		charrange = 0;

	// To avoid division by zero
	if ( min == max )
		max += 1;

	// +100 means: *allways* a minimum of 10% for success
	float chance = ( ( ( float ) charrange * 890.0f ) / ( float ) ( abs( max - min ) ) ) + 100.0f;

	if ( chance > 990 )
		chance = 990;	// *allways* a 1% chance of failure

	if ( chance >= RandomNum( 0, 1000 ) )
		success = true;

	// We can only advance when doing things which aren't below our ability
	if ( advance && skillValue( skill ) < max )
	{
		onSkillGain( skill, min, max, success );
	}

	return success;
}

cItem* cBaseChar::atLayer( cBaseChar::enLayer layer ) const
{
	ItemContainer::const_iterator it = content_.find( layer );
	if ( it != content_.end() )
		return it.data();
	return 0;
}

bool cBaseChar::owns( P_ITEM pItem ) const
{
	if ( !pItem )
		return false;

	return ( pItem->owner() == this );
}

void cBaseChar::addGuard( P_CHAR pPet, bool noGuardingChange )
{
	// Check if already existing in the guard list
	for ( CharContainer::iterator iter = guardedby_.begin(); iter != guardedby_.end(); ++iter )
		if ( *iter == pPet )
			return;

	if ( !noGuardingChange )
	{
		if ( pPet->guarding() )
			pPet->guarding()->removeGuard( pPet );

		pPet->setGuarding( this );
	}

	guardedby_.push_back( pPet );
}

void cBaseChar::removeGuard( P_CHAR pPet, bool noGuardingChange )
{
	for ( CharContainer::iterator iter = guardedby_.begin(); iter != guardedby_.end(); ++iter )
		if ( *iter == pPet )
		{
			guardedby_.erase( iter );
			break;
		}

	if ( !noGuardingChange )
		pPet->setGuarding( 0 );
}

void cBaseChar::addTimer( cTimer* timer )
{
	timers_.push_back( timer );
}

void cBaseChar::removeTimer( cTimer* timer )
{
	TimerContainer::iterator iter = timers_.begin();
	while ( iter != timers_.end() )
	{
		if ( ( *iter ) == timer )
		{
			timers_.erase( iter );
			break;
		}
		++iter;
	}
}

void cBaseChar::processNode( const cElement* Tag )
{
	changed_ = true;
	QString TagName = Tag->name();
	QString Value = Tag->value();

	//<backpack>
	//	<color>0x132</color>
	//	<item id="a">
	//	...
	//	<item id="z">
	//</backpack>
	if ( TagName == "backpack" )
	{
		P_ITEM pBackpack = getBackpack(); // This autocreates a backpack

		if ( Tag->childCount() )
			pBackpack->applyDefinition( Tag );
	}

#if !defined(QT_NO_TRANSLATION)
	else if ( TagName == "title" )
	{
		QString context = Tag->getAttribute("context", "@default");
		Value = qApp->translate(context, Value);
		setTitle(Value);
	}
#endif

	//<stat type="str">100</stats>
	else if ( TagName == "stat" )
	{
		if ( Tag->hasAttribute( "type" ) )
		{
			QString statType = Tag->getAttribute( "type" );
			if ( statType == "str" )
			{
				strength_ = Value.toLong();
				if ( maxHitpoints_ == 0 )
					maxHitpoints_ = strength_;
				hitpoints_ = maxHitpoints_;
			}
			else if ( statType == "dex" )
			{
				dexterity_ = Value.toLong();
				if ( maxStamina_ == 0 )
					maxStamina_ = dexterity_;
				stamina_ = maxStamina_;
			}
			else if ( statType == "int" )
			{
				intelligence_ = Value.toLong();
				if ( maxMana_ == 0 )
					maxMana_ = intelligence_;
				mana_ = maxMana_;
			}
			else if ( statType == "maxhp" || statType == "maxhitpoints" )
			{
				maxHitpoints_ = Value.toLong();
				hitpoints_ = maxHitpoints_;
			}
			else if ( statType == "maxstm" || statType == "maxstamina" )
			{
				maxStamina_ = Value.toLong();
				stamina_ = maxStamina_;
			}
			else if ( statType == "maxmn" || statType == "maxmana" )
			{
				maxMana_ = Value.toLong();
				mana_ = maxMana_;
			}
		}
	}

	// Aliasses <str <dex <int <maxhp <maxstm <maxmn <maxhitpoints <maxstamina <maxmana
	else if ( TagName == "str" )
	{
		strength_ = Value.toLong();
		maxHitpoints_ = strength_;
		hitpoints_ = maxHitpoints_;
	}
	else if ( TagName == "dex" )
	{
		dexterity_ = Value.toLong();
		maxStamina_ = dexterity_;
		stamina_ = maxStamina_;
	}
	else if ( TagName == "int" )
	{
		intelligence_ = Value.toLong();
		maxMana_ = intelligence_;
		mana_ = maxMana_;
	}
	else if ( TagName == "maxhp" || TagName == "maxhitpoints" )
	{
		maxHitpoints_ = Value.toLong();
		hitpoints_ = maxHitpoints_;
	}
	else if ( TagName == "maxstm" || TagName == "maxstamina" )
	{
		maxStamina_ = Value.toLong();
		stamina_ = maxStamina_;
	}
	else if ( TagName == "maxmn" || TagName == "maxmana" )
	{
		maxMana_ = Value.toLong();
		mana_ = maxMana_;
	}

	//<skill type="alchemy">100</skill>
	//<skill type="1">100</skill>
	else if ( TagName == "skill" && Tag->hasAttribute( "type" ) )
	{
		if ( Tag->getAttribute( "type" ).toInt() > 0 && Tag->getAttribute( "type" ).toInt() <= ALLSKILLS )
			setSkillValue( ( Tag->getAttribute( "type" ).toInt() - 1 ), Value.toInt() );
		else
		{
			Q_INT16 skillId = Skills::instance()->findSkillByDef( Tag->getAttribute( "type", "" ) );
			setSkillValue( skillId, Value.toInt() );
		}
	}

	//<equipped>
	//	<item id="a" />
	//	<item id="b" />
	//	...
	//</epuipped>
	else if ( TagName == "equipped" )
	{
		for ( unsigned int i = 0; i < Tag->childCount(); ++i )
		{
			const cElement* element = Tag->getChild( i );

			if ( element->name() == "item" )
			{
				P_ITEM pItem = 0;

				const QString& id = element->getAttribute( "id" );

				if ( id != QString::null )
				{
					pItem = cItem::createFromScript( id );
				}
				else
				{
					const QString& list = element->getAttribute( "list" );

					if ( list != QString::null )
					{
						pItem = cItem::createFromList( list );
					}
				}

				if ( pItem )
				{
					pItem->applyDefinition( element );

					unsigned char mLayer = pItem->layer();
					pItem->setLayer( 0 );

					// Instead of deleting try to get a valid layer instead
					if ( !mLayer )
					{
						tile_st tInfo = TileCache::instance()->getTile( pItem->id() );
						if ( tInfo.layer > 0 )
							mLayer = tInfo.layer;
					}

					if ( !mLayer )
						pItem->remove();
					else
					{
						// Check if there is sth there already.
						// Could happen due to inherit.
						P_ITEM existing = atLayer( static_cast<cBaseChar::enLayer>( mLayer ) );
						if ( existing )
							existing->remove();
						addItem( static_cast<cBaseChar::enLayer>( mLayer ), pItem );
					}
				}
				else
				{
					Console::instance()->log( LOG_ERROR, tr( "Invalid equipped element missing id and list attribute in npc definition '%1'." ).arg( element->getTopmostParent()->getAttribute( "id", "unknown" ) ) );
				}
			}
			else
			{
				Console::instance()->log( LOG_ERROR, tr( "Invalid equipped element '%1' in npc definition '%2'." ).arg( element->name() ).arg( element->getTopmostParent()->getAttribute( "id", "unknown" ) ) );
			}
		}
	}
	// <saycolor>color</saycolor>
	else if ( TagName == "saycolor" )
	{
		bool ok;
		ushort color = Value.toUShort( &ok );
		if ( ok )
			this->setSaycolor( color );
	}
	// <haircolor>color</haircolor> dyes currently equipped hair/beard
	else if ( TagName == "haircolor" )
	{
		bool ok;
		ushort color = Value.toUShort( &ok );
		if( ok )
		{
			P_ITEM hair = getItemOnLayer( Hair );
			if( hair )
				hair->setColor( color );

			P_ITEM facialHair = getItemOnLayer( FacialHair );
			if( facialHair )
				facialHair->setColor( color );
		}
	}
	else
	{
		Q_INT16 skillId = Skills::instance()->findSkillByDef( TagName );

		if ( skillId == -1 )
			cUObject::processNode( Tag );
		else
			setSkillValue( skillId, Value.toInt() );
	}
}

void cBaseChar::addItem( cBaseChar::enLayer layer, cItem* pi, bool handleWeight, bool noRemove )
{
	if ( pi->multi() )
	{
		// Ignore the pseudo-pointer if uninitialized
		if ( !pi->unprocessed() )
		{
			pi->multi()->removeObject( pi );
		}
		pi->setMulti( 0 );
	}

	if ( atLayer( layer ) != 0 )
	{
		log( LOG_ERROR, tr( "Trying to put item 0x%1 on layer %2 which is already occupied.\n" ).arg( pi->serial(), 0, 16 ).arg( layer ) );
		pi->container_ = 0;
		pi->moveTo( pos_, true );
		return;
	}

	if ( !noRemove )
	{
		pi->removeFromCont();
	}

	content_.insert( ( ushort ) ( layer ), pi );

	if ( pi->layer() != layer )
	{
		pi->setLayer( layer );
	}

	if ( Server::instance()->getState() != STARTUP )
	{
		pi->setContainer( this );
	}
	else
	{
		pi->container_ = this; // Avoid a flagChanged()
	}

	if ( handleWeight && ( pi->layer() < 0x1A || pi->layer() == 0x1E ) )
	{
		weight_ += pi->totalweight();
	}

	if ( !noRemove )
	{
		// Dragging doesnt count as Equipping
		if ( layer != Dragging )
			pi->onEquip( this, layer );
	}
}

void cBaseChar::removeItem( cBaseChar::enLayer layer, bool handleWeight )
{
	P_ITEM pi = atLayer( layer );
	if ( pi )
	{
		pi->setContainer( 0 );
		pi->setLayer( 0 );
		content_.remove( ( ushort ) ( layer ) );

		if ( handleWeight )
			weight_ -= pi->totalweight();

		// Dragging doesnt count as Equipping
		if ( layer != Dragging )
			pi->onUnequip( this, layer );
	}
}

// Simple setting and getting of properties for scripts and the set command.
stError* cBaseChar::setProperty( const QString& name, const cVariant& value )
{
	changed( TOOLTIP );
	changed_ = true;
	/*
		\property char.orgname This string property indicates the original name of the character.
	*/
	SET_STR_PROPERTY( "orgname", orgName_ )
	/*
	\property char.title This string property contains the title of the character.
	*/
	else
		SET_STR_PROPERTY( "title", title_ )
		/*
		\property char.incognito This boolean property indicates whether the character is incognito.
		*/
	else if ( name == "incognito" )
	{
		setIncognito( value.toInt() );
		return 0;
	}
	/*
		\property char.polymorph This boolean property indicates whether the character is polymorphed.
	*/
	else if ( name == "polymorph" )
	{
		setPolymorphed( value.toInt() );
		return 0;
	}
	/*
		\property char.dead This boolean property indicates whether the character is dead.
	*/
	else if ( name == "dead" )
	{
		setDead( value.toInt() );
		return 0;
	}
	/*
		\property char.haircolor This integer property sets the haircolor of the character.
		This property is write only.
	*/
	else if ( name == "haircolor" )
	{
		bool ok;
		int data = value.toInt( &ok );
		if ( !ok )
			PROPERTY_ERROR( -2, "Integer expected" )
			setHairColor( data );
		return 0;
	}
	/*
		\property char.hairstyle This integer property sets the hairstyle of the character.
		This property is write only.
	*/
	else if ( name == "hairstyle" )
	{
		bool ok;
		int data = value.toInt( &ok );
		if ( !ok )
			PROPERTY_ERROR( -2, "Integer expected" )
			setHairStyle( data );
		return 0;
	}
	/*
		\property char.beardcolor This integer property sets the beardcolor of the character.
		This property is write only.
	*/
	else if ( name == "beardcolor" )
	{
		bool ok;
		int data = value.toInt( &ok );
		if ( !ok )
			PROPERTY_ERROR( -2, "Integer expected" )
			setBeardColor( data );
		return 0;
	}
	/*
		\property char.beardstyle This integer property sets the beardstyle of the character.
		This property is write only.
	*/
	else if ( name == "beardstyle" )
	{
		bool ok;
		int data = value.toInt( &ok );
		if ( !ok )
			PROPERTY_ERROR( -2, "Integer expected" )
			setBeardStyle( data );
		return 0;
	}
	/*
		\property char.skin This integer property contains the skin color of the character.
	*/
	else if (name == "skin") {
		skin_ = value.toInt();		
		removeFromView(false);
		return 0;
	}		

		// \property char.direction This is the direction this character is facing.
	else
		SET_INT_PROPERTY( "direction", direction_ )

		/*
		\property char.maxhitpoints This integer property contains the maximum hitpoints for the character.
		Please note that the maximum hitpoints are constantly recalculated. Please see hitpointsbonus for
		a better way to increase the maximum hitponts of a character.
		*/
	else
		SET_INT_PROPERTY( "maxhitpoints", maxHitpoints_ )
		/*
		\property char.maxstamina This integer property contains the maximum stamina for the character.
		Please note that the maximum stamina are constantly recalculated. Please see staminabonus for
		a better way to increase the maximum stamina of a character.
		*/
	else
		SET_INT_PROPERTY( "maxstamina", maxStamina_ )
		/*
		\property char.maxmana This integer property contains the maximum mana for the character.
		Please note that the maximum mana are constantly recalculated. Please see manabonus for
		a better way to increase the maximum mana of a character.
		*/
	else
		SET_INT_PROPERTY( "maxmana", maxMana_ )
		/*
		\property char.lastmovement This integer property indicates the servertime of the last movement
		of this character.
		*/
	else
		SET_INT_PROPERTY( "lastmovement", lastMovement_ )
		/*
		\property char.orgskin This integer property indicates the original skin of the character that is restored when he dies or
		a spell that changed the skin wears off.
		*/
	else
		SET_INT_PROPERTY( "orgskin", orgSkin_ )
		/*
		\property char.creationdate This string property indicates the date and time the character was created.
		*/
	else if ( name == "creationdate" )
	{
		creationDate_ = QDateTime::fromString( value.toString() );
		return 0;
	}
	/*
		\property char.stealthedsteps This integer property indicates how many steps the character walked when stealthed already.
	*/
	else
		SET_INT_PROPERTY( "stealthedsteps", stealthedSteps_ )
		/*
		\property char.running This boolean property indicates whether this character was running when he made his last step.
		*/
	else if (name == "running") {
		setRunning( value.toInt() != 0 );
		return 0;
	}
		/*
		\property char.tamed This boolean property indicates whether the character is tamed or not.
		*/
	else if ( name == "tamed" )
	{
		setTamed( value.toInt() );
		return 0;
	}
	/*
		\property char.guarding This character property is the character that is currently guarded by this character.
	*/
	else
		SET_CHAR_PROPERTY( "guarding", guarding_ )
		/*
		\property char.murderer This integer property indicates the serial of the character who killed this character last.
		*/
	else
		SET_INT_PROPERTY( "murderer", murdererSerial_ )
		/*
		\property char.casting This boolean property indicates whether the character is currently casting a spell.
		*/
	else if ( name == "casting" )
	{
		setCasting( value.toInt() );
		return 0;
	}
	/*
		\property char.hidden This boolean property indicates whether the character is currently hidden.
	*/
	else if ( name == "hidden" )
	{
		setHidden( value.toInt() );
		return 0;
	}

	/*
		\property char.hunger This integer property indicates the food level of the character. 0 is the lowest food level, 6 the highest.
	*/
	else
		SET_INT_PROPERTY( "hunger", hunger_ )

		/*
		\property char.hungertime This integer property is the next servertime the foodlevel of this character will be reduced.
		*/
	else
		SET_INT_PROPERTY( "hungertime", hungerTime_ )

		/*
		\property char.poison The strength of the poison applied to this character.
		A value of -1 means that no poision is applied to this character.
		*/
	else
		SET_INT_PROPERTY( "poison", poison_ )

		/*
		\property char.flag The ingame notoriety for this character.
		*/
	else
		SET_INT_PROPERTY( "flag", flag_ )

		/*
		\property propertyflags The bitfield (32 bit) with basechar properties. You can use the
		upper 8 bits for custom properties.
		*/
	else
		SET_INT_PROPERTY( "propertyflags", propertyFlags_ )

		/*
		\property char.murderertime This integer property indicates when the next kill of the murder count will be removed.
		*/
	else
		SET_INT_PROPERTY( "murderertime", murdererTime_ )
		/*
		\property char.criminaltime This integer property indicates the servertime when the criminal flag of this character will wear off.
		*/
	else
		SET_INT_PROPERTY( "criminaltime", criminalTime_ )
		/*
		\property char.meditating This boolean property indicates whether this character is currently meditating.
		*/
	else if ( name == "meditating" )
	{
		setMeditating( value.toInt() );
		return 0;
	}
	/*
		\property char.weight This float property indicates the total weight of this character.
	*/
	else
		SET_FLOAT_PROPERTY( "weight", weight_ )
		/*
		\property char.saycolor This integer property is the saycolor of this character.
		*/
	else
		SET_INT_PROPERTY( "saycolor", saycolor_ )
		/*
		\property char.emotecolor This integer property is the emote color of this character.
		*/
	else
		SET_INT_PROPERTY( "emotecolor", emoteColor_ )
		/*
		\property char.strength This integer property is the strength of this character.
		*/
	else if ( name == "strength" )
	{
		setStrength( value.toInt() );
		return 0;
		/*
				\property char.dexterity This integer property is the dexterity of this character.
			*/
	}
	else if ( name == "dexterity" )
	{
		setDexterity( value.toInt() );
		return 0;
		/*
				\property char.intelligence This integer property is the intelligence of this character.
			*/
	}
	else if ( name == "intelligence" )
	{
		setIntelligence( value.toInt() );
		return 0;
		/*
				\property char.strength2 This integer property contains a modification value applied to strength. This is used to
				determine the real strength of the character if needed.
			*/
	}
	else if ( name == "strength2" )
	{
		setStrengthMod( value.toInt() );
		return 0;
		/*
				\property char.dexterity2 This integer property contains a modification value applied to dexterity. This is used to
				determine the real dexterity of the character if needed.
			*/
	}
	else if ( name == "dexterity2" )
	{
		setDexterityMod( value.toInt() );
		return 0;
		/*
				\property char.intelligence2 This integer property contains a modification value applied to intelligence. This is used to
				determine the real intelligence of the character if needed.
			*/
	}
	else if ( name == "intelligence2" )
	{
		setIntelligenceMod( value.toInt() );
		return 0;
	}
	/*
		\property char.orgid This is the original body id of the character that is restored when he dies or any spell affecting it
		expires.
	*/
	else
		SET_INT_PROPERTY( "orgid", orgBody_ )
		/*
		\property char.hitpoints The current hitpoints of this character.
		*/
	else
		SET_INT_PROPERTY( "hitpoints", hitpoints_ )
		/*
		\property char.health The current hitpoints of this character.
		*/
	else
		SET_INT_PROPERTY( "health", hitpoints_ )
		/*
		\property char.stamina The current stamina of this character.
		*/
	else
		SET_INT_PROPERTY( "stamina", stamina_ )
		/*
		\property char.mana The current mana of this character.
		*/
	else
		SET_INT_PROPERTY( "mana", mana_ )
		/*
		\property char.karma The current karma of this character.
		*/
	else
		SET_INT_PROPERTY( "karma", karma_ )
		/*
		\property char.fame The current fame of this character.
		*/
	else
		SET_INT_PROPERTY( "fame", fame_ )
		/*
		\property char.kills The current kills of this character.
		This is used to determine the murderer status.
		*/
	else
		SET_INT_PROPERTY( "kills", kills_ )
		/*
		\property char.deaths The current deaths of this character.
		*/
	else
		SET_INT_PROPERTY( "deaths", deaths_ )
		/*
		\property char.war This boolean property indicates whether the character is in warmode or not.
		*/
	else if ( name == "war" )
	{
		setAtWar( value.toInt() );
		return 0;
	}
	/*
		\property char.attacktarget The attack target of this character.
	*/
	else
		SET_CHAR_PROPERTY( "attacktarget", attackTarget_ )
		/*
		\property char.nextswing The servertime the character will be able to swing (attack) next.
		*/
	else
		SET_INT_PROPERTY( "nextswing", nextSwing_ )
		/*
		\property char.regenhealth The next servertime the character will try to regenerate hitpoints.
		*/
	else
		SET_INT_PROPERTY( "regenhealth", regenHitpointsTime_ )
		/*
		\property char.regenstamina The next servertime the character will try to regenerate stamina.
		*/
	else
		SET_INT_PROPERTY( "regenstamina", regenStaminaTime_ )
		/*
		\property char.regenmana The next servertime the character will try to regenerate mana.
		*/
	else
		SET_INT_PROPERTY( "regenmana", regenManaTime_ )
		/*
		\property char.skilldelay The servertime the character will be able to use another active skill again.
		*/
	else
		SET_INT_PROPERTY( "skilldelay", skillDelay_ )
		/*
		\property char.gender The gender of this character. False means male, true female.
		*/
	else
		SET_INT_PROPERTY( "gender", gender_ )
		/*
		\property char.id The body id of this character.
		*/
	else
		SET_INT_PROPERTY( "id", body_ )
		/*
		\property char.hitpointsbonus The integer bonus awarded to the maximum hitpoints of this character.
		*/
	else if ( name == "hitpointsbonus" )
	{
		setHitpointsBonus( value.toInt() );
		return 0;
		/*
				\property char.staminabonus The integer bonus awarded to the maximum stamina of this character.
			*/
	}
	else if ( name == "staminabonus" )
	{
		setStaminaBonus( value.toInt() );
		return 0;
		/*
				\property char.manabonus The integer bonus awarded to the maximum mana of this character.
			*/
	}
	else if ( name == "manabonus" )
	{
		setManaBonus( value.toInt() );
		return 0;
		/*
				\property char.invulnerable Indicates whether the character is invulnerable or not.
			*/
	}
	else if ( name == "invulnerable" )
	{
		setInvulnerable( value.toInt() );
		return 0;
	}
	/*
		\property char.invisible Indicates whether the character is invisible or not.
	*/
	else if ( name == "invisible" )
	{
		setInvisible( value.toInt() );
		return 0;
	}
	/*
		\property char.frozen Indicates whether the character is frozen or not.
	*/
	else if ( name == "frozen" )
	{
		setFrozen( value.toInt() );
		return 0;
	}
	/*
		\property char.strengthcap The individual strength cap for this character.
	*/
	else
		SET_INT_PROPERTY( "strengthcap", strengthCap_ )
		/*
		\property char.dexteritycap The individual dexterity cap for this character.
		*/
	else
		SET_INT_PROPERTY( "dexteritycap", dexterityCap_ )
		/*
		\property char.intelligencecap The individual intelligence cap for this character.
		*/
	else
		SET_INT_PROPERTY( "intelligencecap", intelligenceCap_ )
		/*
		\property char.statcap The individual total stat cap for this character.
		*/
	else
		SET_INT_PROPERTY( "statcap", statCap_ )
		/*
		\property char.baseid The name of the definition this character was created from.
		This property is used to link the character to the definitions even after he
		was created.
		*/
	else if ( name == "baseid" )
	{
		setBaseid( value.toString().latin1() );
		return 0;
	}

	return cUObject::setProperty( name, value );
}

PyObject* cBaseChar::getProperty( const QString& name )
{
	// \rproperty char.overloaded This boolean property indicates whether the character is overloaded or not.
	PY_PROPERTY( "overloaded", isOverloaded() )
	// \rproperty char.maxweight The maximum weight this character can carry with his current strength.
	PY_PROPERTY( "maxweight", maxWeight() )
	/* \rproperty char.stepstaken The number of steps this character walked since the server started.
	This value is not saved between server downs.
	*/
	PY_PROPERTY( "stepstaken", stepsTaken() )
	/*
		\rproperty char.bodytype The type of this characters bodies.
		<code>0 - Unknown
		1 Monster
		2 Sea
		3 Animal
		4 Human
		5 Equipment</code>
	*/
	PY_PROPERTY( "bodytype", bodytype() )
	PY_PROPERTY( "orgname", orgName_ )
	PY_PROPERTY( "direction", direction_ )
	PY_PROPERTY( "baseid", baseid() )
	PY_PROPERTY( "lastmovement", lastMovement_ )
	PY_PROPERTY( "title", title_ )
	PY_PROPERTY( "incognito", isIncognito() )
	PY_PROPERTY( "polymorph", isPolymorphed() )
	PY_PROPERTY( "skin", skin_ )
	PY_PROPERTY( "orgskin", orgSkin_ )
	PY_PROPERTY( "creationdate", creationDate_.toString() )
	PY_PROPERTY( "stealthedsteps", stealthedSteps_ )
	PY_PROPERTY( "running", running_ )
	PY_PROPERTY( "tamed", isTamed() )
	PY_PROPERTY( "guarding", guarding_ )
	PY_PROPERTY( "murderer", FindCharBySerial( murdererSerial_ ) )
	PY_PROPERTY( "casting", isCasting() )
	PY_PROPERTY( "hidden", isHidden() )
	PY_PROPERTY( "hunger", hunger_ )
	PY_PROPERTY( "hungertime", hungerTime_ )
	PY_PROPERTY( "poison", poison_ )
	PY_PROPERTY( "flag", flag_ )
	PY_PROPERTY( "propertyflags", propertyFlags_ )
	PY_PROPERTY( "murderertime", murdererTime_ )
	PY_PROPERTY( "criminaltime", criminalTime_ )
	PY_PROPERTY( "meditating", isMeditating() )
	PY_PROPERTY( "weight", weight_ )
	PY_PROPERTY( "saycolor", saycolor_ )
	PY_PROPERTY( "emotecolor", emoteColor_ )
	PY_PROPERTY( "strength", strength_ )
	PY_PROPERTY( "dexterity", dexterity_ )
	PY_PROPERTY( "intelligence", intelligence_ )
	PY_PROPERTY( "strength2", strengthMod_ )
	PY_PROPERTY( "dexterity2", dexterityMod_ )
	PY_PROPERTY( "intelligence2", intelligenceMod_ )
	PY_PROPERTY( "orgid", orgBody_ )
	PY_PROPERTY( "maxhitpoints", maxHitpoints_ )
	PY_PROPERTY( "hitpoints", hitpoints_ )
	PY_PROPERTY( "strengthcap", strengthCap_ )
	PY_PROPERTY( "dexteritycap", dexterityCap_ )
	PY_PROPERTY( "intelligencecap", intelligenceCap_ )
	PY_PROPERTY( "statcap", statCap_ )
	PY_PROPERTY( "health", hitpoints_ )
	PY_PROPERTY( "maxstamina", maxStamina_ )
	PY_PROPERTY( "stamina", stamina_ )
	PY_PROPERTY( "maxmana", maxMana_ )
	PY_PROPERTY( "mana", mana_ )
	PY_PROPERTY( "karma", karma_ )
	PY_PROPERTY( "fame", fame_ )
	PY_PROPERTY( "kills", kills_ )
	PY_PROPERTY( "deaths", deaths_ )
	PY_PROPERTY( "dead", isDead() )
	PY_PROPERTY( "war", isAtWar() )
	PY_PROPERTY( "attacktarget", attackTarget_ )
	PY_PROPERTY( "nextswing", nextSwing_ )
	PY_PROPERTY( "regenhealth", regenHitpointsTime_ )
	PY_PROPERTY( "regenstamina", regenStaminaTime_ )
	PY_PROPERTY( "regenmana", regenManaTime_ )
	PY_PROPERTY( "region", region_ )
	PY_PROPERTY( "skilldelay", skillDelay_ )
	PY_PROPERTY( "gender", gender_ )
	PY_PROPERTY( "id", body_ )
	PY_PROPERTY( "invulnerable", isInvulnerable() )
	PY_PROPERTY( "invisible", isInvisible() )
	PY_PROPERTY( "frozen", isFrozen() )
	PY_PROPERTY( "hitpointsbonus", hitpointsBonus_ )
	PY_PROPERTY( "staminabonus", staminaBonus_ )
	PY_PROPERTY( "manabonus", manaBonus_ )

	/*
	\rproperty char.basesound The base sound id for this creature. Not used for humans.

	This property is inherited from the definition referenced by the baseid property.
	*/
	PY_PROPERTY( "basesound", basesound() )

	/*
	\rproperty char.canfly Indicates whether the creature can fly.

	This property is inherited from the definition referenced by the baseid property.
	*/
	PY_PROPERTY( "canfly", isCanFly() )

	/*
	\rproperty char.antiblink Indicates whether the creature has the anti blink bit set for animations.

	This property is inherited from the definition referenced by the baseid property.
	*/
	PY_PROPERTY( "antiblink", isAntiBlink() )

	/*
	\rproperty char.nocorpse Indicates whether the creature leaves a corpse or not.

	This property is inherited from the definition referenced by the baseid property.
	*/
	PY_PROPERTY( "nocorpse", isNoCorpse() )

	/*
	\rproperty figurine The itemid of the figurine thats created when the creature is shrunk.

	This property is inherited from the definition referenced by the baseid property.
	*/
	PY_PROPERTY( "figurine", figurine() )

	/*
	\rproperty char.mindamage The minimum damage this character type

	This property is inherited from the definition referenced by the baseid property.
	*/
	PY_PROPERTY( "antiblink", isAntiBlink() )

	/*
	\rproperty char.mindamage This is the minimum damage dealt by the creature when unarmed.

	This property is inherited from the definition referenced by the baseid property.
	*/
	PY_PROPERTY( "mindamage", minDamage() )

	/*
	\rproperty char.maxdamage This is the maximum damage dealt by the creature when unarmed.

	This property is inherited from the definition referenced by the baseid property.
	*/
	PY_PROPERTY( "maxdamage", maxDamage() )

	/*
	\rproperty char.mintaming This is the minimum taming skill required to tame this creature.
	This has no meaning for player characters.

	This property is inherited from the definition referenced by the baseid property.
	*/
	PY_PROPERTY( "mintaming", minTaming() )

	/*
	\rproperty char.carve This is the name of the list of items created when a dagger is
	used on the corpse of this creature.

	This property is inherited from the definition referenced by the baseid property.
	*/
	PY_PROPERTY( "carve", carve() )

	/*
	\rproperty char.lootpacks This is a semicolon separated list of lootpacks that are created
	in the corpse of this creature.
	This has no meaning for player characters.

	This property is inherited from the definition referenced by the baseid property.
	*/
	PY_PROPERTY( "lootpacks", lootPacks() )

	/*
	\rproperty char.controlslots The amount of follower slots this npc will consume when owned
	by a player.

	This property is exclusive to npcs and is inherited from the definition referenced by the baseid property.
	*/
	PY_PROPERTY( "controlslots", controlSlots() )

	/*
	\rproperty char.basescripts This is a comma separated list of scripts assigned to this item
	via the baseid. They are called after the scripts assigned dynamically to the item.
	*/
	PY_PROPERTY( "basescripts", basedef_ ? basedef_->baseScriptList() : "" );

	if ( name.left( 6 ) == "skill." )
	{
		QString skill = name.right( name.length() - 6 );
		Q_INT16 skillId = Skills::instance()->findSkillByDef( skill );

		if ( skillId != -1 )
		{
			return createPyObject( skillValue( skillId ) );
		}
		// skillcap.
	}
	else if ( name.left( 9 ) == "skillcap." )
	{
		QString skill = name.right( name.length() - 9 );
		Q_INT16 skillId = Skills::instance()->findSkillByDef( skill );

		if ( skillId != -1 )
		{
			return createPyObject( skillCap( skillId ) );
		}
	}
	else
	{
		// See if there's a skill by that name
		Q_INT16 skillId = Skills::instance()->findSkillByDef( name );

		if ( skillId != -1 )
		{
			return createPyObject( skillValue( skillId ) );
		}
	}
	return cUObject::getProperty( name );
}

void cBaseChar::setSkillValue( Q_UINT16 skill, Q_UINT16 value )
{
	skills_[skill].value = value;
	skills_[skill].changed = true;
}

void cBaseChar::setSkillCap( Q_UINT16 skill, Q_UINT16 cap )
{
	skills_[skill].cap = cap;
	skills_[skill].changed = true;
}

void cBaseChar::setSkillLock( Q_UINT16 skill, Q_UINT8 lock )
{
	if ( lock > 2 )
		lock = 0;

	skills_[skill].lock = lock;
	skills_[skill].changed = true;
}

Q_UINT16 cBaseChar::skillValue( Q_UINT16 skill ) const
{
	return skills_[skill].value;
}

Q_UINT16 cBaseChar::skillCap( Q_UINT16 skill ) const
{
	return skills_[skill].cap;
}

Q_UINT8 cBaseChar::skillLock( Q_UINT16 skill ) const
{
	return skills_[skill].lock;
}

void cBaseChar::setStamina( Q_INT16 data, bool notify )
{
	stamina_ = data;
	changed_ = notify;
}

void cBaseChar::callGuards()
{
	if ( !inGuardedArea() || !Config::instance()->guardsActive() )
		return;

	// Is there a criminal around?
	MapCharsIterator cIter = MapObjects::instance()->listCharsInCircle( pos(), 18 );
	for( P_CHAR pChar = cIter.first(); pChar; pChar = cIter.next() )
	{
		if ( !pChar->isDead() && !pChar->isInnocent() )
		{
			Combat::instance()->spawnGuard( pChar, pChar, pChar->pos() );
		}
	}
}

unsigned int cBaseChar::damage( eDamageType type, unsigned int amount, cUObject* source )
{
	if (isInvulnerable()) {
		return 0;
	}

	if ( isFrozen() )
	{
		setFrozen( false );
		resendTooltip();
	}

	P_CHAR sourceChar = dynamic_cast<P_CHAR>(source);

	if (sourceChar && sourceChar->canHandleEvent(EVENT_DODAMAGE)) {
		PyObject* args = Py_BuildValue( "NiiN", sourceChar->getPyObject(), type, amount, getPyObject() );
		PyObject* result = sourceChar->callEvent(EVENT_DODAMAGE, args);

		if (result) {
			if (PyInt_Check(result))
				amount = PyInt_AsLong(result);
			Py_DECREF(result);
		}

		Py_DECREF(args);
	}

	//
	// First of all, call onDamage with the damage-type, amount and source
	// to modify the damage if needed
	//
	if ( canHandleEvent(EVENT_DAMAGE) )
	{
		PyObject* args = 0;
		if ( sourceChar )
			args = Py_BuildValue( "O&iiO&", PyGetCharObject, this, ( unsigned int ) type, amount, PyGetCharObject, sourceChar );
		else if ( dynamic_cast<P_ITEM>( source ) )
			args = Py_BuildValue( "O&iiO&", PyGetCharObject, this, ( unsigned int ) type, amount, PyGetItemObject, source );
		else
			args = Py_BuildValue( "O&iiO", PyGetCharObject, this, ( unsigned int ) type, amount, Py_None );

		PyObject* result = callEvent(EVENT_DAMAGE, args);

		if ( result )
		{
			if ( PyInt_Check( result ) )
				amount = PyInt_AsLong( result );

			Py_DECREF( result );
		}

		Py_DECREF( args );
	}

	// The damage has been resisted or scripts have taken care of the damage otherwise
	// Invulnerable Targets don't take any damage at all
	if (amount == 0) {
		return 0;
	}

	P_PLAYER player = dynamic_cast<P_PLAYER>( source );

	if ( !player )
	{
		P_ITEM tool = dynamic_cast<P_ITEM>( source );
		if ( tool && tool->owner() )
		{
			player = dynamic_cast<P_PLAYER>( tool->owner() );
		}
	}

	// Show the amount of damage dealt over the head of the victim
	if ( player && player->socket() )
	{
		cUOTxDamage damage;
		damage.setUnknown1( 1 );
		damage.setDamage( amount );
		damage.setSerial( serial_ );
		player->socket()->send( &damage );
	}

    // There is a 25% chance that blood is created on hit by phsical means
	if ( type == DAMAGE_PHYSICAL && !RandomNum( 0, 4 ) )
	{
		int bloodColor = 0;

		if (basedef_) {
			bloodColor = (int)basedef_->getIntProperty("bloodcolor", 0);

			// If we have a strproperty with a custom list of colors,
			// that is used instead	
			if (bloodColor != -1) {
				// this property means: from,to
				if (basedef_->hasStrProperty("bloodcolor")) {
					QStringList bloodColors = QStringList::split(",", basedef_->getStrProperty("bloodcolor"));
					if (bloodColors.count() == 2) {
						bool ok1, ok2;
						int from = bloodColors[0].toInt(&ok1);
						int to = bloodColors[1].toInt(&ok2);
						if (ok1 && ok2) {
							bloodColor = RandomNum(from, to);
						}
					}
				}
			}
		}

		if (bloodColor != -1) {
			P_ITEM blood = 0;

			// If more than 50% of the maximum healthpoints has been dealt as damage
			// we display a big puddle of blood
			if ( amount >= maxHitpoints_ * 0.50 )
			{
				blood = cItem::createFromList( "BIG_BLOOD_PUDDLES" );

				// Otherwise we display a medium puddle of blood if the damage is greater
				// than 25% of the maximum healthpoints
			}
			else if ( amount >= maxHitpoints_ * 0.35 )
			{
				blood = cItem::createFromList( "MEDIUM_BLOOD_PUDDLES" );

				// at last we only display a small stain of blood if the damage has been
				// greater than 10% of the maximum hitpoints
			}
			else if ( amount >= maxHitpoints_ * 0.20 )
			{
				blood = cItem::createFromList( "SMALL_BLOOD_PUDDLES" );
			}

			if ( blood )		
			{
				blood->setColor(bloodColor);
				blood->setNoDecay( false ); // Override the nodecay tag in the definitions
				blood->moveTo( pos_ ); // Move it to the feet of the victim
				blood->update(); // Send it to all sockets in range
			}
		}
	}

	// Would we die?
	if ( amount >= hitpoints_ )
	{
		kill( source );
	}
	else
	{
		hitpoints_ -= amount;
		updateHealth();
		Combat::instance()->playGetHitSoundEffect( this );
		Combat::instance()->playGetHitAnimation( this );
	}

	return amount;
}

void cBaseChar::bark( enBark type )
{
	switch (type) {
		case Bark_Attacking:
			playAttackSound();
			break;
		case Bark_Idle:
			playIdleSound();
			break;
		case Bark_Hit:
			playHitSound();
			break;
		case Bark_GetHit:
			playGetHitSound();
			break;
		case Bark_Death:
			playDeathSound();
			break;
		default:
			break;
	};
}

void cBaseChar::goldSound( unsigned short amount, bool hearall )
{
	unsigned short sound;

	/*if ( amount == 1 )
		sound = 0x35;
	else if ( amount > 1 && amount < 6 )
		sound = 0x36;
	else
		sound = 0x37;*/
	if (amount <= 1)
		sound = 0x2e4;
	else if (amount <= 5)
		sound = 0x2e5;
	else 
		sound = 0x2e6;

	soundEffect( sound, hearall );
}

void cBaseChar::showPaperdoll( cUOSocket* source, bool hotkey )
{
	if ( !source )
		return;

	P_PLAYER pChar = source->player();

	if ( !pChar || onShowPaperdoll( pChar ) )
		return;

	// For players we'll always show the Paperdoll
	if ( isHuman() || objectType() != enNPC )
	{
		// If we're mounted (item on layer 25) and *not* using a hotkey
		// We're trying to unmount
		if ( !hotkey && ( this == pChar ) && pChar->unmount() )
			return; // We have been unmounted

		source->sendPaperdoll( this );
	}

	P_NPC npc = dynamic_cast<P_NPC>(this);

	// Mounting and pack animals
	if (npc) {
		if (body_ == 0x123 || body_  == 0x124) {
			if ( npc->owner() == pChar || pChar->isGM() ) {
				source->sendContainer( getBackpack() );
			}
		} else {
			pChar->mount(npc); // Try mounting this
		}
	}
}

/*
	Event Wrappers
 */
bool cBaseChar::onWalk( unsigned char direction, unsigned char sequence )
{
	bool result = false;
	if (canHandleEvent(EVENT_WALK))
	{
		PyObject* args = Py_BuildValue( "O&bb", PyGetCharObject, this, direction, sequence );
		result = callEventHandler(EVENT_WALK, args);
		Py_DECREF( args );
	}
	return result;
}

bool cBaseChar::onTalk( unsigned char type, unsigned short color, unsigned short font, const QString& text, const QString& lang )
{
	bool result = false;
	if (canHandleEvent(EVENT_TALK))
	{
		PyObject* args = Py_BuildValue( "O&bhhNN", PyGetCharObject, this, type, color, font, QString2Python(text), QString2Python(lang) );
		result = callEventHandler(EVENT_TALK, args);
		Py_DECREF( args );
	}
	return result;
}

bool cBaseChar::onWarModeToggle( bool war )
{
	bool result = false;
	if (canHandleEvent(EVENT_WARMODETOGGLE))
	{
		PyObject* args = Py_BuildValue( "O&i", PyGetCharObject, this, war ? 1 : 0 );
		result = callEventHandler(EVENT_WARMODETOGGLE, args);
		Py_DECREF( args );
	}
	return result;
}

bool cBaseChar::onShowPaperdoll( P_CHAR pOrigin )
{
	bool result = false;
	if (canHandleEvent(EVENT_SHOWPAPERDOLL))
	{
		PyObject* args = Py_BuildValue( "O&O&", PyGetCharObject, this, PyGetCharObject, pOrigin );
		result = callEventHandler(EVENT_SHOWPAPERDOLL, args);
		Py_DECREF( args );
	}
	return result;
}

bool cBaseChar::onShowSkillGump()
{
	bool result = false;
	if (canHandleEvent(EVENT_SHOWSKILLGUMP))
	{
		PyObject* args = Py_BuildValue( "(N)", getPyObject() );
		result = callEventHandler(EVENT_SHOWSKILLGUMP, args);
		Py_DECREF( args );
	}
	return result;
}

bool cBaseChar::onSkillUse( unsigned char skill )
{
	bool result = false;
	if (canHandleEvent(EVENT_SKILLUSE))
	{
		PyObject* args = Py_BuildValue( "O&b", PyGetCharObject, this, skill );
		result = callEventHandler(EVENT_SKILLUSE, args);
		Py_DECREF( args );
	}
	return result;
}

bool cBaseChar::onDropOnChar( P_ITEM pItem )
{
	bool result = false;
	if (canHandleEvent(EVENT_DROPONCHAR))
	{
		PyObject* args = Py_BuildValue( "O&O&", PyGetCharObject, this, PyGetItemObject, pItem );
		result = callEventHandler(EVENT_DROPONCHAR, args);
		Py_DECREF( args );
	}
	return result;
}

QString cBaseChar::onShowPaperdollName( P_CHAR pOrigin )
{
	// I hate this event by the way (DarkStorm)
	QString name = QString::null;

	if ( canHandleEvent(EVENT_SHOWPAPERDOLLNAME) )
	{
		PyObject* args = Py_BuildValue( "O&O&", PyGetCharObject, this, PyGetCharObject, pOrigin );

		PyObject* result = callEvent(EVENT_SHOWPAPERDOLLNAME, args);

		if (result) {
			name = Python2QString(result);
		}

		Py_XDECREF(result);
		Py_DECREF( args );
	}

	return name;
}

bool cBaseChar::onDeath( cUObject* source, P_ITEM corpse )
{
	bool result = false;
	if (canHandleEvent(EVENT_DEATH))
	{
		PyObject* args = Py_BuildValue( "(O&O&O&)", PyGetCharObject, this, PyGetObjectObject, source, PyGetItemObject, corpse );
		result = callEventHandler(EVENT_DEATH, args);
		Py_DECREF( args );
	}
	return result;
}

bool cBaseChar::onResurrect( cUObject* source )
{
	bool result = false;
	if (canHandleEvent(EVENT_RESURRECT))
	{
		PyObject* args = Py_BuildValue( "(O&O&)", PyGetCharObject, this, PyGetObjectObject, source );
		result = callEventHandler(EVENT_RESURRECT, args);
		Py_DECREF( args );
	}
	return result;
}

bool cBaseChar::onCHLevelChange( unsigned int level )
{
	bool result = false;
	if (canHandleEvent(EVENT_CHLEVELCHANGE))
	{
		PyObject* args = Py_BuildValue( "O&i", PyGetCharObject, this, level );
		result = callEventHandler(EVENT_CHLEVELCHANGE, args);
		Py_DECREF( args );
	}
	return result;
}

bool cBaseChar::onSkillGain( unsigned char skill, unsigned short min, unsigned short max, bool success )
{
	bool result = false;
	if (canHandleEvent(EVENT_SKILLGAIN))
	{
		PyObject* args = Py_BuildValue( "O&bhhi", PyGetCharObject, this, skill, min, max, success ? 1 : 0 );
		result = callEventHandler(EVENT_SKILLGAIN, args);
		Py_DECREF( args );
	}
	return result;
}

bool cBaseChar::kill( cUObject* source )
{
	if ( free || isDead() )
		return false;

	changed( TOOLTIP );
	changed_ = true;
	hitpoints_ = 0;
	updateHealth();
	setDead( true );
	setPoison( -1 );

	if ( isIncognito() )
	{
		setBody( orgBody() );
		setSkin( orgSkin() );
		setName( orgName() );
	}
	else if ( isPolymorphed() )
	{
		setBody( orgBody() );
		setSkin( orgSkin() );
	}

	setIncognito( false );
	setPolymorphed( false );

	P_CHAR pKiller = dynamic_cast<P_CHAR>( source );
	P_ITEM pTool = 0;

	// Were we killed by some sort of item?
	if ( source && !pKiller )
	{
		pTool = dynamic_cast<P_ITEM>( source );

		// If we were killed by some sort of tool (explosion potions)
		// the owner is responsible for the murder
		if ( pTool && pTool->owner() )
			pKiller = pTool->owner();
	}

	// Only trigger the reputation system if we can find someone responsible
	// for the murder
	if ( pKiller && pKiller != this )
	{
		// Only award karma and fame in unguarded areas
		if ( !pKiller->inGuardedArea() )
		{
			pKiller->awardFame( fame_ );
			pKiller->awardKarma( this, 0 - karma_ );
		}

		P_PLAYER pPlayer = dynamic_cast<P_PLAYER>( pKiller );

		// Only players can become criminal
		if ( pPlayer )
		{
			// Award fame and karma to the party members of this player if they can see the victim
			if ( pPlayer->party() )
			{
				QPtrList<cPlayer> members = pPlayer->party()->members();

				for ( P_PLAYER member = members.first(); member; member = members.next() )
				{
					if ( member != pPlayer && member->canSeeChar( this ) )
					{
						if ( !member->inGuardedArea() )
						{
							member->awardFame( fame_ );
							member->awardKarma( this, 0 - karma_ );
						}
					}
				}
			}

			if ( isInnocent() )
			{
				pPlayer->makeCriminal();
				pPlayer->setKills( pPlayer->kills() + 1 );
				setMurdererSerial( pPlayer->serial() );

				// Report the number of slain people to the player
				if ( pPlayer->socket() )
					pPlayer->socket()->sysMessage( tr( "You have killed %1 innocent people." ).arg( pPlayer->kills() ) );

				// The player became a murderer
				if ( pPlayer->kills() >= Config::instance()->maxkills() )
				{
					pPlayer->setMurdererTime( getNormalizedTime() + Config::instance()->murderdecay() * MY_CLOCKS_PER_SEC );

					if ( pPlayer->socket() )
						pPlayer->socket()->clilocMessage( 502134 );
				}
			}
		}
	} 

	// Fame is reduced by 10% upon death
	fame_ = ( int ) ( ( float ) fame_ * 0.90 );

	// Create the corpse
	cCorpse* corpse = 0;
	P_ITEM backpack = getBackpack();
	P_NPC npc = dynamic_cast<P_NPC>( this );
	P_PLAYER player = dynamic_cast<P_PLAYER>( this );

	bool summoned = npc && npc->summoned();

	// Kill Logging
	QString logName;
	if (npc) {
		if (summoned) {
			logName = tr("Summoned npc '%1' ('%2', 0x%3)").arg(name()).arg(baseid()).arg(serial_, 0, 16);
		} else {
			logName = tr("Npc '%1' ('%2', 0x%3)").arg(name()).arg(baseid()).arg(serial_, 0, 16);
		}
	} else if (player) {
		logName = tr("Player '%1' ('%2', 0x%3)").arg(name()).arg(player->account() ? player->account()->login() : QString("Unknown")).arg(serial_, 0, 16);
	}

	QString killerName;
	if (pKiller && pKiller != this) {
		P_NPC pKillerNpc = dynamic_cast<P_NPC>(pKiller);
		P_PLAYER pKillerPlayer = dynamic_cast<P_PLAYER>(pKiller);

		if (pKillerNpc) {
			if (pKillerNpc->summoned()) {
				killerName = tr("summoned npc '%1' ('%2', 0x%3)").arg(pKiller->name()).arg(pKiller->baseid()).arg(pKiller->serial(), 0, 16);
			} else {
				killerName = tr("npc '%1' ('%2', 0x%3)").arg(pKiller->name()).arg(pKiller->baseid()).arg(pKiller->serial(), 0, 16);
			}
		} else if (pKillerPlayer) {
			killerName = tr("player '%1' ('%2', 0x%3)").arg(pKiller->name()).arg(pKillerPlayer->account() ? pKillerPlayer->account()->login() : QString("Unknown")).arg(pKillerPlayer->serial(), 0, 16);
		}
	} else if (pKiller && pKiller == this) {
		killerName = tr("himself");
	} else if (!pKiller) {
		killerName = tr("accident");
	}

	log(LOG_TRACE, tr("%1 was killed by %2.\n").arg(logName).arg(killerName));

	if ( player )
		player->unmount();

	// If we are a creature type with a corpse and if we are not summoned
	// we create a corpse
	if ( !summoned && !isNoCorpse() )
	{
		corpse = new cCorpse( true );

		const cElement* elem = Definitions::instance()->getDefinition( WPDT_ITEM, "2006" );
		if ( elem )
			corpse->applyDefinition( elem );

		corpse->setName( name_ );
		corpse->setColor( skin_ );
		corpse->setBodyId( body_ );
		corpse->setTag( "human", cVariant( isHuman() ? 1 : 0 ) );
		corpse->setTag( "name", cVariant( name_ ) );

		// Storing the player's notoriety
		// So a singleclick on the corpse
		// Will display the right color
		corpse->setTag( "notoriety", cVariant( notoriety( this ) ) );
		corpse->setCharBaseid( baseid() );
		corpse->setOwner( this );
		corpse->moveTo( pos_, true );
		corpse->setDirection( direction() );
		corpse->setMurderTime( QDateTime::currentDateTime().toTime_t() );

		// stores the time and the murderer's name
		if ( pKiller )
		{
			corpse->setMurderer( pKiller->serial() );
		}

		// Move possible equipment to the corpse
		for ( unsigned char layer = SingleHandedWeapon; layer <= InnerLegs; ++layer )
		{
			P_ITEM item = getItemOnLayer( layer );

			if ( item )
			{
				if ( layer != Backpack && layer != Hair && layer != FacialHair )
				{
					// Put into the backpack
					if ( item->newbie() )
					{
						backpack->addItem( item );

						if ( player && player->socket() )
							item->update( player->socket() );
					}
					else
					{
						corpse->addItem( item );
						corpse->addEquipment( layer, item->serial() );
					}
				}
				else if ( layer == Hair )
				{
					corpse->setHairStyle( item->id() );
					corpse->setHairColor( item->color() );
				}
				else if ( layer == FacialHair )
				{
					corpse->setBeardStyle( item->id() );
					corpse->setBeardColor( item->color() );
				}
			}
		}

		corpse->update();
	}

	// Summoned monsters simply disappear
	if ( summoned )
	{
		soundEffect( 0x1fe );
		pos_.effect( 0x3735, 10, 30 );

		onDeath( source, 0 );
		remove();
		return true;
	}

	// Create Loot - Either on the corpse or on the ground
	for (ContainerIterator it(backpack); !it.atEnd(); ++it)
	{
		P_ITEM item = *it;
		if ( !item->newbie() )
		{
			if ( corpse )
				corpse->addItem( item );
			else
			{
				item->moveTo( pos_, true );
				item->update();
			}
		}
	}

	playDeathSound();

	cUOTxDeathAction dAction;
	dAction.setSerial( serial_ );

	if ( corpse )
		dAction.setCorpse( corpse->serial() );

	cUOTxRemoveObject rObject;
	rObject.setSerial( serial_ );

	for ( cUOSocket*mSock = Network::instance()->first(); mSock; mSock = Network::instance()->next() )
	{
		if ( mSock->player() && mSock->player()->inRange( this, mSock->player()->visualRange() ) )
		{
			if ( mSock->player() != this )
			{
				mSock->send( &dAction );
				mSock->send( &rObject );
			}
		}
	}

	onDeath( source, corpse );

	if ( npc )
	{
		remove();
	}

	if ( player )
	{
		// Create a death shroud for the player
		P_ITEM shroud = cItem::createFromScript( "204e" );
		if ( shroud )
		{
			addItem( OuterTorso, shroud );
			shroud->update();
		}

		player->resend( false );

		if ( player->socket() )
		{
			// Notify the player of his death
			cUOTxCharDeath death;
			player->socket()->send( &death );
		}

		// Notify the party that we died.
		if ( player->party() )
		{
			QString message;

			if ( source == player )
				message = tr( "I comitted suicide." );
			else if ( pKiller )
				message = tr( "I was killed by %1." ).arg( pKiller->name() );
			else
				message = tr( "I was killed." );

			player->party()->send( player, message );
		}
	}

	return true;
}

bool cBaseChar::canSee( cUObject* object )
{
	P_ITEM item = dynamic_cast<P_ITEM>( object );

	if ( item )
		return canSeeItem( item );

	P_CHAR character = dynamic_cast<P_CHAR>( object );

	if ( character )
		return canSeeChar( character );

	return false;
}

bool cBaseChar::canSeeChar( P_CHAR character )
{
	if ( character != this )
	{
		if ( !character || character->free )
			return false;

		if ( character->isInvisible() || character->isHidden() )
			return false;

		if ( character->isDead() )
		{
			// Only NPCs with spiritspeak >= 1000 can see dead people
			// or if the AI overrides it
			if ( !character->isAtWar() && skillValue( SPIRITSPEAK ) < 1000 )
				return false;
		}

		// Check distance
		if ( pos_.distance( character->pos() ) > VISRANGE )
			return false;

		// Check if the target is a npc and currently stabled
		P_NPC npc = dynamic_cast<P_NPC>( character );
		if ( npc && npc->stablemasterSerial() != INVALID_SERIAL )
		{
			return false;
		}

		// If it's a player see if it's logged out
		P_PLAYER player = dynamic_cast<P_PLAYER>( character );
		if ( player && !player->socket() && !player->logoutTime() )
		{
			return false;
		}
	}

	return true;
}

bool cBaseChar::canSeeItem( P_ITEM item )
{
	if ( !item )
		return false;

	if ( item->visible() == 2 )
		return false;
	else if ( item->visible() == 1 && item->owner() != this )
		return false;

	// Check for container
	if ( item->container() )
		return canSee( item->container() );
	else
	{
		cMulti* multi = dynamic_cast<cMulti*>( item );

		if ( multi )
		{
			if ( pos_.distance( item->pos() ) > BUILDRANGE )
				return false;
		}
		else
		{
			if ( pos_.distance( item->pos() ) > VISRANGE )
				return false;
		}
	}

	return true;
}

cFightInfo* cBaseChar::findFight( P_CHAR enemy )
{
	if ( enemy )
	{
		for ( cFightInfo*fight = fights_.first(); fight; fight = fights_.next() )
		{
			// We are only searching the fights we participate in, thats why we only
			// have to check for our enemy
			if ( fight->attacker() == enemy || fight->victim() == enemy )
				return fight;
		}
	}

	return 0;
}

cBaseChar::FightStatus cBaseChar::fight( P_CHAR enemy )
{
	FightStatus result = FightDenied;

	if ( !inWorld() )
	{
		return result;
	}

	// Ghosts can't fight
	if ( isDead() )
	{
		sysmessage( 500949 );
		return result;
	}

	// If we dont set any serial the attack is rejected
	cUOTxAttackResponse attack;
	attack.setSerial( INVALID_SERIAL );

	if ( enemy )
	{
		// Invisible or hidden creatures cannot be fought
		if ( !canSeeChar( enemy ) )
		{
			sysmessage( 500950 );
			enemy = 0;
		}
		else if ( enemy->isDead() )
		{
			sysmessage( "You cannot fight dead creatures." );
			enemy = 0;
		}
		else if ( enemy->isInvulnerable() )
		{
			sysmessage( 1061621 );
			enemy = 0;
		}
	}

	// If we are fighting someone and our target is null,
	// stop fighting.
	if ( !enemy )
	{
		// Update only if neccesary
		if ( attackTarget_ )
		{
			attackTarget_ = 0;
			send( &attack );
		}
		return result;
	}

	// If there already is an ongoing fight with our target,
	// simply return. Otherwise create the structure and fill it.
	cFightInfo* fight = findFight( enemy );

	if ( fight )
	{
		// There certainly is a reason to renew this fight
		fight->refresh();
		result = FightContinued;
	}
	else
	{
		// Check if it is legitimate to attack the enemy
		bool legitimate = enemy->notoriety( this ) != 0x01;
		fight = new cFightInfo( this, enemy, legitimate );

		// Display a message to the victim if our target changed to him
		if ( attackTarget() != enemy )
		{
			P_PLAYER player = dynamic_cast<P_PLAYER>( enemy );
			if ( player && player->socket() )
				player->socket()->showSpeech( this, tr( "*You see %1 attacking you.*" ).arg( name() ), 0x26, 3, cUOTxUnicodeSpeech::Emote );
		}
		result = FightStarted;
	}

	// Take care of the details
	attackTarget_ = enemy;
	unhide();

	// Accept the attack
	attack.setSerial( enemy->serial() );
	send( &attack );

	// Turn to our enemy
	turnTo( enemy );

	// See if we need to change our warmode status
	if ( !isAtWar() )
	{
		cUOTxWarmode warmode;
		warmode.setStatus( 1 );
		send( &warmode );
		setAtWar( true );
		update();
	}

	return result;
}

bool cBaseChar::sysmessage( const QString& message, unsigned short color, unsigned short font )
{
	Q_UNUSED( message );
	Q_UNUSED( color );
	Q_UNUSED( font );
	return false;
}

bool cBaseChar::sysmessage( unsigned int message, const QString& params, unsigned short color, unsigned short font )
{
	Q_UNUSED( message );
	Q_UNUSED( params );
	Q_UNUSED( color );
	Q_UNUSED( font );
	return false;
}

bool cBaseChar::message( const QString& message, unsigned short color, cUObject* source, unsigned short font, unsigned char mode )
{
	Q_UNUSED( message );
	Q_UNUSED( color );
	Q_UNUSED( source );
	Q_UNUSED( font );
	Q_UNUSED( mode );
	return false;
}

bool cBaseChar::send( cUOPacket* packet )
{
	Q_UNUSED( packet );
	return false;
}

void cBaseChar::poll( unsigned int time, unsigned int events )
{
	if ( events & EventCombat )
	{
		if ( attackTarget_ && nextSwing_ <= time )
		{
			P_CHAR target = attackTarget_;

			// Invulnerable or Dead target. Stop fighting.
			if ( isDead() || target->isInvulnerable() || target->isDead() )
			{
				fight( 0 );
				return;
			}

			// Check weapon range
			unsigned char range = 1;
			P_ITEM weapon = getWeapon();

			if ( weapon )
			{
				if ( weapon->hasTag( "range" ) ) {
					range = weapon->getTag( "range" ).toInt();
				} else if ( weapon->basedef() ) {
					range = weapon->basedef()->getIntProperty( "range", 1 );
				}
			}

			// We are out of range
			if ( pos().distance( target->pos() ) > range )
				return;

			// Can we see our target?
			if ( !canSee( attackTarget_ ) )
				return;

			// Ranged weapons don't need a touch, all other weapons need it.
			if ( weapon && ( weapon->type() == 1006 || weapon->type() == 1007 ) )
			{
				if ( !lineOfSight( attackTarget_ ) )
				{
					return;
				}
			}
			else if ( !lineOfSight( attackTarget_ ) )
			{
				return;
			}

			cPythonScript* global = ScriptManager::instance()->getGlobalHook( EVENT_SWING );

			if ( global )
			{
				PyObject* args = Py_BuildValue( "O&O&i", PyGetCharObject, this, PyGetCharObject, attackTarget_, time );
				global->callEvent( EVENT_SWING, args );
				Py_DECREF( args );
			}
		}
	}
}

bool cBaseChar::isInnocent()
{
	return !isMurderer() && !isCriminal();
}

void cBaseChar::refreshMaximumValues()
{
	if ( objectType() == enPlayer )
		maxHitpoints_ = wpMax<ushort>( 1, ( ( strength_ ) / 2 ) + hitpointsBonus_ + 50 );

	maxStamina_ = wpMax<ushort>( 1, dexterity_ + staminaBonus_ );
	maxMana_ = wpMax<ushort>( 1, intelligence_ + manaBonus_ );
}

bool cBaseChar::lineOfSight( P_CHAR target, bool debug )
{
	if (target == this) {
		return true;
	}

    // From us (eye) to target (no eye)
	return pos_.losCharPoint(true).lineOfSight(target->pos().losCharPoint(), debug);
}

bool cBaseChar::lineOfSight( P_ITEM target, bool debug )
{
	target = target->getOutmostItem();
	Coord pos;

	if (target->container() && target->container()->isChar()) {
		pos = target->container()->pos().losCharPoint(false);
	} else {
		pos = target->pos().losItemPoint(target->id());
	}

	return pos_.losCharPoint(true).lineOfSight(pos, debug);
}

bool cBaseChar::lineOfSight( const Coord& target, bool debug )
{	
	return pos_.losCharPoint(true).lineOfSight(target.losMapPoint(), debug);
}

bool cBaseChar::lineOfSight( const Coord& target, unsigned short id, bool debug )
{
	return pos_.losCharPoint(true).lineOfSight(target.losItemPoint(id), debug);
}

double cBaseChar::getHitpointRate()
{
	// Do the math
	int points = 0;

	if ( hasTag( "regenhitpoints" ) )
	{
		points = wpMax<int>( 0, getTag( "regenhitpoints" ).toInt() );
	}

	return 1.0 / ( 0.1 * ( 1 + points ) );
}

double cBaseChar::getStaminaRate()
{
	if ( !isDead() )
	{
		double chance = ( double ) stamina() / ( double ) maxStamina();
		double value = sqrt( skillValue( FOCUS ) * 0.0005 );
		chance *= ( 1.0 - value );
		chance += value;
		checkSkill( FOCUS, ( int ) floor( ( 1.0 - chance ) * 1200 ), 1200 );
	}

	int points = 0;

	if ( hasTag( "regenstamina" ) )
	{
		points = getTag( "regenstamina" ).toInt();
	}

	points += static_cast<int>( skillValue( FOCUS ) * 0.01 );
	points = wpMax<int>( -1, points );

	return 1.0 / ( 0.1 * ( 2 + points ) );
}

double cBaseChar::getManaRate()
{
	if ( !isDead() )
	{
		double chance = ( double ) mana() / maxMana();
		double value = sqrt( skillValue( FOCUS ) * 0.0005 );
		chance *= ( 1.0 - value );
		chance += value;
		checkSkill( FOCUS, ( int ) floor( ( 1.0 - chance ) * 1200 ), 1200 );
	}

	if ( !isMeditating() ) {
		double chance = ( double ) mana() / maxMana();
		double value = sqrt( skillValue( MEDITATION ) * 0.0005 );
		chance *= ( 1.0 - value );
		chance += value;
		checkSkill( MEDITATION, ( int ) floor( ( 1.0 - chance ) * 1200 ), 1200 );
	}

	double medPoints = wpMin<double>( 13.0, ( intelligence() + skillValue( MEDITATION ) * 0.03 ) * ( skillValue( MEDITATION ) < 1000 ? 0.025 : 0.0275 ) );
	double focusPoints = skillValue( FOCUS ) * 0.005;

	// Wearing type 1009 items without the 'magearmor': 1 or 'spellchanneling': 1 flags
	// eliminates the meditation bonus
	for ( unsigned char layer = SingleHandedWeapon; layer < Mount; ++layer )
	{
		P_ITEM item = atLayer( ( enLayer ) layer );
		if ( item && ( item->type() == 1009 || item->type() == 1008 ) )
		{
			if ( !item->allowMeditation() && !item->hasTag( "magearmor" ) )
			{
				medPoints = 0;
				break;
			}
		}
	}

	int points = 2 + ( int ) ( focusPoints + medPoints + ( isMeditating() ? medPoints : 0.0 ) );

	// Grant a bonus for the char if present
	if ( hasTag( "regenmana" ) )
	{
		points += getTag( "regenmana" ).toInt();
	}

	return 1.0 / ( 0.1 * points );
}

// Light and Region checks
void cBaseChar::moveTo( const Coord& pos, bool noremove )
{
	cUObject::moveTo( pos, noremove );
	Territories::instance()->check( this );
}

void cBaseChar::remove()
{
	// Cancel any ongoing fight.
	QPtrList<cFightInfo> fights( this->fights() );
	fights.setAutoDelete( false );
	for ( cFightInfo*info = fights.first(); info; info = fights.next() )
	{
		info->clear(); // Queue this structure to be deleted
	}

	// Call the onDelete event.
	if (canHandleEvent(EVENT_DELETE)) {
		PyObject* args = Py_BuildValue( "(N)", getPyObject() );
		callEventHandler( EVENT_DELETE, args );
		Py_DECREF( args );
	}

	// Kill all timers
	TimerContainer::iterator iter = timers_.begin();
	while ( iter != timers_.end() ) {
		Timers::instance()->erase( *iter );
		delete *iter;
		++iter;
	}
	timers_.clear();

	removeFromView( false );

	free = true;

	setGuarding( 0 );

	// We need to remove the equipment here.
	cBaseChar::ItemContainer container( content() );
	cBaseChar::ItemContainer::const_iterator it( container.begin() );
	cBaseChar::ItemContainer::const_iterator end( container.end() );
	for ( ; it != end; ++it )
	{
		P_ITEM pItem = *it;
		if ( !pItem )
			continue;

		pItem->remove();
	}
	content_.clear(); // Clear the content

	if ( multi_ )
	{
		multi_->removeObject( this );
		multi_ = 0;
	}

	MapObjects::instance()->remove( this );
	cUObject::remove();
}

void cBaseChar::load( cBufferedReader& reader )
{
	load( reader, reader.version() );
	World::instance()->registerObject( this );
}

PyObject *cBaseChar::callEvent(ePythonEvent event, PyObject *args, bool ignoreErrors) {
	PyObject *result = 0;

	if (scriptChain) {
		result = cPythonScript::callChainedEvent(event, scriptChain, args);
	
		// Break if there has been a result already
		if (result && PyObject_IsTrue(result)) {
			return result;
		}
	}

	// call the basescripts
	if (basedef_) {
		const QPtrList<cPythonScript> &list = basedef_->baseScripts();
		QPtrList<cPythonScript>::const_iterator it(list.begin());
		for (; it != list.end(); ++it) {
			result = (*it)->callEvent(event, args, ignoreErrors);

			if (result && PyObject_IsTrue(result)) {
				return result;
			}
		}
	}

	// check for a global handler
	cPythonScript *globalHook = ScriptManager::instance()->getGlobalHook(event);

	if (globalHook) {
		result = globalHook->callEvent(event, args, ignoreErrors);
	}

	return result;
}

bool cBaseChar::canHandleEvent(ePythonEvent event) {
	// Is there a global event?
	cPythonScript *globalHook = ScriptManager::instance()->getGlobalHook(event);
	
	if (globalHook) {
		return true;
	}

	if (cPythonScript::canChainHandleEvent(event, scriptChain)) {
		return true;
	}

	if (basedef_) {
		const QPtrList<cPythonScript> &list = basedef_->baseScripts();
		QPtrList<cPythonScript>::const_iterator it(list.begin());
		for (; it != list.end(); ++it) {
			if ((*it)->canHandleEvent(event)) {
				return true;
			}
		}
	}

	return false;
}

bool cBaseChar::callEventHandler(ePythonEvent event, PyObject *args, bool ignoreErrors) {
	PyObject *result = callEvent(event, args, ignoreErrors);

	if (result) {
		if (PyObject_IsTrue(result)) {
			Py_DECREF(result);
			return true;
		} else {
			Py_DECREF(result);
		}
	}
	return false;
}

bool cBaseChar::hasScript( const QCString& name )
{
	if (basedef_) {
		const QPtrList<cPythonScript> &list = basedef_->baseScripts();
		QPtrList<cPythonScript>::const_iterator it(list.begin());
		for (; it != list.end(); ++it) {
			if ((*it)->name() == name) {
				return true;
			}
		}
	}

	return cUObject::hasScript(name);
}

void cBaseChar::playAttackSound() {
	stBodyInfo bodyinfo = CharBaseDefs::instance()->getBodyInfo(body_);
	int sound = -1;

	if (basedef_ && basedef_->attackSound().size() != 0) {
		sound = basedef_->attackSound()[RandomNum(0, basedef_->attackSound().size() - 1)];
	} else if (basedef_ && basedef_->basesound() != 0) {
		sound = basedef_->basesound();
	} else if (bodyinfo.attackSound != -1) {
		sound = bodyinfo.attackSound;
	} else if (bodyinfo.basesound != 0) {
		sound = bodyinfo.basesound;
	}

	if (sound >= 0) {
		soundEffect((unsigned short)sound);
	}
}

void cBaseChar::playIdleSound() {
	stBodyInfo bodyinfo = CharBaseDefs::instance()->getBodyInfo(body_);
	int sound = -1;

	if (basedef_ && basedef_->idleSound().size() != 0) {
		sound = basedef_->idleSound()[RandomNum(0, basedef_->idleSound().size() - 1)];
	} else if (basedef_ && basedef_->basesound() != 0) {
		sound = basedef_->basesound() + 1;
	} else if (bodyinfo.idleSound != -1) {
		sound = bodyinfo.idleSound;
	} else if (bodyinfo.basesound != 0) {
		sound = bodyinfo.basesound + 1;
	}

	if (sound >= 0) {
		soundEffect((unsigned short)sound);
	}
}

void cBaseChar::playHitSound() {
	stBodyInfo bodyinfo = CharBaseDefs::instance()->getBodyInfo(body_);
	int sound = -1;

	if (basedef_ && basedef_->hitSound().size() != 0) {
		sound = basedef_->hitSound()[RandomNum(0, basedef_->hitSound().size() - 1)];
	} else if (basedef_ && basedef_->basesound() != 0) {
		sound = basedef_->basesound() + 2;
	} else if (bodyinfo.hitSound != -1) {
		sound = bodyinfo.hitSound;
	} else if (bodyinfo.basesound != 0) {
		sound = bodyinfo.basesound + 2;
	}

	if (sound >= 0) {
		soundEffect((unsigned short)sound);
	}
}

void cBaseChar::playGetHitSound() {
	stBodyInfo bodyinfo = CharBaseDefs::instance()->getBodyInfo(body_);
	int sound = -1;

	if (basedef_ && basedef_->gethitSound().size() != 0) {
		sound = basedef_->gethitSound()[RandomNum(0, basedef_->gethitSound().size() - 1)];
	} else if (basedef_ && basedef_->basesound() != 0) {
		sound = basedef_->basesound() + 3;
	} else if (bodyinfo.gethitSound != -1) {
		sound = bodyinfo.gethitSound;
	} else if (bodyinfo.basesound != 0) {
		sound = bodyinfo.basesound + 3;
	}

	if (sound >= 0) {
		soundEffect((unsigned short)sound);
	}
}

void cBaseChar::playDeathSound() {
	if (isHuman()) {
		if (isFemale()) {
			soundEffect( 0x150 + RandomNum(0, 3) );
		} else {
			soundEffect( 0x15A + RandomNum(0, 3) );
		}
	} else {
		stBodyInfo bodyinfo = CharBaseDefs::instance()->getBodyInfo(body_);
		int sound = -1;

		if (basedef_ && basedef_->deathSound().size() != 0) {
			sound = basedef_->deathSound()[RandomNum(0, basedef_->deathSound().size() - 1)];
		} else if (basedef_ && basedef_->basesound() != 0) {
			sound = basedef_->basesound() + 4;
		} else if (bodyinfo.deathSound != -1) {
			sound = bodyinfo.deathSound;
		} else if (bodyinfo.basesound != 0) {
			sound = bodyinfo.basesound + 4;
		}

		if (sound >= 0) {
			soundEffect((unsigned short)sound);
		}
	}
}
