//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
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

// Platform include
#include "platform.h"

// Wolfpack includes
#include "accounts.h"
#include "wpdefaultscript.h"
#include "chars.h"
#include "items.h"
#include "debug.h"
#include "tilecache.h"
#include "TmpEff.h"
#include "corpse.h"
#include "globals.h"
#include "wolfpack.h"
#include "iserialization.h"
#include "mapobjects.h"
#include "srvparams.h"
#include "utilsys.h"
#include "network.h"
#include "network/uosocket.h"
#include "network/uotxpackets.h" 
#include "maps.h"
#include "skills.h"
#include "wpdefmanager.h"
#include "guildstones.h"
#include "walking.h"
#include "persistentbroker.h"
#include "territories.h"
#include "dbdriver.h"
#include "combat.h"

#include "msgboard.h"
#include "makemenus.h"
#include "wpscriptmanager.h"
#include "world.h"
#include "itemid.h"

// Qt Includes
#include <qstringlist.h>

// Libary Includes
#include <math.h>

#undef  DBGFILE
#define DBGFILE "chars.cpp"

static cUObject* productCreator()
{
	return new cChar;
}

void cChar::registerInFactory()
{
	QStringList fields, tables, conditions;
	buildSqlString( fields, tables, conditions ); // Build our SQL string
	QString sqlString = QString( "SELECT /*! STRAIGHT_JOIN SQL_SMALL_RESULT */ uobjectmap.serial,uobjectmap.type,%1 FROM uobjectmap,%2 WHERE uobjectmap.type = 'cChar' AND %3" ).arg( fields.join( "," ) ).arg( tables.join( "," ) ).arg( conditions.join( " AND " ) );
	UObjectFactory::instance()->registerType("cChar", productCreator);
	UObjectFactory::instance()->registerSqlQuery( "cChar", sqlString );
}

bool cChar::Wears(P_ITEM pi)			{	return (this == pi->container());	}
bool  cChar::isGM() const				{return  priv&0x01 || account_ == 0 || ( account() && ( account()->acl() == "admin" || account()->acl() == "gm" ) );} 
bool  cChar::isCounselor() const		{return (priv&0x80 || ( account() && ( account()->acl() == "counselor") ) );} 
bool  cChar::isGMorCounselor() const	{return (priv&0x81 || ( account() && ( account()->acl() == "admin" || account()->acl() == "gm" || account()->acl() == "counselor" ) ) );} 

cChar::cChar():
	socket_(0), account_(0), owner_(0), guildstone_( INVALID_SERIAL ), guarding_( 0 ),
	regen_( 0 ), regen2_( 0 ), regen3_( 0 )
{
	changed( SAVE|TOOLTIP );
	VisRange_ = VISRANGE;
	Init( false );
}

cChar::cChar( const P_CHAR mob )
{
	changed( SAVE|TOOLTIP );
	this->content_ = mob->content();
	this->followers_ = mob->followers();
	this->guardedby_ = mob->guardedby();
	this->effects_ = mob->effects();
	this->owner_ = new cChar( mob->owner() );
	this->trackingTarget_ = mob->trackingTarget();
	this->GuildType = mob->guildType();
	this->GuildTraitor = mob->guildTraitor();
	this->orgname_ = mob->orgname();
	this->title_ = mob->title();
	this->sex_ = mob->sex();
	this->unicode_ = mob->unicode();
	this->id_ = mob->id();
	this->account_ = mob->account();
	this->incognito_ = mob->incognito();
	this->polymorph_ = mob->polymorph();
	this->haircolor_ = mob->haircolor();
	this->hairstyle_ = mob->hairstyle();
	this->beardstyle_ = mob->beardstyle();
	this->beardcolor_ = mob->beardstyle();
	this->skin_ = mob->skin();
	this->orgskin_ = mob->orgskin();
	this->xskin_ = mob->xskin();
	this->creationday_ = mob->creationday();
	this->stealth_ = mob->stealth();
	this->running_ = mob->running();
	this->logout_ = mob->logout();
	this->clientidletime_ = mob->clientidletime();
	this->swingtarg_ = mob->swingtarg();
	this->holdg_ = mob->holdg();
	this->fly_steps_ = mob->fly_steps();
	this->tamed_ = mob->tamed();
	this->casting_ = mob->casting();
	this->smoketimer_ = mob->smoketimer();
	this->smokedisplaytimer_ = mob->smokedisplaytimer();
	this->antispamtimer_ = mob->antispamtimer();
	this->guarding_ = new cChar( mob->guarding() );
	this->carve_ = mob->carve();
	this->hairserial_ = mob->hairserial();
	this->beardserial_ = mob->beardserial();
	this->begging_timer_ = mob->begging_timer();
	this->postType_ = mob->postType();
	this->questType_ = mob->questType();
	this->questDestRegion_ = mob->questDestRegion();
	this->questBountyReward_ = mob->questBountyReward();
	this->questBountyPostSerial_ = mob->questBountyPostSerial();
	this->murdererSer_ = mob->murdererSer();
	this->prevPos_ = mob->prevPos();
	this->commandLevel_ = mob->commandLevel();
	this->spawnregion_ = mob->spawnregion();
	this->stablemaster_serial_ = mob->stablemaster_serial();
	this->npc_type_ = mob->npc_type();
	this->time_unused_ = mob->time_unused();
	this->timeused_last_ = mob->timeused_last();
	this->spawnserial_ = mob->spawnSerial();
	this->hidden_ = mob->hidden();
	this->invistimeout_ = mob->invistimeout();
	this->attackfirst_ = mob->attackfirst();
	this->hunger_ = mob->hunger();
	this->hungertime_ = mob->hungertime();
	this->tailitem_ = mob->tailitem();
	this->npcaitype_ = mob->npcaitype();
	this->callnum_ = mob->callnum();
	this->playercallnum_ = mob->playercallnum();
	this->poison_ = mob->poison();
	this->poisoned_ = mob->poisoned();
	this->poisontime_ = mob->poisontime();
	this->poisontxt_ = mob->poisontxt();
	this->poisonwearofftime_ = mob->poisonwearofftime();
	this->fleeat_ = mob->fleeat();
	this->reattackat_ = mob->reattackat();
	this->envokeid_ = mob->envokeid();
	this->envokeitem_ = mob->envokeitem();
	this->split_ = mob->split();
	this->splitchnc_ = mob->splitchnc();
	this->ra_ = mob->ra();
	this->trainer_ = mob->trainer();
	this->trainingplayerin_ = mob->trainingplayerin();
	this->cantrain_ = mob->cantrain();
	this->guildtoggle_ = mob->guildtoggle();
	this->guildtitle_ = mob->guildtitle();
	this->guildfealty_ = mob->guildfealty();
	this->guildstone_ = mob->guildstone();
	this->flag_ = mob->flag();
	this->tempflagtime_ = mob->tempflagtime();
	this->trackingTimer_ = mob->trackingTimer();
	this->murderrate_ = mob->murderrate();
	this->crimflag_ = mob->crimflag();
	this->spelltime_ = mob->spelltime();
	this->spell_ = mob->spell();
	this->spellaction_ = mob->spellaction();
	this->nextact_ = mob->nextact();
	this->poisonserial_ = mob->poisonserial();
	this->squelched_ = mob->squelched();
	this->mutetime_ = mob->mutetime();
	this->med_ = mob->med();
	this->skills = mob->skills;

	this->socket_ = mob->socket();
	this->weight_ = mob->weight();
	this->priv = mob->getPriv();
	this->dx = mob->realDex();
	this->dx2 = mob->decDex();
	this->tmpDex = ( mob->effDex() > dx ) ? ( mob->effDex() - dx ) : 0;
	this->loot_ = mob->lootList();
	this->fonttype_ = mob->fonttype();
	this->saycolor_ = mob->saycolor();
	this->emotecolor_ = mob->emotecolor();
	this->st_ = mob->st();
	this->st2_ = mob->st2();
	this->may_levitate_ = mob->may_levitate();
	
	this->dispz_ = mob->dispz();
	this->dir_ = mob->dir();
	this->xid_ = mob->xid();
	this->priv2_ = mob->priv2();
	this->in_ = mob->in();
	this->in2_ = mob->in2();
	this->hp_ = mob->hp();
	this->stm_ = mob->stm();
	this->mn_ = mob->mn();
	this->mn2_ = mob->mn2();
	this->hidamage_ = mob->hidamage();
	this->lodamage_ = mob->lodamage();
	this->npc_ = mob->npc();
	this->shop_ = mob->shop();
	this->cell_ = mob->cell();
	this->jailtimer_ = mob->jailtimer();
	this->jailsecs_ = mob->jailsecs();
	this->robe_ = mob->robe();
	this->karma_ = mob->karma();
	this->fame_ = mob->fame();
	this->kills_ = mob->kills();
	this->deaths_ = mob->deaths();
	this->dead_ = mob->dead();
	this->fixedlight_ = mob->fixedlight();
	this->def_ = mob->def();
	this->war_ = mob->war();
	this->targ_ = mob->targ();
	this->timeout_ = mob->timeout();
	this->regen_ = mob->regen();
	this->regen2_ = mob->regen2();
	this->regen3_ = mob->regen3();
	this->inputmode_ = mob->inputmode();
	this->inputitem_ = mob->inputitem();
	this->attacker_ = mob->attacker();
	this->npcmovetime_ = mob->npcmovetime();
	this->npcWander_ = mob->npcWander();
	this->oldnpcWander_ = mob->oldnpcWander();
	this->ftarg_ = mob->ftarg();
	this->ptarg_ = mob->ptarg();
	this->fx1_ = mob->fx1();
	this->fx2_ = mob->fx2();
	this->fy1_ = mob->fy1();
	this->fy2_ = mob->fy2();
	this->fz1_ = mob->fz1();
	this->setRegion( cAllTerritories::getInstance()->region( mob->region()->name() ) );
	this->skilldelay_ = mob->skilldelay();
	this->objectdelay_ = mob->objectdelay();
	this->making_ = mob->making();
	this->lastTarget_ = mob->lastTarget();
	this->blocked_ = mob->blocked();
	this->dir2_ = mob->dir2();
	this->spiritspeaktimer_ = mob->spiritspeaktimer();
	this->spattack_ = mob->spattack();
	this->spadelay_ = mob->spadelay();
	this->spatimer_ = mob->spatimer();
	this->taming_ = mob->taming();
	this->summontimer_ = mob->summontimer();
	this->VisRange_ = mob->VisRange();
	this->profile_ = mob->profile();
	//this->lastselections_ = 
	this->food_ = mob->food();

	this->skills = mob->skills;
}

