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

/*
*	WOLFPACK Magic stuff
*
*	Completed the restructuration by AntiChrist (9/99)
*	Functions' description added by AntiChrist (9/99)
*
*   UO:3D particle System implementation [and packet hacking]: Lord Binary 4/2001
*/

#include "wolfpack.h"
#include "sregions.h"
#include "SndPkg.h"
#include "itemid.h"
#include "debug.h"

#undef DBGFILE
#define DBGFILE "magic.cpp"
#define NOTUSED 0

/////////////////////////////////////////////////////////////////
/// INDEX:
//		- misc magic functions
//		- NPCs casting spells related functions
//		- ITEMs magic powers related funcions
//		- PCs casting spells related functions
/////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////
/////// MISC MAGIC FUNCTIONS ////////////////////////////////////
/////////////////////////////////////////////////////////////////


///////////////////
// Name:	InitSpells
// History:	AntiChrist, 11 September 1999
// Purpose:	Initialize magery system. Parse spells.scp and store spells data.
//
int cMagic::InitSpells(void)
{
	int curspell = 0; // current spell
	
	openscript("spells.scp");
	// if (!i_scripts[spells_script]->find("SPELLA"))
	//{
	//	closescript();
	//	clConsole.send("Cannot find SPELLS SETTINGS string in spells.scp!");
	//	return false;
	//}
	
	unsigned long loopexit = 0;
	do
	{
		read2();
		if (!strcmp("SPELL", (char*)script1))			curspell = str2num(script2);
		else if (!strcmp("ENABLE", (char*)script1))		spells[curspell].enabled = str2num(script2);
		else if (!strcmp("CIRCLE", (char*)script1))		spells[curspell].circle = str2num(script2);
		else if (!strcmp("MANA", (char*)script1))		spells[curspell].mana = str2num(script2);
		else if (!strcmp("LOSKILL", (char*)script1))	spells[curspell].loskill = str2num(script2);
		else if (!strcmp("HISKILL", (char*)script1))	spells[curspell].hiskill = str2num(script2);
		else if (!strcmp("SCLO", (char*)script1))		spells[curspell].sclo = str2num(script2);
		else if (!strcmp("SCHI", (char*)script1))		spells[curspell].schi = str2num(script2);
		else if (!strcmp("MANTRA", (char*)script1))		strcpy(spells[curspell].mantra, (char*)script2);
		else if (!strcmp("ACTION", (char*)script1))		spells[curspell].action = hex2num(script2);
		else if (!strcmp("DELAY", (char*)script1))		spells[curspell].delay = str2num(script2);
		else if (!strcmp("ASH", (char*)script1))		spells[curspell].reagents.ash = str2num(script2);
		else if (!strcmp("DRAKE", (char*)script1))		spells[curspell].reagents.drake = str2num(script2);
		else if (!strcmp("GARLIC", (char*)script1))		spells[curspell].reagents.garlic = str2num(script2);
		else if (!strcmp("GINSING", (char*)script1))	spells[curspell].reagents.ginseng = str2num(script2);
		else if (!strcmp("MOSS", (char*)script1))		spells[curspell].reagents.moss = str2num(script2);
		else if (!strcmp("PEARL", (char*)script1))		spells[curspell].reagents.pearl = str2num(script2);
		else if (!strcmp("SHADE", (char*)script1))		spells[curspell].reagents.shade = str2num(script2);
		else if (!strcmp("SILK", (char*)script1))		spells[curspell].reagents.silk = str2num(script2);
		else if (!strcmp("TARG", (char*)script1))		strcpy(spells[curspell].strToSay, (char*)script2);
		else if (!strcmp("REFLECT", (char*)script1))	spells[curspell].reflect = str2num(script2);
		else if (!strcmp("RUNIC", (char*)script1))		spells[curspell].runic = str2num(script2);
	}
	while ((strcmp((char*)script1, "EOF")) &&(++loopexit < MAXLOOPS));
	closescript();
	return true;
}

// Spellbook
// Purpose:	Sends the spellbook item (with all the
//			memorized spells) to player when doubleclicked.

void cMagic::SpellBook(UOXSOCKET s, P_ITEM pi)
{
	if (pi == NULL)
		return;
	
//	CHARACTER cc=currchar[s];
	P_CHAR pc_currchar = currchar[s];
	if (!pi && pc_currchar->packitem != INVALID_SERIAL)
	{
		unsigned int ci=0;
		P_ITEM pj;
		vector<SERIAL> vecContainer = contsp.getData(pc_currchar->packitem);
		for ( ci = 0; ci < vecContainer.size(); ci++)
		{
			pj = FindItemBySerial(vecContainer[ci]);

			if (pj->type==9)
			{
				pi=pj;
				break;
			}
		}
	}
	if (!pi)
	{
		unsigned int ci=0;
		P_ITEM pj;
		vector<SERIAL> vecContainer = contsp.getData(pc_currchar->serial);
		for ( ci = 0; ci < vecContainer.size(); ci++)
		{
			pj = FindItemBySerial(vecContainer[ci]);
			if (pj->type==9 && pj->layer==1)
			{
				pi=pj;
				break;
			}
		}
	}
	// LB remark: If you want more than one spellbook per player working predictable
	// quite a lot of that function needs to be rewritten !
	// reason: just have a look at the loop above ...

	if (!pi ||	// no book at all
		(pc_currchar->packitem != INVALID_SERIAL && pi->contserial != pc_currchar->packitem &&	// not in primary pack
				!pc_currchar->Wears(pi)))		// not equipped
	{
		sysmessage(s, "In order to open spellbook, it must be equipped in your hand or in the first layer of your backpack.");
		return;
	}

	if (pi->layer!=1) senditem(s,pi); // prevents crash if pack not open

	char sbookstart[8]="\x24\x40\x01\x02\x03\xFF\xFF";
	sbookstart[1]=pi->ser1;
	sbookstart[2]=pi->ser2;
	sbookstart[3]=pi->ser3;
	sbookstart[4]=pi->ser4;
	Xsend(s, sbookstart, 7);

	int spells[70] = {0,};
	int i, scount=0;

	unsigned int ci=0;
	P_ITEM pj;
	vector<SERIAL> vecContainer = contsp.getData(pi->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pj = FindItemBySerial(vecContainer[ci]);
		if (IsSpellScroll72(pj->id()))
		{
			spells[pj->id()-0x1F2D]=1;
		}
	}

	// Fix for Reactive Armor/Bird's Eye dumbness. :)
	i=spells[0];
	spells[0]=spells[1];
	spells[1]=spells[2];
	spells[2]=spells[3];
	spells[3]=spells[4];
	spells[4]=spells[5];
	spells[5]=spells[6]; // Morac is right! :)
	spells[6]=i;
	// End fix.

	if (spells[64])
	{
		for (i=0;i<70;i++)
			spells[i]=1;
		spells[64]=0;
	}
	spells[64]=spells[65];
	spells[65]=0;

	for (i=0;i<70;i++)
	{
		if (spells[i]) scount++;
	}
	char sbookinit[6]="\x3C\x00\x3E\x00\x03";
	sbookinit[1]=((scount*19)+5)>>8;
	sbookinit[2]=((scount*19)+5)%256;
	sbookinit[3]=scount>>8;
	sbookinit[4]=scount%256;
	if (scount>0) Xsend(s, sbookinit, 5);

	char sbookspell[20]="\x40\x01\x02\x03\x1F\x2E\x00\x00\x01\x00\x48\x00\x7D\x40\x01\x02\x03\x00\x00";
	for (i=0;i<70;i++)
	{
		if (spells[i])
		{
			sbookspell[0]=0x41;
			sbookspell[1]=0x00;
			sbookspell[2]=0x00;
			sbookspell[3]=i+1;
			sbookspell[8]=i+1;
			sbookspell[13]=pi->ser1;
			sbookspell[14]=pi->ser2;
			sbookspell[15]=pi->ser3;
			sbookspell[16]=pi->ser4;
			Xsend(s, sbookspell, 19);
		}
	}
}


///////////////////
// Name:	GateCollision
// History:	Unknown, Modified by AntiChrist - collision dir
// Purpose:	Used when a PLAYER passes throu a gate. Takes the player
//			to the other side of the gate-link.
//
char cMagic::GateCollision(PLAYER s)
{
	unsigned int n;
//	extern cRegion *mapRegions;
	P_CHAR pc_player = MAKE_CHAR_REF(s);

	// Check to make sure that this isn't a NPC (they shouldn't go throught gates)
	if( pc_player->isNpc() )
	return 0;

	// Now check whether the PC (player character) has moved or simply turned
	// If they have only turned, then ignore checking for a gate collision since it would
	// have happened the previous time
	if( ( pc_player->pos.x == pc_player->prevX ) &&
		( pc_player->pos.y == pc_player->prevY ) &&
		( pc_player->pos.z == pc_player->prevZ )	)
	return 0;

	// - Tauriel's region stuff 3/6/99
	int getcell = mapRegions->GetCell(pc_player->pos.x,pc_player->pos.y);

	vector<SERIAL> vecEntries = mapRegions->GetCellEntries(getcell);
	for (unsigned int k = 0; k < vecEntries.size(); k++)
	{
		if (vecEntries.size() == 0) 
			break;
		if (!isItemSerial(vecEntries[k]))
			continue;
		P_ITEM mapitem = FindItemBySerial(vecEntries[k]);
		if (mapitem != NULL)
		{
			if (mapitem->type == 51 || (mapitem->type==52))
			{
				if (mapitem->type==51) n=1;
				else n=0;
				if ((pc_player->pos.x==mapitem->pos.x)&&
					(pc_player->pos.y==mapitem->pos.y)&&
					(pc_player->pos.z>=mapitem->pos.z))
				{
			// Dupois - Check for any NPC's that are following this player
			//			There has to be a better way than this...
			//			Think about it some more and change this.
			// If this is a player character
					if ( pc_player->isPlayer() )
					{
						// Look for an NPC
						for ( unsigned int index = 0; index < charcount; index++ )
						{
							// That is following this player character
							if ( (chars[index].isNpc()) && (chars[index].ftarg == pc_player->serial) )
							{
								// If the NPC that is following this player character is within 5 paces
								if ( chardist(s, index)<=4 )
								{
									// Teleport the NPC along with the player
									chars[index].MoveTo(gatex[mapitem->gatenumber][n], gatey[mapitem->gatenumber][n], gatez[mapitem->gatenumber][n]);
									teleport(index);
								}
							}
						}
					}
					// Set the characters destination
					pc_player->MoveTo(gatex[mapitem->gatenumber][n], gatey[mapitem->gatenumber][n], gatez[mapitem->gatenumber][n]);
					teleport(s);
					soundeffect( calcSocketFromChar( s ), 0x01, 0xFE );
					staticeffect( s, 0x37, 0x2A, 0x09, 0x06 );
				}
			}
		}
	}

	// Since the character has moved a step update the prevXYZ values
	// to prevent the "bounce back" effect of the GateCollision check
	pc_player->prevX = pc_player->pos.x;
	pc_player->prevY = pc_player->pos.y;
	pc_player->prevZ = pc_player->pos.z;

	return(1);
}

///////////////////
// Name:	SummonMonster
// History:	Unknown
// Purpose:	Summon a monster (dispellable with DISPEL).
//
void cMagic::SummonMonster(UOXSOCKET s, unsigned char id1, unsigned char id2, char * monstername, unsigned char color1, unsigned char color2, int x, int y, int z, int spellnum)
{
	CHARACTER c;
//	CHARACTER cc = currchar[s];
	P_CHAR pc_currchar = currchar[s];
	P_CHAR pc_monster = NULL;
	int id=(id1<<8)+id2;
	UI16 color = (color1 << 8) + color2;
	switch(id)
	{
	case 0x0000:	// summon monster
 		soundeffect( s, 0x02, 0x15 );
 		c=Npcs->AddRandomNPC( s, "10000", -1 );
 		if( c == -1 )
 		{
 			sysmessage( s, "Contact your shard op to setup the summon list!" );
 			return;
 		}
		pc_monster = MAKE_CHARREF_LR(c);
		pc_monster->SetOwnSerial(pc_currchar->serial);
		pc_monster->MoveTo(pc_currchar->pos.x+rand()%2, pc_currchar->pos.y+rand()%2, pc_currchar->pos.z);
 		pc_monster->summontimer=(uiCurrentTime+((pc_currchar->skill[MAGERY]/10)*(MY_CLOCKS_PER_SEC*2)));
 		updatechar(DEREF_P_CHAR(pc_monster));
 		npcaction(DEREF_P_CHAR(pc_monster), 0x0C);
		doStaticEffect(DEREF_P_CHAR(pc_monster), spellnum);
 		return;

	case 0x000D: // Energy Vortex & Air elemental
		if (color1==0x00 && color2==0x75)
		{
			soundeffect(s, 0x02, 0x12); // EV
			c=Npcs->AddNPCxyz(s,295,0,pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);
			pc_monster = MAKE_CHARREF_LR(c);
            pc_monster->summontimer=(uiCurrentTime+((pc_currchar->skill[MAGERY]/10)*(MY_CLOCKS_PER_SEC*2)));
			pc_monster->npcaitype=50;
			pc_monster->tamed=false;			
		}
		else
		{
			soundeffect(s, 0x02, 0x17); // AE
			c=Npcs->AddNPCxyz(s,291,0,pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);
			pc_monster = MAKE_CHARREF_LR(c);
			pc_monster->summontimer=(uiCurrentTime+((pc_currchar->skill[MAGERY]/10)*(MY_CLOCKS_PER_SEC*2)));
			pc_monster->npcaitype=50;
			pc_monster->tamed=true;
		}
		break;
	case 0x000A: // Daemon
		soundeffect(s, 0x02, 0x16);
		c=Npcs->AddNPCxyz(s,290,0,pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);
		pc_monster = MAKE_CHARREF_LR(c);
		pc_monster->summontimer=(uiCurrentTime+((pc_currchar->skill[MAGERY]/10)*(MY_CLOCKS_PER_SEC*2)));
		pc_monster->tamed=true;
		break;
	case 0x000E: //Earth
		soundeffect(s, 0x02, 0x17);
		c=Npcs->AddNPCxyz(s,292,0,pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);
		pc_monster = MAKE_CHARREF_LR(c);
		pc_monster->summontimer=(uiCurrentTime+((pc_currchar->skill[MAGERY]/10)*(MY_CLOCKS_PER_SEC*2)));
		pc_monster->tamed=true;
		break;
	case 0x000F: //Fire
		soundeffect(s, 0x02, 0x17);
		c=Npcs->AddNPCxyz(s,293,0,pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);
		pc_monster = MAKE_CHARREF_LR(c);
		pc_monster->summontimer=(uiCurrentTime+((pc_currchar->skill[MAGERY]/10)*(MY_CLOCKS_PER_SEC*2)));
		pc_monster->tamed=true;
		break;
	case 0x0010: //Water
		soundeffect(s, 0x02, 0x17);
		c=Npcs->AddNPCxyz(s,294,0,pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);
		pc_monster = MAKE_CHARREF_LR(c);
		pc_monster->summontimer=(uiCurrentTime+((pc_currchar->skill[MAGERY]/10)*(MY_CLOCKS_PER_SEC*2)));
		pc_monster->tamed=true;
		break;
	case 0x023E: //Blade Spirits
		soundeffect(s, 0x02, 0x12); // I don't know if this is the right effect...	
		c=Npcs->AddNPCxyz(s,296,0,pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);
		pc_monster = MAKE_CHARREF_LR(c);
		pc_monster->summontimer=(uiCurrentTime+((pc_currchar->skill[MAGERY]/10)*(MY_CLOCKS_PER_SEC*2)));
		pc_monster->npcaitype=50;
		pc_monster->tamed=false;
		break;
	case 0x03e2: // Dupre The Hero
		soundeffect(s, 0x02, 0x46);
		c=Npcs->MemCharFree ();
		pc_monster = MAKE_CHARREF_LR(c);
		pc_monster->Init();
		pc_monster->def=50;
		pc_monster->lodamage=50;
		pc_monster->hidamage=100;
		pc_monster->spattack=7552; // 1-7 level spells
		pc_monster->baseskill[MAGERY]=900; // 90 magery
		pc_monster->baseskill[TACTICS]=1000; // 70 tactics
		pc_monster->baseskill[WRESTLING]=900;
		pc_monster->baseskill[SWORDSMANSHIP]=1000;
		pc_monster->baseskill[PARRYING]=1000;
		pc_monster->skill[MAGICRESISTANCE]=650;
		pc_monster->st=pc_monster->hp=600;
		pc_monster->setDex(70);
		pc_monster->stm=70;
		pc_monster->in=pc_monster->mn=100;
		pc_monster->fame=10000;
		pc_monster->karma=10000;
		break;
	case 0x000B: // Black Night
		soundeffect(s, 0x02, 0x16);
		c=Npcs->MemCharFree ();
		pc_monster = MAKE_CHARREF_LR(c);
		pc_monster->Init();
		pc_monster->def=50;
		pc_monster->lodamage=50;
		pc_monster->hidamage=100;
		pc_monster->spattack=7552; // 1-7 level spells
		pc_monster->baseskill[MAGERY]=1000; // 100 magery
		pc_monster->baseskill[TACTICS]=1000;// 100 tactics
		pc_monster->baseskill[WRESTLING]=900;
		pc_monster->baseskill[SWORDSMANSHIP]=1000;
		pc_monster->baseskill[PARRYING]=1000;
		pc_monster->skill[MAGICRESISTANCE]=1000;
		pc_monster->st=pc_monster->hp=600;
		pc_monster->setDex(70);
		pc_monster->stm=70;
		pc_monster->in=pc_monster->mn=100;
		break;
	case 0x0190: // Death Knight
		soundeffect(s, 0x02, 0x46);
		c=Npcs->MemCharFree ();
		pc_monster = MAKE_CHARREF_LR(c);
		pc_monster->Init();
		pc_monster->def=20;
		pc_monster->lodamage=10;
		pc_monster->hidamage=45;
		pc_monster->spattack=4095; // 1-7 level spells
		pc_monster->baseskill[MAGERY]=500; // 90 magery
		pc_monster->baseskill[TACTICS]=1000; // 70 tactics
		pc_monster->baseskill[WRESTLING]=900;
		pc_monster->baseskill[SWORDSMANSHIP]=1000;
		pc_monster->baseskill[PARRYING]=1000;
		pc_monster->skill[MAGICRESISTANCE]=650;
		pc_monster->st=pc_monster->hp=600;
		pc_monster->setDex(70);
		pc_monster->stm=70;
		pc_monster->in=pc_monster->mn=100;
		pc_monster->fame=-10000;
		pc_monster->karma=-10000;
		break;
	default:
		soundeffect(s, 0x02, 0x15);
		return;
	}

	strcpy(pc_monster->name, monstername);

	pc_monster->id1=pc_monster->xid1=id1;
	pc_monster->id2=pc_monster->xid2=id2;
	pc_monster->skin = pc_monster->xskin = color;
	pc_monster->priv2=0x20;
	pc_monster->npc=1;

	if (id!=0x023E && !(id==0x000d && color1==0 && color2==0x75)) // don't own BS or EV.
		pc_monster->SetOwnSerial(pc_currchar->serial); 

	if (x==0)
	{
		pc_monster->MoveTo(pc_currchar->pos.x-1, pc_currchar->pos.y, pc_currchar->pos.z);
	}
	else
	{
		pc_monster->MoveTo(x, y, z);
	}

	pc_monster->spadelay=10;
	pc_monster->summontimer=(uiCurrentTime+((pc_currchar->skill[MAGERY]/10)*(MY_CLOCKS_PER_SEC*2)));
	updatechar(DEREF_P_CHAR(pc_monster));
	npcaction(DEREF_P_CHAR(pc_monster), 0x0C);
	doStaticEffect(DEREF_P_CHAR(pc_monster), spellnum);

	// AntiChrist (9/99) - added the chance to make the monster attack
	// the person you targeted ( if you targeted a char, naturally :) )
	int serial=LongFromCharPtr(buffer[s]+7);
	if (serial==-1) return;
	P_CHAR pc_i = FindCharBySerial( serial );
	if(pc_i == NULL) return;

	npcattacktarget(DEREF_P_CHAR(pc_i), c);
}

