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
#include "basechar.h"
#include "player.h"
#include "globals.h"
#include "world.h"
#include "persistentbroker.h"
#include "dbdriver.h"
#include "wpconsole.h"
#include "maps.h"
#include "chars.h"
#include "mapobjects.h"
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
#include "skills.h"
#include "wpdefmanager.h"
#include "srvparams.h"

cBaseChar::cBaseChar()
{
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
	nutriment_			= 0;
	flag_				= 0x02;
	emoteColor_			= 0x23;
	creationDate_		= QDateTime::currentDateTime();
	stealthedSteps_		= -1;
	runningSteps_		= 0;
	murdererTime_		= 0;
	criminalTime_		= 0;
	nextHitTime_		= 0;
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
	attackerSerial_		= INVALID_SERIAL;
    combatTarget_		= INVALID_SERIAL;
    swingTarget_		= INVALID_SERIAL;
    murdererSerial_		= INVALID_SERIAL;
    guarding_			= NULL;
	cUObject::pos_		= Coord_cl( 100, 100, 0, 0 );
	skills_.resize( ALLSKILLS );
	setDead(false);  // we want to live ;)
	regenHitpointsTime_	= uiCurrentTime + SrvParams->hitpointrate() * MY_CLOCKS_PER_SEC;
	regenStaminaTime_	= uiCurrentTime + SrvParams->staminarate() * MY_CLOCKS_PER_SEC;
	regenManaTime_		= uiCurrentTime + SrvParams->manarate() * MY_CLOCKS_PER_SEC;
	saycolor_			= 600;
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
	fields.push_back( "characters.murderertime,characters.criminaltime,characters.nutriment" );
	fields.push_back( "characters.stealthsteps,characters.gender,characters.propertyflags" );
	fields.push_back( "characters.attacker,characters.combattarget,characters.murderer" );
	fields.push_back( "characters.guarding" );
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
	nutriment_ = atoi( result[offset++] );
	stealthedSteps_ = atoi( result[offset++] );
	gender_ = atoi( result[offset++] );
	propertyFlags_ = atoi( result[offset++] );
	attackerSerial_ = atoi( result[offset++] );
	combatTarget_ = atoi( result[offset++] );
	murdererSerial_ = atoi( result[offset++] );	
	ser = atoi( result[offset++] );
	guarding_ = dynamic_cast<P_PLAYER>(FindCharBySerial( ser ));
	
	// Query the Skills for this character
	QString sql = "SELECT skills.skill,skills.value,skills.locktype,skills.cap FROM skills WHERE serial = '" + QString::number( serial() ) + "'";

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
		addField( "creationdate", creationDate_.toString() );
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
		addField( "nutriment", nutriment_);
		addField( "gender", gender_ );
		addField( "propertyflags", propertyFlags_ );
		addField( "attacker", attackerSerial_ );
		addField( "combattarget", combatTarget_ );
		addField( "murderer", murdererSerial_ );
		addField( "guarding", guarding_ ? guarding_->serial() : INVALID_SERIAL );
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
	changed_ = false;
}

bool cBaseChar::del()
{	
	if( !isPersistent )
		return false; // We didn't need to delete the object

	persistentBroker->addToDeleteQueue( "characters", QString( "serial = '%1'" ).arg( serial() ) );
	persistentBroker->addToDeleteQueue( "skills", QString( "serial = '%1'" ).arg( serial() ) );
	changed( SAVE );
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
				clConsole.send(QString("char/player: %1 : [%2] correted problematic skin hue\n").arg(pc->name()).arg( pc->serial(), 16 ) );
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
	if( Combat::weaponSkill( rightHand ) != WRESTLING )
		return rightHand;

	// Check for two-handed weapons
	P_ITEM leftHand = leftHandItem();
	if( Combat::weaponSkill( leftHand ) != WRESTLING )
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
	changed( SAVE );
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
	changed( SAVE );
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
	changed( SAVE );
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
	changed( SAVE );
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
	if( !orgBodyID_ )
		orgBodyID_ = bodyID_;

	if( orgBodyID_ == 0x0191 )
	{
		switch( RandomNum(0, 3) )
		{
		case 0:		soundEffect( 0x0150 );	break;// Female Death
		case 1:		soundEffect( 0x0151 );	break;// Female Death
		case 2:		soundEffect( 0x0152 );	break;// Female Death
		case 3:		soundEffect( 0x0153 );	break;// Female Death
		}
	}
	else if( orgBodyID_ == 0x0190 )
	{
		switch( RandomNum(0, 3) )
		{
		case 0:		soundEffect( 0x015A );	break;// Male Death
		case 1:		soundEffect( 0x015B );	break;// Male Death
		case 2:		soundEffect( 0x015C );	break;// Male Death
		case 3:		soundEffect( 0x015D );	break;// Male Death
		}
	}
	else
	{
		playmonstersound( this, orgBodyID_, SND_DIE );
	}
}

