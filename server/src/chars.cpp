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
#include "chars.h"
#include "charsmgr.h"
#include "items.h"
#include "debug.h"
#include "globals.h"
#include "wolfpack.h"

// Inline members

bool cChar::Owns(P_ITEM pi)			{	return (serial==pi->ownserial);		}
bool cChar::Wears(P_ITEM pi)		{	return (serial == pi->contserial);	}
unsigned int cChar::dist(cChar* pc)	{	return pos.distance(pc->pos);		}
unsigned int cChar::dist(cItem* pi)	{	return pos.distance(pi->pos);		}
bool cChar::inGuardedArea()			{	return ::region[this->region].priv&1;}


void cChar::setSerial(SERIAL ser)
{
	this->ser1 = static_cast<unsigned char>(ser>>24); // Character serial number
	this->ser2 = static_cast<unsigned char>(ser>>16);
	this->ser3 = static_cast<unsigned char>(ser>>8);
	this->ser4 = static_cast<unsigned char>(ser%256);
	this->serial = ser;
	if ( this->serial != INVALID_SERIAL)
		cCharsManager::getCharsManager().registerChar(this);
}

void cChar::Init(bool ser)
{
	unsigned int i;

	if (ser)
	{
		this->setSerial(cCharsManager::getCharsManager().getUnusedSerial());
	}
	else
	{
		this->ser1=0;
		this->ser2=0;
		this->ser3=0;
		this->ser4=0;
		this->serial = INVALID_SERIAL;
	}
	this->multis=-1;//Multi serial
	this->free = false;
	strcpy(this->name,"Mr. noname");
	strcpy(this->orgname,"Mr. noname");
	this->title[0]=0x00;

	this->antispamtimer=0;//LB - anti spam

	this->unicode = false; // This is set to 1 if the player uses unicode speech, 0 if not
	this->account=-1;
	this->pos.x=100;
	this->pos.y=100;
	this->pos.z=this->dispz=0;
	
//	this->oldpos.x=0; // fix for jail bug
//	this->oldpos.y=0; // fix for jail bug
//	this->oldpos.z=0; // LB, experimental, change back to unsignbed if this give sproblems
	this->race=0; // -Fraz- Race AddOn
	this->dir=0; //&0F=Direction
	this->id1=this->xid1=0x01; // Character body type
	this->id2=this->xid2=0x90; // Character body type
	this->skin = this->xskin = 0x0000; // Skin color
	this->keynumb=-1;  // for renaming keys 
	this->setPriv(0);	// 1:GM clearance, 2:Broadcast, 4:Invulnerable, 8: single click serial numbers
	// 10: Don't show skill titles, 20: GM Pagable, 40: Can snoop others packs, 80: Counselor clearance
	this->priv2=0;	// 1:Allmove, 2: Frozen, 4: View houses as icons, 8: permanently hidden
	// 10: no need mana, 20: dispellable, 40: permanent magic reflect, 80: no need reagents
	for (i=0;i<7;i++)
		this->priv3[i]=0;  // needed for Lord bianrys meta-gm stuff
	this->fonttype=3; // Speech font to use
	this->saycolor=0x1700; // Color for say messages
	this->emotecolor1=0x00; // Color for emote messages
	this->emotecolor2=0x23; // Color for emote messages
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
	
	this->creationday = getPlatformDay() ;
	for (i=0;i<TRUESKILLS;i++)
	{
		this->baseskill[i]=0;
		this->skill[i]=0;
	}
	this->npc=0;
	this->shop=0; //1=npc shopkeeper
	this->cell=0; // Reserved for jailing players 
	            // bugfix, LB 0= player not in jail !, not -1
	
	this->jailtimer=0; //blackwinds jail system
	this->jailsecs=0;

	this->ownserial=-1; // If Char is an NPC, this sets its owner
	this->tamed = false; // True if NPC is tamed
	this->robe = -1; // Serial number of generated death robe (If char is a ghost)
	this->karma=0;
	this->fame=0;
	this->pathnum = PATHNUM;
	this->kills=0; //PvP Kills
	this->deaths=0;
	this->dead = false; // Is character dead
	this->packitem=INVALID_SERIAL; // Only used during character creation
	this->fixedlight=255; // Fixed lighting level (For chars in dungeons, where they dont see the night)
	// changed to -1, LB, bugfix
	this->speech=0; // For NPCs: Number of the assigned speech block
	this->weight=0; //Total weight
	this->att=0; // Intrinsic attack (For monsters that cant carry weapons)
	this->def=0; // Intrinsic defense
	this->war=0; // War Mode
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
	this->spawnserial=-1; // Spawned by
	this->hidden=0; // 0 = not hidden, 1 = hidden, 2 = invisible spell
	this->invistimeout=0;
	this->resetAttackFirst(); // 0 = defending, 1 = attacked first
	this->onhorse=false; // On a horse?
	this->hunger=6;  // Level of hungerness, 6 = full, 0 = "empty"
	this->hungertime=0; // Timer used for hunger, one point is dropped every 20 min
	this->smeltitem= INVALID_SERIAL;
	this->tailitem = INVALID_SERIAL;
	this->npcaitype=0; // NPC ai
	this->callnum=-1; //GM Paging
	this->playercallnum=-1; //GM Paging
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
	this->fishingtimer=0; // Timer used to delay the catching of fish

	this->advobj=0; //Has used advance gate?
	
	this->poison=0; // used for poison skill 
	this->poisoned=0; // type of poison
	this->poisontime=0; // poison damage timer
	this->poisontxt=0; // poision text timer
	this->poisonwearofftime=0; // LB, makes poision wear off ...
	
	this->fleeat=SrvParms->npc_base_fleeat;
	this->reattackat=SrvParms->npc_base_reattackat;
	this->trigger=0; //Trigger number that character activates
	this->trigword[0]='\x00'; //Word that character triggers on.
	this->disabled=0; //Character is disabled for n cicles, cant trigger.
	this->disabledmsg[0] = 0; //Character disabled message. -- by Magius(CHE) §
	this->envokeid1=0x00; //ID1 of item user envoked
	this->envokeid2=0x00; //ID2 of item user envoked
	this->envokeitem = INVALID_SERIAL;
	this->split=0;
	this->splitchnc=0;
	this->targtrig=0; //Stores the number of the trigger the character for targeting
	this->ra=0;  // Reactive Armor spell
	this->trainer=0; // Serial of the NPC training the char, -1 if none.
	this->trainingplayerin=0; // Index in skillname of the skill the NPC is training the player in
	this->cantrain=true;
	// Begin of Guild Related Character information (DasRaetsel)
	this->guildtoggle=0;		// Toggle for Guildtitle								(DasRaetsel)
	this->guildtitle[0]='\x00';	// Title Guildmaster granted player						(DasRaetsel)
	this->guildfealty=-1;		// Serial of player you are loyal to (default=yourself)	(DasRaetsel)
	this->guildnumber=0;		// Number of guild player is in (0=no guild)			(DasRaetsel)
	this->GuildTraitor=false; 
	//this->flag=0x04; //1=red 2=grey 4=Blue 8=green 10=Orange
	this->flag=0x02; //1=red 2=grey 4=Blue 8=green 10=Orange // grey as default - AntiChrist
	this->tempflagtime=0;
	// End of Guild Related Character information
	this->murderrate=0; //#of ticks until one murder decays //REPSYS 
	this->crimflag=-1; //Time when No longer criminal -1=Not Criminal
	this->casting=0; // 0/1 is the cast casting a spell?
	this->spelltime=0; //Time when they are done casting....
	this->spell=0; //current spell they are casting....
	this->spellaction=0; //Action of the current spell....
	this->nextact=0; //time to next spell action....
	this->poisonserial=-1; //AntiChrist -- poisoning skill
	
	this->squelched=0; // zippy  - squelching
	this->mutetime=0; //Time till they are UN-Squelched.
	this->med=0; // 0=not meditating, 1=meditating //Morrolan - Meditation 
	this->stealth=-1; //AntiChrist - stealth ( steps already done, -1=not using )
	this->running=0; //AntiChrist - Stamina Loose while running
	this->logout=0;//Time till logout for this char -1 means in the world or already logged out //Instalog
	this->swingtarg=-1; //Tagret they are going to hit after they swing
	this->holdg=0; // Gold a player vendor is holding for Owner
	this->fly_steps=0; //LB -> used for flyging creatures
	this->menupriv=0; // Lb -> menu priv
	this->guarded=false; // True if CHAR is guarded by some NPC
	this->smoketimer=0;
	this->smokedisplaytimer=0;
	this->carve=-1; // AntiChrist - for new carving system
	this->antiguardstimer=0; // AntiChrist - for "GUARDS" call-spawn
	this->polymorph=false;//polymorph - AntiChrist
	this->incognito=false;//incognito - AntiChrist
    this->postType = LOCALPOST;
    this->questDestRegion = 0;
    this->questOrigRegion= 0;
    this->questBountyReward= 0;
    this->questBountyPostSerial = INVALID_SERIAL;
    this->murdererSer = 0;
    this->spawnregion = 0;
    this->npc_type = 0;
    this->stablemaster_serial = INVALID_SERIAL;
	this->timeused_last = getNormalizedTime();
	this->time_unused = 0;

	for (i=0;i<TRUESKILLS;i++)
	{
		this->baseskill[i]=0;
		this->skill[i]=0;
	}
	for (i = 0; i < ALLSKILLS; i++) 
		this->lockSkill[i]=0;
}

