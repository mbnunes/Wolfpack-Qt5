//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
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
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

// Platform include
#include "platform.h"


// Wolfpack includes
#include "accounts.h"
#include "wpdefaultscript.h"
#include "chars.h"
#include "charsmgr.h"
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
#include "mapstuff.h"
#include "skills.h"
#include "wpdefmanager.h"
#include "guildstones.h"
#include "network/asyncnetio.h"
#include "walking.h"

#undef  DBGFILE
#define DBGFILE "chars.cpp"

bool cChar::Owns(P_ITEM pi)				{	return (serial==pi->ownserial);		}
bool cChar::Wears(P_ITEM pi)			{	return (serial == pi->contserial);	}
unsigned int cChar::dist(cChar* pc)		{	return pos.distance(pc->pos);		}
unsigned int cChar::dist(cItem* pi)		{	return pos.distance(pi->pos);		}
QString cChar::objectID() const			{	return "CHARACTER";					}

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
		pCont->AddItem( pile );
		total -= pile->amount();
	}

	if( socket_ )
		goldsfx( socket_, amount, false );
}

void cChar::setSerial(SERIAL ser)
{
	this->serial = ser;
	if ( this->serial != INVALID_SERIAL)
		cCharsManager::getInstance()->registerChar(this);
}

void cChar::Init(bool ser)
{
	VisRange = VISRANGE ;
	unsigned int i;

	if (ser)
	{
		this->setSerial(cCharsManager::getInstance()->getUnusedSerial());
	}
	else
	{
		this->serial = INVALID_SERIAL;
	}

	this->animated = false;
	this->multis=-1;//Multi serial
	this->free = false;
	this->name = "Mr. noname";
	this->setOrgname("Mr. noname");
	this->title_ = "";
	this->socket_ = 0;
	this->setAntispamtimer(0);//LB - anti spam

	this->setUnicode(true); // This is set to 1 if the player uses unicode speech, 0 if not
	this->pos.x=100;
	this->pos.y=100;
	this->dispz_ = this->pos.z = 0;	
//	this->oldpos.x=0; // fix for jail bug
//	this->oldpos.y=0; // fix for jail bug
//	this->oldpos.z=0; // LB, experimental, change back to unsignbed if this give sproblems
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
	for (i=0;i<TRUESKILLS;i++)
	{
		this->setBaseSkill(i, 0);
		this->setSkill(i, 0);
	}
	this->npc_=false;
	this->shop_=false; //1=npc shopkeeper
	this->cell_=0; // Reserved for jailing players 
	            // bugfix, LB 0= player not in jail !, not -1
	
	this->jailtimer_=0; //blackwinds jail system
	this->jailsecs_=0;

	this->ownserial_=INVALID_SERIAL; // If Char is an NPC, this sets its owner
	this->setTamed(false); // True if NPC is tamed
	this->robe_ = -1; // Serial number of generated death robe (If char is a ghost)
	this->karma_ = 0;
	this->fame_ = 0;
	this->pathnum_=PATHNUM;
	this->kills_ = 0; // PvP Kills
	this->deaths_ = 0;
	this->dead_ = false; // Is character dead
	this->packitem_ = INVALID_SERIAL; // Only used during character creation
	this->fixedlight_ = 255; // Fixed lighting level (For chars in dungeons, where they dont see the night)
	// changed to -1, LB, bugfix
	this->speech_ = 0; // For NPCs: Number of the assigned speech block
	this->setWeight( 0 );
	this->def_ = 0; // Intrinsic defense
	this->war_ = false; // War Mode
	this->targ_=INVALID_SERIAL; // Current combat target
	this->timeout_=0; // Combat timeout (For hitting)
	this->timeout2_=0;
	this->regen_=0;
	this->regen2_=0;
	this->regen3_=0;//Regeneration times for mana, stamin, and str
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
	this->setOnHorse(false); // On a horse?
	this->setHunger(6);  // Level of hungerness, 6 = full, 0 = "empty"
	this->setHungerTime(0); // Timer used for hunger, one point is dropped every 20 min
	this->setTailItem( INVALID_SERIAL );
	this->setNpcAIType(0); // NPC ai
	this->setCallNum(-1); //GM Paging
	this->setPlayerCallNum(-1); //GM Paging
	this->region=NULL;
	this->skilldelay=0;
	this->objectdelay=0;
	this->making=-1; // skill number of skill using to make item, 0 if not making anything.
	this->blocked=0;
	this->dir2=0;
	this->spiritspeaktimer=0; // Timer used for duration of spirit speak
	this->spattack=0;
	this->spadelay=0;
	this->spatimer=0;
	this->taming=0; //Skill level required for taming
	this->summontimer=0; //Timer for summoned creatures.
	this->trackingTimer_ = 0; // Timer used for the duration of tracking
	this->trackingTarget_ = INVALID_SERIAL;
	this->setFishingtimer(0); // Timer used to delay the catching of fish

	this->setAdvobj(0); //Has used advance gate?
	
	this->setPoison(0); // used for poison skill 
	this->setPoisoned(0); // type of poison
	this->setPoisontime(0); // poison damage timer
	this->setPoisontxt(0); // poision text timer
	this->setPoisonwearofftime(0); // LB, makes poision wear off ...
	
	this->setFleeat(SrvParams->npc_base_fleeat());
	this->setReattackat(SrvParams->npc_base_reattackat());
	this->setDisabled(0); //Character is disabled for n cicles, cant trigger.
	this->setDisabledmsg(QString::null); //Character disabled message. -- by Magius(CHE) §
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
	this->setGuildstone(INVALID_SERIAL);		// GuildStone Serial of guild player is in (INVALID_SERIAL=no guild)			(DasRaetsel)
	this->GuildTraitor=false; 
	//this->flag=0x04; //1=red 2=grey 4=Blue 8=green 10=Orange
	this->setCriminal(); //flags = 0x2; 1=red 2=grey 4=Blue 8=green 10=Orange // grey as default - AntiChrist
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
	this->setMenupriv(0); // Lb -> menu priv
	this->setGuarded(false); // True if CHAR is guarded by some NPC
	this->setSmokeTimer(0);
	this->setSmokeDisplayTimer(0);
	this->setCarve( (char*)0 ); // carving system
	this->setAntiguardstimer(0); // AntiChrist - for "GUARDS" call-spawn
	this->setPolymorph(false);//polymorph - AntiChrist
	this->setIncognito(false);//incognito - AntiChrist
    this->setPostType(LOCALPOST);
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

	for (i=0;i<TRUESKILLS;i++)
	{
		this->setBaseSkill(i, 0);
		this->setSkill(i, 0);
	}
	for (i = 0; i < ALLSKILLS; i++) 
		this->lockSkill[i]=0;

	this->setFood( 0 );
}

///////////////////////
// Name:	GetItemOnLayer
// history:	by Duke, 26.3.2001
// Purpose:	returns the item on the given layer, if any

P_ITEM cChar::GetItemOnLayer(unsigned char layer)
{
	P_ITEM pi;
	unsigned int ci;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if ( pi != NULL)
		{
			if( pi->layer() == layer )
				return pi;
		}
	}
	return NULL;
}

///////////////////////
// Name:	GetItemOnLayer
// history:	by Duke, 26.3.2001, touched by Correa, 21.04.2001
// Purpose:	Return the bank box. If banktype == 1, it will return the Item's bank box, else, 
//          gold bankbox is returned. 

P_ITEM cChar::getBankBox( void )
{
	P_ITEM pi;
	unsigned int ci=0;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if( pi && pi->layer() == 0x1D )
			return pi;
	}

	// If we reach this point, bankbox wasn't found == wasn't created yet.
	sprintf((char*)temp, "%s's bank box.", name.c_str());
	UOXSOCKET s = calcSocketFromChar(this);
	pi = Items->SpawnItem(this, 1, (char*)temp, 0, 0x09AB, 0, 0);
	if(pi == NULL) 
		return NULL;
	pi->setLayer( 0x1D );
	pi->SetOwnSerial(this->serial);
	pi->setContSerial(this->serial);
	pi->morex=1;
	pi->setType( 1 );
	if (s != -1)
		wearIt(s, pi);

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
		
		if( socket() )
			socket()->updatePlayer();

		if( isGM() )
			tempeffect(this, this, 34, 3, 0, 0); 
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
	this->targ_ = other->serial;
	this->unhide();
	this->disturbMed();	// Meditation
	this->attacker_ = other->serial;
	if (this->isNpc())
	{

		if (!this->war_)
			toggleCombat();

		this->setNextMoveTime();
	}
}