// This should check soon if we are standing above our 
// corpse and if so, merge with our corpse instead of
// just resurrecting
void cBaseChar::resurrect()
{
	if ( !isDead() )
		return;

	changed( SAVE|TOOLTIP );
	Fame( this, 0 );
	soundEffect( 0x0214 );
	setBodyID( orgBodyID_ );
	setSkin( orgSkin_ );
	setDead( false );
	hitpoints_ = QMAX( 1, (UINT16)( 0.1 * maxHitpoints_ ) );
	stamina_ = (UINT16)( 0.1 * maxStamina_ );
	mana_ = (UINT16)( 0.1 * maxMana_ );
	attackerSerial_ = INVALID_SERIAL;
	setAtWar( false );
	getBackpack(); // Make sure he has a backpack

	// Delete what the user wears on layer 0x16 (Should be death shroud)
	P_ITEM pRobe = GetItemOnLayer( 0x16 );

	if( pRobe )
		Items->DeleItem( pRobe );

	pRobe = Items->createScriptItem( "1f03" );

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

	if( nDir != direction() )
	{
		changed( SAVE );
		setDirection( nDir );
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
		backpack = new cItem;
		backpack->Init();
		backpack->setId( 0xE75 );
		backpack->setOwner( this );
		backpack->setType( 1 );		
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

void cBaseChar::removeItemBonus(cItem* pi)
{
	changed( SAVE|TOOLTIP );
	strength_ -= pi->strengthMod();
	dexterity_ -= pi->dexterityMod();
	intelligence_ -= pi->intelligenceMod();
}

void cBaseChar::giveItemBonus(cItem* pi)
{
	changed( SAVE|TOOLTIP );
	strength_ += pi->strengthMod();
	dexterity_ += pi->dexterityMod();
	intelligence_ += pi->intelligenceMod();
}

void cBaseChar::Init( bool createSerial )
{
	changed( SAVE|TOOLTIP );
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

UI16 cBaseChar::calcDefense( enBodyParts bodypart, bool wearout )
{
	P_ITEM pHitItem = NULL; 
	UI16 total = bodyArmor_; // the body armor is base value

	if( bodypart == ALLBODYPARTS )
	{
		P_ITEM pShield = leftHandItem();
		
		// Displayed AR = ((Parrying Skill * Base AR of Shield) ÷ 200) + 1
		if( pShield && IsShield( pShield->id() ) )
			total += ( (UI16)( (float)( skillValue( PARRYING ) * pShield->def() ) / 200.0f ) + 1 );
	} 	

	if( skillValue( PARRYING ) >= 1000 ) 
		total += 5; // gm parry bonus. 

	P_ITEM pi; 
	ItemContainer::const_iterator it = content_.begin();

	while( it != content_.end() )
	{
		pi = *it;
		if( pi && pi->layer() > 1 && pi->layer() < 25 ) 
		{ 
			//blackwinds new stuff 
			UI16 effdef = 0;
			if( pi->maxhp() > 0 ) 
				effdef = (UI16)( (float)pi->hp() / (float)pi->maxhp() * (float)pi->def() );

			if( bodypart == ALLBODYPARTS )
			{
				if( effdef > 0 )
				{
					total += effdef;
					if( wearout )
						pi->wearOut();
				}
			}
			else 
			{ 
				switch( pi->layer() ) 
				{ 
				case 5: 
				case 13: 
				case 17: 
				case 20: 
				case 22: 
					if( bodypart == BODY ) 
					{ 
						total += effdef; 
						pHitItem = pi; 
					} 
					break; 
				case 19: 
					if( bodypart == ARMS ) 
					{ 
						total += effdef; 
						pHitItem = pi; 
					} 
					break; 
				case 6: 
					if( bodypart == HEAD ) 
					{ 
						total += effdef; 
						pHitItem = pi; 
					} 
					break; 
				case 3: 
				case 4: 
				case 12: 
				case 23: 
				case 24: 
					if( bodypart == LEGS )
					{ 
						total += effdef; 
						pHitItem = pi; 
					} 
					break; 
				case 10: 
					if( bodypart == NECK ) 
					{ 
						total += effdef; 
						pHitItem = pi; 
					} 
					break; 
				case 7: 
					if( bodypart == HANDS )
					{ 
						total += effdef; 
						pHitItem = pi; 
					} 
					break; 
				default: 
					break; 
				} 
			}
		}
		++it;
	} 

	if( pHitItem ) 
	{ 
		// don't damage hairs, beard and backpack 
		// important! this sometimes cause backpack destroy! 
		if( pHitItem->layer() != 0x0B && pHitItem->layer() != 0x10 && pHitItem->layer() != 0x15 )
		{
			if( pHitItem->wearOut() )
				removeItemBonus( pHitItem ); // remove BONUS STATS given by equipped special items
		}
	}
	
	// Base AR ?
	/*if( total < 2 && bodypart == ALLBODYPARTS ) 
		total = 2;*/

	return total;
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
		max += 0.1;

	// +100 means: *allways* a minimum of 10% for success
	float chance = ( ( (float) charrange * 890.0f ) / (float)( max - min ) ) + 100.0f;
	
	if( chance > 990 ) 
		chance = 990;	// *allways* a 1% chance of failure
	
	if( chance >= RandomNum( 0, 1000 ) )
		success = true;
	
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

// Shows the name of a character to someone else
bool cBaseChar::onSingleClick( P_PLAYER Viewer ) 
{
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onSingleClick( (P_PLAYER)this, (P_CHAR)Viewer ) )
			return true;

	// Try to process the hooks then
	QValueVector< cPythonScript* > hooks;
	QValueVector< cPythonScript* >::const_iterator it;

	hooks = ScriptManager->getGlobalHooks( OBJECT_CHAR, EVENT_SINGLECLICK );
	for( it = hooks.begin(); it != hooks.end(); ++it )
		if( (*it)->onSingleClick( (P_PLAYER)this, (P_CHAR)Viewer ) )
			return true;

	return false;
}

// Walks in a specific Direction
bool cBaseChar::onWalk( UI08 Direction, UI08 Sequence )
{
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onWalk( (P_CHAR)this, Direction, Sequence ) )
			return true;

	return false;
}

// The character says something
bool cBaseChar::onTalk( char speechType, UI16 speechColor, UI16 speechFont, const QString &Text, const QString &Lang )
{
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onTalk( (P_CHAR)this, speechType, speechColor, speechFont, Text, Lang ) )
			return true;

	return false;
}

// The character switches warmode
bool cBaseChar::onWarModeToggle( bool War )
{
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onWarModeToggle( this, War ) )
			return true;

	return false;
}

