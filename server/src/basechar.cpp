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
#include "maps.h"
#include "inlines.h"
#include "sectors.h"
#include "multi.h"
#include "network/uosocket.h"
#include "network/uotxpackets.h"
#include "network.h"
#include "combat.h"
#include "items.h"
#include "basics.h"
#include "tilecache.h"
#include "pythonscript.h"
#include "scriptmanager.h"
#include "log.h"
#include "skills.h"
#include "definitions.h"
#include "config.h"

cBaseChar::cBaseChar()
{
	lastMovement_		= 0;
	attackTarget_		= 0;
	nextSwing_			= 0;
	body_				= 0x190;
	orgBody_			= 0x190;
	gender_				= 0;
	orgSkin_			= 0;
	propertyFlags_		= 0;
	weight_				= 0;
	dexterity_			= 0;
	dexterityMod_		= 0;
	maxStamina_			= 0;
	stamina_			= 0;
	strength_			= 0;
	strengthMod_		= 0;
	maxHitpoints_		= 0;
	hitpoints_			= 0;
	intelligence_		= 0;
	intelligenceMod_	= 0;
	maxMana_			= 0;
	mana_				= 0;
	karma_				= 0;
	fame_				= 0;
	kills_				= 0;
	deaths_				= 0;
	hunger_				= 6;
	hungerTime_			= 0;
	flag_				= 0x02;
	emoteColor_			= 0x23;
	creationDate_		= QDateTime::currentDateTime();
	stealthedSteps_		= 0;
	runningSteps_		= 0;
	murdererTime_		= 0;
	criminalTime_		= 0;
	skillDelay_			= 0;
	poison_				= -1;
    title_				= "";
	cUObject::name_		= "Man";
    orgName_			= "Man";
    skin_				= 0;
    region_				= NULL;
    saycolor_			= 0x1700;
    murdererSerial_		= INVALID_SERIAL;
    guarding_			= NULL;
	cUObject::pos_		= Coord_cl( 100, 100, 0, 0 );
	setDead(false);  // we want to live ;)

	saycolor_			= 600;
	hitpointsBonus_		= 0;
	staminaBonus_		= 0;
	manaBonus_			= 0;
	strengthCap_		= 125;
	dexterityCap_		= 125;
	intelligenceCap_	= 125;
	statCap_			= Config::instance()->statcap();
	skills_.resize(ALLSKILLS);
	regenHitpointsTime_	= 0;
	regenStaminaTime_	= 0;
	regenManaTime_		= 0;
}

cBaseChar::cBaseChar(const cBaseChar& right)
{
}

cBaseChar::~cBaseChar()
{
}

cBaseChar& cBaseChar::operator=(const cBaseChar& right)
{
	return *this;
}

void cBaseChar::buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions )
{
	cUObject::buildSqlString( fields, tables, conditions );
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
	fields.push_back( "characters.hitpointsbonus,characters.staminabonus,characters.manabonus");
	fields.push_back( "characters.strcap,characters.dexcap,characters.intcap,characters.statcap" );
	tables.push_back( "characters" );
	conditions.push_back( "uobjectmap.serial = characters.serial" );
}

static void characterRegisterAfterLoading( P_CHAR pc );

void cBaseChar::load( char **result, UINT16 &offset )
{
	cUObject::load( result, offset );

	// Broken Serial?
	if( !isCharSerial( serial() ) )
		throw QString( "Character has invalid char serial: 0x%1" ).arg( serial(), 0, 16 );

	SERIAL ser;

	orgName_ = result[offset++];
	title_ = result[offset++];
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
	poison_ = (char)atoi( result[offset++] );
	murdererTime_ = atoi( result[offset++] ) + Server::instance()->time();
	criminalTime_ = atoi( result[offset++] ) + Server::instance()->time();
	gender_ = atoi( result[offset++] );
	propertyFlags_ = atoi( result[offset++] );
	murdererSerial_ = atoi( result[offset++] );
	ser = atoi( result[offset++] );
	guarding_ = dynamic_cast<P_PLAYER>(FindCharBySerial( ser ));
	hitpointsBonus_ = atoi(result[offset++]);
	staminaBonus_ = atoi(result[offset++]);
	manaBonus_ = atoi(result[offset++]);
	strengthCap_ = atoi(result[offset++]);
	dexterityCap_ = atoi(result[offset++]);
	intelligenceCap_ = atoi(result[offset++]);
	statCap_ = atoi(result[offset++]);

	// Query the Skills for this character
	QString sql = "SELECT skill,value,locktype,cap FROM skills WHERE serial = '" + QString::number( serial() ) + "'";

	cDBResult res = PersistentBroker::instance()->query( sql );
	if( !res.isValid() )
		throw PersistentBroker::instance()->lastError();

	// Fetch row-by-row
	while( res.fetchrow() )
	{
		// row[0] = skill
		// row[1] = value
		// row[2] = locktype
		// row[3] = cap (unused!)
		UINT16 skill = res.getInt( 0 );
		UINT16 value = res.getInt( 1 );
		UINT8 lockType = res.getInt( 2 );
		UINT16 cap = res.getInt( 3 );

		if( lockType > 2 )
			lockType = 0;

		stSkillValue skValue;
		skValue.value = value;
		skValue.lock = lockType;
		skValue.cap = cap;
		skValue.changed = false;

		skills_[ skill ] = skValue;
	}

	res.free();

	characterRegisterAfterLoading( this );
	changed_ = false;
}

void cBaseChar::save()
{
	initSave;
	if (changed_) {
		setTable( "characters" );

		addField( "serial", serial() );
		addStrField( "name", orgName_ );
		addStrField( "title", title_ );
		addStrField( "creationdate", creationDate_.toString(Qt::ISODate) );
		addField( "body", body_ );
		addField( "orgbody", orgBody_ );
		addField( "skin", skin_ );
		addField( "orgskin", orgSkin_ );
		addField( "saycolor", saycolor_);
		addField( "emotecolor", emoteColor_);
		addField( "strength", strength_);
		addField( "strengthmod", strengthMod_);
		addField( "dexterity", dexterity_);
		addField( "dexteritymod", dexterityMod_);
		addField( "intelligence", intelligence_);
		addField( "intelligencemod", intelligenceMod_);
		addField( "maxhitpoints", maxHitpoints_);
		addField( "hitpoints", hitpoints_);
		addField( "maxstamina", maxStamina_);
		addField( "stamina", stamina_);
		addField( "maxmana", maxMana_);
		addField( "mana", mana_);
		addField( "karma", karma_);
		addField( "fame", fame_);
		addField( "kills", kills_);
		addField( "deaths", deaths_);
		addField( "hunger", hunger_);
		addField( "poison", poison_);
		addField( "murderertime", murdererTime_ ? murdererTime_ - Server::instance()->time() : 0);
		addField( "criminaltime", criminalTime_ ? criminalTime_ - Server::instance()->time() : 0);
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
		addCondition( "serial", serial() );
		saveFields;
	}

	QValueVector<stSkillValue>::iterator it;
	PersistentBroker::instance()->lockTable("skills");
	int i = 0;
	QCString query(256); // 256 byte should be enough
	for (it = skills_.begin(); it != skills_.end(); ++it, ++i) {
		if ((*it).changed) {
			query.sprintf("REPLACE INTO skills VALUES(%u,%u,%u,%u,%u);",
				serial_, i, (*it).value, (*it).lock, (*it).cap);
			PersistentBroker::instance()->executeQuery(query.data());
			(*it).changed = false;
		}
	}
	PersistentBroker::instance()->unlockTable("skills");

	cUObject::save();
}

bool cBaseChar::del()
{
	if( !isPersistent )
		return false; // We didn't need to delete the object

	PersistentBroker::instance()->addToDeleteQueue( "characters", QString( "serial = '%1'" ).arg( serial() ) );
	PersistentBroker::instance()->addToDeleteQueue( "skills", QString( "serial = '%1'" ).arg( serial() ) );
	changed_ = true;
	return cUObject::del();
}

