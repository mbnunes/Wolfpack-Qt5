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

// wolfpack includes
#include "accounts.h"
#include "basechar.h"
#include "basedef.h"
#include "corpse.h"
#include "npc.h"
#include "party.h"
#include "player.h"
#include "globals.h"
#include "world.h"
#include "persistentbroker.h"
#include "dbdriver.h"
#include "console.h"
#include "maps.h"
#include "inlines.h"
#include "chars.h"
#include "sectors.h"
#include "network/uosocket.h"
#include "network/uotxpackets.h"
#include "network.h"
#include "combat.h"
#include "items.h"
#include "itemid.h"
#include "basics.h"
#include "tilecache.h"
#include "pythonscript.h"
#include "scriptmanager.h"
#include "log.h"
#include "skills.h"
#include "wpdefmanager.h"
#include "srvparams.h"

cBaseChar::cBaseChar()
{
	lastMovement_		= 0;
	attackTarget_		= 0;
	nextSwing_			= 0;
	bodyID_				= 0x190;
	orgBodyID_			= 0x190;
	gender_				= 0;
	orgSkin_			= 0;
	propertyFlags_		= 0;
	weight_				= 0;
	bodyArmor_			= 2;
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
	stealthedSteps_		= -1;
	runningSteps_		= 0;
	murdererTime_		= 0;
	criminalTime_		= 0;
	skillDelay_			= 0;
	poison_				= 0;
	poisoned_			= 0;
	poisonTime_			= 0;
	poisonWearOffTime_	= 0;
    title_				= "";
	cUObject::name_		= "Man";
    orgName_			= "Man";
    skin_				= 0;
    region_				= NULL;
    saycolor_			= 0x1700;
    murdererSerial_		= INVALID_SERIAL;
    guarding_			= NULL;
	cUObject::pos_		= Coord_cl( 100, 100, 0, 0 );
	skills_.resize( ALLSKILLS );
	setDead(false);  // we want to live ;)
	regenHitpointsTime_	= uiCurrentTime + SrvParams->hitpointrate() * MY_CLOCKS_PER_SEC;
	regenStaminaTime_	= uiCurrentTime + SrvParams->staminarate() * MY_CLOCKS_PER_SEC;
	regenManaTime_		= uiCurrentTime + SrvParams->manarate() * MY_CLOCKS_PER_SEC;
	saycolor_			= 600;
	hitpointsBonus_		= 0;
	staminaBonus_		= 0;
	manaBonus_			= 0;
	strengthCap_		= 125;
	dexterityCap_		= 125;
	intelligenceCap_	= 125;
	statCap_			= SrvParams->statcap();
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
	fields.push_back( "characters.def,characters.hunger" );
	fields.push_back( "characters.poison,characters.poisoned" );
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
	bodyID_ = atoi( result[offset++] );
	orgBodyID_ = atoi( result[offset++] );
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
	bodyArmor_ = atoi( result[offset++] );
	hunger_ = atoi( result[offset++] );
	poison_ = atoi( result[offset++] );
	poisoned_ = atoi( result[offset++] );
	murdererTime_ = atoi( result[offset++] ) + uiCurrentTime;
	criminalTime_ = atoi( result[offset++] ) + uiCurrentTime;
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

	cDBResult res = persistentBroker->query( sql );
	if( !res.isValid() )
		throw persistentBroker->lastError();

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

		skills_[ skill ] = skValue;
	}

	res.free();

	characterRegisterAfterLoading( this );
	changed_ = false;
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
		addStrField( "creationdate", creationDate_.toString() );
		addField( "body", bodyID_ );
		addField( "orgbody", orgBodyID_ );
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
		addField( "def", bodyArmor_);
		addField( "hunger", hunger_);
		addField( "poison", poison_);
		addField( "poisoned", poisoned_);
		addField( "murderertime", murdererTime_ ? murdererTime_ - uiCurrentTime : 0);
		addField( "criminaltime", criminalTime_ ? criminalTime_ - uiCurrentTime : 0);
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

		QValueVector< stSkillValue >::const_iterator it;
		int i = 0;
		persistentBroker->lockTable("skills");
		for( it = skills_.begin(); it != skills_.end(); ++it )
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
	cUObject::save();
}

