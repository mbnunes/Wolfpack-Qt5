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
#include "SndPkg.h"
#include "debug.h"

#undef  DBGFILE
#define DBGFILE "p_ai.cpp"

void cCharStuff::CheckAI(unsigned int currenttime, int i) // Lag Fix -- Zippy
{
	int d, onl;
	unsigned int chance;
	// unsigned int StartGrid, getcell, increment, a, checkgrid;
	P_CHAR pc_i = MAKE_CHARREF_LR(i);
	if ( pc_i == NULL )
		return;
	
	if (!(nextnpcaitime <= currenttime || (overflow)))
		return;
    // in npc.scp add script # for npc`s ai
	// case - script - case - script -   case - script -  case - script
	//   0   -  0     -  4    -  4     -  8    -  8     -  12   -  C
	//   1   -  1     -  5    -  5     -  9    -  9     -  13   -  D
	//   2   -  2     -  6    -  6     -  10   -  A     -  14   -  E
	//   3   -  3     -  7    -  7     -  11   -  B     -  15   -  F
	// case - script - case - script
	//   16  -  10    -  20   -  14
	//   17  -  11    -  21   -  15
	//   18  -  12    -  22   -  16
	//   19  -  13    -  23   -  17  ...this is just a guide...Ripper
	switch (pc_i->npcaitype)
	{
		case 0: // Shopkeepers greet players..Ripper
			if (server_data.VendorGreet == 1 && pc_i->isNpc() && pc_i->shop == 1 && pc_i->isHuman())
			{
				cRegion::RegionIterator4Chars ri(pc_i->pos);
				for (ri.Begin(); ri.GetData() != ri.End(); ri++)
				{
					P_CHAR pc = ri.GetData();
					if (pc != NULL)
					{
						onl = online(pc);
						d = chardist( pc_i, pc );
						if (d > 3)
							continue;
						if (pc->isInvul() || pc->isNpc() || pc->dead || !pc->isInnocent() || !onl)
							continue;
						sprintf((char*)temp, "Hello %s, Welcome to my shop, How may i help thee?.", pc->name);
						npctalkall(pc_i, (char*)temp, 1);
						pc_i->antispamtimer = uiCurrentTime + MY_CLOCKS_PER_SEC*30;
					}
				}
			}
			break;
		case 1: // good healers
			if (!pc_i->war)
			{
				cRegion::RegionIterator4Chars ri(pc_i->pos);
				for (ri.Begin(); ri.GetData() != ri.End(); ri++)
				{
					P_CHAR pc = ri.GetData();
					if (pc != NULL)
					{
						onl = online(pc);
						d = chardist( pc_i, pc );
						if (!pc->dead || d > 3 || pc->isNpc() || !onl)
							continue;
						if (pc->isMurderer()) 
						{
							npctalkall(pc_i, "I will nay give life to a scoundrel like thee!", 1);
							return;
						}
						else if (pc->isCriminal()) 
						{
							npctalkall(pc_i, "I will nay give life to thee for thou art a criminal!", 1);
							return;
						}
						else if (pc->isInnocent())
						{
							npcaction(DEREF_P_CHAR(pc_i), 0x10);
							Targ->NpcResurrectTarget(pc);
							staticeffect(pc, 0x37, 0x6A, 0x09, 0x06);
							switch (RandomNum(0, 4)) 
							{
							case 0: 
								npctalkall(pc_i, "Thou art dead, but 'tis within my power to resurrect thee.  Live!", 1);
								break;
							case 1: 
								npctalkall(pc_i, "Allow me to resurrect thee ghost.  Thy time of true death has not yet come.", 1);
								break;
							case 2: 
								npctalkall(pc_i, "Perhaps thou shouldst be more careful.  Here, I shall resurrect thee.", 1);
								break;
							case 3: 
								npctalkall(pc_i, "Live again, ghost!  Thy time in this world is not yet done.", 1);
								break;
							case 4: 
								npctalkall(pc_i, "I shall attempt to resurrect thee.", 1);
								break;
							}
						}
					}
				}
			}
			break;
		case 2 : // Monsters, PK's - (stupid NPCs)
			if (!pc_i->war)
			{
				cRegion::RegionIterator4Chars ri(pc_i->pos);
				for (ri.Begin(); ri.GetData() != ri.End(); ri++)
				{
					P_CHAR pc = ri.GetData();
					if (pc != NULL)
					{
						onl = online(pc);
						d = chardist( pc_i, pc );
						chance = RandomNum(1, 100);
						if (DEREF_P_CHAR(pc) == DEREF_P_CHAR(pc_i))
							continue;
						if (d>SrvParms->attack_distance)
							continue;
						if (onl &&(pc->isInvul() || pc->isHidden() || pc->dead || (Races.CheckRelation(pc_i,pc)==1)))
							continue;
						if (pc->isNpc() &&(pc->npcaitype == 2 || pc->npcaitype == 1 || (Races.CheckRelation(pc_i,pc)==1)))
							continue;
						if (server_data.monsters_vs_animals == 0 &&(strlen(pc->title) <= 0 && !pc->isHuman()))
							continue;
						if (server_data.monsters_vs_animals == 1 && chance > server_data.animals_attack_chance)
							continue;
						if (pc_i->baseskill[MAGERY]>400)
						{
							if (pc_i->hp <(pc_i->st/2))
							{
								npctalkall(pc_i, "In Vas Mani", 0);
								Magic->NPCHeal(DEREF_P_CHAR(pc_i));
							}
							if (pc_i->poisoned)
							{
								npctalkall(pc_i, "An Nox", 0);
								Magic->NPCCure(DEREF_P_CHAR(pc_i));
							}
							if (pc->priv2&0x20)
							{
								npctalkall(pc_i, "An Ort", 0);
								Magic->NPCDispel(DEREF_P_CHAR(pc_i), DEREF_P_CHAR(pc));
							}
						}
						npcattacktarget(DEREF_P_CHAR(pc_i), DEREF_P_CHAR(pc));
						return;
					}
				}
			}
			break;
		case 3 : // Evil Healers
			if (!pc_i->war)
			{
				cRegion::RegionIterator4Chars ri(pc_i->pos);
				for (ri.Begin(); ri.GetData() != ri.End(); ri++)
				{
					P_CHAR pc = ri.GetData();
					if (pc != NULL)
					{
						onl = online(pc);
						d = chardist( pc_i, pc );
						if (!pc->dead || d > 3 || pc->isNpc() || !onl)
							continue;
						if (pc->isInnocent())
						{
							npctalkall(pc_i, "I dispise all things good. I shall not give thee another chance!", 1);
							return;
						}
						else
						{
							npcaction(DEREF_P_CHAR(pc_i), 0x10);
							Targ->NpcResurrectTarget(pc);
							staticeffect(pc, 0x37, 0x09, 0x09, 0x19); // Flamestrike effect
							switch (RandomNum(0, 4)) 
							{
							case 0: npctalkall(pc_i, "Fellow minion of Mondain, Live!!", 1);											break;
							case 1: npctalkall(pc_i, "Thou has evil flowing through your vains, so I will bring you back to life.", 1);	break;
							case 2: npctalkall(pc_i, "If I res thee, promise to raise more hell!.", 1);									break;
							case 3: npctalkall(pc_i, "From hell to Britannia, come alive!.", 1);										break;
							case 4: npctalkall(pc_i, "Since you are Evil, I will bring you back to consciouness.", 1);					break;
							}
						}
					}
				}
			}
			break;
		case 4 : // Guards
			if (!pc_i->war && pc_i->inGuardedArea())	// this region is guarded
			{
				cRegion::RegionIterator4Chars ri(pc_i->pos);
				for (ri.Begin(); ri.GetData() != ri.End(); ri++)
				{
					P_CHAR pc = ri.GetData();
					if (pc != NULL)
					{
						onl = online(pc);
						d = chardist( pc_i, pc );
						if (d > 10 || pc->isInvul() || pc->dead || !onl || pc->isHidden())
							continue;
						if(pc->isCriminal() || pc->isMurderer())
						{
							npcattacktarget(DEREF_P_CHAR(pc_i), DEREF_P_CHAR(pc));
							npctalkall(pc_i, "Thou shalt regret thine actions, swine!", 1); // ANTISPAM !!! LB
						}
					}
				}
			}
			break;
		case 5: // npc beggars
			if (!pc_i->war)
			{
				cRegion::RegionIterator4Chars ri(pc_i->pos);
				for (ri.Begin(); ri.GetData() != ri.End(); ri++)
				{
					P_CHAR pc = ri.GetData();
					if (pc != NULL)
					{
						onl = online(pc);
						d = chardist( pc_i, pc );
						if (d > 3 || pc->isInvul() || pc->isNpc() || pc->dead || !onl || !pc->isInnocent())
							continue;
						int beg= RandomNum(0, 2);
						{
							switch (beg)
							{
							case 0: 
								npctalkall(pc_i, "Could thou spare a few coins?", 1);
								pc_i->antispamtimer = uiCurrentTime + MY_CLOCKS_PER_SEC*30;
								break;
							case 1: 
								npctalkall(pc_i, "Hey buddy can you spare some gold?", 1);
								pc_i->antispamtimer = uiCurrentTime + MY_CLOCKS_PER_SEC*30;
								break;
							case 2: 
								npctalkall(pc_i, "I have a family to feed, think of the children.", 1);
								pc_i->antispamtimer = uiCurrentTime + MY_CLOCKS_PER_SEC*30;
								break;
							default:
								break;
							}
						}
					}
				}
			}
			break;
		case 6: 
			break; // Ripper- chaos guards.
		case 7: 
			break; // Ripper- order guards.
		case 8: 
			break; // morrolan - old banker
		case 9 : // in world guards, they dont teleport out...Ripper
			if (!pc_i->war	// guard isnt busy 
				&& pc_i->inGuardedArea())	// this region is guarded
			{	// this bracket just to keep compiler happy
				cRegion::RegionIterator4Chars ri(pc_i->pos);
				for (ri.Begin(); ri.GetData() != ri.End(); ri++)
				{
					P_CHAR pc = ri.GetData();
					if (pc != NULL)
					{
						onl = online(pc);
						d = chardist( pc_i, pc);
						if (pc_i == pc || d > 3 || pc->isInvul() || pc->dead || !onl)
							continue;
						if ((pc->isPlayer() && pc->crimflag > 0) || (Races.CheckRelation(pc,pc_i)!=1))
						{
							sprintf((char*)temp, "You better watch your step %s, I am watching thee!!", pc->name);
							npctalkall(pc_i, (char*)temp, 1);
							pc_i->antispamtimer = uiCurrentTime + MY_CLOCKS_PER_SEC*30;
						}
						else if (pc->isPlayer() && pc->isInnocent() && !pc->dead)
						{
							sprintf((char*)temp, "%s is an upstanding citizen, I will protect thee in %s.", pc->name, region[pc->region].name);
							npctalkall(pc_i, (char*)temp, 1);
							pc_i->antispamtimer = uiCurrentTime + MY_CLOCKS_PER_SEC*30;
						}
						else if (d <= 10 &&(
							(pc->isNpc() &&(pc->npcaitype == 2))	// evil npc
							||(pc->isPlayer() && !(pc->isInnocent()) && !(pc->isCriminal()))	// a player, not grey or blue
							||(pc->attackfirst == 1)))	// any agressor
						{
							pc_i->pos.x = pc->pos.x; // Ripper..guards teleport to enemies.
							pc_i->pos.y = pc->pos.y;
							pc_i->pos.z = pc->pos.z;
							soundeffect2(pc_i, 0x01FE); // crashfix, LB
							staticeffect(pc_i, 0x37, 0x2A, 0x09, 0x06);
							npcattacktarget(DEREF_P_CHAR(pc_i), DEREF_P_CHAR(pc));
							npctalkall(pc_i, "Thou shalt regret thine actions, swine!", 1); // ANTISPAM !!! LB
							return;
						}
					}
				}
			}
			break;
		case 10: // Tamed Dragons ..not white wyrm..Ripper
			// so regular dragons attack reds on sight while tamed.
			if (pc_i->isNpc() && pc_i->tamed)
			{
				cRegion::RegionIterator4Chars ri(pc_i->pos);
				for (ri.Begin(); ri.GetData() != ri.End(); ri++)
				{
					P_CHAR pc = ri.GetData();
					if (pc != NULL)
					{
						onl = online(pc);
						d = chardist( pc_i, pc );
						if (d > 10 || pc->isPlayer() || pc->npcaitype != 61)
							continue;
						npcattacktarget(DEREF_P_CHAR(pc_i), DEREF_P_CHAR(pc));
						return;
					}
				}
			}
			break;
		case 11 : // add NPCAI B in scripts to make them attack reds. (Ripper)
			if (!pc_i->war)
			{
				cRegion::RegionIterator4Chars ri(pc_i->pos);
				for (ri.Begin(); ri.GetData() != ri.End(); ri++)
				{
					P_CHAR pc = ri.GetData();
					if (pc != NULL)
					{
						onl = online(pc);
						d = chardist( pc_i, pc );
						if (d > 10 || pc->isInvul() || pc->dead)
							continue;
						if (!(pc->npcaitype == 2 || pc->isCriminal() || pc->isMurderer()))
							continue;
						if (pc->isPlayer() && !onl)
							continue;
						npcattacktarget(DEREF_P_CHAR(pc_i), DEREF_P_CHAR(pc));
					}
				}
			}
			break;
		case 17: 
			break; // Zippy Player Vendors.
		case 18: // Ripper.. Escort speech.
		if (!pc_i->war && pc_i->questType == ESCORTQUEST)
		{
			cRegion::RegionIterator4Chars ri(pc_i->pos);
			for (ri.Begin(); ri.GetData() != ri.End(); ri++)
			{
				P_CHAR pc = ri.GetData();
				if (pc != NULL)
				{
				    onl = online(pc);
				    d = chardist(pc_i, pc);
				    if (d > 10 || pc->isNpc() || pc->isInvul() || pc->dead || (pc->isPlayer() && !onl))
					    continue;

				    sprintf((char*)temp,"I am waiting for my escort to %s, Will you take me?", region[pc_i->questDestRegion].name);
				    npctalkall(pc_i,(char*)temp,1);
				    pc_i->antispamtimer=uiCurrentTime+MY_CLOCKS_PER_SEC*30;
				    return;
				}
			}
		}
		break;
		case 30: // why is this the same as case 50???..Ripper
			if (!pc_i->war)
			{
				cRegion::RegionIterator4Chars ri(pc_i->pos);
				for (ri.Begin(); ri.GetData() != ri.End(); ri++)
				{
					P_CHAR pc = ri.GetData();
					if (pc != NULL)
					{
						onl = online(pc);
						d = chardist(pc_i, pc);
						if (d > 10 || pc->isInvul() || pc->dead || pc->npcaitype != 2 || !onl)
							continue;
						npcattacktarget(DEREF_P_CHAR(pc_i), DEREF_P_CHAR(pc));
						return;
					}
				}
			}
			break;
		case 32: // Pets Guarding..Ripper
			if (pc_i->isNpc() && pc_i->tamed)
			{
				cRegion::RegionIterator4Chars ri(pc_i->pos);
				for (ri.Begin(); ri.GetData() != ri.End(); ri++)
				{
					P_CHAR pc = ri.GetData();
					if (pc != NULL)
					{
						onl = online(pc);
						d = chardist(pc_i, pc);
						if (d > 10 || pc->isNpc() || pc->dead || !pc->guarded || !onl)
							continue;
						if (pc->Owns(pc_i))
						{
							P_CHAR pc_target = FindCharBySerial(pc->attacker);
							npcattacktarget(DEREF_P_CHAR(pc_i), DEREF_P_CHAR(pc_target));
							return;
						}
					}
				}
			}
			break;
		case 50:// EV/BS
			if (!pc_i->war)
			{
				cRegion::RegionIterator4Chars ri(pc_i->pos);
				for (ri.Begin(); ri.GetData() != ri.End(); ri++)
				{
					P_CHAR pc = ri.GetData();
					if (pc != NULL)
					{
						onl = online(pc);
						d = chardist(pc_i, pc);
						if (d > 10 || pc->isInvul() || pc->dead || !onl)
							continue;
						if (Races.CheckRelation(pc,pc_i)==1)
							continue;
						npcattacktarget(DEREF_P_CHAR(pc_i), DEREF_P_CHAR(pc));
						return;
					}
				}
			}
			break;
			// Case 60-70 is Skyfires new AI
		case 96:
		case 60: // Skyfire - Dragon AI
			DragonAI->DoAI(DEREF_P_CHAR(pc_i), currenttime);
			break;
		case 97:
		case 61:// Skyfire - Banker AI
			break;
		default:
			clConsole.send("ERROR: cCharStuff::CheckAI-> Error npc (%x %x %x %x) has invalid AI type %i\n", pc_i->ser1, pc_i->ser2, pc_i->ser3, pc_i->ser4, pc_i->npcaitype); // Morrolan
			return;
	}// switch
}// void checknpcai