static void characterRegisterAfterLoading(P_CHAR pc) {
	World::instance()->registerObject(pc);
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
void cBaseChar::updateHealth( void )
{
	RegionIterator4Chars cIter( pos() );
	for( cIter.Begin(); !cIter.atEnd(); cIter++ )
	{
		P_PLAYER pPlayer = dynamic_cast<P_PLAYER>(cIter.GetData());

		// Send only if target can see us
		if( !pPlayer || !pPlayer->socket() || !pPlayer->inRange( this, pPlayer->visualRange() ) || ( isHidden() && !pPlayer->isGM() && this != pPlayer ) )
			continue;

		pPlayer->socket()->updateHealth( this );
	}
}

void cBaseChar::action(unsigned char id, unsigned char speed, bool reverse)
{
	bool mounted = atLayer( Mount ) != 0;

	// Bow + Area Cast
	if (mounted) {
		switch (id) {
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
		case ANIM_ATTACK_2H_JAB:
		case ANIM_ATTACK_2H_WIDE:
		case ANIM_ATTACK_2H_DOWN:
			id = ANIM_HORSE_SLAP;
			break;
		case ANIM_ATTACK_1H_WIDE:
		case ANIM_ATTACK_1H_JAB:
		case ANIM_ATTACK_1H_DOWN:
		case ANIM_CAST_DIR:
		case ANIM_ATTACK_UNARM:
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

	if (body_ < 0x190 && body_ >= 0xc8) {
		// Animal specific translation
		switch (id) {
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
			switch(RandomNum(0, 1)) {
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
	action.setSpeed(speed);
	if (reverse) {
		action.setBackwards(1);
	}

	for( cUOSocket *socket = Network::instance()->first(); socket; socket = Network::instance()->next() )
	{
		if( socket->player() && socket->player()->inRange( this, socket->player()->visualRange() ) && ( !isHidden() || socket->player()->isGM() ) )
			socket->send( &action );
	}
}

P_ITEM cBaseChar::getWeapon() const
{
	// Check if we have something on our right hand
	P_ITEM rightHand = rightHandItem();
	if( rightHand && Combat::instance()->weaponSkill( rightHand ) != WRESTLING )
		return rightHand;

	// Check for two-handed weapons
	P_ITEM leftHand = leftHandItem();
	if( leftHand && Combat::instance()->weaponSkill( leftHand ) != WRESTLING )
		return leftHand;

	return NULL;
}

P_ITEM cBaseChar::getShield() const
{
	P_ITEM leftHand = leftHandItem();
	if( leftHand && leftHand->isShield() )
		return leftHand;
	return NULL;
}

void cBaseChar::setHairColor( UINT16 d)
{
	changed_ = true;
	cItem* pHair = GetItemOnLayer( 11 );
	if( pHair )
		pHair->setColor( d );
	pHair->update();
}

void cBaseChar::setHairStyle( UINT16 d)
{
	if( !isHair( d ) )
		return;
	changed_ = true;
	cItem* pHair = GetItemOnLayer( 11 );
	if( pHair )
	{
		pHair->setId( d );
	}
	else
	{
		pHair = new cItem;
		pHair->Init();

		pHair->setDye(1);
		pHair->setNewbie( true );
		pHair->setId( d );
		addItem( cBaseChar::Hair, pHair );
	}
	pHair->update();
}

void cBaseChar::setBeardColor( UINT16 d)
{
	changed_ = true;
	cItem* pBeard = GetItemOnLayer( 16 );
	if( pBeard )
		pBeard->setColor( d );
	pBeard->update();
}

void cBaseChar::setBeardStyle( UINT16 d)
{
	if( !isBeard( d ) )
		return;
	changed_ = true;
	cItem* pBeard = GetItemOnLayer( 16 );
	if( pBeard )
		pBeard->setId( d );
	else
	{
		pBeard = new cItem;
		pBeard->Init();

		pBeard->setDye(1);
		pBeard->setNewbie( true );
		pBeard->setId( d );
		addItem( cBaseChar::FacialHair, pBeard );
	}
	pBeard->update();
}

void cBaseChar::playDeathSound()
{
	if( orgBody_ == 0x0191 )
	{
		switch( RandomNum(0, 3) )
		{
		case 0:		soundEffect( 0x0150 );	break;// Female Death
		case 1:		soundEffect( 0x0151 );	break;// Female Death
		case 2:		soundEffect( 0x0152 );	break;// Female Death
		case 3:		soundEffect( 0x0153 );	break;// Female Death
		}
	} else if( orgBody_ == 0x0190 ) {
		switch( RandomNum(0, 3) )
		{
		case 0:		soundEffect( 0x015A );	break;// Male Death
		case 1:		soundEffect( 0x015B );	break;// Male Death
		case 2:		soundEffect( 0x015C );	break;// Male Death
		case 3:		soundEffect( 0x015D );	break;// Male Death
		}
	} else {
		bark(Bark_Death);
	}
}

// This should check soon if we are standing above our
// corpse and if so, merge with our corpse instead of
// just resurrecting
void cBaseChar::resurrect() {
	if ( !isDead() )
		return;

	cCorpse *corpse = 0;

	// See if there's his corpse at his feet
	cItemSectorIterator *iter = SectorMaps::instance()->findItems(pos(), 0);
	for (P_ITEM item = iter->first(); item; item = iter->next()) {
		corpse = dynamic_cast<cCorpse*>(item);

		if (!corpse || corpse->owner() != this) {
			corpse = 0;
		} else {
			break;
		}
	}
	delete iter;

	if (corpse && corpse->direction() != direction()) {
		setDirection(corpse->direction());
		update();
	}

	changed( TOOLTIP );
	changed_ = true;
	awardFame(0);
	soundEffect(0x0214);
	setBody(orgBody_);
	setSkin(orgSkin_);
	setDead(false);
	hitpoints_ = QMAX(1, (UINT16)(0.1 * maxHitpoints_));
	stamina_ = (UINT16)(0.1 * maxStamina_);
	mana_ = (UINT16)(0.1 * maxMana_);
	fight(0);
	P_ITEM backpack = getBackpack(); // Make sure he has a backpack

	// Delete what the user wears on layer 0x16 (Should be death shroud)
	P_ITEM pRobe = atLayer(OuterTorso);

	if (pRobe) {
		pRobe->remove();
	}

	if (!corpse) {
		pRobe = cItem::createFromScript("1f03");

		if (pRobe) {
			pRobe->setColor(0);
			pRobe->setHp(1);
			pRobe->setMaxhp(1);
			this->addItem(cBaseChar::OuterTorso, pRobe);
			pRobe->update();
		}

		resend(false);
	} else {
		// Move all items from the corpse to the backpack and then look for
		// previous equipment
		cItem::ContainerContent content = corpse->content();
		cItem::ContainerContent::iterator it;
		for (it = content.begin(); it != content.end(); ++it) {
			backpack->addItem(*it, false);
			(*it)->update();
		}

		for (unsigned char layer = SingleHandedWeapon; layer < Mount; layer++) {
			if (layer != Backpack && layer != Hair && layer != FacialHair) {
				P_ITEM item = atLayer((enLayer)layer);

				if (item) {
					backpack->addItem(item);
					item->update();
				}

				SERIAL equipment = corpse->getEquipment(layer);
				item = World::instance()->findItem(equipment);

				if (item && item->container() == backpack) {
					addItem((enLayer)layer, item);
					item->update();
				}
			}
		}

		unsigned char action = 0x15;
		if (corpse->direction() & 0x80) {
			action = 0x16;
		}

		corpse->remove();

		resend(false);

		// Let him "stand up"
		this->action(action, 2, true);
	}
}

void cBaseChar::turnTo( const Coord_cl &pos )
{
	INT16 xdif = (INT16)( pos.x - this->pos().x );
	INT16 ydif = (INT16)( pos.y - this->pos().y );
	UINT8 nDir;

	if( xdif == 0 && ydif < 0 )
		nDir = 0;
	else if( xdif > 0 && ydif < 0 )
		nDir = 1;
	else if( xdif > 0 && ydif ==0 )
		nDir = 2;
	else if( xdif > 0 && ydif > 0 )
		nDir = 3;
	else if( xdif ==0 && ydif > 0 )
		nDir = 4;
	else if( xdif < 0 && ydif > 0 )
		nDir = 5;
	else if( xdif < 0 && ydif ==0 )
		nDir = 6;
	else if( xdif < 0 && ydif < 0 )
		nDir = 7;
	else
		return;

	if (nDir != direction()) {
		setDirection(nDir);
		update();
	}
}

void cBaseChar::turnTo( cUObject *object )
{
	turnTo( object->pos() );
}

/*!
  Wears the given item and sends an update to those in range.
*/
void cBaseChar::wear( P_ITEM pi )
{
	UINT8 layer = pi->layer();

	if( !pi->container() )
	{
		pi->setLayer( 0 );
		if( !layer )
		{
			tile_st tile = TileCache::instance()->getTile( pi->id() );
			layer = tile.layer;
		}
	}

	if( !layer )
		return;

	this->addItem( static_cast<cBaseChar::enLayer>(layer), pi );

	cUOTxCharEquipment packet;
	packet.setWearer( this->serial() );
	packet.setSerial( pi->serial() );
	packet.fromItem( pi );
	for ( cUOSocket* socket = Network::instance()->first(); socket != 0; socket = Network::instance()->next() )
		if( socket->player() && socket->player()->inRange( this, socket->player()->visualRange() ) )
			socket->send( &packet );
}

void cBaseChar::unhide()
{
	setStealthedSteps(0);

	if (isHidden() && !isInvisible()) {
		setHidden(false);
		resend(false); // They cant see us anyway
		sysmessage(500814);
	}
}

int cBaseChar::CountItems( short ID, short col )
{
	// Dont you think it's better to search the char's equipment as well?
	UINT32 number = 0;
	ItemContainer container = content_;
	ItemContainer::const_iterator it  = container.begin();
	ItemContainer::const_iterator end = container.end();

	for( ; it != end; ++it )
	{
		P_ITEM pItem = *it;

		if( !pItem )
			continue;

		if( ( pItem->id() == ID ) && ( pItem->color() == col ) )
			++number;
	}

	P_ITEM pi = getBackpack();

	if( pi )
		number = pi->CountItems( ID, col );
	return number;
}

int cBaseChar::CountGold()
{
	return CountItems(0x0EED);
}

P_ITEM cBaseChar::GetItemOnLayer(unsigned char layer)
{
	return atLayer( static_cast<enLayer>(layer) );
}

P_ITEM cBaseChar::getBankbox()
{
	P_ITEM pi = atLayer(BankBox);

	if (!pi) {
		pi = cItem::createFromScript("9ab");
		pi->setOwner(this);
		pi->setType(1);
		addItem(BankBox, pi, true, false);
	}

	return pi;
}

P_ITEM cBaseChar::getBackpack()
{
	P_ITEM backpack = atLayer( Backpack );

	// None found so create one
	if (!backpack) {
		backpack = cItem::createFromScript("e75");
		backpack->setOwner(this);
		addItem(Backpack, backpack);
		backpack->update();
	}

	return backpack;
}

void cBaseChar::setSerial( const SERIAL ser )
{
	// This is not allowed
	if( ser == INVALID_SERIAL )
		return;

	if( this->serial() != INVALID_SERIAL )
		World::instance()->unregisterObject( this->serial() );

	cUObject::setSerial( ser );

	World::instance()->registerObject( this );
}

void cBaseChar::MoveTo(short newx, short newy, signed char newz) {
	cUObject::moveTo( Coord_cl(newx, newy, newz, pos().map) );
}

bool cBaseChar::Wears(P_ITEM pi)
{
	return (this == pi->container());
}

unsigned int cBaseChar::getSkillSum() const
{
	unsigned int sum = 0;

	QValueVector< stSkillValue >::const_iterator it = skills_.begin();
	for( ; it != skills_.end(); ++it )
		sum += (*it).value;

	return sum;		// this *includes* the decimal digit ie. xxx.y
}

void cBaseChar::Init( bool createSerial )
{
	changed( TOOLTIP );
	changed_ = true;
	cUObject::setSerial( INVALID_SERIAL );

	if( createSerial )
		this->setSerial( World::instance()->findCharSerial() );
}

bool cBaseChar::inGuardedArea()
{
	cTerritory* Region = Territories::instance()->region( this->pos().x, this->pos().y, this->pos().map );
	if( Region )
		return Region->isGuarded();
	else
		return false;
}

void cBaseChar::emote( const QString &emote, UI16 color )
{
	if( color == 0xFFFF )
		color = emoteColor_;

	cUOTxUnicodeSpeech textSpeech;
	textSpeech.setSource( serial() );
	textSpeech.setModel( body_ );
	textSpeech.setFont( 3 ); // Default Font
	textSpeech.setType( cUOTxUnicodeSpeech::Emote );
	textSpeech.setName( name() );
	textSpeech.setColor( color );
	textSpeech.setText( emote );

	for( cUOSocket *mSock = Network::instance()->first(); mSock; mSock = Network::instance()->next() )
		if( mSock->player() && mSock->player()->inRange( this, mSock->player()->visualRange() ) )
			mSock->send( &textSpeech );
}

bool cBaseChar::checkSkill( UI16 skill, SI32 min, SI32 max, bool advance )
{
	bool success = false;

	// Maximum Value of 120 for checks
	// I disabled this so you can make skillchecks for grandmasters that are still tough
	//if( max > 1200 )
	//    max = 1200;

	// How far is the players skill above the required minimum.
	int charrange = abs(skillValue(skill) - min);

	if( charrange < 0 )
		charrange = 0;

	// To avoid division by zero
	if( min == max )
		max += 1;

	// +100 means: *allways* a minimum of 10% for success
	float chance = ( ( (float) charrange * 890.0f ) / (float)(abs(max - min)) ) + 100.0f;

	if( chance > 990 )
		chance = 990;	// *allways* a 1% chance of failure

	if( chance >= RandomNum( 0, 1000 ) )
		success = true;

	// We can only advance when doing things which aren't below our ability
	if (advance && skillValue(skill) < max) {
	  	onSkillGain(skill, min, max, success);
	}

	return success;
}

cItem* cBaseChar::atLayer( cBaseChar::enLayer layer ) const
{
	ItemContainer::const_iterator it = content_.find(layer);
	if ( it != content_.end() )
		return it.data();
	return 0;
}

bool cBaseChar::Owns( P_ITEM pItem ) const
{
	if( !pItem )
		return false;

	return ( pItem->owner() == this );
}

void cBaseChar::addGuard( P_CHAR pPet, bool noGuardingChange )
{
	// Check if already existing in the guard list
	for( CharContainer::iterator iter = guardedby_.begin(); iter != guardedby_.end(); ++iter )
		if( *iter == pPet )
			return;

	if( !noGuardingChange )
	{
		if( pPet->guarding() )
			pPet->guarding()->removeGuard( pPet );

		pPet->setGuarding( this );
	}

	guardedby_.push_back( pPet );
}

void cBaseChar::removeGuard( P_CHAR pPet, bool noGuardingChange )
{
	for( CharContainer::iterator iter = guardedby_.begin(); iter != guardedby_.end(); ++iter )
		if( *iter == pPet )
		{
			guardedby_.erase( iter );
			break;
		}

	if( !noGuardingChange )
		pPet->setGuarding( 0 );
}

void cBaseChar::addTimer( cTimer *timer )
{
	timers_.push_back(timer);
}

void cBaseChar::removeTimer(cTimer *timer)
{
	TimerContainer::iterator iter = timers_.begin();
	while( iter != timers_.end() )
	{
		if( (*iter) == timer)
		{
			timers_.erase( iter );
			break;
		}
		++iter;
	}
}

void cBaseChar::processNode( const cElement *Tag )
{
	changed_ = true;
	QString TagName = Tag->name();
	QString Value = Tag->value();

	// <bindmenu>contextmenu</bindmenu>
	// <bindmenu id="contextmenu />
	if( TagName == "bindmenu" )
	{
		QString bindmenu = this->bindmenu();
		if( !Tag->getAttribute( "id" ).isNull() )
			this->setBindmenu( bindmenu.isEmpty() ? Tag->getAttribute( "id" ) : bindmenu + "," + Tag->getAttribute( "id" ) );
		else
			setBindmenu( bindmenu.isEmpty() ? Value : bindmenu + "," + Value );
	}

	//<backpack>
	//	<color>0x132</color>
	//	<item id="a">
	//	...
	//	<item id="z">
	//</backpack>
	else if( TagName == "backpack" )
	{
		P_ITEM pBackpack = getBackpack(); // This autocreates a backpack

		if( Tag->childCount() )
			pBackpack->applyDefinition( Tag );
	}

	//<stat type="str">100</stats>
	else if( TagName == "stat" )
	{
		if( Tag->hasAttribute( "type" ) )
		{
			QString statType = Tag->getAttribute( "type" );
			if( statType == "str" )
			{
				strength_ = Value.toLong();
				if( maxHitpoints_ == 0 )
					maxHitpoints_ = strength_;
				hitpoints_ = maxHitpoints_;
			}
			else if( statType == "dex" )
			{
				dexterity_ = Value.toLong();
				if( maxStamina_ == 0 )
					maxStamina_ = dexterity_;
				stamina_ = maxStamina_;
			}
			else if( statType == "int" )
			{
				intelligence_ = Value.toLong();
				if( maxMana_ == 0 )
					maxMana_ = intelligence_;
				mana_ = maxMana_;
			}
			else if( statType == "maxhp" || statType == "maxhitpoints" )
			{
				maxHitpoints_ = Value.toLong();
				hitpoints_ = maxHitpoints_;
			}
			else if( statType == "maxstm" || statType == "maxstamina" )
			{
				maxStamina_ = Value.toLong();
				stamina_ = maxStamina_;
			}
			else if( statType == "maxmn" || statType == "maxmana" )
			{
				maxMana_ = Value.toLong();
				mana_ = maxMana_;
			}
		}
	}

	// Aliasses <str <dex <int <maxhp <maxstm <maxmn <maxhitpoints <maxstamina <maxmana
	else if( TagName == "str" )
	{
		strength_ = Value.toLong();
		if( maxHitpoints_ == 0 )
			maxHitpoints_ = strength_;
		hitpoints_ = maxHitpoints_;
	}

	else if( TagName == "dex" )
	{
		dexterity_ = Value.toLong();
		if( maxStamina_ == 0 )
			maxStamina_ = dexterity_;
		stamina_ = maxStamina_;
	}

	else if( TagName == "int" )
	{
		intelligence_ = Value.toLong();
		if( maxMana_ == 0 )
			maxMana_ = intelligence_;
		mana_ = maxMana_;
	}

	else if( TagName == "maxhp" || TagName == "maxhitpoints" )
	{
		maxHitpoints_ = Value.toLong();
		hitpoints_ = maxHitpoints_;
	}

	else if( TagName == "maxstm" || TagName == "maxstamina" )
	{
		maxStamina_ = Value.toLong();
		stamina_ = maxStamina_;
	}

	else if( TagName == "maxmn" || TagName == "maxmana" )
	{
		maxMana_ = Value.toLong();
		mana_ = maxMana_;
	}

	//<gold>100</gold>
	else if( TagName == "gold" )
	{
		giveGold( Value.toInt(), false );
	}

	//<skill type="alchemy">100</skill>
	//<skill type="1">100</skill>
	else if( TagName == "skill" && Tag->hasAttribute("type") )
	{
		if( Tag->getAttribute("type").toInt() > 0 &&
			Tag->getAttribute("type").toInt() <= ALLSKILLS )
			setSkillValue( ( Tag->getAttribute( "type" ).toInt() - 1 ), Value.toInt() );
		else
		{
			INT16 skillId = Skills::instance()->findSkillByDef( Tag->getAttribute( "type", "" ) );
			setSkillValue( skillId, Value.toInt() );
		}
	}

	//<equipped>
	//	<item id="a" />
	//	<item id="b" />
	//	...
	//</epuipped>
	else if( TagName == "equipped" )
	{
		for( unsigned int i = 0; i < Tag->childCount(); ++i )
		{
			const cElement *element = Tag->getChild( i );

			if( element->name() == "item" )
			{
				P_ITEM pItem = 0;

				const QString &id = element->getAttribute( "id" );

				if( id != QString::null )
				{
					pItem = cItem::createFromScript( id );
				}
				else
				{
					const QString &list = element->getAttribute( "list" );

					if( list != QString::null )
					{
						pItem = cItem::createFromList( list );
					}
				}

				if( pItem )
				{
					pItem->applyDefinition( element );

					unsigned char mLayer = pItem->layer();
					pItem->setLayer( 0 );

					// Instead of deleting try to get a valid layer instead
					if( !mLayer )
					{
						tile_st tInfo = TileCache::instance()->getTile( pItem->id() );
						if( tInfo.layer > 0 )
							mLayer = tInfo.layer;
					}

					if( !mLayer )
						pItem->remove();
					else
					{
						// Check if there is sth there already.
						// Could happen due to inherit.
						P_ITEM existing = atLayer(static_cast<cBaseChar::enLayer>(mLayer));
						if ( existing )
							existing->remove();
						addItem( static_cast<cBaseChar::enLayer>(mLayer), pItem );
					}
				}
				else
				{
					Console::instance()->log( LOG_ERROR, QString( "Invalid equipped element missing id and list attribute in npc definition '%1'." ).arg( element->getTopmostParent()->getAttribute( "id", "unknown" ) ) );
				}
			}
			else
			{
				Console::instance()->log( LOG_ERROR, QString( "Invalid equipped element '%1' in npc definition '%2'." ).arg( element->name() ).arg( element->getTopmostParent()->getAttribute( "id", "unknown" ) ) );
			}
		}
	}
	// <saycolor>color</saycolor>
	else if ( TagName == "saycolor" )
	{
		bool ok;
		ushort color = Value.toUShort(&ok);
		if ( ok )
			this->setSaycolor( color );
	}
	else
	{
		INT16 skillId = Skills::instance()->findSkillByDef( TagName );

		if( skillId == -1 )
			cUObject::processNode( Tag );
		else
			setSkillValue( skillId, Value.toInt() );
	}

}

void cBaseChar::addItem( cBaseChar::enLayer layer, cItem* pi, bool handleWeight, bool noRemove )
{
	if (pi->multi()) {
		// Ignore the pseudo-pointer if uninitialized
		if (!pi->unprocessed()) {
			pi->multi()->removeObject(pi);
		}
		pi->setMulti(0);
	}

	if (atLayer(layer) != 0) {
		log(LOG_ERROR, QString("Trying to put item 0x%1 on layer %2 which is already occupied.\n").arg(pi->serial(), 0, 16).arg(layer));
		pi->container_ = 0;
		pi->moveTo(pos_, true);
		return;
	}

	if( !noRemove )
	{
		pi->removeFromCont();
	}

	content_.insert( (ushort)(layer), pi );

	if (pi->layer() != layer) {
		pi->setLayer(layer);
	}

	if (Server::instance()->getState() != STARTUP) {
		pi->setContainer(this);
	} else {
		pi->container_ = this; // Avoid a flagChanged()
	}

	if (handleWeight && (pi->layer() < 0x1A || pi->layer() == 0x1E)) {
		weight_ += pi->totalweight();
	}

	if (!noRemove) {
		// Dragging doesnt count as Equipping
		if( layer != Dragging )
			pi->onEquip( this, layer );
	}
}

void cBaseChar::removeItem( cBaseChar::enLayer layer, bool handleWeight )
{
	P_ITEM pi = atLayer(layer);
	if ( pi )
	{
		pi->setContainer(0);
		pi->setLayer( 0 );
		content_.remove((ushort)(layer));

		if( handleWeight )
			weight_ -= pi->totalweight();

		// Dragging doesnt count as Equipping
		if( layer != Dragging )
			pi->onUnequip( this, layer );
	}
}

// Simple setting and getting of properties for scripts and the set command.
stError *cBaseChar::setProperty( const QString &name, const cVariant &value )
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
	else SET_STR_PROPERTY( "title", title_ )
	/*
		\property char.incognito This boolean property indicates whether the character is incognito.
	*/
	else if( name == "incognito" )
	{
		setIncognito( value.toInt() );
		return 0;
	}
	/*
		\property char.polymorph This boolean property indicates whether the character is polymorphed.
	*/
	else if( name == "polymorph" )
	{
		setPolymorphed( value.toInt() );
		return 0;
	}
	/*
		\property char.dead This boolean property indicates whether the character is dead.
	*/
	else if( name == "dead" )
	{
		setDead( value.toInt() );
		return 0;
	}
	/*
		\property char.haircolor This integer property sets the haircolor of the character.
		This property is write only.
	*/
	else if( name == "haircolor" )
	{
		bool ok;
		INT32 data = value.toInt( &ok );
		if( !ok )
			PROPERTY_ERROR( -2, "Integer expected" )
		setHairColor( data );
		return 0;
	}
	/*
		\property char.hairstyle This integer property sets the hairstyle of the character.
		This property is write only.
	*/
	else if( name == "hairstyle" )
	{
		bool ok;
		INT32 data = value.toInt( &ok );
		if( !ok )
			PROPERTY_ERROR( -2, "Integer expected" )
		setHairStyle( data );
		return 0;
	}
	/*
		\property char.beardcolor This integer property sets the beardcolor of the character.
		This property is write only.
	*/
	else if( name == "beardcolor" )
	{
		bool ok;
		INT32 data = value.toInt( &ok );
		if( !ok )
			PROPERTY_ERROR( -2, "Integer expected" )
		setBeardColor( data );
		return 0;
	}
	/*
		\property char.beardstyle This integer property sets the beardstyle of the character.
		This property is write only.
	*/
	else if( name == "beardstyle" )
	{
		bool ok;
		INT32 data = value.toInt( &ok );
		if( !ok )
			PROPERTY_ERROR( -2, "Integer expected" )
		setBeardStyle( data );
		return 0;
	}
	/*
		\property char.skin This integer property contains the skin color of the character.
	*/
	else SET_INT_PROPERTY( "skin", skin_ )

	/*
		\property char.maxhitpoints This integer property contains the maximum hitpoints for the character.
		Please note that the maximum hitpoints are constantly recalculated. Please see hitpointsbonus for
		a better way to increase the maximum hitponts of a character.
	*/
	else SET_INT_PROPERTY( "maxhitpoints", maxHitpoints_ )
	/*
		\property char.maxstamina This integer property contains the maximum stamina for the character.
		Please note that the maximum stamina are constantly recalculated. Please see staminabonus for
		a better way to increase the maximum stamina of a character.
	*/
	else SET_INT_PROPERTY( "maxstamina", maxStamina_ )
	/*
		\property char.maxmana This integer property contains the maximum mana for the character.
		Please note that the maximum mana are constantly recalculated. Please see manabonus for
		a better way to increase the maximum mana of a character.
	*/
	else SET_INT_PROPERTY( "maxmana", maxMana_ )
	/*
		\property char.lastmovement This integer property indicates the servertime of the last movement
		of this character.
	*/
	else SET_INT_PROPERTY( "lastmovement", lastMovement_ )
	/*
		\property char.orgskin This integer property indicates the original skin of the character that is restored when he dies or
		a spell that changed the skin wears off.
	*/
	else SET_INT_PROPERTY( "orgskin", orgSkin_ )
	/*
		\property char.creationdate This string property indicates the date and time the character was created.
	*/
	else if( name == "creationdate" )
	{
		creationDate_ = QDateTime::fromString( value.toString() );
		return 0;
	}
	/*
		\property char.stealthedsteps This integer property indicates how many steps the character walked when stealthed already.
	*/
	else SET_INT_PROPERTY( "stealthedsteps", stealthedSteps_ )
	/*
		\property char.runningsteps This integer property indicates how many steps the character is running so far.
	*/
	else SET_INT_PROPERTY( "runningsteps", runningSteps_ )
	/*
		\property char.tamed This boolean property indicates whether the character is tamed or not.
	*/
	else if( name == "tamed" )
	{
		setTamed( value.toInt() );
		return 0;
	}
	/*
		\property char.guarding This character property is the character that is currently guarded by this character.
	*/
	else SET_CHAR_PROPERTY( "guarding", guarding_ )
	/*
		\property char.murderer This integer property indicates the serial of the character who killed this character last.
	*/
	else SET_INT_PROPERTY( "murderer", murdererSerial_ )
	/*
		\property char.casting This boolean property indicates whether the character is currently casting a spell.
	*/
	else if( name == "casting" )
	{
		setCasting( value.toInt() );
		return 0;
	}
	/*
		\property char.hidden This boolean property indicates whether the character is currently hidden.
	*/
	else if( name == "hidden" )
	{
		setHidden( value.toInt() );
		return 0;
	}

	/*
		\property char.hunger This integer property indicates the food level of the character. 0 is the lowest food level, 6 the highest.
	*/
	else SET_INT_PROPERTY( "hunger", hunger_ )

	/*
		\property char.hungertime This integer property is the next servertime the foodlevel of this character will be reduced.
	*/
	else SET_INT_PROPERTY( "hungertime", hungerTime_ )

	/*
		\property char.poison The strength of the poison applied to this character.
		A value of -1 means that no poision is applied to this character.
	*/
	else SET_INT_PROPERTY("poison", poison_)

	/*
		\property char.flag The ingame notoriety for this character.
	*/
	else SET_INT_PROPERTY("flag", flag_)

	/*
		\property propertyflags The bitfield (32 bit) with basechar properties. You can use the
		upper 8 bits for custom properties.
	*/
	else SET_INT_PROPERTY("propertyflags", propertyFlags_)

	/*
		\property char.murderertime This integer property indicates when the next kill of the murder count will be removed.
	*/
	else SET_INT_PROPERTY( "murderertime", murdererTime_ )
	/*
		\property char.criminaltime This integer property indicates the servertime when the criminal flag of this character will wear off.
	*/
	else SET_INT_PROPERTY( "criminaltime", criminalTime_ )
	/*
		\property char.meditating This boolean property indicates whether this character is currently meditating.
	*/
	else if( name == "meditating" )
	{
		setMeditating( value.toInt() );
		return 0;
	}
	/*
		\property char.weight This float property indicates the total weight of this character.
	*/
	else SET_FLOAT_PROPERTY( "weight", weight_ )
	/*
		\property char.saycolor This integer property is the saycolor of this character.
	*/
	else SET_INT_PROPERTY( "saycolor", saycolor_ )
	/*
		\property char.emotecolor This integer property is the emote color of this character.
	*/
	else SET_INT_PROPERTY( "emotecolor", emoteColor_ )
	/*
		\property char.strength This integer property is the strength of this character.
	*/
	else if (name == "strength") {
		setStrength(value.toInt());
		return 0;
	/*
		\property char.dexterity This integer property is the dexterity of this character.
	*/
	} else if (name == "dexterity") {
		setDexterity(value.toInt());
		return 0;
	/*
		\property char.intelligence This integer property is the intelligence of this character.
	*/
	} else if (name == "intelligence") {
		setIntelligence(value.toInt());
		return 0;
	/*
		\property char.strength2 This integer property contains a modification value applied to strength. This is used to
		determine the real strength of the character if needed.
	*/
	} else if (name == "strength2") {
		setStrengthMod(value.toInt());
		return 0;
	/*
		\property char.dexterity2 This integer property contains a modification value applied to dexterity. This is used to
		determine the real dexterity of the character if needed.
	*/
	} else if (name == "dexterity2") {
		setDexterityMod(value.toInt());
		return 0;
	/*
		\property char.intelligence2 This integer property contains a modification value applied to intelligence. This is used to
		determine the real intelligence of the character if needed.
	*/
	} else if (name == "intelligence2") {
		setIntelligenceMod(value.toInt());
		return 0;
	}
	/*
		\property char.orgid This is the original body id of the character that is restored when he dies or any spell affecting it
		expires.
	*/
	else SET_INT_PROPERTY( "orgid", orgBody_ )
	/*
		\property char.hitpoints The current hitpoints of this character.
	*/
	else SET_INT_PROPERTY( "hitpoints", hitpoints_ )
	/*
		\property char.health The current hitpoints of this character.
	*/
	else SET_INT_PROPERTY( "health", hitpoints_ )
	/*
		\property char.stamina The current stamina of this character.
	*/
	else SET_INT_PROPERTY( "stamina", stamina_ )
	/*
		\property char.mana The current mana of this character.
	*/
	else SET_INT_PROPERTY( "mana", mana_ )
	/*
		\property char.karma The current karma of this character.
	*/
	else SET_INT_PROPERTY( "karma", karma_ )
	/*
		\property char.fame The current fame of this character.
	*/
	else SET_INT_PROPERTY( "fame", fame_ )
	/*
		\property char.kills The current kills of this character.
		This is used to determine the murderer status.
	*/
	else SET_INT_PROPERTY( "kills", kills_ )
	/*
		\property char.deaths The current deaths of this character.
	*/
	else SET_INT_PROPERTY( "deaths", deaths_ )
	/*
		\property char.war This boolean property indicates whether the character is in warmode or not.
	*/
	else if( name == "war" )
	{
		setAtWar( value.toInt() );
		return 0;
	}
	/*
		\property char.attacktarget The attack target of this character.
	*/
	else SET_CHAR_PROPERTY( "attacktarget", attackTarget_ )
	/*
		\property char.nextswing The servertime the character will be able to swing (attack) next.
	*/
	else SET_INT_PROPERTY( "nextswing", nextSwing_ )
	/*
		\property char.regenhealth The next servertime the character will try to regenerate hitpoints.
	*/
	else SET_INT_PROPERTY( "regenhealth", regenHitpointsTime_ )
	/*
		\property char.regenstamina The next servertime the character will try to regenerate stamina.
	*/
	else SET_INT_PROPERTY( "regenstamina", regenStaminaTime_ )
	/*
		\property char.regenmana The next servertime the character will try to regenerate mana.
	*/
	else SET_INT_PROPERTY( "regenmana", regenManaTime_ )
	/*
		\property char.skilldelay The servertime the character will be able to use another active skill again.
	*/
	else SET_INT_PROPERTY( "skilldelay", skillDelay_ )
	/*
		\property char.gender The gender of this character. False means male, true female.
	*/
	else SET_INT_PROPERTY( "gender", gender_ )
	/*
		\property char.id The body id of this character.
	*/
	else SET_INT_PROPERTY( "id", body_ )
	/*
		\property char.hitpointsbonus The integer bonus awarded to the maximum hitpoints of this character.
	*/
	else if (name == "hitpointsbonus") {
		setHitpointsBonus(value.toInt());
		return 0;
	/*
		\property char.staminabonus The integer bonus awarded to the maximum stamina of this character.
	*/
	} else if (name == "staminabonus") {
		setStaminaBonus(value.toInt());
		return 0;
	/*
		\property char.manabonus The integer bonus awarded to the maximum mana of this character.
	*/
	} else if (name == "manabonus") {
		setManaBonus(value.toInt());
		return 0;
	/*
		\property char.invulnerable Indicates whether the character is invulnerable or not.
	*/
	} else if( name == "invulnerable" )
	{
		setInvulnerable( value.toInt() );
		return 0;
	}
	/*
		\property char.invisible Indicates whether the character is invisible or not.
	*/
	else if( name == "invisible" )
	{
		setInvisible( value.toInt() );
		return 0;
	}
	/*
		\property char.frozen Indicates whether the character is frozen or not.
	*/
	else if( name == "frozen" )
	{
		setFrozen( value.toInt() );
		return 0;
	}
	/*
		\property char.strengthcap The individual strength cap for this character.
	*/
	else SET_INT_PROPERTY( "strengthcap", strengthCap_ )
	/*
		\property char.dexteritycap The individual dexterity cap for this character.
	*/
	else SET_INT_PROPERTY( "dexteritycap", dexterityCap_ )
	/*
		\property char.intelligencecap The individual intelligence cap for this character.
	*/
	else SET_INT_PROPERTY( "intelligencecap", intelligenceCap_ )
	/*
		\property char.statcap The individual total stat cap for this character.
	*/
	else SET_INT_PROPERTY( "statcap", statCap_ )

	return cUObject::setProperty( name, value );
}

stError *cBaseChar::getProperty( const QString &name, cVariant &value ) const
{
	GET_PROPERTY( "orgname", orgName_ )
	else GET_PROPERTY( "lastmovement", (int)lastMovement_ )
	else GET_PROPERTY( "title", title_ )
	else GET_PROPERTY( "incognito", isIncognito() )
	else GET_PROPERTY( "polymorph", isPolymorphed() )
	else GET_PROPERTY( "skin", skin_ )
	else GET_PROPERTY( "orgskin", orgSkin_ )
	else GET_PROPERTY( "creationdate", creationDate_.toString() )
	else GET_PROPERTY( "stealthedsteps", stealthedSteps_ )
	else GET_PROPERTY( "runningsteps", (int)runningSteps_ )
	else GET_PROPERTY( "tamed", isTamed() )
	else GET_PROPERTY( "guarding", guarding_ )
	else GET_PROPERTY( "murderer", FindCharBySerial( murdererSerial_ ) )
	else GET_PROPERTY( "casting", isCasting() )
	else GET_PROPERTY( "hidden", isHidden() )
	else GET_PROPERTY( "hunger", hunger_ )
	else GET_PROPERTY( "hungertime", (int)hungerTime_ )
	else GET_PROPERTY( "poison", poison_ )
	else GET_PROPERTY( "flag", flag_ )
	else GET_PROPERTY( "propertyflags", (int)propertyFlags_ )
	else GET_PROPERTY( "murderertime", (int)murdererTime_ )
	else GET_PROPERTY( "criminaltime", (int)criminalTime_ )
	else GET_PROPERTY( "meditating", isMeditating() )
	else GET_PROPERTY( "weight", weight_ )
	else GET_PROPERTY( "saycolor", saycolor_ )
	else GET_PROPERTY( "emotecolor", emoteColor_ )
	else GET_PROPERTY( "strength", strength_ )
	else GET_PROPERTY( "dexterity", dexterity_ )
	else GET_PROPERTY( "intelligence", intelligence_ )
	else GET_PROPERTY( "strength2", strengthMod_ )
	else GET_PROPERTY( "dexterity2", dexterityMod_ )
	else GET_PROPERTY( "intelligence2", intelligenceMod_ )
	else GET_PROPERTY( "orgid", orgBody_ )
	else GET_PROPERTY( "maxhitpoints", maxHitpoints_ )
	else GET_PROPERTY( "hitpoints", hitpoints_ )
	else GET_PROPERTY( "strengthcap", strengthCap_ )
	else GET_PROPERTY( "dexteritycap", dexterityCap_ )
	else GET_PROPERTY( "intelligencecap", intelligenceCap_ )
	else GET_PROPERTY( "statcap", statCap_ )
	else GET_PROPERTY( "health", hitpoints_ )
	else GET_PROPERTY( "maxstamina", maxStamina_ )
	else GET_PROPERTY( "stamina", stamina_ )
	else GET_PROPERTY( "maxmana", maxMana_ )
	else GET_PROPERTY( "mana", mana_ )
	else GET_PROPERTY( "karma", karma_ )
	else GET_PROPERTY( "fame", fame_ )
	else GET_PROPERTY( "kills", (int)kills_ )
	else GET_PROPERTY( "deaths", (int)deaths_ )
	else GET_PROPERTY( "dead", isDead() )
	else GET_PROPERTY( "war", isAtWar() )
	else GET_PROPERTY( "attacktarget", attackTarget_ )
	else GET_PROPERTY( "nextswing", (int)nextSwing_ )
	else GET_PROPERTY( "regenhealth", (int)regenHitpointsTime_ )
	else GET_PROPERTY( "regenstamina", (int)regenStaminaTime_ )
	else GET_PROPERTY( "regenmana", (int)regenManaTime_ )
	else GET_PROPERTY( "region", ( region_ != 0 ) ? region_->name() : QString( "" ) )
	else GET_PROPERTY( "skilldelay", (int)skillDelay_ )
	else GET_PROPERTY( "gender", gender_ )
	else GET_PROPERTY( "id", body_ )
	else GET_PROPERTY( "invulnerable", isInvulnerable() )
	else GET_PROPERTY( "invisible", isInvisible() )
	else GET_PROPERTY( "frozen", isFrozen() )
	else GET_PROPERTY( "hitpointsbonus", hitpointsBonus_ )
	else GET_PROPERTY( "staminabonus", staminaBonus_ )
	else GET_PROPERTY( "manabonus", manaBonus_ )

	// skill.
	else if( name.left( 6 ) == "skill." )
	{
		QString skill = name.right( name.length() - 6 );
		INT16 skillId = Skills::instance()->findSkillByDef( skill );

		if( skillId != -1 )
		{
			value = cVariant( this->skillValue( skillId ) );
			return 0;
		}
	// skillcap.
	} else if( name.left( 9 ) == "skillcap." ) {
		QString skill = name.right( name.length() - 9 );
		INT16 skillId = Skills::instance()->findSkillByDef( skill );

		if( skillId != -1 )
		{
			value = cVariant( this->skillCap( skillId ) );
			return 0;
		}

	} else {
		// See if there's a skill by that name
		INT16 skillId = Skills::instance()->findSkillByDef(name);

		if (skillId != -1) {
			value = cVariant(skillValue(skillId));
			return 0;
		}
	}


	return cUObject::getProperty( name, value );
}

void cBaseChar::setSkillValue( UINT16 skill, UINT16 value )
{
	skills_[skill].value = value;
	skills_[skill].changed = true;
}

void cBaseChar::setSkillCap( UINT16 skill, UINT16 cap )
{
	skills_[skill].cap = cap;
	skills_[skill].changed = true;
}

void cBaseChar::setSkillLock( UINT16 skill, UINT8 lock )
{
	if( lock > 2 )
		lock = 0;

	skills_[skill].lock = lock;
	skills_[skill].changed = true;
}

UINT16 cBaseChar::skillValue( UINT16 skill ) const
{
	return skills_[ skill ].value;
}

UINT16 cBaseChar::skillCap( UINT16 skill ) const
{
	return skills_[ skill ].cap;
}

UINT8 cBaseChar::skillLock( UINT16 skill ) const
{
	return skills_[ skill ].lock;
}

void cBaseChar::setStamina(INT16 data, bool notify /* = true */ )
{
    stamina_ = data;
	changed_ = true;
}

void cBaseChar::callGuards()
{
	if (!inGuardedArea() || !Config::instance()->guardsActive() )
		return;

	// Is there a criminal around?
	RegionIterator4Chars ri(pos());
	for( ri.Begin(); !ri.atEnd(); ri++ )
	{
		P_CHAR pc = ri.GetData();
		if( pc )
		{
			if( !pc->isDead() && !pc->isInnocent() && inRange( pc, 14 ) )
			{
				Combat::instance()->spawnGuard( pc, pc, pc->pos() );
			}
		}
	}
}

unsigned int cBaseChar::damage( eDamageType type, unsigned int amount, cUObject *source )
{
	if (isFrozen()) {
		setFrozen(false);
		resendTooltip();
	}

	//
	// First of all, call onDamage with the damage-type, amount and source
	// to modify the damage if needed
	//
	if( scriptChain )
	{
		PyObject *args = 0;
		if( dynamic_cast< P_CHAR >( source ) != 0 )
			args = Py_BuildValue( "O&iiO&", PyGetCharObject, this, (unsigned int)type, amount, PyGetCharObject, source );
		else if( dynamic_cast< P_ITEM >( source ) )
			args = Py_BuildValue( "O&iiO&", PyGetCharObject, this, (unsigned int)type, amount, PyGetItemObject, source );
		else
			args = Py_BuildValue( "O&iiO", PyGetCharObject, this, (unsigned int)type, amount, Py_None );

		PyObject *result = cPythonScript::callChainedEvent( EVENT_DAMAGE, scriptChain, args );

		if( result )
		{
			if( PyInt_Check( result ) )
				amount = PyInt_AsLong( result );

			Py_DECREF( result );
		}

		Py_DECREF( args );
	}

	// The damage has been resisted or scripts have taken care of the damage otherwise
	// Invulnerable Targets don't take any damage at all
	if (amount == 0 || isInvulnerable()) {
		return 0;
	}

	P_PLAYER player = dynamic_cast<P_PLAYER>(source);

	if (!player) {
		P_ITEM tool = dynamic_cast<P_ITEM>(source);
		if (tool && tool->owner()) {
			player = dynamic_cast<P_PLAYER>(tool->owner());
		}
	}

	// Show the amount of damage dealt over the head of the victim
	if (player && player->socket()) {
		cUOTxDamage damage;
		damage.setUnknown1(1);
		damage.setDamage(amount);
		damage.setSerial(serial_);
		player->socket()->send(&damage);
	}

	// There is a 33% chance that blood is created on hit by phsical means
	if (type == DAMAGE_PHYSICAL && !RandomNum(0,2)) {
		P_ITEM blood = 0;

		// If more than 50% of the maximum healthpoints has been dealt as damage
		// we display a big puddle of blood
		if (amount >= maxHitpoints_ * 0.50) {
			blood = cItem::createFromList("BIG_BLOOD_PUDDLES");

		// Otherwise we display a medium puddle of blood if the damage is greater
		// than 25% of the maximum healthpoints
		} else if (amount >= maxHitpoints_ * 0.35) {
			blood = cItem::createFromList("MEDIUM_BLOOD_PUDDLES");

		// at last we only display a small stain of blood if the damage has been
		// greater than 10% of the maximum hitpoints
		} else if (amount >= maxHitpoints_ * 0.20) {
			blood = cItem::createFromList("SMALL_BLOOD_PUDDLES");
		}

		if (blood) {
			blood->moveTo(pos_); // Move it to the feet of the victim
			blood->setNoDecay(false); // Override the nodecay tag in the definitions
			blood->setDecayTime(Server::instance()->time() + 20 * MY_CLOCKS_PER_SEC); // Let it decay in 20 seconds from now
			blood->update(); // Send it to all sockets in range
		}
	}

	// Would we die?
	if( amount >= hitpoints_ ) {
		kill(source);
	} else {
		hitpoints_ -= amount;
		updateHealth();
		Combat::instance()->playGetHitSoundEffect(this);
		Combat::instance()->playGetHitAnimation(this);
	}

	return amount;
}

void cBaseChar::bark( enBark type )
{
	if( body() == 0x190 || body() == 0x192 )
	{
		if( type == Bark_GetHit )
		{
			unsigned short sound = hex2dec( Definitions::instance()->getRandomListEntry( "SOUNDS_COMBAT_HIT_HUMAN_MALE" ) ).toUShort();

			if( sound > 0 )
				soundEffect( sound );
			else
				soundEffect( 0x156 );

			return;
		}
	}
	else if( body() == 0x191 || body() == 0x193 )
	{
		unsigned short sound = hex2dec( Definitions::instance()->getRandomListEntry( "SOUNDS_COMBAT_HIT_HUMAN_FEMALE" ) ).toUShort();
		if( sound > 0 )
			soundEffect( sound );
		else
			soundEffect( 0x14b );
	}

	cCharBaseDef *basedef = BaseDefManager::instance()->getCharBaseDef( body_ );

	if( !basedef || !basedef->basesound() )	// Nothing known about this creature
		return;

	switch( basedef->soundmode() )
	{
	// Only Attack, Hit and Death sounds available (Falltrough!)
	case 2:
		if( type == Bark_GetHit )
			return;

	// Only Attack, Hit, GetHit and Death
	case 3:
		if( type == Bark_Idle )
			return;

		break;

	// Only the first sound is available
	case 4:
		if( type != Bark_Attacking )
			return;

	default:
		break;
	}

	soundEffect( basedef->basesound() + (unsigned char)type );
}

void cBaseChar::goldSound( unsigned short amount, bool hearall )
{
	unsigned short sound;

	if( amount == 1 )
		sound = 0x35;
	else if( amount > 1 && amount < 6 )
		sound = 0x36;
	else
		sound = 0x37;

	soundEffect( sound, hearall );
}

void cBaseChar::showPaperdoll(cUOSocket *source, bool hotkey) {
	if( !source )
		return;

	P_PLAYER pChar = source->player();

	if( !pChar || onShowPaperdoll( pChar ) )
		return;

	// For players we'll always show the Paperdoll
	if (isHuman() || objectType() != enNPC) {

		// If we're mounted (item on layer 25) and *not* using a hotkey
		// We're trying to unmount
		if (!hotkey && (this == pChar) && pChar->unmount())
			return; // We have been unmounted

		source->sendPaperdoll(this);
	}

	// Is that faster ??
	switch( body_ )
	{
	case 0x004E:
	case 0x0050:
	case 0x003A:
	case 0x0039:
	case 0x003B:
	case 0x0074:
	case 0x0075:
	case 0x0072:
	case 0x007A:
	case 0x0084:
	case 0x0073:
	case 0x0076:
	case 0x0077:
	case 0x0078:
	case 0x0079:
	case 0x00AA:
	case 0x00AB:
	case 0x00BB:
	case 0x0090:
	case 0x00C8:
	case 0x00E2:
	case 0x00E4:
	case 0x00CC:
	case 0x00DC:
	case 0x00D2:
	case 0x00DA:
	case 0x00DB:
	case 0x0317:
	case 0x0319:
	case 0x031A:
	case 0x031F:
		// Try to mount the rideable animal
		if( dist( pChar ) <  2 || pChar->isGM() )
		{
			if (!pChar->isHuman()) {
				source->clilocMessage(1061628); // You can't do that while polymorphed
				return;
			}

			if (pChar->isDead()) {
				source->clilocMessage(500949);
				return;
			}

			if( isAtWar() )
				source->sysMessage( tr( "Your pet is in battle right now!" ) );
			else
				pChar->mount( dynamic_cast<P_NPC>( this ) );
		} else {
			source->clilocMessage(500206);
		}

		break;
	case 0x123:
	case 0x124:
		if( objectType() == enNPC && dynamic_cast<P_NPC>(this)->owner() == pChar )
				source->sendContainer( getBackpack() );
	};
}

/*
	Event Wrappers
 */

bool cBaseChar::onWalk( unsigned char direction, unsigned char sequence )
{
	bool result = false;

	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&bb", PyGetCharObject, this, direction, sequence );
		result = cPythonScript::callChainedEventHandler( EVENT_WALK, scriptChain, args );
		Py_DECREF( args );
	}

	return result;
}

bool cBaseChar::onTalk( unsigned char type, unsigned short color, unsigned short font, const QString &text, const QString &lang )
{
	bool result = false;

	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&bhhuu", PyGetCharObject, this, type, color, font, text.ucs2(), lang.ucs2() );
		result = cPythonScript::callChainedEventHandler( EVENT_TALK, scriptChain, args );
		Py_DECREF( args );
	}

	return result;
}

bool cBaseChar::onWarModeToggle( bool war )
{
	bool result = false;

	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&i", PyGetCharObject, this, war ? 1 : 0 );
		result = cPythonScript::callChainedEventHandler( EVENT_WARMODETOGGLE, scriptChain, args );
		Py_DECREF( args );
	}

	return result;
}

bool cBaseChar::onShowPaperdoll( P_CHAR pOrigin )
{
	bool result = false;

	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&O&", PyGetCharObject, this, PyGetCharObject, pOrigin );
		result = cPythonScript::callChainedEventHandler( EVENT_SHOWPAPERDOLL, scriptChain, args );
		Py_DECREF( args );
	}

	return result;
}

bool cBaseChar::onShowSkillGump()
{
	PyObject *args = Py_BuildValue("(N)", getPyObject());
	bool result = cPythonScript::callChainedEventHandler(EVENT_SHOWSKILLGUMP, scriptChain);
	Py_DECREF(args);
	return result;
}

bool cBaseChar::onSkillUse( unsigned char skill )
{
	cPythonScript *global = ScriptManager::instance()->getGlobalHook( EVENT_SKILLUSE );
	bool result = false;

	if( scriptChain || global )
	{
		PyObject *args = Py_BuildValue( "O&b", PyGetCharObject, this, skill );

		result = cPythonScript::callChainedEventHandler( EVENT_SKILLUSE, scriptChain, args );

		if( !result && global )
			result = global->callEventHandler( EVENT_SKILLUSE, args );

		Py_DECREF( args );
	}

	return result;
}

bool cBaseChar::onDropOnChar( P_ITEM pItem )
{
	bool result = false;

	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&O&", PyGetCharObject, this, PyGetItemObject, pItem );
		result = cPythonScript::callChainedEventHandler( EVENT_DROPONCHAR, scriptChain, args );
		Py_DECREF( args );
	}

	return result;
}