cChar::cChar():
	socket_(0), account_(0)
{
	VisRange = VISRANGE;
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
	vector< SERIAL > equipment = contsp.getData( serial );

	for( UINT32 i = 0; i < equipment.size(); ++i )
	{
		P_ITEM pItem = FindItemBySerial( equipment[i] );

		if( !pItem )
			continue;

		if( ( pItem->id() == ID ) && ( pItem->color() == col ) )
			number++;
	}

	P_ITEM pi = getBackpack();
	
	if( pi )
		number = pi->CountItems( ID, col );
	return number ;
}

int cChar::CountBankGold()
{
	P_ITEM pi = getBankBox(); //we want gold bankbox.
	return pi->CountItems( 0x0EED );
}

void cChar::openBank( UOXSOCKET socket )
{
	// Send to ourself ?
	if( socket == INVALID_UOXSOCKET )
	{
		if( socket_ )
			socket_->sendContainer( getBankBox() );
	}
	else
	{
		// Send it to the socket
		/*P_ITEM bankBox = getBankBox();
		backpack( socket, bankBox->serial );*/
	}
}

///////////////////////
// Name:	CountItems
// history:	by Duke, 13.5.2001
// Purpose:	assigns the halo of the given item to a character

void cChar::addHalo(P_ITEM pi)
{
	glowsp.insert(this->serial, pi->serial);
}

void cChar::removeHalo(P_ITEM pi)
{
	glowsp.remove(this->serial, pi->serial);
}

void cChar::glowHalo(P_ITEM pi)
{
	if (pi->glow != INVALID_SERIAL)
	{
		P_ITEM pHalo=FindItemBySerial(pi->glow);
		if (!pHalo) return;
		
		pHalo->setLayer( pi->layer() ); // copy layer information of the glowing item to the invisible light emitting object
		
		if(pHalo->layer()==0 && pi->isInWorld()) // unequipped -> light source coords = item coords
		{
			pHalo->dir=29;
			pHalo->moveTo( pi->pos );
		} else if (pHalo->layer() == 0 && !pi->isInWorld()) // euqipped -> light source coords = players coords
		{
			pHalo->pos.x=this->pos.x;
			pHalo->pos.y=this->pos.y;
			pHalo->pos.z=this->pos.z+4;
			pHalo->dir=99; // gives no light in backpacks
		} else
		{
			pHalo->pos.x=this->pos.x;
			pHalo->pos.y=this->pos.y;
			pHalo->pos.z=this->pos.z+4;
			pHalo->dir=29;
		}

		pHalo->update();//AntiChrist
	}
}