void cChar::giveGold( Q_UINT32 amount, bool inBank )
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

void cChar::setSerial( const SERIAL ser )
{
	// This is not allowed
	if( ser == INVALID_SERIAL )
		return;

	if( this->serial() != INVALID_SERIAL )
		World::instance()->unregisterObject( this->serial() );

	cUObject::setSerial( ser );
	
	World::instance()->registerObject( this );
}

void cChar::Init( bool createSerial )
{
	changed( SAVE|TOOLTIP );
	VisRange_ = VISRANGE;
	unsigned int i;
	cUObject::setSerial( INVALID_SERIAL );

	if( createSerial )
		this->setSerial( World::instance()->findCharSerial() );

	this->animated = false;
	this->setMultis( INVALID_SERIAL );//Multi serial
	this->free = false;
	this->setName("Man");
	this->setOrgname( "Man" );
	this->title_ = "";
	this->sex_ = true;
	this->socket_ = 0;
	this->setAntispamtimer(0);//LB - anti spam

	this->setUnicode(true); // This is set to 1 if the player uses unicode speech, 0 if not
	this->setPos( Coord_cl(100, 100, 0 ) );
	this->dir_=0; //&0F=Direction
	this->xid_ = 0x0190;
	this->setId(0x0190);
	this->setSkin(0); // Skin color
	this->setXSkin(0); // Skin color
	this->setPriv(0);	// 1:GM clearance, 2:Broadcast, 4:Invulnerable, 8: single click serial numbers
	// 10: Don't show skill titles, 20: GM Pagable, 40: Can snoop others packs, 80: Counselor clearance
	this->priv2_=0;	// 1:Allmove, 2: Frozen, 4: View houses as icons, 8: permanently hidden
	// 10: no need mana, 20: dispellable, 40: permanent magic reflect, 80: no need reagents
	this->setFontType( 3 ); // Speech font to use
	this->setSayColor( 0x1700 ); // Color for say messages
	this->setEmoteColor( 0x0023 ); // Color for emote messages
	this->setSt( 50 ); // Strength
	this->setSt2( 0 ); // Reserved for calculation
	this->dx=50; //  Dexterity
	this->dx2=0; // Reserved for calculation
	this->tmpDex=0; // Reserved for calculation
	this->in_=50; // Intelligence
	this->in2_=0; // Reserved for calculation
	this->hp_=50; // Hitpoints
	this->stm_=50; // Stamina
	this->mn_=50; // Mana
	this->mn2_=0; // Reserved for calculation
	this->hidamage_=0; //NPC Damage
	this->lodamage_=0; //NPC Damage
	this->jailtimer_=0; //blackwinds jail system 
    this->jailsecs_=0;
	
	this->setCreationDay(getPlatformDay());
	this->npc_=false;
	this->shop_=false; //1=npc shopkeeper
	this->cell_=0; // Reserved for jailing players 
	            // bugfix, LB 0= player not in jail !, not -1
	
	this->jailtimer_=0; //blackwinds jail system
	this->jailsecs_=0;

	this->setTamed(false); // True if NPC is tamed
	this->robe_ = -1; // Serial number of generated death robe (If char is a ghost)
	this->karma_ = 0;
	this->fame_ = 0;
	this->kills_ = 0; // PvP Kills
	this->deaths_ = 0;
	this->dead_ = false; // Is character dead
	this->fixedlight_ = 255; // Fixed lighting level (For chars in dungeons, where they dont see the night)
	// changed to -1, LB, bugfix
	this->setWeight( 0 );
	this->def_ = 0; // Intrinsic defense
	this->war_ = false; // War Mode
	this->targ_=INVALID_SERIAL; // Current combat target
	this->timeout_=0; // Combat timeout (For hitting)
	this->inputmode_ = enNone;
	this->inputitem_ = INVALID_SERIAL;
	this->attacker_ = INVALID_SERIAL; // Character's serial who attacked this character
	this->npcmovetime_ = 0; // Next time npc will walk
	this->npcWander_ = 0; // NPC Wander Mode
	this->oldnpcWander_ = 0; // Used for fleeing npcs
	this->ftarg_ = INVALID_SERIAL; // NPC Follow Target
	this->fx1_ = -1; //NPC Wander Point 1 x or Deed's Serial
	this->fx2_ = -1; //NPC Wander Point 2 x
	this->fy1_ = -1; //NPC Wander Point 1 y
	this->fy2_ = -1; //NPC Wander Point 2 y
	this->fz1_ = 0; //NPC Wander Point 1 z
	this->setSpawnSerial( INVALID_SERIAL ); // Spawned by
	this->setHidden(0); // 0 = not hidden, 1 = hidden, 2 = invisible spell
	this->setInvisTimeout(0);
	this->resetAttackFirst(); // 0 = defending, 1 = attacked first
	this->setHunger(6);  // Level of hungerness, 6 = full, 0 = "empty"
	this->setHungerTime(0); // Timer used for hunger, one point is dropped every 20 min
	this->setTailItem( INVALID_SERIAL );
	this->setNpcAIType(0); // NPC ai
	this->setCallNum(-1); //GM Paging
	this->setPlayerCallNum(-1); //GM Paging
	this->region_= NULL;
	this->skilldelay_ = 0;
	this->objectdelay_ = 0;
	this->making_ = -1; // skill number of skill using to make item, 0 if not making anything.
	this->blocked_ = 0;
	this->dir2_ = 0;
	this->spiritspeaktimer_ = 0; // Timer used for duration of spirit speak
	this->spattack_ = 0;
	this->spadelay_ = 0;
	this->spatimer_ = 0;
	this->taming_ = 0; //Skill level required for taming
	this->summontimer_ = 0; //Timer for summoned creatures.
	this->trackingTimer_ = 0; // Timer used for the duration of tracking
	this->trackingTarget_ = INVALID_SERIAL;

	this->setPoison(0); // used for poison skill 
	this->setPoisoned(0); // type of poison
	this->setPoisontime(0); // poison damage timer
	this->setPoisontxt(0); // poision text timer
	this->setPoisonwearofftime(0); // LB, makes poision wear off ...
	
	this->setFleeat(SrvParams->npc_base_fleeat());
	this->setReattackat(SrvParams->npc_base_reattackat());
	this->setEnvokeid(0x00); //ID of item user envoked
	this->setEnvokeitem(INVALID_SERIAL);
	this->setSplit(0);
	this->setSplitchnc(0);
	this->setRa(0);  // Reactive Armor spell
	this->setTrainer(INVALID_SERIAL); // Serial of the NPC training the char, -1 if none.
	this->setTrainingplayerin(0); // Index in skillname of the skill the NPC is training the player in
	this->setCantrain(true);
	// Begin of Guild Related Character information (DasRaetsel)
	this->setGuildtoggle(false);		// Toggle for Guildtitle								(DasRaetsel)
	this->setGuildtitle(QString::null);	// Title Guildmaster granted player						(DasRaetsel)
	this->setGuildfealty(INVALID_SERIAL);		// Serial of player you are loyal to (default=yourself)	(DasRaetsel)
	this->GuildTraitor=false; 
	//this->flag=0x04; //1=red 2=grey 4=Blue 8=green 10=Orange
	setcharflag( this );
	this->setTempflagtime(0);
	// End of Guild Related Character information
	this->setMurderrate(0); //# of ticks till murder decays.
	this->setCrimflag(0); // time when no longer criminal -1 = not criminal
	this->setCasting(false); // 0/1 is the cast casting a spell?
	this->setSpelltime(0); //Time when they are done casting....
	this->setSpell(0); //current spell they are casting....
	this->setSpellaction(0); //Action of the current spell....
	this->setNextact(0); //time to next spell action....
	this->setPoisonserial(INVALID_SERIAL); //AntiChrist -- poisoning skill
	
	this->setSquelched(0); // zippy  - squelching
	this->setMutetime(0); //Time till they are UN-Squelched.
	this->setMed(false); // false = not meditating, true = meditating //Morrolan - Meditation 
	this->setStealth(-1); //AntiChrist - stealth ( steps already done, -1=not using )
	this->setRunning(0); //AntiChrist - Stamina Loose while running
	this->setLogout(0);//Time till logout for this char -1 means in the world or already logged out //Instalog
	this->setSwingTarg(-1); //Tagret they are going to hit after they swing
	this->setHoldg(0); // Gold a player vendor is holding for Owner
	this->setFlySteps(0); //LB -> used for flyging creatures
	this->setSmokeTimer(0);
	this->setSmokeDisplayTimer(0);
	this->setAntiguardstimer(0); // AntiChrist - for "GUARDS" call-spawn
	this->setPolymorph(false);//polymorph - AntiChrist
	this->setIncognito(false);//incognito - AntiChrist
    this->setQuestDestRegion(0);
    this->setQuestOrigRegion(0);
    this->setQuestBountyReward(0);
    this->setQuestBountyPostSerial(INVALID_SERIAL);
    this->setMurdererSer(INVALID_SERIAL);
    this->setSpawnregion(0);
    this->setNpc_type(0);
    this->setStablemaster_serial(INVALID_SERIAL);
	this->setTimeused_last(getNormalizedTime());
	this->setTime_unused(0);
	this->questType_ = 0; //??
	this->GuildType = 0;
	this->setFood( 0 );

	this->skills.resize( ALLSKILLS );
}

///////////////////////
// Name:	GetItemOnLayer
// history:	by Duke, 26.3.2001
// Purpose:	returns the item on the given layer, if any

P_ITEM cChar::GetItemOnLayer(unsigned char layer)
{
	return atLayer( static_cast<enLayer>(layer) );
}

///////////////////////
// Name:	GetItemOnLayer
// history:	by Duke, 26.3.2001, touched by Correa, 21.04.2001
// Purpose:	Return the bank box. If banktype == 1, it will return the Item's bank box, else, 
//          gold bankbox is returned. 

P_ITEM cChar::getBankBox( void )
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