QString cBaseChar::onShowPaperdollName( P_CHAR pOrigin )
{
	// I hate this event by the way (DarkStorm)
	QString name = QString::null;

	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "O&O&", PyGetCharObject, this, PyGetCharObject, pOrigin );

		PyObject *result = cPythonScript::callChainedEvent( EVENT_SHOWPAPERDOLLNAME, scriptChain, args );

		if( result )
		{
			// Strings and Unicode Objects gladly accepted
			if (PyString_Check(result)) {
				name = PyString_AsString(result);
			} else if (PyUnicode_Check(result)) {
				name = QString::fromUcs2((ushort*)PyUnicode_AS_UNICODE(result));
			}
		}

		Py_XDECREF( result );
		Py_DECREF( args );
	}

	return name;
}

bool cBaseChar::onDeath(cUObject *source, P_ITEM corpse)
{
	bool result = false;

	if( scriptChain )
	{
		PyObject *args = Py_BuildValue( "(O&O&O&)", PyGetCharObject, this, PyGetObjectObject, source, PyGetItemObject, corpse );
		result = cPythonScript::callChainedEventHandler( EVENT_DEATH, scriptChain, args );
		Py_DECREF( args );
	}

	return result;
}

bool cBaseChar::onCHLevelChange( unsigned int level )
{
	cPythonScript *global = ScriptManager::instance()->getGlobalHook( EVENT_CHLEVELCHANGE );
	bool result = false;

	if( scriptChain || global )
	{
		PyObject *args = Py_BuildValue( "O&i", PyGetCharObject, this, level );

		result = cPythonScript::callChainedEventHandler( EVENT_CHLEVELCHANGE, scriptChain, args );

		if( !result && global )
			result = global->callEventHandler( EVENT_CHLEVELCHANGE, args );

		Py_DECREF( args );
	}

	return result;
}
/*
bool cBaseChar::onShowTooltip( P_PLAYER sender, cUOTxTooltipList* tooltip )
{
	cPythonScript *global = ScriptManager::instance()->getGlobalHook( EVENT_SHOWTOOLTIP );
	bool result = false;

	if( scriptChain || global )
	{
		PyObject *args = Py_BuildValue( "O&O&O&", PyGetCharObject, sender, PyGetCharObject, this, PyGetTooltipObject, tooltip );

		result = cPythonScript::callChainedEventHandler( EVENT_SHOWTOOLTIP, scriptChain, args );

		if( !result && global )
			result = global->callEventHandler( EVENT_SHOWTOOLTIP, args );

		Py_DECREF( args );
	}

	return result;
}
*/
bool cBaseChar::onSkillGain( unsigned char skill, unsigned short min, unsigned short max, bool success )
{
	cPythonScript *global = ScriptManager::instance()->getGlobalHook( EVENT_SKILLGAIN );
	bool result = false;

	if( scriptChain || global )
	{
		PyObject *args = Py_BuildValue( "O&bhhi", PyGetCharObject, this, skill, min, max, success ? 1 : 0 );

		result = cPythonScript::callChainedEventHandler( EVENT_SKILLGAIN, scriptChain, args );

		if( !result && global )
			result = global->callEventHandler( EVENT_SKILLGAIN, args );

		Py_DECREF( args );
	}

	return result;
}

