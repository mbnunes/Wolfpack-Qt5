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

// just temporary
#undef  DBGFILE
#define DBGFILE "chars.cpp"

// Platform include
#include "platform.h"


// Wolfpack includes
#include "wpdefaultscript.h"
#include "chars.h"
#include "charsmgr.h"
#include "items.h"
#include "debug.h"
#include "globals.h"
#include "wolfpack.h"
#include "iserialization.h"
#include "regions.h"
#include "srvparams.h"
#include "utilsys.h"
#include "network.h"
#include "mapstuff.h"
#include "classes.h"
#include "wpdefmanager.h"

// Inline members

bool cChar::Owns(P_ITEM pi)			{	return (serial==pi->ownserial);		}
bool cChar::Wears(P_ITEM pi)			{	return (serial == pi->contserial);	}
unsigned int cChar::dist(cChar* pc)	{	return pos.distance(pc->pos);		}
unsigned int cChar::dist(cItem* pi)	{	return pos.distance(pi->pos);		}
string cChar::objectID()				{	return string("CHARACTER");			}

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
	this->multis=-1;//Multi serial
	this->free = false;
	this->name = "Mr. noname";
	this->setOrgname("Mr. noname");
	this->title_ = "";

	this->setAntispamtimer(0);//LB - anti spam

	this->setUnicode(true); // This is set to 1 if the player uses unicode speech, 0 if not
	this->setAccount(-1);
	this->pos.x=100;
	this->pos.y=100;
	this->pos.z=this->dispz=0;
	
