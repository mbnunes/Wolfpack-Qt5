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

#include "wolfpack.h"
#include "itemid.h"
#include "sregions.h"
#include "SndPkg.h"
#include "debug.h"
#include "utilsys.h"

#undef  DBGFILE
#define DBGFILE "npcs.cpp"

#define CHAR_RESERVE 100	// minimum of free slots that should be left in the array.
							// otherwise, more memory will be allocated in the mainloop (Duke)

int addrandomcolor(int s, char *colorlist)
{
	P_CHAR pc_s = MAKE_CHAR_REF(s);
	char sect[512];
	int i,j,storeval;
	i=0; j=0;
	openscript("colors.scp");
	sprintf(sect, "RANDOMCOLOR %s", colorlist);
	if (!i_scripts[colors_script]->find(sect))
	{
		closescript();

		//sprintf(pc_s->name, "Error Colorlist %s Not Found(1)", colorlist);
		// LB: wtf should this do apart from crashing ? copying an error message in a chars name ??
		// very weired! think it should look like this:

		clConsole.send("Error Colorlist %s not found on character: %s\n",colorlist,pc_s->name);

		return 0;
	}
	unsigned long loopexit=0;
	do
	{
		read1();
		if (script1[0]!='}')
		{
			i++;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	closescript();
	if(i>0)
	{
		i=rand()%i;
		i++;
		openscript("colors.scp");
		if(!i_scripts[colors_script]->find(sect))
		{
			closescript();
			//sprintf(pc_s->name, "Error Colorlist %s Not Found(2)", colorlist);
			clConsole.send("Error Colorlist %s not found on character: %s\n",colorlist,pc_s->name);
			return 0;
		}
		loopexit=0;
		do
		{
			read1();
			if (script1[0]!='}')
			{
				j++;
				if(j==i)
				{
					storeval=hex2num(script1);
				}
			}
		}
		while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
		closescript();
	}
	return (storeval);
}

static int addrandomhaircolor(int s, char *colorlist)
{
	P_CHAR pc_s = MAKE_CHAR_REF(s);
	char sect[512];
	int i,j,haircolor = 0x044e;
	i=0; j=0;
	openscript("colors.scp");
	sprintf(sect, "RANDOMCOLOR %s", colorlist);
	if (!i_scripts[colors_script]->find(sect))
	{
		closescript();
		clConsole.send("Error Colorlist %s not found on character: %s\n",colorlist,pc_s->name);
		return 0;
	}
	unsigned long loopexit=0;
	do
	{
		read1();
		if (script1[0]!='}')
		{
			i++;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
	closescript();
	if(i>0)
	{
		i=rand()%i;
		i++;
		openscript("colors.scp");
		if(!i_scripts[colors_script]->find(sect))
		{
			closescript();
			//sprintf(pc_s->name, "Error Colorlist %s Not Found(2)", colorlist);
			clConsole.send("Error Colorlist %s not found on character: %s\n",colorlist,pc_s->name);
			return 0;
		}
		loopexit=0;
		do
		{
			read1();
			if (script1[0]!='}')
			{
				j++;
				if(j==i)
				{
					haircolor=hex2num(script1);
				}
			}
		}
		while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
		closescript();
	}
	return (haircolor);
}

void setrandomname(P_CHAR pc_s, char * namelist)
{
	char sect[512];
	int i=0,j=0;

	sprintf(sect, "RANDOMNAME %s", namelist);
	Script *pScpBase=i_scripts[npc_script];
	Script *pScp=pScpBase->Select(sect,custom_npc_script);
	if (!pScp)
	{
		sprintf(pc_s->name, "Error Namelist %s Not Found", namelist);
		return;
	}

	unsigned long loopexit=0;
	do
	{
		pScp->NextLine();
		if (script1[0]!='}')
		{
			i++;
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
	pScp->Close();

	if(i>0)
	{
		i=rand()%(i);
		pScp=pScpBase->Select(sect,custom_npc_script);
		if (!pScp) return;

		loopexit=0;
		do
		{
			pScp->NextLine();
			if (script1[0]!='}')
			{
				if(j==i)
				{
					strcpy(pc_s->name,(char*)script1);
					break;
				}
				else j++;
			}
		}
		while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
		pScp->Close();
	}
}

void cChar::Init(bool ser)
{
	unsigned int i;

	if (this->serial==charcount) charcount++;

	if (ser)
	{
		this->ser1 = static_cast<unsigned char>(charcount2>>24); // Character serial number
		this->ser2 = static_cast<unsigned char>(charcount2>>16);
		this->ser3 = static_cast<unsigned char>(charcount2>>8);
		this->ser4 = static_cast<unsigned char>(charcount2%256);
		this->serial = charcount2;
		setptr(charsp, charcount2, DEREF_P_CHAR(this));
		charcount2++;
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
    this->questBountyPostSerial = 0;
    this->murdererSer = 0;
    this->spawnregion = 0;
    this->npc_type = 0;
    this->stablemaster_serial = 0;
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

void cCharStuff::DeleteChar (P_CHAR pc_k) // Delete character
{
	int j;//,serial; //Zippy lag
	//int ptr,ci;

	removeitem[1]=pc_k->ser1;
	removeitem[2]=pc_k->ser2;
	removeitem[3]=pc_k->ser3;
	removeitem[4]=pc_k->ser4;

	if (pc_k->spawnregion>0 && pc_k->spawnregion<255)
	{
		spawnregion[pc_k->spawnregion].current--;
	}


	removefromptr(charsp, pc_k->serial);
	
	if (pc_k->spawnserial != INVALID_SERIAL) 
		cspawnsp.remove(pc_k->spawnserial, pc_k->serial);
	if (pc_k->ownserial != INVALID_SERIAL) 
		cownsp.remove(pc_k->ownserial, pc_k->serial);
	
	for (j=0;j<now;j++)
	{
		if (perm[j]) 
			Xsend(j, removeitem, 5);		
	}
	
	if (pc_k != NULL) 
		mapRegions->Remove(pc_k); // taking it out of mapregions BEFORE x,y changed, LB
	
	pc_k->free = true;
	pc_k->pos.x=20+(xcounter++);
	pc_k->pos.y=50+(ycounter);
	pc_k->pos.z=9;
	pc_k->summontimer=0;
	if (xcounter==40)
	{
		ycounter++;
		xcounter=0;
	}
	if (ycounter==80)
	{
		ycounter=0;
		xcounter=0;
	}
	
	if (cmemcheck<300)
	{
		cmemcheck++;
		freecharmem[cmemcheck]=DEREF_P_CHAR(pc_k);
	}
	else cmemover=1;
}

bool cCharStuff::AllocateMemory(int NumberOfChars)
{
	bool memerr = false;
	cmem=NumberOfChars;
	if (cmem<100) cmem=100;
	cmem+=(2*CHAR_RESERVE);	// some reserve so realloc doesn't occur immediately after serverstart(Duke)

	clConsole.send(" Allocating initial dynamic Character memory of %i... ",cmem);
	if ((realchars= (cChar *) malloc(cmem*sizeof(cChar)) )==NULL)
		memerr=true;
//	else if ((teffects = (cTmpEff *)malloc(cmem*5*sizeof(cTmpEff)) )==NULL)//MAXEFFECTS = 5*MAXCHARS
	else if (!AllTmpEff->Alloc(cmem*5))
		memerr=1;

	if (memerr)
	{
		clConsole.send("\nERROR: Could not Allocate character memory!\n");
		error=1;
		return false;
	}
	int ilimit=0 ;
	if ((cmem-200) > 0)
		ilimit = cmem-200 ;	
	for (int i=ilimit;i<cmem;i++)
		realchars[i].free=1;

	clConsole.send("Done\n");
	return true;
}

bool cCharStuff::ResizeMemory()
{
	char memerr=0;
	const int slots=4000;   // bugfix for crashes under w95/98, LB never ever touch this number ...
	// it has major influence on performance !! if it too low -> slow and can cause crashes under w95/98.
	// this is because w95/98 can only handle 8196 subsequent realloc calls ( crappy thing...)
	// free() calls DONT help !!!, btw so we have to make that number real big
	// under nt and unix this limit doesnt exist

	if ((realchars = (cChar *)realloc(realchars, (cmem + slots)*sizeof(cChar)))==NULL)
		memerr=1;
//	else if ((teffects = (cTmpEff *)realloc(teffects, (cmem*5+slots*5)*sizeof(cTmpEff)))==NULL)//MAXEFFECTS = 5*MAXCHARS
	else if (!AllTmpEff->ReAlloc(cmem*5+slots*5))
		memerr=1;
	
	if (memerr)
	{
		LogCriticalVar("Could not reallocate char memory after %i. No more chars will be created.\nWOLFPACK may become unstable.",cmem);
		cwmWorldState->savenewworld(1);
		return false;
	}
	else
	{
		for (int i=cmem;i<cmem+slots;i++)
			realchars[i].free=1;
		cmem+=slots;
		moreCharMemoryRequested=false;
		return true;
	}
}

void cCharStuff::CheckMemoryRequest()
{
	if (moreCharMemoryRequested)
		ResizeMemory();
}

void cCharStuff::CollectReusableSlots()
{
	if (cmemover!=1)
	{
		cmemover=0;
		cmemcheck++;
		AllCharsIterator iter_char;
		for (iter_char.Begin(); iter_char.GetData() != NULL; iter_char++)
		{
			P_CHAR toCheck = iter_char.GetData();
			if (cmemcheck==300)
			{
				cmemover=1; 
				break;
			}
			if (toCheck->free)
			{
				freecharmem[cmemcheck] = DEREF_P_CHAR(toCheck);
				cmemcheck++;
			}
		}
	}
}

#if 0
int cCharStuff::GetReusableSlot()
{
	int newSlot=-1;
	unsigned int ctime=uiCurrentTime;
	static unsigned int NextMemCheck=0;

	if (cmemcheck==-1 &&						// no free slots on the stack and
		(NextMemCheck<=ctime || (overflow)))	// didn't search for slots in the past minute
	{
		NextMemCheck=ctime+speed.checkmem*60*MY_CLOCKS_PER_SEC;
		CollectReusableSlots();					// go look for slots !
	}

	if (cmemcheck!=-1)
	{
		newSlot=freecharmem[cmemcheck];
		cmemcheck--;
	}

	return newSlot;
}
#else
///////////////////////
// Name:	GetReusableSlot
// history:	by Duke, 6.09.2001
// Purpose:	searches for an empty slot in the chars array
//			It does this by simply searching from the *last* position where it found a slot.
//			If it reaches the end, it will search from the start. A timer prevents too many wrap-arounds.
//
int cCharStuff::GetReusableSlot()
{
	static unsigned int NextMemCheck=0;
	static unsigned int LastFree=0;	// remember this point in the array to continue search from here next time

	unsigned int ctime=uiCurrentTime;
	if (NextMemCheck<=ctime || (overflow))	// didn't wrap around in the past minute
	{
		unsigned int started=LastFree;		// remember where we started the search from here next time
		for (  ;LastFree<charcount;LastFree++)
		{
			if (chars[LastFree].free)
				return LastFree;	
		}
		// we have reached the end so start again from the beginning of the array
		for (LastFree=0;LastFree<started;LastFree++)
		{
			if (chars[LastFree].free)
				return LastFree;	
		}
		NextMemCheck=ctime+speed.checkmem*60*MY_CLOCKS_PER_SEC;	// nothing found, set time for next try
	}
	return -1;
}
#endif

int cCharStuff::MemCharFree()			// Find a free char slot
{
	if (charcount+CHAR_RESERVE >= cmem-C_W_O_1)	//less than 'Reserve' free slots left, so get more memory
		moreCharMemoryRequested=true;

	int c=GetReusableSlot();			// try to use the space of a deleted item
	if (c==-1)
	{
		if (charcount+1>=cmem-C_W_O_1)	//theres no more free sluts.. er slots
		{
			LogCritical("couldn't resize character memory in time");
			return -1;
		}
		c=charcount;
		charcount++;
	}
	return c;
}

#if 0
int cCharStuff::MemCharFree()
{
	unsigned int i, ctime=uiCurrentTime;
	signed int nChar=-1;
	static unsigned int NextMemCheck=0;
	char memerr=0;
	
	if (cmemcheck!=-1)
	{
		nChar=freecharmem[cmemcheck];
		cmemcheck--;
	}
	
	if (nChar==-1)
	{
		if (NextMemCheck<=ctime || (overflow))
		{
			NextMemCheck=ctime+speed.checkmem*60*MY_CLOCKS_PER_SEC; //60 seconds per min
			if (cmemover==1)
			{
				cmemover=0;
				cmemcheck++;
				for (i=0;i<charcount;i++)
				{
					if (cmemcheck==300)
					{
						cmemover=1; 
						break;
					}
					if (chars[i].free)
					{
						if (nChar==-1) nChar=i;
						else {
							freecharmem[cmemcheck]=i;
							cmemcheck++;
						}
					}
				}
			}
		}
	}

	int slots=3000*4;   // bugfix for crashes under w95/98, LB
	                  // never ever touch this number ...
	                  // it has major influence on performance !!
	                  // if it too low -> slow and can cause crashes under
	                  // w95/98. this is because w95/98 can only handle 8196 
	                  // subsequent realloc calls ( crappy thing...)
	                  // free() calls DOESNT help !!!, btw
	                  // so we have to make that number real big
	                  // under nt and unix this limit doesnt exist



	if (nChar==-1 && charcount>=cmem-C_W_O_1) //Lets ASSUME theres no more memory left instead of acctually checking all the items to find a free one.
	{
		
		if ((realchars = (cChar *)realloc(realchars, (cmem + slots)*sizeof(cChar)))==NULL)
			memerr=1;
		else if ((talkingto = (int *)realloc(talkingto, (cmem + slots)*sizeof(int)))==NULL)
			memerr=1;	
		else if ((teffects = (teffect_st *)realloc(teffects, (cmem*5+slots*5)*sizeof(teffect_st)))==NULL)//MAXEFFECTS = 5*MAXCHARS
			memerr=1;
        
		if (memerr)
		{
			clConsole.send("ERROR: Could not reallocate character memory after %i. No more characters will be created.\nWARNING: WOLFPACK may become unstable.\n", cmem);
			cwmWorldState->savenewworld(1);
			NextMemCheck=ctime+speed.checkmem*60*MY_CLOCKS_PER_SEC; //60 seconds per min
			if (cmemover==1)
			{
				cmemover=0;
				cmemcheck++;
				for (i=0;i<charcount;i++)
				{
					if (cmemcheck==300)
					{
						cmemover=1; 
						break;
					}
					if (chars[i].free)
					{
						if (nChar==-1) nChar=i;
						else {
							freecharmem[cmemcheck]=i;
							cmemcheck++;
						}
					}
				}
			}
		} else {
			for (i=cmem;i<cmem+slots;i++)	realchars[i].free = true;
			nChar=cmem-I_W_O_1;
			cmem+=slots;
		}
	} else if (nChar==-1) nChar = charcount++;
	
	return nChar;
}
#endif

P_ITEM cCharStuff::AddRandomLoot(P_ITEM pBackpack, char * lootlist)
{
	char sect[512];
	int i,j, storeval,loopexit=0;
	P_ITEM retitem = NULL;
	storeval=-1;
	i=0; j=0;

	sprintf(sect, "LOOTLIST %s", lootlist);
	
	Script *pScpBase=i_scripts[npc_script];
	Script *pScp=pScpBase->Select(sect,custom_npc_script);
	if (!pScp) return NULL;

	loopexit=0;
	do
	{
		pScp->NextLine();
		if (script1[0]!='}')
		{
			i++; // Count number of entries on list.
		}
	} while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
	pScp->Close();

	if(i>0)
	{
		i=rand()%(i);
		pScp=pScpBase->Select(sect,custom_npc_script);

		loopexit=0;
		do
		{
			pScp->NextLine();
			if (script1[0]!='}')
			{
				if(j==i)
				{
					storeval=str2num(script1);	//script1 = ITEM#

					scpMark m=pScp->Suspend();
					retitem = Targ->AddMenuTarget(-1, 0, storeval);
					pScp->Resume(m);

					if(retitem!=NULL)
					{
						retitem->pos.x=50+(rand()%80);
						retitem->pos.y=50+(rand()%80);
						retitem->pos.z=9;
						retitem->SetContSerial(pBackpack->serial);
					}
					break;;    
				}
				else j++;
			}
		}	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
		pScp->Close();
	}
	return retitem;
}

/*** s: socket ***/
int cCharStuff::AddRandomNPC(int s, char * npclist, int spawnpoint)
{
	//This function gets the random npc number from the list and recalls
	//addrespawnnpc passing the new number
	char sect[512];
	unsigned int uiTempList[100];
	int i=0,k=0;
	sprintf(sect, "NPCLIST %s", npclist);

	Script *pScpBase=i_scripts[npc_script];
	Script *pScp=pScpBase->Select(sect,custom_npc_script);
	if (!pScp) return -1;

	unsigned long loopexit=0;
	do
	{
		pScp->NextLine();
		if (script1[0]!='}')
		{
			uiTempList[i]=str2num(script1);
			i++;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS));
	pScp->Close();

	if(i>0)
	{
		i=rand()%(i);
		k=uiTempList[i];
	}
	if(k!=0)
	{
		if (spawnpoint==-1)
		{
			addmitem[s]=k;
			return Targ->NpcMenuTarget(s);
			//return -1;
		}
		else
		{
			return k; //addrespawnnpc(spawnpoint,k,1);
		}
	}
	return -1;
}

//o---------------------------------------------------------------------------o
//| Function   : AddNPC (3 interfaces)
//| Programmer : Duke, 23.05.2000
//o---------------------------------------------------------------------------o
//| Purpose    : creates the scripted NPC given by npcNum
//|              The position of the NPC can be given in three different ways:
//|				 1. by parms x1 y1 z1 (trigger)
//|				 2. by passing a socket (GM add)
//|				 3. by passing an item index (spawn rune)
//|
//| Remarks    : This function was created from the former AddRespawnNPC() and
//|				 AddNPCxyz() that were 95% identical
//o---------------------------------------------------------------------------o
/*
int cCharStuff::AddRespawnNPC(int s, int npcNum, int type)
{
	if (type == 1)
		return AddNPC(-1, s, npcNum, 0,0,0);	// 's' is an item index
	else
		return AddNPC(s, NULL, npcNum, 0,0,0);	// 's' is a socket
}*/
int cCharStuff::AddNPCxyz(int s, int npcNum, int type, int x1, int y1, signed char z1) //Morrolan - replacement for old Npcs->AddNPCxyz(), fixes a LOT of problems.
{
	if (type == 0)
		return AddNPC(s, NULL, npcNum, x1,y1,z1);	// 's' maybe(!) is a socket
	if (type == 1)
		clConsole.send("ERROR: type == 1 not supported!\n");
	return -1;
}

int cCharStuff::AddNPC(int s, P_ITEM pi_i, int npcNum, int x1, int y1, signed char z1)
{
	int tmp, z,c, lovalue, hivalue;
	int k=0, xos=0, yos=0, lb;
	char sect[512];
	int haircolor = -1; //(we need this to remember the haircolor)
	short postype;				// determines how xyz of the new NPC are set, see below
	short fx1,fx2,fy1,fy2,fz1;	// temp. hold the rectangle or circle for npcwander from script
	fx1=fx2=fy1=fy2=fz1=0;

	if (x1 > 0 && y1 > 0)
 		postype = 3;	// take position from parms
	else if ( s > -1 && pi_i == NULL)
		postype = 2;	// take position from socket's buffer
	else if ( s == -1 && pi_i != NULL)
		postype = 1;	// take position from items[i]
	else
	{
		clConsole.send("ERROR: bad parms in call to AddNPC\n");
		return -1;
	}

	P_ITEM pBackpack = NULL;
	P_ITEM retitem = NULL;
	P_ITEM shoppack1 = NULL;
	P_ITEM shoppack2 = NULL;
	P_ITEM shoppack3 = NULL;
	//
	// First things first...lets find out what NPC# we should spawn
	//
	Script *pScpBase=i_scripts[npc_script];
	sprintf(sect, "NPC %i", npcNum);
	Script *pScp=pScpBase->Select(sect,custom_npc_script);
	if (!pScp) return -1;
	
	unsigned long loopexit=0;
	do
	{
		pScp->NextLineSplitted();
		if (script1[0]!='}')
		{
			if (!(strcmp("NPCLIST", (char*)script1)))
			{
				scpMark m=pScp->Suspend();

//				if (postype==1 || postype==3)	// we have a pos from item or parms
//				{
					npcNum=Npcs->AddRandomNPC(s,(char*)script2,1);
					sprintf(sect, "NPC %i", npcNum);
					if (npcNum==-1) 
					{ 
						pScp->Close();
						return -1;
					}
/*				}
				else
				{
					npcNum=Npcs->AddRandomNPC(s,script2,-1);	// ask for position
					if (npcNum==-1) 
					{
						closescript(); // AC
						return -1;
					}
				}
				//AntiChrist */
				pScp->Resume(m);
				break;  //got the NPC number to add stop reading
			}
		}
	} while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	pScp->Close();
	
	//
	// Now lets spawn him/her
	//
	c=Npcs->MemCharFree ();
	if(c==-1) return -1;
	P_CHAR pc_c = MAKE_CHARREF_LRV(c,-1);
	pc_c->Init();
	

	pc_c->setPriv(0x10);
	pc_c->npc=1;
	pc_c->att=1;
	pc_c->def=1;
	pc_c->spawnserial=-1;
	
	pScp=pScpBase->Select(sect,custom_npc_script);
	if (!pScp)
	{
		Npcs->DeleteChar(pc_c);
		return -1;
	}

	loopexit=0;
	do
	{
		pScp->NextLineSplitted();

		if (script1[0]!='}') {
			switch(script1[0])
			{
			case 'A':
			case 'a':
			if (!strcmp("ALCHEMY",(char*)script1))		pc_c->baseskill[ALCHEMY] = getstatskillvalue((char*)script2);
			else if (!strcmp("ANATOMY",(char*)script1))		pc_c->baseskill[ANATOMY] = getstatskillvalue((char*)script2);
			else if (!strcmp("ARCHERY",(char*)script1))		pc_c->baseskill[ARCHERY] = getstatskillvalue((char*)script2);
			else if (!strcmp("ARMSLORE",(char*)script1))		pc_c->baseskill[ARMSLORE] = getstatskillvalue((char*)script2);
			else if (!strcmp("ANIMALLORE",(char*)script1))	pc_c->baseskill[ANIMALLORE] = getstatskillvalue((char*)script2);
			break;

			case 'B':
			case 'b':
			if (!strcmp("BACKPACK", (char*)script1))
			{
				if (pBackpack == NULL)
				{
					scpMark m=pScp->Suspend();
					pBackpack = Items->SpawnItem(-1, pc_c,1,"Backpack",0,0x0E,0x75,0,0,0,0);
					if(pBackpack == NULL)
					{
						Npcs->DeleteChar(pc_c);
						return -1;
					}
					pc_c->packitem = pBackpack->serial;
					pBackpack->pos.x=0;
					pBackpack->pos.y=0;
					pBackpack->pos.z=0;
					pBackpack->SetContSerial(pc_c->serial);
					pBackpack->layer=0x15;
					pBackpack->type=1;
					pBackpack->dye=1;
					
					retitem = pBackpack;
					pScp->Resume(m);
					strcpy((char*)script1, "DUMMY"); // Prevents unexpected matchups...
				}
			}
			else if (!strcmp("BEGGING",(char*)script1))			pc_c->baseskill[BEGGING] = getstatskillvalue((char*)script2);
			else if (!strcmp("BLACKSMITHING",(char*)script1))	pc_c->baseskill[BLACKSMITHING] = getstatskillvalue((char*)script2);
			else if (!strcmp("BOWCRAFT",(char*)script1))			pc_c->baseskill[BOWCRAFT] = getstatskillvalue((char*)script2);
			break;

			case 'C':
			case 'c':

			if (!strcmp("COLOR",(char*)script1)) {
				if (retitem != NULL)
				{
					retitem->color1=(hex2num(script2))>>8;
					retitem->color2=(hex2num(script2))%256;
				}
			}
			else if (!strcmp("CARVE",(char*)script1)) pc_c->carve=str2num(script2);
			else if (!strcmp("CAMPING",(char*)script1)) pc_c->baseskill[CAMPING] = getstatskillvalue((char*)script2);
			else if (!strcmp("CARPENTRY",(char*)script1)) pc_c->baseskill[CARPENTRY] = getstatskillvalue((char*)script2);
			else if (!strcmp("CARTOGRAPHY",(char*)script1)) pc_c->baseskill[CARTOGRAPHY] = getstatskillvalue((char*)script2);
			else if (!strcmp("CANTRAIN",(char*)script1)) pc_c->cantrain=true;
			else if (!strcmp("COOKING",(char*)script1)) pc_c->baseskill[COOKING] = getstatskillvalue((char*)script2);
			else if (!strcmp("COLORMATCHHAIR",(char*)script1))
			{
				if (retitem != NULL && haircolor!=-1)
				{
					retitem->color1=(haircolor)>>8;
					retitem->color2=(haircolor)%256;
				}
			}
			else if (!strcmp("COLORLIST",(char*)script1))
			{
				scpMark m=pScp->Suspend();
				int storeval = addrandomcolor(DEREF_P_CHAR(pc_c), (char*)script2);
				if (retitem != NULL)
				{
					retitem->color1=(storeval)>>8;
					retitem->color2=(storeval)%256;
				}
				pScp->Resume(m);
				strcpy((char*)script1, "DUMMY"); // To prevent accidental exit of loop.
			}
			break;

			case 'D':
			case 'd':

			if (!strcmp("DIRECTION",(char*)script1)) {
				if (!strcmp("NE",(char*)script2)) pc_c->dir=1;
				else if (!strcmp("E",(char*)script2)) pc_c->dir=2;
				else if (!strcmp("SE",(char*)script2)) pc_c->dir=3;
				else if (!strcmp("S",(char*)script2)) pc_c->dir=4;
				else if (!strcmp("SW",(char*)script2)) pc_c->dir=5;
				else if (!strcmp("W",(char*)script2)) pc_c->dir=6;
				else if (!strcmp("NW",(char*)script2)) pc_c->dir=7;
				else if (!strcmp("N",(char*)script2)) pc_c->dir=0;
			}
			else if (!strcmp("DEX",(char*)script1) || !strcmp("DEXTERITY",(char*)script1)) 
			{
				pc_c->setDex(getstatskillvalue((char*)script2));
				pc_c->stm = pc_c->realDex();
			}
			else if (!strcmp("DEF",(char*)script1)) pc_c->def = getstatskillvalue((char*)script2);
			else if (!strcmp("DETECTINGHIDDEN",(char*)script1)) pc_c->baseskill[DETECTINGHIDDEN] = getstatskillvalue((char*)script2);
			else if (!strcmp("DAMAGE",(char*)script1) || !strcmp("ATT",(char*)script1)) {
				gettokennum((char*)script2, 0);
				lovalue=str2num(gettokenstr);
				gettokennum((char*)script2, 1);
				hivalue=str2num(gettokenstr);
				pc_c->lodamage = lovalue;
				pc_c->hidamage = lovalue;
				if(hivalue) {
					pc_c->hidamage = hivalue;
				}
			}
			break;

			case 'E':
			case 'e':

			if (!(strcmp("EMOTECOLOR",(char*)script1))) {
				pc_c->emotecolor1=(hex2num(script2))>>8;
				pc_c->emotecolor2=(hex2num(script2))%256;
			}
			else if (!strcmp("ENTICEMENT",(char*)script1)) pc_c->baseskill[ENTICEMENT] = getstatskillvalue((char*)script2);
			else if (!strcmp("EVALUATINGINTEL",(char*)script1)) pc_c->baseskill[EVALUATINGINTEL] = getstatskillvalue((char*)script2);
			break;

			case 'F':
			case 'f':

			if (!strcmp("FISHING",(char*)script1)) pc_c->baseskill[FISHING] = getstatskillvalue((char*)script2);
			else if (!strcmp("FORENSICS",(char*)script1)) pc_c->baseskill[FORENSICS] = getstatskillvalue((char*)script2);
			else if (!strcmp("FX1",(char*)script1)) fx1=str2num(script2);  // new NPCWANDER implementation
			else if (!strcmp("FX2",(char*)script1)) fx2=str2num(script2);
			else if (!strcmp("FLEEAT",(char*)script1)) pc_c->fleeat=str2num(script2);
			else if (!strcmp("FAME",(char*)script1)) pc_c->fame=str2num(script2);
			else if (!strcmp("FENCING",(char*)script1)) pc_c->baseskill[FENCING] = getstatskillvalue((char*)script2);
			else if (!strcmp("FY1",(char*)script1)) fy1=str2num(script2);
			else if (!strcmp("FY2",(char*)script1)) fy2=str2num(script2);
			else if (!strcmp("FZ1",(char*)script1)) fz1=str2num(script2);
			break;

			case 'G':
			case 'g':

			if (!strcmp("GOLD", (char*)script1))
			{
				if (pBackpack != NULL)
				{ 
					scpMark m=pScp->Suspend();
					P_ITEM pGold = Items->SpawnItem(pc_c,1,"#",1,0x0EED,0,1);
					if(!pGold)
					{
						Npcs->DeleteChar(pc_c);
						return -1;
					}
					pScp->Resume(m);

					pGold->priv|=0x01;
					gettokennum((char*)script2, 0);
					lovalue=str2num(gettokenstr);
					gettokennum((char*)script2, 1);
					hivalue=str2num(gettokenstr);
					if (hivalue==0)
					{
						if (lovalue/2!=0) pGold->amount=lovalue/2 + (rand()%(lovalue/2));
						else pGold->amount=0;
					} else
					{
						if (hivalue-lovalue!=0) pGold->amount=lovalue + (rand()%(hivalue-lovalue));
						else pGold->amount=lovalue;
					}
				}
				else
					clConsole.send("Warning: Bad NPC Script %d with problem no backpack for gold.\n", npcNum);
			}
			break;

			case 'H':
			case 'h':

			if (!strcmp("HEALING",(char*)script1)) pc_c->baseskill[HEALING] = getstatskillvalue((char*)script2);
			else if (!strcmp("HIDAMAGE",(char*)script1)) pc_c->hidamage=str2num(script2);
			else if (!strcmp("HERDING",(char*)script1)) pc_c->baseskill[HERDING] = getstatskillvalue((char*)script2);
			else if (!strcmp("HIDING",(char*)script1)) pc_c->baseskill[HIDING] = getstatskillvalue((char*)script2);
			else if (!strcmp("HAIRCOLOR",(char*)script1))
			{
				scpMark m=pScp->Suspend();
				
				if (retitem != NULL) // LB, ouple of bugfixes
				{
					haircolor=addrandomhaircolor(DEREF_P_CHAR(pc_c),(char*)script2);
					if (haircolor!=-1)
					{
						retitem->color1=(haircolor)>>8;
						retitem->color2=(haircolor)%256;
					}
				}
				pScp->Resume(m);
				strcpy((char*)script1, "DUMMY"); // To prevent accidental exit of loop.
			}
			break;

			case 'I':
			case 'i':

			if (!strcmp("ID",(char*)script1)) 
			{
				tmp=hex2num(script2);
				pc_c->id1=tmp>>8;
				pc_c->id2=tmp%256;
				pc_c->xid1=pc_c->id1;
				pc_c->xid2=pc_c->id2;
			}
			else if (!strcmp("ITEM",(char*)script1)) 
			{
				int storeval=str2num(script2);

				scpMark m=pScp->Suspend();
				retitem = Targ->AddMenuTarget(-1, 0, storeval);
				pScp->Resume(m);

				if ( retitem != NULL )
				{
					retitem->SetContSerial(pc_c->serial);
					if (retitem->layer==0) {
						clConsole.send("Warning: Bad NPC Script %d with problem item %d executed!\n", npcNum, storeval);
					}
				}
				strcpy((char*)script1, "DUMMY"); // Prevents unexpected matchups...
			}
			else if (!strcmp("INT",(char*)script1) || !strcmp("INTELLIGENCE",(char*)script1)) {
				pc_c->in  = getstatskillvalue((char*)script2);
				pc_c->in2 = pc_c->in;
				pc_c->mn  = pc_c->in;
			}
			//Done Handling Stats
			//Handle Skills
			else if (!strcmp("ITEMID",(char*)script1)) pc_c->baseskill[ITEMID] = getstatskillvalue((char*)script2);
			else if (!strcmp("INSCRIPTION",(char*)script1)) pc_c->baseskill[INSCRIPTION] = getstatskillvalue((char*)script2);
			break;
			
			case 'K':
			case 'k':
			if (!strcmp("KARMA",(char*)script1)) pc_c->karma=str2num(script2);
			break;

			case 'L':
			case 'l':

			if (!strcmp("LOOT",(char*)script1))
			{
				if (pBackpack != NULL)
				{
					scpMark m=pScp->Suspend();
					retitem = Npcs->AddRandomLoot(pBackpack, script2);
					pScp->Resume(m);

					strcpy((char*)script1, "DUMMY"); // Prevents unexpected matchups...
				} else
				{
					clConsole.send("Warning: Bad NPC Script %d with problem no backpack for loot.\n", npcNum);
				}
			}
			else if (!strcmp("LODAMAGE",(char*)script1)) pc_c->lodamage=str2num(script2);
			else if (!strcmp("LUMBERJACKING",(char*)script1)) pc_c->baseskill[LUMBERJACKING] = getstatskillvalue((char*)script2);
			else if (!strcmp("LOCKPICKING",(char*)script1)) pc_c->baseskill[LOCKPICKING] = getstatskillvalue((char*)script2);
			break;

			case 'M':
			case 'm':
			if ((!(strcmp("MACEFIGHTING",(char*)script1)))||(!(strcmp("SKILL41",(char*)script1)))) pc_c->baseskill[MACEFIGHTING] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("MINING",(char*)script1)))||(!(strcmp("SKILL45",(char*)script1)))) pc_c->baseskill[MINING] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("MAGERY",(char*)script1)))||(!(strcmp("SKILL25",(char*)script1)))) pc_c->baseskill[MAGERY] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("MAGICRESISTANCE",(char*)script1)))||(!(strcmp("RESIST",(char*)script1)))||(!(strcmp("SKILL26",(char*)script1)))) pc_c->baseskill[MAGICRESISTANCE] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("MUSICIANSHIP",(char*)script1)))||(!(strcmp("SKILL29",(char*)script1)))) pc_c->baseskill[MUSICIANSHIP] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("MEDITATION",(char*)script1)))||(!(strcmp("SKILL46",(char*)script1)))) pc_c->baseskill[MEDITATION] = getstatskillvalue((char*)script2);
			break;

			case 'N':
			case 'n':

			if (!(strcmp("NAME",(char*)script1))) strcpy(pc_c->name, (char*)script2);
			else if (!(strcmp("NAMELIST", (char*)script1))) {
				scpMark m=pScp->Suspend();
				setrandomname(pc_c,(char*)script2);
				pScp->Resume(m);
				strcpy((char*)script1, "DUMMY"); // To prevent accidental exit of loop.
			}
			else if (!(strcmp((char*)script1, "NOTRAIN"))) pc_c->cantrain=false;
			else if (!(strcmp("NPCWANDER",(char*)script1))) pc_c->npcWander=str2num(script2);
			else if (!(strcmp("NPCAI",(char*)script1))) pc_c->npcaitype=hex2num(script2);
			break;

			case 'O':
			case 'o':
			if (!(strcmp("ONHORSE",(char*)script1))) pc_c->onhorse=true;
			break;

			case 'P':
			case 'p':

			if (!strcmp("PARRYING",(char*)script1)) pc_c->baseskill[PARRYING] = getstatskillvalue((char*)script2);
			else if (!(strcmp("PRIV1",(char*)script1))) pc_c->setPriv(str2num(script2));
			else if (!(strcmp("PRIV2",(char*)script1))) pc_c->priv2=str2num(script2);
			else if (!(strcmp("POISON",(char*)script1))) pc_c->poison=str2num(script2);
			else if ((!(strcmp("PEACEMAKING",(char*)script1)))||(!(strcmp("SKILL9",(char*)script1)))) pc_c->baseskill[PEACEMAKING] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("PROVOCATION",(char*)script1)))||(!(strcmp("SKILL22",(char*)script1)))) pc_c->baseskill[PROVOCATION] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("POISONING",(char*)script1)))||(!(strcmp("SKILL30",(char*)script1)))) pc_c->baseskill[POISONING] = getstatskillvalue((char*)script2);
			else if (!(strcmp("PACKITEM",(char*)script1)))
			{
				if (pBackpack != NULL) 
				{
					int storeval=str2num(script2);

					scpMark m=pScp->Suspend();
					retitem = Targ->AddMenuTarget(-1, 0, storeval);
					pScp->Resume(m);

					if (retitem != NULL)
					{
						retitem->SetContSerial(pBackpack->serial);
						retitem->pos.x=50+(rand()%80);
						retitem->pos.y=50+(rand()%80);
						retitem->pos.z=9;
					}
					strcpy((char*)script1, "DUMMY"); // Prevents unexpected matchups...
				} else
				{
					clConsole.send("Warning: Bad NPC Script %d with problem no backpack for packitem.\n", npcNum);
				}
			}
			break;

			case 'R':
			case 'r':
			if (!(strcmp("RACE",(char*)script1))) pc_c->race=str2num(script2);
			else if (!(strcmp("REATTACKAT",(char*)script1))) pc_c->reattackat=str2num(script2);
			else if ((!(strcmp("REMOVETRAPS",(char*)script1)))||(!(strcmp("SKILL48",(char*)script1)))) pc_c->baseskill[REMOVETRAPS] = getstatskillvalue((char*)script2);
			else if (!(strcmp("RSHOPITEM",(char*)script1)))
			{
				if ( shoppack1 == NULL)
				{
					AllItemsIterator iterItems;
					for(iterItems.Begin(); !iterItems.atEnd(); iterItems++)
					{
						P_ITEM pz = iterItems.GetData();
						if (!pz->free)
						{
							if (pc_c->Wears(pz) &&
								pz->layer==0x1A)
							{
								shoppack1 = pz;
								break;
							}
						}
					}
					//if (shoppack1 == -1)
					  //LogError("Error creating shoppack1\n");
				}
				if ( shoppack1 != NULL )
				{
					int storeval=str2num(script2);

					scpMark m=pScp->Suspend();
					retitem = Targ->AddMenuTarget(-1, 0, storeval);
					pScp->Resume(m);

					if (retitem != NULL)
					{
						retitem->SetContSerial(shoppack1->serial);
						retitem->pos.x=50+(rand()%80);
						retitem->pos.y=50+(rand()%80);
						retitem->pos.z=9;
						if (retitem->name2 && (strcmp(retitem->name2,"#"))) strcpy(retitem->name,retitem->name2); // Item identified! -- by Magius(CHE)					}
					}
					strcpy((char*)script1, "DUMMY"); // Prevents unexpected matchups...
				} else
				{
					clConsole.send("Warning: Bad NPC Script %d with problem no shoppack1 for item.\n", npcNum);
				}
			}
			break;

			case 'S':
			case 's':

			if (!(strcmp("SKIN",(char*)script1))) 
			{
				pc_c->skin = pc_c->xskin = hex2num(script2);
			}
			else if (!(strcmp("SHOPKEEPER", (char*)script1))) 
			{
				scpMark m=pScp->Suspend();
				Commands->MakeShop(pc_c); 
				pScp->Resume(m);
			}
			else if (!(strcmp("SELLITEM",(char*)script1))) 
			{
				if (shoppack3 == NULL) 
				{
					AllItemsIterator iterItems;
					for(iterItems.Begin(); !iterItems.atEnd(); iterItems++) 
					{
						P_ITEM pz = iterItems.GetData();
						if (!pz->free)
						{
							if (pc_c->Wears(pz) && pz->layer==0x1C)
							{
								shoppack3 = pz;
								break;
							}
						}
					}
				}
				if ( shoppack3 != NULL ) 
				{
					int storeval=str2num(script2);

					scpMark m=pScp->Suspend();
					retitem = Targ->AddMenuTarget(-1, 0, storeval);
					pScp->Resume(m);

					if (retitem != NULL)
					{
						retitem->SetContSerial(shoppack3->serial);
						retitem->value=retitem->value/2;
						retitem->pos.x=50+(rand()%80);
						retitem->pos.y=50+(rand()%80);
						retitem->pos.z=9;
						if (retitem->name2 && (strcmp(retitem->name2,"#"))) 
							strcpy(retitem->name,retitem->name2); // Item identified! -- by Magius(CHE)					}
					}
					strcpy((char*)script1, "DUMMY"); // Prevents unexpected matchups...
				} else
				{
					clConsole.send("Warning: Bad NPC Script %d with problem no shoppack3 for item.\n", npcNum);
				}
			}
			else if (!(strcmp("SHOPITEM",(char*)script1)))
			{
				if ( shoppack2 == NULL )
				{
					AllItemsIterator iterItems;
					for (iterItems.Begin(); !iterItems.atEnd(); iterItems++)
					{
						P_ITEM pz = iterItems.GetData();
						if (!pz->free)
						{
							if (pc_c->Wears(pz) && pz->layer==0x1B)
							{
								shoppack2 = pz;
								break;
							}
						}
					}
				}
				if (shoppack2 != NULL)
				{
					int storeval = str2num(script2);

					scpMark m = pScp->Suspend();
					retitem = Targ->AddMenuTarget(-1, 0, storeval);
					pScp->Resume(m);

					if ( retitem != NULL)
					{
						retitem->SetContSerial(shoppack2->serial);
						retitem->pos.x=50+(rand()%80);
						retitem->pos.y=50+(rand()%80);
						retitem->pos.z=9;
						if (retitem->name2 && (strcmp(retitem->name2,"#"))) strcpy(retitem->name,retitem->name2); // Item identified! -- by Magius(CHE)					}
					}
					strcpy((char*)script1, "DUMMY"); // Prevents unexpected matchups...
				} else
				{
					clConsole.send("Warning: Bad NPC Script %d with problem no shoppack2 for item.\n", npcNum);
				}
			}
			else if (!(strcmp("SPATTACK",(char*)script1))) pc_c->spattack=str2num(script2);
			else if (!(strcmp("SPEECH",(char*)script1))) pc_c->speech=str2num(script2);
			else if (!(strcmp("SPLIT",(char*)script1))) pc_c->split=str2num(script2);
			else if ((!(strcmp("STR",(char*)script1)))||(!(strcmp("STRENGTH",(char*)script1)))) {
				pc_c->st  = getstatskillvalue((char*)script2);
				pc_c->st2 = pc_c->st;
				pc_c->hp  = pc_c->st;
			}
			else if (!(strcmp("SPLITCHANCE",(char*)script1))) pc_c->splitchnc=str2num(script2);
			else if (!(strcmp("SAYCOLOR",(char*)script1))) 			pc_c->saycolor = static_cast<UI16>(hex2num(script2));
			else if (!(strcmp("SPADELAY",(char*)script1))) pc_c->spadelay=str2num(script2);
			else if ((!(strcmp("SPIRITSPEAK",(char*)script1)))||(!(strcmp("SKILL32",(char*)script1)))) pc_c->baseskill[SPIRITSPEAK] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("STEALTH",(char*)script1)))||(!(strcmp("SKILL47",(char*)script1)))) pc_c->baseskill[STEALTH] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("SWORDSMANSHIP",(char*)script1)))||(!(strcmp("SKILL40",(char*)script1)))) pc_c->baseskill[SWORDSMANSHIP] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("SNOOPING",(char*)script1)))||(!(strcmp("SKILL28",(char*)script1)))) pc_c->baseskill[SNOOPING] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("STEALING",(char*)script1)))||(!(strcmp("SKILL33",(char*)script1)))) pc_c->baseskill[STEALING] = getstatskillvalue((char*)script2);
			else if (!(strcmp("SKINLIST",(char*)script1)))
			{
				scpMark m=pScp->Suspend();
				pc_c->xskin = pc_c->skin = addrandomcolor(DEREF_P_CHAR(pc_c),(char*)script2);
				pScp->Resume(m);
				strcpy((char*)script1, "DUMMY"); // To prevent accidental exit of loop.
			}
			else if (!(strcmp("SKILL", (char*)script1)))
			{
				gettokennum((char*)script2, 0);
				z=str2num(gettokenstr);
				gettokennum((char*)script2, 1);
				pc_c->baseskill[z]=str2num(gettokenstr);
			}

            else if (!(strcmp("STABLEMASTER",(char*)script1))) pc_c->npc_type=1;
			break;

			case 'T':
			case 't':
			if (!(strcmp("TITLE",(char*)script1))) strcpy(pc_c->title, script2);
			else if ((!(strcmp("TOTAME", (char*)script1)))||(!(strcmp("TAMING", (char*)script1)))) pc_c->taming=str2num(script2);
			else if (!(strcmp("TRIGGER",(char*)script1)))	pc_c->trigger=str2num(script2);
			else if (!(strcmp("TRIGWORD",(char*)script1)))	strcpy(pc_c->trigword,(char*)script2);
			else if ((!(strcmp("TACTICS",(char*)script1)))||(!(strcmp("SKILL27",(char*)script1)))) pc_c->baseskill[TACTICS] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("TAILORING",(char*)script1)))||(!(strcmp("SKILL34",(char*)script1)))) pc_c->baseskill[TAILORING] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("TAMING",(char*)script1)))||(!(strcmp("SKILL35",(char*)script1)))) pc_c->baseskill[TAMING] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("TASTEID",(char*)script1)))||(!(strcmp("SKILL36",(char*)script1)))) pc_c->baseskill[TASTEID] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("TINKERING",(char*)script1)))||(!(strcmp("SKILL37",(char*)script1)))) pc_c->baseskill[TINKERING] = getstatskillvalue((char*)script2);
			else if ((!(strcmp("TRACKING",(char*)script1)))||(!(strcmp("SKILL38",(char*)script1)))) pc_c->baseskill[TRACKING] = getstatskillvalue((char*)script2);
			break;

			case 'V':
			case 'v':
			if (!(strcmp("VALUE",(char*)script1))) if (retitem != NULL) retitem->value=(str2num(script2));
			else if (!strcmp("VETERINARY",(char*)script1)) pc_c->baseskill[VETERINARY] = getstatskillvalue((char*)script2);
			break;
			
			case 'W':
			case 'w':
			if ((!(strcmp("WRESTLING",(char*)script1)))||(!(strcmp("SKILL43",(char*)script1)))) pc_c->baseskill[WRESTLING] = getstatskillvalue((char*)script2);
			break;

			default:
				clConsole.send("Warning: Fall out of switch statement in npcs.cpp AddNPC()\n");
			}
			
			//Done Handling Skills
			//Handle Extras

			//Done Handling Obsolete Stuff
			//--------------- DONE NEW STAT & SKILL FORMAT ---------------------
      }
   }
   while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
   pScp->Close();

   // Now that we have created the NPC, lets place him
   switch (postype)
   {
   case 1:					// take position from (spawning) item
	   if (triggerx)
	   {
		   pc_c->pos.x=triggerx;
		   pc_c->pos.y=triggery;
		   pc_c->dispz=pc_c->pos.z=triggerz;
		   triggerx=DEREF_P_CHAR(pc_c);
	   } else
	   {
	   /*Zippy's Code chages for area spawns --> (Type 69) xos and yos (X OffSet, Y OffSet) 
	   are used to find a random number that is then added to the spawner's x and y (Using 
	   the spawner's z) and then place the NPC anywhere in a square around the spawner. 
	   This square is random anywhere from -10 to +10 from the spawner's location (for x and 
	   y) If the place chosen is not a valid position (the NPC can't walk there) then a new 
	   place will be chosen, if a valid place cannot be found in a certain # of tries (50), 
		   the NPC will be placed directly on the spawner and the server op will be warned. */
		   if ((pi_i->type==69 || pi_i->type==125)&& pi_i->isInWorld())
		   {
			   if (pi_i->more3==0) pi_i->more3=10;
			   if (pi_i->more4==0) pi_i->more4=10;
			   //signed char z, ztemp, found;
	   
			   k=0;
			   do
			   {
				   if (k>=50) //this CAN be a bit laggy. adjust as nessicary
				   {
					   clConsole.send("WOLFPACK: Problem area spawner found at [%i,%i,%i]. NPC placed at default location.\n",pi_i->pos.x,pi_i->pos.y,pi_i->pos.z);
					   xos=0;
					   yos=0;
					   break;
				   }
				   xos=RandomNum(-pi_i->more3,pi_i->more3);
				   yos=RandomNum(-pi_i->more4,pi_i->more4);
				   // clConsole.send("Spawning at Offset %i,%i (%i,%i,%i) [-%i,%i <-> -%i,%i]. [Loop #: %i]\n",xos,yos,pi_i->x+xos,pi_i->y+yos,pi_i->z,pi_i->more3,pi_i->more3,pi_i->more4,pi_i->more4,k); /** lord binary, changed %s to %i, crash when uncommented ! **/
				   k++;
				   if ((pi_i->pos.x+xos<1) || (pi_i->pos.y+yos<1)) lb=0; /* lord binary, fixes crash when calling npcvalid with negative coordiantes */
				   else lb=Movement->validNPCMove(pi_i->pos.x+xos,pi_i->pos.y+yos,pi_i->pos.z,DEREF_P_CHAR(pc_c));				 
				   
				   //Bug fix Monsters spawning on water:
				   MapStaticIterator msi(pi_i->pos.x + xos, pi_i->pos.y + yos);

				   staticrecord *stat;
				   loopexit=0;
				   while ( (stat = msi.Next()) && (++loopexit < MAXLOOPS) )
				   {
					   tile_st tile;
					   msi.GetTile(&tile);
					   if(!(strcmp((char *) tile.name, "water")))//Water
					   {//Don't spawn on water tiles... Just add other stuff here you don't want spawned on.
						   lb=0;
					   }
				   }
			   } while (!lb);
		   } // end Zippy's changes (exept for all the +xos and +yos around here....)

		   pc_c->pos.x=pi_i->pos.x+xos;
		   pc_c->pos.y=pi_i->pos.y+yos;
		   pc_c->dispz=pc_c->pos.z=pi_i->pos.z;
		   pc_c->SetSpawnSerial(pi_i->serial);
		   if ( pi_i->type == 125 )
		   {
			  MsgBoardQuestEscortCreate( DEREF_P_CHAR(pc_c) );
		   }
	   } // end of if !triggerx
		break;
	case 2: // take position from Socket
		if (s!=-1)
		{
			pc_c->pos.x=(buffer[s][11]<<8)+buffer[s][12];
			pc_c->pos.y=(buffer[s][13]<<8)+buffer[s][14];
			pc_c->dispz=pc_c->pos.z=buffer[s][16]+Map->TileHeight((buffer[s][17]<<8)+buffer[s][18]);
		}
		break;
	case 3: // take position from Parms
		pc_c->pos.x=x1;
		pc_c->pos.y=y1;
		pc_c->dispz=pc_c->pos.z=z1;
		break;
	} // no default coz we tested on entry to function

	// now we have a position, let's set the borders
	switch (pc_c->npcWander)
	{
	case 2:		// circle
		pc_c->fx1=pc_c->pos.x;
		pc_c->fy1=pc_c->pos.y;
		pc_c->fz1=pc_c->pos.z;
		pc_c->fx2=(fx2>=0) ? fx2 : 2;	// radius; if not given from script,default=2
		break;
	case 3:		// box
		if (fx1 || fx2 || fy1 || fy2)	// any offset for rectangle given from script ?
		{
			pc_c->fx1=pc_c->pos.x+fx1;
			pc_c->fx2=pc_c->pos.x+fx2;
			pc_c->fy1=pc_c->pos.y+fy1;
			pc_c->fy2=pc_c->pos.y+fy2;
			pc_c->fz1= -1;			// irrelevant for box
		}
		break;
	//default: do nothing
	}
   
	pc_c->region = calcRegionFromXY(pc_c->pos.x, pc_c->pos.y);
   
   //Now find real 'skill' based on 'baseskill' (stat modifiers)
   for(z=0;z<TRUESKILLS;z++)
   {
	   Skills->updateSkillLevel(pc_c, z);
   }
   
   updatechar(pc_c);

   // Dupois - Added April 4, 1999
   // After the NPC has been fully initialized, then post the message (if its a quest spawner) type==125
   if (postype==1) // lb crashfix
   {
   }
   // End - Dupois

   //Char mapRegions
   mapRegions->Remove(pc_c);
   mapRegions->Add(pc_c);
   return DEREF_P_CHAR(pc_c);
}

void cCharStuff::Split(P_CHAR pc_k) // For NPCs That Split during combat
{
	int c,serial,z;

	if ( pc_k == NULL ) return;
	
	c=Npcs->MemCharFree();
	
	P_CHAR pc_c = MAKE_CHARREF_LR(c);
	pc_c->Init();
	serial=pc_c->serial;
	memcpy(pc_c, pc_k, sizeof(cChar));
	pc_c->ser1=serial>>24;
	pc_c->ser2=serial>>16;
	pc_c->ser3=serial>>8;
	pc_c->ser4=serial%256;
	pc_c->serial=serial;
	pc_c->ftarg = INVALID_SERIAL;
	pc_c->MoveTo(pc_k->pos.x+1, pc_k->pos.y, pc_k->pos.z);
	pc_c->kills=0;
	pc_c->hp=pc_k->st;
	pc_c->stm=pc_k->realDex();
	pc_c->mn=pc_k->in;
	z=rand()%35;
	if (z==5) pc_c->split=1; else pc_c->split=0;	
	updatechar(pc_c);
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