bool cBaseChar::kill(cUObject *source) {
	if (free || isDead())
		return false;

	changed(TOOLTIP);
	changed_ = true;
	hitpoints_ = 0;
	updateHealth();
	setDead(true);
	setPoison(-1);

	if (isIncognito()) {
		setBody(orgBody());
		setSkin(orgSkin());
		setName(orgName());
	} else if (isPolymorphed()) {
		setBody(orgBody());
		setSkin(orgSkin());
	}

	setIncognito(false);
	setPolymorphed(false);

	P_CHAR pKiller = dynamic_cast<P_CHAR>(source);
	P_ITEM pTool = 0;

	// Were we killed by some sort of item?
	if (source && !pKiller)
	{
		pTool = dynamic_cast<P_ITEM>(source);

		// If we were killed by some sort of tool (explosion potions)
		// the owner is responsible for the murder
		if (pTool && pTool->owner())
			pKiller = pTool->owner();
	}

	// Only trigger the reputation system if we can find someone responsible
	// for the murder
	if (pKiller && pKiller != this)
	{
		// Only award karma and fame in unguarded areas
		if (!pKiller->inGuardedArea())
		{
			pKiller->awardFame(fame_);
			pKiller->awardKarma(this, 0 - karma_);
		}

		P_PLAYER pPlayer = dynamic_cast<P_PLAYER>(pKiller);

		// Only players can become criminal
		if (pPlayer)
		{
			// Award fame and karma to the party members of this player if they can see the victim
			if (pPlayer->party())
			{
				QPtrList<cPlayer> members = pPlayer->party()->members();

				for (P_PLAYER member = members.first(); member; member = members.next())
				{
					if (member != pPlayer && member->canSeeChar(this))
					{
						if (!member->inGuardedArea())
						{
							member->awardFame(fame_);
							member->awardKarma(this, 0 - karma_);
						}
					}
				}
			}

			if (isInnocent())
			{
				pPlayer->makeCriminal();
				pPlayer->setKills(pPlayer->kills() + 1);
				setMurdererSerial(pPlayer->serial());

				// Report the number of slain people to the player
				if (pPlayer->socket())
					pPlayer->socket()->sysMessage(tr("You have killed %1 innocent people.").arg(pPlayer->kills()));

				// The player became a murderer
				if (pPlayer->kills() >= Config::instance()->maxkills())
				{
					pPlayer->setMurdererTime(getNormalizedTime() + Config::instance()->murderdecay() * MY_CLOCKS_PER_SEC);

					if (pPlayer->socket())
						pPlayer->socket()->clilocMessage(502134);
				}
			}
		}
	}

	// Fame is reduced by 10% upon death
	fame_ *= 0.90;

	// Create the corpse
	cCorpse *corpse = 0;
	P_ITEM backpack = getBackpack();
	P_NPC npc = dynamic_cast<P_NPC>(this);
	P_PLAYER player = dynamic_cast<P_PLAYER>(this);

	bool summoned = npc && npc->summoned();

	if (player)
		player->unmount();

	cCharBaseDef *basedef = BaseDefManager::instance()->getCharBaseDef(body_);

	// If we are a creature type with a corpse and if we are not summoned
	// we create a corpse
	if (!summoned && basedef && !basedef->noCorpse()) {
		corpse = new cCorpse(true);

		const cElement *elem = Definitions::instance()->getDefinition(WPDT_ITEM, "2006");
		if (elem)
			corpse->applyDefinition(elem);

		corpse->setName(name_);
		corpse->setColor(skin_);
		corpse->setBodyId(body_);
		corpse->setTag("human", cVariant(isHuman() ? 1 : 0 ));
		corpse->setTag("name", cVariant(name_));

		// Storing the player's notoriety
		// So a singleclick on the corpse
		// Will display the right color
		corpse->setTag("notoriety", cVariant(notoriety(this)));

		if (npc)
			corpse->setCarve(npc->carve());

        corpse->setOwner(this);
		corpse->moveTo(pos_);
		corpse->setDirection(direction());

		// stores the time and the murderer's name
		if (pKiller)
		{
			corpse->setMurderer(pKiller->name());
			corpse->setMurderTime(Server::instance()->time());
		}

		// Move possible equipment to the corpse
		for (unsigned char layer = SingleHandedWeapon; layer <= InnerLegs; ++layer)
		{
			P_ITEM item = GetItemOnLayer(layer);

			if (item)
			{
				if (layer != Backpack && layer != Hair && layer != FacialHair) {
					// Put into the backpack
					if (item->newbie())
					{
						backpack->addItem(item);

						if (player && player->socket())
							item->update(player->socket());
					}
					else
					{
						corpse->addItem(item);
						corpse->addEquipment(layer, item->serial());
					}
				}
				else if (layer == Hair)
				{
					corpse->setHairStyle(item->id());
					corpse->setHairColor(item->color());
				}
				else if (layer == FacialHair)
				{
					corpse->setBeardStyle(item->id());
					corpse->setBeardColor(item->color());
				}
			}
		}

		corpse->setDecayTime(Server::instance()->time() + Config::instance()->corpseDecayTime() * MY_CLOCKS_PER_SEC);
		corpse->update();
	}

	// Create Loot - Either on the corpse or on the ground
	cItem::ContainerContent content = backpack->content();
	cItem::ContainerContent::iterator it;

	for (it = content.begin(); it != content.end(); ++it) {
		P_ITEM item = *it;
		if (!item->newbie())
		{
			if (corpse)
				corpse->addItem(item);
			else
			{
				item->moveTo(pos_);
				item->update();
			}
		}
	}

	// Create Loot for NPcs
	if (npc && !npc->lootList().isEmpty())
	{
		QStringList lootlist = Definitions::instance()->getList(npc->lootList());

		QStringList::const_iterator it;
		for (it = lootlist.begin(); it != lootlist.end(); ++it)
		{
			P_ITEM loot = cItem::createFromScript(*it);

			if (loot)
			{
				if (corpse)
					corpse->addItem(loot);
				else
				{
					loot->moveTo(pos_);
					loot->update();
				}
			}
		}
	}

	// Summoned monsters simply disappear
	if (summoned)
	{
		soundEffect(0x1fe);
		pos_.effect(0x3735, 10, 30);

		onDeath(source, 0);
		remove();
		return true;
	}

	playDeathSound();

	cUOTxDeathAction dAction;
	dAction.setSerial(serial_);

	if (corpse)
		dAction.setCorpse(corpse->serial());

	cUOTxRemoveObject rObject;
	rObject.setSerial(serial_);

	for( cUOSocket *mSock = Network::instance()->first(); mSock; mSock = Network::instance()->next() )
	{
		if (mSock->player() && mSock->player()->inRange( this, mSock->player()->visualRange()))
		{
			if (mSock->player() != this)
			{
				mSock->send(&dAction);
				mSock->send(&rObject);
			}
		}
	}

	onDeath(source, corpse);

	if (npc) {
		remove();
	}

	if (player)
	{
		// Create a death shroud for the player
		P_ITEM shroud = cItem::createFromScript("204e");
		if (shroud)
		{
			addItem(OuterTorso, shroud);
			shroud->update();
		}

		player->resend(false);

		if (player->socket())
		{
			// Notify the player of his death
			cUOTxCharDeath death;
			player->socket()->send(&death);
		}

		// Notify the party that we died.
		if (player->party())
		{
			QString message;

			if (source == player)
				message = tr("I comitted suicide.");
			else if (pKiller)
				message = tr("I was killed by %1.").arg(pKiller->name());
			else
				message = tr("I was killed.");

			player->party()->send(player, message);
		}
	}

	return true;
}