void cCharStuff::cDragonAI::DoAI(int i, int currenttime)
{
	int randvalue;
	int distance;
	P_CHAR pc_i = MAKE_CHARREF_LR(i);
	if (pc_i->war)
	{
		npctalkall(pc_i, "Who dares disturbe me?!?!", 1);
		cRegion::RegionIterator4Chars ri(pc_i->pos);
		for (ri.Begin(); ri.GetData() != ri.End(); ri++)
		{
			P_CHAR pc = ri.GetData();
			if (pc != NULL)
			{
				distance = chardist(pc_i, pc);
				if (!pc->npc && !online(pc))	// no offline players (Duke)
					continue;
				if (Races.CheckRelation(pc,pc_i)==1)
							continue;
				if (!(pc->dead))
				{
					if (distance>4)
					{
						randvalue = RandomNum(0, 4);
						switch (randvalue)
						{
							case 1:	
								Breath(i, currenttime);
								break;
							case 3:	
								HarmMagic(i, currenttime, pc);
								break;
							case 4:	
								HealMagic(i, currenttime);
								break;
						}
					}
					else
						HarmMagic(i, currenttime, pc);
				}
				HealMagic(i, currenttime);
			}
		}
	}
	else
		HealMagic(i, currenttime);
	return;
}

void cCharStuff::cDragonAI::Breath(int i, int currenttime)
{
	P_CHAR pc_i = MAKE_CHARREF_LR(i);
	Magic->PFireballTarget(i, DEREF_P_CHAR(FindCharBySerial(pc_i->targ)), 20);
	DoneAI(i, currenttime);
	return; 
}