///////////////////
// Name:	CheckBook
// History:	Unknown
// Purpose:	Check if the spell is memorized into the spellbook.
//
bool cMagic::CheckBook(int circle, int spell, P_ITEM pi)
{
	bool raflag = false;

	int spellnum = spell+(circle-1)*8;
	// Fix for OSI stupidity. :)
	if (spellnum==6) raflag = true;
	if (spellnum>=0 && spellnum<6) spellnum++;
	if (raflag) spellnum=0;

	unsigned int ci=0;
	P_ITEM pj;
	vector<SERIAL> vecContainer = contsp.getData(pi->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pj = FindItemBySerial(vecContainer[ci]);
		if((pj->id()==(0x1F2D+spellnum) || pj->id()==0x1F6D))
		{
			return false;
		}
	}
	return true;
}

int cMagic::SpellsInBook(P_ITEM pi)
{
	int ci = 0;
	int spellcount = 0;
	P_ITEM pj;
	if (pi == NULL)
		return -1;
	vector<SERIAL> vecContainer = contsp.getData(pi->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pj = FindItemBySerial(vecContainer[ci]);
		if (pj->id() == 0x1F6D)
			spellcount = 64;
		else
			spellcount++;
	}
	if (spellcount >= 64)
		spellcount = 64;
	return spellcount;
}


///////////////////
// Name:	SbOpenContainer
// History:	Unknown
// Purpose:	Open player's spellbook as a container.
//			(used for SBOPEN command)
//
void cMagic::SbOpenContainer(UOXSOCKET s)
{
	P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (pi)
	{
		if ((pi->type==9))
			backpack(s, pi->serial);
		else
			sysmessage(s,"That is not a spellbook.");
	}
}



///////////////////
// Name:	CheckMana
// History:	Unknown, Modified by AntiChrist to use spells[] array.
// Purpose:	Check if character has enought mana to cast a spell of that circle.
//
char cMagic::CheckMana(CHARACTER s, int num)
{

	if (chars[s].priv2&0x10)
		return 1;

	if (chars[s].mn >= spells[num].mana) 
		return 1;
	else 
	{
		UOXSOCKET p = calcSocketFromChar(s);
		if (p != -1) 
			sysmessage(p, "You have insufficient mana to cast that spell.");
		return 0;
	}
}



///////////////////
// Name:	SubtractMana
// History:	Unknown
// Purpose:	Substract the required mana from character's mana reserve.
//
char cMagic::SubtractMana(P_CHAR pc, int mana)
{
	if (pc->priv2&0x10)
		return 1;

	if (pc->mn >= mana)
		pc->mn-=mana;
	else 
		pc->mn = 0;

	updatestats((pc), 1);//AntiChrist - bugfix
	return 1;
}

///////////////////
// Name:	CheckMagicReflect
// History:	Unknown
// Purpose:	Check if character is protected by MagicReflect;
//			if yes, remove the protection and do visual effect.
//
bool cMagic::CheckMagicReflect(CHARACTER i)
{
	if (chars[i].priv2&0x40)
	{
		chars[i].priv2 &= 0xBF;
		staticeffect(i, 0x37, 0x3A, 0, 15);
		return true;
	}
	return false;
}

P_CHAR cMagic::CheckMagicReflect(P_CHAR &attacker, P_CHAR &defender)
{
	if (defender->priv2&0x40)
	{
		defender->priv2 &= 0xBF;
		staticeffect(DEREF_P_CHAR(defender), 0x37, 0x3A, 0, 15);
		return attacker;
	}
	return defender;
}


///////////////////
// Name:	CheckResist
// History:	Unknown, Modified by AntiChrist to add EV.INT. check
// Purpose:	Check character's magic resistance.
//
char cMagic::CheckResist(CHARACTER attacker, CHARACTER defender, int circle)
{
	char i=Skills->CheckSkill(defender, MAGICRESISTANCE, 80*circle, 800+(80*circle));

	if (i)
	{
		//AntiChrist - 7/10/99
		//A higher Eval.Int. can disable opponent's resistspell!
		if(attacker>=0) //NOTE: only do the EV.INT. check if attacker >= 0
		{
			if(chars[attacker].skill[EVALUATINGINTEL]<chars[defender].skill[MAGICRESISTANCE] && (rand()%3)==0)
			{
				UOXSOCKET s = calcSocketFromChar(defender);
				if (s!=-1)
					sysmessage(s, "You feel yourself resisting magical energy!");
			} else
				return 0;
		} else {//attacker=-1: used for checking field effects
			UOXSOCKET s = calcSocketFromChar(defender);
			if (s!=-1)
				sysmessage(s, "You feel yourself resisting magical energy!");
		}
	}
	return i;
}



///////////////////
// Name:	MagicDamage
// History:	Unknown
// Purpose:	Calculate and inflict magic damage.
//
void cMagic::MagicDamage(CHARACTER p, int amount)
{
	P_CHAR pc = MAKE_CHAR_REF(p);
	MagicDamage(pc, amount);
}

void cMagic::MagicDamage(P_CHAR pc, int amount)
{
	if ( pc->priv2&0x02  &&  pc->effDex() > 0 )
	{
		pc->priv2 &= 0xFD; // unfreeze
		UOXSOCKET s = calcSocketFromChar(pc);
		if (s != -1) sysmessage(s, "You are no longer frozen.");
	}
	if ( !pc->isInvul() && (region[pc->region].priv&0x40)) // LB magic-region change
	{
		if (pc->isNpc()) amount *= 2;			// double damage against non-players
		pc->hp = max(0, pc->hp-amount);
		updatestats((pc), 0);
		if (pc->hp <= 0)
		{
			deathstuff(DEREF_P_CHAR(pc));
		}
	}
}


///////////////////
// Name:	PoisonDamage
// History:	Unknown
// Purpose:	Apply the poison to the character.
//
void cMagic::PoisonDamage(CHARACTER p, int poison) // new functionality, lb !!!
{
	UOXSOCKET s = calcSocketFromChar(p);

	if (chars[p].priv2&0x02)
	{
		chars[p].priv2 &= 0xFD;
		if (s!=-1) sysmessage(s, "You are no longer frozen.");
	}
	if ( !chars[p].isInvul() && (region[chars[p].region].priv&0x40)) // LB magic-region change
	{
		if (poison>5) poison = 5;
		else if (poison<0) poison = 1;
		chars[p].poisoned=poison;
		chars[p].poisonwearofftime=uiCurrentTime+(MY_CLOCKS_PER_SEC*SrvParms->poisontimer);	// lb
		if (s != -1) 
			impowncreate(s, p, 1); //Lb, sends the green bar !
	}
}



///////////////////
// Name:	CheckFieldEffects2
// History:	Unknown
// Purpose:	Check if character stands on a magic-field, and apply effects.
//
// timecheck: 0: always executed no matter of the nextfieldspelltime value
// timecheck: 1: only executed if the time is right for next fieldeffect check
// we need this cauz its called from npccheck and pc-check
// npc-check already has its own timer, pc check not.
// thus in npccheck its called with 0, in pc check with 1
// we could add the fieldeffect check time the server.scp but i think this solution is better.
// LB October 99
//

void cMagic::CheckFieldEffects2(unsigned int currenttime, CHARACTER c,char timecheck)//c=character (Not socket) //Lag fix -- Zippy

{
	// - Tauriel's region stuff 3/6/99

	int j;

	if (timecheck)
	{
		if (nextfieldeffecttime<=currenttime) j=1; else j=0;
	} else j=1;

	if (j)
	{
		cRegion::RegionIterator4Items ri(chars[c].pos);
		for ( ri.Begin(); ri.GetData() != ri.End(); ri++)
		{
			P_ITEM mapitem = ri.GetData();
			if (mapitem != NULL)
			{
				//clConsole.send("itemname: %s\n",items[mapitem].name);// perfect for mapregion debugging, LB
				if ((mapitem->pos.x==chars[c].pos.x)&&(mapitem->pos.y==chars[c].pos.y))	// lb
				{
					if (mapitem->id()==0x3996 || mapitem->id()==0x398C)
					{
						if (!CheckResist(-1, c, 4))
							MagicDamage(c, mapitem->morex/100);
						else
							MagicDamage(c, mapitem->morex/200);
						soundeffect2(c, 2, 8);
						return; //Ripper
					} else if (mapitem->id()==0x3915 || mapitem->id()==0x3920)
					{//Poison Field
						if (!CheckResist(-1, c, 5))
						{
							if ((mapitem->morex<997))
								PoisonDamage(c,2);
							else
								PoisonDamage(c,3); // gm mages can cast greater poison field, LB
						} else PoisonDamage(c,1); // cant be completly resited
						
						soundeffect2(c, 2, 8);
						return; //Ripper
					} else if (mapitem->id()==0x3979 || mapitem->id()==0x3967)
					{//Para Field
						if (!CheckResist(-1, c, 6))
							tempeffect(c, c, 1, 0, 0, 0);
						soundeffect2(c, 0x02, 0x04);
						return; //Ripper
					}
					break;
				}
			}
		}
	}
}

///////////////////
// Name:	BoxSpell
// History:	Unknown
// Purpose:	Calculate the spell box effect, depending on character's magery skill.
//
void cMagic::BoxSpell(UOXSOCKET s, int& x1, int& x2, int& y1, int& y2, int& z1, int& z2)
{
	int x, y, z, lenght;

	x=(buffer[s][11]<<8)+buffer[s][12];
	y=(buffer[s][13]<<8)+buffer[s][14];
	z=buffer[s][16];

	lenght=currchar[s]->skill[MAGERY]/170; // increased max-range, LB

	x1=x-lenght;
	x2=x+lenght;
	y1=y-lenght;
	y2=y+lenght;
	z1=z;
	z2=z+3;
}

///////////////////
// Name:	MagicTrap
// History:	Unknown
// Purpose:	Do the visual effect and apply magic damage when a player opens a trapped container.
//
void cMagic::MagicTrap(PLAYER s, P_ITEM pTrap)
{
	if (!pTrap) return;
	staticeffect(s, 0x36, 0xB0, 0x09, 0x09);
	soundeffect2(s, 0x02, 0x07);
	if(CheckResist(-1, s, 4)) 
		MagicDamage(s,pTrap->moreb2);
	else 
		MagicDamage(s,pTrap->moreb2/2);
	pTrap->moreb1=0;
	pTrap->moreb2=0;
	pTrap->moreb3=0;
}

///////////////////
// Name:	CheckReagents
// History:	Unknown, Modified by AntiChrist to use reag_st
// Purpose:	Check for required reagents in player's backpack.
//
char cMagic::CheckReagents(CHARACTER s, reag_st reagents)
{
	reag_st failmsg;

	if (chars[s].priv2&0x80) return 1;

	memset(&failmsg,0,sizeof(reag_st)); // set all members to 0

	if (reagents.ash!=0 && getamount(s, 0x0F8C)<reagents.ash)
		failmsg.ash=1;
	if (reagents.drake!=0 && getamount(s, 0x0F86)<reagents.drake)
		failmsg.drake=1;
	if (reagents.garlic!=0 && getamount(s, 0x0F84)<reagents.garlic)
		failmsg.garlic=1;
	if (reagents.ginseng!=0 && getamount(s, 0x0F85)<reagents.ginseng)
		failmsg.ginseng=1;
	if (reagents.moss!=0 && getamount(s, 0x0F7B)<reagents.moss)
		failmsg.moss=1;
	if (reagents.pearl!=0 && getamount(s, 0x0F7A)<reagents.pearl)
		failmsg.pearl=1;
	if (reagents.shade!=0 && getamount(s, 0x0F88)<reagents.shade)
		failmsg.shade=1;
	if (reagents.silk!=0 && getamount(s, 0x0F8D)<reagents.silk)
		failmsg.silk=1;

	int fail = RegMsg(s,failmsg);

	return fail;
}

///////////////////
// Name:	RegMsg
// History:	Unknown, Reprogrammed by AntiChrist to display missing reagents types.
// Purpose:	Display an error message if character has no enougth resgs.
//
int cMagic::RegMsg(CHARACTER s, reag_st failmsg)
{
	bool display = false;
	char message[100] = {0,};  //just to make sure we end with '\0'

	strcpy(message, "You do not have enough reagents to cast that spell.[");

	if (failmsg.ash)	{ display = true; strcat(message,"Sa,"); }
	if (failmsg.drake)	{ display = true; strcat(message,"Mr,"); }
	if (failmsg.garlic)	{ display = true; strcat(message,"Ga,"); }
	if (failmsg.ginseng){ display = true; strcat(message,"Gi,"); }
	if (failmsg.moss)	{ display = true; strcat(message,"Bm,"); }
	if (failmsg.pearl)	{ display = true; strcat(message,"Bp,"); }
	if (failmsg.shade)	{ display = true; strcat(message,"Ns,"); }
	if (failmsg.silk)	{ display = true; strcat(message,"Ss,"); }

	message[strlen(message)-1]=']';

	if (display)
	{
		UOXSOCKET i = calcSocketFromChar(s);
		if (i != -1) 
			sysmessage(i, message);
		return 0;
	}

	return 1;
}


///////////////////
// Name:	PFireballTarget
// History:	Unknown
// Purpose:	Calculate and inflict a magic damage.
//
void cMagic::PFireballTarget(int i, int k, int j) //j = % dammage
{
	int dmg;
	movingeffect(i, k, 0x36, 0xD5, 0x05, 0x00, 0x01);
	soundeffect2(i, 0x1, 0x5E);
	// do we have to calculate attacker hp percentage,
	// or defender hp percentage?!?!?!
	dmg=(int)(((float)chars[k].hp/100) * j);
	MagicDamage(k, dmg);
}