bool cBaseChar::del()
{
	if( !isPersistent )
		return false; // We didn't need to delete the object

	persistentBroker->addToDeleteQueue( "characters", QString( "serial = '%1'" ).arg( serial() ) );
	persistentBroker->addToDeleteQueue( "skills", QString( "serial = '%1'" ).arg( serial() ) );
	changed_ = true;
	return cUObject::del();
}

static void characterRegisterAfterLoading( P_CHAR pc )
{
	World::instance()->registerObject( pc );
	pc->setRegion( AllTerritories::instance()->region( pc->pos().x, pc->pos().y, pc->pos().map ) );

	if (pc->bodyID() <= 0x3e1)
	{
		unsigned short k = pc->bodyID();
		unsigned short c1 = pc->skin();
		unsigned short b = c1&0x4000;
		if ((b == 16384 && (k >=0x0190 && k<=0x03e1)) || c1==0x8000)
		{
			if (c1!=0xf000)
			{
				pc->setSkin( 0xF000 );
				pc->setOrgSkin( 0xF000 );
				Console::instance()->send(QString("char/player: %1 : [%2] correted problematic skin hue\n").arg(pc->name()).arg( pc->serial(), 16 ) );
			}
		}
	}

	UINT16 max_x = Map->mapTileWidth(pc->pos().map) * 8;
	UINT16 max_y = Map->mapTileHeight(pc->pos().map) * 8;

	// only > max_x and > max_y are invalid
	if( pc->pos().x >= max_x || pc->pos().y >= max_y )
	{
		cCharStuff::DeleteChar( pc );
		return;
	}
}

bool cBaseChar::isMurderer() const
{
	return murdererTime_ > uiCurrentTime;
}