bool cChar::hasWeapon()
{
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for( UINT32 ci = 0; ci < vecContainer.size(); ++ci )
	{
		P_ITEM pi = FindItemBySerial( vecContainer[ci] );
		if( pi && ( ( pi->layer() == 1 && pi->type() != 9 ) || ( pi->layer() == 2 && !IsShield( pi->id() ) ) ) )
			return true;
	}

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

P_ITEM Packitem(P_CHAR pc) // Find packitem
{
	if(pc == NULL) 
		return NULL;
	P_ITEM pi = FindItemBySerial(pc->packitem());
	if (pi != NULL)
	{
		if (pc->Wears(pi) && pi->layer()==0x15)
		{
			return pi;
		}
	}

	// - For some reason it's not defined, so go look for it.
	unsigned int ci = 0;
	vector<SERIAL> vecContainer = contsp.getData(pc->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		P_ITEM pi = FindItemBySerial(vecContainer[ci]);
		if (pi != NULL && pi->layer()==0x15)
		{
			pc->setPackItem( pi->serial );	//Record it for next time
			return (pi);
		}
	}
	return NULL;
}

P_ITEM cChar::getBackpack()	
{
	P_ITEM backpack = FindItemBySerial( packitem_ );

	// None found so create one
	if( !backpack )
	{
		backpack = Items->SpawnItem( this, 1, "#", false, 0xE75, 0x0000, false );
		if( backpack )
		{
			backpack->setLayer( 0x15 );
			backpack->setOwner( this );
			backpack->setContSerial( serial );
			backpack->setType( 1 );
			backpack->update();
			packitem_ = backpack->serial;
		}
	}

	return backpack;
}

///////////////////////
// Name:	setters for various serials
// history:	by Duke, 2.6.2001
// Purpose:	encapsulates revoval/adding to the pointer arrays

void cChar::setOwnSerialOnly(long ownser)
{
	ownserial_=ownser;
}

void cChar::SetOwnSerial(long ownser)
{
	if (ownserial_ != INVALID_SERIAL)	// if it was set, remove the old one
		cownsp.remove(ownserial_, serial);
	
	setOwnSerialOnly(ownser);
	if (ownser != serial && ownser != INVALID_SERIAL)
		tamed_ = true;
	else
		tamed_ = false;

	if (ownser != INVALID_SERIAL)		// if there is an owner, add it
		cownsp.insert(ownserial_, serial);
}

void cChar::SetSpawnSerial(long spawnser)
{
	if (spawnSerial() != INVALID_SERIAL)	// if it was set, remove the old one
		cspawnsp.remove(spawnSerial(), serial);

	spawnserial_ = spawnser;

	if (spawnser != INVALID_SERIAL)		// if there is a spawner, add it
		cspawnsp.insert(spawnserial_, serial);
}

void cChar::SetMultiSerial(long mulser)
{
	this->multis = mulser;
}

void cChar::MoveToXY(short newx, short newy)
{
	this->MoveTo(newx,newy,pos.z);	// keep the old z value
}

void cChar::MoveTo(short newx, short newy, signed char newz)
{
	// Avoid crash if go to 0,0
	if (newx < 1 || newy < 1)
		return;

	cMapObjects::getInstance()->remove(this);
	pos.x = newx;
	pos.y = newy;
	setDispz( newz );
	pos.z = dispz_;
	cMapObjects::getInstance()->add(this);
}

unsigned int cChar::getSkillSum()
{
	unsigned int sum = 0, a = 0;
	for (; a < ALLSKILLS; ++a)
	{
		sum += this->baseSkill_[a];
	}
	return sum;		// this *includes* the decimal digit ie. xxx.y
}

///////////////////////
// Name:	getTeachingDelta
// history:	by Duke, 27.7.2001
// Purpose:	calculates how much the given player can learn from this teacher

int cChar::getTeachingDelta(cChar* pPlayer, int skill, int sum)
{
	int delta = QMIN(250,this->baseSkill(skill)/2);		// half the trainers skill, but not more than 250
	delta -= pPlayer->baseSkill(skill);					// calc difference
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
//	this->st -= pi->st2;
	this->setSt( ( this->st() ) - pi->st2);
	this->chgDex(-1 * pi->dx2);
	this->in_ -= pi->in2;
}

////////////
// Name:	canPickUp
// history:	by Duke, 20.9.2001
// Purpose:	checks if the char can drag the item

bool cChar::canPickUp(cItem* pi)
{
	if (!pi)
	{
		LogCritical("cChar::canPickUp() - bad parm");
		return false;
	}

	if (this->priv2_&1)	// allmove
		return true;

	if ( (pi->isOwnerMovable() || pi->isLockedDown()) && !this->Owns(pi) )	// owner movable or locked down ?
		return false;

	tile_st tile = cTileCache::instance()->getTile( pi->id() );
	if ( pi->isGMMovable() || (tile.weight == 255 && !pi->isAllMovable()))
		return false;
	return true;
}

void cChar::Serialize(ISerialization &archive)
{
	if (archive.isReading())
	{
		archive.read("name",			orgname_);
		archive.read("title",			title_);
		QString login;
		archive.read("account",			login);
		setAccount( Accounts->getRecord( login ) );
		archive.read("creationday",		creationday_);
		archive.read("gmmoveeff",		gmMoveEff);
		archive.read("guildtype",		GuildType);
		archive.read("guildtraitor",	GuildTraitor);
		archive.read("dispz",			dispz_ );
		archive.read("cell",			cell_);
		archive.read("dir",				dir_);
		archive.read("body",			xid_);	setId(xid_);
		archive.read("xbody",			xid_);
		archive.read("skin",			skin_);	
		archive.read("xskin",           xskin_);
		archive.read("priv",			priv);
		
		archive.read("stablemaster",	stablemaster_serial_);
		archive.read("npctype",			npc_type_);
		archive.read("time_unused",		time_unused_);
		
		archive.read("allmove",			priv2_);
		archive.read("font",			fonttype_);
		archive.read("say",				saycolor_);
		archive.read("emote",			emotecolor_);
		archive.read("strength",		st_);
		archive.read("strength2",		st2_);
		archive.read("dexterity",		dx);
		archive.read("dexterity2",		dx2);
		archive.read("intelligence",	in_);
		archive.read("intelligence2",	in2_);
		archive.read("hitpoints",		hp_);
		archive.read("spawnregion",		spawnregion_);
		archive.read("stamina",			stm_);
		archive.read("mana",			mn_);
		archive.read("npc",				npc_);
		archive.read("holdgold",		holdg_);
		archive.read("shop",			shop_);
		archive.read("own",				ownserial_);
		archive.read("robe",			robe_);
		archive.read("karma",			karma_);
		archive.read("fame",			fame_);
		archive.read("kills",			kills_);
		archive.read("deaths",			deaths_);
		archive.read("dead",			dead_);
		archive.read("packitem",		packitem_);
		archive.read("fixedlight",		fixedlight_);
		archive.read("speech",			speech_);

		archive.read("disablemsg",		disabledmsg_);
		register unsigned int j;
		for ( j = 0; j < TRUESKILLS; j++ )
		{
			archive.read( (char*)QString( "skill%1" ).arg( j ).latin1(), baseSkill_[j] );
			archive.read( (char*)QString( "skl%1" ).arg( j ).latin1(), lockSkill[j] );
		}
		archive.read("cantrain",		cantrain_);
		
		archive.read("def",				def_);
		archive.read("lodamage",		lodamage_);
		archive.read("hidamage",		hidamage_);
		archive.read("war",				war_);
		archive.read("npcwander",		npcWander_);
		archive.read("oldnpcwander",	oldnpcWander_);
		archive.read("carve",			carve_);
		archive.read("fx1",				fx1_);
		archive.read("fy1",				fy1_);
		archive.read("fz1",				fz1_);
		archive.read("fx2",				fx2_);
		archive.read("fy2",				fy2_);
		archive.read("spawn",			spawnserial_);
		archive.read("hidden",			hidden_);
		archive.read("hunger",			hunger_);
		archive.read("npcaitype",		npcaitype_);
		archive.read("spattack",		spattack);
		archive.read("spadelay",		spadelay);
		archive.read("taming",			taming);

		archive.read("summontimer",		summontimer);
		if (summontimer != 0)
			summontimer += uiCurrentTime;
		archive.read("advobj",			advobj_);
		archive.read("poison",			poison_);
		archive.read("poisoned",		poisoned_);
		archive.read("fleeat",			fleeat_);
		archive.read("reattackat",		reattackat_);
		archive.read("split",			split_);
		archive.read("splitchance",		splitchnc_);
		// Begin of Guild related things (DasRaetsel)
		archive.read("guildtoggle",		guildtoggle_);  
		archive.read("guildstone",		guildstone_);  
		archive.read("guildtitle",		guildtitle_);  
		archive.read("guildfealty",		guildfealty_);  
		archive.read("murderrate",		murderrate_);
		archive.read("menupriv",		menupriv_);
		archive.read("questtype",		questType_);
		archive.read("questdestregion",	questDestRegion_);
		archive.read("questorigregion",	questOrigRegion_);
		archive.read("questbountypostserial", questBountyPostSerial_);
		archive.read("questbountyreward", questBountyReward_);
		archive.read("jailtimer",		jailtimer_);
		if (jailtimer_ != 0)
			jailtimer_ += uiCurrentTime;
		archive.read("jailsecs",		jailsecs_); 
		archive.read("lootlist",		loot_ );
		archive.read("food",			food_ );
		SetOwnSerial(ownserial_);
		SetSpawnSerial(spawnserial_);
		setAccount( account_ );
	}
	else if ( archive.isWritting())
	{
		if(incognito())
		{//save original name
			archive.write("name", orgname());
		} 
		else
		{
			archive.write("name", name);
		}

		archive.write("title",			title_);
		if( account_ )
			archive.write( "account", account_->login() );
		else
			archive.write( "account", QString( "" ) );
		archive.write("creationday",	creationday_);
		archive.write("gmmoveeff",		gmMoveEff);
		archive.write("guildtype",		GuildType);
		archive.write("guildtraitor",	GuildTraitor);
		archive.write("dispz",			dispz_);
		archive.write("cell",			cell_);
		archive.write("dir",			dir_);
		//AntiChrist - incognito and polymorph spell special stuff - 12/99
		if(incognito() || polymorph())
		{//if under incognito spell, don't save BODY but the original XBODY
			archive.write("body", xid_);
		} 
		else
		{//else backup body normally
			archive.write("body", id());
		}
		archive.write("xbody", xid_);
		//AntiChrist - incognito spell special stuff - 12/99
		if(incognito())
		{//if under incognito spell, don't save SKIN but the original XSKIN
			archive.write("skin", xskin_);
		} 
		else
		{//else backup skin normally
			archive.write("skin", skin_);
		}
		
		archive.write("xskin",			xskin_);
		archive.write("priv",			priv);
		
		archive.write("stablemaster",	stablemaster_serial_);
		archive.write("npctype",		npc_type_);
		archive.write("time_unused",	time_unused_);
		
		archive.write("allmove",		priv2_);
		archive.write("font",			fonttype_);
		archive.write("say",			saycolor_);
		archive.write("emote",			emotecolor_);
		archive.write("strength",		st_);
		archive.write("strength2",		st2_);
		archive.write("dexterity",		dx);
		archive.write("dexterity2",		dx2);
		archive.write("intelligence",	in_);
		archive.write("intelligence2",	in2_);
		archive.write("hitpoints",		hp_);
		archive.write("spawnregion",	spawnregion_);
		archive.write("stamina",		stm_);
		archive.write("mana",			mn_);
		archive.write("npc",			npc_);
		archive.write("holdgold",		holdg_);
		archive.write("shop",			shop_);
		archive.write("own",			ownserial_);
		archive.write("robe",			robe_);
		archive.write("karma",			karma_);
		archive.write("fame",			fame_);
		archive.write("kills",			kills_);
		archive.write("deaths",			deaths_);
		archive.write("dead",			dead_);
		archive.write("packitem",		packitem_);
		archive.write("fixedlight",		fixedlight_);
		archive.write("speech",			speech_);
		archive.write("disablemsg",		disabledmsg_);
		register unsigned int j;
		for( j = 0; j < TRUESKILLS; j++ )
		{
			archive.write( (char*)QString( "skill%1" ).arg( j ).latin1(), baseSkill_[j] );
			archive.write( (char*)QString( "skl%1" ).arg( j ).latin1(), lockSkill[j] );
		}
		archive.write("cantrain", cantrain_);
		
		archive.write("def",			def_);
		archive.write("lodamage",		lodamage_);
		archive.write("hidamage",		hidamage_);
		archive.write("war",			war_);
		archive.write("npcwander",		npcWander_);
		archive.write("oldnpcwander",	oldnpcWander_);
		archive.write("carve",			carve_);
		archive.write("fx1",			fx1_);
		archive.write("fy1",			fy1_);
		archive.write("fz1",			fz1_);
		archive.write("fx2",			fx2_);
		archive.write("fy2",			fy2_);
		archive.write("spawn",			spawnserial_);
		archive.write("hidden",			hidden_);
		archive.write("hunger",			hunger_);
		archive.write("npcaitype",		npcaitype_);
		archive.write("spattack",		spattack);
		archive.write("spadelay",		spadelay);
		archive.write("taming",			taming);
		unsigned int summtimer = summontimer-uiCurrentTime;
		archive.write("summonremainingseconds", summtimer);
		
		archive.write("advobj",			advobj_);
		archive.write("poison",			poison_);
		archive.write("poisoned",		poisoned_);
		archive.write("fleeat",			fleeat_);
		archive.write("reattackat",		reattackat_);
		archive.write("split",			split_);
		archive.write("splitchance",	splitchnc_);
		// Begin of Guild related things (DasRaetsel)
		archive.write("guildtoggle",	guildtoggle_);  
		archive.write("guildnumber",	guildstone_);  
		archive.write("guildtitle",		guildtitle_);  
		archive.write("guildfealty",	guildfealty_);  
		archive.write("murderrate",		murderrate_);
		archive.write("menupriv",		menupriv_);
		archive.write("questtype",		questType_);
		archive.write("questdestregion",questDestRegion_);
		archive.write("questorigregion",questOrigRegion_);
		archive.write("questbountypostserial", questBountyPostSerial_);
		archive.write("questbountyreward", questBountyReward_);
		unsigned int jtimer = jailtimer_-uiCurrentTime;
		archive.write("jailtimer",		jtimer); 
		archive.write("jailsecs",		jailsecs_); 
		archive.write("lootlist",		loot_);
		archive.write("food",			food_);
	}
	cUObject::Serialize(archive);
}

//========== WRAPPER EVENTS

// Shows the name of a character to someone else
bool cChar::onShowCharName( P_CHAR Viewer ) 
{
	if( scriptChain.empty() )
		return false;
 
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onShowCharName( (P_CHAR)this, (P_CHAR)Viewer ) )
			return true;

	return false;
}