bool cBaseChar::canSee(cUObject *object)
{
	P_ITEM item = dynamic_cast<P_ITEM>(object);

	if (item)
		return canSeeItem(item);

	P_CHAR character = dynamic_cast<P_CHAR>(object);

	if (character)
		return canSeeChar(character);

	return false;
}

bool cBaseChar::canSeeChar(P_CHAR character)
{
	if (character != this)
	{
		if (!character || character->free)
			return false;

		if (character->isInvisible() || character->isHidden())
			return false;

		if (character->isDead())
		{
			// Only NPCs with spiritspeak >= 1000 can see dead people
			// or if the AI overrides it
			if (!character->isAtWar() && skillValue(SPIRITSPEAK) < 1000)
				return false;
		}

		// Check distance
		if (pos_.distance(character->pos()) > VISRANGE)
			return false;

		// Check if the target is a npc and currently stabled
		P_NPC npc = dynamic_cast<P_NPC>(character);
		if (npc && npc->stablemasterSerial() != INVALID_SERIAL) {
			return false;
		}

		// If it's a player see if it's logged out
		P_PLAYER player = dynamic_cast<P_PLAYER>(character);
		if (player && !player->socket() && !player->logoutTime()) {
			return false;
		}
	}

	return true;
}

bool cBaseChar::canSeeItem(P_ITEM item)
{
	if (!item)
		return false;

	if (item->visible() == 2)
		return false;
	else if (item->visible() == 1 && item->owner() != this)
		return false;

	// Check for container
	if (item->container())
		return canSee(item->container());
	else
	{
		cMulti *multi = dynamic_cast<cMulti*>(item);

		if (multi) {
			if (pos_.distance(item->pos()) > BUILDRANGE)
				return false;
		} else {
			if (pos_.distance(item->pos()) > VISRANGE)
				return false;
		}
	}

	return true;
}