bool cBaseChar::isCriminal() const
{
	return criminalTime_ > uiCurrentTime;
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

void cBaseChar::action( UINT8 id )
{
	bool mounted = ( atLayer( Mount ) != 0 );

	// Bow + Area Cast
	if( mounted && ( id == 0x11 || id == 0x12 ) )
		id = 0x1B;

	else if( mounted && ( id == 0x0D || id == 0x14 ) )
		id = 0x1D;

	// Attack (1H,Side,Down) + Cast Directed
	else if( mounted && ( id == 0x09 || id == 0x0a ||id == 0x0b ||id == 0x10 ) )
		id = 0x1A;

	// Bow + Salute + Eat
	else if( mounted && ( id == 0x13 || id == 0x20 || id == 0x21 || id == 0x22 ) )
		id = 0x1C;

	else if( ( mounted || bodyID_ < 0x190 ) && ( bodyID_ == 0x22 ) )
		return;

	cUOTxAction action;
	action.setAction( id );
	action.setSerial( serial() );
	action.setDirection( direction() );
	action.setRepeat( 1 );
	action.setRepeatFlag( 0 );
	action.setSpeed( 1 );

	for( cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next() )
	{
		if( socket->player() && socket->player()->inRange( this, socket->player()->visualRange() ) && ( !isHidden() || socket->player()->isGM() ) )
			socket->send( &action );
	}
}

P_ITEM cBaseChar::getWeapon() const
{
	// Check if we have something on our right hand
	P_ITEM rightHand = rightHandItem();
	if( Combat::instance()->weaponSkill( rightHand ) != WRESTLING )
		return rightHand;

	// Check for two-handed weapons
	P_ITEM leftHand = leftHandItem();
	if( Combat::instance()->weaponSkill( leftHand ) != WRESTLING )
		return leftHand;

	return NULL;
}

P_ITEM cBaseChar::getShield() const
{
	P_ITEM leftHand = leftHandItem();
	if( leftHand->isShield() )
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
	resend();
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
	resend();
}

void cBaseChar::setBeardColor( UINT16 d)
{
	changed_ = true;
	cItem* pBeard = GetItemOnLayer( 16 );
	if( pBeard )
		pBeard->setColor( d );
	pBeard->update();
	resend();
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
	resend();
}

void cBaseChar::playDeathSound()
{
	if( orgBodyID_ == 0x0191 )
	{
		switch( RandomNum(0, 3) )
		{
		case 0:		soundEffect( 0x0150 );	break;// Female Death
		case 1:		soundEffect( 0x0151 );	break;// Female Death
		case 2:		soundEffect( 0x0152 );	break;// Female Death
		case 3:		soundEffect( 0x0153 );	break;// Female Death
		}
	} else if( orgBodyID_ == 0x0190 ) {
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
void cBaseChar::resurrect()
{
	if ( !isDead() )
		return;

	changed( TOOLTIP );
	changed_ = true;
	awardFame( 0 );
	soundEffect( 0x0214 );
	setBodyID( orgBodyID_ );
	setSkin( orgSkin_ );
	setDead( false );
	hitpoints_ = QMAX( 1, (UINT16)( 0.1 * maxHitpoints_ ) );
	stamina_ = (UINT16)( 0.1 * maxStamina_ );
	mana_ = (UINT16)( 0.1 * maxMana_ );
	fight(0);
	getBackpack(); // Make sure he has a backpack

	// Delete what the user wears on layer 0x16 (Should be death shroud)
	P_ITEM pRobe = GetItemOnLayer( 0x16 );

	if( pRobe )
		pRobe->remove();

	pRobe = cItem::createFromScript( "1f03" );

	if( !pRobe )
		return;

	pRobe->setColor( 0 );
	pRobe->setHp( 1 );
	pRobe->setMaxhp( 1 );
	this->addItem( cBaseChar::OuterTorso, pRobe );
	pRobe->update();

	removeFromView( false );
	resend( false );
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
	for ( cUOSocket* socket = cNetwork::instance()->first(); socket != 0; socket = cNetwork::instance()->next() )
		if( socket->player() && socket->player()->inRange( this, socket->player()->visualRange() ) )
			socket->send( &packet );
}

void cBaseChar::unhide()
{
	//if hidden but not permanently
	if( isHidden() && !isInvisible() )
	{
		setStealthedSteps( -1 );
		setHidden( false );
		resend( false ); // They cant see us anyway
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

P_ITEM cBaseChar::getBackpack()
{
	P_ITEM backpack = atLayer( Backpack );

	// None found so create one
	if( !backpack )
	{
		backpack = cItem::createFromScript( "e75" );
		backpack->setOwner( this );
		addItem( Backpack, backpack );
		backpack->update();
	}

	return backpack;
}

void cBaseChar::SetMultiSerial(long mulser)
{
	this->setMultis( mulser );
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

void cBaseChar::MoveTo(short newx, short newy, signed char newz)
{
	// Avoid crash if go to 0,0
	if (newx < 1 || newy < 1)
		return;
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
	cTerritory* Region = AllTerritories::instance()->region( this->pos().x, this->pos().y, this->pos().map );
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
	textSpeech.setModel( bodyID_ );
	textSpeech.setFont( 3 ); // Default Font
	textSpeech.setType( cUOTxUnicodeSpeech::Emote );
	textSpeech.setName( name() );
	textSpeech.setColor( color );
	textSpeech.setText( emote );

	for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
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
	int charrange = skillValue( skill ) - min;

	if( charrange < 0 )
		charrange = 0;

	// To avoid division by zero
	if( min == max )
		max += 1;

	// +100 means: *allways* a minimum of 10% for success
	float chance = ( ( (float) charrange * 890.0f ) / (float)( max - min ) ) + 100.0f;

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

void cBaseChar::addEffect( cTempEffect *effect )
{
	effects_.push_back( effect );
}

void cBaseChar::removeEffect( cTempEffect *effect )
{
	EffectContainer::iterator iter = effects_.begin();
	while( iter != effects_.end() )
	{
		if( (*iter) == effect )
		{
			effects_.erase( iter );
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
			INT16 skillId = Skills->findSkillByDef( Tag->getAttribute( "type", "" ) );
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
		INT16 skillId = Skills->findSkillByDef( TagName );

		if( skillId == -1 )
			cUObject::processNode( Tag );
		else
			setSkillValue( skillId, Value.toInt() );
	}

}

void cBaseChar::addItem( cBaseChar::enLayer layer, cItem* pi, bool handleWeight, bool noRemove )
{
	// DoubleEquip is *NOT* allowed
	if ( atLayer( layer ) != 0 )
	{
		log( LOG_WARNING, QString( "Trying to put an item on layer %1 which is already occupied\n" ).arg( layer ) );
		pi->setContainer( 0 );
		return;
	}

	if( !noRemove )
	{
		// Dragging doesnt count as Equipping
		if( layer != Dragging )
			pi->onEquip( this, layer );
		pi->removeFromCont();
	}

	content_.insert( (ushort)(layer), pi );
	pi->setLayer( layer );
	pi->setContainer(this);

	if( handleWeight )
		weight_ += pi->totalweight();
}

void cBaseChar::removeItem( cBaseChar::enLayer layer, bool handleWeight )
{
	P_ITEM pi = atLayer(layer);
	if ( pi )
	{
		// Dragging doesnt count as Equipping
		if( layer != Dragging )
			pi->onUnequip( this, layer );

		pi->setContainer(0);
		pi->setLayer( 0 );
		content_.remove((ushort)(layer));

		if( handleWeight )
			weight_ -= pi->totalweight();
	}
}

// Simple setting and getting of properties for scripts and the set command.
stError *cBaseChar::setProperty( const QString &name, const cVariant &value )
{
	changed( TOOLTIP );
	changed_ = true;
	SET_STR_PROPERTY( "orgname", orgName_ )
	else SET_STR_PROPERTY( "title", title_ )
	else if( name == "incognito" )
	{
		setIncognito( value.toInt() );
		return 0;
	}
	else if( name == "polymorph" )
	{
		setPolymorphed( value.toInt() );
		return 0;
	}
	else if( name == "dead" )
	{
		setDead( value.toInt() );
		return 0;
	}
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
	else SET_INT_PROPERTY( "maxhitpoints", maxHitpoints_ )
	else SET_INT_PROPERTY( "maxstamina", maxStamina_ )
	else SET_INT_PROPERTY( "maxmana", maxMana_ )
	else SET_INT_PROPERTY( "lastmovement", lastMovement_ )
	else SET_INT_PROPERTY( "xskin", orgSkin_ )
	else SET_INT_PROPERTY( "orgskin", orgSkin_ )
	else if( name == "creationdate" )
	{
		creationDate_ = QDateTime::fromString( value.toString() );
		return 0;
	}
	else SET_INT_PROPERTY( "stealthedsteps", stealthedSteps_ )
	else SET_INT_PROPERTY( "runningsteps", runningSteps_ )
	else SET_INT_PROPERTY( "swingtarget", swingTarget_ )
	else if( name == "tamed" )
	{
		setTamed( value.toInt() );
		return 0;
	}
	else SET_CHAR_PROPERTY( "guarding", guarding_ )
	else SET_INT_PROPERTY( "murderer", murdererSerial_ )
	else if( name == "casting" )
	{
		setCasting( value.toInt() );
		return 0;
	}
	else if( name == "hidden" )
	{
		setHidden( value.toInt() );
		return 0;
	}
	else SET_INT_PROPERTY( "hunger", hunger_ )
	else SET_INT_PROPERTY( "hungertime", hungerTime_ )
	else SET_INT_PROPERTY( "poison", poison_ )
	else SET_INT_PROPERTY( "poisoned", poisoned_ )
	else SET_INT_PROPERTY( "poisontime", poisonTime_ )
	else SET_INT_PROPERTY( "poisonwearofftime", poisonWearOffTime_ )
	else if( name == "ra" )
	{
		setReactiveArmor( value.toInt() );
		return 0;
	}
	else SET_INT_PROPERTY( "flag", flag_ )
	else SET_INT_PROPERTY( "murderertime", murdererTime_ )
	else SET_INT_PROPERTY( "criminaltime", criminalTime_ )
	else if( name == "meditating" )
	{
		setMeditating( value.toInt() );
		return 0;
	}
	else SET_FLOAT_PROPERTY( "weight", weight_ )
	else SET_INT_PROPERTY( "saycolor", saycolor_ )
	else SET_INT_PROPERTY( "emotecolor", emoteColor_ )
	else if (name == "strength") {
		setStrength(value.toInt());
		return 0;
	} else if (name == "dexterity") {
		setDexterity(value.toInt());
		return 0;
	} else if (name == "intelligence") {
		setIntelligence(value.toInt());
		return 0;
	} else if (name == "strength2") {
		setStrengthMod(value.toInt());
		return 0;
	} else if (name == "dexterity2") {
		setDexterityMod(value.toInt());
		return 0;
	} else if (name == "intelligence2") {
		setIntelligenceMod(value.toInt());
		return 0;
	} else SET_INT_PROPERTY( "xid", orgBodyID_ )
	else SET_INT_PROPERTY( "orgid", orgBodyID_ )
	else SET_INT_PROPERTY( "hitpoints", hitpoints_ )
	else SET_INT_PROPERTY( "health", hitpoints_ )
	else SET_INT_PROPERTY( "stamina", stamina_ )
	else SET_INT_PROPERTY( "mana", mana_ )
	else SET_INT_PROPERTY( "karma", karma_ )
	else SET_INT_PROPERTY( "fame", fame_ )
	else SET_INT_PROPERTY( "kills", kills_ )
	else SET_INT_PROPERTY( "deaths", deaths_ )
	else SET_INT_PROPERTY( "defense", bodyArmor_ )
	else if( name == "war" )
	{
		setAtWar( value.toInt() );
		return 0;
	}
	else SET_CHAR_PROPERTY( "attacktarget", attackTarget_ )
	else SET_INT_PROPERTY( "nextswing", nextSwing_ )
	else SET_INT_PROPERTY( "regenhealth", regenHitpointsTime_ )
	else SET_INT_PROPERTY( "regenstamina", regenStaminaTime_ )
	else SET_INT_PROPERTY( "regenmana", regenManaTime_ )
	else SET_INT_PROPERTY( "skilldelay", skillDelay_ )
	else SET_INT_PROPERTY( "sex", gender_ )
	else SET_INT_PROPERTY( "gender", gender_ )
	else SET_INT_PROPERTY( "id", bodyID_ )
	else if (name == "hitpointsbonus") {
		setHitpointsBonus(value.toInt());
		return 0;
	} else if (name == "staminabonus") {
		setStaminaBonus(value.toInt());
		return 0;
	} else if (name == "manabonus") {
		setManaBonus(value.toInt());
		return 0;
	} else if( name == "invulnerable" )
	{
		setInvulnerable( value.toInt() );
		return 0;
	}
	else if( name == "invisible" )
	{
		setInvisible( value.toInt() );
		return 0;
	}
	else if( name == "frozen" )
	{
		setFrozen( value.toInt() );
		return 0;
	}
	else SET_INT_PROPERTY( "strengthcap", strengthCap_ )
	else SET_INT_PROPERTY( "dexteritycap", dexterityCap_ )
	else SET_INT_PROPERTY( "intelligencecap", intelligenceCap_ )
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
	else GET_PROPERTY( "xskin", orgSkin_ )
	else GET_PROPERTY( "creationdate", creationDate_.toString() )
	else GET_PROPERTY( "stealthedsteps", stealthedSteps_ )
	else GET_PROPERTY( "runningsteps", (int)runningSteps_ )
	else GET_PROPERTY( "swingtarget", FindCharBySerial( swingTarget_ ) )
	else GET_PROPERTY( "tamed", isTamed() )
	else GET_PROPERTY( "guarding", guarding_ )
	else GET_PROPERTY( "murderer", FindCharBySerial( murdererSerial_ ) )
	else GET_PROPERTY( "casting", isCasting() )
	else GET_PROPERTY( "hidden", isHidden() )
	else GET_PROPERTY( "hunger", hunger_ )
	else GET_PROPERTY( "hungertime", (int)hungerTime_ )
	else GET_PROPERTY( "poison", poison_ )
	else GET_PROPERTY( "poisoned", (int)poisoned_ )
	else GET_PROPERTY( "poisontime", (int)poisonTime_ )
	else GET_PROPERTY( "poisonwearofftime", (int)poisonWearOffTime_ )
	else GET_PROPERTY( "ra", hasReactiveArmor() )
	else GET_PROPERTY( "flag", flag_ )
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
	else GET_PROPERTY( "orgid", orgBodyID_ )
	else GET_PROPERTY( "xid", orgBodyID_ )
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
	else GET_PROPERTY( "defense", (int)bodyArmor_ )
	else GET_PROPERTY( "war", isAtWar() )
	else GET_PROPERTY( "attacktarget", attackTarget_ )
	else GET_PROPERTY( "nextswing", (int)nextSwing_ )
	else GET_PROPERTY( "regenhealth", (int)regenHitpointsTime_ )
	else GET_PROPERTY( "regenstamina", (int)regenStaminaTime_ )
	else GET_PROPERTY( "regenmana", (int)regenManaTime_ )
	else GET_PROPERTY( "region", ( region_ != 0 ) ? region_->name() : QString( "" ) )
	else GET_PROPERTY( "skilldelay", (int)skillDelay_ )
	else GET_PROPERTY( "gender", gender_ )
	else GET_PROPERTY( "sex", gender_ )
	else GET_PROPERTY( "id", bodyID_ )
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
		INT16 skillId = Skills->findSkillByDef( skill );

		if( skillId != -1 )
		{
			value = cVariant( this->skillValue( skillId ) );
			return 0;
		}
	}


	return cUObject::getProperty( name, value );
}

void cBaseChar::setSkillValue( UINT16 skill, UINT16 value )
{
	skills_[ skill ].value = value;

	// Check if we can delete the current skill
//	const stSkillValue &skValue = skills_[ skill ];

//	if( skValue.cap == 1000 && skValue.lock == 0 && skValue.value == 0 )
//		skills_.remove( skill );

	changed( TOOLTIP );
	changed_ = true;
}

void cBaseChar::setSkillCap( UINT16 skill, UINT16 cap )
{
	skills_[ skill ].cap = cap;

	// Check if we can delete the current skill
//	const stSkillValue &skValue = skills_[ skill ];

//	if( skValue.cap == 1000 && skValue.lock == 0 && skValue.value == 0 )
//		skills_.remove( skill );

	changed( TOOLTIP );
	changed_ = true;
}

void cBaseChar::setSkillLock( UINT16 skill, UINT8 lock )
{
	if( lock > 2 )
		lock = 0;

	skills_[ skill ].lock = lock;

	// Check if we can delete the current skill
//	const stSkillValue &skValue = skills_[ skill ];

//	if( skValue.cap == 1000 && skValue.lock == 0 && skValue.value == 0 )
//		skills_.remove( skill );

	changed( TOOLTIP );
	changed_ = true;
}

UINT16 cBaseChar::skillValue( UINT16 skill ) const
{
	return skills_[ skill ].value;

//	if( skValue == skills_.end() )
//		return 0;
}

UINT16 cBaseChar::skillCap( UINT16 skill ) const
{
	return skills_[ skill ].cap;

//	if( skValue == skills_.end() )
//		return 1000;
}

UINT8 cBaseChar::skillLock( UINT16 skill ) const
{
	return skills_[ skill ].lock;

//	if( skValue == skills_.end() )
//		return 0;
}

void cBaseChar::setStamina(INT16 data, bool notify /* = true */ )
{
    stamina_ = data;
	changed_ = true;
}

void cBaseChar::callGuards()
{
	if (!inGuardedArea() || !SrvParams->guardsActive() )
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
	
	// There is a 33% chance that blood is created on hit
	if (!RandomNum(0,2)) {
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
			blood->setDecayTime(uiCurrentTime + 20 * MY_CLOCKS_PER_SEC); // Let it decay in 20 seconds from now
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
	if( bodyID() == 0x190 || bodyID() == 0x192 )
	{
		if( type == Bark_GetHit )
		{
			unsigned short sound = hex2dec( DefManager->getRandomListEntry( "SOUNDS_COMBAT_HIT_HUMAN_MALE" ) ).toUShort();

			if( sound > 0 )
				soundEffect( sound );
			else
				soundEffect( 0x156 );

			return;
		}
	}
	else if( bodyID() == 0x191 || bodyID() == 0x193 )
	{
		unsigned short sound = hex2dec( DefManager->getRandomListEntry( "SOUNDS_COMBAT_HIT_HUMAN_FEMALE" ) ).toUShort();
		if( sound > 0 )
			soundEffect( sound );
		else
			soundEffect( 0x14b );
	}

	cCharBaseDef *basedef = BaseDefManager::instance()->getCharBaseDef( bodyID_ );

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
	switch( bodyID_ )
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
	return cPythonScript::callChainedEventHandler( EVENT_SHOWSKILLGUMP, scriptChain );
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

bool cBaseChar::onStatGain( unsigned char stat )
{
	cPythonScript *global = ScriptManager::instance()->getGlobalHook( EVENT_STATGAIN );
	bool result = false;

	if( scriptChain || global )
	{
		PyObject *args = Py_BuildValue( "O&b", PyGetCharObject, this, stat );

		result = cPythonScript::callChainedEventHandler( EVENT_STATGAIN, scriptChain, args );

		if( !result && global )
			result = global->callEventHandler( EVENT_STATGAIN, args );

		Py_DECREF( args );
	}

	return result;
}

bool cBaseChar::kill(cUObject *source) 
{
	
	if (free || isDead()) 
		return false;


	changed(TOOLTIP);
	changed_ = true;
	setDead(true);
	hitpoints_ = 0;
	setPoisoned(0);
	setPoison(0);

	if (isPolymorphed()) 
	{
		setBodyID(orgBodyID_);
		setSkin(orgSkin_);
		setPolymorphed(false);
	}

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
				if (pPlayer->kills() >= SrvParams->maxkills()) 
				{
					pPlayer->setMurdererTime(getNormalizedTime() + SrvParams->murderdecay() * MY_CLOCKS_PER_SEC);
	
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
	
	cCharBaseDef *basedef = BaseDefManager::instance()->getCharBaseDef(bodyID_);

	// If we are a creature type with a corpse and if we are not summoned
	// we create a corpse
	if (!summoned && basedef && !basedef->noCorpse()) 
	{
		corpse = new cCorpse(true);
		
		const cElement *elem = DefManager->getDefinition(WPDT_ITEM, "2006");
		if (elem) 
			corpse->applyDefinition(elem);

		corpse->setName(name_);
		corpse->setColor(skin_);
		corpse->setBodyId(bodyID_);
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
			corpse->setMurderTime(uiCurrentTime);
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
        
		corpse->setDecayTime(uiCurrentTime + SrvParams->corpseDecayTime() * MY_CLOCKS_PER_SEC);		
		corpse->update();
	}

	// Create Loot - Either on the corpse or on the ground
	QPtrList<cItem> posessions = backpack->getContainment();

	for (P_ITEM item = posessions.first(); item; item = posessions.next()) 
	{
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
		QStringList lootlist = DefManager->getList(npc->lootList());

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
		cCharStuff::DeleteChar(this);
		return true;
	}

	playDeathSound();

	cUOTxDeathAction dAction;
	dAction.setSerial(serial_);

	if (corpse) 
		dAction.setCorpse(corpse->serial());

	cUOTxRemoveObject rObject;
	rObject.setSerial(serial_);
	
	for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() ) 
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

	if (npc)
		cCharStuff::DeleteChar(this);

	if (player) 
	{
		// Create a death shroud for the player
		P_ITEM shroud = cItem::createFromScript("204e");
		if (shroud) 
		{
			addItem(OuterTorso, shroud);
			shroud->update();
		}

		player->resend(false, true);

		if (player->socket()) 
		{
			player->socket()->resendPlayer(true);
			
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

bool cBaseChar::canSee(cUObject *object, bool lineOfSight) 
{
	P_ITEM item = dynamic_cast<P_ITEM>(object);

	if (item) 
		return canSeeItem(item, lineOfSight);

	P_CHAR character = dynamic_cast<P_CHAR>(object);

	if (character) 
		return canSeeChar(character, lineOfSight);

	return false;
}

bool cBaseChar::canSeeChar(P_CHAR character, bool lineOfSight) 
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
	}

	return true;
}

bool cBaseChar::canSeeItem(P_ITEM item, bool lineOfSight) 
{
	if (!item) 
		return false;

	if (item->visible() == 2) 
		return false;
	else if (item->visible() == 1 && item->owner() != this) 
		return false;

	// Check for container
	if (item->container()) 
		return canSee(item->container(), lineOfSight);
	else 
	{
		if (pos_.distance(item->pos()) > VISRANGE) 
			return false;

		if (lineOfSight && !pos_.lineOfSight(item->pos())) 
			return false;
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

	if (!inWorld()) 
		return result;

	// Ghosts can't fight
	if (isDead()) 
	{
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
		if (attackTarget_) 
		{
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
	if (!isAtWar()) 
	{
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
		if( pPet->combatTarget() == INVALID_SERIAL && pPet->inRange( _player, SrvParams->attack_distance() ) ) // is it on screen?
		{
			pPet->fight( pc_i );

			// Show the You see XXX attacking YYY messages
			QString message = tr( "*You see %1 attacking %2*" ).arg( pPet->name() ).arg( pc_i->name() );
			for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
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

	if( pc_i->inGuardedArea() && SrvParams->guardsActive() )
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
	for( cUOSocket *s = cNetwork::instance()->first(); s; s = cNetwork::instance()->next() )
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
			if (!canSee(attackTarget_, true))
				return;
	
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
		maxHitpoints_ = QMAX(1, ((strength_ - strengthMod_) / 2) + strengthMod_ + hitpointsBonus_ + 50);

	maxStamina_ = QMAX(1, dexterity_ + dexterityMod_ + staminaBonus_);
	maxMana_ = QMAX(1, intelligence_ + intelligenceMod_ + manaBonus_);
}