//	this->oldpos.x=0; // fix for jail bug
//	this->oldpos.y=0; // fix for jail bug
//	this->oldpos.z=0; // LB, experimental, change back to unsignbed if this give sproblems
	this->race=0; // -Fraz- Race AddOn
	this->dir=0; //&0F=Direction
	this->xid = 0x0190;
	this->setId(0x0190);
	this->setSkin(0); // Skin color
	this->setXSkin(0); // Skin color
	this->setPriv(0);	// 1:GM clearance, 2:Broadcast, 4:Invulnerable, 8: single click serial numbers
	// 10: Don't show skill titles, 20: GM Pagable, 40: Can snoop others packs, 80: Counselor clearance
	this->priv2=0;	// 1:Allmove, 2: Frozen, 4: View houses as icons, 8: permanently hidden
	// 10: no need mana, 20: dispellable, 40: permanent magic reflect, 80: no need reagents
	this->setPrivLvl("");  // needed for special privs
	this->fonttype=3; // Speech font to use
	this->saycolor=0x1700; // Color for say messages
	this->emotecolor = 0x0023; // Color for emote messages
	this->st=50; // Strength
	this->st2=0; // Reserved for calculation
	this->dx=50; // Dexterity
	this->dx2=0; // Reserved for calculation
	this->tmpDex=0; // Reserved for calculation
	this->in=50; // Intelligence
	this->in2=0; // Reserved for calculation
	this->hp=50; // Hitpoints
	this->stm=50; // Stamina
	this->mn=50; // Mana
	this->mn2=0; // Reserved for calculation
	this->hidamage=0; //NPC Damage
	this->lodamage=0; //NPC Damage
	this->jailtimer=0; //blackwinds jail system 
    this->jailsecs=0;
	
	this->setCreationDay(getPlatformDay());
	for (i=0;i<TRUESKILLS;i++)
	{
		this->setBaseSkill(i, 0);
		this->setSkill(i, 0);
	}
	this->npc=false;
	this->shop=false; //1=npc shopkeeper
	this->cell=0; // Reserved for jailing players 
	            // bugfix, LB 0= player not in jail !, not -1
	
	this->jailtimer=0; //blackwinds jail system
	this->jailsecs=0;

	this->ownserial=INVALID_SERIAL; // If Char is an NPC, this sets its owner
	this->setTamed(false); // True if NPC is tamed
	this->robe = -1; // Serial number of generated death robe (If char is a ghost)
	this->karma=0;
	this->fame=0;
	this->pathnum = PATHNUM;
	this->kills=0; // PvP Kills
	this->deaths=0;
	this->dead = false; // Is character dead
	this->packitem=INVALID_SERIAL; // Only used during character creation
	this->fixedlight=255; // Fixed lighting level (For chars in dungeons, where they dont see the night)
	// changed to -1, LB, bugfix
	this->speech=0; // For NPCs: Number of the assigned speech block
	this->weight=0; //Total weight
	this->att=0; // Intrinsic attack (For monsters that cant carry weapons)
	this->def=0; // Intrinsic defense
	this->war=false; // War Mode
	this->targ=INVALID_SERIAL; // Current combat target
	this->timeout=0; // Combat timeout (For hitting)
	this->timeout2=0;
	this->regen=0;
	this->regen2=0;
	this->regen3=0;//Regeneration times for mana, stamin, and str
	this->inputmode = enNone;
	this->inputitem = INVALID_SERIAL;
	this->attacker = INVALID_SERIAL; // Character's serial who attacked this character
	this->npcmovetime=0; // Next time npc will walk
	this->npcWander=0; // NPC Wander Mode
	this->oldnpcWander=0; // Used for fleeing npcs
	this->ftarg = INVALID_SERIAL; // NPC Follow Target
	this->fx1=-1; //NPC Wander Point 1 x or Deed's Serial
	this->fx2=-1; //NPC Wander Point 2 x
	this->fy1=-1; //NPC Wander Point 1 y
	this->fy2=-1; //NPC Wander Point 2 y
	this->fz1=0; //NPC Wander Point 1 z
	this->setSpawnSerial( INVALID_SERIAL ); // Spawned by
	this->setHidden(0); // 0 = not hidden, 1 = hidden, 2 = invisible spell
	this->setInvisTimeout(0);
	this->resetAttackFirst(); // 0 = defending, 1 = attacked first
	this->setOnHorse(false); // On a horse?
	this->setHunger(6);  // Level of hungerness, 6 = full, 0 = "empty"
	this->setHungerTime(0); // Timer used for hunger, one point is dropped every 20 min
	this->setSmeltItem( INVALID_SERIAL );
	this->setTailItem( INVALID_SERIAL );
	this->setNpcAIType(0); // NPC ai
	this->setCallNum(-1); //GM Paging
	this->setPlayerCallNum(-1); //GM Paging
	this->region=255;
	this->skilldelay=0;
	this->objectdelay=0;
	this->combathitmessage=0;
	this->making=-1; // skill number of skill using to make item, 0 if not making anything.
	this->blocked=0;
	this->dir2=0;
	this->spiritspeaktimer=0; // Timer used for duration of spirit speak
	this->spattack=0;
	this->spadelay=0;
	this->spatimer=0;
	this->taming=0; //Skill level required for taming
	this->summontimer=0; //Timer for summoned creatures.
	this->trackingtimer=0; // Timer used for the duration of tracking
	this->trackingtarget = INVALID_SERIAL; // Tracking target ID
	for (i=0;i<MAXTRACKINGTARGETS;i++)
		this->trackingtargets[i]=0;
	this->setFishingtimer(0); // Timer used to delay the catching of fish

	this->setAdvobj(0); //Has used advance gate?
	
	this->setPoison(0); // used for poison skill 
	this->setPoisoned(0); // type of poison
	this->setPoisontime(0); // poison damage timer
	this->setPoisontxt(0); // poision text timer
	this->setPoisonwearofftime(0); // LB, makes poision wear off ...
	
	this->setFleeat(SrvParams->npc_base_fleeat());
	this->setReattackat(SrvParams->npc_base_reattackat());
	this->setTrigger(0); //Trigger number that character activates
	this->setTrigword(QString::null);
	this->setDisabled(0); //Character is disabled for n cicles, cant trigger.
	this->setDisabledmsg(QString::null); //Character disabled message. -- by Magius(CHE) §
	this->setEnvokeid(0x00); //ID of item user envoked
	this->setEnvokeitem(INVALID_SERIAL);
	this->setSplit(0);
	this->setSplitchnc(0);
	this->setTargtrig(0); //Stores the number of the trigger the character for targeting
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
	this->setCarve(-1); // AntiChrist - for new carving system
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