// The paperdoll of this character has been requested
bool cBaseChar::onShowPaperdoll( P_CHAR pOrigin )
{
	for( UI08 i = 0; i < scriptChain.size(); i++ )
	{
		if( scriptChain[ i ]->onShowPaperdoll( this, pOrigin ) )
			return true;
	}

	return false;
}

bool cBaseChar::onShowSkillGump()
{
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onShowSkillGump( this ) )
			return true;
	
	// Try to process the hooks then
	QValueVector< cPythonScript* > hooks;
	QValueVector< cPythonScript* >::const_iterator it;

	hooks = ScriptManager->getGlobalHooks( OBJECT_CHAR, EVENT_SHOWSKILLGUMP );
	for( it = hooks.begin(); it != hooks.end(); ++it )
		if( (*it)->onShowSkillGump( this ) )
			return true;

	return false;	
}

// The character uses %Skill
bool cBaseChar::onSkillUse( UI08 Skill ) 
{
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onSkillUse( this, Skill ) )
			return true;

	// Try to process the hooks then
	QValueVector< cPythonScript* > hooks;
	QValueVector< cPythonScript* >::const_iterator it;

	hooks = ScriptManager->getGlobalHooks( OBJECT_CHAR, EVENT_SKILLUSE );
	for( it = hooks.begin(); it != hooks.end(); ++it )
		if( (*it)->onSkillUse( this, Skill ) )
			return true;

	return false;
}

bool cBaseChar::onCollideChar( P_CHAR Obstacle ) 
{
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onCollideChar( this, Obstacle ) )
			return true;

	return false;
}