cFightInfo *cBaseChar::findFight(P_CHAR enemy)
{
	if (enemy)
	{
		for (cFightInfo *fight = fights_.first(); fight; fight = fights_.next())
		{
			// We are only searching the fights we participate in, thats why we only
			// have to check for our enemy
			if (fight->attacker() == enemy || fight->victim() == enemy)
				return fight;
		}
	}

	return 0;
}

cBaseChar::FightStatus cBaseChar::fight(P_CHAR enemy)
{
	FightStatus result = FightDenied;

	if (!inWorld()) {
		return result;
	}

	// Ghosts can't fight
	if (isDead()) {
		sysmessage(500949);
		return result;
	}

	// If we dont set any serial the attack is rejected
	cUOTxAttackResponse attack;
	attack.setSerial(INVALID_SERIAL);

	if (enemy)
	{
		// Invisible or hidden creatures cannot be fought
		if (!canSeeChar(enemy))
		{
			sysmessage(500950);
			enemy = 0;
		}
		else if (enemy->isDead())
		{
			sysmessage("You cannot fight dead creatures.");
			enemy = 0;
		}
		else if (enemy->isInvulnerable())
		{
			sysmessage(1061621);
			enemy = 0;
		}
	}

	// If we are fighting someone and our target is null,
	// stop fighting.
	if (!enemy)
	{
		// Update only if neccesary
		if (attackTarget_) {
			attackTarget_ = 0;
			send(&attack);
		}
		return result;
	}

	// If there already is an ongoing fight with our target,
	// simply return. Otherwise create the structure and fill it.
	cFightInfo *fight = findFight(enemy);

	if (fight)
	{
		// There certainly is a reason to renew this fight
		fight->refresh();
		result = FightContinued;
	}
	else
	{
		// Check if it is legitimate to attack the enemy
		bool legitimate = enemy->notoriety(this) != 0x01;
		fight = new cFightInfo(this, enemy, legitimate);

		// Display a message to the victim if our target changed to him
		if (attackTarget() != enemy)
		{
			P_PLAYER player = dynamic_cast<P_PLAYER>(enemy);
			if (player && player->socket())
				player->socket()->showSpeech(this, tr("*You see %1 attacking you.*").arg(name()), 0x26, 3, cUOTxUnicodeSpeech::Emote);
		}
		result = FightStarted;
	}

	// Take care of the details
	attackTarget_ = enemy;
	unhide();

	// Accept the attack
	attack.setSerial(enemy->serial());
	send(&attack);

	// Turn to our enemy
	turnTo(enemy);

	// See if we need to change our warmode status
	if (!isAtWar()) {
		cUOTxWarmode warmode;
		warmode.setStatus(1);
		send(&warmode);
		setAtWar(true);
		update();
	}

	return result;
}