P_ITEM cChar::GetBankBox( short banktype )			
{
	P_ITEM pi;
	unsigned int ci=0;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if( pi->type() == 1 && pi->morex == 1 )
			if ( banktype == 1 && pi->morez == 123 && SrvParams->useSpecialBank()) 
				return pi;
			else if ( banktype != 1 || !SrvParams->useSpecialBank())
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
	if(SrvParams->useSpecialBank() && banktype == 1)//AntiChrist - Special Bank
		pi->morey=123;//gold only bank
	pi->setType( 1 );
	if (s != -1)
		wearIt(s, pi);

	return pi;
}


///////////////////////
// Name:	disturbMed
// history:	by Duke, 17.3.2001
// Purpose:	stops meditation if necessary. Displays message if a socket is passed

void cChar::disturbMed(UOXSOCKET s)
{
	if (this->med()) //Meditation
	{
		this->setMed( false );
		if (s != INVALID_UOXSOCKET)
			sysmessage(s, "You break your concentration.");
	}
}

///////////////////////
// Name:	unhide
// history:	by Duke, 17.3.2001
// Purpose:	reveals the char if he was hidden

void cChar::unhide()
{
	if (this->isHidden() && !(this->priv2&8))	//if hidden but not permanently
	{
		this->setStealth(-1);
		this->setHidden(0);
		updatechar(this);	// LB, necassary for client 1.26.2
		if (this->isGM())
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
		this->npcmovetime=(unsigned int)((uiCurrentTime+double(NPCSPEED*MY_CLOCKS_PER_SEC/5)));
	else if(this->war)
		this->npcmovetime=(unsigned int)((uiCurrentTime+double(NPCSPEED*MY_CLOCKS_PER_SEC/5)));
	else
		this->npcmovetime=(unsigned int)((uiCurrentTime+double(NPCSPEED*MY_CLOCKS_PER_SEC)));
}

///////////////////////
// Name:	fight
// history:	by Duke, 20.3.2001
// Purpose:	makes a character fight the other
//
void cChar::fight(P_CHAR other)
{
	this->targ = other->serial;
	this->unhide();
	this->disturbMed();	// Meditation
	this->attacker = other->serial;
	if (this->isNpc())
	{
		if (!this->war)
			npcToggleCombat(this);
		this->setNextMoveTime();
	}
}


cChar::cChar()
{
	VisRange = VISRANGE ;
}
///////////////////////
// Name:	CountItems
// history:	by Duke, 26.3.2001
// Purpose:	searches the character recursively,
//			counting the items of the given ID and (if given) color

int cChar::CountItems(short ID, short col)
{
	P_ITEM pi=this->getBackpack();
		
	int number = 0 ;
	if (pi != NULL)
		number = pi->CountItems(ID, col);
	return number ;
}

int cChar::CountBankGold()
{
	P_ITEM pi = GetBankBox(1); //we want gold bankbox.
	if (!pi) return 0;
	return pi->CountItems(0x0EED);
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
			pHalo->pos.x=pi->pos.x;
			pHalo->pos.y=pi->pos.y;
			pHalo->pos.z=pi->pos.z;
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
		RefreshItem(pHalo);//AntiChrist
	}
}

///////////////////////
// Name:	getWeapon
// history:	moved here from combat.cpp by Duke, 20.5.2001
// Purpose:	finds the equipped weapon of a character

P_ITEM cChar::getWeapon()
{
	unsigned int ci=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi != NULL)
		if ( ( pi->layer() == 1 && pi->type() != 9 )		// not a spellbook (hozonko)
			|| (pi->layer() == 2 && !getShield()) ) //Morrolan don't check for shields
		{
			return pi;
		}
	}
	return NULL;
}

///////////////////////
// Name:	getShield
// history:	by Duke, 20.5.2001
// Purpose:	finds the equipped shield of a character

P_ITEM cChar::getShield()
{
	P_ITEM pi=GetItemOnLayer(2);
	if (pi && IsShield(pi->id()) )
		return pi;
	else
		return NULL;
}