void cCharStuff::cDragonAI::HarmMagic(int i, int currenttime, P_CHAR pc)
{
	P_CHAR pc_i = MAKE_CHARREF_LR(i);
	if (currenttime >= pc_i->spatimer)
	{
		switch (RandomNum(0, 5))
		{
			case 0:	
				Magic->NPCEBoltTarget(i, DEREF_P_CHAR(pc));
				break;
			case 1:	
				Magic->NPCFlameStrikeTarget(i, DEREF_P_CHAR(pc));
				break;
			case 2:	
				Magic->ParalyzeSpell(i, DEREF_P_CHAR(pc));
				break;
			case 3:	
				Magic->NPCLightningTarget(i, DEREF_P_CHAR(pc));
				break;
			case 4:	
				Magic->ParalyzeSpell(i, DEREF_P_CHAR(pc));
				break;
			case 5: 
				if (pc->priv2&0x20)
				{
					Magic->NPCDispel(i, DEREF_P_CHAR(pc));
				} 
				break;
		}
	}
	DoneAI(i, currenttime);
	return;
}

void cCharStuff::cDragonAI::HealMagic(int i, int currenttime)
{
	P_CHAR pc_i = MAKE_CHARREF_LR(i);
	if (currenttime >= pc_i->spatimer)
	{
		if (pc_i->poisoned)
		{
			Magic->NPCCure(i);
		}
		else if (pc_i->hp < (pc_i->st/2))
		{
			Magic->NPCHeal(i);
		}
		if (pc_i->targ != INVALID_SERIAL)
			npcattacktarget(i, DEREF_P_CHAR(FindCharBySerial(pc_i->targ)));
	}
	DoneAI(i, currenttime);
}
void cCharStuff::cDragonAI::DoneAI(int i, int currenttime)
{
	P_CHAR pc_i = MAKE_CHARREF_LR(i);
	pc_i->spatimer = currenttime + (pc_i->spadelay*MY_CLOCKS_PER_SEC); 
	return;
}