//void cPlayer::fight(P_CHAR enemy) {
/*
	// The person being attacked is guarded by pets ?
	cBaseChar::CharContainer guards = pc_i->guardedby();
	for( cBaseChar::CharContainer::iterator iter = guards.begin(); iter != guards.end(); ++iter )
	{
		P_NPC pPet = dynamic_cast<P_NPC>(*iter);
		if( pPet->combatTarget() == INVALID_SERIAL && pPet->inRange( _player, Config::instance()->attack_distance() ) ) // is it on screen?
		{
			pPet->fight( pc_i );

			// Show the You see XXX attacking YYY messages
			QString message = tr( "*You see %1 attacking %2*" ).arg( pPet->name() ).arg( pc_i->name() );
			for( cUOSocket *mSock = Network::instance()->first(); mSock; mSock = Network::instance()->next() )
				if( mSock->player() && mSock->player() != pc_i && mSock->player()->inRange( pPet, mSock->player()->visualRange() ) )
					mSock->showSpeech( pPet, message, 0x26, 3, cUOTxUnicodeSpeech::Emote );

			if( pc_i->objectType() == enPlayer )
			{
				P_PLAYER pp = dynamic_cast<P_PLAYER>(pc_i);
				if( pp->socket() )
					pp->socket()->showSpeech( pPet, tr( "*You see %1 attacking you*" ).arg( pPet->name() ), 0x26, 3, cUOTxUnicodeSpeech::Emote );
			}
		}
	}

	if( pc_i->inGuardedArea() && Config::instance()->guardsActive() )
	{
		if( pc_i->objectType() == enPlayer && pc_i->isInnocent() ) //REPSYS
		{
			_player->makeCriminal();
			Combat::instance()->spawnGuard( _player, pc_i, _player->pos() );
		}
		else if( pc_i->objectType() == enNPC && pc_i->isInnocent() && !pc_i->isHuman() )//&& pc_i->npcaitype() != 4 )
		{
			_player->makeCriminal();
			Combat::instance()->spawnGuard( _player, pc_i, _player->pos() );
		}
		else if( pc_i->objectType() == enNPC && pc_i->isInnocent() && pc_i->isHuman() )//&& pc_i->npcaitype() != 4 )
		{
			pc_i->talk( tr("Help! Guards! I've been attacked!") );
			_player->makeCriminal();
			pc_i->callGuards();
		}
		else if ((pc_i->objectType() == enNPC || pc_i->isTamed()) && !pc_i->isAtWar() )//&& pc_i->npcaitype() != 4) // changed from 0x40 to 4, cauz 0x40 was removed LB
		{
			P_NPC pn = dynamic_cast<P_NPC>(pc_i);
			pn->setNextMoveTime();
		}
		else if( pc_i->objectType() == enNPC )
		{
			dynamic_cast<P_NPC>(pc_i)->setNextMoveTime();
		}
	}
	else // not a guarded area
	{
		if( pc_i->isInnocent() )
		{
			if( pc_i->objectType() == enPlayer )
			{
				_player->makeCriminal();
			}
			else if( pc_i->objectType() == enNPC )
			{
				_player->makeCriminal();

				if( pc_i->combatTarget() == INVALID_SERIAL )
					pc_i->fight( _player );

				if( !pc_i->isTamed() && pc_i->isHuman() )
					pc_i->talk( tr( "Help! Guards! Tis a murder being commited!" ) );
			}
		}
	}

	// Send the "You see %1 attacking %2" string to all surrounding sockets
	// Except the one being attacked
	QString message = tr( "*You see %1 attacking %2*" ).arg(_player->name()).arg(pc_i->name());
	for( cUOSocket *s = Network::instance()->first(); s; s = Network::instance()->next() )
		if( s->player() && s != this && s->player()->inRange( _player, s->player()->visualRange() ) && s->player() != pc_i )
			s->showSpeech( _player, message, 0x26, 3, cUOTxUnicodeSpeech::Emote );

	// Send an extra message to the victim
	if( pc_i->objectType() == enPlayer )
	{
		P_PLAYER pp = dynamic_cast<P_PLAYER>(pc_i);
		if( pp->socket() )
			pp->socket()->showSpeech( _player, tr( "*You see %1 attacking you*" ).arg( _player->name() ), 0x26, 3, cUOTxUnicodeSpeech::Emote );
	}*/

	// I am already fighting this character.