// Walks in a specific Direction
bool cChar::onWalk( UI08 Direction, UI08 Sequence )
{
	if( scriptChain.empty() )
		return false;
 
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onWalk( (P_CHAR)this, Direction, Sequence ) )
			return true;

	return false;
}

// The character says something
bool cChar::onTalk( char speechType, UI16 speechColor, UI16 speechFont, const QString &Text, const QString &Lang )
{
	if( scriptChain.empty() )
		return false;
 
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onTalk( (P_CHAR)this, speechType, speechColor, speechFont, Text, Lang ) )
			return true;

	return false;
}

// Someone talks to the NPC, this is only triggered for the npc
bool cChar::onTalkToNPC( P_CHAR Talker, const QString &Text )
{
	if( scriptChain.empty() )
		return false;
 
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onTalkToNPC( (P_CHAR)this, Talker, Text ) )
			return true;

	return false;
}

// The character switches warmode
bool cChar::onWarModeToggle( bool War )
{
	if( scriptChain.empty() )
		return false;
 
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onWarModeToggle( this, War ) )
			return true;

	return false;
}

// The character enters the world
bool cChar::onEnterWorld( void )
{
	if( scriptChain.empty() )
		return false;
 
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onEnterWorld( this ) )
			return true;

	return false;
}

// The character wants help
bool cChar::onHelp( void )
{
	if( scriptChain.empty() )
		return false;
 
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onHelp( this ) )
			return true;

	return false;
}

// The character wants to chat
bool cChar::onChat( void )
{
	if( scriptChain.empty() )
		return false;
 
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onChat( this ) )
			return true;

	return false;
}

// The character uses %Skill
bool cChar::onSkillUse( UI08 Skill ) 
{
	if( scriptChain.empty() )
		return false;
 
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onSkillUse( this, Skill ) )
			return true;

	return false;
}

bool cChar::onCollideChar( P_CHAR Obstacle ) 
{
	if( scriptChain.empty() )
		return false;
 
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
		if( scriptChain[ i ]->onCollideChar( this, Obstacle ) )
			return true;

	return false;
}

void cChar::processNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	QString Value = this->getNodeValue( Tag );
	QDomNodeList ChildTags;

	// <bindmenu>contextmenu</bindmenu>
	// <bindmenu id="contextmenu />
	if( TagName == "bindmenu" )
	{
		if( !Tag.attribute( "id" ).isNull() ) 
			this->bindmenu = Tag.attribute( "id" );
		else
			bindmenu = Value;
	}

	//<name>my this</name>
	if( TagName == "name" )
		this->name = Value;
		
	//<backpack>
	//	<color>0x132</color>
	//	<item id="a">
	//	...
	//	<item id="z">
	//</backpack>
	else if( TagName == "backpack" )
	{
		if( this->packitem_ == INVALID_SERIAL )
		{
			P_ITEM pBackpack = Items->SpawnItem( -1, this, 1, "Backpack", 0, 0x0E,0x75,0,0,0);
			if( pBackpack == NULL )
			{
				cCharStuff::DeleteChar( this );
				return;
			}
			
			this->packitem_ = pBackpack->serial;
			
			pBackpack->pos.x = 0;
			pBackpack->pos.y = 0;
			pBackpack->pos.z = 0;
			pBackpack->setContSerial(this->serial);
			pBackpack->setLayer( 0x15 );
			pBackpack->setType( 1 );
			pBackpack->dye=1;

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

			for( UINT8 i = 0; i < ALLSKILLS; ++i )
				Skills->updateSkillLevel( this, i );
		}
	}

	// Aliasses <str <dex <int
	else if( TagName == "str" )
	{
		st_ = Value.toLong();
		hp_ = st_;
		
		for( UINT8 i = 0; i < ALLSKILLS; ++i )
			Skills->updateSkillLevel( this, i );
	}

	else if( TagName == "dex" )
	{
		setDex( Value.toLong() );
		stm_ = realDex();

		for( UINT8 i = 0; i < ALLSKILLS; ++i )
			Skills->updateSkillLevel( this, i );
	}
	
	else if( TagName == "int" )
	{
		in_ = Value.toLong();
		mn_ = in_;

		for( UINT8 i = 0; i < ALLSKILLS; ++i )
			Skills->updateSkillLevel( this, i );
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
					this->fx1_ = this->pos.x + Tag.attribute("x1").toInt();
					this->fx2_ = this->pos.x + Tag.attribute("x2").toInt();
					this->fy1_ = this->pos.y + Tag.attribute("y1").toInt();
					this->fy2_ = this->pos.y + Tag.attribute("y2").toInt();
					this->fz1_ = -1 ;
				}
			else if( wanderType == "circle" || wanderType == "2" )
			{
				this->npcWander_ = 2;
				this->fx1_ =  this->pos.x ;
				this->fy1_ = this->pos.y ;
				this->fz1_ = this->pos.z ;
				if( Tag.attributes().contains("radius") )
					this->fx2_ =  Tag.attribute("radius").toInt();
				else
					this->fx2_ = 2 ;
			}
			else if( wanderType == "free" || wanderType == "1" )
				this->npcWander_ = 1;
			else
				this->npcWander_ = 0; //default
		}
	}
	//<ai>2</ai>
	else if( TagName == "ai" )
		this->setNpcAIType( Value.toInt() );

	//<onhorse />
	else if( TagName == "onhorse" )
		this->setOnHorse( true );
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
	}
		
	//<spattack>3</spattack>
	else if( TagName == "spattack" )
		this->spattack = Value.toInt();

	//<speech>13</speech>
	else if( TagName == "speech" )
		this->speech_ = Value.toUShort();

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
		this->spadelay = Value.toInt();

	//<stablemaster />
	else if( TagName == "stablemaster" )
		this->setNpc_type(1);

	//<title>the king</title>
	else if( TagName == "title" )
		this->setTitle( Value );

	//<totame>115</totame>
	else if( TagName == "totame" )
		this->taming = Value.toInt();

	//<skill type="alchemy">100</skill>
	//<skill type="1">100</skill>
	else if( TagName == "skill" && Tag.attributes().contains("type") )
	{
		if( Tag.attribute("type").toInt() > 0 &&
			Tag.attribute("type").toInt() <= ALLSKILLS )
			this->setBaseSkill( ( Tag.attribute( "type" ).toInt() - 1 ), Value.toInt() );
		else
		{
			for( UI32 j = 0; j < ALLSKILLS; j++ )
			{
				if( Tag.attribute("type").contains( QString(skillname[j]), false ) )
					this->setBaseSkill( j, Value.toInt() );
			}
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
			P_ITEM nItem = Items->MemItemFree();
	
			if( nItem == NULL )
				continue;
	
			nItem->Init( true );
			cItemsManager::getInstance()->registerItem( nItem );

			QDomElement tItem = (*iter);

			nItem->applyDefinition( tItem );

			// Instead of deleting try to get a valid layer instead
			if( nItem->layer() == 0 )
			{
				tile_st tInfo = cTileCache::instance()->getTile( nItem->id() );
				if( tInfo.layer > 0 )
					nItem->setLayer( tInfo.layer );
			}
				
			// Recheck
			if( !nItem->layer() )
				Items->DeleItem( nItem );
			else
				nItem->setContSerial( this->serial );

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

		QDomElement* DefSection = DefManager->getSection( WPDT_NPC, inheritID );
		if( !DefSection->isNull() )
			this->applyDefinition( *DefSection );
	}

	else
	{
		bool found = false;

		for( UINT8 i = 0; i < ALLSKILLS; ++i )
		{
			// It's a skillname
			if( TagName.upper() == skillname[i] )
			{
				setBaseSkill( i, Value.toInt() );
				Skills->updateSkillLevel( this, i );
				found = true;
				break;
			}
		}

		if( !found )
			cUObject::processNode( Tag );
	}		
}