bool cCharStuff::cBankerAI::DoAI(int c, int i, char *comm)
{
	P_CHAR pc_currchar = currchar[c];
	char search1[50], search2[50], search3[50], search4[50];
	char *response1 = 0;
	char *response2 = 0;
	char *response3 = 0;
	char *response4 = 0;
	strcpy(search1, "BANK");
    strcpy(search2, "BALANCE");
	strcpy(search3, "WITHDRAW");
	strcpy(search4, "CHECK");
    response1 = (strstr(comm, search1));
    response2 = (strstr(comm, search2));
	response3 = (strstr(comm, search3));
	response4 = (strstr(comm, search4));
	if (SrvParms->usespecialbank)
	{
		strcpy(search1, SrvParms->specialbanktrigger);
		response1 = (strstr(comm, search1));
		if (response1 &&(!(pc_currchar->dead)))
		{
			openspecialbank(c, DEREF_P_CHAR(currchar[c]));
		}
	}
    else if (response1 &&(!(pc_currchar->dead)))
	{
		OpenBank(c);
		return true;
	}
    else if (response2 &&(!(pc_currchar->dead)))
	{
		return Balance(c, i);
	}
	else if (response3 &&(!(pc_currchar->dead)))
	{
		return Withdraw(c, i, comm);
	}
	else if (response4 &&(!(pc_currchar->dead)))
	{
		return BankCheck(c, i, comm);
	}
	return true;
}