///////////////////////
// Name:	cChar::day
// history:	by punt, 15.4.2001
// Purpose:	return the day this was created
//
unsigned long cChar::day()
{
	return creationday ;
}
///////////////////////
// Name:	cChar::day(unsigned long)
// history:	by punt, 15.4.2001
// Purpose:	set the day this was created
//
void cChar::day(unsigned long CreateDay)
{
	creationday = CreateDay ;
}

///////////////////////
// Name:	GetItemOnLayer
// history:	by Duke, 26.3.2001
// Purpose:	returns the item on the given layer, if any
//
P_ITEM cChar::GetItemOnLayer(unsigned char layer)
{
	P_ITEM pi;
	int ci;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->layer==layer)
			return pi;
	}
	return NULL;
}

///////////////////////
// Name:	GetItemOnLayer
// history:	by Duke, 26.3.2001, touched by Correa, 21.04.2001
// Purpose:	Return the bank box. If banktype == 1, it will return the Item's bank box, else, 
//          gold bankbox is returned. 
//
P_ITEM cChar::GetBankBox( short banktype )			
{
	P_ITEM pi;
	unsigned int ci=0;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->type == 1 && pi->morex == 1)
			if ( banktype == 1 && pi->morez == 123 && SrvParms->usespecialbank) 
				return pi;
			else if ( banktype != 1 || !SrvParms->usespecialbank)
				return pi;
	}
	// If we reach this point, bankbox wasn't found == wasn't created yet.

	sprintf((char*)temp, "%s's bank box.", name);
	UOXSOCKET s = calcSocketFromChar(this);
	pi = Items->SpawnItem(this, 1, (char*)temp, 0, 0x09AB, 0, 0);
	if(pi == NULL) 
		return NULL;
	pi->layer=0x1d;
	pi->SetOwnSerial(this->serial);
	pi->SetContSerial(this->serial);
	pi->morex=1;
	if(SrvParms->usespecialbank && banktype == 1)//AntiChrist - Special Bank
		pi->morey=123;//gold only bank
	pi->type=1;
	if (s != -1)
		wearIt(s, pi);

	return pi;
}