void cChar::soundEffect( UI16 soundId, bool hearAll )
{
	cUOTxSoundEffect pSoundEffect;
	pSoundEffect.setSound( soundId );
	pSoundEffect.setCoord( pos );

	if( !hearAll )
	{
		if( socket_ )
			socket_->send( &pSoundEffect );
	}
	else 
	{
		// Send the sound to all sockets in range
		for( cUOSocket *s = cNetwork::instance()->first(); s; s = cNetwork::instance()->next() )
			if( s->player() && s->player()->inRange( this, s->player()->VisRange ) )
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

	QString lang( "ENU" );

	if( this->socket() )
		lang = socket_->lang();
	
	cUOTxUnicodeSpeech::eSpeechType speechType;

	switch( type )
	{
	case 0x01:
		speechType = cUOTxUnicodeSpeech::Broadcast;
	case 0x06:
		speechType = cUOTxUnicodeSpeech::System;
	case 0x09:
		speechType = cUOTxUnicodeSpeech::Yell;
	case 0x02:
		speechType = cUOTxUnicodeSpeech::Emote;
	case 0x08:
		speechType = cUOTxUnicodeSpeech::Whisper;
	default:
		speechType = cUOTxUnicodeSpeech::Regular;
	};

	cUOTxUnicodeSpeech* textSpeech = new cUOTxUnicodeSpeech();
	textSpeech->setSource( serial );
	textSpeech->setModel( id() );
	textSpeech->setFont( 3 ); // Default Font
	textSpeech->setType( speechType );
	textSpeech->setLanguage( lang );
	textSpeech->setName( name.c_str() );
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
			if( !socket->player()->dead() && !socket->player()->spiritspeaktimer && !socket->player()->isGMorCounselor() )
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
				if( mSock->player() && ( mSock->player()->pos.distance( pos ) < 18 ) )
				{
					// Take the dead-status into account
					if( dead_ && !isNpc() )
						if( !mSock->player()->dead() && !mSock->player()->spiritspeaktimer && !mSock->player()->isGMorCounselor() )
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
	textSpeech.setSource( serial );
	textSpeech.setModel( id() );
	textSpeech.setFont( 3 ); // Default Font
	textSpeech.setType( cUOTxUnicodeSpeech::Emote );
	textSpeech.setLanguage( "ENU" );
	textSpeech.setName( name.c_str() );
	textSpeech.setColor( color );
	textSpeech.setText( emote );
	
	for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
		if( mSock->player() && mSock->player()->inRange( this, mSock->player()->VisRange ) )
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
	if( moveFromAccToAcc && account_ != 0 )
		account_->removeCharacter( this );

	account_ = data;

	if( account_ != 0 )
		account_->addCharacter( this );
}

void cChar::giveItemBonus(cItem* pi)
{
	st_ += pi->st2;
	chgDex( pi->dx2 );
	in_ += pi->in2;
}

void cChar::showName( cUOSocket *socket )
{
	if( !socket->player() )
		return;

	if( onShowCharName( socket->player() ) )
		return;

	QString charName = name.c_str();

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
		charName.append( QString( " [0x%1]" ).arg( serial, 4, 16 ) );

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
	if( guarded() )
		charName.append( tr(" [guarded]") );

	// Guarding
	if( tamed() && npcaitype_ == 32 && socket->player()->Owns( this ) && socket->player()->guarded() )
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
	
	Q_UINT8 gStatus = GuildCompare( this, socket->player() );
	Q_UINT16 speechColor;

	if( !gStatus )
	{
		if( isGMorCounselor() )
			speechColor = 0x35;
		else switch( flag() )
		{
			case 0x01:	speechColor = 0x26; break; //red
			case 0x04:	speechColor = 0x5A; break; //blue
			case 0x08:	speechColor = 0x4A; break; //green
			case 0x10:	speechColor = 0x30; break; //orange
			default:	speechColor = 0x3B2; break; //grey
		}
	}

	// Same Guild or Allied Guild
	else if( gStatus == 1 )
		speechColor = 0x43;

	// Enemy Guild
	else if( gStatus == 2 )
		speechColor = 0x30;

	// Show it to the socket
	socket->showSpeech( this, charName, speechColor, 3, cUOTxUnicodeSpeech::System );
}

// Update flags etc.
void cChar::update( void )
{
	cUOTxUpdatePlayer* updatePlayer = new cUOTxUpdatePlayer();
	updatePlayer->fromChar( this );

	for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		P_CHAR pChar = mSock->player();

		if( pChar && pChar->socket() && pChar->inRange( this, pChar->VisRange ) )
		{
			updatePlayer->setHighlight( notority( pChar ) );
			mSock->send( new cUOTxUpdatePlayer( *updatePlayer ) );	
		}
	}
	delete updatePlayer;
}

// Resend the char to all sockets in range
void cChar::resend( bool clean )
{
	if( socket_ )
		socket_->resendPlayer();

	// We are stabled and therefore we arent visible to others
	if( stablemaster_serial() != INVALID_SERIAL )
		return;

	RegionIterator4Chars ri( pos );

	cUOTxDrawChar drawChar;
	drawChar.fromChar( this );

	for( ri.Begin(); !ri.atEnd(); ri++ )
	{
		P_CHAR pChar = ri.GetData();

		if( !pChar || !pChar->socket() )
			continue;

		if( pChar->pos.distance( pos ) > pChar->VisRange )
			continue;

        // Remove it ONLY before resending if we have to do it "clean"
		if( clean )
			pChar->socket()->removeObject( this );

		if( ( isHidden() || ( dead_ && !war_ ) ) && !pChar->isGMorCounselor() )
			continue;

		drawChar.setHighlight( notority( pChar ) );
		pChar->socket()->send( &drawChar );
	}
}

QString cChar::fullName( void )
{
	QString fName;

	if( isGM() )
		fName = QString( "%1 %2" ).arg( name.c_str() ).arg( title_ );

	// Normal Criminal
	else if( ( crimflag_ > 0 ) && !dead_ && ( kills_ < SrvParams->maxkills() ) )
		fName = tr( "The Criminal %1, %2%3 %4" ).arg( name.c_str() ).arg( title_ ).arg( title1( this ) ).arg( title2( this ) );

	// The Serial Killer
	else if( ( kills_ >= SrvParams->maxkills() ) && ( kills_ < 10 ) && !dead_ )
		fName = tr( "The Serial Killer %1, %2%3 %4" ).arg( name.c_str() ).arg( title_ ).arg( title1( this ) ).arg( title2( this ) );

	// The Murderer
	else if( ( kills_ >= 10 ) && ( kills_ < 20 ) && !dead_ )
		fName = tr( "The Murderer %1, %2%3 %4" ).arg( name.c_str() ).arg( title_ ).arg( title1( this ) ).arg( title2( this ) );

	// The Mass Murderer
	else if( ( kills_ >= 20 ) && ( kills_ < 50 ) && !dead_ )
		fName = tr( "The Mass Murderer %1, %2%3 %4" ).arg( name.c_str() ).arg( title_ ).arg( title1( this ) ).arg( title2( this ) );

	// The Evil Dread Murderer
	else if( ( kills_ >= 50 ) && ( kills_ < 100 ) && !dead_ )
		fName = tr( "The Evil Dread Murderer %1, %2%3 %4" ).arg( name.c_str() ).arg( title_ ).arg( title1( this ) ).arg( title2( this ) );

	// The Evil Emperor
	else if( ( kills_ >= 100 ) && !dead_ )
		fName = tr( "The Evil Emperor %1, %2%3 %4" ).arg( name.c_str() ).arg( title_ ).arg( title1( this ) ).arg( title2( this ) );

	// Normal Player
	else if( title_.isEmpty() )
		fName = QString( "%1%2, %3 %4" ).arg( title3( this ) ).arg( name.c_str() ).arg( title1( this ) ).arg( title2( this ) );

	else
		fName = QString( "%1%2 %4, %4 %5" ).arg( title3( this ) ).arg( name.c_str() ).arg( title_ ).arg( title1( this ) ).arg( title2( this ) );

	return fName;
}

cGuildStone *cChar::getGuildstone()
{ 
	return dynamic_cast<cGuildStone*>( FindItemBySerial( guildstone_ ) ); 
}

void cChar::makeShop( void )
{
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
			pItem->setContSerial( serial);
			pItem->setLayer( layer );
			pItem->setType( 1 );
			pItem->priv |= 0x02;
		}
	}
}