P_ITEM Packitem(P_CHAR pc) // Find packitem
{
	if(pc == NULL) 
		return NULL;
	P_ITEM pi = FindItemBySerial(pc->packitem);
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
			pc->packitem = pi->serial;	//Record it for next time
			return (pi);
		}
	}
	return NULL;
}

P_ITEM cChar::getBackpack()	{return Packitem(this);}

///////////////////////
// Name:	setters for various serials
// history:	by Duke, 2.6.2001
// Purpose:	encapsulates revoval/adding to the pointer arrays

void cChar::setOwnSerialOnly(long ownser)
{
	ownserial=ownser;
}

void cChar::SetOwnSerial(long ownser)
{
	if (ownserial != INVALID_SERIAL)	// if it was set, remove the old one
		cownsp.remove(ownserial, serial);
	
	setOwnSerialOnly(ownser);
	if (ownser != serial && ownser != INVALID_SERIAL)
		tamed_ = true;
	else
		tamed_ = false;

	if (ownser != INVALID_SERIAL)		// if there is an owner, add it
		cownsp.insert(ownserial, serial);
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
	if (multis != INVALID_SERIAL)	// if it was set, remove the old one
		cmultisp.remove(multis, this->serial);

	this->multis = mulser;

	if (mulser != INVALID_SERIAL)		// if there is multi, add it
		cmultisp.insert(multis, this->serial);
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

	mapRegions->Remove(this);
	pos.x = newx;
	pos.y = newy;
	pos.z = dispz=newz;
	mapRegions->Add(this);
}

unsigned int cChar::getSkillSum()
{
	register unsigned int sum = 0, a;
	for (a=0;a<ALLSKILLS;a++)
	{
		sum+=this->baseSkill_[a];
	}
	return sum;		// this *includes* the decimal digit ie. xxx.y
}

///////////////////////
// Name:	getTeachingDelta
// history:	by Duke, 27.7.2001
// Purpose:	calculates how much the given player can learn from this teacher