/*	if( isAtWar() && combatTarget() == other->serial() )
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
		if (oldwar != isAtWar()) {
			update( true );
		}
	}*/
//}

bool cBaseChar::sysmessage(const QString &message, unsigned short color, unsigned short font)
{
	return false;
}

bool cBaseChar::sysmessage(unsigned int message, const QString &params, unsigned short color, unsigned short font)
{
	return false;
}

bool cBaseChar::message(const QString &message, unsigned short color, cUObject *source, unsigned short font, unsigned char mode)
{
	return false;
}

bool cBaseChar::send(cUOPacket *packet)
{
	return false;
}

void cBaseChar::poll(unsigned int time, unsigned int events)
{
	if (events & EventCombat)
	{
		if (attackTarget_ && nextSwing_ <= time)
		{
			P_CHAR target = attackTarget_;

			// Invulnerable or Dead target. Stop fighting.
			if (isDead() || target->isInvulnerable() || target->isDead()) {
				fight(0);
				return;
			}

			// Check weapon range
			unsigned char range = 1;
			P_ITEM weapon = getWeapon();

			if (weapon && weapon->hasTag("range"))
				range = weapon->getTag("range").toInt();

			// We are out of range
			if (pos().distance(target->pos()) > range)
				return;

			// Can we see our target?
			if (!canSee(attackTarget_))
				return;

			// Ranged weapons don't need a touch, all other weapons need it.
			if (weapon && (weapon->type() == 1006 || weapon->type() == 1007)) {
				if (!lineOfSight(attackTarget_, false)) {
					return;
				}
			} else if (!lineOfSight(attackTarget_, true)) {
				return;
			}

			cPythonScript *global = ScriptManager::instance()->getGlobalHook(EVENT_SWING);

			if (global)
			{
				PyObject *args = Py_BuildValue("O&O&i", PyGetCharObject, this, PyGetCharObject, attackTarget_, time);
				global->callEvent(EVENT_SWING, args);
				Py_DECREF(args);
			}
		}
	}
}

void cBaseChar::refreshMaximumValues()
{
	if (objectType() == enPlayer)
		maxHitpoints_ = QMAX(1, ((strength_) / 2) + hitpointsBonus_ + 50);

	maxStamina_ = (int)QMAX(1, dexterity_ + staminaBonus_);
	maxMana_ = (int)QMAX(1, intelligence_ + manaBonus_);
}

bool cBaseChar::lineOfSight(P_CHAR target, bool touch)
{
	if (target == this)
	{
		return true;
	}

	Coord_cl eyes = pos_ + Coord_cl( 0, 0, 15 );
	return eyes.lineOfSight(target->pos(), 15, touch);
}

bool cBaseChar::lineOfSight(P_ITEM target, bool touch)
{
	tile_st tile = TileCache::instance()->getTile( target->id() );
	Coord_cl eyes = pos_ + Coord_cl(0, 0, 15);

	return eyes.lineOfSight(target->pos(), tile.height, touch);
}

bool cBaseChar::lineOfSight(Coord_cl target, bool touch)
{
	Coord_cl eyes = pos_ + Coord_cl(0, 0, 15);

	return eyes.lineOfSight(target, 0, touch);
}

double cBaseChar::getHitpointRate() {
	// Do the math
	int points = 0;

	if (hasTag("regenhitpoints")) {
		points = QMAX(0, getTag("regenhitpoints").toInt());
	}

	return 1.0 / (0.1 * (1 + points));
}

double cBaseChar::getStaminaRate() {
	if (!isDead()) {
		double chance = (double)stamina() / maxStamina();
		double value = sqrt(skillValue(FOCUS) * 0.0005);
		chance *= (1.0 - value);
		chance += value;
		checkSkill(FOCUS, floor((1.0 - chance) * 1200), 1200);
	}

	int points = 0;

	if (hasTag("regenstamina")) {
		points = getTag("regenstamina").toInt();
	}

	points += (int)(skillValue(FOCUS) * 0.01);
	points = QMAX(-1, points);

	return 1.0 / (0.1 * (2 + points));
}

double cBaseChar::getManaRate() {
	if (!isDead()) {
		double chance = (double)mana() / maxMana();
		double value = sqrt(skillValue(FOCUS) * 0.0005);
		chance *= (1.0 - value);
		chance += value;
		checkSkill(FOCUS, floor((1.0 - chance) * 1200), 1200);
	}

	double medPoints = QMIN(13.0, (intelligence() + skillValue(MEDITATION) * 0.03) * (skillValue(MEDITATION) < 1000 ? 0.025 : 0.0275));
	double focusPoints = skillValue(FOCUS) * 0.005;

	// Wearing type 1009 items without the 'magearmor': 1 or 'spellchanneling': 1 flags
	// eliminates the meditation bonus
	for (unsigned char layer = SingleHandedWeapon; layer < Mount; ++layer) {
		P_ITEM item = atLayer((enLayer)layer);
		if (item && (item->type() == 1009 || item->type() == 1008)) {
			if (!item->allowMeditation() && !item->hasTag("magearmor")) {
				medPoints = 0;
				break;
			}
		}
	}

	int points = 2 + (int)(focusPoints + medPoints + (isMeditating() ? medPoints : 0.0));

	// Grant a bonus for the char if present
	if (hasTag("regenmana")) {
		points = getTag("regenmana").toInt();
	}

	return 1.0 / (0.1 * points);
}

// Light and Region checks
void cBaseChar::moveTo(const Coord_cl &pos, bool noremove) {
	cUObject::moveTo(pos, noremove);
	Territories::instance()->check(this);
}

void cBaseChar::remove() {
	// Cancel any ongoing fight.
	QPtrList<cFightInfo> fights(this->fights());
	fights.setAutoDelete(false);
	for (cFightInfo *info = fights.first(); info; info = fights.next()) {
		delete info;
	}

	// Call the onDelete event.
	PyObject *args = Py_BuildValue("(N)", getPyObject());
	cPythonScript::callChainedEventHandler(EVENT_DELETE, scriptChain, args);
	Py_DECREF(args);

	removeFromView(false);

	free = true;

	setGuarding(0);

	// We need to remove the equipment here.
	cBaseChar::ItemContainer container(content());
	cBaseChar::ItemContainer::const_iterator it (container.begin());
	cBaseChar::ItemContainer::const_iterator end(container.end());
	for (; it != end; ++it )
	{
		P_ITEM pItem = *it;
		if( !pItem )
			continue;

		pItem->remove();
	}

	if (multi_) {
		multi_->removeObject(this);
		multi_ = 0;
	}

	MapObjects::instance()->remove(this);
	cUObject::remove();
}