// Send the changed health-bar to all sockets in range
void cChar::updateHealth( void )
{
	RegionIterator4Chars cIter( pos );
	for( cIter.Begin(); !cIter.atEnd(); cIter++ )
	{
		P_CHAR pChar = cIter.GetData();

		// Send only if target can see us
		if( !pChar || !pChar->socket() || !pChar->inRange( this, pChar->VisRange ) || ( isHidden() && !pChar->isGM() && this != pChar ) )
			continue;
	
		pChar->socket()->sendStatWindow( this );
	}
}

class cResetAnimated: public cTempEffect
{
public:
	cResetAnimated( P_CHAR pChar, UINT32 ms )
	{
		expiretime = uiCurrentTime + ms;
		sourSer = pChar->serial;
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
	// No "double" actions using this function
	if( animated )
		return;

	bool mounted = onHorse();

	if( mounted && ( id == 0x10 || id == 0x11 ) )
		id = 0x1b;
	else if( mounted && ( id == 0x0D ) )
		id = 0x1D;
	else if( ( mounted || this->id() < 0x190 ) && ( id == 0x22 ) )
		return;

	cUOTxAction action;
	action.setAction( id );
	action.setSerial( serial );
	action.setDirection( dir_ );
	action.setRepeat( 1 );
	action.setRepeatFlag( 0 );
	action.setSpeed( 1 );

	for( cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next() )
	{
		if( socket->player() && socket->player()->inRange( this, socket->player()->VisRange ) && ( !isHidden() || socket->player()->isGM() ) )
			socket->send( &action );
	}

	// Reset the animated status after a given amount of time.
	cTempEffects::getInstance()->insert( new cResetAnimated( this, 1500 ) );
}

UINT8 cChar::notority( P_CHAR pChar ) // Gets the notority toward another char
{
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
	else switch( flag() )
	{	//1=blue 2=green 5=orange 6=Red 7=Transparent(Like skin 66 77a)
		case 0x01: result = 0x06; break; // If a bad, show as red.
		case 0x04: result = 0x01; break; // If a good, show as blue.
		case 0x08: result = 0x02; break; // green (guilds)
		case 0x10: result = 0x05; break; // orange (guilds)
		default:   
			if( npcaitype() > 0 || !isNpc() )
				return 0x01; // 1 = Blue -> Innocent
			else
				return 0x03; // grey
	}

	return result;
}

// Formerly deathstuff()
void cChar::kill()
{
	int l, q, ele;
	int nType=0;
	UINT32 ci;

	if( free )
		return;

	if( dead_ || npcaitype() == 17 || isInvul() )
		return;

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
		murderer = pAttacker->name.c_str();

	}

	// We do know our murderer here (or if there is none it's null)
	// So here it's time to kall onKilled && onKill
	// And give them a chance to return true
	// But take care. You would need to create the corpse etc. etc.
	// Which is *hard* work
	// TODO: Call onKilled/onKill events