bool cBaseChar::onDropOnChar( P_ITEM pItem )
{
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onDropOnChar( this, pItem ) )
			return true;

	return false;
}

QString cBaseChar::onShowPaperdollName( P_CHAR pOrigin )
{
	for( UI08 i = 0; i < scriptChain.size(); i++ )
	{
		QString result = scriptChain[ i ]->onShowPaperdollName( this, pOrigin );
		if( !result.isNull() )
			return result;
	}

	return (char*)0;
}

bool cBaseChar::onDeath()
{
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onDeath( this ) )
			return true;

	return false;
}

bool cBaseChar::onShowTooltip( P_PLAYER sender, cUOTxTooltipList* tooltip )
{
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onShowToolTip( sender, this, tooltip  ) )
			return true;

	// Try to process the hooks then
	QValueVector< cPythonScript* > hooks;
	QValueVector< cPythonScript* >::const_iterator it;

	hooks = ScriptManager->getGlobalHooks( OBJECT_CHAR, EVENT_SHOWTOOLTIP );
	for( it = hooks.begin(); it != hooks.end(); ++it )
		if( (*it)->onShowToolTip( sender, this, tooltip ) ) 
			return true;

	return false;
}

void cBaseChar::processNode( const cElement *Tag )
{
	changed( SAVE );
	QString TagName = Tag->name();
	QString Value = Tag->getValue();
	QDomNodeList ChildTags;

	// <bindmenu>contextmenu</bindmenu>
	// <bindmenu id="contextmenu />
	if( TagName == "bindmenu" )
	{
		if( !Tag->getAttribute( "id" ).isNull() ) 
			this->setBindmenu(Tag->getAttribute( "id" ));
		else
			setBindmenu(Value);
	}

	//<name>my this</name>
	if( TagName == "name" )
		this->setName(Value);
		
	//<backpack>
	//	<color>0x132</color>
	//	<item id="a">
	//	...
	//	<item id="z">
	//</backpack>
	else if( TagName == "backpack" )
	{
		if( !this->getBackpack() )
		{
			P_ITEM pBackpack = Items->SpawnItem( this, 1, "Backpack", 0, 0x0E75,0,0);
			if( pBackpack == NULL )
			{
				cCharStuff::DeleteChar( this );
				return;
			}
			
			pBackpack->setPos( Coord_cl(0, 0, 0) );
			this->addItem( Backpack, pBackpack );
			pBackpack->setType( 1 );
			pBackpack->setDye(1);

			if( Tag->childCount() )
				pBackpack->applyDefinition( Tag );
		}
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

	//<defense>10</defense>
	else if( TagName == "defense" )
		this->bodyArmor_ = Value.toUInt();

	//<emotecolor>0x482</emotecolor>
	else if( TagName == "emotecolor" )
		this->emoteColor_ = Value.toUShort();

	//<fame>8000</fame>
	else if( TagName == "fame" )
		this->fame_ = Value.toInt();

	//<food>3</food>
	else if( TagName == "food" )
	{
		UI16 bit = Value.toUShort();
		if( bit < 32 && bit > 0 )
			this->nutriment_ |= ( 1 << (bit-1) );
	}

	//<gold>100</gold>
	else if( TagName == "gold" )
	{
		giveGold( Value.toInt(), false );
	}

	//<id>0x11</id>
	else if( TagName == "id" )
	{
		bodyID_ = Value.toInt();
		orgBodyID_ = bodyID_;
	}

	//<karma>-500</karma>
	else if( TagName == "karma" )
		this->karma_ = Value.toInt();

	//<poison>2</poison>
	else if( TagName == "poison" )
		this->setPoison( Value.toInt() );

	//<skin>0x342</skin>
	else if( TagName == "skin" )
	{
		skin_ = Value.toUShort();
		orgSkin_ = Value.toUShort();
	}
		
	//<saycolor>0x110</saycolor>
	else if( TagName == "saycolor" )
		saycolor_ = Value.toUShort();

	//<title>the king</title>
	else if( TagName == "title" )
		this->setTitle( Value );

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
		std::vector< const cElement* > equipment;
		
		unsigned int i;
		for( i = 0; i < Tag->childCount(); ++i )
		{
			const cElement *childNode = Tag->getChild( i );

			if( childNode->name() == "item" )
				equipment.push_back( childNode );
			else if( childNode->name() == "getlist" && childNode->hasAttribute( "id" ) )
			{
				QStringList list = DefManager->getList( childNode->getAttribute( "id" ) );
				
				for( QStringList::iterator it = list.begin(); it != list.end(); it++ )
				{
					const cElement *listNode = DefManager->getDefinition( WPDT_ITEM, *it );
					
					if( listNode )
						equipment.push_back( listNode );
				}
			}
		}
		
		std::vector< const cElement* >::iterator iter = equipment.begin();
		
		while( iter != equipment.end() )
		{
			P_ITEM nItem = new cItem;
	
			if( nItem == NULL )
				continue;
	
			nItem->Init( true );

			nItem->applyDefinition( *iter );

			UINT8 mLayer = nItem->layer();
			nItem->setLayer( 0 );

			// Instead of deleting try to get a valid layer instead
			if( !mLayer )
			{
				tile_st tInfo = TileCache::instance()->getTile( nItem->id() );
				if( tInfo.layer > 0 )
					mLayer = tInfo.layer;
			}
				
			// Recheck
			if( !mLayer )
				Items->DeleItem( nItem );
			else
				this->addItem( static_cast<cBaseChar::enLayer>(mLayer), nItem ); // not sure about this one.

			++iter;
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
	{
		INT16 skillId = Skills->findSkillByDef( TagName );

		if( skillId == -1 )
			cUObject::processNode( Tag );
		else
			setSkillValue( skillId, Value.toInt() );
	}
	
//	cUObject::processNode( Tag );
}

void cBaseChar::addItem( cBaseChar::enLayer layer, cItem* pi, bool handleWeight, bool noRemove )
{
	// DoubleEquip is *NOT* allowed
	if ( atLayer( layer ) != 0 )
	{
		clConsole.send( tr("WARNING: Trying to put an item on layer %1 which is already occupied\n").arg(layer) );
		pi->setContainer(0);
		return;
	}

	if( !noRemove )
		pi->removeFromCont();

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
	changed( SAVE|TOOLTIP );
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
	else SET_INT_PROPERTY( "weight", weight_ )
	else if( name == "stones" )
	{
		weight_ = value.toInt() * 10;
		return 0;
	}
	else SET_INT_PROPERTY( "saycolor", saycolor_ )
	else SET_INT_PROPERTY( "emotecolor", emoteColor_ )
	else SET_INT_PROPERTY( "strength", strength_ )
	else SET_INT_PROPERTY( "dexterity", dexterity_ )
	else SET_INT_PROPERTY( "intelligence", intelligence_ )
	else SET_INT_PROPERTY( "strength2", strengthMod_ )
	else SET_INT_PROPERTY( "dexterity2", dexterityMod_ )
	else SET_INT_PROPERTY( "intelligence2", intelligenceMod_ )
	else SET_INT_PROPERTY( "xid", orgBodyID_ )
	else SET_INT_PROPERTY( "orgid", orgBodyID_ )
	else SET_INT_PROPERTY( "maxhitpoints", maxHitpoints_ )
	else SET_INT_PROPERTY( "hitpoints", hitpoints_ )
	else SET_INT_PROPERTY( "health", hitpoints_ )
	else SET_INT_PROPERTY( "maxstamina", maxStamina_ )
	else SET_INT_PROPERTY( "stamina", stamina_ )
	else SET_INT_PROPERTY( "maxmana", maxMana_ )
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
	else SET_INT_PROPERTY( "target", combatTarget_ )
	else SET_INT_PROPERTY( "nextswing", nextHitTime_ )
	else SET_INT_PROPERTY( "regenhealth", regenHitpointsTime_ )
	else SET_INT_PROPERTY( "regenstamina", regenStaminaTime_ )
	else SET_INT_PROPERTY( "regenmana", regenManaTime_ )
	else SET_INT_PROPERTY( "attacker", attackerSerial_ )
	else SET_INT_PROPERTY( "skilldelay", skillDelay_ )
	else SET_INT_PROPERTY( "nutriment", nutriment_ )
	else SET_INT_PROPERTY( "food", nutriment_ )
	else SET_INT_PROPERTY( "sex", gender_ )
	else SET_INT_PROPERTY( "gender", gender_ )
	else SET_INT_PROPERTY( "id", bodyID_ )
	else if( name == "attackfirst" )
	{
		setAttackFirst( value.toInt() );
		return 0;
	}
	else if( name == "invulnerable" )
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


	return cUObject::setProperty( name, value );
}

stError *cBaseChar::getProperty( const QString &name, cVariant &value ) const
{

	GET_PROPERTY( "orgname", orgName_ )
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
	else GET_PROPERTY( "stones", weight_ / 10 )
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
	else GET_PROPERTY( "target", FindCharBySerial( combatTarget_ ) )
	else GET_PROPERTY( "nextswing", (int)nextHitTime_ )
	else GET_PROPERTY( "regenhealth", (int)regenHitpointsTime_ )
	else GET_PROPERTY( "regenstamina", (int)regenStaminaTime_ )
	else GET_PROPERTY( "regenmana", (int)regenManaTime_ )
	else GET_PROPERTY( "attacker", FindCharBySerial( attackerSerial_ ) )
	else GET_PROPERTY( "region", ( region_ != 0 ) ? region_->name() : QString( "" ) )
	else GET_PROPERTY( "skilldelay", (int)skillDelay_ )
	else GET_PROPERTY( "nutriment", (int)nutriment_ )
	else GET_PROPERTY( "food", (int)nutriment_ )
	else GET_PROPERTY( "gender", gender_ )
	else GET_PROPERTY( "sex", gender_ )
	else GET_PROPERTY( "id", bodyID_ )
	else GET_PROPERTY( "attackfirst", attackFirst() )
	else GET_PROPERTY( "invulnerable", isInvulnerable() )
	else GET_PROPERTY( "invisible", isInvisible() )
	else GET_PROPERTY( "frozen", isFrozen() )
	
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

	changed( SAVE | TOOLTIP );
}

void cBaseChar::setSkillCap( UINT16 skill, UINT16 cap )
{
	skills_[ skill ].cap = cap;

	// Check if we can delete the current skill
//	const stSkillValue &skValue = skills_[ skill ];

//	if( skValue.cap == 1000 && skValue.lock == 0 && skValue.value == 0 )
//		skills_.remove( skill );

	changed( SAVE | TOOLTIP );
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

	changed( SAVE | TOOLTIP );
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
	changed( SAVE );
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
				Combat::spawnGuard( pc, pc, pc->pos() );
			}
		}
	}
}