///////////////////////
// Name:	disturbMed
// history:	by Duke, 17.3.2001
// Purpose:	stops meditation if necessary. Displays message if a socket is passed

void cChar::disturbMed()
{
	if( med() ) //Meditation
	{
		this->setMed( false );

		if( socket_ )
			socket_->sysMessage( tr( "You loose your concentration" ) );
	}
}

///////////////////////
// Name:	unhide
// history:	by Duke, 17.3.2001
// Purpose:	reveals the char if he was hidden

void cChar::unhide()
{
	//if hidden but not permanently
	if( isHidden() && !( priv2_ & 8 ) )
	{
		setStealth( -1 );
		setHidden( 0 );
		resend( false ); // They cant see us anyway
	}
}

///////////////////////
// Name:	setNextMoveTime
// history:	by Duke, 20.3.2001
// Purpose:	sets the move timer. tamediv can shorten the time for tamed npcs

const double NPCSPEED = .3;

void cChar::setNextMoveTime(short tamediv)
{
//	if ( && this->tamed) return;	// MUST be nonzero
	// let's let them move once in a while ;)
	if(this->tamed())
		this->npcmovetime_=(unsigned int)((uiCurrentTime+double(NPCSPEED*MY_CLOCKS_PER_SEC/5)));
	else if(this->war_)
		this->npcmovetime_=(unsigned int)((uiCurrentTime+double(NPCSPEED*MY_CLOCKS_PER_SEC/5)));
	else
		this->npcmovetime_=(unsigned int)((uiCurrentTime+double(NPCSPEED*MY_CLOCKS_PER_SEC)));
}

///////////////////////
// Name:	fight
// history:	by Duke, 20.3.2001
// Purpose:	makes a character fight the other
//
void cChar::fight(P_CHAR other)
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
	else if( socket_ )
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

///////////////////////
// Name:	CountItems
// history:	by Duke, 26.3.2001
// Purpose:	searches the character recursively,
//			counting the items of the given ID and (if given) color

int cChar::CountItems( short ID, short col )
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

int cChar::CountBankGold()
{
	P_ITEM pi = getBankBox(); //we want gold bankbox.
	return pi->CountItems( 0x0EED );
}

bool cChar::hasWeapon()
{
	P_ITEM pi = atLayer( SingleHandedWeapon );
	if ( pi && pi->type() != 9 )
		return true;
	// Checking the other hand
	pi = atLayer( DualHandedWeapon );
	if ( pi && !IsShield(pi->id()) )
		return true;

	return false;
}

bool cChar::hasShield()
{
	P_ITEM pi=GetItemOnLayer(2);
	if (pi && IsShield(pi->id()) )
		return true;
	else
		return false;
}

P_ITEM cChar::getBackpack()	
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

///////////////////////
// Name:	setters for various serials
// history:	by Duke, 2.6.2001
// Purpose:	encapsulates revoval/adding to the pointer arrays

void cChar::SetSpawnSerial(long spawnser)
{
	changed( SAVE );
	if (spawnSerial() != INVALID_SERIAL)	// if it was set, remove the old one
		cspawnsp.remove(spawnSerial(), serial());

	spawnserial_ = spawnser;

	if (spawnser != INVALID_SERIAL)		// if there is a spawner, add it
		cspawnsp.insert(spawnserial_, serial());
}

void cChar::SetMultiSerial(long mulser)
{
	this->setMultis( mulser );
}

void cChar::MoveToXY(short newx, short newy)
{
	this->MoveTo(newx,newy,pos().z);	// keep the old z value
}

void cChar::MoveTo(short newx, short newy, signed char newz)
{
	// Avoid crash if go to 0,0
	if (newx < 1 || newy < 1)
		return;
	cUObject::moveTo( Coord_cl(newx, newy, newz, pos().map) );
}

unsigned int cChar::getSkillSum()
{
	unsigned int sum = 0;

	QValueVector< stSkillValue >::const_iterator it = skills.begin();
	for( ; it != skills.end(); ++it )
		sum += (*it).value;

	return sum;		// this *includes* the decimal digit ie. xxx.y
}

///////////////////////
// Name:	getTeachingDelta
// history:	by Duke, 27.7.2001
// Purpose:	calculates how much the given player can learn from this teacher

int cChar::getTeachingDelta(cChar* pPlayer, int skill, int sum)
{
	int delta = QMIN(250,skillValue(skill)/2);		// half the trainers skill, but not more than 250
	delta -= skillValue(skill);					// calc difference
	if (delta <= 0)
		return 0;

	if (sum+delta >= SrvParams->skillcap() * 10)			// would new skill value be above cap ?
		delta = (SrvParams->skillcap() * 10) - sum;		// yes, so reduce it
	return delta;
}

////////////
// Name:	removeItemBonus
// history:	by Duke, 19.8.2001
// Purpose:	removes boni given by an item

void cChar::removeItemBonus(cItem* pi)
{
	changed( SAVE|TOOLTIP );
//	this->st -= pi->st2;
	this->setSt( ( this->st() ) - pi->st2());
	this->chgDex(-1 * pi->dx2());
	this->in_ -= pi->in2();
}

////////////
// Name:	canPickUp
// history:	by Duke, 20.9.2001
// Purpose:	checks if the char can drag the item

bool cChar::canPickUp( cItem* pi )
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

void cChar::buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions )
{
	cUObject::buildSqlString( fields, tables, conditions );
	fields.push_back( "characters.name,characters.title,characters.account,characters.creationday" );
	fields.push_back( "characters.guildtype,characters.guildtraitor,characters.cell" );
	fields.push_back( "characters.dir,characters.body,characters.xbody,characters.skin" );
	fields.push_back( "characters.xskin,characters.priv,characters.stablemaster,characters.npctype" );
	fields.push_back( "characters.time_unused,characters.allmove,characters.font,characters.say" );
	fields.push_back( "characters.emote,characters.strength,characters.strength2,characters.dexterity" );
	fields.push_back( "characters.dexterity2,characters.intelligence,characters.intelligence2" );
	fields.push_back( "characters.hitpoints,characters.spawnregion,characters.stamina" );
	fields.push_back( "characters.mana,characters.npc,characters.holdgold,characters.shop" );
	fields.push_back( "characters.owner,characters.robe,characters.karma,characters.fame" );
	fields.push_back( "characters.kills,characters.deaths,characters.dead,characters.fixedlight" );
	fields.push_back( "characters.cantrain,characters.def" );
	fields.push_back( "characters.lodamage,characters.hidamage,characters.war,characters.npcwander" );
	fields.push_back( "characters.oldnpcwander,characters.carve,characters.fx1,characters.fy1,characters.fz1" );
	fields.push_back( "characters.fx2,characters.fy2,characters.spawn,characters.hidden,characters.hunger" );
	fields.push_back( "characters.npcaitype,characters.spattack,characters.spadelay,characters.taming" );
	fields.push_back( "characters.summontimer,characters.poison,characters.poisoned" );
	fields.push_back( "characters.fleeat,characters.reattackat,characters.split,characters.splitchance" );
	fields.push_back( "characters.guildtoggle,characters.guildstone,characters.guildtitle,characters.guildfealty" );
	fields.push_back( "characters.murderrate,characters.questtype,characters.questdestregion" );
	fields.push_back( "characters.questorigregion,characters.questbountypostserial,characters.questbountyreward,characters.jailtimer" );
	fields.push_back( "characters.jailsecs,characters.lootlist,characters.food,characters.profile,characters.guarding,characters.destination" );
	tables.push_back( "characters" );
	conditions.push_back( "uobjectmap.serial = characters.serial" );
}

static void characterRegisterAfterLoading( P_CHAR pc );

void cChar::load( char **result, UINT16 &offset )
{
	cUObject::load( result, offset );

	// Broken Serial?
	if( !isCharSerial( serial() ) )
		throw QString( "Character has invalid char serial: 0x%1" ).arg( serial(), 0, 16 );

	orgname_ = result[offset++];
	title_ = result[offset++];
	setAccount( Accounts::instance()->getRecord( result[offset++] ) );
	creationday_ = atoi( result[offset++] );
	GuildType = atoi( result[offset++] );
	GuildTraitor = atoi( result[offset++] );
	cell_ = atoi( result[offset++] );
	dir_ = atoi( result[offset++] );
	xid_ = atoi( result[offset++] ); setId( xid_ );
	xid_ = atoi( result[offset++] );
	skin_ = atoi( result[offset++] );
	xskin_ = atoi( result[offset++] );
	priv = atoi( result[offset++] );
	stablemaster_serial_ = atoi( result[offset++] );
	npc_type_ = atoi( result[offset++] );
	time_unused_ = atoi( result[offset++] );
	priv2_ = atoi( result[offset++] );
	fonttype_ = atoi( result[offset++] );
	saycolor_ = atoi( result[offset++] );
	emotecolor_ = atoi( result[offset++] );
	st_ = atoi( result[offset++] );
	st2_ = atoi( result[offset++] );
	dx = atoi( result[offset++] );
	dx2 = atoi( result[offset++] );
	in_ = atoi( result[offset++] );
	in2_ = atoi( result[offset++] );
	hp_ = atoi( result[offset++] );
	spawnregion_ = result[offset++];
	stm_ = atoi( result[offset++] );
	mn_ = atoi( result[offset++] );
	npc_ = atoi( result[offset++] );
	holdg_ = atoi( result[offset++] );
	shop_ = atoi( result[offset++] );

	//  Warning, ugly optimization ahead, if you have a better idea, we want to hear it. 
	//  For load speed and memory conservation, we will store the SERIAL of the container
	//  here and then right after load is done we replace that value with it's memory address
	//  as it should be.
	owner_ = (P_CHAR)atoi( result[offset++] );
	if( (SERIAL)owner_ == INVALID_SERIAL )
		owner_ = 0;

	robe_ = atoi( result[offset++] );
	karma_ = atoi( result[offset++] );
	fame_ = atoi( result[offset++] );
	kills_ = atoi( result[offset++] );
	deaths_ = atoi( result[offset++] );
	dead_ = atoi( result[offset++] );
	fixedlight_ = atoi( result[offset++] );
	cantrain_ = atoi( result[offset++] );
	def_ = atoi( result[offset++] );
	lodamage_ = atoi( result[offset++] );
	hidamage_ = atoi( result[offset++] );
	war_ = atoi( result[offset++] );
	npcWander_ = atoi( result[offset++] );
	oldnpcWander_ = atoi( result[offset++] );
	carve_ = result[offset++];
	fx1_ = atoi( result[offset++] );
	fy1_ = atoi( result[offset++] );
	fz1_ = atoi( result[offset++] );
	fx2_ = atoi( result[offset++] );
	fy2_ = atoi( result[offset++] );
	spawnserial_ = atoi( result[offset++] );
	hidden_ = atoi( result[offset++] );
	hunger_ = atoi( result[offset++] );
	npcaitype_ = atoi( result[offset++] );
	spattack_ = atoi( result[offset++] );
	spadelay_ = atoi( result[offset++] );
	taming_ = atoi( result[offset++] );
	summontimer_ = atoi( result[offset++] );
	if( summontimer_ )
		summontimer_ += uiCurrentTime;

	poison_ = atoi( result[offset++] );
	poisoned_ = atoi( result[offset++] );
	fleeat_ = atoi( result[offset++] );
	reattackat_ = atoi( result[offset++] );
	split_ = atoi( result[offset++] );
	splitchnc_ = atoi( result[offset++] );
	guildtoggle_ = atoi( result[offset++] );
	guildstone_ = atoi( result[offset++] );
	guildtitle_ = result[offset++];
	guildfealty_ = atoi( result[offset++] );
	murderrate_ = atoi( result[offset++] );
	questType_ = atoi( result[offset++] );
	questDestRegion_ = atoi( result[offset++] );
	questOrigRegion_ = atoi( result[offset++] );
	questBountyPostSerial_ = atoi( result[offset++] );
	questBountyReward_ = atoi( result[offset++] );
	jailtimer_ = atoi( result[offset++] );
	if (jailtimer_ != 0)
		jailtimer_ += uiCurrentTime;

	jailsecs_ = atoi( result[offset++] );
	loot_ = result[offset++];
	food_ = atoi( result[offset++] );
	profile_ = result[offset++];
	
	// UGLY OPTIMIZATION!
	guarding_ = (P_CHAR)atoi( result[offset++] );
	if( (SERIAL)guarding_ == -1 )
		guarding_ = 0;
	
	parseCoordinates( result[offset++], ptarg_ );
	sex_ = atoi( result[offset++] );

	SetSpawnSerial( spawnserial_ );

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

		skills[ skill ] = skValue;
	}

	res.free();

	characterRegisterAfterLoading( this );
	changed_ = false;
}

