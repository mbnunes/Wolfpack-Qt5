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

cBaseChar::cBaseChar()
{
	bodyID_				= 0x190;
	orgBodyID_			= 0x190;
	gender_				= 0;
	orgSkin_			= 0;
	propertyFlags_		= 0;
	weight_				= 0;
	bodyArmor_			= 2;
	direction_			= 0;
	dexterity_			= 50;
	dexterityMod_		= 0;
	dexEff_				= 0;
	maxStamina_			= dexterity_;
	stamina_			= maxStamina_;
	strength_			= 50;
	strengthMod_		= 0;
	maxHitpoints_		= strength_;
	hitpoints_			= maxHitpoints_;
	intelligence_		= 50;
	intelligenceMod_	= 0;
	maxMana_			= intelligence_;
	mana_				= maxMana_;
	karma_				= 0;
	fame_				= 0;
	kills_				= 0;
	deaths_				= 0;
	hunger_				= 0;
	hungerTime_			= 0;
	nutriment_			= 0;
	flag_				= 0x02;
	emoteColor_			= 0x23;
	creationDate_		= QDateTime::currentDateTime();
	stealthedSteps_		= -1;
	runningSteps_		= 0;
	murdererTime_		= 0;
	criminalTime_		= -1;
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
	cUObject::pos_		= Coord_cl( 100, 100, 0, 0 );
	skills_.resize( ALLSKILLS );
}

cBaseChar::cBaseChar(const cBaseChar& right)
{
}

cBaseChar::~cBaseChar()
{
}

cBaseChar& cBaseChar::operator=(const cBaseChar& right)
{
}

void cBaseChar::buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions )
{
	cUObject::buildSqlString( fields, tables, conditions );
	fields.push_back( "characters.name,characters.title,characters.creationdate" );
	fields.push_back( "characters.dir,characters.body,characters.orgbody,characters.skin" );
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

	orgName_ = result[offset++];
	title_ = result[offset++];
	creationDate_ = QDateTime::fromString( result[offset++] );
	direction_ = atoi( result[offset++] );
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
	murdererTime_ = atoi( result[offset++] );
	criminalTime_ = atoi( result[offset++] );
	nutriment_ = atoi( result[offset++] );
	stealthedSteps_ = atoi( result[offset++] );
	gender_ = atoi( result[offset++] );
	propertyFlags_ = atoi( result[offset++] );
	attackerSerial_ = atoi( result[offset++] );
	combatTarget_ = atoi( result[offset++] );
	murdererSerial_ = atoi( result[offset++] );	
	
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
		addField( "dir", direction_ );
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
		addField( "murderertime", murdererTime_);
		addField( "criminaltime", criminalTime_);
		addField( "nutriment", nutriment_);
		addField( "gender", gender_ );
		addField( "propertyflags", propertyFlags_ );
		addField( "attacker", attackerSerial_ );
		addField( "combattarget", combatTarget_ );
		addField( "murderer", murdererSerial_ );
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
				clConsole.send("char/player: %s : %i correted problematic skin hue\n", pc->name().latin1(),pc->serial());
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

	else if( ( mounted || this->id() < 0x190 ) && ( id == 0x22 ) )
		return;

	cUOTxAction action;
	action.setAction( id );
	action.setSerial( serial() );
	action.setDirection( dir_ );
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
	if( isShield( leftHand ) )
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
		addItem( cChar::Hair, pHair );
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
		addItem( cChar::FacialHair, pBeard );
	}
	pBeard->update();
	resend();
}

void cBaseChar::playDeathSound()
{
	if( !xid_ )
		xid_ = id_;

	if( xid_ == 0x0191 )
	{
		switch( RandomNum(0, 3) )
		{
		case 0:		soundEffect( 0x0150 );	break;// Female Death
		case 1:		soundEffect( 0x0151 );	break;// Female Death
		case 2:		soundEffect( 0x0152 );	break;// Female Death
		case 3:		soundEffect( 0x0153 );	break;// Female Death
		}
	}
	else if( xid_ == 0x0190 )
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
		playmonstersound( this, xid_, SND_DIE );
	}
}