bool cBaseChar::onSkillGain( UI08 Skill, SI32 min, SI32 max, bool success )
{
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onSkillGain( this, Skill, min, max, success ) )
			return true;

	// Try to process the hooks then
	QValueVector< cPythonScript* > hooks;
	QValueVector< cPythonScript* >::const_iterator it;

	hooks = ScriptManager->getGlobalHooks( OBJECT_CHAR, EVENT_SKILLGAIN );
	for( it = hooks.begin(); it != hooks.end(); ++it )
		if( (*it)->onSkillGain( this, Skill, min, max, success ) )
			return true;

	return false;
}

bool cBaseChar::onStatGain( UI08 stat, SI08 amount )
{
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onStatGain( this, stat, amount ) )
			return true;

	// Try to process the hooks then
	QValueVector< cPythonScript* > hooks;
	QValueVector< cPythonScript* >::const_iterator it;

	hooks = ScriptManager->getGlobalHooks( OBJECT_CHAR, EVENT_STATGAIN );
	for( it = hooks.begin(); it != hooks.end(); ++it )
		if( (*it)->onStatGain( this, stat,amount ) )
			return true;

	return false;
}

unsigned int cBaseChar::damage( eDamageType type, unsigned int amount, cUObject *source )
{
	//
	// First of all, call onDamage with the damage-type, amount and source
	// to modify the damage if needed
	//

	for( UINT8 i = 0; i < scriptChain.size(); ++i )
		amount = scriptChain[ i ]->onDamage( this, type, amount, source );

	QValueVector< cPythonScript* > hooks;
	QValueVector< cPythonScript* >::const_iterator it;

	hooks = ScriptManager->getGlobalHooks( OBJECT_CHAR, EVENT_DAMAGE );
	for( it = hooks.begin(); it != hooks.end(); ++it )
		amount = (*it)->onDamage( this, type, amount, source );

	// Invulnerable Targets don't take any damage at all
	if( isInvulnerable() )
		amount = 0;

	// The damage has been resisted or scripts have taken care of the damage otherwise
	if( !amount )
		return 0;

	// Would we die?
	if( amount >= hitpoints_ )
	{
		this->kill();
	}
	else
	{
		hitpoints_ -= amount;
		updateHealth();
		Combat::playGetHitSoundEffect( this );
		Combat::playGetHitAnimation( this );		
	}

	return amount;
}