void cCharStuff::cBankerAI::OpenBank(int c)
{
	openbank(c, DEREF_P_CHAR(currchar[c]));
	return;
}

bool cCharStuff::cBankerAI::Balance(int c, int i)
{
	P_CHAR pc_currchar = currchar[c];
	sprintf(temp, "%s's balance as of now is %i.", pc_currchar->name, pc_currchar->CountBankGold());
	npctalk(c, i, temp, 1);
	return true;
}

bool cCharStuff::cBankerAI::Withdraw(int c, int i, char *comm)
{
	P_CHAR pc_currchar = currchar[c];
	int a = 0;
	char value1[50]={' '};
	char value2[50]={' '};
	value1[0] = 0;
	value2[0] = 0;
	while (comm[a] != 0 && comm[a] != ' ' && a < 50)
	{
		a++;
	}
	strncpy(value1, temp, a);
	value1[a] = 0;
	if (value1[0] != '[' && comm[a] != 0)
		strcpy(value2, comm + a + 1);
	if (pc_currchar->CountBankGold() >= str2num(value2))
	{
		int goldcount = str2num(value2);
		addgold(c, goldcount);
		goldsfx(c, goldcount);
		DeleBankItem(DEREF_P_CHAR(pc_currchar), 0x0EED, 0, goldcount);
		sprintf(temp, "%s here is your withdraw of %i.", pc_currchar->name, goldcount);
		npctalk(c, i, temp, 1);
		return true;
	}
	else
		sprintf(temp, "%s you have insufficent funds!", pc_currchar->name);
	npctalk(c, i, temp, 1);
	return true;
}