void cChar::save()
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
		addField( "guildtype", GuildType );
		addField( "guildtraitor", GuildTraitor );
		addField( "cell", cell_ );
		addField( "dir", dir_ );
		
		addField( "body", (incognito() || polymorph()) ? xid_ : id_ );
		addField( "xbody", xid_ );
		addField( "skin", incognito() ? xskin_ : skin_ );
		addField( "xskin", xskin_ );
		addField( "priv", priv );
		addField( "stablemaster", stablemaster_serial_ );
		addField( "npctype", npc_type_ );
		addField( "time_unused", time_unused_ );
		
		addField( "allmove", priv2_);
		addField( "font", fonttype_);
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
		addField( "holdgold", holdg_);
		addField( "shop", shop_);
		
		addField( "owner", owner_ ? owner_->serial() : INVALID_SERIAL );
		
		addField( "robe", robe_);
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
		addField( "spawn", spawnserial_);
		addField( "hidden", hidden_);
		addField( "hunger", hunger_);
		addField( "npcaitype", npcaitype_);
		addField( "spattack", spattack_);
		addField( "spadelay", spadelay_);
		addField( "taming", taming_);
		unsigned int summtimer = summontimer_ - uiCurrentTime;
		addField( "summonremainingseconds", summtimer);
		addField( "poison", poison_);
		addField( "poisoned", poisoned_);
		addField( "fleeat", fleeat_);
		addField( "reattackat", reattackat_);
		addField( "split", split_);
		addField( "splitchance",	splitchnc_);
		addField( "guildtoggle",	guildtoggle_);  
		addField( "guildstone", guildstone_);  
		addField( "guildtitle", guildtitle_);  
		addField( "guildfealty", guildfealty_);  
		addField( "murderrate", murderrate_);
		addField( "questtype", questType_);
		addField( "questdestregion", questDestRegion_);
		addField( "questorigregion", questOrigRegion_);
		addField( "questbountypostserial", questBountyPostSerial_);
		addField( "questbountyreward", questBountyReward_);
		unsigned int jtimer = jailtimer_-uiCurrentTime;
		addField( "jailtimer", jtimer); 
		addField( "jailsecs", jailsecs_); 
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
	cUObject::save();
	changed_ = false;
}

bool cChar::del()
{	
	if( !isPersistent )
		return false; // We didn't need to delete the object

	persistentBroker->addToDeleteQueue( "characters", QString( "serial = '%1'" ).arg( serial() ) );
	persistentBroker->addToDeleteQueue( "skills", QString( "serial = '%1'" ).arg( serial() ) );
	changed( SAVE );
	return cUObject::del();
}

//========== WRAPPER EVENTS

bool cChar::onPickup( P_ITEM pItem )
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

// Shows the name of a character to someone else
bool cChar::onSingleClick( P_CHAR Viewer ) 
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
bool cChar::onWalk( UI08 Direction, UI08 Sequence )
{
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onWalk( (P_CHAR)this, Direction, Sequence ) )
			return true;

	return false;
}

// The character says something
bool cChar::onTalk( char speechType, UI16 speechColor, UI16 speechFont, const QString &Text, const QString &Lang )
{
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onTalk( (P_CHAR)this, speechType, speechColor, speechFont, Text, Lang ) )
			return true;

	return false;
}

// The character switches warmode
bool cChar::onWarModeToggle( bool War )
{
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onWarModeToggle( this, War ) )
			return true;

	return false;
}

bool cChar::onLogin( void )
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

bool cChar::onLogout( void )
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
bool cChar::onHelp( void )
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

// The paperdoll of this character has been requested
bool cChar::onShowPaperdoll( P_CHAR pOrigin )
{
	for( UI08 i = 0; i < scriptChain.size(); i++ )
	{
		if( scriptChain[ i ]->onShowPaperdoll( this, pOrigin ) )
			return true;
	}

	return false;
}

// The character wants to chat
bool cChar::onChat( void )
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

// The character uses %Skill
bool cChar::onSkillUse( UI08 Skill ) 
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

bool cChar::onCollideChar( P_CHAR Obstacle ) 
{
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onCollideChar( this, Obstacle ) )
			return true;

	return false;
}

bool cChar::onDropOnChar( P_ITEM pItem )
{
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onDropOnChar( this, pItem ) )
			return true;

	return false;
}

void cChar::processNode( const QDomElement &Tag )
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

	//<carve>3</carve>
	else if( TagName == "carve" ) 
		this->setCarve( Value );

	//<cantrain />
	else if( TagName == "cantrain" )
		this->setCantrain( true );

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

	//<emotecolor>0x482</emotecolor>
	else if( TagName == "emotecolor" )
		this->emotecolor_ = Value.toUShort();

	//<fleeat>10</fleeat>
	else if( TagName == "fleeat" )
		this->setFleeat( Value.toShort() );

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

	//<hidamage>10</hidamage>
	else if( TagName == "hidamage" )
		this->hidamage_ = Value.toInt();

	//<id>0x11</id>
	else if( TagName == "id" )
	{
		this->setId( Value.toInt() );
		this->xid_ = this->id();
	}

	//<karma>-500</karma>
	else if( TagName == "karma" )
		this->karma_ = Value.toInt();

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

	//<priv1>0</priv1>
	else if( TagName == "priv1" )
		this->setPriv( Value.toUShort() );

	//<priv2>0</priv2>
	else if( TagName == "priv2" )
		this->priv2_ = Value.toUShort();
	//<poison>2</poison>
	else if( TagName == "poison" )
		this->setPoison( Value.toInt() );
	//<reattackat>40</reattackat>
	else if( TagName == "reattackat" )
		this->setReattackat( Value.toShort() );

	//<skin>0x342</skin>
	else if( TagName == "skin" )
	{
		this->setSkin( Value.toUShort() );
		this->setXSkin( Value.toUShort() );
	}

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
		
	//<spattack>3</spattack>
	else if( TagName == "spattack" )
		this->spattack_ = Value.toInt();

	//<split>1</split>
	else if( TagName == "split" )
		this->setSplit( Value.toUShort() );

	//<splitchance>10</splitchance>
	else if( TagName == "splitchance" )
		this->setSplitchnc( Value.toUShort() );

	//<saycolor>0x110</saycolor>
	else if( TagName == "saycolor" )
		this->setSayColor( Value.toUShort() );

	//<spadelay>3</spadelay>
	else if( TagName == "spadelay" )
		this->spadelay_ = Value.toInt();

	//<stablemaster />
	else if( TagName == "stablemaster" )
		this->setNpc_type(1);

	//<title>the king</title>
	else if( TagName == "title" )
		this->setTitle( Value );

	//<totame>115</totame>
	else if( TagName == "totame" )
		this->taming_ = Value.toInt();

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
}

void cChar::soundEffect( UI16 soundId, bool hearAll )
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

void cChar::talk( const QString &message, UI16 color, UINT8 type, bool autospam, cUOSocket* socket )
{
	if( autospam )
	{
		if( antispamtimer() < uiCurrentTime )
			setAntispamtimer( uiCurrentTime + MY_CLOCKS_PER_SEC*10 );
		else 
			return;
	}

	if( color == 0xFFFF )
		color = saycolor_;

	QString lang;

	if( this->socket() )
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
		if( dead_ && !isNpc() )
			if( !socket->player()->dead() && !socket->player()->spiritspeaktimer() && !socket->player()->isGMorCounselor() )
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
					if( dead_ && !isNpc() )
						if( !mSock->player()->dead() && !mSock->player()->spiritspeaktimer() && !mSock->player()->isGMorCounselor() )
							textSpeech->setText( ghostSpeech );
						else
							textSpeech->setText( message );

					mSock->send( new cUOTxUnicodeSpeech( *textSpeech ) );
				}
		}
		delete textSpeech;
	}
}