///////////////////
// Name:	SpellFail
// History:	Unknown
// Purpose:	Do visual and sound effects when a player fails to cast a spell.
//
void cMagic::SpellFail(UOXSOCKET s)
{
	P_CHAR pc_currchar = currchar[s];
	//Use Reagents on failure ( if casting from spellbook )
	if (currentSpellType[s]==0) DelReagents( DEREF_P_CHAR(pc_currchar), spells[pc_currchar->spell].reagents );

	//npcaction(cc, 128); // whaaaaaaaaaaaaaat ?
	//orders the PG to move a step on, but the pg doesn't really move
	//disappearing from the other clients. solarin
	
	if ( rand()%5==2 ) doStaticEffect(DEREF_P_CHAR(pc_currchar), 99); else staticeffect(DEREF_P_CHAR(pc_currchar), 0x37, 0x35, 0, 30);
	soundeffect2(DEREF_P_CHAR(pc_currchar), 0x00, 0x5C);
	npcemote(s, DEREF_P_CHAR(pc_currchar), "The spell fizzles.",0);
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////// NPCs CASTING SPELLS RELATED FUNCTIONS ///////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////


///////////////////
// Name:	NPCLightningTarget
// History:	Unknown
// Purpose:	Used for NPC; cast a lightning spell.
//
void cMagic::NPCLightningTarget(CHARACTER nAttacker, CHARACTER nDefender)
{
	P_CHAR pc_Attacker = MAKE_CHARREF_LR(nAttacker);
	P_CHAR pc_Defender = MAKE_CHARREF_LR(nDefender);
	P_CHAR pc_trg = CheckMagicReflect(pc_Attacker, pc_Defender);
	
	CHARACTER trg = DEREF_P_CHAR(pc_trg);
	SubtractMana(pc_Attacker, 11);
	bolteffect(trg, true);
	soundeffect2(trg, 0x00, 0x29);

	if (CheckResist(nAttacker, trg, 4))
	{
		MagicDamage(pc_trg, pc_Attacker->skill[MAGERY]/180+RandomNum(1,2));
		//MagicDamage(t, (2+(rand()%3)+1)*(pc_currchar->skill[MAGERY]/1000+1));
	}
	else
	{
		MagicDamage(pc_trg, (pc_Attacker->skill[MAGERY]+1*(pc_Attacker->skill[EVALUATINGINTEL]/3))/(89+1*(pc_trg->skill[MAGICRESISTANCE]/30))+RandomNum(1,5));
		//MagicDamage(t, (4+(rand()%5)+2)*(pc_currchar->skill[MAGERY]/750+1));
	}
	return;
}

void cMagic::NPCHeal(CHARACTER s)
{
    int loskill=spells[10].loskill;
    int hiskill=spells[10].hiskill;

	if (!Skills->CheckSkill(s, MAGERY, loskill, hiskill))
	{
		UOXSOCKET ss=calcSocketFromChar(s);
		if (ss>-1)
		{
			SpellFail(ss);
		}
		return;
	}
	if (CheckMana(s,10))
	{
		P_CHAR pc = MAKE_CHARREF_LR(s);
		SubtractMana(pc, 10);
		int j=pc->hp+(pc->skill[MAGERY]/30+RandomNum(1,12));
		pc->hp=min(pc->st, j);
		doStaticEffect(DEREF_P_CHAR(pc), 4);
		updatestats(pc, 0);
	}
}

void cMagic::NPCCure(CHARACTER s)
{
    int loskill=spells[11].loskill;
    int hiskill=spells[11].hiskill;
	if (!Skills->CheckSkill(s, MAGERY, loskill, hiskill))
	{
		UOXSOCKET ss=calcSocketFromChar(s);
		if (ss>-1)
		{
			SpellFail(ss);
		}
		return;
	}
	if (CheckMana(s,11))
	{
		P_CHAR pc = MAKE_CHARREF_LR(s);
		doStaticEffect(s, 11);
		SubtractMana(pc,5);
		pc->poisoned=0;
		pc->poisonwearofftime=uiCurrentTime;
		npcemoteall(s,"Laughs at the poison attempt",0);
	}

}

void cMagic::NPCDispel(CHARACTER s, CHARACTER i)
{
	int loskill=spells[41].loskill;
	int hiskill=spells[41].hiskill;
	if (!Skills->CheckSkill(s, MAGERY, loskill, hiskill))
	{
		UOXSOCKET ss=calcSocketFromChar(s);
		if (ss>-1)
		{
			SpellFail(ss);
		}
		return;
	}
	if (CheckMana(s,41))
	{
		if (chars[i].priv2&0x20)
		{
			P_CHAR pc_s = MAKE_CHARREF_LR(s);
			P_CHAR pc_i = MAKE_CHARREF_LR(i);
			SubtractMana(pc_s,20);
			tileeffect(pc_i->pos.x,pc_i->pos.y,pc_i->pos.z, 0x37, 0x2A, 0x00, 0x00);
			if (pc_i->isNpc()) Npcs->DeleteChar(i);
			else deathstuff(i);
		}
	}
}

///////////////////
// Name:	NPCEBoltTarget
// History:	Unknown
// Purpose:	Used for NPC; cast a energy bolt spell.
//
void cMagic::NPCEBoltTarget(CHARACTER s, CHARACTER t)
{
	P_CHAR pc_attacker = MAKE_CHARREF_LR(s);
	P_CHAR pc_defender = MAKE_CHARREF_LR(t);
	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	CHARACTER trg = DEREF_P_CHAR(pc_target);

	SubtractMana(pc_attacker, 20);
	doMoveEffect(trg, t, 42);
	soundeffect2(trg, 0x2, 0x0A);

	if (CheckResist(s, t, 6))
	{
		MagicDamage(pc_target, pc_attacker->skill[MAGERY]/120);
		//MagicDamage(t, (4+(rand()%4)+2)*(pc_currchar->skill[MAGERY]/1000+1));
	}
	else
	{
		MagicDamage(pc_target, (pc_attacker->skill[MAGERY]+1*(pc_attacker->skill[EVALUATINGINTEL]/3))/(34+1*(pc_defender->skill[MAGICRESISTANCE]/30))+RandomNum(1,10));
		//MagicDamage(t, (8+(rand()%5)+4)*(pc_currchar->skill[MAGERY]/750+1));
	}
	return;
}

///////////////////
// Name:	NPCCannonTarget
// History:	Unknown
// Purpose:	Used for NPC; cast a cannon spell.
//
void cMagic::NPCCannonTarget(CHARACTER s, CHARACTER t)
{

	if (CheckMagicReflect(t)) t=s;

	staticeffect(t, 0x36, 0xB0, 0x09, 0x09);
	soundeffect2(t, 0x02, 0x07);

	//Char mapRegions

	cRegion::RegionIterator4Chars ri(chars[s].pos);
	for (ri.Begin(); ri.End() != ri.GetData(); ri++)
	{
		P_CHAR mapchar = ri.GetData();
		if (mapchar != NULL)
		{
			if (mapchar->pos.x==chars[t].pos.x && mapchar->pos.y==chars[t].pos.y && mapchar->pos.z==chars[t].pos.z)
			{
				if (CheckParry(DEREF_P_CHAR(mapchar), 6))
				{
					MagicDamage(DEREF_P_CHAR(mapchar), chars[s].skill[TACTICS]/50);
				}
				else
				{
					MagicDamage(DEREF_P_CHAR(mapchar), chars[s].skill[TACTICS]/25);
				}
			}
		}//if mapitem
	}
	return;
}

///////////////////
// Name:	CheckParry
// History:	Unknown
// Purpose:	Check player's parrying skill (for cannonblast).
//
char cMagic::CheckParry(CHARACTER player, int circle)
{
	char i=Skills->CheckSkill(player, PARRYING, 80*circle, 800+(80*circle));
	int s;
	if(i)
	{
		s=calcSocketFromChar(player);
		if (s!=-1)
		{
			sysmessage(s, "You have dodged the cannon blast, and have taken less damage.");
		}
	}
	return i;
}

///////////////////
// Name:	NPCFlameStrikeTarget
// History:	Unknown
// Purpose:	Used for NPC; cast a flame strike spell.
//
void cMagic::NPCFlameStrikeTarget(CHARACTER s, CHARACTER t)
{
	P_CHAR pc_attacker = MAKE_CHARREF_LR(s);
	P_CHAR pc_defender = MAKE_CHARREF_LR(t);
	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	CHARACTER trg = DEREF_P_CHAR(pc_target);

	SubtractMana(pc_attacker, 40);
	doStaticEffect(trg, 51);
	soundeffect2(trg, 0x02, 0x08);

	if (CheckResist(s, t, 7))
	{
		MagicDamage(pc_target, pc_attacker->skill[MAGERY]/80);
		//MagicDamage(t, (3+(rand()%11)+1)*(chars[cc].skill[MAGERY]/1000+1));
	}
	else
	{
		MagicDamage(pc_target, (pc_attacker->skill[MAGERY]+1*(pc_attacker->skill[EVALUATINGINTEL]/3))/(34+1*(pc_target->skill[MAGICRESISTANCE]/30))+RandomNum(1,25));
		//MagicDamage(t, (10+(rand()%12)+1)*(chars[cc].skill[MAGERY]/750+1));
	}
	return;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////// ITEMs MAGIC POWER RELATED FUNCTIONS /////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////


///////////////////
// Name:	MagicArrowSpellItem
// History:	Modifyed by Correa to be used by both Items and Players
// Purpose:	Cast magic arrow
//
void cMagic::MagicArrow(CHARACTER attacker, CHARACTER defender, bool usemana)
{
	if( attacker <0 || defender < 0 ) return;

	P_CHAR pc_attacker = MAKE_CHARREF_LR(attacker);
	P_CHAR pc_defender = MAKE_CHARREF_LR(defender);
	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	CHARACTER trg = DEREF_P_CHAR(pc_target);

	doMoveEffect(trg, defender, 5);
	soundeffect2(trg, 0x1, 0xE5);
	if (usemana)
		SubtractMana(pc_attacker, 4);
	if (CheckResist(attacker, trg, 1))
	{
		MagicDamage(pc_target, (1+(rand()%1)+1)*(pc_attacker->skill[MAGERY]/2000+1));
	}
	else
	{
		MagicDamage(pc_target, (1+(rand()%1)+2)*(pc_attacker->skill[MAGERY]/1500+1));
	}
	return;
}

///////////////////
// Name:	ClumsySpell
// History:	Modifyed by Correa to be used by both Items and Chars
// Purpose:	cast a clumsy spell.
//
void cMagic::ClumsySpell(CHARACTER attacker, CHARACTER defender, bool usemana)
{
	P_CHAR pc_attacker = MAKE_CHARREF_LR(attacker);
	P_CHAR pc_defender = MAKE_CHARREF_LR(defender);
	CheckMagicReflect(pc_attacker, pc_defender);
	
//	CHARACTER trg = DEREF_P_CHAR(pc_target);

	if (usemana)
		SubtractMana(pc_attacker, 4);

	doStaticEffect(defender, 1);
	soundeffect2(defender, 0x01, 0xDF);
	if (CheckResist(attacker, defender, 1)) return;
	tempeffect(attacker, defender, 3, pc_attacker->skill[MAGERY]/100, 0, 0);
	return;
}

///////////////////
// Name:	FeebleMindSpellItem
// History:	Unknown
// Purpose:	Used for ITEMS; cast a feeblemind spell.
//
void cMagic::FeebleMindSpell(CHARACTER attacker, CHARACTER defender, bool usemana)
{
	P_CHAR pc_attacker = MAKE_CHARREF_LR(attacker);
	P_CHAR pc_defender = MAKE_CHARREF_LR(defender);
	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	CHARACTER trg = DEREF_P_CHAR(pc_target);

	if (usemana)
		SubtractMana(pc_attacker, 4);

	doStaticEffect(trg, 3);
	soundeffect2(trg, 0x01, 0xE4);
	if (CheckResist(attacker, trg, 1)) return;
	tempeffect(attacker, trg, 4, pc_attacker->skill[MAGERY]/100, 0, 0);
	return;
}

///////////////////
// Name:	WeakenSpellItem
// History:	Unknown
// Purpose:	Used for ITEMS; cast a weaken spell.
//
void cMagic::WeakenSpell(CHARACTER attacker, CHARACTER defender, bool usemana)
{
	P_CHAR pc_attacker = MAKE_CHARREF_LR(attacker);
	P_CHAR pc_defender = MAKE_CHARREF_LR(defender);
	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	CHARACTER trg = DEREF_P_CHAR(pc_target);

	if (usemana)
		SubtractMana(pc_attacker, 4);
	
	doStaticEffect(trg, 8);
	soundeffect2(trg, 0x01, 0xE6);
	if(CheckResist(attacker, trg, 1)) return;
	tempeffect(attacker, trg, 5, pc_attacker->skill[MAGERY]/100, 0, 0);
	return;
}

///////////////////
// Name:	HarmSpellItem
// History:	Unknown
// Purpose:	Used for ITEMS; cast a harm spell.
//
void cMagic::HarmSpell(CHARACTER attacker, CHARACTER defender, bool usemana)
{
	P_CHAR pc_attacker = MAKE_CHARREF_LR(attacker);
	P_CHAR pc_defender = MAKE_CHARREF_LR(defender);
	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	CHARACTER trg = DEREF_P_CHAR(pc_target);

	if (usemana)
		SubtractMana(pc_attacker, 6);

	doStaticEffect(trg, 12);
	soundeffect2(trg, 0x01, 0xF1);
	if (CheckResist(attacker, trg, 2))
	{
		MagicDamage(defender, pc_attacker->skill[MAGERY]/500+1);
	}
	else
	{
		MagicDamage(defender, pc_attacker->skill[MAGERY]/250+RandomNum(1,2));
	}
	return;
}


///////////////////
// Name:	FireballSpellItem
// History:	Unknown
// Purpose:	Used for ITEMS; cast a fireball spell.
//
void cMagic::FireballSpell(CHARACTER attacker, CHARACTER defender, bool usemana)
{
	P_CHAR pc_attacker = MAKE_CHARREF_LR(attacker);
	P_CHAR pc_defender = MAKE_CHARREF_LR(defender);
	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	CHARACTER trg = DEREF_P_CHAR(pc_target);

	if (usemana)
		SubtractMana(pc_attacker, 9);

	doMoveEffect(attacker, trg, 18);
	soundeffect2(attacker, 0x1, 0x5E);
	if (CheckResist(attacker, trg, 3)) 
		MagicDamage(pc_target, pc_attacker->skill[MAGERY]/280+1);
	else 
		MagicDamage(pc_target, (pc_attacker->skill[MAGERY]+1*(pc_attacker->skill[EVALUATINGINTEL]/3))/(139+1*(pc_target->skill[MAGICRESISTANCE]/30))+RandomNum(1,4));
	return;
}

///////////////////
// Name:	CurseSpellItem
// History:	Unknown
// Purpose:	Used for ITEMS; cast a curse spell.
//
void cMagic::CurseSpell(CHARACTER attacker, CHARACTER defender, bool usemana)
{
	int j;

	P_CHAR pc_attacker = MAKE_CHARREF_LR(attacker);
	P_CHAR pc_defender = MAKE_CHARREF_LR(defender);
	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	CHARACTER trg = DEREF_P_CHAR(pc_target);

	if (usemana)
		SubtractMana(pc_attacker, 11);

	doStaticEffect(trg, 27);
	soundeffect2(trg, 0x01, 0xE1);
	if(CheckResist(attacker, trg, 1)) return;
	j = pc_attacker->skill[MAGERY]/100;
	tempeffect(attacker, trg, 12, j, j, j);
	return;
}

///////////////////
// Name:	LightningSpellItem
// History:	Unknown
// Purpose:	Used for ITEMS; cast a lightning spell.
//
void cMagic::LightningSpellItem(CHARACTER attaker, CHARACTER defender)
{
	if (CheckMagicReflect(defender))
	{//AntiChrist
			int t=defender;
			defender=attaker;
			attaker=t;
	}
	bolteffect(defender, true);
	soundeffect2(defender, 0x00, 0x29);
	if (CheckResist(attaker, defender, 4))
	{
		MagicDamage(defender, chars[attaker].skill[MAGERY]/180+RandomNum(1,2));
	}
	else
	{
		MagicDamage(defender, chars[attaker].skill[MAGERY]/90+RandomNum(1,5));
	}
	return;
}



///////////////////
// Name:	MindBlastSpellItem
// History:	Unknown
// Purpose:	Used for ITEMS; cast a mindblast spell.
//
void cMagic::MindBlastSpell(CHARACTER attacker, CHARACTER defender, bool usemana)
{
	P_CHAR pc_attacker = MAKE_CHARREF_LR(attacker);
	P_CHAR pc_defender = MAKE_CHARREF_LR(defender);
	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	CHARACTER trg = DEREF_P_CHAR(pc_target);

	if (usemana)
		SubtractMana(pc_attacker, 14);

	doStaticEffect(trg, 37);
	soundeffect2(trg, 0x02, 0x13);
	if (pc_attacker->in>pc_target->in)
	{
		if (CheckResist(attacker, trg, 5))
		{
			MagicDamage(pc_target, (pc_attacker->in-pc_target->in)/4);
		}
		else
		{
			MagicDamage(pc_target, (pc_attacker->in-pc_target->in)/2);
		}
	}
	else
	{
		if (CheckResist(defender, trg, 5))
		{
			MagicDamage(pc_attacker, (pc_target->in-pc_attacker->in)/4);
		}
		else
		{
			MagicDamage(pc_attacker, (pc_target->in-pc_attacker->in)/2);
		}
	}
	return;
}


///////////////////
// Name:	ParalyzeSpellItem
// History:	Unknown
// Purpose:	Used for ITEMS; cast a paralyze spell.
//
void cMagic::ParalyzeSpell(CHARACTER attacker, CHARACTER defender, bool usemana)
{
	P_CHAR pc_attacker = MAKE_CHARREF_LR(attacker);
	P_CHAR pc_defender = MAKE_CHARREF_LR(defender);
	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	CHARACTER trg = DEREF_P_CHAR(pc_target);

	if (usemana)
		SubtractMana(pc_attacker, 14);

	if (CheckResist(attacker, trg, 5)) return;
	doStaticEffect(trg, 38);
	soundeffect2(trg, 0x02, 0x04);
	tempeffect(attacker, trg, 1, 0, 0, 0);
	return;
}


///////////////////
// Name:	EsplosionSpellItem
// History:	Modified by Correa to be used by both Items and Chars
// Purpose:	Used for ITEMS; cast a explosion spell.
//
void cMagic::ExplosionSpell(CHARACTER attacker, CHARACTER defender, bool usemana)
{
	P_CHAR pc_attacker = MAKE_CHARREF_LR(attacker);
	P_CHAR pc_defender = MAKE_CHARREF_LR(defender);
	P_CHAR pc_target = CheckMagicReflect(pc_attacker, pc_defender);
	
	CHARACTER trg = DEREF_P_CHAR(pc_target);

	if (usemana)
		SubtractMana(pc_attacker, 20);

	
	doStaticEffect(trg, 43);
	soundeffect2(trg, 0x02, 0x07);
	if (CheckResist(attacker, trg, 6))
	{
		MagicDamage(pc_target, pc_attacker->skill[MAGERY]/120+RandomNum(1,5));
	}
	else
	{
		MagicDamage(pc_target, pc_attacker->skill[MAGERY]/40+RandomNum(1,10));
	}
	return;
}

///////////////////
// Name:	FlameStrikeSpellItem
// History:	Unknown
// Purpose:	Used for ITEMS; cast a flamestrike spell.
//
void cMagic::FlameStrikeSpellItem(CHARACTER attaker, CHARACTER defender)
{
	if (CheckMagicReflect(defender))
	{//AntiChrist
			int t=defender;
			defender=attaker;
			attaker=t;
	}
	doStaticEffect(defender, 51);
	soundeffect2(defender, 0x02, 0x08);

	MagicDamage(defender, chars[attaker].skill[MAGERY]/40+RandomNum(1,25));
	return;
}


/////////////////////////////////////////////////////////////////
/////// PCs CASTING SPELLS RELATED FUNCTIONS ////////////////////
/////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
// Name:	newSelectSpell2Cast                         //
// History:	Abaddon, 28 August 1999                     //
//			AntiChrist 10 September 1999                //
//			Frazurbluu July 2001 -cleaning this mess up-//
// Purpose:	Execute the selected spell to cast.         //
//////////////////////////////////////////////////////////
bool cMagic::newSelectSpell2Cast( UOXSOCKET s, int num)
{
	/*
	char mantra[25];
	char sect[512];
	char strToSay[100];
	int action = 0,
	*/
	// Much reordering follows -Fraz-
	int loskill, hiskill;
	int type = currentSpellType[s];
	P_CHAR pc_currchar = currchar[s];
//	int cc=currchar[s];
	pc_currchar->spell=num;
	int curSpell = pc_currchar->spell;

	if ((pc_currchar->cell !=0) && (!pc_currchar->isGM()))
	{
		sysmessage(s,"You are in jail and cannot cast spells");
		pc_currchar->spell = 0;
		pc_currchar->casting = 0;
		return false;
	}
	if (!townTarget( curSpell ) && !pc_currchar->isGM())
	{
		if (region[pc_currchar->region].priv&0x80)
		{
			//if (region[pc_currchar->region].guardowner)
			if (pc_currchar->inGuardedArea())
			{
			sysmessage(s,"you can't cast spells here");
			pc_currchar->spell = 0;
			pc_currchar->casting = 0;
			return false;
			}
		}
	}
	if( spells[num].enabled != 1 )
	{
		sysmessage( s, "That spell is currently not enabled" );
		pc_currchar->spell = 0;
		pc_currchar->casting = 0;
		return false;
	}

	// The following loop checks to see if any item is currently equipped (if not a GM)
	if (!pc_currchar->isGM())
	{
		unsigned int ci=0;
		P_ITEM pj;
		vector<SERIAL> vecContainer = contsp.getData(pc_currchar->serial);
		for ( ci = 0; ci < vecContainer.size(); ci++)
		{
			pj = FindItemBySerial(vecContainer[ci]);
			if (type!=2 && (pj->layer==2||(pj->layer==1 && pj->type!=9 )))
			{
				if (!(pj->id()==0x13F9 || pj->id()==0x0E8A || pj->id()==0x0DF0 || pj->id()==0x0DF2
					|| IsChaosOrOrderShield(pj->id()) ))
				{
					sysmessage(s,"You cannot cast with a weapon equipped.");
					pc_currchar->spell = 0;
					pc_currchar->casting = 0;
					return false;
				}
			}
		}
	}

	pc_currchar->unhide();
	pc_currchar->disturbMed(s); // Meditation

	//Check for enough reagents
	if (type==0 && (!CheckReagents(DEREF_P_CHAR(pc_currchar), spells[num].reagents)))
	{
		pc_currchar->spell = 0;
		pc_currchar->casting = 0;
		return false;
	}
	if (type != 2)// -Fraz-
	{
		if ((pc_currchar->mn < spells[num].mana) && !(pc_currchar->priv2&0x10)) // was 0x01, thats allmove !!!
		{
			//success=0;
			sysmessage(s, "You have insufficient mana to cast that spell.");
			pc_currchar->spell = 0;
			pc_currchar->casting = 0;
			return false;
		}
	}
	//Only speak if not a rod/staff/wand -Fraz-
	if (type!=2)
	{
		if (spells[num].runic || pc_currchar->skill[MAGERY]/10 > 99 )	// Add runic talk to gm mages (Blackwind)
			npctalkall_runic(DEREF_P_CHAR(pc_currchar),strupr(spells[num].mantra),0);
		else
			npctalkall(DEREF_P_CHAR(pc_currchar), spells[num].mantra,0);
		impaction(s, spells[num].action);
	}
	//spell section for implementation specific stuff to get the spell to work
	//AntiChrist - 26/10/99
	//LET'S USE SCRIPT-CONFIGURABLE DELAYS!!!
	pc_currchar->casting=1;
	if(pc_currchar->spell!=999) pc_currchar->spell=num;
	pc_currchar->spellaction=spells[num].action;
	pc_currchar->nextact=75;
	if (type==0 && (!(pc_currchar->isGM())))//if they are a gm they don't have a delay :-)
	{
		pc_currchar->spelltime=((spells[num].delay/10)*MY_CLOCKS_PER_SEC)+uiCurrentTime;
		pc_currchar->priv2 |= 2;//freeze
	} 
	else
	{
		pc_currchar->spelltime=0;
	}
	if (type==1)
	{
		loskill=spells[curSpell].sclo;
		hiskill=spells[curSpell].schi;
	}
	if (!SrvParms->cutscrollreq)
	{
		if (type==1 && !(pc_currchar->isGM()) && !Skills->CheckSkill(DEREF_P_CHAR(pc_currchar), MAGERY, loskill, hiskill))
		{
				SpellFail(s);
				pc_currchar->spell = 0;
				pc_currchar->casting = 0;
				return false;
		}
	}
	if (type != 2) // if it's not a wand -Fraz-
	{
		impaction(s, spells[num].action);//do the action
		cMagic::preParticles(num,DEREF_P_CHAR(pc_currchar)); // show the beautiful casting particles for UO:3D clients
		if (type==1)
		{
			return true;
		}
	}
	return false;
}

void cMagic::preParticles(int num, CHARACTER c)
{
    stat_st t;
	t = cMagic::getStatEffects_pre(num);
	if( t.effect[4] != -1 && t.effect[5] != -1 && t.effect[6] != -1 && t.effect[7] != -1 )
	staticeffect(c, NOTUSED, NOTUSED, NOTUSED, NOTUSED, true, &t, true); 			 			  		
}

void cMagic::afterParticles(int num, CHARACTER c)
{
    stat_st t;
	t = cMagic::getStatEffects_after(num);
	if( t.effect[4] != -1 && t.effect[5] != -1 && t.effect[6] != -1 && t.effect[7] != -1 )
	staticeffect(c, NOTUSED, NOTUSED, NOTUSED, NOTUSED, true, &t, true); 
}

void cMagic::itemParticles(int num, P_ITEM pi)
{
    stat_st t;
	t = cMagic::getStatEffects_item(num);
	if( t.effect[4] != -1 && t.effect[5] != -1 && t.effect[6] != -1 && t.effect[7] != -1 )
	staticeffect2(pi, NOTUSED, NOTUSED, NOTUSED, NOTUSED, NOTUSED, true, &t, true); 
}

void cMagic::NewCastSpell( UOXSOCKET s )
{
	// for LocationTarget spell like ArchCure, ArchProtection etc...
	int	j, x1, x2, y1, y2, z1, z2, range, distx, disty;
	unsigned int ii;
	int	StartGrid;
	int	getcell;
	int dmg, dmgmod;
	
	unsigned int increment;
	unsigned int checkgrid;
	
	int loskill, hiskill;//AntiChrist moved here
	P_CHAR pc_currchar = currchar[s];
	int curSpell = pc_currchar->spell;
	short xo,yo;
	signed char zo;
	bool b;
	
	int calcreg;
	int defender;
	unsigned long loopexit=0;
	int a;
	double d;
	bool recalled;
	int n;
	int x, y, z, dx, dy, dz;
	int fx[5], fy[5]; // bugfix LB, was fx[4] ...
	unsigned char id1, id2;	
	int snr;
	bool char_selected, item_selected, terrain_selected;
	
	if (pc_currchar->dead==1) return;
	if (currentSpellType[s]==0)
	{
		loskill=spells[curSpell].loskill;
		hiskill=spells[curSpell].hiskill;
		if (!(pc_currchar->isGM()) && !Skills->CheckSkill(DEREF_P_CHAR(pc_currchar), MAGERY, loskill, hiskill))
		{
			SpellFail(s);
			pc_currchar->spell = 0;
			pc_currchar->casting = 0;
			return;
		}
	}
	if (currentSpellType[s]!=2) SubtractMana(pc_currchar, spells[curSpell].mana);
	
	if (currentSpellType[s]==0) DelReagents(DEREF_P_CHAR(pc_currchar), spells[curSpell].reagents );
	
	if( requireTarget( curSpell ) )					// target spells if true
	{
		if( travelSpell( curSpell )	)				// travel spells.... mark, recall and gate
		{
			// mark, recall and gate go here
			P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
			if(pi)
			{
				if( !pi->isInWorld() || line_of_sight( s, pc_currchar->pos, pi->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING ) || pc_currchar->isGM() ) // bugfix LB
				{
					if ((pi->type==50))
					{
						playSound( DEREF_P_CHAR(pc_currchar), curSpell );
						//doMoveEffect( curSpell, DEREF_P_ITEM(pi), DEREF_P_CHAR(pc_currchar) );
						doStaticEffect( DEREF_P_CHAR(pc_currchar), curSpell );
						switch( curSpell )
						{
							//////////// (32) RECALL ////////////////
						case 32:
							if ( pi->morex<=200 && pi->morey<=200 )
							{
								sysmessage(s,"That rune has not been marked yet!");
								recalled = false;
							}
							else if (!pc_currchar->isGM() && Weight->CheckWeight2(DEREF_P_CHAR(pc_currchar))) //Morrolan no recall if too heavy, GM's excempt
							{
								sysmessage(s, "You are too heavy to do that!");
								sysmessage(s, "You feel drained from the attempt.");
								statwindow(s, DEREF_P_CHAR(pc_currchar));
								recalled=false;
							}
							else
							{										
								xo=pc_currchar->pos.x;
								yo=pc_currchar->pos.y;
								zo=pc_currchar->pos.z;											                                  
								
								cMagic::invisibleItemParticles(DEREF_P_CHAR(pc_currchar), curSpell, xo, yo, zo);
								
								pc_currchar->MoveTo(pi->morex,pi->morey,pi->morez); //LB
								teleport(DEREF_P_CHAR(pc_currchar));
								doStaticEffect( DEREF_P_CHAR(pc_currchar), curSpell );
								sysmessage(s,"You have recalled from the rune.");
								recalled=true;
							}
							break;
							//////////// (45) MARK //////////////////
						case 45:
							pi->morex=pc_currchar->pos.x;
							pi->morey=pc_currchar->pos.y;
							pi->morez=pc_currchar->pos.z;
							sysmessage(s,"Recall rune marked.");
							//antichrist
							calcreg=calcRegionFromXY(pc_currchar->pos.x, pc_currchar->pos.y);
							sprintf(pi->name, "Rune to: %s.", region[calcreg].name);
							
							cMagic::invisibleItemParticles(DEREF_P_CHAR(pc_currchar), curSpell, pc_currchar->pos.x, pc_currchar->pos.y, pc_currchar->pos.z);						
							
							break;
							//////////// (52) GATE //////////////////
						case 52:
							if ( pi->morex<=200 && pi->morey<=200 )
							{
								sysmessage(s,"That rune has not been marked yet!");
								recalled=false;
							}
							else
							{
								gatex[gatecount][0]=pc_currchar->pos.x;	//create gate a player location
								gatey[gatecount][0]=pc_currchar->pos.y;
								gatez[gatecount][0]=pc_currchar->pos.z;
								gatex[gatecount][1]=pi->morex; //create gate at marked location
								gatey[gatecount][1]=pi->morey;
								gatez[gatecount][1]=pi->morez;
								recalled=true;
								
								for (n=0;n<2;n++)
								{
									strcpy((char*)temp,"a blue moongate");
									P_ITEM pi_c = Items->SpawnItem(-1,s,1,"#",0,0x0f,0x6c,0,0,0,0);
									if(pi_c != NULL)	//AntiChrist - to prevent crashes
									{
										pi_c->type=51+n;
										pi_c->pos.x=gatex[gatecount][n];
										pi_c->pos.y=gatey[gatecount][n];
										pi_c->pos.z=gatez[gatecount][n];
										pi_c->gatetime=(uiCurrentTime+(SrvParms->gatetimer*MY_CLOCKS_PER_SEC));
										//clConsole.send("GATETIME:%i UICURRENTTIME:%d GETCLOCK:%d\n",SrvParms->gatetimer,uiCurrentTime,getclock());
										pi_c->gatenumber=gatecount;
										pi_c->dir=1;
										
										mapRegions->Add(pi_c);	//add gate to list of items in the region
										RefreshItem(pi_c);//AntiChrist
									}
									if (n==1)
									{
										gatecount++;
										if (gatecount>MAXGATES) gatecount=0;
									}
									addid1[s]=0;
									addid2[s]=0;
								}
							}
							break;
						default:
							clConsole.send("MAGIC-ERROR: Unknown Travel spell %i, magic.cpp\n", curSpell );
							break;
						}
					}
					else
						sysmessage( s, "That item is not a recall rune." );
				}
			}
			else
				sysmessage( s, "Not a valid target on item!" );
			pc_currchar->spell = 0;
			return;
		}
		
		/////////////////////////////////////////////////////////
		// spell code goes starts for targetted spells
		/////////////////////////////////////////////////////////
		
		if( reqCharTarget( curSpell ) )
		{
			// TARGET CALC HERE
			defender=LongFromCharPtr(buffer[s]+7);	// character we are attacking is HERE
			P_CHAR pc_defender = FindCharBySerial( defender );
			// IF TARGET VALID
			if (pc_defender != NULL)				// we have to have targetted a person to kill them :)
			{
				if(true == Races[pc_defender->race]->CheckSpellImune(curSpell))
				{
					sysmessage(s,"He seems unaffected by your spell!");
					return;
				}
				if( chardist( DEREF_P_CHAR(pc_defender), DEREF_P_CHAR(pc_currchar) ) > SrvParms->attack_distance )
				{
					sysmessage( s, "You can't cast on someone that far away!" );
					return;
				}
				if ((line_of_sight( s, pc_currchar->pos, pc_defender->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
					(pc_currchar->isGM())))
				{
					if( aggressiveSpell( curSpell ) )
					{
						if ((pc_defender->isInnocent())&&(DEREF_P_CHAR(pc_defender) != DEREF_P_CHAR(pc_currchar))&& !pc_currchar->Owns(pc_defender)&&(!Guilds->Compare(DEREF_P_CHAR(pc_defender),DEREF_P_CHAR(pc_currchar)))&&(!Races.CheckRelation(pc_currchar,pc_defender)) )
						{
							criminal(DEREF_P_CHAR(pc_currchar));
						}
						if (pc_defender->npcaitype==17) // Ripper 11-14-99
						{
							sysmessage(s," They are Invulnerable merchants!");
							return;
						}
						npcattacktarget(DEREF_P_CHAR(pc_defender), DEREF_P_CHAR(pc_currchar));
					}
					if( spellReflectable( curSpell ) )
					{
						//clConsole.send("spell-reflectable\n");
						pc_defender = CheckMagicReflect(pc_currchar, pc_defender);
					}
					
					playSound( DEREF_P_CHAR(pc_currchar), curSpell );
					doMoveEffect( curSpell, DEREF_P_CHAR(pc_defender), DEREF_P_CHAR(pc_currchar) );
					if (curSpell !=30 && curSpell !=58) doStaticEffect( DEREF_P_CHAR(pc_defender), curSpell ); // Lb, bugfix, was (DEREF_P_CHAR(pc_currchar) ...)																									
					
					switch( curSpell )
					{
						//////////// (1) CLUMSY /////////////////
					case 1:
						tempeffect( DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 3, pc_currchar->skill[MAGERY]/100, 0, 0);
						break;
						//////////// (3) FEEBLEMIND /////////////
					case 3:
						tempeffect( DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 4, pc_currchar->skill[MAGERY]/100, 0, 0);
						break;
						//////////// (4) HEAL ///////////////////
					case 4:
						int m,h;
						m=pc_currchar->skill[MAGERY];
						if ((pc_currchar->isInnocent()) &&(pc_currchar->serial != pc_defender->serial))
						{
							if ((pc_defender->crimflag>0) ||(pc_defender->isMurderer()))
							{
								criminal(DEREF_P_CHAR(pc_currchar));
							}
						}
						
						if (m<100) h=1; else
							if (m>=100 && m<200) h=2; else
								if (m>=200 && m<300) h=4; else
									if (m>=300 && m<400) h=5; else
										if (m>=400) h=6;
										if (h>2) h+=rand()%3;
										
										pc_defender->hp=pc_defender->hp+h;
										updatestats((pc_defender), 0);
										
										break;
										//////////// (5) MAGIC ARROW ////////////
					case 5:
						MagicDamage(DEREF_P_CHAR(pc_defender), (1+(rand()%1)+1)*(pc_currchar->skill[MAGERY]/2000+1));
						break;
						//////////// (6) NIGHT SIGHT ////////////
					case 6:
						tempeffect( DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 2, 0, 0, 0);
						break;
						//////////// (7) REACTIVE ARMOR /////////
					case 7:
						if (Skills->GetAntiMagicalArmorDefence(DEREF_P_CHAR(pc_currchar))>10)
						{
							sysmessage(s,"Spell fails due to the armor on target!");
							break;
						}
						tempeffect( DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 15, pc_currchar->skill[MAGERY]/100, 0, 0 );
						pc_defender->ra=1;
						break;
						//////////// (8) WEAKEN /////////////////
					case 8:
						tempeffect( DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 5, pc_currchar->skill[MAGERY]/100, 0, 0);
						break;
						//////////// (9) AGILITY ////////////////
					case 9:
						tempeffect( DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 6, pc_currchar->skill[MAGERY]/100, 0, 0);
						break;
						//////////// (10) CUNNING ///////////////
					case 10:
						tempeffect( DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 7, pc_currchar->skill[MAGERY]/100, 0, 0);
						break;
						//////////// (11) CURE //////////////////
					case 11:
						pc_defender->poisoned=0;
						pc_defender->poisonwearofftime=uiCurrentTime;
						impowncreate(s,DEREF_P_CHAR(pc_defender),1); // updating to blue bar
						break;
						//////////// (12) HARM //////////////////
					case 12:
						if (CheckResist(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 2))
							MagicDamage(DEREF_P_CHAR(pc_defender), pc_currchar->skill[MAGERY]/500+1);
						else
							MagicDamage(DEREF_P_CHAR(pc_defender), (pc_currchar->skill[MAGERY]+1*(pc_currchar->skill[EVALUATINGINTEL]/3))/(249+1*(pc_defender->skill[MAGICRESISTANCE]/30))+RandomNum(1,2));
						break;
						//////////// (15) PROTECTION ////////////
					case 15:
						if (Skills->GetAntiMagicalArmorDefence(DEREF_P_CHAR(pc_currchar))>10)
						{
							sysmessage(s,"Spell fails due to the armor on target!");
							break;
						}
						tempeffect( DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 21, pc_currchar->skill[MAGERY]/100, 0, 0 );
						break;
						//////////// (16) STRENGTH //////////////
					case 16:
						tempeffect( DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 8, pc_currchar->skill[MAGERY]/100, 0, 0);
						break;
						//////////// (17) BLESS /////////////////
					case 17:
						j=pc_currchar->skill[MAGERY]/100;
						tempeffect( DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 11, j, j, j);
						break;
						//////////// (18) FIREBALL //////////////
					case 18:
						if (CheckResist(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 3))
							MagicDamage(DEREF_P_CHAR(pc_defender), pc_currchar->skill[MAGERY]/280+1);
						else
							MagicDamage(DEREF_P_CHAR(pc_defender), (pc_currchar->skill[MAGERY]+1*(pc_currchar->skill[EVALUATINGINTEL]/3))/(139+1*(pc_defender->skill[MAGICRESISTANCE]/30))+RandomNum(1,4));
						break;
						//////////// (20) POISON ////////////////
					case 20:
						if(CheckResist(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 1)) return;
						{
							pc_defender->poisoned=2;
							pc_defender->poisonwearofftime=uiCurrentTime+(MY_CLOCKS_PER_SEC*SrvParms->poisontimer); // LB
							impowncreate( s, DEREF_P_CHAR(pc_defender), 1); //Lb, sends the green bar !
						}
						break;
						//////////// (27) CURSE /////////////////
					case 27:
						if(!CheckResist(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 1))
						{
							j=pc_currchar->skill[MAGERY]/100;
							tempeffect(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 12, j, j, j);
						}
						break;
						//////////// (29) GREATER HEAL //////////
					case 29:
						if ((pc_currchar->isInnocent()) &&(pc_currchar->serial != pc_defender->serial))
						{
							if ((pc_defender->crimflag>0) ||(pc_defender->isMurderer()))
							{
								criminal(DEREF_P_CHAR(pc_currchar));
							}
						}
						j=pc_defender->hp+(pc_currchar->skill[MAGERY]/30+RandomNum(1,12));
						pc_defender->hp=min(pc_defender->st, j);
						updatestats((pc_defender), 0);
						break;
						//////////// (30) LIGHTNING /////////////
					case 30:
						bolteffect(DEREF_P_CHAR(pc_defender), true);
						if (CheckResist(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 4))
							MagicDamage(DEREF_P_CHAR(pc_defender), pc_currchar->skill[MAGERY]/180+RandomNum(1,2));
						else
							MagicDamage(DEREF_P_CHAR(pc_defender), (pc_currchar->skill[MAGERY]+1*(pc_currchar->skill[EVALUATINGINTEL]/3))/(89+1*(pc_defender->skill[MAGICRESISTANCE]/30))+RandomNum(1,5));
						break;
						//////////// (31) MANA DRAIN ////////////
					case 31:
						if(!CheckResist(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 4))
						{
							pc_defender->mn-=pc_currchar->skill[MAGERY]/35;
							if (pc_defender->mn<0) pc_defender->mn=0;
							updatestats((pc_defender), 1);
						}
						break;
						//////////// (37) MIND BLAST ////////////
					case 37:
						if (pc_currchar->in>pc_defender->in)
						{
							if (CheckResist(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 5))
								MagicDamage(DEREF_P_CHAR(pc_defender), (pc_currchar->in-pc_defender->in)/4);
							else
								MagicDamage(DEREF_P_CHAR(pc_defender), (pc_currchar->in-pc_defender->in)/2);
						}
						else
						{
							if (CheckResist(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_currchar), 5))
								MagicDamage(DEREF_P_CHAR(pc_currchar), (pc_defender->in-pc_currchar->in)/4);
							else
								MagicDamage(DEREF_P_CHAR(pc_currchar), (pc_defender->in-pc_currchar->in)/2);
						}
						cMagic::afterParticles(37, DEREF_P_CHAR(pc_currchar));
						break;
						//////////// (38) PARALYZE //////////////
					case 38:
						if (!CheckResist(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 7))
							tempeffect(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 1, 0, 0, 0);
						break;
						//////////// (41) DISPEL ////////////////
					case 41:
						if (pc_defender->priv2&0x20)
						{
							//staticeffect(DEREF_P_CHAR(pc_defender), 0x37, 0x2A, 0x09, 0x06);
							tileeffect(pc_defender->pos.x,pc_defender->pos.y,pc_defender->pos.z, 0x37, 0x2A, 0x00, 0x00);
							if (pc_defender->isNpc()) Npcs->DeleteChar(DEREF_P_CHAR(pc_defender));
							else deathstuff(DEREF_P_CHAR(pc_defender));
						}
						break;
						//////////// (42) ENERGY BOLT ///////////
					case 42:
						if (CheckResist(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 6))
							MagicDamage(DEREF_P_CHAR(pc_defender), pc_currchar->skill[MAGERY]/120);
						else
							MagicDamage(DEREF_P_CHAR(pc_defender), (pc_currchar->skill[MAGERY]+1*(pc_currchar->skill[EVALUATINGINTEL]/3))/(34+1*(pc_defender->skill[MAGICRESISTANCE]/30))+RandomNum(1,10));						
						
						break;
						//////////// (43) EXPLOSION /////////////
					case 43:
						if (CheckResist(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 6))
							MagicDamage(DEREF_P_CHAR(pc_defender), pc_currchar->skill[MAGERY]/120+RandomNum(1,5));
						else
							MagicDamage(DEREF_P_CHAR(pc_defender), (pc_currchar->skill[MAGERY]+1*(pc_currchar->skill[EVALUATINGINTEL]/3))/(39+1*(pc_defender->skill[MAGICRESISTANCE]/30))+RandomNum(1,10));
						break;
						//////////// (44) INVISIBILITY //////////
					case 44:						
						
						cMagic::invisibleItemParticles(DEREF_P_CHAR(pc_defender), curSpell, pc_defender->pos.x, pc_defender->pos.y, pc_defender->pos.z+12);
						
						pc_defender->hidden=2;
						updatechar(DEREF_P_CHAR(pc_defender));
						pc_defender->invistimeout=uiCurrentTime+(SrvParms->invisibiliytimer*MY_CLOCKS_PER_SEC);
						break;
						//////////// (51) FLAMESTRIKE ///////////
					case 51:
						if (CheckResist(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 7))
							MagicDamage(DEREF_P_CHAR(pc_defender), pc_currchar->skill[MAGERY]/80);
						else
							MagicDamage(DEREF_P_CHAR(pc_defender), (pc_currchar->skill[MAGERY]+1*(pc_currchar->skill[EVALUATINGINTEL]/3))/(34+1*(pc_defender->skill[MAGICRESISTANCE]/30))+RandomNum(1,25));
						break;
						//////////// (53) MANA VAMPIRE //////////
					case 53:
						if(!CheckResist(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_defender), 7))
						{
							if (pc_defender->mn<40)
							{
								pc_currchar->mn += pc_defender->mn;
								pc_defender->mn=0;
							}
							else
							{
								pc_defender->mn-=40;
								pc_currchar->mn+=40;
							}
							updatestats((pc_defender), 1);
							updatestats((pc_currchar), 1);
						}
						break;
						//////////// (59) RESURRECTION //////////
					case 59:
						if (pc_defender->dead && online(DEREF_P_CHAR(pc_defender)))
						{
							cMagic::doStaticEffect(DEREF_P_CHAR(pc_defender), curSpell);
							b=Targ->NpcResurrectTarget(DEREF_P_CHAR(pc_defender));		
							cMagic::invisibleItemParticles(DEREF_P_CHAR(pc_defender), curSpell, pc_defender->pos.x, pc_defender->pos.y, pc_defender->pos.z);
							return;
						}
						else if (!pc_defender->dead) sysmessage(s,"That player isn't dead!");
						else sysmessage(s,"That player isn't online!");
						break;
						//////////// (66) CANNON FIRING /////////
					case 66:
						if (CheckParry(DEREF_P_CHAR(pc_defender), 6))
							MagicDamage(DEREF_P_CHAR(pc_defender), pc_currchar->skill[TACTICS]/50);
						else
							MagicDamage(DEREF_P_CHAR(pc_defender), pc_currchar->skill[TACTICS]/25);
						break;
					default:
						clConsole.send("MAGIC-ERROR: Unknown CharacterTarget spell %i, magic.cpp\n", curSpell );
						break;
					}
				}
				else
				{
					sysmessage(s,"There seems to be something in the way.");
				}
			}
			else
				sysmessage(s,"That is not a person.");
		}
		else if( reqLocTarget( curSpell ) )
		{
			// field spells mostly go here
			x=fx[0]=(buffer[s][11]<<8)+buffer[s][12];
			y=fy[0]=(buffer[s][13]<<8)+buffer[s][14];
			//z=buffer[s][16];
			z=buffer[s][16]+Map->TileHeight((buffer[s][17]<<8)+buffer[s][18]); // bugfix, LB
			
			defender=LongFromCharPtr(buffer[s]+7);
			P_CHAR pc_i = FindCharBySerial( defender );
			P_ITEM pi_l = FindItemBySerial( defender );
			
			// ..[1] 0-> object or item selected, 1-> terrain selected
			char_selected=item_selected=terrain_selected=false;
			if      (pc_i != NULL && buffer[s][1]==0)      char_selected=true;
            else if (pi_l != NULL && buffer[s][1]==0)      item_selected=true;
			else						                   terrain_selected=true;
			
			//AntiChrist - location check
			if (!(buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF))
			{
				Coord_cl clTemp1(x,y,z);
				
				if ((line_of_sight( s, pc_currchar->pos, clTemp1,WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
					(pc_currchar->isGM())))
				{
					if( fieldSpell( curSpell ) )
					{
						//AntiChrist - fixed the fields missing middle piece - 9/99
						//(changed the order of fy and fy also!)
						j=fielddir(DEREF_P_CHAR(pc_currchar), x, y, z); // lb bugfix, socket char# confusion
						if (j)
						{
							fx[0]=fx[1]=fx[2]=fx[3]=fx[4]=x;
							fy[0]=y;
							fy[1]=y+1;
							fy[2]=y-1;
							fy[3]=y+2;
							fy[4]=y-2;
						}
						else
						{
							fy[0]=fy[1]=fy[2]=fy[3]=fy[4]=y;
							fx[0]=x;
							fx[1]=x+1;
							fx[2]=x-1;
							fx[3]=x+2;
							fx[4]=x-2;
						}	// end else
					}	// end if
					
					playSound( DEREF_P_CHAR(pc_currchar), curSpell );
					//doMoveEffect( curSpell, i, DEREF_P_CHAR(pc_currchar) );
					if (curSpell != 33 && curSpell !=58) doStaticEffect( DEREF_P_CHAR(pc_currchar), curSpell );
					
					cRegion::RegionIterator4Chars ri(pc_currchar->pos);
					Coord_cl clTemp2(x,y,z) ;
					
					switch( curSpell )
					{
						//////////// (22) TELEPORT //////////////////
					case 22:
						{
							
							if (char_selected) { sysmessage(s, "You can't use that spell on characters");  return; }
							if (item_selected) { sysmessage(s, "You can't teleport there.");  return; }
							
							P_ITEM pi_multi = findmulti( Coord_cl(x, y, z) );
							if( pi_multi == NULL )
							{
								sysmessage( s, "You can't teleport here!" );
								return;
							}
							tile_st tile;
							Map->SeekTile(((buffer[s][0x11]<<8)+buffer[s][0x12]), &tile);
							if( (!strcmp((char *)tile.name, "water")) || (tile.flag1&0x80) )
							{
								sysmessage(s,"Give up wanabe Jesus !");
								return;
							}
							
							mapRegions->Remove(pc_currchar); //LB
							
							xo=pc_currchar->pos.x;
							yo=pc_currchar->pos.y;
							zo=pc_currchar->pos.z;
							
							pc_currchar->MoveTo(x,y,z);
							teleport( DEREF_P_CHAR(pc_currchar) );
							doStaticEffect( DEREF_P_CHAR(pc_currchar), curSpell );
							cMagic::invisibleItemParticles(DEREF_P_CHAR(pc_currchar), curSpell, xo, yo, zo);
						}
						break;
						//////////// (24) WALL OF STONE /////////////
					case 24:
						
						if (char_selected) { sysmessage(s, "You can't use that spell on characters");  return; }
						id1 = 0x00;
						id2 = 0x80;
						snr = 24;
						break;
						//////////// (25) ARCH CURE /////////////////
					case 25:
						
						if (terrain_selected || item_selected) { sysmessage(s, "This spell can be used only on characters");  return; }
						
						x1=0; x2=0; y1=0; y2=0; z1=0; z2=0;
						
						BoxSpell(s, x1, x2, y1, y2, z1, z2);
						
						//Char mapRegions
						ri = pc_currchar->pos;
						for (ri.Begin(); ri.GetData() != ri.End(); ri++)
						{
							P_CHAR mapchar = ri.GetData();
							if (mapchar != NULL)
							{
								if ((online(DEREF_P_CHAR(mapchar)) || (mapchar->isNpc())) && (mapchar->pos.x>=x1&&mapchar->pos.x<=x2)&&
								(mapchar->pos.y>=y1&&mapchar->pos.y<=y2)/*&&
								(chars[ii].pos.z>=z1&&chars[ii].pos.z<=z2)*/)
								{
									if ((line_of_sight(s, pc_currchar->pos, mapchar->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
										(pc_currchar->isGM())))
									{
										cMagic::doStaticEffect(DEREF_P_CHAR(mapchar), 25);										
										// tempeffect(DEREF_P_CHAR(pc_currchar), ii, 2, 0, 0, 0); // lb bugfix ?? why does this cll night-sight effect
										soundeffect2(DEREF_P_CHAR(mapchar), 0x01, 0xE9);
										mapchar->poisoned=0;
									}
									else
									{
										sprintf((char*)temp,"There seems to be something in the way between you and %s.", mapchar->name);
										sysmessage(s, (char*)temp);
									}
								}
							}//if mapitem
						}
						break;
						////////// (26) ARCH PROTECTION ///////////////
					case 26:
						
						if (terrain_selected || item_selected) { sysmessage(s, "This spell can be used only on characters");  return; }
						
						x1 = x2 = y1 = y2 = z1 = z2 = 0;
						BoxSpell( s, x1, x2, y1, y2, z1, z2 );
						for (ri.Begin(); ri.GetData() != ri.End(); ri++)
						{
							P_CHAR mapchar = ri.GetData();
							if( mapchar != NULL )
							{
								if (( online( DEREF_P_CHAR(mapchar) ) || ( mapchar->isNpc() )) && ( mapchar->pos.x >= x1 && mapchar->pos.x <= x2 ) &&
								( mapchar->pos.y >= y1 && mapchar->pos.y <= y2 ) /*&&
								( chars[ii].pos.z >= z1 && chars[ii].pos.z <= z2 )*/)
								{
									if(( line_of_sight( s, pc_currchar->pos, mapchar->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
										( pc_currchar->isGM() )))
									{
										playSound( DEREF_P_CHAR(mapchar), curSpell );
										doStaticEffect( DEREF_P_CHAR(mapchar), 26 );	// protection
										tempeffect( DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(mapchar), 21, pc_currchar->skill[MAGERY]/100, 0, 0 );
									}
									else
									{
										sprintf( (char*)temp, "There seems to be something in the way between you and %s.", mapchar->name );
										sysmessage( s, (char*)temp );
									}
								}
							} // if mapitem
						}
						break;
						//////////// (28) FIRE FIELD ////////////////
					case 28:
						
						if (char_selected) { sysmessage(s, "This spell can't be used on characters");  return; }
						
						if (j)
						{
							id1=0x39;
							id2=0x96;
						}		// end if
						else
						{
							id1=0x39;
							id2=0x8C;
						}		// end else
						snr = 28;
						break;
						//////////// (33) BLADE SPIRITS /////////////
					case 33:
						if (char_selected) { sysmessage(s, "This spell can't be used on characters");  return; }
						SummonMonster( s, 0x02, 0x3E, "a blade spirit", 0x00, 0x00, x, y, z, curSpell );
						break;
						//////////// (34) DISPEL FIELD //////////////
					case 34:
						{
							if (char_selected) { sysmessage(s, "This spell can't be used on characters");  return; }							
							P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
							if (pi)
							{
								if ((line_of_sight(s, pc_currchar->pos, pi->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
									(pc_currchar->isGM())))
								{
									if (pi->priv&5 || pi->priv&4) Items->DeleItem( pi );
									soundeffect3(pi,0x0201);
								}
								else
								{
									sysmessage(s, "There seems to be something in the way.");
								}
							}
						}
						break;
						//////////// (39) POISON FIELD //////////////
					case 39:
						if (char_selected) { sysmessage(s, "This spell can't be used on characters");  return; }
						if (j)
						{
							id1=0x39;
							id2=0x20;
						}
						else
						{
							id1=0x39;
							id2=0x15;
						}
						snr = 39;
						break;
						//////////// (46) MASS CURSE ////////////////
					case 46:
						x1=0; x2=0; y1=0; y2=0; z1=0; z2=0;
						if (terrain_selected || item_selected) { sysmessage(s, "This spell can be used only on characters");  return; }
						
						BoxSpell(s, x1, x2, y1, y2, z1, z2);
						
						//Char mapRegions
						ri = pc_currchar->pos;
						for (ri.Begin(); ri.GetData() != ri.End(); ri++)
						{
							P_CHAR mapchar = ri.GetData();
							if (mapchar != NULL)
							{
								if ((online(DEREF_P_CHAR(mapchar))||(mapchar->isNpc())) && (mapchar->pos.x>=x1&&mapchar->pos.x<=x2)&&
								(mapchar->pos.y>=y1&&mapchar->pos.y<=y2)/*&&
								(chars[ii].pos.z>=z1&&chars[ii].pos.z<=z2)*/)
								{
									if ((line_of_sight(s, pc_currchar->pos, mapchar->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
										(pc_currchar->isGM())))
									{
										if (mapchar->npcaitype==17) // Ripper 11-14-99
										{
											sysmessage(s," They are Invulnerable merchants!");
											return;
										}
										
										if (mapchar->isNpc()) 
											npcattacktarget(DEREF_P_CHAR(mapchar), DEREF_P_CHAR(pc_currchar));
										doStaticEffect(DEREF_P_CHAR(mapchar), curSpell);
										soundeffect2(DEREF_P_CHAR(mapchar), 0x01, 0xFB);
										if(CheckResist(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(mapchar), 6)) j=pc_currchar->skill[MAGERY]/200;
										else j=pc_currchar->skill[MAGERY]/75;
										tempeffect(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(mapchar), 12, j, j, j);
									}
									else
									{
										sprintf((char*)temp,"Try as you might, but you cannot see %s well enough to cover.", mapchar->name);
										sysmessage(s, (char*)temp);
									}
								}
							}//if	mapitem
						}
						break;
						//////////// (47) PARALYZE FIELD ////////////
					case 47:
						if (char_selected) { sysmessage(s, "This spell can't be used on characters");  return; }
						if (j)
						{
							id1=0x39;
							id2=0x79;
						}
						else
						{
							id1=0x39;
							id2=0x67;
						}
						snr = 47;
						break;
						//////////// (48) REVEAL ////////////////////
					case 48:
						if (terrain_selected || item_selected) { sysmessage(s, "This spell can be used only on characters");  return; }
						
						if ((line_of_sight(s, pc_currchar->pos, clTemp2, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
							(pc_currchar->isGM())))
						{
							j=pc_currchar->skill[MAGERY];
							range=(((j-261)*(15))/739)+5;
							//If the caster has a Magery of 26.1 (min to cast reveal w/ scroll), range radius is
							//5 tiles, if magery is maxed out at 100.0 (except for gms I suppose), range is 20
							
							//Char mapRegions
							StartGrid=mapRegions->StartGrid(pc_currchar->pos.x,pc_currchar->pos.y);
							getcell=mapRegions->GetCell(pc_currchar->pos.x,pc_currchar->pos.y);
							
							increment=0;
							cRegion::RegionIterator4Chars ri(pc_currchar->pos);
							for (ri.Begin(); ri.GetData() != ri.End(); ri++)
							{
								P_CHAR mapchar = ri.GetData();
								if (mapchar != NULL)
								{
									if (mapchar->isHidden()) //does reveal hidden people as well now :)
									{
										dx=abs(mapchar->pos.x-x);
										dy=abs(mapchar->pos.y-y);
										dz=abs(mapchar->pos.z-z); // new--difference in z coords
										d=hypot(dx, dy);
										if ((d<=range)&&(dz<=15)) //char to reveal is within radius or range and no more than 15 z coords away
										{
											mapchar->unhide();
										}
									}
								}
							}
							soundeffect(s,0x01,0xFD);
						}
						else
						{
							sysmessage(s, "You would like to see if anything was there, but there is too much stuff in the way.");
						}
						break;
						
					case 49:///////// Chain Lightning ///////////
						///////// Forgotten, added by LB ////
						
						x1=0; x2=0; y1=0; y2=0; z1=0; z2=0;
						BoxSpell(s, x1, x2, y1, y2, z1, z2);
						
						if (terrain_selected || item_selected) { sysmessage(s, "This spell can be used only on characters");  return; }
						
						//Char mapRegions
						ri = pc_currchar->pos;
						for (ri.Begin(); ri.GetData() != ri.End(); ri++)
						{
							P_CHAR mapchar = ri.GetData();
							if (mapchar != NULL)
							{
								if ((online(DEREF_P_CHAR(mapchar))||(mapchar->isNpc())) && (mapchar->pos.x>=x1&&mapchar->pos.x<=x2)&&
								(mapchar->pos.y>=y1&&mapchar->pos.y<=y2)/*&&
								(chars[ii].pos.z>=z1&&chars[ii].pos.z<=z2)*/)
								{
									if ((line_of_sight(s, pc_currchar->pos, mapchar->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
										(pc_currchar->isGM())))
									{
										if (mapchar->npcaitype==17) // Ripper 11-14-99
										{
											sysmessage(s," They are Invulnerable merchants!");
											return;
										}
										if (mapchar->isNpc()) 
											npcattacktarget(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(mapchar));
										bolteffect(DEREF_P_CHAR(mapchar),true);
										soundeffect2(DEREF_P_CHAR(pc_currchar), 0x00, 0x29); //Homey fix for chainlightning sound
										
										int def, att, ii;
										if(CheckMagicReflect(DEREF_P_CHAR(mapchar)))//AntiChrist
										{
											def=DEREF_P_CHAR(pc_currchar);
											att = ii = DEREF_P_CHAR(mapchar);
										} else
										{
											def = ii = DEREF_P_CHAR(mapchar);
											att=DEREF_P_CHAR(pc_currchar);
										}
										if (CheckResist(att, def, 7))
										{
											MagicDamage(def, chars[att].skill[MAGERY]/70);
										}
										else
										{
											MagicDamage(def, (pc_currchar->skill[MAGERY]+1*(pc_currchar->skill[EVALUATINGINTEL]/3))/(49+1*(chars[ii].skill[MAGICRESISTANCE]/30)));
										}
									}
									else
									{
										sysmessage(s, "You cannot see the target well.");
									}
								}
							}//if Mapitem
						}
						break;
						
						//////////// (50) ENERGY FIELD //////////////
					case 50:
						if (char_selected) { sysmessage(s, "This spell can't be used on characters");  return; }
						if (j)
						{
							id1=0x39;
							id2=0x56;
						}
						else
						{
							id1=0x39;
							id2=0x46;
						}
						snr = 50;
						break;
						//////////// (54) MASS DISPEL ///////////////
					case 54:
						x1=0; x2=0; y1=0; y2=0; z1=0; z2=0;
						
						BoxSpell(s, x1, x2, y1, y2, z1, z2);
						
						//if (char_selected) { sysmessage(s, "This spell can't be used on characters");  return; }
						
						//Char mapRegions
						ri = pc_currchar->pos;
						for (ri.Begin(); ri.GetData() != ri.End(); ri++)
						{
							P_CHAR mapchar = ri.GetData();
							if (mapchar != NULL)
							{
								ii = DEREF_P_CHAR(mapchar);
								if ((online(DEREF_P_CHAR(mapchar))||(mapchar->isNpc())) && (mapchar->priv2&0x20)&&
									(mapchar->pos.x>=x1&&mapchar->pos.x<=x2)&&
									(mapchar->pos.y>=y1&&mapchar->pos.y<=y2)/*&&
									(mapchar->pos.z>=z1&&mapchar->pos.z<=z2)*/)
								{
									if ((line_of_sight(s, pc_currchar->pos, mapchar->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
										(pc_currchar->isGM())))
									{
										if(CheckResist(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(mapchar), 7) && rand()%2==0 ) // cant be 100% resisted , LB, osi
										{
											if (mapchar->isNpc()) npcattacktarget(DEREF_P_CHAR(mapchar), DEREF_P_CHAR(pc_currchar));
											return;
										}
										if (mapchar->isNpc()) deathstuff(DEREF_P_CHAR(mapchar)); // LB !!!!
										soundeffect2(DEREF_P_CHAR(mapchar), 0x02, 0x04);
										doStaticEffect(DEREF_P_CHAR(mapchar), curSpell);
									}
								}
							}//if mapitem
						}
						break;
						//////////// (55) METEOR SWARM //////////////
					case 55:
						
						if (terrain_selected || item_selected) { sysmessage(s, "This spell can be used only on characters");  return; }
						x1=0; x2=0; y1=0; y2=0; z1=0; z2=0;
						
						BoxSpell(s, x1, x2, y1, y2, z1, z2);
						
						//Char mapRegions
						ri = pc_currchar->pos;
						for (ri.Begin(); ri.GetData() != ri.End(); ri++)
						{
							P_CHAR mapchar = ri.GetData();
							if (mapchar != NULL)
							{
								ii = DEREF_P_CHAR(mapchar);
								if ((online(DEREF_P_CHAR(mapchar))||(mapchar->isNpc())) && (mapchar->pos.x>=x1&&mapchar->pos.x<=x2)&&
								(mapchar->pos.y>=y1&&mapchar->pos.y<=y2)/*&&
								(mapchar->pos.z>=z1&&mapchar->pos.z<=z2)*/)
								{
									if ((line_of_sight(s, pc_currchar->pos, mapchar->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
										(pc_currchar->isGM())))
									{
										if (mapchar->npcaitype==17) // Ripper 11-14-99
										{
											sysmessage(s," They are Invulnerable merchants!");
											return;
										}
										if (mapchar->isNpc()) npcattacktarget(DEREF_P_CHAR(pc_currchar),DEREF_P_CHAR(mapchar));
										soundeffect2(DEREF_P_CHAR(mapchar), 0x1, 0x60); //sound fix for swarm--Homey
										cMagic::doMoveEffect(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(mapchar), curSpell);
										cMagic::doStaticEffect(DEREF_P_CHAR(mapchar), curSpell);
										if (CheckResist(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(mapchar), 7))
										{
											MagicDamage(mapchar, pc_currchar->skill[MAGERY]/80);
										}
										else
										{
											MagicDamage(mapchar, (pc_currchar->skill[MAGERY]+1*(pc_currchar->skill[EVALUATINGINTEL]/3))/(39+1*(pc_i->skill[MAGICRESISTANCE]/30)));
										}
									}
									else
									{
										sysmessage(s, "You cannot see the target well.");
									}
								}
							}//if Mapitem
						}
						break;
						//////////// (58) ENERGY VORTEX /////////////
					case 58:
						SummonMonster( s, 0x00, 0x0d, "an energy vortex", 0x00, 0x75, x, y, z, curSpell );
						break;
					default:
						clConsole.send("MAGIC-ERROR: Unknown LocationTarget spell %i, magic.cpp\n", curSpell );
						break;
				}
				
				//AntiChrist - bugfix -
				//wall of stone has to be 3 tiles of length, like OSI
				int j2;
				if(id1==0x00 &&	id2==0x80)
				{//if a wall of stone
					j2=2;
				} else
				{
					j2=4;
				}
				if( fieldSpell( curSpell ) )
				{
					int R=calcRegionFromXY(x,y);
					if (R==-1) R=255;
					//clConsole.send("x: %i, y: %i, z: %i",x,y,z);
					if (region[R].priv&0x01) // Ripper 11-14-99
						if (region[R].priv&0x80 || region[R].priv&0x40) // LB magic region changes !
						{
							sysmessage(s," You cant cast in town!");
							return;
						}
						for( j=0; j<=j2; j++ )
						{
							P_ITEM pi=Items->SpawnItem(DEREF_P_CHAR(currchar[s]),1,"#",0,(id1<<8)+id2,0,0);
							if(pi)
							{
								pi->priv |= 0x05;
								pi->decaytime=uiCurrentTime+((pc_currchar->skill[MAGERY]/15)*MY_CLOCKS_PER_SEC);
								pi->morex=pc_currchar->skill[MAGERY]; // remember casters magery skill for damage, LB
								pi->MoveTo(fx[j], fy[j], Map->Height( fx[j], fy[j], z ));
								pi->dir=29;
								pi->magic=2;
								RefreshItem(pi);
								cMagic::itemParticles(snr,pi);
							}
						}
				}
			} // if los
			
			} else { // location check
				sysmessage(s,"That is not a valid location.");
			}
		}
		else if( reqItemTarget( curSpell ) )
		{
			// ITEM TARGET
			P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
			if(pi)
			{
				if ((line_of_sight( s, pc_currchar->pos, pi->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING)||
					(pc_currchar->isGM())))
				{
					playSound( DEREF_P_CHAR(pc_currchar), curSpell );
					switch( curSpell )
					{
						//////////// (13) MAGIC TRAP ////////////
					case 13:
						if((pi->type==1 || pi->type==63 ||
							pi->type==8 || pi->type==64)
							&& pi->id()!=0x0E75)
						{
							pi->moreb1=1;
							pi->moreb2=pc_currchar->skill[MAGERY]/20;
							pi->moreb3=pc_currchar->skill[MAGERY]/10;
							soundeffect3(pi,0x01F0);									
							cMagic::itemParticles(13,pi);
							sysmessage(s,"It's trapped!");
						}
						else
							sysmessage(DEREF_P_CHAR(pc_currchar),"You cannot trap this!!!");
						break;
						//////////// (14) MAGIC UNTRAP //////////
					case 14:
						if((pi->type==1 || pi->type==63 ||
							pi->type==8 || pi->type==64))
						{
							if(pi->moreb1==1)
							{
								if(rand()%100<=50+(pc_currchar->skill[MAGERY]/10)-pi->moreb3)
								{
									pi->moreb1=0;
									pi->moreb2=0;
									pi->moreb3=0;
									soundeffect3(pi, 0x01F1);
									cMagic::itemParticles(14,pi);
									sysmessage(s,"You successfully untrap this item!");
								}
								else sysmessage(s,"You miss to untrap this item!");
							}
							else sysmessage(s,"This item doesn't seem to be trapped!");
						}
						else sysmessage(s,"This item cannot be trapped!");
						break;
						//////////// (19) MAGIC LOCK ////////////
					case 19:
						if((pi->type==1 || pi->type==63)
							&& pi->id()!=0x0E75 )
						{
							switch(pi->type)
							{
							case 1:	pi->type=8; break;
							case 63: pi->type=64; break;
							default:
								clConsole.send("ERROR: Fallout of switch statement without default. magic.cpp, magiclocktarget()/n"); //Morrolan
								break;
							}
							soundeffect3(pi, 0x0200);
							cMagic::itemParticles(19,pi);
							sysmessage(s,"It's locked!");
						}
						else
							sysmessage(s,"You cannot lock this!!!");
						break;
						
						//////////// (23) MAGIC UNLOCK //////////
						
					case 23:
						if(pi->type==8 || pi->type==64)
						{
							if(pi->more1==0 && pi->more2==0 && pi->more3==0 && pi->more4==0)
							{ //Make sure it isn't an item that has a key (i.e. player house, chest..etc)
								switch(pi->type)
								{
								case 8: pi->type=1; break;
								case 64: pi->type=63; break;
								default:
									//LogError("switch reached default");
									return;
								}
								soundeffect3(pi,0x01FF);
								cMagic::itemParticles(23,pi);
								sysmessage(s, "You manage to pick the lock.");
							}
						} else
						{
							if ( pi->type==1 || pi->type==63 || pi->type == 65 || pi->type == 87 )
							{
								sysmessage( s, "That is not locked." );
							}
							else
							{
								sysmessage( s, "That does not have a lock.");
							}
						}
						break;
						//////////// (41) DISPEL ////////////////
						//case 41:
						//	if (pi->isInvul()) Items->DeleItem(i);
						//	break;
						// Uhmm.....this already is in CharacterTarget spells
						// don't think if it should be here......
					default:
						clConsole.send("MAGIC-ERROR: Unknown ItemTarget spell %i, magic.cpp\n", curSpell );
						break;
					}
				}
				else
					sysmessage( s, "There is something in the way!" );
			}
			else
				sysmessage( s, "That is not a valid item" );
		}
		else
			sysmessage( s, "Can't cope with this spell, requires a target but it doesn't specify what type" );
		pc_currchar->spell = 0;
		return;
	}
	else
	{
		// non targetted spells
		playSound( DEREF_P_CHAR(pc_currchar), curSpell );
		if ( curSpell != 35 && curSpell !=40) doStaticEffect( DEREF_P_CHAR(pc_currchar), curSpell );
		
		switch( curSpell )
		{
			//////////// (2) CREATE FOOD ////////////////
		case 2:
			{
				P_ITEM pi_j = Items->SpawnItem(s, DEREF_P_CHAR(pc_currchar), 1, "#", 1, 0x09, 0xD3, 0x00, 0x00, 1, 1 );
				if(pi_j != NULL)//AntiChrist - to prevent crashes
				{
					pi_j->type=14;
					RefreshItem(pi_j);
				}
			}
			break; // LB crashfix
			//////////// (3) SUMMON MONSTER ////////////
		case 40: //33:
			SummonMonster( s, 0, 0, "#", 0, 0, pc_currchar->pos.x+1, pc_currchar->pos.y+1, pc_currchar->pos.z, curSpell );
			break;
			//////////// (35) INCOGNITO ////////////
		case 35:
			tempeffect(DEREF_P_CHAR(pc_currchar),DEREF_P_CHAR(pc_currchar),19,0,0,0);//Incognito temp effect
			cMagic::doStaticEffect(DEREF_P_CHAR(pc_currchar), curSpell);
			break;
			//////////// (36) MAGIC REFLECTION /////////
		case 36:
			pc_currchar->priv2=pc_currchar->priv2|0x40;
			break;
			//////////// (56) POLYMORPH /////////////////
		case 56:
			PolymorphMenu( s, POLYMORPHMENUOFFSET ); // Antichrist's Polymorph
			break;
			//////////// (57) EARTHQUAKE ///////////////
		case 57:
			
			dmg=(pc_currchar->skill[MAGERY]/40)+(rand()%20-10);
			dmgmod = 0;
			
			//Char mapRegions
			loopexit=0;
			StartGrid=mapRegions->StartGrid(pc_currchar->pos.x,pc_currchar->pos.y);
			getcell=mapRegions->GetCell(pc_currchar->pos.x,pc_currchar->pos.y);
			increment=0;
			for (checkgrid=StartGrid+(increment*mapRegions->GetColSize());increment<3;increment++, checkgrid=StartGrid+(increment*mapRegions->GetColSize()))
			{
				for (a=0;a<3;a++)
				{
					vector<SERIAL> vecEntries = mapRegions->GetCellEntries(checkgrid+a);
					for ( unsigned int k = 0; k < vecEntries.size(); k++)
					{
						P_CHAR pc = FindCharBySerial(vecEntries[k]);
						if (pc != NULL)
						{
							if (pc->isInvul() || pc->npcaitype==17)		// don't affect vendors
								continue;
							if (pc->isSameAs(pc_currchar))				// nor the caster
								continue;
							ii=DEREF_P_CHAR(pc);
							distx=abs(pc->pos.x - pc_currchar->pos.x);
							disty=abs(pc->pos.y - pc_currchar->pos.y);
							if(distx<=15 && disty<=15 && (pc->isNpc() || online(ii)))
							{
								if(pc->isInnocent()) criminal(DEREF_P_CHAR(currchar[s]));
								
								if (!pc->isGM() && pc->account!=0)
									dmgmod = min(distx,disty);
								dmgmod = -(dmgmod - 7);
								pc->hp -=  dmg+dmgmod;
								pc->stm -= rand()%10+5;
								
								if(pc->stm<0)  pc->stm=0;
								if(pc->hp<0) pc->hp=0;						 							
								
								if (pc->isPlayer() && online(ii))
								{
									if(rand()%2) npcaction(ii, 0x15); else npcaction(ii, 0x16);
									if((pc->isNpc() || online(ii)) && pc->hp==0)
									{
										deathstuff(ii);                              
									}
								}	
								else
								{ 
									if (pc->hp<=0) deathstuff(ii); 
									else
									{ 
										if (pc->isNpc())
										{
											npcaction(ii, 0x2); 
											npcattacktarget(DEREF_P_CHAR(currchar[s]),ii); 
										}
									}
								}	
							} //if Distance
						}
					}
				}
			}
			break;			//////////// (60) SUMMON AIR ELEMENTAL /////
		case 60:
			SummonMonster( s, 0x00, 0x0d, "Air", 0, 0, pc_currchar->pos.x+1, pc_currchar->pos.y+1, pc_currchar->pos.z, curSpell );
			break;
			//////////// (61) SUMMON DAEMON ////////////
		case 61:
			SummonMonster( s, 0x00, 0x0A, "Daemon", 0, 0, pc_currchar->pos.x+1, pc_currchar->pos.y+1, pc_currchar->pos.z, curSpell );
			break;
			//////////// (62) SUMMON EARTH ELEMENTAL ///
		case 62:
			SummonMonster( s, 0x00, 0x0E, "Earth", 0, 0, pc_currchar->pos.x+1, pc_currchar->pos.y+1, pc_currchar->pos.z, curSpell );
			break;
			//////////// (63) SUMMON FIRE ELEMENTAL ////
		case 63:
			SummonMonster( s, 0x00, 0x0F, "Fire", 0, 0, pc_currchar->pos.x+1, pc_currchar->pos.y+1, pc_currchar->pos.z, curSpell );
			break;
			//////////// (64) SUMMON WATER ELEMENTAL ///
		case 64:
			SummonMonster( s, 0x00, 0x10, "Water", 0, 0, pc_currchar->pos.x+1, pc_currchar->pos.y+1, pc_currchar->pos.z, curSpell );
			break;
			//////////// (65) SUMMON HERO //////////////
		case 65:
			SummonMonster( s, 0x03, 0xE2, "Dupre the Hero", 0, 0, pc_currchar->pos.x+1, pc_currchar->pos.y+1, pc_currchar->pos.z, curSpell );
			break;
			//////////// (67) SUMMON BLACK NIGHT ///////
		case 67:
			SummonMonster( s, 0x00, 0x0A, "Black Night", 5000>>8, 5000%256, pc_currchar->pos.x+1, pc_currchar->pos.y+1, pc_currchar->pos.z, curSpell );
			break;
		default:
			clConsole.send("MAGIC-ERROR: Unknown NonTarget spell %i, magic.cpp\n", curSpell );
			break;
		}
		//sysmessage( s, "Spells like meteor swarm will go here, eventually" );
		pc_currchar->spell = 0;
		return;
	}
	
}

bool cMagic::townTarget( unsigned char num )
{
	switch( num )
	{
		case 2: // create food
		case 4: // heal
		case 6: // nightsight
		case 7: // reactive armor
		case 9: // agility
		case 11: // cure
		case 15: // protection
		case 16: // strength
		case 17: // bless
		case 22: // teleport
		case 25: // arch cure
		case 26: // arch protection
		case 29: // greater heal
		case 32: // recall
		case 40: // summoncreature
		case 44: // invisibility
		case 45: // mark
		case 48: // reveal
		case 52: // gate travel
		case 59: // resurrection
		case 36: // magic reflection
		case 56: // polymorph
		case 60: // air elemental
		case 61: // daemon
		case 62: // earth elemental
		case 63: // fire elemental
		case 64: // water elemental
			return true;
		default:
			return false;
	}
	return true;
}

bool cMagic::requireTarget( unsigned char num )
{

	switch( num )
	{
	case 1: // Clumsy
	case 3: // Feeblemind
	case 4: // Heal
	case 5: // Magic Arrow
	case 6: // Night Sight
	case 7:	// Reactive Armour
	case 8: // Weaken
	case 9: // Agility
	case 10:// Cunning
	case 11:// Cure
	case 12:// Harm
	case 13:// Magic Trap
	case 14:// Magic Untrap
	case 15:// Protection
	case 16:// Strength
	case 17:// Bless
	case 18:// Fireball
	case 19:// Magic Lock
	case 20:// Poison
	case 22:// Teleport
	case 23:// Unlock
	case 24:// Wall of Stone
	case 25:// Arch Cure
	case 26:// Arch protection
	case 27:// Curse
	case 28:// Fire Field
	case 29:// Greater Heal
	case 30:// Lightning
	case 31:// Mana drain
	case 32:// Recall
	case 33:// Blade Spirits
	case 34:// Dispel Field
	case 37:// Mind Blast
	case 38:// Paralyze
	case 39:// Poison Field
	case 41:// Dispel
	case 42:// Energy Bolt
	case 43:// Explosion
	case 44:// Invisibility
	case 45:// Mark
	case 46:// Mass curse
	case 47:// Paralyze Field
	case 48:// Reveal
	case 49:// Chain Lightning
	case 50:// Energy Field
	case 51:// Flamestrike
	case 52:// Gate Travel
	case 53:// Mana Vampire
	case 54:// Mass Dispel
	case 55:// Meteor Swarm
	case 58:// Energy Vortex
	case 59:// Resurrection
	case 66:// Cannon Firing
			return true;


	case 36:// Magic Reflection
	case 57:// Earthquake
	case 56:// Polymorph
	case 60:// Summon Air Elemental
	case 61:// Summon Daemon
	case 62:// Summon Earth Elemental
	case 63:// Summon Fire Elemental
	case 64:// Summon Water Elemental
	case 65:// Summon Hero
	case 67:// Summon Black Night
	default:
			return false;
	}
	return false;
}

void cMagic::DelReagents( CHARACTER s, reag_st reags )
{
	if (chars[s].priv2&0x80) return;
	delequan(s, 0x0F7A, reags.pearl);
	delequan(s, 0x0F7B, reags.moss);
	delequan(s, 0x0F84, reags.garlic);
	delequan(s, 0x0F85, reags.ginseng);
	delequan(s, 0x0F86, reags.drake);
	delequan(s, 0x0F88, reags.shade);
	delequan(s, 0x0F8C, reags.ash);
	delequan(s, 0x0F8D, reags.silk);
}

bool cMagic::spellReflectable( int num )
{

	//AntiChrist - customizable in spells.scp
	if(spells[num].reflect)
		return true;
	else return false;
}


bool cMagic::travelSpell( int num )
{
	switch( num )
	{
		case 52:
		case 32:
		case 45:
			return true;
		default:
			return false;
	}
	return false;
}

bool cMagic::reqCharTarget( int num )
{
	switch( num )
	{
		case 1:	// Clumsy
		case 3:	// Feeblemind
		case 4:	// Heal
		case 5:	// Magic Arrow
		case 6:	// Night Sight
		case 7:	// Reactive armour
		case 8:	// Weaken
		case 9:	// Agility
		case 10: // Cunning
		case 11: // Cure
		case 12: // Harm
		case 15: // Protection
		case 16: // Strength
		case 17: // Bless
		case 18: // Fireball
		case 20: // Poison
		case 27: // Curse
		case 29: // Greater Heal
		case 30: // Lightning
		case 31: // Mana drain
		case 37: // Mind Blast
		case 38: // Paralyze
		case 41: // Dispel
		case 42: // Energy Bolt
		case 43: // Explosion
		case 44: // Invisibility
		case 51: // Flamestrike
		case 53: // Mana Vampire
		case 56: // Polymorph
		case 59: // Resurrection
		case 66: // Cannon Firing
			return true;
		default:
			return false;
	}
	return false;
}

bool cMagic::reqLocTarget( int num )
{
	switch( num )
	{
		case 22:// Teleport
		case 24:// Wall of Stone
		case 25:// Arch Cure
		case 26:// Arch protection
		case 28:// Fire Field
		case 33:// Blade Spirits
		case 34:// Dispel Field
		case 39:// Poison Field
		case 46:// Mass curse
		case 47:// Paralyze Field
		case 48:// Reveal
		case 49:// Chainlightning
		case 50:// Energy Field
		case 54:// Mass Dispel
		case 55:// Meteor Swarm
		case 58:// Energy Vortex
			return true;
		default:
			return false;
	}
	return false;
}

bool cMagic::reqItemTarget( int num )
{
	switch( num )
	{
		case 13:// Magic Trap
		case 14:// Magic Untrap
		case 19:// Magic Lock
		case 23:// Unlock
			return true;
		default:
			return false;
	}
	return false;
}

move_st cMagic::getMoveEffects( int num )
{
	move_st temp;
	int te0,te1,te2,te3,te4;                // 2d move efefcts
	int te5,te6,te7,te8,te9,te10,te11,te12,te13, te14, te15, te16, te17; // 3d moving effects
	switch( num )
	{
	case 5:		te0=0x36; te1=0xE4; te2=0x05; te3=0x00; te4=0x01; 
		        te5=0x36; te6=0xe4; te7=0x07; te8=0x00; te9=0x0b; te10=0xbe; te11=0x0f; te12=0xa6; te13=0x00; te14=0x00; te15=0; te16=0; te17=0;
				break;

	case 18:	te0=0x36; te1=0xD5; te2=0x07; te3=0x00; te4=0x01; 
		        te5=0x36; te6=0xd4; te7=0x07; te8=0x00; te9=0x0b; te10=0xcb; te11=0x0f; te12=0xb3; te13=0x1; te14=0x60;te15=0; te16=1; te17=0;
		        break;

	case 42:	te0=0x37; te1=0x9F; te2=0x07; te3=0x00; te4=0x01; 
		        te5=0x37; te6=0x9f; te7=0x07; te8=0x00; te9=0x0b; te10=0xe3; te11=0x0f; te12=0xcb; te13=0x2; te14=0x11; te15=0; te16=1; te17=0;
		        break;

	case 55:	te0=0x36; te1=0xD5; te2=0x07; te3=0x00; te4=0x01; 
		        te5=0x36; te6=0xd4; te7=0x0a; te8=0x07; te9=0x25; te10=0x1d; te11=0x00; te12=0x01; te13=0x0; te14=0x0; te15=0; te16=1; te17=1;
		        break;
						        

	default:	te0=-1;	  te1=-1;   te2=-1;   te3=-1;   te4=-1;	  break;
	}

	temp.effect[0]=te0;
	temp.effect[1]=te1;
	temp.effect[2]=te2;
	temp.effect[3]=te3;
	temp.effect[4]=te4;
	temp.effect[5]=te5;
	temp.effect[6]=te6;
	temp.effect[7]=te7;
	temp.effect[8]=te8;
	temp.effect[9]=te9;
	temp.effect[10]=te10;
	temp.effect[11]=te11;
	temp.effect[12]=te12;
	temp.effect[13]=te13;
	temp.effect[14]=te14;
	temp.effect[15]=te15;
	temp.effect[16]=te16;
	temp.effect[17]=te17;

	return temp;
}

stat_st cMagic::getStatEffects( int num )
{
	stat_st temp;
	int te0,te1,te2,te3,te4;      // 2d effects
	int te5,te6,te7,te8,te9,te10,te11,te12; // 3d particles

	//te5=te6=te7=te8=te9=te10=0;
    te11=0; te12=1;
	switch( num )
	{
	case 1:		te0=0x37; te1=0x4A; te2=0x00; te3=15; 
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x8a; te10=0xffffff00;			
				break;
	case 3:		te0=0x37; te1=0x4A; te2=0x00; te3=15; 
		        te4=0x37; te5=0x79; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x8c; te10=0xffffff00;
		        break;
	case 4:		te0=0x37; te1=0x6A; te2=0x09; te3=0x06; 
		        te4=0x37; te5=0x6A; te6=0x09; te7=0x20;
				te8=0x13; te9=0x8d; te10=0xffffff00;
		        break;
	case 7:		te0=0x37; te1=0x3A; te2=0x00; te3=15;
		        te4=0x37; te5=0x6A; te6=0x09; te7=0x20;
				te8=0x13; te9=0x90; te10=0xffffff03;
		        break; 
	case 8:		te0=0x37; te1=0x4A; te2=0x00; te3=15;
		        te4=0x37; te5=0x79; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x91; te10=0xffffff00;
				break;
	case 9:		te0=0x37; te1=0x3A; te2=0x00; te3=15;
		        te4=0x37; te5=0x5a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x92; te10=0xffffff03;
				break;
	case 10:	te0=0x37; te1=0x3A; te2=0x00; te3=15;
		        te4=0x37; te5=0x5a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x93; te10=0xffffff00;
				break;
	case 11:	te0=0x37; te1=0x6A; te2=0x09; te3=0x06;
		        te4=0x37; te5=0x3a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x94; te10=0xffffff03;
				break;
	case 12:	te0=0x37; te1=0x4A; te2=0x09; te3=0x07; 
		        te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x95; te10=0xffffff03;
		        break;
	case 15:	te0=0x37; te1=0x6A; te2=0x09; te3=0x06; 
		        te4=0x37; te5=0x5a; te6=0x09; te7=0x14;
				te8=0x13; te9=0x98; te10=0xffffff03;
		        break;
	case 16:	te0=0x37; te1=0x3A; te2=0x00; te3=15; 
		        te4=0x37; te5=0x5a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x99; te10=0xffffff03;
		        break;
	case 17:	te0=0x37; te1=0x3A; te2=0x00; te3=15; 
		        te4=0x37; te5=0x3a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x9a; te10=0xffffff02;
		        break;
	case 20:	te0=0x37; te1=0x4A; te2=0x00; te3=15;
		        te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x9d; te10=0xffffff03;
		        break;
	case 22:	te0=0x37; te1=0x2A; te2=0x09; te3=0x06; 
		        te4=0x37; te5=0x28; te6=0x0a; te7=0x00;
				te8=0x13; te9=0x9f; te10=0xffffff00;
		        break;
	case 25:	te0=0x37; te1=0x6A; te2=0x09; te3=0x06; 
		        te4=0x37; te5=0x6a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x94; te10=0xffffff03;
		        break;
	case 26:    te0=0x37; te1=0x5A; te2=0x09; te3=0x06; 
		        te4=0x37; te5=0x5a; te6=0x09; te7=0x14;
				te8=0x13; te9=0xa3; te10=0xffffff03;
				break;
	case 27:	te0=0x37; te1=0x4A; te2=0x00; te3=15;
		        te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0xa4; te10=0xffffff03;
				break;
	case 29:	te0=0x37; te1=0x6A; te2=0x09; te3=0x06; 
                te4=0x37; te5=0x6a; te6=0x09; te7=0x20;
				te8=0x13; te9=0xa6; te10=0xffffff03;
		        break;
	case 49:
	case 30:	te0=0x37; te1=0x4A; te2=0x00; te3=15; 		
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x05;
				te8=0x13; te9=0xa7; te10=0xffffff03;
		        break;
	case 31:	te0=0x37; te1=0x4A; te2=0x00; te3=15; 		
		        te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0xa8; te10=0xffffff00;
		        break;
	case 32:  	te0=0x37; te1=0x4A; te2=0x00; te3=15; 		
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x05;
				te8=0x13; te9=0xa9; te10=0xffffff03;
		        break;
	case 33:  	te0=0x37; te1=0x4A; te2=0x00; te3=15; 		
		        te4=0x37; te5=0x28; te6=0x0a; te7=0x00;
				te8=0x13; te9=0xaa; te10=0xffffff00;
		        break;
	case 35:  	te0=0x37; te1=0x4A; te2=0x00; te3=15; 		
		        te4=0x37; te5=0x3a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0xac; te10=0xffffff00;
		        break;
	case 36:	te0=0x37; te1=0x3A; te2=0x00; te3=15;
		        te4=0x37; te5=0x5a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0xad; te10=0xffffff03;				
		        break;
	case 37:	te0=0x37; te1=0x4A; te2=0x00; te3=15; 	            
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x0f;
				te8=0x07; te9=0xf6; te10=0xffffff00;	
		        break;
	case 38:	te0=0x37; te1=0x35; te2=0x00; te3=30;
		        te4=0x37; te5=0x6a; te6=0x06; te7=0x01;
				te8=0x13; te9=0xaf; te10=0xffffff05;
				break;
	case 40:
	case 48:    te0=0x37; te1=0x35; te2=0x00; te3=30;
		        te4=0x37; te5=0x28; te6=0x0a; te7=0x0a;
				te8=0x13; te9=0xb1; te10=0xffffff00;
				break;
	case 43:	te0=0x36; te1=0xB0; te2=0x09; te3=0x09; 
		        te4=0x36; te5=0xbd; te6=0x0a; te7=0x0a;
				te8=0x13; te9=0xb4; te10=0xffffff00;
		        break;
	case 46:	te0=0x37; te1=0x4A; te2=0x00; te3=15; 
		        te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x95; te10=0xffffff03;		 
		        break;		
	case 51:	te0=0x37; te1=0x09; te2=0x09; te3=0x19; 			   
		        te4=0x37; te5=0x09; te6=0x0a; te7=0x1e;
				te8=0x13; te9=0xbc; te10=0xffffff05;
		        break;
	case 53:	te0=0x37; te1=0x4A; te2=0x00; te3=15; 
		        te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0xbe; te10=0xffffff00;		
		        break;
	case 54:	te0=0x37; te1=0x2A; te2=0x09; te3=0x06; 
		        te4=0x37; te5=0x28; te6=0x0a; te7=0x0a;
				te8=0x13; te9=0x9f; te10=0xffffff00;		
		        break;
	case 55:	te0=0x37; te1=0x2A; te2=0x09; te3=0x06; 
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x05;
				te10=rand()%3;
				switch (te10)
				{
				  case 0: te8=0x13; te9=0xc0; break;
				  case 1: te8=0x17; te9=0xa8; break;
				  case 2: te9=0x1b; te9=0x90; break;
				  default: te8=0x13; te9=0xc0; 
				}
				te10=0xffffff00;		
		        break;
	case 58: 	te0=0x37; te1=0x2A; te2=0x09; te3=0x06; 
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x05;
				te8=0x13; te9=0xc3; te10=0xffffff00;
				break;
	case 59: 	te0=0x37; te1=0x2A; te2=0x09; te3=0x06; 
		        te4=0x36; te5=0x6a; te6=0x09; te7=0x20;
			    te8=0x25; te9=0x1d; te10=0xffffff03;			
				break;
	case 60: 	te0=0x37; te1=0x2A; te2=0x09; te3=0x06; 
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x20;
			    te8=0x13; te9=0xc5; te10=0xffffff03;			
				//te11=27; te12=0xf;
				break;
	case 61: 	te0=0x37; te1=0x2A; te2=0x09; te3=0x06; 
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x00;
			    te8=0x13; te9=0xc6; te10=0xffffff03;			
				break;
	case 62: 	te0=0x37; te1=0x2A; te2=0x09; te3=0x06; 
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x20;
			    te8=0x13; te9=0xc7; te10=0xffffff03;			
				break;
	case 63: 	te0=0x37; te1=0x2A; te2=0x09; te3=0x06; 
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x20;
			    te8=0x13; te9=0xc8; te10=0xffffff03;			
				break;
	case 64: 	te0=0x37; te1=0x2A; te2=0x09; te3=0x06; 
		        te4=0x00; te5=0x00; te6=0x09; te7=0x00;
			    te8=0x13; te9=0xc9; te10=0xffffff03;			
				break;

	case 66:	te0=0x36; te1=0xB0; te2=0x09; te3=0x09; 
		        te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f;
				te8=0x13; te9=0x9e; te10=0xffffff00;	
		        break;

	case 99:	te0=0x37; te1=0x35; te2=0x00; te3=0x30;  // fizzle
		        te4=0x00; te5=0x00; te6=0x0a; te7=0x0f;
				te8=0x0f; te9=0xcb; te10=0xffffff01;	
		        break;			 

	default:	te0=-1;	te1=-1;	te2=-1;	te3=-1; break;
	}

	temp.effect[0]=te0;
	temp.effect[1]=te1;
	temp.effect[2]=te2;
	temp.effect[3]=te3;
	temp.effect[4]=te4;
	temp.effect[5]=te5;
	temp.effect[6]=te6;
	temp.effect[7]=te7;
	temp.effect[8]=te8;
	temp.effect[9]=te9;
	temp.effect[10]=te10;
	temp.effect[11]=te11;
	temp.effect[12]=te12;
	
	return temp;
}


stat_st cMagic::getStatEffects_pre( int num)
{
    stat_st temp;
    memset( &temp, -1, sizeof ( stat_st ) );    	
	int te4,te5,te6,te7,te8,te9,te10,te11,te12; // 3d particles

	te11=0; te12=1;
	switch( num )
	{
	case 1:	   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x47; te10=0xffff0102; break;
	case 2:	   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x33; te10=0xffff0102; break;
	case 3:	   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x47; te10=0xffff0102; break;		        
	case 4:	   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x65; te10=0xffff0102; break;		
	case 5:	   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x51; te10=0xffff0102; break;
	case 6:	   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x47; te10=0xffff0102; break;
	case 7:	   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x33; te10=0xffff0102; break;		        
	case 8:	   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x47; te10=0xffff0102; break;

	case 9:	   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x65; te10=0xffff0102; break;
	case 10:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x65; te10=0xffff0102; break;
	case 11:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x65; te10=0xffff0102; break;
	case 12:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x51; te10=0xffff0102; break;
	case 13:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x29; te10=0xffff0102; break;
	case 14:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x29; te10=0xffff0102; break;
	case 15:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x33; te10=0xffff0102; break;
	case 16:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x65; te10=0xffff0102; break;

	case 17:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x51; te10=0xffff0102; break;
	case 18:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x51; te10=0xffff0102; break;
	case 19:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x29; te10=0xffff0102; break;
	case 20:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x5b; te10=0xffff0102; break;
	case 21:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x47; te10=0xffff0102; break;
	case 22:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x47; te10=0xffff0102; break;
	case 23:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x29; te10=0xffff0102; break;
	case 24:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x33; te10=0xffff0102; break;
	case 25:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x65; te10=0xffff0102; break;

	case 26:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x33; te10=0xffff0102; break;
	case 27:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x47; te10=0xffff0102; break;
	case 28:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x51; te10=0xffff0102; break;
	case 29:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x65; te10=0xffff0102; break;
	case 30:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x3d; te10=0xffff0102; break;
	case 31:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x47; te10=0xffff0102; break;
	case 32:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x47; te10=0xffff0102; break;

	case 33:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x50; te10=0x02010102; break;
	case 34:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x2a; te10=0xffff0102; break;
	case 35:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x2a; te10=0xffff0102; break;
	case 36:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x34; te10=0xffff0102; break;
	case 37:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x48; te10=0xffff0102; break;
	case 38:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x34; te10=0xffff0102; break;
	case 39:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x5c; te10=0xffff0102; break;
	case 40:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x50; te10=0xffff0102; break;

	case 41:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x2a; te10=0xffff0102; break;
	case 42:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x3e; te10=0xffff0102; break;
	case 43:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x51; te10=0xffff0102; break;
	case 44:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x2a; te10=0xffff0102; break;
	case 45:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x2a; te10=0xffff0102; break;
	case 46:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x47; te10=0xffff0102; break;
	case 47:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x34; te10=0xffff0102; break;
	case 48:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x2a; te10=0xffff0102; break;

	case 49:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x3e; te10=0xffff0102; break;
	case 50:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x3e; te10=0xffff0102; break;
	case 51:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x52; te10=0xffff0102; break;
	case 52:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x48; te10=0xffff0102; break;
	case 53:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x48; te10=0xffff0102; break;
	case 54:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x2a; te10=0xffff0102; break;
	case 55:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x52; te10=0xffff0102; break;
	case 56:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x2a; te10=0xffff0102; break;

	case 57:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x34; te10=0xffff0102; break;
	case 58:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x48; te10=0xffff0102; break;
	case 59:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x66; te10=0xffff0102; break;
	case 60:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x32; te10=0xffff0102; break;
	case 61:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x5a; te10=0xffff0102; break;
	case 62:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x3c; te10=0xffff0102; break;
	case 63:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x5a; te10=0xffff0102; break;
	case 64:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te9=0x6e; te10=0xffff0102; break;

	default:	te4=-1;	te5=-1;	te6=-1;	te7=-1; break;
	}

	temp.effect[4]=te4;
	temp.effect[5]=te5;
	temp.effect[6]=te6;
	temp.effect[7]=te7;
	temp.effect[8]=te8;
	temp.effect[9]=te9;
	temp.effect[10]=te10;
	temp.effect[11]=te11;
	temp.effect[12]=te12;
	
	return temp;
}

stat_st cMagic::getStatEffects_after( int num)
{
    stat_st temp;
    memset( &temp, -1, sizeof ( stat_st ) );    	
	int te4,te5,te6,te7,te8,te9,te10, te11,te12;

	te11=0; te12=1;
	switch( num )
	{
	   case 6:	  te4=0x37; te5=0x6a; te6=0x09; te7=0x20; te8=0x13; te9=0x8f; te10=0xffff0003; break;	  	 
	   case 37:   te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f; te8=0x13; te9=0xae; te10=0xffff0000; break;
	   case 38:   te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x13; te9=0xaf; te10=0xffffff05; te11=0x27; te12=0x0f; break;
	   default:	te4=-1;	te5=-1;	te6=-1;	te7=-1; break;
	}

	temp.effect[4]=te4;
	temp.effect[5]=te5;
	temp.effect[6]=te6;
	temp.effect[7]=te7;
	temp.effect[8]=te8;
	temp.effect[9]=te9;
	temp.effect[10]=te10;
	temp.effect[11]=te11;
	temp.effect[12]=te12;
	
	return temp;
}


stat_st cMagic::getStatEffects_item( int num)
{
    stat_st temp;
    memset( &temp, -1, sizeof ( stat_st ) );    	
	int te4,te5,te6,te7,te8,te9,te10, te11;

	switch( num )
	{	  
	   case 13:   te4=0x37; te5=0x6a; te6=0x09; te7=0x0a; te8=0x13; te9=0x96; te10=0xffff0000; te11=0; break;
	   case 14:   te4=0x37; te5=0x6a; te6=0x09; te7=0x20; te8=0x13; te9=0x8f; te10=0xffff0003; te11=0; break;
	   case 19:   te4=0x37; te5=0x6a; te6=0x09; te7=0x20; te8=0x13; te9=0x9c; te10=0xffff0002; te11=0; break;
	   case 22:   te4=0x37; te5=0x28; te6=0x0a; te7=0x00; te8=0x07; te9=0xe7; te10=0xffff0002; te11=0; break;
	   case 23:   te4=0x37; te5=0x6a; te6=0x09; te7=0x20; te8=0x13; te9=0xa0; te10=0xffff0003; te11=0; break;
	   case 24:   te4=0x37; te5=0x6a; te6=0x09; te7=0x0a; te8=0x13; te9=0xa1; te10=0xffff0003; te11=0; break;
	   case 28:   te4=0x37; te5=0x6a; te6=0x0f; te7=0x0a; te8=0x13; te9=0xa5; te10=0xffff0000; te11=0; break;
	   case 32:   te4=0x00; te5=0x00; te6=0x00; te7=0x00; te8=0x13; te9=0xa9; te10=0xffff0003; te11=0; break;
	   case 39:   te4=0x37; te5=0x6a; te6=0x09; te7=0x0a; te8=0x13; te9=0xb0; te10=0xffff0000; te11=0; break;
	   case 44:   te4=0x37; te5=0x6a; te6=0x0a; te7=0x0f; te8=0x13; te9=0xb5; te10=0xffff0003; te11=0; break;
	   case 45:   te4=0x37; te5=0x79; te6=0x0a; te7=0x0f; te8=0x13; te9=0xb6; te10=0xffff0003; te11=0; break;
	   case 47:   te4=0x37; te5=0x6a; te6=0x09; te7=0x0a; te8=0x13; te9=0xb8; te10=0xffff0003; te11=0; break;
	   case 50:   te4=0x37; te5=0x6a; te6=0x09; te7=0x0a; te8=0x13; te9=0xbb; te10=0xffff0003; te11=0; break;
	   case 59:   te4=0x00; te5=0x00; te6=0x00; te7=0x00; te8=0x13; te9=0xc4; te10=0xffff0003; te11=0; break;

	   default:	te4=-1;	te5=-1;	te6=-1;	te7=-1; break;
	}

	temp.effect[4]=te4;
	temp.effect[5]=te5;
	temp.effect[6]=te6;
	temp.effect[7]=te7;
	temp.effect[8]=te8;
	temp.effect[9]=te9;
	temp.effect[10]=te10;
	temp.effect[11]=te11;
	
	return temp;
}


void cMagic::invisibleItemParticles(CHARACTER c, int spellNum, short x, short y, signed char z)
{
	P_ITEM it;

    // create a dummy item for the effect on old location 
    it=Items->SpawnItem(c, 1, "bugalert, plz let the devteam know", 0, 0x1, 0x00, 0);					
    it->pos.x=x;
	it->pos.y=y;
	it->pos.z=z;					
	RefreshItem( it );
	itemParticles(spellNum, it );
	// this is rather tricky, deleitem can't be applied there 
	// because the client has to tihnk its still there. np because it's an invisible item anyway
	// but we have to tell the memory manger to delete it
	cItemsManager::getItemsManager().unregisterItem(it);
	delete it;	
}

sound_st cMagic::getSoundEffects( int num )
{
	sound_st temp;
	int te0,te1;

	switch( num )	// The reason why I deleted some is because they already have
	{				// sounds in other parts of the code, keep them in if you want,
					// even the deleted numbers have been corrected - Paul77
	case 1:		te0=0x01; te1=0xDF; break;
	case 2:		te0=0x01; te1=0xE2; break;
	case 3:		te0=0x01; te1=0xE4; break;
	case 4:		te0=0x01; te1=0xF2; break;
	case 5:		te0=0x01; te1=0xE5; break;
	case 6:		te0=0x01; te1=0xE3; break;
	case 7:		te0=0x01; te1=0xEE; break;
	case 8:		te0=0x01; te1=0xE6; break;
	case 9:		te0=0x01; te1=0xE7; break;
	case 10:	te0=0x01; te1=0xEB; break;
	case 11:	te0=0x01; te1=0xE0; break; // Changed by Paul77
	case 12:	te0=0x01; te1=0xF1; break;
	case 13:	te0=0x01; te1=0xEF; break; // Added by Paul77
	case 14:	te0=0x01; te1=0xF0; break; // Added by Paul77
	case 15:	te0=0x01; te1=0xED; break; // Changed by Paul77
	case 16:	te0=0x01; te1=0xEE; break;
	case 17:	te0=0x01; te1=0xEA; break;
	case 18:	te0=0x01; te1=0x5E; break;
	case 19:	te0=0x01; te1=0xF4; break; // Added by Paul77
	case 20:	te0=0x02; te1=0x05; break; // Changed by Paul77
	case 22:	te0=0x01; te1=0xFE; break;
	case 23:	te0=0x01; te1=0xFF; break; // Added by Paul77
	case 24:	te0=0x01; te1=0xF6; break;
//	case 25:	te0=0x01; te1=0xE9; break; Deleted by Paul77
	case 26:	te0=0x01; te1=0xF7; break;
	case 27:	te0=0x01; te1=0xE1; break;
	case 28:	te0=0x02; te1=0x0C; break;
	case 29:	te0=0x02; te1=0x02; break;
	case 30:	te0=0x00; te1=0x29; break;
	case 31:	te0=0x01; te1=0xF9; break;
	case 32:	te0=0x01; te1=0xFC; break;
//	case 33:	te0=0x02; te1=0x12; break; Deleted by Paul77
	case 34:	te0=0x02; te1=0x01; break;
//	case 35:	te0=0x02; te1=0x03; break; Deleted by Paul77 - Should not have a sound for Incognito
	case 36:	te0=0x01; te1=0xE9; break;
	case 37:	te0=0x02; te1=0x13; break;
	case 38:	te0=0x02; te1=0x04; break;
	case 39:	te0=0x02; te1=0x0C; break;
	case 41:	te0=0x02; te1=0x01; break; // Changed by Paul77
	case 42:	te0=0x02; te1=0x0A; break;
	case 43:	te0=0x02; te1=0x07; break;
	case 44:	te0=0x02; te1=0x03; break;
	case 45:	te0=0x01; te1=0xFA; break;
//	case 46:	te0=0x01; te1=0xFB; break; Changed and Deleted by Paul77
	case 47:	te0=0x02; te1=0x0B; break;
	case 48:	te0=0x01; te1=0xFD; break; // Sound happens twice (probably justified so kept it) - Paul77
//	case 49:	te0=0x00; te1=0x29; break; Deleted by Paul77
	case 50:	te0=0x02; te1=0x0B; break; // Changed by Paul77
	case 51:	te0=0x02; te1=0x08; break;
	case 52:	te0=0x02; te1=0x0E; break;
	case 53:	te0=0x01; te1=0xF9; break; // Changed by Paul77
	case 54:	te0=0x02; te1=0x09; break; // Changed by Paul77
	case 55:	te0=0x01; te1=0x60; break; // Sound happens twice (probably justified so kept it) - Paul77
//	case 58:	te0=0x02; te1=0x12; break; Changed and Deleted by Paul77
//	case 60:	te0=0x02; te1=0x17; break; Deleted by Paul77
//	case 61:	te0=0x02; te1=0x16; break; Deleted by Paul77
//	case 62:	te0=0x02; te1=0x17; break; Deleted by Paul77
//	case 63:	te0=0x02; te1=0x17; break; Deleted by Paul77
//	case 64:	te0=0x02; te1=0x17; break; Deleted by Paul77
	case 65:	te0=0x02; te1=0x46; break;
	case 66:	te0=0x02; te1=0x07; break;

	default:	te0=-1;	te1= -1; break;
	}
	temp.effect[0]=te0;
	temp.effect[1]=te1;
	return temp;
}

void cMagic::playSound( int source, int num )
{
	sound_st temp;
	temp.effect[0] = -1;
	temp.effect[1] = -1;

	temp = getSoundEffects( num );
	if( temp.effect[0] != -1 && temp.effect[1] != -1 )
		soundeffect2( source, temp.effect[0], temp.effect[1] );
}

void cMagic::doStaticEffect( int source, int num )
{
	stat_st temp;
	memset( &temp , -1, sizeof(stat_st) );	
	temp = getStatEffects( num );

	if( temp.effect[0] != -1 && temp.effect[1] != -1 && temp.effect[2] != -1 && temp.effect[3] != -1 )
	{
		staticeffect( source, temp.effect[0], temp.effect[1], temp.effect[2], temp.effect[3], true,  &temp);
		// looks stupid to pass a pointer to a struct variable and elements of the *same* struct variable
		// actually it's very tricky but ok. (via pointer #4..15 is accessed, saved work to change a few 1000 LOC's) 
		// please don't touch. thx, LB 
	}
}

void cMagic::doMoveEffect( int num, int target, int source )
{
	move_st temp;
	memset( &temp , -1, sizeof( move_st) );	

	temp = getMoveEffects( num );

	if( temp.effect[0] != -1 && temp.effect[1] != -1 && temp.effect[2] != -1 && temp.effect[3] != -1 && temp.effect[4] != -1 )
		movingeffect(source, target, temp.effect[0], temp.effect[1], temp.effect[2], temp.effect[3], temp.effect[4], true, &temp );
}

bool cMagic::aggressiveSpell( int num )
{
	switch( num )
	{
	case 1: // Clumsy
	case 3: // Feeblemind
	case 5: // Magic Arrow
	case 8: // Weaken
	case 12:// Harm
	case 18:// Fireball
	case 20:// Poison
	case 27:// Curse
	case 30:// Lightning
	case 31:// Mana drain
	case 37:// Mind Blast
	case 38:// Paralyze
	case 42:// Energy Bolt
	case 43:// Explosion
	case 46:// Mass curse
	case 49:// Chain Lightning
	case 51:// Flamestrike
	case 53:// Mana Vampire
	case 55:// Meteor Swarm
	case 66:// Cannon Firing
			return true;
	default:
			return false;
	}
	return false;
}

bool cMagic::fieldSpell( int num )
{
	switch( num )
	{
		case 24:// Wall of Stone
		case 28:// Fire Field
		case 39:// Poison Field
		case 47:// Paralyze Field
		case 50:// Energy Field
			return true;
		default:
			return false;
	}
}

// added by AntiChrist (9/99)
void cMagic::PolymorphMenu(int s,int gmindex)
{
	int total, i, loopexit=0;
	char lentext;
	char sect[512];
	static char gmtext[30][257];
	int gmid[30];
	int gmnumber=0,dummy=0;

	openscript("polymorph.scp");
	sprintf(sect, "POLYMORPHMENU %i",gmindex);
	if (!i_scripts[polymorph_script]->find(sect))
	{
		closescript();
		return;
	}
	read1();
	lentext=sprintf(gmtext[0],(char*)script1);
	do
	{
		read2();
		if (script1[0]!='}' && strcmp("POLYMORPHID",(char*)script1))
		{
			gmnumber++;
			gmid[gmnumber]=hex2num(script1);
			strcpy(gmtext[gmnumber],(char*)script2);
			read1();
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );

	// reading polymorph duration time ...
	sprintf(sect, "POLYMORPHDURATION %i",dummy);
	if (!i_scripts[polymorph_script]->find(sect))
	{
		closescript();
		return;
	}

	loopexit=0;
	do
	{
		read2();
		if (script1[0]!='}')
		{
			polyduration=str2num(script1);
			read1();
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	closescript();

//	CHARACTER cc=currchar[s];
	P_CHAR pc_currchar = currchar[s];
	total=9+1+lentext+1;
	for (i=1;i<=gmnumber;i++) total+=4+1+strlen(gmtext[i]);
	gmprefix[1]=total>>8;
	gmprefix[2]=total%256;
	gmprefix[3]=pc_currchar->ser1;
	gmprefix[4]=pc_currchar->ser2;
	gmprefix[5]=pc_currchar->ser3;
	gmprefix[6]=pc_currchar->ser4;
	gmprefix[7]=gmindex>>8;
	gmprefix[8]=gmindex%256;
	Xsend(s, gmprefix, 9);
	Xsend(s, &lentext, 1);
	Xsend(s, gmtext[0], lentext);
	lentext=gmnumber;
	Xsend(s, &lentext, 1);
	for (i=1;i<=gmnumber;i++)
	{
		gmmiddle[0]=gmid[i]>>8;
		gmmiddle[1]=gmid[i]%256;
		Xsend(s, gmmiddle, 4);
		lentext=strlen(gmtext[i]);
		Xsend(s, &lentext, 1);
		Xsend(s, gmtext[i], lentext);
	}
}

// added by AntiChrist (9/99)
void cMagic::Polymorph(int s, int gmindex, int creaturenumber)
{
	int i,k,loopexit=0;
	int id1,id2;

	char sect[512];
	openscript("polymorph.scp");
	sprintf(sect, "POLYMORPHMENU %i",gmindex);
	if (!i_scripts[polymorph_script]->find(sect))
	{
		closescript();
		return;
	}
	read1();
	i=0,k=0;

	do
	{
		read2();
		if (script1[0]!='}' && strcmp("POLYMORPHID",(char*)script1))
		{
			i++;
		}
	}
	while ( (script1[0]!='}') && (i<creaturenumber) && (++loopexit < MAXLOOPS) );

	read2();
	k=hex2num(script2);

	closescript();

	id1=k>>8;
	id2=k%256;
//	int cc=currchar[s];
	P_CHAR pc_currchar = currchar[s];
	//	soundeffect2(DEREF_P_CHAR(pc_currchar), 0x02, 0x0F); Deleted by Paul77 - Polymorph doesn't have a sound
	tempeffect(DEREF_P_CHAR(pc_currchar),DEREF_P_CHAR(pc_currchar),18,id1,id2,0);

	teleport(DEREF_P_CHAR(pc_currchar));
}

// only used for the /heal command
// LB
void cMagic::Heal(UOXSOCKET s)
{
	P_CHAR pc_currchar = currchar[s];
	SERIAL defender=LongFromCharPtr(buffer[s]+7);
	P_CHAR pc_defender = FindCharBySerial( defender );
	if (pc_defender != NULL)
	{
		playSound( DEREF_P_CHAR(pc_currchar), 4);
		doStaticEffect(DEREF_P_CHAR(pc_defender), 4);
		pc_defender->hp = pc_defender->st;
		updatestats((pc_defender), 0);
	} else 
		sysmessage(s,"Not a valid heal target");

}

// only used for the /recall command
// AntiChrist
void cMagic::Recall(UOXSOCKET s)
{
	P_ITEM pi=FindItemBySerPtr(buffer[s]+7);	//Targeted item
//	CHARACTER cc=currchar[s];
	P_CHAR pc_currchar = currchar[s];
	if (pi)
	{
		if ( pi->morex<=200 && pi->morey<=200 )
		{
			sysmessage(s,"That rune has not been marked yet!");
		}
		else
		{
			pc_currchar->MoveTo(pi->morex,pi->morey,pi->morez); //LB
			teleport(DEREF_P_CHAR(pc_currchar));
			sysmessage(s,"You have recalled from the rune.");
		}
	} else sysmessage(s,"Not a valid recall target");//AntiChrist
}

// only used for the /mark command
// AntiChrist
void cMagic::Mark(UOXSOCKET s)
{
	//Targeted item
	P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	P_CHAR pc_currchar = currchar[s];
	if (pi)
	{
		pi->morex=pc_currchar->pos.x;
		pi->morey=pc_currchar->pos.y;
		pi->morez=pc_currchar->pos.z;
		sysmessage(s,"Recall rune marked.");
	} else sysmessage(s,"Not a valid mark target");//AntiChrist
}
/*
void cMagic::CannonTarget(int s)
{
	int i, j, success;
	for (i=0;i<charcount;i++)
	{
		if ((chars[i].ser1==buffer[s][7])&&(chars[i].ser2==buffer[s][8])&&
			(chars[i].ser3==buffer[s][9])&&(chars[i].ser4==buffer[s][10]))
		{
			
			if (CheckMagicReflect(i)) i=cc;
			staticeffect(i, 0x36, 0xB0, 0x09, 0x09);
			soundeffect2(i, 0x02, 0x07);
			for (j=0;j<charcount;j++)
			{
				if (chars[j].x==chars[i].x && chars[j].y==chars[i].y && chars[j].z==chars[i].z)
				{
					npcattacktarget(j, cc);
					if (CheckParry(j, 6))
					{
						MagicDamage(j, pc_currchar->skill[TACTICS]/50);
					}
					else
					{
						MagicDamage(j, pc_currchar->skill[TACTICS]/25);
					}
				}
			}
			return;
		}
	}
}
*/
void cMagic::BuildCannon(int s)
{
	if (buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) return;
	
	soundeffect(s, 0x02, 0x45);
	soundeffect(s, 0x02, 0x46);
	
	P_ITEM pi_k = Items->SpawnItem(-1,s,1,"#",0,0x0E,0x91,0,0,0,1);
	pi_k->type=15;
	pi_k->morex=8;
	pi_k->morey=10;
	pi_k->morez=0;
	pi_k->magic=1;
	pi_k->decaytime=0;
	pi_k->pos.x=(buffer[s][11]*256)+buffer[s][12];
	pi_k->pos.y=(buffer[s][13]*256)+buffer[s][14];
	pi_k->pos.z=buffer[s][16];
	RefreshItem(pi_k);
}


// only used for the /gate command
// AntiChrist
void cMagic::Gate(UOXSOCKET s)
{
	int n;

	P_ITEM pi=FindItemBySerPtr(buffer[s]+7);	//Targeted item
	if (pi)
	{
		if ( pi->morex<=200 && pi->morey<=200 )
		{
			sysmessage(s,"That rune has not been marked yet!");
		}
		else
		{
//			CHARACTER cc = currchar[s];
			P_CHAR pc_currchar = currchar[s];
			gatex[gatecount][0]=pc_currchar->pos.x;	//create gate a player location
			gatey[gatecount][0]=pc_currchar->pos.y;
			gatez[gatecount][0]=pc_currchar->pos.z;
			gatex[gatecount][1]=pi->morex; //create gate at marked location
			gatey[gatecount][1]=pi->morey;
			gatez[gatecount][1]=pi->morez;

			for (n=0;n<2;n++)
			{
				strcpy((char*)temp,"a blue moongate");
				P_ITEM pi_c = Items->SpawnItem(-1,s,1,"#",0,0x0f,0x6c,0,0,0,0);
				if(pi_c != NULL)//AntiChrist - to prevent crashes
				{
					pi_c->type=51+n;
					pi_c->pos.x=gatex[gatecount][n];
					pi_c->pos.y=gatey[gatecount][n];
					pi_c->pos.z=gatez[gatecount][n];
					pi_c->gatetime=(uiCurrentTime+(SrvParms->gatetimer*MY_CLOCKS_PER_SEC));
					//clConsole.send("GATETIME:%i UICURRENTTIME:%d GETCLOCK:%d\n",SrvParms->gatetimer,uiCurrentTime,getclock());
					pi_c->gatenumber=gatecount;
					pi_c->dir=1;
					mapRegions->Add(pi_c);	//add gate to list of items in the region
					RefreshItem(pi_c);//AntiChrist
				}
				if (n==1)
				{
					gatecount++;
					if (gatecount>MAXGATES) gatecount=0;
				}
				addid1[s]=0;
				addid2[s]=0;
			}
		}
	} else sysmessage(s,"Not a valid gate target");//AntiChrist
}

///////////////////
// Name:	SpellNumFromScrollID
// History:	by Duke, 18.3.2001
// Purpose:	Encapsulates the conversion of spell numbers from UO to WP
//			'reactive armor' is #1 in UO, but #7 in WP. So we must shift scrolls 2-7.
//
short cMagic::SpellNumFromScrollID(short id)
{
	if (id==0x1F2D)					return 7;			// Reactive Armor
	if (id>=0x1F2E && id<=0x1F33)	return (short) (id-0x1F2D);	// first circle without weaken
	if (id>=0x1F34 && id<=0x1F6C)	return (short)(id-0x1F2D+1);	// 2 to 8 circle spell scrolls plus weaken
	return -1;						// not a scroll
}