int cChar::getTeachingDelta(cChar* pPlayer, int skill, int sum)
{
	int delta = min(250,this->baseSkill(skill)/2);		// half the trainers skill, but not more than 250
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
	this->st -= pi->st2;
	this->chgDex(-1 * pi->dx2);
	this->in -= pi->in2;
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

	if (this->priv2&1)	// allmove
		return true;

	if ( (pi->isOwnerMovable() || pi->isLockedDown()) && !this->Owns(pi) )	// owner movable or locked down ?
		return false;

	tile_st tile;
	Map->SeekTile(pi->id(), &tile);
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
		archive.read("account",			account_);
		archive.read("creationday",		creationday_);
		archive.read("gmmoveeff",		gmMoveEff);
		archive.read("guildtype",		GuildType);
		archive.read("guildtraitor",	GuildTraitor);
		archive.read("dispz",			dispz);
		archive.read("cell",			cell);
		archive.read("dir",				dir);
		archive.read("race",			race);
		archive.read("body",			xid);	setId(xid);
		archive.read("xbody",			xid);
		archive.read("skin",			skin_);	
		archive.read("xskin",           xskin_);
		archive.read("priv",			priv);
		
		archive.read("privlvl",			privlvl_);
		
		archive.read("stablemaster",	stablemaster_serial_);
		archive.read("npctype",			npc_type_);
		archive.read("time_unused",		time_unused_);
		
		archive.read("allmove",			priv2);
		archive.read("font",			fonttype);
		archive.read("say",				saycolor);
		archive.read("emote",			emotecolor);
		archive.read("strength",		st);
		archive.read("strength2",		st2);
		archive.read("dexterity",		dx);
		archive.read("dexterity2",		dx2);
		archive.read("intelligence",	in);
		archive.read("intelligence2",	in2);
		archive.read("hitpoints",		hp);
		archive.read("spawnregion",		spawnregion_);
		archive.read("stamina",			stm);
		archive.read("mana",			mn);
		archive.read("npc",				npc);
		archive.read("holdgold",		holdg_);
		archive.read("shop",			shop);
		archive.read("own",				ownserial);
		archive.read("robe",			robe);
		archive.read("karma",			karma);
		archive.read("fame",			fame);
		archive.read("kills",			kills);
		archive.read("deaths",			deaths);
		archive.read("dead",			dead);
		archive.read("packitem",		packitem);
		archive.read("fixedlight",		fixedlight);
		archive.read("speech",			speech);
		archive.read("trigger",			trigger_);
		archive.read("trigword",		trigword_);
		archive.read("disablemsg",		disabledmsg_);
		unsigned int j;
		for (j=0;j<TRUESKILLS;j++)
		{
			char t[256] = {0,};
			numtostr(j, t);
			string temp = string("skill") + string(t);
			archive.read(temp.c_str(), baseSkill_[j]);
			temp = string("skl") + string(t);
			archive.read(temp.c_str(), lockSkill[j] );
		}
		archive.read("cantrain", cantrain_);
		
		archive.read("att",				att);
		archive.read("def",				def);
		archive.read("lodamage",		lodamage);
		archive.read("hidamage",		hidamage);
		archive.read("war",				war);
		archive.read("npcwander",		npcWander);
		archive.read("oldnpcwander",	oldnpcWander);
		archive.read("carve",			carve_);
		archive.read("fx1",				fx1);
		archive.read("fy1",				fy1);
		archive.read("fz1",				fz1);
		archive.read("fx2",				fx2);
		archive.read("fy2",				fy2);
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
		archive.read("jailtimer",		jailtimer);
		if (jailtimer != 0)
			jailtimer += uiCurrentTime;
		archive.read("jailsecs",		jailsecs); 
		archive.read("gmrestrict",		gmrestrict_);
		SetOwnSerial(ownserial);
		SetSpawnSerial(spawnserial_);
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
		archive.write("account",		account_);
		archive.write("creationday",	creationday_);
		archive.write("gmmoveeff",		gmMoveEff);
		archive.write("guildtype",		GuildType);
		archive.write("guildtraitor",	GuildTraitor);
		archive.write("dispz",			dispz);
		archive.write("cell",			cell);
		archive.write("dir",			dir);
		archive.write("race",			race);
		//AntiChrist - incognito and polymorph spell special stuff - 12/99
		if(incognito() || polymorph())
		{//if under incognito spell, don't save BODY but the original XBODY
			archive.write("body", xid);
		} 
		else
		{//else backup body normally
			archive.write("body", id());
		}
		archive.write("xbody", xid);
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
		
		archive.write("privlvl",			privlvl_);
		// end of meta-gm save
		
		archive.write("stablemaster",	stablemaster_serial_);
		archive.write("npctype",		npc_type_);
		archive.write("time_unused",	time_unused_);
		
		archive.write("allmove",		priv2);
		archive.write("font",			fonttype);
		archive.write("say",			saycolor);
		archive.write("emote",			emotecolor);
		archive.write("strength",		st);
		archive.write("strength2",		st2);
		archive.write("dexterity",		dx);
		archive.write("dexterity2",		dx2);
		archive.write("intelligence",	in);
		archive.write("intelligence2",	in2);
		archive.write("hitpoints",		hp);
		archive.write("spawnregion",	spawnregion_);
		archive.write("stamina",		stm);
		archive.write("mana",			mn);
		archive.write("npc",			npc);
		archive.write("holdgold",		holdg_);
		archive.write("shop",			shop);
		archive.write("own",			ownserial);
		archive.write("robe",			robe);
		archive.write("karma",			karma);
		archive.write("fame",			fame);
		archive.write("kills",			kills);
		archive.write("deaths",			deaths);
		archive.write("dead",			dead);
		archive.write("packitem",		packitem);
		archive.write("fixedlight",		fixedlight);
		archive.write("speech",			speech);
		archive.write("trigger",		trigger_);
		archive.write("trigword",		trigword_);
		archive.write("disablemsg",		disabledmsg_);
		unsigned int j;
		for (j=0;j<TRUESKILLS;j++)
		{
			char t[256] = {0,};
			numtostr(j, t);
			string temp = string("skill") + string(t);
			archive.write(temp.c_str(), baseSkill_[j]);
			temp = string("skl") + string(t);
			archive.write(temp.c_str(), lockSkill[j] );
		}
		archive.write("cantrain", cantrain_);
		
		archive.write("att",			att);
		archive.write("def",			def);
		archive.write("lodamage",		lodamage);
		archive.write("hidamage",		hidamage);
		archive.write("war",			war);
		archive.write("npcwander",		npcWander);
		archive.write("oldnpcwander",	oldnpcWander);
		archive.write("carve",			carve_);
		archive.write("fx1",			fx1);
		archive.write("fy1",			fy1);
		archive.write("fz1",			fz1);
		archive.write("fx2",			fx2);
		archive.write("fy2",			fy2);
		archive.write("spawn",			spawnserial_);
		archive.write("hidden",			hidden_);
		archive.write("hunger",			hunger_);
		archive.write("npcaitype",		npcaitype_);
		archive.write("spattack",		spattack);
		archive.write("spadelay",		spadelay);
		archive.write("taming",			taming);
		archive.write("summonremainingseconds", summontimer/MY_CLOCKS_PER_SEC);
		
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
		archive.write("jailtimer",		jailtimer/MY_CLOCKS_PER_SEC); 
		archive.write("jailsecs",		jailsecs); 
		archive.write("gmrestrict",		gmrestrict_);
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
		if( this->packitem == INVALID_SERIAL )
		{
			P_ITEM pBackpack = Items->SpawnItem( -1, this, 1, "Backpack", 0, 0x0E,0x75,0,0,0);
			if( pBackpack == NULL )
			{
				Npcs->DeleteChar( this );
				return;
			}
			
			this->packitem = pBackpack->serial;
			
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
		this->setCarve( Value.toInt() );

	//<cantrain />
	else if( TagName == "cantrain" )
		this->setCantrain( true );

	//<direction>SE</direction>
	else if( TagName == "direction" )
	{
		if( Value == "NE" )
			this->dir=1;
		else if( Value == "E" )
			this->dir=2;
		else if( Value == "SE" )
			this->dir=3;
		else if( Value == "S" )
			this->dir=4;
		else if( Value == "SW" )
			this->dir=5;
		else if( Value == "W" )
			this->dir=6;
		else if( Value == "NW" )
			this->dir=7;
		else if( Value == "N" )
			this->dir=0;
	}

	//<stat type="str">100</stats>
	else if( TagName == "stat" )
	{
		if( Tag.attributes().contains("type") )
		{
			QString statType = Tag.attribute("type");
			if( statType == "str" )
			{
				this->st = Value.toShort();
				this->st2 = this->st;
				this->hp = this->st;
			}
			else if( statType == "dex" )
			{
				this->setDex( Value.toShort() );
				this->stm = this->realDex();
			}
			else if( statType == "int" )
			{
				this->in = Value.toShort();
				this->in2 = this->in;
				this->mn = this->in;
			}
		}
	}

	//<defense>10</defense>
	else if( TagName == "defense" )
		this->def = Value.toUInt();

	//<attack>10</attack>
	else if( TagName == "attack" )
		this->att = Value.toUInt();

	//<emotecolor>0x482</emotecolor>
	else if( TagName == "emotecolor" )
		this->emotecolor = Value.toUShort();

	//<fleeat>10</fleeat>
	else if( TagName == "fleeat" )
		this->setFleeat( Value.toShort() );

	//<fame>8000</fame>
	else if( TagName == "fame" )
		this->fame = Value.toInt();

	//<gold>100</gold>
	else if( TagName == "gold" )
	{
		if( this->packitem != INVALID_SERIAL )
		{
			P_ITEM pGold = Items->SpawnItem(this,1,"#",1,0x0EED,0,1);
			if(pGold == NULL)
			{
				Npcs->DeleteChar(this);
				return;
			}
			pGold->priv |= 0x01;

			pGold->setAmount( Value.toInt() );
		}
	}

	//<hidamage>10</hidamage>
	else if( TagName == "hidamage" )
		this->hidamage = Value.toInt();

#pragma note("Hair color tag not yet implemented!")
/*
	//<haircolor>2</haircolor> (colorlist)
	else if( TagName == "haircolor" )
	{
		unsigned short haircolor = addrandomhaircolor(this, (this*)Value.latin1());
		if( haircolor != -1 )
			this->setHairColor( haircolor );
	}
*/

	//<id>0x11</id>
	else if( TagName == "id" )
	{
		bool* ok = false;
		this->setId( Value.toInt( ok, 16 ) );
		this->xid = this->id();
	}

	//<karma>-500</karma>
	else if( TagName == "karma" )
		this->karma = Value.toInt();

	//<loot>3</loot>
	else if( TagName == "loot" )
	{
		if( this->packitem != INVALID_SERIAL )
			Npcs->AddRandomLoot( FindItemBySerial(this->packitem), (char*)Value.latin1() );
	}

	//<lodamage>10</lodamage>
	else if( TagName == "lodamage" )
		this->lodamage = Value.toInt();

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
					this->npcWander = 3;
					this->fx1 = this->pos.x + Tag.attribute("x1").toInt();
					this->fx2 = this->pos.x + Tag.attribute("x2").toInt();
					this->fy1 = this->pos.y + Tag.attribute("y1").toInt();
					this->fy2 = this->pos.y + Tag.attribute("y2").toInt();
					this->fz1 = -1;
				}
			else if( wanderType == "circle" || wanderType == "2" )
			{
				this->npcWander = 2;
				this->fx1 = this->pos.x;
				this->fy1 = this->pos.y;
				this->fz1 = this->pos.z;
				if( Tag.attributes().contains("radius") )
					this->fx2 = Tag.attribute("radius").toInt();
				else
					this->fx2 = 2;
			}
			else if( wanderType == "free" || wanderType == "1" )
				this->npcWander = 1;
			else
				this->npcWander = 0; //default
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
		this->priv2 = Value.toUShort();
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
		Commands->MakeShop( this );
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
		this->speech = Value.toUShort();

	//<split>1</split>
	else if( TagName == "split" )
		this->setSplit( Value.toUShort() );

	//<splitchance>10</splitchance>
	else if( TagName == "splitchance" )
		this->setSplitchnc( Value.toUShort() );

	//<saycolor>0x110</saycolor>
	else if( TagName == "saycolor" )
		this->saycolor = Value.toUShort();

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

	//<trigger>3</trigger>
	else if( TagName == "trigger" )
		this->setTrigger( Value.toInt() );

	//<trigword>abc</trigword>
	else if( TagName == "trigword" )
		this->setTrigword( Value );

	//<skill type="alchemy">100</skill>
	//<skill type="1">100</skill>
	else if( TagName == "skill" && Tag.attributes().contains("type") )
	{
		if( Tag.attribute("type").toInt() > 0 &&
			Tag.attribute("type").toInt() <= ALLSKILLS )
			this->setBaseSkill((Tag.attribute("type").toInt() - 1), Value.toInt());
		else
		{
			for( UI32 j = 0; j < ALLSKILLS; j++ )
			{
				if( Tag.attribute("type").contains( QString(skillname[j]), false ) )
					this->setBaseSkill(j, Value.toInt());
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
		while( !childNode.isNull() )
		{
			if( childNode.nodeName() == "item" && childNode.attributes().contains("id") )
			{
				P_ITEM nItem = Items->createScriptItem( childNode.toElement().attributeNode("id").nodeValue() );
				if( nItem != NULL )
				{
					if( nItem->layer() == 0 )
						Items->DeleItem( nItem );
					else
					{
						nItem->setContSerial( this->serial );

						if( childNode.hasChildNodes() )  // color
						nItem->applyDefinition( childNode.toElement() );
					}
				}
			}
			childNode = childNode.nextSibling();
		}
	}

	else if( TagName == "inherit" && Tag.attributes().contains( "id" ) )
	{
		QDomElement* DefSection = DefManager->getSection( WPDT_NPC, Tag.attribute( "id" ) );
		if( !DefSection->isNull() )
			this->applyDefinition( *DefSection );
	}

	else
		cUObject::processNode( Tag );
}