	// Reputation system ( I dont like the idea of this loop )
	AllCharsIterator iter_char;
	for( iter_char.Begin(); !iter_char.atEnd(); iter_char++ )
	{
		P_CHAR pc_t = iter_char.GetData();
		if( pc_t->targ() == serial && !pc_t->free )
		{
			if( pc_t->npcaitype() == 4 )
			{
				pc_t->summontimer = ( uiCurrentTime + ( MY_CLOCKS_PER_SEC * 20 ) );
				pc_t->setNpcWander(2);
				pc_t->setNextMoveTime();
				pc_t->talk( tr( "Thou have suffered thy punishment, scoundrel." ), -1, 0, true );
			}

			pc_t->setTarg( INVALID_SERIAL );
			pc_t->setTimeOut(0);

			if( pc_t->attacker() != INVALID_SERIAL )
			{
				P_CHAR pc_attacker = FindCharBySerial(pc_t->attacker());
				pc_attacker->resetAttackFirst();
				pc_attacker->setAttacker(INVALID_SERIAL);
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
						// BountyAskVictim( this->serial, pc_t->serial );
						setMurdererSer( pc_t->serial );
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
						sprintf((char*)temp,"%s was killed by %s!\n", name.c_str(),pc_t->name.c_str());
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

	P_ITEM pi_j;
	vector<SERIAL> vecContainer( contsp.getData( serial ) );
	for( ci = 0; ci < vecContainer.size(); ++ci )
	{
		pi_j = FindItemBySerial(vecContainer[ci]);
		if(pi_j && pi_j->type()==1 && pi_j->pos.x==26 && pi_j->pos.y==0 &&
			pi_j->pos.z==0 && pi_j->id()==0x1E5E )
		{
			endtrade( pi_j->serial );
		}
	}
	ele = 0;

	// I would *NOT* do that but instead replace several *send* things
	// We have ->dead already so there shouldn't be any checks regarding
	// 0x192-0x193 to see if the char is dead or not
	if( xid_ == 0x0191 )
		setId( 0x0193 );	// Male or Female
	else
		setId( 0x0192 );

	PlayDeathSound( this );

	setSkin( 0x0000 ); // Undyed
	dead_ = true; // Dead
	hp_ = 0; // With no hp left
	
	// Reset poison
	setPoisoned(0);
	setPoison(0);

	// Create our Corpse
	cCorpse *corpse = new cCorpse( true );
	cItemsManager::getInstance()->registerItem( corpse );

	QDomElement *elem = DefManager->getSection( WPDT_ITEM, "2006" );
	
	if( elem && !elem->isNull() )
		corpse->applyDefinition( (*elem) );

	corpse->setName( tr( "corpse of %1" ).arg( name.c_str() ) );
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
			corpse->more2 = 1;
	    else if( isCriminal() )
			corpse->more2 = 2;
	    else if( isMurderer() )
			corpse->more2 = 3;

        corpse->ownserial = serial;
	}

	corpse->setBodyId( xid_ );
	corpse->morey = ishuman( this ); //is human??
	corpse->setCarve( carve() ); //store carve section
	corpse->setName2( name.c_str() );

	corpse->moveTo( pos );

	corpse->more1 = nType;
	corpse->dir = dir_;
	corpse->startDecay();
	
	// If it was a player set the ownerserial to the player's
	if( isPlayer() )
	{
		corpse->SetOwnSerial(serial);
		// This is.... stupid...
		corpse->more4 = char( SrvParams->playercorpsedecaymultiplier()&0xff ); // how many times longer for the player's corpse to decay
	}

	// stores the time and the murderer's name
	corpse->setMurderer( murderer );
	corpse->murdertime = uiCurrentTime;

	// create loot
	if( isNpc() )
	{
		QStringList lootItemSections = DefManager->getList( lootList() );
		QStringList::const_iterator it = lootItemSections.begin();

		while( it != lootItemSections.end() )
		{
			P_ITEM pi_loot = Items->createScriptItem( (*it) );
			if( pi_loot )
				pi_loot->setContSerial( corpse->serial );

			it++;
		}
	}
	
	// Put objects on corpse
	vecContainer.clear();
	vecContainer = contsp.getData( serial );
	bool resetShop = false;

	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi_j = FindItemBySerial( vecContainer[ci] );
		// for BONUS ITEMS - remove bonus
		removeItemBonus( pi_j );

		// unequip trigger...
		if( ( pi_j->contserial == serial ) && ( pi_j->layer() != 0x0B ) && ( pi_j->layer() != 0x10 ) )
		{	// Let's check all items, except HAIRS and BEARD
			// Ripper...so order/chaos shields disappear when on corpse backpack.
			if( pi_j->id() == 0x1BC3 || pi_j->id() == 0x1BC4 )
			{
				soundEffect( 0x01FE );
				staticeffect( this, 0x37, 0x2A, 0x09, 0x06 );
				Items->DeleItem( pi_j );
			}

			if( pi_j->type() == 1 && pi_j->layer() != 0x1A && pi_j->layer() != 0x1B && pi_j->layer() != 0x1C && pi_j->layer() != 0x1D )
			{   // if this is a pack but it's not a VendorContainer(like the buy container) or a bankbox
				unsigned int ci1;
				vector<SERIAL> vecContainer = contsp.getData( pi_j->serial );
				for ( ci1 = 0; ci1 < vecContainer.size(); ++ci1)
				{
					P_ITEM pi_k = FindItemBySerial( vecContainer[ ci1 ] );

					if( !pi_k )
						continue;

					// put the item in the corpse only of we're sure it's not a newbie item or a spellbook
					if( !pi_k->newbie() && ( pi_k->type() != 9 ) )
					{
						corpse->AddItem( pi_k );
						
						// Ripper...so order/chaos shields disappear when on corpse backpack.
						if( pi_k->id() == 0x1BC3 || pi_k->id() == 0x1BC4 )
						{
							soundEffect( 0x01FE );
							staticeffect( this, 0x37, 0x2A, 0x09, 0x06 );
							Items->DeleItem( pi_k );
						}
					}
				}
			}
			// if it's a NPC vendor special container
			else if( pi_j->layer() == 0x1A )
			{
				// This seems odd to me
				// but i converted it from the old version
				resetShop = true;
			}
			// if it's a normal item but ( not newbie and not bank items )
			else if ( !pi_j->newbie() && pi_j->layer() != 0x1D )
			{
				if( pi_j != pi_backpack )
				{
					corpse->addEquipment( pi_j->layer(), pi_j->serial );
					corpse->AddItem( pi_j );					
				}
			}
			else if( ( pi_j != pi_backpack ) && ( pi_j->layer() != 0x1D ) )
			{	
				// else if the item is newbie put it into char's backpack
				pi_backpack->AddItem( pi_j );
			}

			//if( ( pi_j->layer() == 0x15 ) && ( shop == 0 ) ) 
			//	pi_j->setLayer( 0x1A );
		}
	}	

	cUOTxDeathAction dAction;
	dAction.setSerial( serial );
	dAction.setCorpse( corpse->serial );

	cUOTxClearBuy rShop;
	rShop.setSerial( serial );

	cUOTxRemoveObject rObject;
	rObject.setSerial( serial );

	for( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
		if( mSock->player() && mSock->player()->inRange( this, mSock->player()->VisRange ) && ( mSock != socket_ ) )
		{
			if( SrvParams->showDeathAnim() )
				mSock->send( &dAction );

			mSock->send( &rObject );

			if( resetShop )
				mSock->send( &rShop );
		}
	
	corpse->update();

	if( isPlayer() )
	{
#pragma note( "Deathshroud has to be defined as 204e in the scripts" )
		P_ITEM pItem = Items->createScriptItem( "204e" );
		if( pItem )
		{
			robe_ = pItem->serial;
			pItem->setContSerial( serial );
			pItem->setLayer( 0x16 );
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

	// St00pid -> spawned chars shoudl become invis so they keep their stats
	// and information
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

	pRobe->setContSerial( serial );
	pRobe->setColor( 0 );
	pRobe->setHp( 1 );
	pRobe->setMaxhp( 1 );
	pRobe->setLayer( 0x16 );
	pRobe->update();

	resend( false );
}

void cChar::turnTo( cUObject *object )
{
	INT16 xdif = (INT16)( object->pos.x - pos.x );
	INT16 ydif = (INT16)( object->pos.y - pos.y );
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
		dir_ = nDir;
		// TODO: we could try to use an update here because our direction
		// changed for sure
		resend( false );
	}
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
	this->setOnHorse( false );
	unsigned int ci = 0;
	P_ITEM pi;
	vector<SERIAL> vecContainer(contsp.getData(this->serial));
	for ( ci = 0; ci < vecContainer.size(); ++ci)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi != NULL && !pi->free)
		{
			if (pi->layer() == 0x19)
				this->setOnHorse( true );
			cUOTxCharEquipment packet;
			packet.setWearer( this->serial );
			packet.setSerial( pi->serial );
			packet.fromItem( pi );
			for ( cUOSocket* socket = cNetwork::instance()->first(); socket != 0; socket = cNetwork::instance()->next() )
				if( socket->player() && socket->player()->inRange( this, socket->player()->VisRange ) ) 
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
	this->setOnHorse( false );
	unsigned int ci=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer(contsp.getData(this->serial));
	for ( ci = 0; ci < vecContainer.size(); ++ci)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi != NULL && !pi->free)
		{
			if (pi->layer()==0x19)
				this->setOnHorse( true );
			cUOTxCharEquipment packet;
			packet.setWearer( this->serial );
			packet.setSerial( pi->serial );
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
	pi->setContSerial( this->serial );
	cUOTxCharEquipment packet;
	packet.setWearer( this->serial );
	packet.setSerial( pi->serial );
	packet.fromItem( pi );
	for ( cUOSocket* socket = cNetwork::instance()->first(); socket != 0; socket = cNetwork::instance()->next() )
		if( socket->player() && socket->player()->inRange( this, socket->player()->VisRange ) ) 
			socket->send( &packet );
}

P_CHAR cChar::unmount()
{
	std::vector< SERIAL > vecContainer = contsp.getData( serial );
	std::vector< SERIAL >::iterator it = vecContainer.begin();
	while( it != vecContainer.end() )
	{
		P_ITEM pi = FindItemBySerial( *it );
		if( pi && pi->layer() == 0x19 && !pi->free)
		{
			setOnHorse( false );

			P_CHAR pMount = FindCharBySerial( pi->morex );
			if( pMount )
			{
				pMount->setFx1( pi->pos.x );
				pMount->setFy1( pi->pos.y );
				pMount->setFz1( pi->pos.z );
				pMount->setId( pi->morey );
				pMount->setNpcWander(pi->moreb1());
				pMount->setSt( pi->moreb2() );
				pMount->setDex( pi->moreb3() );
				pMount->setIn( pi->moreb4() );
				pMount->setFx2( pi->att );
				pMount->setFy2( pi->def );
				pMount->setHp( pi->hp() );
				pMount->setFame( pi->lodamage() );
				pMount->setKarma( pi->hidamage() );
				pMount->setPoisoned( pi->poisoned );
				pMount->summontimer = pi->decaytime;

				pMount->moveTo( pos );
				pMount->resend( false );
			}
			
			Items->DeleItem( pi );
			resend( false );
			return pMount;
		}
		++it;
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

	if( Owns( pMount ) || isGM() )
	{
		if( onHorse() )
			unmount();

		setOnHorse( true );
		P_ITEM pMountItem = Items->SpawnItem( this, 1, pMount->name.c_str(), 0, 0x0915, pMount->skin(), 0 );
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
			case 0x1a: pMountItem->setId(0x3EBD); break;// swamp dragon
			case 0x1f: pMountItem->setId(0x3EBE); break;// armor dragon
		}
		
		pMountItem->setContSerial( serial );
		pMountItem->setLayer( 0x19 );
		Coord_cl npos( pos );
		npos.x = pMount->fx1();
		npos.y = pMount->fy1();
		npos.z = pMount->fz1();
		pMountItem->moveTo(npos);
		
		pMountItem->morex = pMount->serial;
		pMountItem->morey = pMount->id();

		pMountItem->setMoreb1( pMount->npcWander() );
		pMountItem->setMoreb2( pMount->st() );
		pMountItem->setMoreb3( pMount->realDex() );
		pMountItem->setMoreb4( pMount->in() );
		pMountItem->att = pMount->fx2();
		pMountItem->def = pMount->fy2();
		pMountItem->setHp( pMount->hp() );
		pMountItem->setLodamage( pMount->fame() );
		pMountItem->setHidamage( pMount->karma() );
		pMountItem->poisoned = pMount->poisoned();
		if (pMount->summontimer != 0)
			pMountItem->decaytime = pMount->summontimer;
	
		// Sends update.
		wear( pMountItem );

		// if this is a gm lets tame the animal in the process
		if( isGM() )
		{
			pMount->SetOwnSerial( serial );
			pMount->setNpcAIType( 0 );
		}
		
		// remove it from screen!
		pMount->setId(0);
		cMapObjects::getInstance()->remove( pMount );
		pMount->pos = Coord_cl(0, 0, 0);
		
		pMount->removeFromView( true );
		
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
	QDomElement *startItems = DefManager->getSection( WPDT_STARTITEMS, ( skill == 0xFF ) ? QString("default") : QString( skillname[ skill ] ).lower() );

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
				QDomElement *DefSection = DefManager->getSection( WPDT_ITEM, node.attribute( "id" ) );
				if( DefSection && !DefSection->isNull() )
				{
					// books wont work without this
					pItem = Items->createScriptItem( node.attribute("id") );
				}
				else
				{
					pItem = Items->MemItemFree();
					pItem->Init( true );
					cItemsManager::getInstance()->registerItem( pItem );
				}

				if( pItem )
				{
					pItem->applyDefinition( node );
					pItem->priv |= 0x02; // make it newbie

					if( pItem->id() <= 1 )
						Items->DeleItem( pItem );
					else
					{
						// Put it into the backpack
						P_ITEM backpack = getBackpack();
						if( backpack )
							backpack->AddItem( pItem );
						else
							Items->DeleItem( pItem );
					}
				}
			}
			else if( node.nodeName() == "bankitem" )
			{
				P_ITEM pItem = NULL;
				QDomElement *DefSection = DefManager->getSection( WPDT_ITEM, node.attribute( "id" ) );
				if( DefSection && !DefSection->isNull() )
				{
					// books wont work without this
					pItem = Items->createScriptItem( node.attribute("id") );
				}
				else
				{
					pItem = Items->MemItemFree();
					pItem->Init( true );
					cItemsManager::getInstance()->registerItem( pItem );
				}

				if( pItem )
				{
					pItem->applyDefinition( node );
					pItem->priv |= 0x02; // make it newbie

					if( pItem->id() <= 1 )
						Items->DeleItem( pItem );
					else
					{
						// Put it into the bankbox
						P_ITEM bankbox = getBankBox();
						if( bankbox )
							bankbox->AddItem( pItem );
						else
							Items->DeleItem( pItem );
					}
				}
			}
			else if( node.nodeName() == "equipment" )
			{
				P_ITEM pItem = NULL;
				QDomElement *DefSection = DefManager->getSection( WPDT_ITEM, node.attribute( "id" ) );
				if( DefSection && !DefSection->isNull() )
				{
					// books wont work without this
					pItem = Items->createScriptItem( node.attribute("id") );
				}
				else
				{
					pItem = Items->MemItemFree();
					pItem->Init( true );
					cItemsManager::getInstance()->registerItem( pItem );
				}

				if( pItem )
				{
					pItem->applyDefinition( node );
					pItem->priv |= 0x02; // make it newbie
					if( pItem->layer() == 0 )
					{
						tile_st tile = cTileCache::instance()->getTile( pItem->id() );
						pItem->setLayer( tile.layer );
					}

					if( pItem->id() <= 1 || pItem->layer() == 0 )
						Items->DeleItem( pItem );
					else
					{
						// Put it onto the char
						pItem->setContSerial( serial );
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
				QDomElement* inheritNode = DefManager->getSection( WPDT_STARTITEMS, node.attribute("id") );
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

//	if (defender->pos.z > (attacker->pos.z +10)) return;//FRAZAI
//	if (defender->pos.z < (attacker->pos.z -10)) return;//FRAZAI

	Coord_cl attpos( pos );
	Coord_cl defpos( defender->pos );

	attpos.z += 13; // eye height of attacker

	if( !lineOfSight( attpos, defpos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING ) )
		return;

	playmonstersound( defender, defender->id(), SND_STARTATTACK );
	int i;
	unsigned int cdist=0 ;

	P_CHAR target = FindCharBySerial( defender->targ() );
	if( target )
		cdist = defender->pos.distance( target->pos );
	else 
		cdist = 30;

	if( cdist > defender->pos.distance( pos ) )
	{
		defender->setTarg( serial );
		defender->setAttacker(serial);
		defender->setAttackFirst();
	}

	target = FindCharBySerial( targ_ );
	if( target )
		cdist = pos.distance( target->pos );
	else 
		cdist = 30;

	if( ( cdist > defender->pos.distance( pos ) ) &&
		( !(npcaitype() == 4) || target ) )
	{
		targ_ = defender->serial;
		attacker_ = defender->serial;
		resetAttackFirst();
	}

	defender->unhide();
	defender->disturbMed();

	unhide();
	disturbMed();

	if( defender->isNpc() )
	{
		if( !( defender->war() ) )
			defender->toggleCombat();
		defender->setNextMoveTime();
	}
	
	if( ( isNpc() ) && !( npcaitype() == 4 ) )
	{
		if ( !( war_ ) )
			toggleCombat();

		setNextMoveTime();
	}
	
	// Send a message to the defender
	if( defender->socket() )
	{
		QString message = tr( "You see %1 attacking you!" ).arg( name.c_str() );
		defender->socket()->showSpeech( this, message, 0x26, 3, cUOTxUnicodeSpeech::Emote );
	}

	QString emote = tr( "You see %1 attacking %2" ).arg( name.c_str() ).arg( defender->name.c_str() );

	RegionIterator4Chars cIter( pos );
	for( cIter.Begin(); !cIter.atEnd(); cIter++ )
	{
		P_CHAR pChar = cIter.GetData();

		if( pChar && ( pChar != defender ) && pChar->socket() && pChar->inRange( this, pChar->VisRange ) )
			pChar->socket()->showSpeech( this, emote, 0x26, 3, cUOTxUnicodeSpeech::Emote );
	}
}

void cChar::toggleCombat()
{
	war_ = !war_;
	Movement->CombatWalk( this );
}

P_ITEM cChar::rightHandItem()
{
	return GetItemOnLayer( 1 );
}

P_ITEM cChar::leftHandItem()
{
	return GetItemOnLayer( 2 );
}

void cChar::applyPoison( P_CHAR defender )
{
	if( !defender )
		return;

	if( poison() && ( defender->poisoned() < poison() ) )
	{
		if( RandomNum( 0, 2 ) == 2 )
		{
			defender->setPoisoned( poison() );

			// a lev.1 poison takes effect after 40 secs, a deadly pois.(lev.4) takes 40/4 secs - AntiChrist
			defender->setPoisontime( uiCurrentTime + ( MY_CLOCKS_PER_SEC*( 40 / defender->poisoned() ) ) ); 

			// wear off starts after poison takes effect - AntiChrist
			defender->setPoisonwearofftime( defender->poisontime() + ( MY_CLOCKS_PER_SEC*SrvParams->poisonTimer() ) ); 

			defender->resend( false );
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
			total += ( (UI16)floor( (float)( skill( PARRYING ) * pShield->def ) / 200.0f ) + 1 );
	} 	

	if( skill( PARRYING ) >= 1000 ) 
		total += 5; // gm parry bonus. 

	P_ITEM pi; 
	std::vector< SERIAL > vecContainer = contsp.getData( serial );
	std::vector< SERIAL >::iterator it = vecContainer.begin();

	while( it != vecContainer.end() )
	{
		pi = FindItemBySerial( *it );
		if( pi && pi->layer() > 1 && pi->layer() < 25 ) 
		{ 
			//blackwinds new stuff 
			UI16 effdef = 0;
			if( pi->maxhp() > 0 ) 
				effdef = (UI16)floor( (float)pi->hp() / (float)pi->maxhp() * (float)pi->def );

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
	
	if( total < 2 && bodypart == ALLBODYPARTS ) 
		total = 2;

	return total;
}

bool cChar::checkSkill( UI16 skill, SI32 min, SI32 max, bool advance )
{
	bool skillused = false;
	
	if( dead_ )
	{
		if( socket_ )
			socket_->sysMessage( tr( "Ghosts can not train %1" ).arg( QString( skillname[ skill ] ).lower() ) );
		return false;
	}

	if( max > 1200 )
		max = 1200;

	// how far is the player's skill above the required minimum ?
	int charrange = this->skill( skill ) - min;	
	
	if( charrange < 0 )
		charrange = 0;

	if( min == max )
	{
		LogCritical("cChar::checkSkill(..): minskill == maxskill, avoided division by zero\n");
		return false;
	}
	float chance = (((float)charrange*890.0f)/(float)(max-min))+100.0f;	// +100 means: *allways* a minimum of 10% for success
	if( chance > 990 ) 
		chance=990;	// *allways* a 1% chance of failure
	
	if( chance >= RandomNum( 0, 1000 ) ) 
		skillused = true;
	
	if( baseSkill_[ skill ] < max )
	{
		// Take care. Only gain skill when not using scrolls
		//if( sk != MAGERY || ( sk == MAGERY && pc->isPlayer() && currentSpellType[s] == 0 ) )
		//{
			if( advance && Skills->AdvanceSkill( this, skill, skillused ) )
			{
				Skills->updateSkillLevel(this, skill); 
				if( socket_ )
					socket_->sendSkill( skill );
			}
		//}
	}
	return skillused;
}

void cChar::setSkillDelay() 
{ 	
	SetTimerSec(&skilldelay,SrvParams->skillDelay());
}