// This should check soon if we are standing above our 
// corpse and if so, merge with our corpse instead of
// just resurrecting
void cBaseChar::resurrect()
{
	if ( !dead_ )
		return;

	changed( SAVE|TOOLTIP );
	Fame( this, 0 );
	soundEffect( 0x0214 );
	setId( xid_ );
	setSkin( xskin() );
	dead_ = false;
	hp_ = QMAX( 1, (UINT16)( 0.1 * st_ ) );
	stm_ = (UINT16)( 0.1 * effDex() );
	mn_ = (UINT16)( 0.1 * in_ );
	attacker_ = INVALID_SERIAL;
	resetAttackFirst();
	war_ = false;

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
	this->addItem( cChar::OuterTorso, pRobe );
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

	if( nDir != dir_ )
	{
		changed( SAVE );
		dir_ = nDir;
		
		update( true );
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

	this->addItem( static_cast<cChar::enLayer>(layer), pi );
	cUOTxCharEquipment packet;
	packet.setWearer( this->serial() );
	packet.setSerial( pi->serial() );
	packet.fromItem( pi );
	for ( cUOSocket* socket = cNetwork::instance()->first(); socket != 0; socket = cNetwork::instance()->next() )
		if( socket->player() && socket->player()->inRange( this, socket->player()->VisRange() ) ) 
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
	ContainerContent container = this->content();
	ContainerContent::const_iterator it  = container.begin();
	ContainerContent::const_iterator end = container.end();

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

void cBaseChar::MoveToXY(short newx, short newy)
{
	this->MoveTo(newx,newy,pos().z);	// keep the old z value
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

unsigned int cBaseChar::getSkillSum()
{
	unsigned int sum = 0;

	QValueVector< stSkillValue >::const_iterator it = skills.begin();
	for( ; it != skills.end(); ++it )
		sum += (*it).value;

	return sum;		// this *includes* the decimal digit ie. xxx.y
}

void cBaseChar::removeItemBonus(cItem* pi)
{
	changed( SAVE|TOOLTIP );
	strength_ -= pi->st2();
	dexterity_ -= pi->dx2();
	intelligence_ -= pi->in2();
}

void cBaseChar::giveItemBonus(cItem* pi)
{
	changed( SAVE|TOOLTIP );
	strength_ += pi->st2();
	dexterity_ += pi->dx2();
	intelligence_ += pi->in2();
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
		color = emotecolor_;

	cUOTxUnicodeSpeech textSpeech;
	textSpeech.setSource( serial() );
	textSpeech.setModel( id() );
	textSpeech.setFont( 3 ); // Default Font
	textSpeech.setType( cUOTxUnicodeSpeech::Emote );
	textSpeech.setName( name() );
	textSpeech.setColor( color );
	textSpeech.setText( emote );
	
	for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
		if( mSock->player() && mSock->player()->inRange( this, mSock->player()->VisRange() ) )
			mSock->send( &textSpeech );
}

UI16 cBaseChar::calcDefense( enBodyParts bodypart, bool wearout )
{
	P_ITEM pHitItem = NULL; 
	UI16 total = def(); // the body armor is base value

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
	ContainerContent::const_iterator it = content_.begin();

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
	ContainerContent::const_iterator it = content_.find(layer);
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

void cBaseChar::addGuard( P_NPC pPet, bool noGuardingChange )
{
	// Check if already existing in the guard list
	for( Followers::iterator iter = guardedby_.begin(); iter != guardedby_.end(); ++iter )
		if( *iter == pPet )
			return;

	if( !noGuardingChange )
	{
		if( pPet->guarding() )
			pPet->guarding()->removeGuard( pPet );

		pPet->setGuardingOnly( this );
	}

	guardedby_.push_back( pPet );
}

void cBaseChar::removeGuard( P_NPC pPet, bool noGuardingChange )
{
	for( Followers::iterator iter = guardedby_.begin(); iter != guardedby_.end(); ++iter )
		if( *iter == pPet )
		{
			guardedby_.erase( iter );
			break;
		}

	if( !noGuardingChange )
		pPet->setGuardingOnly( 0 );
}

void cBaseChar::addEffect( cTempEffect *effect )
{
	effects_.push_back( effect );
}

void cBaseChar::removeEffect( cTempEffect *effect )
{
	Effects::iterator iter = effects_.begin();
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
bool cBaseChar::onSingleClick( P_CHAR Viewer ) 
{
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onSingleClick( (P_CHAR)this, (P_CHAR)Viewer ) )
			return true;

	// Try to process the hooks then
	QValueVector< WPDefaultScript* > hooks;
	QValueVector< WPDefaultScript* >::const_iterator it;

	hooks = ScriptManager->getGlobalHooks( OBJECT_CHAR, EVENT_SINGLECLICK );
	for( it = hooks.begin(); it != hooks.end(); ++it )
		if( (*it)->onSingleClick( (P_CHAR)this, (P_CHAR)Viewer ) )
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

// The character uses %Skill
bool cBaseChar::onSkillUse( UI08 Skill ) 
{
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onSkillUse( this, Skill ) )
			return true;

	// Try to process the hooks then
	QValueVector< WPDefaultScript* > hooks;
	QValueVector< WPDefaultScript* >::const_iterator it;

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

QString cChar::onShowPaperdollName( P_CHAR pOrigin )
{
	for( UI08 i = 0; i < scriptChain.size(); i++ )
	{
		QString result = scriptChain[ i ]->onShowPaperdollName( this, pOrigin );
		if( !result.isNull() )
			return result;
	}

	return (char*)0;
}

void cBaseChar::processNode( const QDomElement &Tag )
{
	changed( SAVE );
	QString TagName = Tag.nodeName();
	QString Value = this->getNodeValue( Tag );
	QDomNodeList ChildTags;

	// <bindmenu>contextmenu</bindmenu>
	// <bindmenu id="contextmenu />
	if( TagName == "bindmenu" )
	{
		if( !Tag.attribute( "id" ).isNull() ) 
			this->setBindmenu(Tag.attribute( "id" ));
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

			if( Tag.hasChildNodes() )
				pBackpack->applyDefinition( Tag );
		}
	}

	//<direction>SE</direction>
	else if( TagName == "direction" )
	{
		if( Value == "NE" )
			this->dir_ = 1;
		else if( Value == "E" )
			this->dir_ = 2;
		else if( Value == "SE" )
			this->dir_ = 3;
		else if( Value == "S" )
			this->dir_ = 4;
		else if( Value == "SW" )
			this->dir_ = 5;
		else if( Value == "W" )
			this->dir_ = 6;
		else if( Value == "NW" )
			this->dir_ = 7;
		else if( Value == "N" )
			this->dir_ = 0;
		else
			this->dir_ = Value.toUShort();
	}

	//<stat type="str">100</stats>
	else if( TagName == "stat" )
	{
		if( Tag.attributes().contains( "type" ) )
		{
			QString statType = Tag.attribute( "type" );
			if( statType == "str" )
			{
				this->st_ = Value.toLong();
				this->hp_ = this->st_;
			}
			else if( statType == "dex" )
			{
				this->setDex( Value.toLong() );
				this->stm_ = this->realDex();
			}
			else if( statType == "int" )
			{
				this->in_ = Value.toLong();
				this->mn_ = this->in_;
			}
		}
	}

	// Aliasses <str <dex <int
	else if( TagName == "str" )
	{
		st_ = Value.toLong();
		hp_ = st_;
	}

	else if( TagName == "dex" )
	{
		setDex( Value.toLong() );
		stm_ = realDex();
	}
	
	else if( TagName == "int" )
	{
		in_ = Value.toLong();
		mn_ = in_;
	}

	//<defense>10</defense>
	else if( TagName == "defense" )
		this->def_ = Value.toUInt();

	//<emotecolor>0x482</emotecolor>
	else if( TagName == "emotecolor" )
		this->emotecolor_ = Value.toUShort();

	//<fame>8000</fame>
	else if( TagName == "fame" )
		this->fame_ = Value.toInt();

	//<food>3</food>
	else if( TagName == "food" )
	{
		UI16 bit = Value.toUShort();
		if( bit < 32 && bit > 0 )
			this->food_ |= ( 1 << (bit-1) );
	}

	//<gold>100</gold>
	else if( TagName == "gold" )
	{
		giveGold( Value.toInt(), false );
	}

	//<id>0x11</id>
	else if( TagName == "id" )
	{
		this->setId( Value.toInt() );
		this->xid_ = this->id();
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
		this->setSkin( Value.toUShort() );
		this->setXSkin( Value.toUShort() );
	}
		
	//<saycolor>0x110</saycolor>
	else if( TagName == "saycolor" )
		this->setSayColor( Value.toUShort() );

	//<title>the king</title>
	else if( TagName == "title" )
		this->setTitle( Value );

	//<skill type="alchemy">100</skill>
	//<skill type="1">100</skill>
	else if( TagName == "skill" && Tag.attributes().contains("type") )
	{
		if( Tag.attribute("type").toInt() > 0 &&
			Tag.attribute("type").toInt() <= ALLSKILLS )
			setSkillValue( ( Tag.attribute( "type" ).toInt() - 1 ), Value.toInt() );
		else
		{
			INT16 skillId = Skills->findSkillByDef( Tag.attribute( "type", "" ) );
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
		QDomNode childNode = Tag.firstChild();
		std::vector< QDomElement > equipment;
		
		while( !childNode.isNull() )
		{		
			if( childNode.nodeName() == "item" )
				equipment.push_back( childNode.toElement() );
			else if( childNode.nodeName() == "getlist" && childNode.attributes().contains( "id" ) )
			{
				QStringList list = DefManager->getList( childNode.toElement().attribute( "id" ) );
				for( QStringList::iterator it = list.begin(); it != list.end(); it++ )
					if( DefManager->getSection( WPDT_ITEM, *it ) )
						equipment.push_back( *DefManager->getSection( WPDT_ITEM, *it ) );
			}

			childNode = childNode.nextSibling();
		}
		
		std::vector< QDomElement >::iterator iter = equipment.begin();
		while( iter != equipment.end() )
		{
			P_ITEM nItem = new cItem;
	
			if( nItem == NULL )
				continue;
	
			nItem->Init( true );

			QDomElement tItem = (*iter);

			nItem->applyDefinition( tItem );

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
				this->addItem( static_cast<cChar::enLayer>(mLayer), nItem ); // not sure about this one.

			++iter;
		}
	}

	else if( TagName == "inherit" )
	{
		QString inheritID;
		if( Tag.attributes().contains( "id" ) )
			inheritID = Tag.attribute( "id" );
		else
			inheritID = Value;

		const QDomElement* DefSection = DefManager->getSection( WPDT_NPC, inheritID );
		if( !DefSection->isNull() )
			this->applyDefinition( *DefSection );
	}

	else
	{
		INT16 skillId = Skills->findSkillByDef( TagName );

		if( skillId == -1 )
			cUObject::processNode( Tag );
		else
			setSkillValue( skillId, Value.toInt() );
	}
	
	cUObject::processNode( Tag );
}

void cBaseChar::addItem( cBaseChar::enLayer layer, cItem* pi, bool handleWeight, bool noRemove )
{
	// DoubleEquip is *NOT* allowed
	if ( atLayer( layer ) != 0 )
	{
		clConsole.send( "WARNING: Trying to put an item on layer %i which is already occupied\n", layer );
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
	else SET_STR_PROPERTY( "lootlist", lootlist_ )
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
	
	return cUObject::setProperty( name, value );
}

stError *cBaseChar::getProperty( const QString &name, cVariant &value ) const
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
	GET_PROPERTY( "lootlist", lootlist_ )
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

	return cUObject::getProperty( name, value );
}

void cBaseChar::setSkillValue( UINT16 skill, UINT16 value )
{
	skills[ skill ].value = value;

	// Check if we can delete the current skill
	const stSkillValue &skValue = skills[ skill ];

//	if( skValue.cap == 1000 && skValue.lock == 0 && skValue.value == 0 )
//		skills.remove( skill );

	changed( SAVE | TOOLTIP );
}

void cBaseChar::setSkillCap( UINT16 skill, UINT16 cap )
{
	skills[ skill ].cap = cap;

	// Check if we can delete the current skill
//	const stSkillValue &skValue = skills[ skill ];

//	if( skValue.cap == 1000 && skValue.lock == 0 && skValue.value == 0 )
//		skills.remove( skill );

	changed( SAVE | TOOLTIP );
}

void cBaseChar::setSkillLock( UINT16 skill, UINT8 lock )
{
	if( lock > 2 )
		lock = 0;

	skills[ skill ].lock = lock;

	// Check if we can delete the current skill
	const stSkillValue &skValue = skills[ skill ];

//	if( skValue.cap == 1000 && skValue.lock == 0 && skValue.value == 0 )
//		skills.remove( skill );

	changed( SAVE | TOOLTIP );
}

UINT16 cBaseChar::skillValue( UINT16 skill ) const
{
	return skills[ skill ].value;

//	if( skValue == skills.end() )
//		return 0;
}

UINT16 cBaseChar::skillCap( UINT16 skill ) const
{
	return skills[ skill ].cap;

//	if( skValue == skills.end() )
//		return 1000;
}

UINT8 cBaseChar::skillLock( UINT16 skill ) const
{
	return skills[ skill ].lock;

//	if( skValue == skills.end() )
//		return 0;
}