bool cCharStuff::cBankerAI::BankCheck(int c, int i, char *comm)
{
	P_CHAR pc_currchar = currchar[c];
	int a = 0;
	char value1[50]={' '};
	char value2[50]={' '};
	value1[0] = 0;
	value2[0] = 0;
	while (comm[a] != 0 && comm[a] != ' ' && a < 50)
	{
		a++;
	}
	strncpy(value1, temp, a);
	value1[a] = 0;
	if (value1[0] != '[' && comm[a] != 0)
		strcpy(value2, comm + a + 1);
	int d = pc_currchar->CountBankGold();
	{
		int goldcount = str2num(value2);
		if (goldcount < 5000 || goldcount > 1000000)
		{
			sprintf(temp, "%s you can only get checks worth 5000gp to 1000000gp.", pc_currchar->name);
			npctalk(c, i, temp, 1);
			return false;
		}
		if (d >= goldcount)
		{
			const P_ITEM pi = Items->SpawnItem(c, pc_currchar, 1, "bank check", 0, 0x14, 0xF0, 0, 0, 0, 0); // bank check
			if (pi != NULL)
			pi->type = 1000;
			pi->setId(0x14F0);
			pi->color1 = 0x00;
			pi->color2 = 0x99;
			pi->priv |= 0x02;
			pi->value = goldcount;
			DeleBankItem(DEREF_P_CHAR(pc_currchar), 0x0EED, 0, goldcount);
			P_ITEM bankbox = pc_currchar->GetBankBox();
			bankbox->AddItem(pi);
			statwindow(c, pc_currchar);
			sprintf(temp, "%s your check has been placed in your bankbox, it is worth %i.", pc_currchar->name, goldcount);
			npctalk(c, i, temp, 1);
			return true;
		}
		else
			sprintf(temp, "%s you have insufficent funds!", pc_currchar->name);
		npctalk(c, i, temp, 1);
		return true;
	}
}