///////////////////////
// Name:	disturbMed
// history:	by Duke, 17.3.2001
// Purpose:	stops meditation if necessary. Displays message if a socket is passed
//
void cChar::disturbMed(UOXSOCKET s)
{
	if (this->med) //Meditation
	{
		this->med=0;
		if (s != -1)
			sysmessage(s, "You break your concentration.");
	}
}

///////////////////////
// Name:	unhide
// history:	by Duke, 17.3.2001
// Purpose:	reveals the char if he was hidden
//
void cChar::unhide()
{
	if (this->isHidden() && !(this->priv2&8))	//if hidden but not permanently
	{
		this->stealth=-1;
		this->hidden=0;
		updatechar(this);	// LB, necassary for client 1.26.2
		if (this->isGM())
			tempeffect(this, this, 34, 3, 0, 0); 
	}
}

///////////////////////
// Name:	setNextMoveTime
// history:	by Duke, 20.3.2001
// Purpose:	sets the move timer. tamediv can shorten the time for tamed npcs
//
void cChar::setNextMoveTime(short tamediv)
{
//	if ( && this->tamed) return;	// MUST be nonzero
	// let's let them move once in a while ;)
	if(this->tamed)
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

///////////////////////
// Name:	CountItems
// history:	by Duke, 26.3.2001
// Purpose:	searches the character recursively,
//			counting the items of the given ID and (if given) color
//
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
//
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
	if (pi->glow>0)
	{
		P_ITEM pHalo=FindItemBySerial(pi->glow);
		if (!pHalo) return;
		
		pHalo->layer=pi->layer; // copy layer information of the glowing item to the invisible light emitting object
		
		if(pHalo->layer==0 && pi->isInWorld()) // unequipped -> light source coords = item coords
		{
			pHalo->dir=29;
			pHalo->pos.x=pi->pos.x;
			pHalo->pos.y=pi->pos.y;
			pHalo->pos.z=pi->pos.z;
		} else if (pHalo->layer==0 && !pi->isInWorld()) // euqipped -> light source coords = players coords
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
//
P_ITEM cChar::getWeapon()
{
	unsigned int ci=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi != NULL)
		if ((pi->layer==1 && pi->type!=9)		// not a spellbook (hozonko)
			|| (pi->layer==2 && !getShield()) ) //Morrolan don't check for shields
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
//
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
		if (pc->Wears(pi) && pi->layer==0x15)
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
		if (pi != NULL && pi->layer==0x15)
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
//
void cChar::setOwnSerialOnly(long ownser)
{
	ownserial=ownser;
}

void cChar::SetOwnSerial(long ownser)
{
	if (ownserial!=-1)	// if it was set, remove the old one
		cownsp.remove(ownserial, serial);
	
	setOwnSerialOnly(ownser);
	if (ownser != serial && ownser != -1)
		tamed = true;
	else
		tamed = false;

	if (ownser!=-1)		// if there is an owner, add it
		cownsp.insert(ownserial, serial);
}

void cChar::SetSpawnSerial(long spawnser)
{
	if (spawnserial!=-1)	// if it was set, remove the old one
		cspawnsp.remove(spawnserial, serial);

	spawnserial=spawnser;

	if (spawnser!=-1)		// if there is a spawner, add it
		cspawnsp.insert(spawnserial, serial);
}

void cChar::SetMultiSerial(long mulser)
{
	if (multis!=-1)	// if it was set, remove the old one
		cmultisp.remove(multis, this->serial);

	this->multis = mulser;

	if (mulser!=-1)		// if there is multi, add it
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

int cChar::getSkillSum()
{
	int sum=0,a;
	for (a=0;a<ALLSKILLS;a++)
	{
		sum+=this->baseskill[a];
	}
	return sum;		// this *includes* the decimal digit ie. xxx.y
}

///////////////////////
// Name:	getTeachingDelta
// history:	by Duke, 27.7.2001
// Purpose:	calculates how much the given player can learn from this teacher
//
int cChar::getTeachingDelta(cChar* pPlayer, int skill, int sum)
{
	int delta = min(250,this->baseskill[skill]/2);		// half the trainers skill, but not more than 250
	delta -= pPlayer->baseskill[skill];					// calc difference
	if (delta <= 0)
		return 0;

	if (sum+delta >= SrvParms->skillcap * 10)			// would new skill value be above cap ?
		delta = (SrvParms->skillcap * 10) - sum;		// yes, so reduce it
	return delta;
}

////////////
// Name:	removeItemBonus
// history:	by Duke, 19.8.2001
// Purpose:	removes boni given by an item
//
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
//
bool cChar::canPickUp(cItem* pi)
{
	if (!pi)
	{
		LogCritical("bad parm");
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

int cChar::MyHome()
{
	int h;
	h=HouseManager->GetHouseNum(this);
	if(h>=0)
		if(House[h]->OwnerSerial==this->serial)
			return h;
	return -1;
}