void cChar::emote( const QString &emote, UI16 color )
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

void cChar::message( const QString &message, UI16 color )
{
	if( !socket_ )
		return;

	socket_->showSpeech( this, message, color, 3 );
}

void cChar::setAccount( AccountRecord* data, bool moveFromAccToAcc )
{
	changed( SAVE|TOOLTIP );
	if( moveFromAccToAcc && account_ != 0 )
		account_->removeCharacter( this );

	account_ = data;

	if( account_ != 0 )
		account_->addCharacter( this );
}

void cChar::giveItemBonus(cItem* pi)
{
	changed( SAVE|TOOLTIP );
	st_ += pi->st2();
	chgDex( pi->dx2() );
	in_ += pi->in2();
}

void cChar::showName( cUOSocket *socket )
{
	if( !socket->player() )
		return;

	if( onSingleClick( socket->player() ) )
		return;

	QString charName = name();

	// For NPCs we can apply titles
	if( !isPlayer() && SrvParams->showNpcTitles() && !title_.isEmpty() )
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

	// Append offline flag
	if( !isNpc() && !socket_ )
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

	cGuildStone *guildStone = dynamic_cast< cGuildStone* >( FindItemBySerial( guildstone_ ) );

	// If we belong to a guild append the guilds title
	// [Guildmaster, SdV] [Chaos]
	if( guildStone )
	{
		if( !guildtitle_.isEmpty() )
			charName.append( QString( " [%1, %2]" ).arg( guildtitle_ ).arg( guildStone->abbreviation.c_str() ) );
		else
			charName.append( QString( " [%2]" ).arg( guildStone->abbreviation.c_str() ) );

		switch( guildStone->guildType )
		{
		case cGuildStone::order:	charName.append( tr(" [Order]") ); break;
		case cGuildStone::chaos:	charName.append( tr(" [Chaos]") ); break;
		}
	}
	
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

UINT16 cChar::bestSkill()
{
	UINT16 skill = 0;
	UINT16 skillVal = 0;
	bool skillUp = false;

	for( UINT32 i = 0; i < ALLSKILLS; ++i )
	{
		// Non Locked Skills have Priority
		if( skillValue( i ) > skillVal || ( skillValue( i ) == skillVal && !skillLock( i ) && !skillUp ) )
		{
			skill = i;
			skillVal = skillValue( i );
			skillUp = ( skillLock( i ) == 0 );
		}
	}

	return skill;
}

QString cChar::reputationTitle()
{
	// !!! THIS IS A SLOW IMPLEMENTATION !!!
	// Need to cache titles somewhere
	
	// Get our Reputation Titles
	QStringList titles = DefManager->getList( "REPUTATION_TITLES" );

	// Calculate the Row and Col of our Title
	UINT8 col;

	// FAME Column
	if( fame_ >= 5000 )
		col = 4;
	else if( fame_ >= 5000 )
		col = 3;
	else if( fame_ >= 2500 )
		col = 2;
	else if( fame_ >= 1250 )
		col = 1;
	else
		col = 0;

	UINT8 row;

	// Positive Karma
	if( karma_ >= 10000 )
		row = 0;
	else if( karma_ >= 5000 )
		row = 1;
	else if( karma_ >= 2500 )
		row = 2;
	else if( karma_ >= 1250 )
		row = 4;
	else if( karma_ >= 625 )
		row = 5;

	// Neutral Karma
	else if( karma_ >= -625 )
		row = 6;

	// Negative Karma
	else if( karma_ >= -1249 )
		row = 7;

	else if( karma_ >= -2499 )
		row = 8;

	else if( karma_ >= -4999 )
		row = 9;

	else if( karma_ >= -9999 )
		row = 10;

	else
		row = 11;
	
	// Offset = row * 5 + col
	UINT32 offset = row * 5 + col;
	
	if( offset > titles.count() )
		return QString::null;

	QString fametitle;
	
	// Lady/Lord Title
	if( fame_ >= 10000 )
	{
		if( kills_ >= (unsigned)SrvParams->maxkills() )
		{
			if ( id_ == 0x0191 ) 
				fametitle = "The Murderous Lady ";
			else
				fametitle = "The Murderous Lord ";
		}
		else if( id_ == 0x0191 )
			fametitle = QString( "The %1Lady " ).arg( titles[offset] );
		else
			fametitle = QString( "The %1Lord " ).arg( titles[offset] );
	}
	else
	{
		if( kills_ >= (unsigned)SrvParams->maxkills() )
		{
			fametitle = "The Murderer "; //Morrolan rep
		}
		else if( !titles[offset].isNull() && !titles[offset].isEmpty() )
			fametitle = QString( "The %1" ).arg( titles[offset] );
		else
			fametitle = "";
	}

	return fametitle;
}

QString cChar::fullName( void )
{
	QString fName;

	// Skill Title (full)
	QString skillTitle = Skills->getSkillTitle( this );
	QString repTitle = reputationTitle();

	if( isGM() )
		fName = QString( "%1 %2" ).arg( name() ).arg( title_ );

	// Normal Criminal
	else if( ( crimflag_ > 0 ) && !dead_ && ( kills_ < SrvParams->maxkills() ) )
		fName = tr( "The Criminal %1" ).arg( name() );

	// The Serial Killer
	else if( ( kills_ >= SrvParams->maxkills() ) && ( kills_ < 10 ) && !dead_ )
		fName = tr( "The Serial Killer %1" ).arg( name() );

	// The Murderer
	else if( ( kills_ >= 10 ) && ( kills_ < 20 ) && !dead_ )
		fName = tr( "The Murderer %1" ).arg( name() );

	// The Mass Murderer
	else if( ( kills_ >= 20 ) && ( kills_ < 50 ) && !dead_ )
		fName = tr( "The Mass Murderer %1" ).arg( name() );

	// The Evil Dread Murderer
	else if( ( kills_ >= 50 ) && ( kills_ < 100 ) && !dead_ )
		fName = tr( "The Evil Dread Murderer %1" ).arg( name() );

	// The Evil Emperor
	else if( ( kills_ >= 100 ) && !dead_ )
		fName = tr( "The Evil Emperor %1" ).arg( name() );

	// Normal Player
	else if( !repTitle.isNull() )
		fName = QString( "%1%2" ).arg( repTitle ).arg( name() );

	else
		fName = name();

	// Append title (skillTitle)
	if( !title_.isEmpty() )
		fName.append( QString( ", %1" ).arg( title_ ) );

	if( !skillTitle.isNull() && !skillTitle.isEmpty() )
		fName.append( QString( ", %1" ).arg( skillTitle ) );

	return fName;
}

cGuildStone *cChar::getGuildstone()
{ 
	return dynamic_cast<cGuildStone*>( FindItemBySerial( guildstone_ ) ); 
}

void cChar::makeShop( void )
{
	changed( SAVE );
	shop_ = true;

	// We need to create the same item on several layers
	for( UINT8 layer = 0x1A; layer <= 0x1C; ++layer )
	{
		// If there already is something we just skip
		if( GetItemOnLayer( layer ) )
			continue;

		P_ITEM pItem = Items->SpawnItem( this, 1, "#", 0, 0x2AF8, 0, 0 );
		
		if( pItem )
		{
			pItem->setType( 1 );
			pItem->setPriv( pItem->priv() | 0x02 );
			this->addItem( static_cast<cChar::enLayer>(layer), pItem );
		}
	}
}

// Send the changed health-bar to all sockets in range
void cChar::updateHealth( void )
{
	RegionIterator4Chars cIter( pos() );
	for( cIter.Begin(); !cIter.atEnd(); cIter++ )
	{
		P_CHAR pChar = cIter.GetData();

		// Send only if target can see us
		if( !pChar || !pChar->socket() || !pChar->inRange( this, pChar->VisRange() ) || ( isHidden() && !pChar->isGM() && this != pChar ) )
			continue;
	
		pChar->socket()->updateHealth( this );
	}
}

class cResetAnimated: public cTempEffect
{
public:
	cResetAnimated( P_CHAR pChar, UINT32 ms )
	{
		expiretime = uiCurrentTime + ms;
		sourSer = pChar->serial();
		serializable = false; // We dont want to save this
	}

	virtual void Expire() 
	{
		// Reset the animated status
		P_CHAR pChar = FindCharBySerial( sourSer );

		if( pChar )
			pChar->setAnimated( false );
	}
};

void cChar::action( UINT8 id )
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
		if( socket->player() && socket->player()->inRange( this, socket->player()->VisRange() ) && ( !isHidden() || socket->player()->isGM() ) )
			socket->send( &action );
	}
}

UINT8 cChar::notority( P_CHAR pChar ) // Gets the notority toward another char
{
	// 0x01 Blue, 0x02 Green, 0x03 Grey, 0x05 Orange, 0x06 Red
	UINT8 result;

	// Check for Guild status + Highlight
	UINT8 guildStatus = GuildCompare( this, pChar );

	if( npcaitype() == 0x02 )
		return 0x06; // 6 = Red -> Monster

	if( pChar->kills() > SrvParams->maxkills() )
		result = 0x06; // 6 = Red -> Murderer
	
	else if( guildStatus == 1 )
		result = 0x02; // 2 = Green -> Same Guild
	
	else if( guildStatus == 2 )
		result = 0x05; // 5 = Orange -> Enemy Guild

	// Only players have accounts
	else if( account() )
	{
		if( crimflag() )
			result = 0x03;
		else if( karma_ < -2000 ) 
			result = 0x06;
		else if( karma_ < 0 )
			result = 0x03;
		else
			result = 0x01;
	}
	// Monsters, Human NPCs, Animals
	else
	{
		// Monsters are always bad
		if( npcaitype_ == 4 )
			return 0x01;
		
		else if( isHuman() )
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

// Formerly deathstuff()
void cChar::kill()
{
	changed( SAVE|TOOLTIP );
	int ele;
	int nType=0;

	if( free )
		return;

	if( dead_ || npcaitype() == 17 || isInvul() )
		return;

	// Do this in the beginning
	dead_ = true; // Dead
	hp_ = 0; // With no hp left

	if( polymorph() )
	{
		setId( xid_ );
		setPolymorph( false );
		// Resending here is pointless as the character will be removed l8er anyway
	}

	xid_ = id(); // lb bugfix
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
	if( isPlayer() )
	{
	    if( isInnocent() )
			corpse->setMore2(1);
	    else if( isCriminal() )
			corpse->setMore2(2);
	    else if( isMurderer() )
			corpse->setMore2(3);

        corpse->setOwner( this );
	}

	corpse->setBodyId( xid_ );
	corpse->setMoreY( ishuman( this ) ); //is human??
	corpse->setCarve( carve() ); //store carve section
	corpse->setName2( name() );

	corpse->moveTo( pos() );

	corpse->setMore1(nType);
	corpse->setDirection( dir_ );
	corpse->startDecay();
	
	// If it was a player set the ownerserial to the player's
	if( isPlayer() )
	{
		corpse->SetOwnSerial(serial());
		// This is.... stupid...
		corpse->setMore4( char( SrvParams->playercorpsedecaymultiplier()&0xff ) ); // how many times longer for the player's corpse to decay
	}

	// stores the time and the murderer's name
	corpse->setMurderer( murderer );
	corpse->setMurderTime(uiCurrentTime);

	// create loot
	if( isNpc() )
	{
		QStringList lootItemSections = DefManager->getList( lootList() );
		QStringList::const_iterator it = lootItemSections.begin();

		while( it != lootItemSections.end() )
		{
			P_ITEM pi_loot = Items->createScriptItem( (*it) );
			if( pi_loot )
				corpse->addItem( pi_loot );
//			if( pi_loot )
//				pi_loot->setContSerial( corpse->serial() );
// Restructuring
			it++;
		}
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

	if( isPlayer() )
	{
#pragma note( "Deathshroud has to be defined as 204e in the scripts" )
		P_ITEM pItem = Items->createScriptItem( "204e" );
		if( pItem )
		{
			robe_ = pItem->serial();
			this->addItem( cChar::OuterTorso, pItem );
			pItem->update();
		}
	}

	resend( true );

	if( socket_ )
	{
		cUOTxCharDeath cDeath;
		socket_->send( &cDeath );
	}

//	if ((ele==13)||(ele==15)||(ele==16)||(ele==574))//-Frazurbluu, we're gonna remove this strange little function :)
// it becomes OSI exact with it removed! -Fraz- I DO NEED TO CHECK ENERGY VORTEXS!!!!!!!!!!!!!!!!!!!!!!
//	{		// *** This looks very strange to me! Turning the shroud into a backpack ??? Duke 9.8.2k ***
//		strcpy(corpse->name,"a backpack");
//		corpse->color1=0;
//		corpse->color2=0;
//		corpse->amount = 1;
//		corpse->setId(0x09B2);
//		corpse->corpse=0;
//	}

	if( isNpc() )
		cCharStuff::DeleteChar( this );

	// Wtf ?? Summoned Creatures -> should be flag
	// if( ele == 65535 )
	//	Items->DeleItem( corpse );
}

// Formerly NpcResurrectTarget
// This should check soon if we are standing above our 
// corpse and if so, merge with our corpse instead of
// just resurrecting
void cChar::resurrect()
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

void cChar::turnTo( const Coord_cl &pos )
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

		// Send a different packet for ourself
		if( socket_ )
		{
			socket_->resendPlayer( true );

			/*cUOTxForceWalk forceWalk;
			forceWalk.setDirection( dir_ );
			socket_->send( &forceWalk );*/
		}
	}
}

void cChar::turnTo( cUObject *object )
{
	turnTo( object->pos() );
}

UINT32 cChar::takeGold( UINT32 amount, bool useBank )
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

/*!
  Updates everyone in range with the character's equipped items, including
  himself.
  \warning This could generate a lot of network trafic, always prefer using
  \a wear()
*/
void cChar::updateWornItems()
{
	P_ITEM pi;
	ContainerContent container = this->content();
	ContainerContent::const_iterator it  = container.begin();
	ContainerContent::const_iterator end = container.end();
	for ( ; it != end; ++it )
	{
		pi = *it;
		if (pi != NULL && !pi->free)
		{
			cUOTxCharEquipment packet;
			packet.setWearer( this->serial() );
			packet.setSerial( pi->serial() );
			packet.fromItem( pi );
			for ( cUOSocket* socket = cNetwork::instance()->first(); socket != 0; socket = cNetwork::instance()->next() )
				if( socket->player() && socket->player()->inRange( this, socket->player()->VisRange() ) ) 
					socket->send( &packet );
		}
	}
}

/*!
  \overloaded
  Just like the above, but updates only the given socket with the worn items
*/
void cChar::updateWornItems( cUOSocket* socket )
{
	ContainerContent container = this->content();
	ContainerContent::const_iterator it  = container.begin();
	ContainerContent::const_iterator end = container.end();
	for ( ; it != end; ++it )
	{
		cItem* pi = *it;
		if (pi != NULL && !pi->free)
		{
			cUOTxCharEquipment packet;
			packet.setWearer( this->serial() );
			packet.setSerial( pi->serial() );
			packet.fromItem( pi );
			socket->send( &packet );
		}
	}
}

/*!
  Wears the given item and sends an update to those in range.
*/
void cChar::wear( P_ITEM pi )
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

P_CHAR cChar::unmount()
{
	P_ITEM pi = atLayer(Mount);
	if( pi && !pi->free)
	{
		P_CHAR pMount = FindCharBySerial( pi->morex() );
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

void cChar::mount( P_CHAR pMount )
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
		
		// set timer
		pMount->setTime_unused( 0 );
		pMount->setTimeused_last( getNormalizedTime() );
	}
	else
		socket->sysMessage( tr("You dont own that creature.") );
}

void cChar::giveNewbieItems( Q_UINT8 skill ) 
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

void cChar::applyStartItemDefinition( const QDomElement &Tag )
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

QPtrList< cMakeSection > cChar::lastSelections( cMakeMenu* basemenu )
{ 
	QMap< cMakeMenu*, QPtrList< cMakeSection > >::iterator it = lastselections_.find( basemenu );
	if( it != lastselections_.end() )
		return it.data();
	else
		return QPtrList< cMakeSection >();
}

cMakeSection* cChar::lastSection( cMakeMenu* basemenu )
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

void cChar::setLastSection( cMakeMenu* basemenu, cMakeSection* data )
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

void cChar::clearLastSelections( void )
{
	lastselections_.clear();
}

void cChar::attackTarget( P_CHAR defender )
{
	if( this == defender || !defender || dead() || defender->dead() ) 
		return;

//	if (defender->pos().z > (attacker->pos().z +10)) return;//FRAZAI
//	if (defender->pos().z < (attacker->pos().z -10)) return;//FRAZAI

	//Coord_cl attpos( pos );
	//Coord_cl defpos( defender->pos );

	//attpos().z += 13; // eye height of attacker

	// I think this is nuts, it should be possible to go into combat even if 
	// there is something between us
	//if( !lineOfSight( attpos, defpos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING ) )
	//	return;

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
	
	if( isNpc() && npcaitype_ != 4 )
	{
		if ( !war_ )
			toggleCombat();

		setNextMoveTime();
	}

	// Check if the defender has pets defending him
	Followers guards = defender->guardedby();

	for( Followers::const_iterator iter = guards.begin(); iter != guards.end(); ++iter )
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
	if( defender->socket() )
	{
		QString message = tr( "You see %1 attacking you!" ).arg( name() );
		defender->socket()->showSpeech( this, message, 0x26, 3, cUOTxUnicodeSpeech::Emote );
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

void cChar::toggleCombat()
{
	war_ = !war_;
	Movement::instance()->CombatWalk( this );
}

P_ITEM cChar::rightHandItem() const
{
	return atLayer( SingleHandedWeapon );
}

P_ITEM cChar::leftHandItem() const
{
	return atLayer( DualHandedWeapon );
}

void cChar::applyPoison( P_CHAR defender )
{
	if( !defender )
		return;

	if( poison() && ( defender->poisoned() < poison() ) )
	{
		if( RandomNum( 0, 2 ) == 2 )
		{
			bool update = ( defender->poisoned() != 0 );

			defender->setPoisoned( poison() );

			// a lev.1 poison takes effect after 40 secs, a deadly pois.(lev.4) takes 40/4 secs - AntiChrist
			defender->setPoisontime( uiCurrentTime + ( MY_CLOCKS_PER_SEC*( 40 / defender->poisoned() ) ) ); 

			// wear off starts after poison takes effect - AntiChrist
			defender->setPoisonwearofftime( defender->poisontime() + ( MY_CLOCKS_PER_SEC*SrvParams->poisonTimer() ) ); 

			// Only update if flags have changed
			if( update )
				defender->update( false );

			if( defender->socket() )
				defender->socket()->sysMessage( tr("You have been poisoned!" ) );
		}
	}
}

UI16 cChar::calcDefense( enBodyParts bodypart, bool wearout )
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

bool cChar::checkSkill( UI16 skill, SI32 min, SI32 max, bool advance )
{
	bool success = false;
	
	if( dead_ )
	{
		if( socket_ )
			socket_->clilocMessage( 0x7A12C ); // You cannot use skills while dead.
		return false;
	}

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

void cChar::setSkillDelay() 
{ 	
	this->setSkillDelay( SetTimerSec(skilldelay_, SrvParams->skillDelay() ) );
}

void cChar::startRepeatedAction( UINT8 action, UINT16 delay )
{
	stopRepeatedAction();
	TempEffects::instance()->insert( new cRepeatAction( this, action, delay ) );
}

void cChar::stopRepeatedAction()
{
	TempEffects::instance()->dispel( this, this, "repeataction", false );
}

static void characterRegisterAfterLoading( P_CHAR pc )
{
	World::instance()->registerObject( pc );
	pc->setPriv2(pc->priv2() & 0xBF); // ???

	pc->setHidden( 0 );
	pc->setStealth( -1 );
	pc->SetSpawnSerial( pc->spawnSerial() );
	
	pc->setRegion( cAllTerritories::getInstance()->region( pc->pos().x, pc->pos().y, pc->pos().map ) );
	
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

bool cChar::onShowContext( cUObject *object )
{
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onShowContextMenu( (P_CHAR)this, object ) )
			return true;

	return false;
}

bool cChar::onShowTooltip( P_CHAR sender, cUOTxTooltipList* tooltip )
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

void cChar::addItem( cChar::enLayer layer, cItem* pi, bool handleWeight, bool noRemove )
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

void cChar::removeItem( cChar::enLayer layer, bool handleWeight )
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

cChar::ContainerContent cChar::content() const
{
	return content_;
}

cItem* cChar::atLayer( cChar::enLayer layer ) const
{
	ContainerContent::const_iterator it = content_.find(layer);
	if ( it != content_.end() )
		return it.data();
	return 0;
}

// Remove from current owner
void cChar::setOwner( P_CHAR data )
{
	// We CANT be our own owner
	if( data == this )
		return;

	if( owner_ )
	{
		owner_->removeFollower( this, true );
		tamed_ = false;
	}

	owner_ = data;
	changed( SAVE|TOOLTIP );

	if( owner_ )
	{
		owner_->addFollower( this, true );
		tamed_ = true;
	}
}

void cChar::addFollower( P_CHAR pPet, bool noOwnerChange )
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

void cChar::removeFollower( P_CHAR pPet, bool noOwnerChange )
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

cChar::Followers cChar::followers() const
{
	return followers_;
}

cChar::Followers cChar::guardedby() const
{
	return guardedby_;
}

void cChar::setGuarding( P_CHAR data )
{
	if( data == guarding_ )
		return;

	if( guarding_ )
		guarding_->removeGuard( this );

	guarding_ = data;
	changed( SAVE|TOOLTIP );

	if( guarding_ )
		guarding_->addGuard( this );		
}

void cChar::addGuard( P_CHAR pPet, bool noGuardingChange )
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

void cChar::removeGuard( P_CHAR pPet, bool noGuardingChange )
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

bool cChar::Owns( P_ITEM pItem ) const
{
	if( !pItem )
		return false;

	return ( pItem->owner() == this );
}

P_ITEM cChar::getWeapon() const
{
	// Check if we have something on our right hand
	P_ITEM rightHand = rightHandItem(); 
	if( Combat::weaponSkill( rightHand ) != WRESTLING )
		return rightHand;

	// Check for two-handed weapons
	P_ITEM leftHand = leftHandItem();
	if( Combat::weaponSkill( leftHand ) != WRESTLING )
		return leftHand;

	return 0;
}

void cChar::addEffect( cTempEffect *effect )
{
	effects_.push_back( effect );
}

void cChar::removeEffect( cTempEffect *effect )
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

void cChar::restock()
{
	P_ITEM pStock = atLayer( BuyRestockContainer );

	if( pStock )
	{
		cItem::ContainerContent container(pStock->content());
		cItem::ContainerContent::const_iterator it (container.begin());
		cItem::ContainerContent::const_iterator end(container.end());
		for (; it != end; ++it )
		{
			P_ITEM pItem = *it;
			if( pItem )
			{
				if( pItem->restock() < pItem->amount() )
				{
					UINT16 restock = QMAX( ( pItem->amount() - pItem->restock() ) / 2, 1 );
					pItem->setRestock( pItem->restock() + restock );
				}

				if( SrvParams->trade_system() )
				{					
					if( region_ )
						StoreItemRandomValue( pItem, region_->name() );
				}
			}
		}
	}
}

/*!
	Make someone criminal.
*/
void cChar::criminal()
{
	if( this->isGMorCounselor() )
		return;

	//Not an npc, not grey, not red	
	if( this->isPlayer() && !this->isCriminal() || this->isMurderer() )
	{ 
		 this->setCrimflag((SrvParams->crimtime()*MY_CLOCKS_PER_SEC)+uiCurrentTime);

		 if( this->socket() )
			 this->socket()->sysMessage( tr( "You are now a criminal!" ) );

		 // Update the highlight flag.
		 changed( SAVE );
	}
}

// Simple setting and getting of properties for scripts and the set command.
stError *cChar::setProperty( const QString &name, const cVariant &value )
{
	changed( SAVE|TOOLTIP );
	SET_INT_PROPERTY( "guildtype", GuildType )
	else SET_INT_PROPERTY( "guildtraitor", GuildTraitor )
	else SET_STR_PROPERTY( "orgname", orgname_ )
	else SET_STR_PROPERTY( "title", title_ )
	else SET_INT_PROPERTY( "unicode", unicode_ )
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
	else SET_INT_PROPERTY( "orgskin", orgskin_ )
	else SET_INT_PROPERTY( "xskin", xskin_ )
	else SET_INT_PROPERTY( "trackingtarget", trackingTarget_ )
	else SET_INT_PROPERTY( "creationday", creationday_ )
	else SET_INT_PROPERTY( "stealth", stealth_ )
	else SET_INT_PROPERTY( "running", running_ )
	else SET_INT_PROPERTY( "logout", logout_ )
	else SET_INT_PROPERTY( "clientidletime", clientidletime_ )
	else SET_INT_PROPERTY( "swingtarget", swingtarg_ )
	else SET_INT_PROPERTY( "holdgold", holdg_ )
	else SET_INT_PROPERTY( "flysteps", fly_steps_ )
	else SET_INT_PROPERTY( "tamed", tamed_ )
	else SET_INT_PROPERTY( "antispamtimer", antispamtimer_ )
	else SET_INT_PROPERTY( "antiguardstimer", antiguardstimer_ )
	else SET_CHAR_PROPERTY( "guarding", guarding_ )
	else SET_STR_PROPERTY( "carve", carve_ )
	else SET_INT_PROPERTY( "hair", hairserial_ )
	else SET_INT_PROPERTY( "beard", beardserial_ )
	else SET_INT_PROPERTY( "murderer", murdererSer_ )
	else SET_STR_PROPERTY( "spawnregion", spawnregion_ )
	else SET_INT_PROPERTY( "stablemaster", stablemaster_serial_ )
	else SET_INT_PROPERTY( "npctype", npc_type_ )
	else SET_INT_PROPERTY( "timeunused", time_unused_ )
	else SET_INT_PROPERTY( "timeusedlast", timeused_last_ )
	else SET_INT_PROPERTY( "casting", casting_ )
	else SET_INT_PROPERTY( "spawn", spawnserial_ )
	else SET_INT_PROPERTY( "hidden", hidden_ )
	else SET_INT_PROPERTY( "invistimeout", invistimeout_ )
	else SET_INT_PROPERTY( "attackfirst", attackfirst_ )
	else SET_INT_PROPERTY( "hunger", hunger_ )
	else SET_INT_PROPERTY( "hungertime", hungertime_ )
	else SET_INT_PROPERTY( "npcaitype", npcaitype_ )
	else SET_INT_PROPERTY( "poison", poison_ )
	else SET_INT_PROPERTY( "poisoned", poisoned_ )
	else SET_INT_PROPERTY( "poisontime", poisontime_ )
	else SET_INT_PROPERTY( "poisontxt", poisontxt_ )
	else SET_INT_PROPERTY( "poisonwearofftime", poisonwearofftime_ )
	else SET_INT_PROPERTY( "fleeat", fleeat_ )
	else SET_INT_PROPERTY( "reattackat", reattackat_ )
	else SET_INT_PROPERTY( "disabled", disabled_ )
	else SET_STR_PROPERTY( "disabledmsg", disabledmsg_ )
	else SET_INT_PROPERTY( "split", split_ )
	else SET_INT_PROPERTY( "splitchance", splitchnc_ )
	else SET_INT_PROPERTY( "ra", ra_ )
	else SET_INT_PROPERTY( "trainer", trainer_ )
	else SET_INT_PROPERTY( "trainingplayerin", trainingplayerin_ )
	else SET_INT_PROPERTY( "cantrain", cantrain_ )
	else SET_INT_PROPERTY( "guildtoggle", guildtoggle_ )
	else SET_INT_PROPERTY( "guildtitle", guildtitle_ )
	else SET_INT_PROPERTY( "guildfealty", guildfealty_ )
	else if( name == "guildstone" )
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
	}
	else SET_INT_PROPERTY( "flag", flag_ )
	else SET_INT_PROPERTY( "flagwearofftime", tempflagtime_ )
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
	else SET_INT_PROPERTY( "font", fonttype_ )
	else SET_INT_PROPERTY( "saycolor", saycolor_ )
	else SET_INT_PROPERTY( "emotecolor", emotecolor_ )
	else SET_INT_PROPERTY( "strength", st_ )
	else SET_INT_PROPERTY( "dexterity", dx )
	else SET_INT_PROPERTY( "intelligence", in_ )
	else SET_INT_PROPERTY( "strength2", st2_ )
	else SET_INT_PROPERTY( "dexterity2", dx2 )
	else SET_INT_PROPERTY( "intelligence2", in2_ )
	else SET_INT_PROPERTY( "maylevitate", may_levitate_ )
	else SET_INT_PROPERTY( "direction", dir_ )
	else SET_INT_PROPERTY( "xid", xid_ )
	else SET_INT_PROPERTY( "priv", priv )
	else SET_INT_PROPERTY( "priv2", priv2_ )
	else SET_INT_PROPERTY( "health", hp_ )
	else SET_INT_PROPERTY( "stamina", stm_ )
	else SET_INT_PROPERTY( "mana", mn_ )
	else SET_INT_PROPERTY( "mana2", mn2_ )
	else SET_INT_PROPERTY( "hidamage", hidamage_ )
	else SET_INT_PROPERTY( "lodamage", lodamage_ )
	else SET_INT_PROPERTY( "npc", npc_ )
	else SET_INT_PROPERTY( "shop", shop_ )
	else SET_INT_PROPERTY( "cell", cell_ )
	else SET_INT_PROPERTY( "jailtimer", jailtimer_ )
	else SET_INT_PROPERTY( "jailsecs", jailsecs_ )
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
	else SET_INT_PROPERTY( "making", making_ )
	else SET_INT_PROPERTY( "lasttarget", lastTarget_  )
	else SET_INT_PROPERTY( "direction2", dir2_ )
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

stError *cChar::getProperty( const QString &name, cVariant &value ) const
{
	GET_PROPERTY( "guildtype", GuildType )
	GET_PROPERTY( "guildtraitor", GuildTraitor )
	GET_PROPERTY( "orgname", orgname_ )
	GET_PROPERTY( "title", title_ )
	GET_PROPERTY( "unicode", unicode_ )
	GET_PROPERTY( "account", ( account_ != 0 ) ? account_->login() : QString( "" ) )
	GET_PROPERTY( "incognito", incognito_ )
	GET_PROPERTY( "polymorph", polymorph_ )
	GET_PROPERTY( "haircolor", haircolor_ )
	GET_PROPERTY( "hairstyle", hairstyle_ )
	GET_PROPERTY( "beardcolor", beardcolor_ )
	GET_PROPERTY( "beardstyle", beardstyle_ )
	GET_PROPERTY( "skin", skin_ )
	GET_PROPERTY( "orgskin", orgskin_ )
	GET_PROPERTY( "xskin", xskin_ )
	GET_PROPERTY( "trackingtarget", FindCharBySerial( trackingTarget_ ) )
	GET_PROPERTY( "creationday", (int)creationday_ )
	GET_PROPERTY( "stealth", stealth_ )
	GET_PROPERTY( "running", (int)running_ )
	GET_PROPERTY( "logout", (int)logout_ )
	GET_PROPERTY( "clientidletime", (int)clientidletime_ )
	GET_PROPERTY( "swingtarget", FindCharBySerial( swingtarg_ ) )
	GET_PROPERTY( "holdgold", (int)holdg_ )
	GET_PROPERTY( "flysteps", fly_steps_ )
	GET_PROPERTY( "tamed", tamed_ )
	GET_PROPERTY( "antispamtimer", (int)antispamtimer_ )
	GET_PROPERTY( "antiguardstimer", (int)antiguardstimer_ )
	GET_PROPERTY( "guarding", guarding_ )
	GET_PROPERTY( "carve", carve_ )
	GET_PROPERTY( "hair", FindItemBySerial( hairserial_ ) )
	GET_PROPERTY( "beard", FindItemBySerial( beardserial_ ) )
	GET_PROPERTY( "murderer", FindCharBySerial( murdererSer_ ) )
	GET_PROPERTY( "spawnregion", spawnregion_ )
	GET_PROPERTY( "stablemaster", FindCharBySerial( stablemaster_serial_ ) )
	GET_PROPERTY( "npctype", npc_type_ )
	GET_PROPERTY( "timeunused", (int)time_unused_ )
	GET_PROPERTY( "timeusedlast", (int)timeused_last_ )
	GET_PROPERTY( "casting", casting_ )
	GET_PROPERTY( "spawn", FindItemBySerial( spawnserial_ ) )
	GET_PROPERTY( "hidden", hidden_ )
	GET_PROPERTY( "invistimeout", (int)invistimeout_ )
	GET_PROPERTY( "attackfirst", attackfirst_ )
	GET_PROPERTY( "hunger", hunger_ )
	GET_PROPERTY( "hungertime", (int)hungertime_ )
	GET_PROPERTY( "npcaitype", npcaitype_ )
	GET_PROPERTY( "poison", poison_ )
	GET_PROPERTY( "poisoned", (int)poisoned_ )
	GET_PROPERTY( "poisontime", (int)poisontime_ )
	GET_PROPERTY( "poisontxt", (int)poisontxt_ )
	GET_PROPERTY( "poisonwearofftime", (int)poisonwearofftime_ )
	GET_PROPERTY( "fleeat", fleeat_ )
	GET_PROPERTY( "reattackat", reattackat_ )
	GET_PROPERTY( "split", split_ )
	GET_PROPERTY( "splitchance", splitchnc_ )
	GET_PROPERTY( "ra", ra_ )
	GET_PROPERTY( "trainer", FindCharBySerial( trainer_ ) )
	GET_PROPERTY( "trainingplayerin", trainingplayerin_ )
	GET_PROPERTY( "cantrain", cantrain_ )
	GET_PROPERTY( "guildtoggle", guildtoggle_ )
	GET_PROPERTY( "guildtitle", guildtitle_ )
	GET_PROPERTY( "guildfealty", guildfealty_ )
	GET_PROPERTY( "guildstone", FindItemBySerial( guildstone_ ) )
	GET_PROPERTY( "flag", flag_ )
	GET_PROPERTY( "flagwearofftime", (int)tempflagtime_ )
	GET_PROPERTY( "murderrate", (int)murderrate_ )
	GET_PROPERTY( "crimflag", crimflag_ )
	GET_PROPERTY( "squelched", squelched_ )
	GET_PROPERTY( "mutetime", (int)mutetime_ )
	GET_PROPERTY( "meditating", med_ )
	GET_PROPERTY( "weight", weight_ )
	GET_PROPERTY( "stones", weight_ / 10 )
	GET_PROPERTY( "lootlist", loot_ )
	GET_PROPERTY( "font", fonttype_ )
	GET_PROPERTY( "saycolor", saycolor_ )
	GET_PROPERTY( "emotecolor", emotecolor_ )
	GET_PROPERTY( "strength", st_ )
	GET_PROPERTY( "dexterity", dx )
	GET_PROPERTY( "intelligence", in_ )
	GET_PROPERTY( "strength2", st2_ )
	GET_PROPERTY( "dexterity2", dx2 )
	GET_PROPERTY( "intelligence2", in2_ )
	GET_PROPERTY( "maylevitate", may_levitate_ )
	GET_PROPERTY( "direction", dir_ )
	GET_PROPERTY( "xid", xid_ )
	GET_PROPERTY( "priv", priv )
	GET_PROPERTY( "priv2", priv2_ )
	GET_PROPERTY( "health", hp_ )
	GET_PROPERTY( "stamina", stm_ )
	GET_PROPERTY( "mana", mn_ )
	GET_PROPERTY( "mana2", mn2_ )
	GET_PROPERTY( "hidamage", hidamage_ )
	GET_PROPERTY( "lodamage", lodamage_ )
	GET_PROPERTY( "npc", npc_ )
	GET_PROPERTY( "shop", shop_ )
	GET_PROPERTY( "cell", cell_ )
	GET_PROPERTY( "jailtimer", (int)jailtimer_ )
	GET_PROPERTY( "jailsecs", jailsecs_ )
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
	GET_PROPERTY( "making", making_ )
	if( name == "lasttarget" )
	{
		if( isCharSerial( lastTarget_ ) )
			value = cVariant( FindCharBySerial( lastTarget_ ) );
		else
			value = cVariant( FindItemBySerial( lastTarget_ ) );

		return 0;
	}
	GET_PROPERTY( "direction2", dir2_ )
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

void cChar::playDeathSound()
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

void cChar::setSkillValue( UINT16 skill, UINT16 value )
{
	skills[ skill ].value = value;

	// Check if we can delete the current skill
	const stSkillValue &skValue = skills[ skill ];

//	if( skValue.cap == 1000 && skValue.lock == 0 && skValue.value == 0 )
//		skills.remove( skill );

	changed( SAVE | TOOLTIP );
}

void cChar::setSkillCap( UINT16 skill, UINT16 cap )
{
	skills[ skill ].cap = cap;

	// Check if we can delete the current skill
//	const stSkillValue &skValue = skills[ skill ];

//	if( skValue.cap == 1000 && skValue.lock == 0 && skValue.value == 0 )
//		skills.remove( skill );

	changed( SAVE | TOOLTIP );
}

void cChar::setSkillLock( UINT16 skill, UINT8 lock )
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

UINT16 cChar::skillValue( UINT16 skill ) const
{
	return skills[ skill ].value;

//	if( skValue == skills.end() )
//		return 0;
}

UINT16 cChar::skillCap( UINT16 skill ) const
{
	return skills[ skill ].cap;

//	if( skValue == skills.end() )
//		return 1000;
}

UINT8 cChar::skillLock( UINT16 skill ) const
{
	return skills[ skill ].lock;

//	if( skValue == skills.end() )
//		return 0;
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

void cChar::setHairColor( unsigned short d)	
{ 
	haircolor_ = d; 
	changed( SAVE );
	cItem* pHair = GetItemOnLayer( 11 );
	if( pHair )
		pHair->setColor( d );
	pHair->update();
	resend();
}

void cChar::setHairStyle( unsigned short d)	
{ 
	if( !isHair( d ) )
		return;
	hairstyle_ = d; 
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
		pHair->setColor( haircolor_ );
		addItem( cChar::Hair, pHair );
	}
	pHair->update();
	resend();
}

void cChar::setBeardColor( unsigned short d)	
{ 
	beardcolor_ = d; 
	changed( SAVE );
	cItem* pBeard = GetItemOnLayer( 16 );
	if( pBeard )
		pBeard->setColor( d );
	pBeard->update();
	resend();
}

void cChar::setBeardStyle( unsigned short d)	
{ 
	if( !isBeard( d ) )
		return;
	beardstyle_ = d; 
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
		pBeard->setColor( beardcolor_ );
		addItem( cChar::FacialHair, pBeard );
	}
	pBeard->update();
	resend();
}

/*!
	If this character is in a guarded area, it checks the surroundings for criminals
	or murderers and spawns a guard if one is found.
*/
void cChar::callGuards()
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

/*!
	Queries if the character is an online player
*/
bool cChar::online() const
{
	if ( socket() && socket()->state() == cUOSocket::InGame )
		return true;
	return false;
}

Coord_cl cChar::nextMove()
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

void cChar::pushMove( UI16 x, UI16 y, SI08 z )
{
	path_.push_back( Coord_cl( x, y, z, 0 ) );
}

void cChar::pushMove( const Coord_cl &move )
{
	path_.push_back( move );
}

void cChar::popMove( void )
{
	path_.pop_front();
}

void cChar::clearPath( void )
{
	path_.clear();
}

bool cChar::hasPath( void )
{
	return !path_.empty();
}

Coord_cl cChar::pathDestination( void ) const
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
	We use simple 3-dim. euclid distance: d = sqrt( |x1-x2|² + |y1-y2|² + |z1-z2|² )
*/
float cChar::pathHeuristic( const Coord_cl &source, const Coord_cl &destination )
{
	return (float)( sqrt( pow( abs(source.x - destination.x), 2 ) + pow( abs(source.y - destination.y), 2 ) + pow( abs(source.z - destination.z) / 5, 2 ) ) );
}

/*!
	The algorithm..
	currently works in x,y,z direction. no idea how to implement map jumping yet.
*/
void cChar::findPath( const Coord_cl &goal, float sufficient_cost = 0.0f )
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