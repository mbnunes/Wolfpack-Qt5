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

// *******************************************************************
// trigger.cpp (Trigger processing module)
//
// The trigger is activated here.  When the users doubleclicks on a triggered
// item or a triggered NPC, this routine will load the trigger from the
// triggers.scp (for dynamic), ntrigrs.scp (for NPCs) file or wtrigrs.scp
// (static world objects) and process the tokens for the trigger.
// NOTE: By using this source you are agreeing to all the terms of the GNU
// agreement.  Any use of this code, sniplets of this code, concepts of
// this code or ideas derived from this code requires that any program(s)
// and source containing any of the above mentioned states of the code, must
// be released in its entirety.
//
// DO NOT MODIFY THIS CODE UNLESS you are sure you know what your doing.
// The trigger system is not designed for making tokens that do just one
// thing.  Remember, if it can't be used for multiple things, it shouldnt
// be here. Do not distribute your changes unless its either by consent of
// the author(Zadius) or if the change and/or addition is by popular
// demand and it has been thouroughly tested.
// 
// Zadius 11-4-98 (linkage@xmission.com)
// *******************************************************************


#include "trigger.h"

#undef  DBGFILE
#define DBGFILE "trigger.cpp"

//////////////
// name:	handle_IADD
// purpose:	handles the IADD tag ie. adds an item
// history:	cut from both triggerwitem and triggernpcs because they were (almost) identical (duke, 8.4.2001)
// remarks: many other tags seem to be identical...
//
static void handle_IADD(UOXSOCKET const ts, int const ttype, 
						const int coloring,const unsigned char memcolor1,const unsigned char memcolor2,
						char* scpname)
{
	cline = &script2[0];
	splitline();
	unsigned int ItemNum = makenumber(0);
	unsigned int InBackpack = makenumber(1);
//	CHARACTER cc = currchar[ts];
	P_CHAR pc_currchar = MAKE_CHARREF_LR(currchar[ts]);
	
	switch (pc_currchar->dir)
	{
	case 0: 
		triggerx = pc_currchar->pos.x;
		triggery = pc_currchar->pos.y - 1;
		break;
	case 1: 
		triggerx = pc_currchar->pos.x + 1;
		triggery = pc_currchar->pos.y - 1;
		break;
	case 2: 
		triggerx = pc_currchar->pos.x + 1;
		triggery = pc_currchar->pos.y;
		break;
	case 3: 
		triggerx = pc_currchar->pos.x + 1;
		triggery = pc_currchar->pos.y + 1;
		break;
	case 4: 
		triggerx = pc_currchar->pos.x;
		triggery = pc_currchar->pos.y + 1;
		break;
	case 5: 
		triggerx = pc_currchar->pos.x - 1;
		triggery = pc_currchar->pos.y + 1;
		break;
	case 6: 
		triggerx = pc_currchar->pos.x - 1;
		triggery = pc_currchar->pos.y;
		break;
	case 7: 
		triggerx = pc_currchar->pos.x - 1;
		triggery = pc_currchar->pos.y - 1;
		break;
	default: 
		triggerx = pc_currchar->pos.x + 1;
		triggery = pc_currchar->pos.y + 1;
		break;
	}
	triggerz = pc_currchar->pos.z;

	long pos = ftell(scpfile);
	closescript();
	// first create the item on the ground
	P_ITEM pi_i = MAKE_ITEM_REF(Targ->AddMenuTarget(ts, 1, str2num(script2)));
	triggerx = 0;
	P_ITEM pPack = Packitem(pc_currchar);
	
	if (pPack != NULL && pi_i != NULL)
	{
		// if player has a backpack
		if (InBackpack)
		{
			// and item has to be added in player's backpack
			pPack->AddItem(pi_i);
			Weight->NewCalc(DEREF_P_CHAR(pc_currchar));
			statwindow(ts, DEREF_P_CHAR(pc_currchar));
		}
	}// if player has backpack
	// else leave it where it is (on the ground)
	// Added colormem token here! by Magius(CHE) §
	if (pi_i != NULL && coloring>-1)
	{
		pi_i->color1 = memcolor1;
		pi_i->color2 = memcolor2;
		RefreshItem(pi_i);
	}
	// end addons
	if (ttype)
		openscript(scpname);
	else 
		openscript("wtrigrs.scp");
	fseek(scpfile, pos, SEEK_SET);
	strcpy((char*)script1, "DUMMY");
}


// This routine processes both Static and Dynamic items
// ts: socket
void triggerwitem(UOXSOCKET const ts, int ti, int ttype)
{
	char sect[512], buff[512], effect[29], fmsg[512];
	char cmsg[512]; // completed trigger message
	char clr1;
	char clr2;
	int serial, serhash=-1, ci;
	unsigned int i, uiTempi, uiCompleted = 0;
	int tl;
	int j, c, r;
	P_ITEM pi_itemnum  = NULL;
	P_ITEM pi_needitem = NULL;
	int npcnum=-1;
	int trig = 0;
	int x1 = 0, y1 = 0, x2 = 0, y2 = 0, z2 = 0, dx, dy;
	long int pos;
	
	// Addons by Magius(CHE)
	P_ITEM pi_evti = NULL;
	tile_st tile;
	char tempname[512], tempname2[512], tempname3[512], tempstr[512];
	tempname[0] = 0;
	tempname2[0] = 0;
	tempname3[0] = 0;
	tempstr[0] = 0;
	cmsg[0] = 0;
	float total; // Magius(CHE) §
	char dismsg[512]; // Magius(CHE) §
	dismsg[0] = 0; // Magius(CHE) §
	unsigned char memcolor1, memcolor2; // Magius(CHE) §
	int coloring=-1, loopexit = 0;  // Magius(CHE) §
	// end declaretion for magius
	
	fmsg[0] = 0;
	
	if (ts < 0)
		return; // §Magius crash fix
	
	P_ITEM pi = NULL;	// on error return
	if (ti!=-1)
		pi= MAKE_ITEMREF_LR(ti);

	P_CHAR pc_ts = MAKE_CHARREF_LR(currchar[ts]);

	if (ttype == 1)
	{
		if (pc_ts->targtrig)
			trig = pc_ts->targtrig;
		else
			if (ti>-1)
				trig = pi->trigger;
			pc_ts->targtrig = 0;
			openscript("triggers.scp");
			sprintf(sect, "TRG %i", trig);
			if (!i_scripts[trigger_script]->find(sect))
			{
				closescript();
				return;
			}
	}
	else
	{
		openscript("wtrigrs.scp");
		sprintf(buff, "x%x%x", buffer[ts][0x11], buffer[ts][0x12]);
		if (!i_scripts[wtrigger_script]->isin(buff))
		{
			strcpy(sect, "You cant think of a way to use that.");
			sysmessage(ts, sect);
			closescript();
			return;
		}
	}
	
	// Get Temporany Name of the Item (Magius(CHE))
	if (ti>-1) // crashfix LB
	{
		if (pi->name[0] != '#')
			strcpy(tempname, pi->name);
		else
		{
			Map->SeekTile(pi->id(), &tile);
			strcpy(tempname,(char*)tile.name);
		}
		
		if (pi->disabled>uiCurrentTime) // Added by Magius(CHE) §
		{
			// if (!pi->disabledmsg[0]==0x0) strcpy(temp,pi->disabledmsg);
			if (strlen(pi->disabledmsg)>0)
				strcpy((char*)temp, pi->disabledmsg);// AntiChrist
			else 
				strcpy(temp, "That doesnt seem to work right now.");
			sysmessage(ts, (char*)temp);
			closescript();
			return;
		}
	}
	// End addons by magius
	
	do
	{
		read2();
		if (script1[0] != '}')
		{
			switch (script1[0])
			{
				case 'A':
					if (!(strcmp("ACT", (char*)script1)))  // Make player perform an action
					{
						cline = &script2[0];
						splitline();
						int act = hexnumber(0);// AntiChrist
						// AntiChrist - auto-convert code of animation
						// that automatically check if player is on a horse
						switch (act)
						{
						case 0x1A:// Mining-Gravedigging
						case 0x0B:
							if (pc_ts->onhorse)
								act = 0x1A;
							else 
								act = 0x0b;
							break;
						case 0x1C:// LumberJacking-Bowcraft
						case 0x0D:
							if (pc_ts->onhorse)
								act = 0x1C;
							else 
								act = 0x0D;
							break;
						case 0x1D:// Swordtarget
							// case 0x0D:
							if (pc_ts->onhorse)
								act = 0x1D;
							else 
								act = 0x0D;
							break;
						case 0x0A:// Fist Fighting
							if (pc_ts->onhorse)
								act = 0x1A;
							else 
								act = 0x0A;
							break;
						case 0x0E:// Smelting irons
							if (pc_ts->onhorse)
								act = 0x1C;
							else 
								act = 0x0E;
							break;
						case 0x09:// Working ingots
							if (pc_ts->onhorse)
								act = 0x1A;
							else 
								act = 0x09;
							break;
						case 0x14:// These can be done only if not onhorse
						case 0x22:
							if (pc_ts->onhorse)
								act = 0x00;
							break;
						default:
							break;
						}
						if (act>0)
							action(ts, act);
					}
					break;
				case 'C':
					if (!(strcmp("CADD", (char*)script1)))  // add item into triggered container
					{
						P_ITEM pi_temp = MAKE_ITEM_REF(Targ->AddMenuTarget(ts, 1, str2num(script2)));
						pi->AddItem(pi_temp);
						Weight->NewCalc(DEREF_P_CHAR(pc_ts));
						statwindow(ts, DEREF_P_CHAR(pc_ts));
					}
					else if (!(strcmp("CMSG", (char*)script1)))  // Set Token Completed message
					{
						if (uiCompleted)
						{
							strcpy(cmsg, script2);
							if (strlen(cmsg))
								sysmessage(ts, cmsg);
						}
					}
					else if (!(strcmp("COLOR", (char*)script1)))  // Set the color check by Magius(CHE) §
					{
						cline = &script2[0];
						splitline();
						clr1 = hexnumber(0);
						clr2 = hexnumber(1);
						j = makenumber(2);
						if (ti>-1)
						{// AntiChrist
							if (clr1 != pi->color1 || clr2 != pi->color2)
							{
								if (strlen(fmsg))
									sysmessage(ts, fmsg);
								else
								{
									sprintf((char*)temp, "You need another type of %s!", tempname);
									sysmessage(ts, (char*)temp);
								}
								closescript();
								return;
							}
							if (j>0)
							{
								pc_ts->targtrig = j;
								closescript();
								triggerwitem(ts, ti, 1);
								return;
							}
						}
					}
					else if (!(strcmp("CALCUSES", (char*)script1)))  // Calculate an items uses
					{
						if (ti>-1)
						{
							if (pi->tuses)
							{
								pi->tuses += str2num(script2);
								closescript();
								return;
							}
						}
					}
					else if (!(strcmp("CSKL", (char*)script1)))  // Make a check on the players skill
					{
						// Magius(CHE) §
						//*comm[1]='\0';
						cline = &script2[0];
						splitline();
						int p = makenumber(0);
						j = makenumber(1);
						// End Magius(CHE) §
						i = (rand()%1000) + 1;
						// Taur 69.02 added to get some chance of
						// skill gain on failure
						unsigned int skill = p; // Magius(CHE) §
						unsigned int baseskill = pc_ts->baseskill[skill];
						if (i > baseskill)
						{
							Skills->AdvanceSkill(currchar[ts], skill, 0);
							Skills->updateSkillLevel(currchar[ts], skill);
							updateskill(ts, skill);
							if (strlen(fmsg))
								sysmessage(ts, fmsg); // by Magius(CHE) §
							else 
							{
								sprintf((char*)temp, "You fail in your attempt..."); // by Magius(CHE) §
								sysmessage(ts, (char*)temp); // by Magius(CHE) §
							}
							closescript();
							return;
						} // Taur end 69.02 change
						if (j>0)
						{
							// Magius(CHE) §
							pc_ts->targtrig = j;
							closescript();
							triggerwitem(ts, ti, 1);
							return;
						}
					}
					break;
				case 'D':
					if (!(strcmp("DEX", (char*)script1)))  // Do math on players dexterity
					{
						j = str2num(script2);
						if (j < 0)
						{
							sysmessage(ts, "You are getting clumsier!");
						}
						else 
						{
							sysmessage(ts, "You are getting more agile!");
						}
						pc_ts->chgRealDex(j);
					}
					else if (!(strcmp("DUR", (char*)script1)))  // Math on item HP ---- rewrite by Magius(CHE) §
					{
						cline = &script2[0];
						splitline();
						j = makenumber(0);
						int p = makenumber(1);
						if (p <= 0)
							p = 100;
						if (ti>-1)
						{
							c = pi->hp;
							if (pi->maxhp>0)
							{
								if ((rand()%(100)) + 1 <= p)
								{
									if ((c >= pi->maxhp) &&(j>0))
									{
										sprintf(tempstr, "Your %s is already totally repaired!", tempname);
										sysmessage(ts, tempstr);
									}
									pi->hp += j;
									if (pi->hp >= pi->maxhp)
										pi->hp = pi->maxhp;
									if (pi->hp - c>0)
									{
										if (strlen(cmsg))
											sysmessage(ts, cmsg);
										else 
										{
											total = (float) pi->hp/pi->maxhp; // Magius(CHE) §
											sprintf(tempstr, "Your %s is now repaired! [%.1f%%]", tempname, total*100);  // Magius(CHE) §
											sysmessage(ts, tempstr);
										}
									}
									else if (pi->hp - c < 0)
									{
										if (strlen(fmsg))
											sysmessage(ts, fmsg);
										else 
										{
											sprintf(tempstr, "Your %s appears to be more ruined than before!", tempname);
											sysmessage(ts, tempstr);
										}
									}
									if (pi->hp <= 0)
									{
										sprintf(tempstr, "Your %s was too old and it has been destroyed!", tempname);
										sysmessage(ts, tempstr);
										if (pi->amount>1)
											pi->amount--;
										else // -Frazurbluu- may need check here for new trigger if can be done while equipped
											Items->DeleItem(ti);
									}
								}
							}
							else 
							{
								sprintf(tempstr, "You can't repair %s !", tempname);
								sysmessage(ts, tempstr);
							}
						}
					}
					else if (!(strcmp("DISABLE", (char*)script1)))  // Disable Item for n seconds
					{
						if (ti>-1)
						{
							pi->disabled = (uiCurrentTime +(MY_CLOCKS_PER_SEC*str2num(script2)));
							strcpy(pi->disabledmsg, dismsg); // Added by Magius(CHE) §
						}
					}
					else if (!(strcmp("DISABLEMSG", (char*)script1)))  // Disable Item Message --- by Magius(CHE) §
					{
						strcpy(dismsg, (char*)script2);
					}
					break;
				case 'E':
					if (!(strcmp("EVDUR", (char*)script1)))  // Math on Evoked item HP ---- Rewrite by Magius(CHE) §
					{
						cline = &script2[0];
						splitline();
						j = makenumber(0);
						int p = makenumber(1);
						if (p <= 0)
							p = 100;
						if (pi_evti != NULL)
						{
							c = pi_evti->hp;
							if (pi_evti->maxhp>0)
							{
								if ((rand()%(100)) + 1 <= p)
								{
									if ((c >= pi_evti->maxhp) &&(j>0))
									{
										sprintf(tempstr, "Your %s is already totally repaired!", tempname2);
										sysmessage(ts, tempstr);
									}
									pi_evti->hp += j;
									if (pi_evti->hp >= pi_evti->maxhp)
										pi_evti->hp = pi_evti->maxhp;
									if (pi_evti->hp - c>0)
									{
										if (strlen(cmsg))
											sysmessage(ts, cmsg);
										else 
										{
											total = (float) pi_evti->hp/pi_evti->maxhp;
											sprintf(tempstr, "Your %s is now repaired! [%.1f%%]", tempname2, total*100);
											sysmessage(ts, tempstr);
										}
									}
									else if (pi_evti->hp - c < 0)
									{
										if (strlen(fmsg))
											sysmessage(ts, fmsg);
										else 
										{
											sprintf(tempstr, "Your %s appears to be more ruined than before!", tempname2);
											sysmessage(ts, tempstr);
										}
									}
									if (pi_evti->hp <= 0)
									{
										sprintf(tempstr, "Your %s was too old and it has been destroyed!", tempname2);
										sysmessage(ts, tempstr);
										if (pi_evti->amount>1)
											pi_evti->amount--;
										else 
											Items->DeleItem(DEREF_P_ITEM(pi_evti));
									}
								}
							}
							else 
							{
								sprintf(tempstr, "You can't repair %s !", tempname2);
								sysmessage(ts, tempstr);
							}
						}
					}
					
					else if (!(strcmp("EVMAXDUR", (char*)script1)))  // Math on Evoked item MAXHP ---- Rewrite by Magius(CHE) §
					{
						cline = &script2[0];
						splitline();
						j = makenumber(0);
						int p = makenumber(1);
						if (p <= 0)
							p = 100;
						if (pi_evti != NULL)
						{
							if (pi_evti->maxhp>0)
							{
								if ((rand()%(100)) + 1 <= p)
								{
									pi_evti->maxhp += j; // Magius(CHE) §
									if (pi_evti->hp >= pi_evti->maxhp)
										pi_evti->hp = pi_evti->maxhp;
									if (str2num(script2) >= 0)
									{
										if (strlen(cmsg))
											sysmessage(ts, cmsg);
										else 
										{
											sprintf(tempstr, "You increased the maximum durability of your %s !", tempname2);
											sysmessage(ts, tempstr);
										}
									}
									else 
									{
										if (strlen(fmsg))
											sysmessage(ts, fmsg);
										else 
										{
											sprintf(tempstr, "Your %s appears to be not resistant as before!", tempname2);
											sysmessage(ts, tempstr);
										}
									}
									if (pi_evti->maxhp <= 0)
									{
										sprintf(tempstr, "Your %s was too old and it has been destroyed!", tempname2);
										sysmessage(ts, tempstr);
										if (pi_evti->amount>1)
											pi_evti->amount--;
										else 
											Items->DeleItem(DEREF_P_ITEM(pi_evti));
									}
								}
							}
							else 
							{
								sprintf(tempstr, "You can't increase durability of %s !", tempname2);
								sysmessage(ts, tempstr);
							}
						}
					}
					else if (!(strcmp("EMT", (char*)script1)))  // Player says something when trigger is activated
					{
						strcpy(sect, script2);
						for (i = 0; i < now; i++)
						{
							if (inrange1p(currchar[ts], currchar[i]) && perm[i])
							{
								tl = 44 + strlen(sect) + 1;
								talk[1] = tl >> 8;
								talk[2] = tl%256;
								talk[3] = pc_ts->ser1;
								talk[4] = pc_ts->ser2;
								talk[5] = pc_ts->ser3;
								talk[6] = pc_ts->ser4;
								talk[7] = pc_ts->id1;
								talk[8] = pc_ts->id2;
								talk[9] = 0; // Type
								ShortToCharPtr(pc_ts->saycolor, &talk[10]);
								talk[12] = 0;
								talk[13] = pc_ts->fonttype;
								Xsend(i, talk, 14);
								Xsend(i, pc_ts->name, 30);
								Xsend(i, sect, strlen(sect) + 1);
							}
						}
					}
					else if (!(strcmp("END_TRIGGER", (char*)script1)))  // End execution of a trigger
					{
						uiTempi = str2num(script2);
						if ((uiTempi && uiCompleted) ||(!uiTempi && !uiCompleted))
						{
							closescript();
							return;
						}
					}
					break;
					case 'F':
						if (!(strcmp("FMSG", (char*)script1)))  // Set fail message
						{
							strcpy(fmsg,script2);
						}
						else if (!(strcmp("FAME", (char*)script1)))  // Do math on the players fame - Magius(CHE)
						{
							cline = &script2[0];
							splitline();
							j = makenumber(0);
							c = pc_ts->fame;
							if ((j>0) &&(c >= 10000))
								sysmessage(ts, "You are already the most glorious person in this world!");
							if ((j < 0) &&(c <=-10000))
								sysmessage(ts, "You are already the most unknown person in this world!");
							pc_ts->fame += j;
							if (pc_ts->fame>10000)
								pc_ts->fame = 10000;
							if (pc_ts->fame < -10000)
								pc_ts->fame=-10000;
							j = pc_ts->fame - c;
							if (j>0)
							{
								if (j < 25)
									sysmessage(ts, "You have gained a little fame!");
								else if (j < 75)
									sysmessage(ts, "You have gained some fame!");
								else if (j < 100)
									sysmessage(ts, "You have gained a lot of fame!");
								else 
									sysmessage(ts, "You have gained a huge amount of fame!");
							}
							else if (j < 0)
							{
								if (abs(j) < 25)
									sysmessage(ts, "You have lost a little fame!");
								else if (abs(j) < 75)
									sysmessage(ts, "You have lost some fame!");
								else if (abs(j) < 100)
									sysmessage(ts, "You have lost a lot of fame!");
								else 
									sysmessage(ts, "You have lost a huge amount of fame!");
							}
							// refresh title
						}
						break;
					case 'H':
						if (!(strcmp("HUNGER", (char*)script1)))  // Do math on players hunger from 0 to 6 - Magius(CHE)
						{
							j = str2num(script2);
							pc_ts->hunger += j;
							if (pc_ts->hunger>6)
								pc_ts->hunger = 6;
							if (pc_ts->hunger < 1)
								pc_ts->hunger = 1;
							switch (pc_ts->hunger)
							{
							case 0:  sysmessage(ts, "You eat the food, but are still extremely hungry.");		break;
							case 1:  sysmessage(ts, "You eat the food, but are still extremely hungry.");		break;
							case 2:  sysmessage(ts, "After eating the food, you feel much less hungry.");		break;
							case 3:  sysmessage(ts, "You eat the food, and begin to feel more satiated.");		break;
							case 4:  sysmessage(ts, "You feel quite full after consuming the food.");			break;
							case 5:  sysmessage(ts, "You are nearly stuffed, but manage to eat the food.");		break;
							case 6:  sysmessage(ts, "You are simply too full to eat any more!");				break;
							}
						}
						else if (!(strcmp("HEA", (char*)script1)))  // Do math on players health
						{
							j = str2num(script2);
							if ((j < 0) &&(pc_ts->hp < abs(j)) &&(!(pc_ts->isInvul())))
							{
								deathstuff(currchar[ts]);
								closescript();
								return;
							}
							pc_ts->hp += j;
							if (pc_ts->hp>pc_ts->st)
								pc_ts->hp = pc_ts->st;
							updatestats(currchar[ts], 0);
						}
						break;
					case 'I':
						if (!(strcmp("IFHUNGER", (char*)script1)))  // If hunger meets a certain criteria - Magius(CHE)
						{
							j = str2num(script2);
							if (j >= 0)
							{
								if (!(pc_ts->hunger >= j))
								{
									if (strlen(fmsg))
										sysmessage(ts, fmsg);
									closescript();
									return;
								}
							}
							else 
							{
								if (!(pc_ts->hunger <= abs(j)))
								{
									if (strlen(fmsg))
										sysmessage(ts, fmsg);
									closescript();
									return;
								}
							}
						}
						else if (!(strcmp("IFKARMA", (char*)script1)))  // If karma meets a certain criteria - Magius(CHE)
						{
							char opt;
							gettokennum((char*)script2, 0);
							opt = gettokenstr[0];
							gettokennum((char*)script2, 1);
							j = str2num(gettokenstr);
							// clConsole.send("Script2: %s\nOPT: %c\n%i %c %i\n",script2,opt,chars[p].karma,opt,j);
							if (opt == '>')
							{
								if (!(pc_ts->karma >= j))
								{
									if (strlen(fmsg))
										sysmessage(ts, fmsg);
									else 
										sysmessage(ts, "You need more Karma to do that!");
									closescript();
									return;
								}
							}
							else if (opt == '<')
							{
								if (!(pc_ts->karma <= j))
								{
									if (strlen(fmsg))
										sysmessage(ts, fmsg);
									else 
										sysmessage(ts, "You need less Karma to do that!");
									closescript();
									return;
								}
							}
							else 
								clConsole.send("TRIGGER.SCP:Triggerwitem() Syntax Error in IFKARMA token.\n   IFKARMA <opt> <value>\n    <opt>= '<' or '>'\n    <value> range from '-10000' to '10000'.\n");
						}
						else if (!(strcmp("IDADD", (char*)script1)))  // Add item in front of player by ID
						{
							unsigned int low = 0, high = 0;
							cline = &script2[0];
							splitline();
							
							low = makenumber(2);
							high = (tnum == 4) ? makenumber(3) : low;
							r = RandomNum(low, high);
							// AntiChrist
							pos = ftell(scpfile);
							closescript();
							pi_itemnum = MAKE_ITEM_REF(Items->SpawnItem(ts,currchar[ts], r, "#", 1, hexnumber(0), hexnumber(1), 0, 0, 1, 1));
							if (pi_itemnum == NULL)
							{
								closescript();
								return;
							}
							// Added colormem token here! by Magius(CHE) §
							if (coloring > -1)
							{
								pi_itemnum->color1 = memcolor1;
								pi_itemnum->color2 = memcolor2;
								RefreshItem(pi_itemnum);
							}
							// end addons	
							if (ttype)
								openscript("triggers.scp");// AntiChrist
							else 
								openscript("wtrigrs.scp");
							fseek(scpfile, pos, SEEK_SET);
						}
						else if (!(strcmp("IDFX", (char*)script1)))  // Makes an effect at players by ID
						{
							for (i = 0; i < 29; i++)
							{
								effect[i] = 0;
							}
							if (ti>-1)
							{
								j = packitem(currchar[ts]);
								if (j>-1)
								{
									if (pi->contserial == items[j].serial)
									{
										closescript();
										return;
									}
									else
									{
										triggerx = pi->pos.x;
										triggery = pi->pos.y;
										triggerz = pi->pos.z;
									}
								}
							}
							else 
							{
								triggerx = (buffer[ts][11] << 8) + buffer[ts][12];
								triggery = (buffer[ts][13] << 8) + buffer[ts][14];
								triggerz = (buffer[ts][15] << 8) + buffer[ts][16];
							}
							cline = &script2[0];
							splitline();
							effect[0] = 0x70; // Effect message
							effect[1] = 0x00; // Moving effect
							effect[2] = pc_ts->ser1;
							effect[3] = pc_ts->ser2;
							effect[4] = pc_ts->ser3;
							effect[5] = pc_ts->ser4;
							//        effect[6]=pc_ts->ser1;
							//        effect[7]=pc_ts->ser2;
							//        effect[8]=pc_ts->ser3;
							//        effect[9]=pc_ts->ser4;
							effect[10] = hexnumber(0);// Object id of the effect
							effect[11] = hexnumber(1);
							effect[12] = pc_ts->pos.x >> 8;
							effect[13] = pc_ts->pos.x%256;
							effect[14] = pc_ts->pos.y >> 8;
							effect[15] = pc_ts->pos.y%256;
							effect[16] = pc_ts->pos.z;
							effect[17] = triggerx >> 8;
							effect[18] = triggerx%256;
							effect[19] = triggery >> 8;
							effect[20] = triggery%256;
							effect[21] = triggerz;
							effect[22] = 0x09;
							effect[23] = 0x06; // 0 is really long.  1 is the shortest.
							effect[24] = 0; // This value is unknown
							effect[25] = 0; // This value is unknown
							effect[26] = 1; // This value is unknown
							effect[27] = 0x01; // This value is used for moving effects that explode on impact.
							for (uiTempi = 0; uiTempi < now; uiTempi++)
							{
								if ((inrange1p(currchar[uiTempi], ts)) &&(inrange1p(currchar[uiTempi], ts)) &&(perm[uiTempi]))
								{
									Xsend(uiTempi, effect, 28);
								}
							}
							triggerx = 0;
						}
						else if (!(strcmp("INT", (char*)script1)))  // Do math on players intelligence
						{
							j = str2num(script2);
							if (j < 0)
								sysmessage(ts, "Your mind is growing weaker!");
							else 
								sysmessage(ts, "Your mind is getting stronger!");
							pc_ts->in += j;
							if (pc_ts->in < 1)
								pc_ts->in = 1;
						}
						else if (!(strcmp("IADD", (char*)script1)))  // Add item in front of player //if 2nd param. is 1, add item into player's backpack - AntiChrist (with autostack)
							handle_IADD(ts, ttype, coloring, memcolor1, memcolor2, "triggers.scp");
						else if (!(strcmp("IFOWNER", (char*)script1)))  // If item is owned by triggerer
						{
							if (ti>-1)
							{
								if (!pc_ts->Owns(pi))
								{
									sysmessage(ts, "You do not own that.");
									closescript();
									return;
								}
							}
						}
						else if (!(strcmp("IFSKL", (char*)script1)))  // If skill meets a certain criteria
						{
							cline = &script2[0];
							splitline();
							i = makenumber(0);
							j = makenumber(1);
							if (j >= 0)
							{
								if (!(pc_ts->baseskill[i] >= j))
								{
									if (strlen(fmsg))
										sysmessage(ts, fmsg);
									closescript();
									return;
								}
							}
							else 
							{
								if (!(pc_ts->baseskill[i] <= abs(j)))
								{
									if (strlen(fmsg))
										sysmessage(ts, fmsg);
									closescript();
									return;
								}
							}
						}
						else if (!(strcmp("IFFAME", (char*)script1)))  // If karma meets a certain criteria - Magius(CHE)
						{
							char opt;
							gettokennum((char*)script2, 0);
							opt = gettokenstr[0];
							gettokennum((char*)script2, 1);
							j = str2num(gettokenstr);
							if (opt == '>')
							{
								if (!(pc_ts->fame >= j))
								{
									if (strlen(fmsg))
										sysmessage(ts, fmsg);
									else 
										sysmessage(ts, "You need more Fame to do that!");
									closescript();
									return;
								}
							}
							else if (opt == '<')
							{
								if (!(pc_ts->fame <= abs(j)))
								{
									if (strlen(fmsg))
										sysmessage(ts, fmsg);
									else 
										sysmessage(ts, "You need less Fame to do that!");
									closescript();
									return;
								}
							}
							else 
								clConsole.send("TRIGGER.SCP:Triggerwitem() Syntax Error in IFFAME token.\n   IFFAME <opt> <value>\n    <opt>= '<' or '>'\n    <value> range from '-10000' to '10000'.\n");
						}
						else if (!strcmp("IFREQ", (char*)script1)) // Check Envoked ID, if true JUMP to another trigger (in trigger.scp)
						{
							cline = &script2[0];
							splitline();
							j = makenumber(1);
							sprintf(sect, "x%x%x", pc_ts->envokeid1, pc_ts->envokeid2);
							if (strstr((char*)comm[0], sect))
							{
								// IEnvoked item found in IFREQ --> Jump Trigger
								// clConsole.send("IFREQ %s  -> Jumping to trigger %i\n",comm[0],j);
								pc_ts->targtrig = j;
								closescript();
								triggerwitem(ts, ti, 1);
								return;
							}
						}
						break;
					case 'K':
						if (!(strcmp("KARMA", (char*)script1)))  // Do math on the players karma - Magius(CHE)
						{
							cline = &script2[0];
							splitline();
							j = makenumber(0);
							c = pc_ts->karma;
							if ((j>0) &&(c >= 10000))
								sysmessage(ts, "You are already the most honest person in this world!");
							if ((j < 0) &&(c <=-10000))
								sysmessage(ts, "You are already the most evil person in this world!");
							pc_ts->karma += j;
							if (pc_ts->karma>10000)
								pc_ts->karma = 10000;
							if (pc_ts->karma < -10000)
								pc_ts->karma=-10000;
							j = pc_ts->karma - c;
							if (j>0)
							{
								if (j < 25)
									sysmessage(ts, "You have gained a little karma!");
								else if (j < 75)
									sysmessage(ts, "You have gained some karma!");
								else if (j < 100)
									sysmessage(ts, "You have gained a lot of karma!");
								else 
									sysmessage(ts, "You have gained a huge amount of karma!");
							}
							else if (j < 0)
							{
								if (abs(j) < 25)
									sysmessage(ts, "You have lost a little karma!");
								else if (abs(j) < 75)
									sysmessage(ts, "You have lost some karma!");
								else if (abs(j) < 100)
									sysmessage(ts, "You have lost a lot of karma!");
								else 
									sysmessage(ts, "You have lost a huge amount of karma!");
							}
						}
						break;
					case 'M':
						if (!(strcmp("MAXDUR", (char*)script1)))  // Math on item MAXHP ---- rewrite by Magius(CHE) §
						{
							cline = &script2[0];
							splitline();
							j = makenumber(0);
							int p = makenumber(1);
							if (p <= 0)
								p = 100;
							if (ti>-1)
							{
								if (pi->maxhp>0)
								{
									if ((rand()%(100)) + 1 <= p)
									{
										pi->maxhp += j;
										if (pi->hp >= pi->maxhp)
											pi->hp = pi->maxhp;
										if (str2num(script2) >= 0)
										{
											if (strlen(cmsg))
												sysmessage(ts, cmsg);
											else 
											{
												sprintf(tempstr, "You increased the maximum durability of your %s !", tempname);
												sysmessage(ts, tempstr);
											}
										}
										else 
										{
											if (strlen(fmsg))
												sysmessage(ts, fmsg);
											else 
											{
												sprintf(tempstr, "Your %s appears to be more ruined than before!", tempname);
												sysmessage(ts, tempstr);
											}
										}
										if (pi->maxhp <= 0)
										{
											sprintf(tempstr, "Your %s was too old and it has been destoryed!", tempname);
											sysmessage(ts, tempstr);
											if (pi->amount>1)
												pi->amount--;
											else 
												Items->DeleItem(ti);
										}
									}
								}
								else 
								{
									sprintf(tempstr, "You can't increase durability of %s !", tempname);
									sysmessage(ts, tempstr);
								}
							}
						}
						else if (!(strcmp("MAN", (char*)script1)))  // Do math on players mana
						{
							j = str2num(script2);
							if ((j < 0) &&(pc_ts->mn < abs(j)))
							{
								sysmessage(ts, "Your mind is too tired to do that.");
								closescript();
								return;
							}
							pc_ts->mn += j;
							if (pc_ts->mn>pc_ts->in)
								pc_ts->mn = pc_ts->in;
							updatestats(currchar[ts], 1);
						}
						else if (!(strcmp("MAKE", (char*)script1)))  // Give user the make menu
						{
							// itemmenu(ts,str2num(script2));
							cline = &script2[0];
							splitline();
							itemmake[ts].Mat1id = ((hexnumber(2)) << 8) + hexnumber(3);
							itemmake[ts].has = getamount(currchar[ts], itemmake[ts].Mat1id); 
							itemmake[ts].has2 = getamount(currchar[ts], itemmake[ts].Mat2id);
							itemmake[ts].coloring = coloring; // Magius(CHE) §
							if (coloring>-1)
							{
								itemmake[ts].newcolor1 = memcolor1;  // Magius(CHE) §
								itemmake[ts].newcolor2 = memcolor2;  // Magius(CHE) §
							}
							pos = ftell(scpfile);
							closescript();
							Skills->MakeMenu(ts, makenumber(0), makenumber(1));
							if (ttype)
								openscript("triggers.scp");
							else 
								openscript("wtrigrs.scp");
							fseek(scpfile, pos, SEEK_SET);
							strcpy((char*)script1, "DUMMY");
						}
						else if (!(strcmp("MISC", (char*)script1)))  // Perform a miscellaneous function
						{
							if (!(strcmp("bank", strlwr((char*)script2))))
								openbank(ts, currchar[ts]);
						}
						else if (!(strcmp("MSG", (char*)script1)))  // Display a message when trigger is activated
						{
							strcpy(sect,script2);
							sysmessage(ts, sect);
						}
						else if (!(strcmp("MEMCOLOR", (char*)script1)))  // Store the item color in memory by Magius(CHE) §
						{
							if (!strcmp((char*)script2, "EMPTY"))
							{
								coloring=-1;
								closescript();
								return;
							}
							P_ITEM p = MAKE_ITEM_REF(ti);
							if (!strcmp((char*)script2, "REQ"))
								p = pi_evti;
							if (!strcmp((char*)script2, "NEED"))
								p = pi_needitem;
							if (p != NULL)
							{
								memcolor1 = p->color1;
								memcolor2 = p->color2;
								coloring = 1;
							}
							else
							{
								clConsole.send("WOLFPACK: Error in Trigger script.\n Check MEMCOLOR %s in SECTION ", script2);
								if (ttype == 1)
									clConsole.send("%i.\n", trig);
								else 
									clConsole.send("...x%x%x...\n", buffer[ts][0x11], buffer[ts][0x12]);
								clConsole.send(" Make sure that it was after %s.\n", script2);
							}
						}
						break;
					case 'N':
						if ((!(strcmp("NEWTYPE", (char*)script1))) ||(!(strcmp("SETTYPE", (char*)script1))))  // Set active item type
						{
							if (pi_itemnum != NULL)
								pi_itemnum->type = str2num(script2);
							else
							{
								if (ti>-1)
									pi->type = str2num(script2);
							}
						}
						else if (!(strcmp("NADD", (char*)script1)))  // Add a NPC at given location - AntiChrist
						{// Usage: NADD <npc_number> <life_in_seconds>
							uiCompleted = 0;
							cline = &script2[0];
							splitline();
							unsigned int uiNpcNum = makenumber(0);
//							unsigned int uiTimer = makenumber(1);
							
							if (ti>-1)
							{
								if (pi->contserial == items[pc_ts->packitem].serial)
								{
									triggerx = pc_ts->pos.x + 1;
									triggery = pc_ts->pos.y + 1;
									triggerz = pc_ts->pos.z;
								}
								else 
								{
									triggerx = pi->pos.x;
									triggery = pi->pos.y;
									triggerz = pi->pos.z;
								}
							}
							else
							{
								x2 = (buffer[ts][11] << 8) + buffer[ts][12];
								y2 = (buffer[ts][13] << 8) + buffer[ts][14];
								z2 = (buffer[ts][15] << 8) + buffer[ts][16];
								triggerx = x2;
								triggery = y2;
								triggerz = z2;
							}
							pos = ftell(scpfile);
							closescript();
							Npcs->AddRespawnNPC(ti, uiNpcNum, 1);
							// Npcs->AddRespawnNPC(ts,str2num(script2),1);
							npcnum = triggerx;
							
							/*
							if (uiTimer>0)
							{// if we have a timer
							chars[npcnum].summontimer = uiCurrentTime +(uiTimer*MY_CLOCKS_PER_SEC);
							}
							Commented out by Correa, reason: Bug fix. Should it be a new feature? If so,
							it's not parsing the second param when it doesn't exist, so uiTimer == uiNpcNum.
							*/
							
							triggerx = 0;
							if (ttype)
								openscript("triggers.scp");
							else 
								openscript("wtrigrs.scp");
							fseek(scpfile, pos, SEEK_SET);
							strcpy((char*)script1, "DUMMY");
						}
						else if (!(strcmp("NEWBIEITEM", (char*)script1)))  // Set item newbie
						{
							if (ti>-1)
							{
								items[ti].priv = items[ti].priv | 0x02;
							}
						}
						else if (!(strcmp("NEWOWNER", (char*)script1)))  // Set ownership of item
						{
							int p;
							if (pi_itemnum != NULL)
							{
								p = currchar[ts];
								pi_itemnum->SetOwnSerial(chars[p].serial);
							}
							if (npcnum>-1)
							{
								p = currchar[ts];
								chars[npcnum].SetOwnSerial(chars[p].serial);
								chars[npcnum].tamed = true;// AntiChrist FIX
							}
						}
						else if (!(strcmp("NEWSPEECH", (char*)script1)))  // Give the new npc a new spech -- MAgius(CHE) §
						{
							if (npcnum>-1)
							{
								chars[npcnum].speech = str2num(script2);
							}
						}
						else if (!(strcmp("NEED", (char*)script1)))  // The item here is required and will be removed
						{
							P_CHAR ts_currchar = MAKE_CHAR_REF(currchar[ts]);
							P_ITEM pBackpack = Packitem(ts_currchar);
							if (pBackpack != NULL)
							{
								serial = pBackpack->serial;
								vector<SERIAL> vecContainer = contsp.getData(serial);
								for (ci = 0; ci < vecContainer.size(); ci++)
								{
									P_ITEM pi = FindItemBySerial(vecContainer[ci]);
									if (pi != NULL)
									{
										sprintf(sect, "x%x%x", pi->id1, pi->id2);
										if (strstr((char*)script2, sect))
										{
											pi_needitem = pi;
											// Get Temporany Name of the NEED Item - Magius(CHE) §
											if (pi->name[0] != '#') // Get Temporany Name of the NEED Item - Magius(CHE) §
												strcpy(tempname3, pi->name);
											else 
											{
												Map->SeekTile(pi->id(), &tile);
												strcpy(tempname3, (char*)tile.name);
											}
											// End Get Temporany Name of the NEED Item - Magius(CHE) §
											break;
										}
									}
								}
							}
							
							if (pi_needitem != NULL)
							{
								if (strlen(fmsg))
									sysmessage(ts, fmsg); // Added by Magius(CHE)
								else 
									sysmessage(ts, "You don't have the neccessary supplies to do that."); // Changed by Magius(CHE)
								closescript();
								return;
							}
						}
						else if (!(strcmp("NEEDCOLOR", (char*)script1)))  // Set the color check on NEEDED item by Magius(CHE) §
						{
							cline = &script2[0];
							splitline();
							clr1 = hexnumber(0);
							clr2 = hexnumber(1);
							j = makenumber(2);
							if (pi_needitem != NULL)
							{// AntiChrist
								if (clr1 != pi_needitem->color1 || clr2 != pi_needitem->color2)
								{
									if (strlen(fmsg))
										sysmessage(ts, fmsg);
									else
									{
										sprintf((char*)temp, "You need another type of %s!", tempname3);
										sysmessage(ts, (char*)temp);
									}
									closescript();
									return;
								}
							}
							
							if (j>0)
							{
								pc_ts->targtrig = j;
								closescript();
								triggerwitem(ts, ti, 1);
								return;
							}
						}
						else if (!(strcmp("NEEDDUR", (char*)script1)))  // Math on NEED item HP (only if NEEDITEM>-1)---- rewrite by Magius(CHE) §
						{
							cline = &script2[0];
							splitline();
							j = makenumber(0);
							int p = makenumber(1);
							if (p <= 0)
								p = 100;
							if (pi_needitem != NULL)
							{
								c = pi_needitem->hp;
								if (pi_needitem->maxhp>0)
								{
									if ((rand()%(100)) + 1 <= p)
									{
										if ((c >= pi_needitem->maxhp) &&(j>0))
										{
											sprintf(tempstr, "Your %s is already totally repaired!", tempname3);
											sysmessage(ts, tempstr);
										}
										pi_needitem->hp += j;
										if (pi_needitem->hp >= pi_needitem->maxhp)
											pi_needitem->hp = pi_needitem->maxhp;
										if (pi_needitem->hp - c>0)
										{
											if (strlen(cmsg))
												sysmessage(ts, cmsg);
											else 
											{
												total = (float) pi_needitem->hp/pi_needitem->maxhp; // Magius(CHE) §
												sprintf(tempstr, "Your %s is now repaired! [%.1f%%]", tempname3, total*100);  // Magius(CHE) §
												sysmessage(ts, tempstr);
											}
										}
										else if (pi_needitem->hp - c < 0)
										{
											if (strlen(fmsg))
												sysmessage(ts, fmsg);
											else 
											{
												sprintf(tempstr, "Your %s appears to be more ruined than before!", tempname3);
												sysmessage(ts, tempstr);
											}
										}
										if (pi_needitem->hp <= 0)
										{
											sprintf(tempstr, "Your %s was too old and it has been destroyed!", tempname3);
											sysmessage(ts, tempstr);
											if (pi_needitem->amount>1)
												pi_needitem->amount--;
											else 
												Items->DeleItem(pi_needitem);
										}
									}
								}
								else 
								{
									sprintf(tempstr, "You can't repair %s !", tempname3);
									sysmessage(ts, tempstr);
								}
							}
						}
						else if (!(strcmp("NEEDMAXDUR", (char*)script1)))  // Math on NEED item HP (only if NEEDITEM>-1) ---- rewrite by Magius(CHE) §
						{
							cline = &script2[0];
							splitline();
							j = makenumber(0);
							int p = makenumber(1);
							if (p <= 0)
								p = 100;
							if (pi_needitem != NULL)
							{
								if (pi_needitem->maxhp>0)
								{
									if ((rand()%(100)) + 1 <= p)
									{
										pi_needitem->maxhp += j;
										if (pi_needitem->hp >= pi_needitem->maxhp)
											pi_needitem->hp = pi_needitem->maxhp;
										if (str2num(script2) >= 0)
										{
											if (strlen(cmsg))
												sysmessage(ts, cmsg);
											else 
											{
												sprintf(tempstr, "You increased the maximum durability of your %s !", tempname3);
												sysmessage(ts, tempstr);
											}
										}
										else 
										{
											if (strlen(fmsg))
												sysmessage(ts, fmsg);
											else 
											{
												sprintf(tempstr, "Your %s appears to be more ruined than before!", tempname3);
												sysmessage(ts, tempstr);
											}
										}
										if (pi_needitem->maxhp <= 0)
										{
											sprintf(tempstr, "Your %s was too old and it has been destoryed!", tempname3);
											sysmessage(ts, tempstr);
											if (pi_needitem->amount>1)
												pi_needitem->amount--;
											else 
												Items->DeleItem(pi_needitem);
										}
									}
								}
								else 
								{
									sprintf(tempstr, "You can't increase durability of %s !", tempname3);
									sysmessage(ts, tempstr);
								}
							}
						}
						else if ((!(strcmp("NEWNAME", (char*)script1))))  // Give the new item/npc a name
						{
							if (pi_itemnum != NULL)
							{
								strcpy(pi_itemnum->name, (char*)script2);
							}
							if (npcnum>-1)
							{
								strcpy(chars[npcnum].name, (char*)script2);
							}
						}
						else if (!(strcmp("NEWTRIG", (char*)script1)))  // Give the new item/npc a dynamic trigger number
						{
							if (pi_itemnum != NULL)
							{
								pi_itemnum->trigger = str2num(script2);
							}
							if (npcnum>-1)
							{
								chars[npcnum].trigger = str2num(script2);
							}
						}
						else if (!(strcmp("NEWWORD", (char*)script1)))  // Give the new npc a trigger word
						{
							if (npcnum>-1)
							{
								strcpy(chars[npcnum].trigword, (char*)script2);
							}
						}
						break;
					case 'O':
						if (!(strcmp("OFFEMT", (char*)script1)))  // Player says something when trigger is activated
						{
							if (pi->trigon)
							{
								strcpy(sect, script2);
								for (i = 0; i < now; i++)
								{
									if (inrange1p(currchar[ts], currchar[i]) && perm[i])
									{
										tl = 44 + strlen(sect) + 1;
										talk[1] = tl >> 8;
										talk[2] = tl%256;
										talk[3] = pc_ts->ser1;
										talk[4] = pc_ts->ser2;
										talk[5] = pc_ts->ser3;
										talk[6] = pc_ts->ser4;
										talk[7] = pc_ts->id1;
										talk[8] = pc_ts->id2;
										talk[9] = 0; // Type
										ShortToCharPtr(pc_ts->saycolor, &talk[10]);
										talk[12] = 0;
										talk[13] = pc_ts->fonttype;
										Xsend(i, talk, 14);
										Xsend(i, pc_ts->name, 30);
										Xsend(i, sect, strlen(sect) + 1);
									}
								}
							}
						}
						else if (!(strcmp("ONEMT", (char*)script1)))  // Player says something when trigger is activated
						{
							if (!pi->trigon)
							{
								strcpy(sect, script2);
								for (i = 0; i < now; i++)
								{
									if (inrange1p(currchar[ts], currchar[i]) && perm[i])
									{
										tl = 44 + strlen(sect) + 1;
										talk[1] = tl >> 8;
										talk[2] = tl%256;
										talk[3] = pc_ts->ser1;
										talk[4] = pc_ts->ser2;
										talk[5] = pc_ts->ser3;
										talk[6] = pc_ts->ser4;
										talk[7] = pc_ts->id1;
										talk[8] = pc_ts->id2;
										talk[9] = 0; // Type
										ShortToCharPtr(pc_ts->saycolor, &talk[10]);
										talk[12] = 0;
										talk[13] = pc_ts->fonttype;
										Xsend(i, talk, 14);
										Xsend(i, pc_ts->name, 30);
										Xsend(i, sect, strlen(sect) + 1);
									}
								}
							}
						}
						else if (!(strcmp("OFFMSG", (char*)script1)))// -Frazurbluu- New script command (type 2)
						{
							if (pi->trigon)
							{
							strcpy(sect,script2);
							sysmessage(ts, sect);
							}
						}
						else if (!(strcmp("ONMSG", (char*)script1)))// -Frazurbluu- New script command (type 2)
						{
							if (!pi->trigon)
							{
							strcpy(sect,script2);
							sysmessage(ts, sect);
							}
						}
						else if (!(strcmp("OUTRANGE", (char*)script1)))  // Player is in range of an item (only if this item is out of backpack) -- Magius(CHE) §
						{
							cline = &script2[0];
							splitline();
							// comm[0] contains the id of the out-items!
							int p = makenumber(1);
							
							// int tmp=hex2num(comm[0]);
							// int id1=tmp>>8;
							// int id2=tmp%256;
							c=-1;
							
							// clConsole.send("OUTRANGE: SEARCHING FOR %x %x MAX DISTANCE %i\n",id1,id2,p);
							
							// x1=pc_ts->pos.x;
							// y1=pc_ts->pos.y;
							// printf("Magius(CHE): OUTRANGE called!\n This operation maybe cause some lag. It is in TEST-MODE!\n");
							// this FOR cicle maybe cause same lag, if someone know another method to work, plz change this!
							/* for (j = 0; j < itemcount; j++)
							{
							sprintf(sect, "x%x%x", items[j].id1, items[j].id2);
							if (strstr(comm[0], sect))
							{
								if (items[j].contserial!=-1)
								{
								// sprintf(sect,"You need to put the item out of your backpack before working.");
								// sysmessage(ts,sect);
								}
								else 
								{
									x2 = items[j].x;
									y2 = items[j].y;
									dx = abs(x1 - x2);
									dy = abs(y1 - y2);
									if ((dx <= p) &&(dy <= p))
										c = j;
									}
								}
							}
							*/
							// AntiChrist
							int	StartGrid = mapRegions->StartGrid(pc_ts->pos.x, pc_ts->pos.y);
							
							unsigned long loopexit = 0;
							unsigned int increment = 0;
							cRegion::RegionIterator4Items ri(pc_ts->pos);
							for (ri.Begin(); ri.GetData() != ri.End(); ri++)
							{
								P_ITEM mapitem = ri.GetData();
								if (mapitem != NULL)
								{// if it's close enought
									if (iteminrange(ts, DEREF_P_ITEM(mapitem), p))
									{
										sprintf(sect, "x%x%x", mapitem->id1, mapitem->id2);
										if (strstr((char*)comm[0], sect))
										{// if it's the item we want
											c = DEREF_P_ITEM(mapitem);// we found it :D
										}
									}
								}
							}
							
							if (c==-1)
							{
								if (strlen(fmsg))
									sysmessage(ts, fmsg);
								else
								{
									strcpy(sect, "You are not close enough to work on that.");
									sysmessage(ts, sect);
								}
								closescript();
								return;
							}
						}
						else if (!strcmp("OPENGUMP", (char*)script1))
							Gumps->Menu(ts, str2num(script2), -1);
						
						break;
					case 'P':
						if (!(strcmp("PUT", (char*)script1)))  // Send player to X Y Z when triggered
						{
							cline = &script2[0];
							splitline();
							
							pc_ts->MoveTo(makenumber(0),makenumber(1),makenumber(2));

							pos = ftell(scpfile);	// teleport might open scripts
							closescript();
							teleport(currchar[ts]);
							if (ttype)
								openscript("triggers.scp");
							else 
								openscript("wtrigrs.scp");
							fseek(scpfile, pos, SEEK_SET);
							strcpy((char*)script1, "DUMMY");
						}
						break;
					case 'R':
						if (!(strcmp("RANDOM_ITEM", (char*)script1)))  //%chance to IADD item
						{
							uiCompleted = 0;
							cline = &script2[0];
							splitline();
							unsigned int uiItemNum = makenumber(0);
							unsigned int uiChance = makenumber(1);
							unsigned int InBackpack = makenumber(2);
							
							if (uiChance >=((rand()%100) + 1))
							{
								strcpy((char*)script1, "IADD");
								sprintf((char*)script2, "%i %i", uiItemNum, InBackpack);
								uiCompleted = 1;
							}
							else if (strlen(fmsg))
								sysmessage(ts, fmsg); // Added by Magius(CHE) §
						}
						else if (!(strcmp("RANDOM_NPC", (char*)script1)))  //%chance to NADD item
						{
							uiCompleted = 0;
							cline = &script2[0];
							splitline();
							unsigned int uiItemNum = makenumber(0);
							unsigned int uiChance = makenumber(1);
							
							if (uiChance >=((rand()%100) + 1))
							{
								strcpy((char*)script1, "NADD");
								sprintf((char*)script2, "%i", uiItemNum);
								uiCompleted = 1;
							}
							else if (strlen(fmsg))
								sysmessage(ts, fmsg);// Added by Magius(CHE) §
						}
						else if (!(strcmp("REQCOLOR", (char*)script1)))  // Set the color check on REQUIRED item by Magius(CHE) §
						{
							cline = &script2[0];
							splitline();
							clr1 = hexnumber(0);
							clr2 = hexnumber(1);
							j = makenumber(2);
							if (pi_evti != NULL)
							{// AntiChrist
								if (clr1 != pi_evti->color1 || clr2 != pi_evti->color2)
								{
									if (strlen(fmsg))
										sysmessage(ts, fmsg);
									else
									{
										sprintf((char*)temp, "You need another type of %s!", tempname2);
										sysmessage(ts, (char*)temp);
									}
									closescript();
									return;
								}
							}
							
							if (j>0)
							{
								pc_ts->targtrig = j;
								closescript();
								triggerwitem(ts, ti, 1);
								return;
							}
						}
						else if (!(strcmp("RANGE", (char*)script1)))  // Player is in range
						{
							x1 = pc_ts->pos.x;
							y1 = pc_ts->pos.y;
							if (ti>-1)
							{
								x2 = pi->pos.x;
								y2 = pi->pos.y;
								z2 = pi->pos.z;
								int p = currchar[ts];
								r = packitem(currchar[ts]);
								if (r!=-1) // lb
									if (pi->contserial != items[r].serial)
									{
										dx = abs(x1 - x2);
										dy = abs(y1 - y2);
										if ((dx>str2num(script2)) ||(dy>str2num(script2)))
										{
											// Magius(CHE) §
											if (strlen(fmsg))
												sysmessage(ts, fmsg);
											else
											{
												strcpy(sect, "You are not close enough to use that.");
												sysmessage(ts, sect);
											}
											// End MAgius Fix
											closescript();
											return;
										}
									}
							}
							else 
							{
								x2 = (buffer[ts][11] << 8) + buffer[ts][12];
								y2 = (buffer[ts][13] << 8) + buffer[ts][14];
								z2 = (buffer[ts][15] << 8) + buffer[ts][16];
								dx = abs(x1 - x2);
								dy = abs(y1 - y2);
								if ((dx>str2num(script2)) ||(dy>str2num(script2)))
								{
									// Magius(CHE) §
									if (strlen(fmsg))
										sysmessage(ts, fmsg);
									else 
									{
										strcpy(sect, "You are not close enough to use that.");
										sysmessage(ts, sect);
									}
									// End MAgius Fix
									closescript();
									return;
								}
							}
						}
						else if (!(strcmp("RAND", (char*)script1)))  // Does a random check
						{
							cline = &script2[0];
							splitline();
							int p;
							if (makenumber(0) != 0)
								p = (rand()%(makenumber(0))) + 1;
							else 
								p = 1;
							if (p>makenumber(1))
							{
								if (strlen(fmsg))
									sysmessage(ts, fmsg);
								closescript();
								return;
							}
						}
						else if (!(strcmp("REMOVE", (char*)script1)))  // Remove item after triggered
						{
							if (ti>-1)
							{
								if (pi->amount>1)
								{
									pi->amount--;
								}
								else 
								{
									Items->DeleItem(ti);
								}
							}
						}
						else if (!(strcmp("RNDUSES", (char*)script1)))  // Randomly set an items uses
						{
							if (ti>-1)
							{
								if (!pi->tuses)
								{
									cline = &script2[0];
									splitline();
									if (makenumber(1) != 0)
										pi->tuses = (rand()%(makenumber(1))) + makenumber(0);
									else 
										pi->tuses = makenumber(0);
								}
							}
						}
						else if (!(strcmp("REQ", (char*)script1)))  // Check if envoked by certain item.
						{
							sprintf(sect, "x%x%x", pc_ts->envokeid1, pc_ts->envokeid2);
							if (!strstr((char*)script2, sect))
							{
								sysmessage(ts, "That didn't seem to work.");
								closescript();
								return;
							}
							else
							{ // Lines Added By Magius(CHE) to fix Targ trigger
								pi_evti = MAKE_ITEM_REF(pc_ts->envokeitem);
								if (pi_evti!= NULL)// AntiChrist
								{
									if (pi_evti->name[0] != '#') // Get Temporany Name of the REQUIRED Item - Magius(CHE)
									{
										strcpy(tempname2, pi_evti->name);
									}
									else
									{
										Map->SeekTile(pi_evti->id(), &tile);
										strcpy(tempname2, (char*)tile.name);
									}
								}
							} // End Addon
						}
						break;
					case 'S':
						if (!(strcmp("SKL", (char*)script1)))  // Do math on the players skill
						{
							cline = &script2[0];
							splitline();
							int p = makenumber(0);
							j = makenumber(1);
							
							if (j)
							{
								if (pc_ts->baseskill[p] < 1000)
								{
									pc_ts->baseskill[p] += j;
									if (pc_ts->baseskill[p]>1000)
										pc_ts->baseskill[p] = 1000;
									Skills->AdvanceStats(currchar[ts], p);
									Skills->updateSkillLevel(currchar[ts], p);
									updateskill(ts, p);
								}
							}
							else 
							{
								Skills->AdvanceSkill(currchar[ts], p, 1);
								Skills->updateSkillLevel(currchar[ts], p);
								updateskill(ts, p);
							}
						}
						else if (!(strcmp("SETUSES", (char*)script1)))  // Set an items uses
						{
							if (ti>-1)
							{
								pi->tuses = str2num(script2);
							}
						}
						else if (!(strcmp("STAM", (char*)script1)))  // Do math on players stamina
						{
							j = str2num(script2);
							if ((j < 0) &&(pc_ts->stm < abs(j)))
							{
								sysmessage(ts, "You are too tired to do that.");
								closescript();
								return;
							}
							pc_ts->stm += j;
							if (pc_ts->stm>pc_ts->effDex())
								pc_ts->stm = pc_ts->effDex();
							updatestats(currchar[ts], 2);
						}
						else if (!(strcmp("STR", (char*)script1)))  // Do math on players strength
						{
							j = str2num(script2);
							if (j < 0)
							{
								sysmessage(ts, "You lost strength!");
							}
							else 
							{
								sysmessage(ts, "You are getting stronger!");
							}
							pc_ts->st += j;
							if (pc_ts->st < 1)
								pc_ts->st = 1;
						}
						else if (!(strcmp("SND", (char*)script1)))  // Play a sound when trigger is activated
						{
							cline = &script2[0];
							splitline();
							soundeffect(ts, hexnumber(0), hexnumber(1));
						}
						else if (!(strcmp("SETEVID", (char*)script1)))  // Set envoked items id to new id
						{
							if (pc_ts->envokeitem>-1)
							{
								cline = &script2[0];
								splitline();
								items[pc_ts->envokeitem].id1 = hexnumber(0);
								items[pc_ts->envokeitem].id2 = hexnumber(1);		
								RefreshItem(pc_ts->envokeitem);// AntiChrist
								pi_itemnum = MAKE_ITEM_REF(pc_ts->envokeitem);
							}
						}
						else if (!(strcmp("SETOWNER", (char*)script1)))  // Set ownership of item
						{
							if (pi != NULL)
							{
								int p = currchar[ts];
								pi->SetOwnSerial(chars[p].serial);
							}
						}
						else if (!(strcmp("SETTRG", (char*)script1)))  // Set items trigger to new trigger
						{
							if (ti>-1)
								pi->trigger = str2num(script2);
						}
						else if (!(strcmp("SETID", (char*)script1)))  // Set items id to new id
						{
							if (ti>-1)
							{
								cline = &script2[0];
								splitline();
								pi->id1 = hexnumber(0);
								pi->id2 = hexnumber(1);				
								RefreshItem(ti);// AntiChrist
								pi_itemnum = MAKE_ITEM_REF(ti);
							}
						}
						break;
					case 'T':
						if (!(strcmp("TARG", (char*)script1)))  // Give a targeter with trigger number --- Fixed By Magius(CHE)
						{
							target(ts, 0, 1, 0, 204, "Select a target");
							pc_ts->targtrig = str2num(script2);
						}
						else if (!(strcmp("TRONSK", (char*)script1))) // Trigger On Skill
						{
							cline = &script2[0];
							splitline();
							int p = makenumber(0);
							j = makenumber(1);
							if (pi->trigon) // -Frazurbluu- beginning of skill raising items
							{
								if ((p>-1) && (p<49))
								{
									pc_ts->baseskill[p] -= j;
									Skills->updateSkillLevel(currchar[ts], p);
									updateskill(ts, p);
									pi->trigon=0;
								}
								else
								{
									if (p==49)// strength
									{
										pc_ts->st -= j;
										if (pc_ts->st < 1)
											pc_ts->st = 1;
										updatestats(ts, p);
									}
									if (p==50)// dexterity
									{
										pc_ts->chgRealDex(-1 * j);
										updatestats(ts, p);
									}
									if (p==51)// intelligence
									{
										pc_ts->in -= j;
										if (pc_ts->in < 1)
											pc_ts->in = 1;
										updatestats(ts, p);
									}
									pi->trigon=0;
								}
							}
							else
							{
								if ((p>-1) && (p<49))
								{
									pc_ts->baseskill[p] += j;
									Skills->updateSkillLevel(currchar[ts], p);
									updateskill(ts, p);
									pi->trigon=1;
								}
								else
								{
									if (p==49)// strength
									{
										pc_ts->st += j;
										if (pc_ts->st < 1)
											pc_ts->st = 1;
										updatestats(ts, p);
									}
									if (p==50)// dexterity
									{
										pc_ts->chgRealDex(j);
										updatestats(ts, p);
									}
									if (p==51)// intelligence
									{
										pc_ts->in += j;
										if (pc_ts->in < 1)
											pc_ts->in = 1;
										updatestats(ts, p);
									}
									pi->trigon=1;
								}
							}
						}
						closescript();
						return;			// Fraz, are you sure ??? (Duke, 20.10.01)
						break;
					case 'U':
						if (!(strcmp("USEUP", (char*)script1)))  // The item here is required and will be removed
						{
							int p = packitem(currchar[ts]);
							if (p==-1)
								p = 0;
							
							if (pi_needitem == NULL)
							{
								vector<SERIAL> vecContainer = contsp.getData(items[p].serial);
								for (ci = 0; ci < vecContainer.size(); ci++)
								{
									P_ITEM pi = FindItemBySerial(vecContainer[ci]);
									if (pi != NULL)
									{
										sprintf(sect, "x%x%x", pi->id1, pi->id2);
										if (strstr((char*)script2, sect))
										{
											pi_needitem = pi;
											break;
										}
									}
								}
							}
							if (pi_needitem == NULL)
							{
								sysmessage(ts, "It appears as though you have insufficient supplies to make that with.");
								closescript();
								return;
							}
							else
							{
								if (pi_needitem->amount>1)
									pi_needitem->amount--;
								else
									Items->DeleItem(pi_needitem);
							}
						}
					default:
						break;
			}
		}
  }
  while ((script1[0] != '}') &&(++loopexit < MAXLOOPS));
  closescript();
}

//**************************************************************************
// This routine processes tokens for the NPC triggers.
// ts: socket!!!! (Player fire the trigger)- AntiChrist
// ti: no-socket!!!! (NPC triggered) - Magius(CHE)
void triggernpc(UOXSOCKET ts, int ti, int ttype) // Changed by Magius(CHE) §
{
	char sect[512], effect[29];
	signed int j;
	int p, c;
	unsigned int i;
	P_ITEM pi_itemnum = NULL;
	int npcnum=-1;
	P_ITEM pi_needitem = NULL;
	long int pos;
	char fmsg[512];
	
	// Added by Magius(CHE) §
	int tl;
	P_ITEM pi_evti = NULL;
	tile_st tile;
	char tempname2[512], tempname3[512], buff[512], dismsg[512], tempstr[512], cmsg[512];
	float total;
	char clr1, clr2;
	char memcolor1, memcolor2;
	int coloring=-1, trig=-1, loopexit = 0;
	tempname2[0] = 0;
	buff[0] = 0;
	dismsg[0] = 0;
	tempstr[0] = 0;
	cmsg[0] = 0;
	fmsg[0] = 0;
	
	if (ts < 0)
		return; // §Magius crash fix

	P_CHAR pc_ts = MAKE_CHARREF_LR(currchar[ts]);
	
	if (ttype == 1)
	{
		openscript("ntrigrs.scp");
		sprintf(sect, "TRG %i", chars[ti].trigger);
		if (ti>-1)
			trig = chars[ti].trigger;
		if (!i_scripts[ntrigger_script]->find(sect))
		{
			closescript();
			return;
		}
	}
	else	
	{
		openscript("wtrigrs.scp");
		sprintf(buff, "x%x%x", buffer[ts][0x11], buffer[ts][0x12]);
		if (!i_scripts[wtrigger_script]->isin(buff))
		{
			strcpy(sect, "You cant think of a way to use that.");
			sysmessage(ts, sect);
			closescript();
			return;
		}
	}
	if (chars[ti].disabled>uiCurrentTime) // Added by Magius(CHE) §
	{
		if (!chars[ti].disabledmsg[0] == 0x0)
			strcpy((char*)temp, chars[ti].disabledmsg);
		else 
			strcpy((char*)temp, "You cant work now! Wait more time!");
		sysmessage(ts, (char*)temp);
		closescript();
		return;
	}
	do
	{
		read2();
		if (script1[0] != '}')
		{
			switch (script1[0])
			{
				case 'A':
					if (!(strcmp("ACT", (char*)script1)))  // Make player perform an action
					{
						cline = &script2[0];
						splitline();
						action(ts, hexnumber(0));
					}
					break;
				case 'C':
					if (!(strcmp("CSKL", (char*)script1)))  // Make a check on the players skill -- Rewrite by Magius(CHE) §
					{
						// Magius(CHE) §
						cline = &script2[0];
						splitline();
						p = makenumber(0);
						j = makenumber(1);
						// End Magius(CHE) §
						
						i = (rand()%1000) + 1;
						// Taur 69.02 added to get some chance of
						// skill gain on failure
						unsigned int skill = p; // Magius(CHE) §
						unsigned int baseskill = pc_ts->baseskill[skill];
						if (i > baseskill)
						{
							Skills->AdvanceSkill(DEREF_P_CHAR(pc_ts), skill, 0);
							Skills->updateSkillLevel(DEREF_P_CHAR(pc_ts), skill);
							updateskill(ts, skill);
							if (strlen(fmsg))
								sysmessage(ts, fmsg); // by Magius(CHE) §
							else 
							{
								strcpy((char*)temp, "You fail in your attempt..."); // by Magius(CHE) §
								sysmessage(ts, (char*)temp); // by Magius(CHE) §
							}
							closescript();
							return;
						} // Taur end 69.02 change
						if (j>0)
						{
							// Magius(CHE) §
							closescript();
							triggernpc(ts, ti, 1);
							return;
						}
					}
					break;
				case 'D':
					if (!(strcmp("DEX", (char*)script1)))  // Do math on players dexterity
					{
						j = str2num(script2);
						if (j < 0)
						{
							sysmessage(ts, "You are getting clumsier!");
						}
						else 
						{
							sysmessage(ts, "You are getting more agile!");
						}
						pc_ts->chgRealDex(j);
					}
					else if (!(strcmp("DISABLE", (char*)script1)))  // Disable NPC for n seconds
					{
						if (ti>-1)
						{
							chars[ti].disabled = (uiCurrentTime +(MY_CLOCKS_PER_SEC*str2num(script2)));
							strcpy(chars[ti].disabledmsg, dismsg); // Added by Magius(CHE) §
						}
					}
					else if (!(strcmp("DISABLEMSG", (char*)script1)))  // Disable NPC Message --- by Magius(CHE) §
					{
						strcpy(dismsg, (char*)script2);
					}
					break;
				case 'E':
					if (!(strcmp("EVDUR", (char*)script1)))  // Math on Evoked item HP ---- Rewrite by Magius(CHE) §
					{
						cline = &script2[0];
						splitline();
						j = makenumber(0);
						p = makenumber(1);
						if (p <= 0)
							p = 100;
						if (pi_evti != NULL)
						{
							c = pi_evti->hp;
							if (pi_evti->maxhp>0)
							{
								if ((rand()%(100)) + 1 <= p)
								{
									if ((c >= pi_evti->maxhp) &&(j>0))
									{
										sysmessage(ts, "Your %s is already totally repaired!", tempname2);
									}
									pi_evti->hp += j;
									if (pi_evti->hp >= pi_evti->maxhp)
										pi_evti->hp = pi_evti->maxhp;
									if (pi_evti->hp - c>0)
									{
										if (strlen(cmsg))
											sysmessage(ts, cmsg);
										else 
										{
											total = (float) pi_evti->hp/pi_evti->maxhp;
											sysmessage(ts, "Your %s is now repaired! [%.1f%%]", tempname2, total*100);
										}
									}
									else if (pi_evti->hp - c < 0)
									{
										if (strlen(fmsg))
											sysmessage(ts, fmsg);
										else 
										{
											sysmessage(ts, "Your %s appears to be more ruined than before!", tempname2);
										}
									}
									if (pi_evti->hp <= 0)
									{
										sysmessage(ts, "Your %s was too old and it has been destroyed!", tempname2);
										if (pi_evti->amount>1)
											pi_evti->amount--;
										else 
											Items->DeleItem(DEREF_P_ITEM(pi_evti));
									}
								}
							}
							else 
							{
								sysmessage(ts, "You can't repair %s !", tempname2);
							}
						}
					}
					else if (!(strcmp("EMT", (char*)script1)))  // Player says something when trigger is activated -- Changed by Magius(CHE) §
					{
						strcpy(sect, (char*)script2);
						for (i = 0; i < now; i++)
						{
							if (inrange1p(DEREF_P_CHAR(pc_ts), currchar[i]) && perm[i])
							{
								tl = 44 + strlen(sect) + 1;
								talk[1] = tl >> 8;
								talk[2] = tl%256;
								talk[3] = pc_ts->ser1;
								talk[4] = pc_ts->ser2;
								talk[5] = pc_ts->ser3;
								talk[6] = pc_ts->ser4;
								talk[7] = pc_ts->id1;
								talk[8] = pc_ts->id2;
								talk[9] = 0; // Type
								ShortToCharPtr(pc_ts->saycolor, &talk[10]);
								talk[12] = 0;
								talk[13] = pc_ts->fonttype;
								Xsend(i, talk, 14);
								Xsend(i, pc_ts->name, 30);
								Xsend(i, sect, strlen(sect) + 1);
							}
						}
					}
					else if (!(strcmp("EVMAXDUR", (char*)script1)))  // Math on Evoked item MAXHP ---- Rewrite by Magius(CHE) §
					{
						cline = &script2[0];
						splitline();
						j = makenumber(0);
						p = makenumber(1);
						if (p <= 0)
							p = 100;
						if (pi_evti != NULL)
						{
							if (pi_evti->maxhp>0)
							{
								if ((rand()%(100)) + 1 <= p)
								{
									pi_evti->maxhp += j; // Magius(CHE) §
									if (pi_evti->hp >= pi_evti->maxhp)
										pi_evti->hp = pi_evti->maxhp;
									if (str2num(script2) >= 0)
									{
										if (strlen(cmsg))
											sysmessage(ts, cmsg);
										else 
										{
											sysmessage(ts, "You increased the maximum durability of your %s !", tempname2);
										}
									}
									else 
									{
										if (strlen(fmsg))
											sysmessage(ts, fmsg);
										else 
										{
											sysmessage(ts, "Your %s appears to be not resistant as before!", tempname2);
										}
									}
									if (pi_evti->maxhp <= 0)
									{
										sysmessage(ts, "Your %s was too old and it has been destroyed!", tempname2);
										if (pi_evti->amount>1)
											pi_evti->amount--;
										else 
											Items->DeleItem(DEREF_P_ITEM(pi_evti));
									}
								}
							}
							else 
							{
								sysmessage(ts, "You can't increase durability of %s !", tempname2);
							}
						}
					}
					break;
					case 'F':
						if (!(strcmp("FAME", (char*)script1)))  // Do math on the players fame - Magius(CHE)
						{
							cline = &script2[0];
							splitline();
							j = makenumber(0);
							c = pc_ts->fame;
							if ((j>0) &&(c >= 10000))
								sysmessage(ts, "You are already the most gloriosus person in this world!");
							if ((j < 0) &&(c <=-10000))
								sysmessage(ts, "You are already the most unknown person in this world!");
							pc_ts->fame += j;
							if (pc_ts->fame>10000)
								pc_ts->fame = 10000;
							if (pc_ts->fame < -10000)
								pc_ts->fame=-10000;
							j = pc_ts->fame - c;
							if (j>0)
							{
								if (j < 25)
									sysmessage(ts, "You have gained a little fame!");
								else if (j < 75)
									sysmessage(ts, "You have gained some fame!");
								else if (j < 100)
									sysmessage(ts, "You have gained a lot of fame!");
								else 
									sysmessage(ts, "You have gained a huge amount of fame!");
							}
							
							else if (j < 0)
							{
								if (abs(j) < 25)
									sysmessage(ts, "You have lost a little fame!");
								else if (abs(j) < 75)
									sysmessage(ts, "You have lost some fame!");
								else if (abs(j) < 100)
									sysmessage(ts, "You have lost a lot of fame!");
								else 
									sysmessage(ts, "You have lost a huge amount of fame!");
							}
							
							
							// refresh title
						}
						else if (!(strcmp("FMSG", (char*)script1)))  // Set fail message
						{
							strcpy(fmsg, (char*)script2);
						}
						break;
					case 'H':
						if (!(strcmp("HEA", (char*)script1)))  // Do math on players health
						{
							j = str2num(script2);
							
							if ((j < 0) &&(pc_ts->hp < abs(j)) &&(!(pc_ts->isInvul())))
							{
								deathstuff(DEREF_P_CHAR(pc_ts));
								closescript();
								return;
							}
							pc_ts->hp += j;
							if (pc_ts->hp>pc_ts->st)
								pc_ts->hp = pc_ts->st;
							updatestats(DEREF_P_CHAR(pc_ts), 0);
						}
						else if (!(strcmp("HUNGER", (char*)script1)))  // Do math on players hunger from 0 to 6 - Magius(CHE)
						{
							j = str2num(script2);
							pc_ts->hunger += j;
							if (pc_ts->hunger>6)
								pc_ts->hunger = 6;
							if (pc_ts->hunger < 1)
								pc_ts->hunger = 1;
							switch (pc_ts->hunger)
							{
							case 0:	sysmessage(ts, "You eat the food, but are still extremely hungry.");	break;
							case 1: sysmessage(ts, "You eat the food, but are still extremely hungry.");	break;
							case 2: sysmessage(ts, "After eating the food, you feel much less hungry.");	break;
							case 3: sysmessage(ts, "You eat the food, and begin to feel more satiated.");	break;
							case 4: sysmessage(ts, "You feel quite full after consuming the food.");		break;
							case 5: sysmessage(ts, "You are nearly stuffed, but manage to eat the food.");	break;
							case 6: sysmessage(ts, "You are simply too full to eat any more!");				break;
							}
						}
						break;
					case 'I':
						if (!strcmp("IFREQ", (char*)script1))
						{
							// Check Envoked ID, if true JUMP to another trigger (in ntrig.scp)
							cline = &script2[0];
							splitline();
							j = makenumber(1);
							sprintf(sect, "x%x%x", pc_ts->envokeid1, pc_ts->envokeid2);
							if (strstr((char*)comm[0], sect))
							{
								// IEnvoked item found in IFREQ --> Jump Trigger
								// clConsole.send("IFREQ %s  -> Jumping to NPC trigger %i\n",comm[0],j);
								closescript();
								triggernpc(ts, ti, 1);
								return;
							}
						}
						else if (!(strcmp("IFKARMA", (char*)script1)))  // If karma meets a certain criteria - Magius(CHE)
						{
							char opt;
							gettokennum((char*)script2, 0);
							opt = gettokenstr[0];
							gettokennum((char*)script2, 1);
							j = str2num(gettokenstr);
							// clConsole.send("Script2: %s\nOPT: %c\n%i %c %i\n",script2,opt,chars[p].karma,opt,j);
							if (opt == '>')
							{
								if (!(pc_ts->karma >= j))
								{
									if (strlen(fmsg))
										sysmessage(ts, fmsg);
									else 
										sysmessage(ts, "You need more Karma to do that!");
									closescript();
									return;
								}
							}
							else if (opt == '<')
							{
								if (!(pc_ts->karma <= j))
								{
									if (strlen(fmsg))
										sysmessage(ts, fmsg);
									else 
										sysmessage(ts, "You need less Karma to do that!");
									closescript();
									return;
								}
							}
							else 
								clConsole.send("TRIGGER.SCP:Triggerwitem() Syntax Error in IFKARMA token.\n   IFKARMA <opt> <value>\n    <opt>= '<' or '>'\n    <value> range from '-10000' to '10000'.\n");
						}
						else if (!(strcmp("IFFAME", (char*)script1)))  // If karma meets a certain criteria - Magius(CHE)
						{
							char opt;
							gettokennum((char*)script2, 0);
							opt = gettokenstr[0];
							gettokennum((char*)script2, 1);
							j = str2num(gettokenstr);
							if (opt == '>')
							{
								if (!(pc_ts->fame >= j))
								{
									if (strlen(fmsg))
										sysmessage(ts, fmsg);
									else 
										sysmessage(ts, "You need more Fame to do that!");
									closescript();
									return;
								}
							}
							else if (opt == '<')
							{
								if (!(pc_ts->fame <= abs(j)))
									
								{
									if (strlen(fmsg))
										sysmessage(ts, fmsg);
									else 
										sysmessage(ts, "You need less Fame to do that!");
									closescript();
									return;
								}
							}
							else 
								clConsole.send("TRIGGER.SCP:Triggerwitem() Syntax Error in IFFAME token.\n   IFFAME <opt> <value>\n    <opt>= '<' or '>'\n    <value> range from '-10000' to '10000'.\n");
						}
						else if (!(strcmp("IFHUNGER", (char*)script1)))  // If hunger meets a certain criteria - Magius(CHE)
						{
							j = str2num(script2);
							if (j >= 0)
							{
								if (!(pc_ts->hunger >= j))
								{
									if (strlen(fmsg))
										sysmessage(ts, fmsg);
									closescript();
									return;
								}
							}
							else 
							{
								if (!(pc_ts->hunger <= abs(j)))
									
								{
									if (strlen(fmsg))
										sysmessage(ts, fmsg);
									closescript();
									return;
								}
							}
						}
						else if (!(strcmp("IFOWNER", (char*)script1)))  // If character is owned by triggerer
						{
							if (ti>-1)
							{
								if (!pc_ts->Owns(&chars[ti]))
								{
									sysmessage(ts, "You do not own this creature.");
									closescript();
									return;
								}
							}
						}
						else if (!(strcmp("IDADD", (char*)script1)))  // Add item in player pack by ID
						{
							cline = &script2[0];
							splitline();
							pos = ftell(scpfile);
							closescript();
							P_ITEM pi_c = MAKE_ITEM_REF(Items->SpawnItem(ts, DEREF_P_CHAR(pc_ts), 1, "#", 1, hexnumber(0), hexnumber(1), 0, 0, 1, 1));
							if (pi_c == NULL)
								return;// AntiChrist to preview crashes
							// Added colormem token here! by Magius(CHE) §
							if (coloring>-1)
							{
								pi_c->color1 = memcolor1;
								pi_c->color2 = memcolor2;
								RefreshItem(pi_c);
							}
							// end addons	
							if (ttype)
								openscript("ntrigrs.scp"); // Added by Magius(CHE) §
							else 
								openscript("wtrigrs.scp");  // Added by Magius(CHE) §
							fseek(scpfile, pos, SEEK_SET);
						}
						else if (!(strcmp("IDFX", (char*)script1)))  // Makes an effect at players by ID
						{
							if (ti>-1)
							{
								for (i = 0; i < 29; i++)
								{
									effect[i] = 0;
								}
								cline = &script2[0];
								splitline();
								effect[0] = 0x70; // Effect message
								effect[1] = 0x00; // Moving effect
								effect[2] = chars[ti].ser1;
								effect[3] = chars[ti].ser2;
								effect[4] = chars[ti].ser3;
								effect[5] = chars[ti].ser4;
								effect[6] = pc_ts->ser1;
								effect[7] = pc_ts->ser2;
								effect[8] = pc_ts->ser3;
								effect[9] = pc_ts->ser4;
								effect[10] = hexnumber(0);// Object id of the effect
								effect[11] = hexnumber(1);
								effect[12] = chars[ti].pos.x >> 8;
								effect[13] = chars[ti].pos.x%256;
								effect[14] = chars[ti].pos.y >> 8;
								effect[15] = chars[ti].pos.y%256;
								effect[16] = chars[ti].pos.z;
								effect[17] = pc_ts->pos.x >> 8;
								effect[18] = pc_ts->pos.x%256;
								effect[19] = pc_ts->pos.y >> 8;
								effect[20] = pc_ts->pos.y%256;
								effect[21] = pc_ts->pos.z;
								effect[22] = 0x09;
								effect[23] = 0x06; // 0 is really long.  1 is the shortest.
								effect[24] = 0; // This value is unknown
								effect[25] = 0; // This value is unknown
								effect[26] = 1; // This value is unknown
								effect[27] = 0x00; // This value is used for moving effects that explode on impact.
								for (j = 0; j < now; j++)
								{
									if ((inrange1p(currchar[j], DEREF_P_CHAR(pc_ts))) &&(perm[j]))
									{
										Xsend(j, effect, 28);
									}
								}
							}
						}
						else if (!(strcmp("INT", (char*)script1)))  // Do math on players intelligence
						{
							j = str2num(script2);
							if (j < 0)
							{
								sysmessage(ts, "Your mind is growing weaker!");
							}
							else 
							{
								sysmessage(ts, "Your mind is getting stronger!");
							}
							pc_ts->in += j;
							if (pc_ts->in < 1)
								pc_ts->in = 1;
						}
						else if (!(strcmp("IADD", (char*)script1)))  // Add item in front of player //if 2nd param. is 1, add item into player's backpack - AntiChrist (with autostack)
						{
							handle_IADD(ts, ttype, coloring, memcolor1, memcolor2, "ntrigrs.scp");
						}
						break;
					case 'K':
						if (!(strcmp("KARMA", (char*)script1)))  // Do math on the players karma - Magius(CHE)
						{
							cline = &script2[0];
							splitline();
							j = makenumber(0);
							c = pc_ts->karma;
							if ((j>0) &&(c >= 10000))
								sysmessage(ts, "You are already the most honest person in this world!");
							if ((j < 0) &&(c <=-10000))
								sysmessage(ts, "You are already the most evil person in this world!");
							pc_ts->karma += j;
							if (pc_ts->karma>10000)
								pc_ts->karma = 10000;
							if (pc_ts->karma < -10000)
								pc_ts->karma=-10000;
							j = pc_ts->karma - c;
							if (j>0)
							{
								if (j < 25)
									sysmessage(ts, "You have gained a little karma!");
								else if (j < 75)
									sysmessage(ts, "You have gained some karma!");
								else if (j < 100)
									sysmessage(ts, "You have gained a lot of karma!");
								else 
									sysmessage(ts, "You have gained a huge amount of karma!");
							}
							else if (j < 0)
							{
								if (abs(j) < 25)
									sysmessage(ts, "You have lost a little karma!");
								else if (abs(j) < 75)
									sysmessage(ts, "You have lost some karma!");
								else if (abs(j) < 100)
									sysmessage(ts, "You have lost a lot of karma!");
								
								else 
									sysmessage(ts, "You have lost a huge amount of karma!");
							}
						}
						break;
					case 'M':
						if (!(strcmp("MAKE", (char*)script1)))  // Give user the make menu
						{
							cline = &script2[0];
							splitline();
							itemmake[ts].Mat1id = ((hexnumber(2)) << 8) + hexnumber(3);
							itemmake[ts].has = getamount(DEREF_P_CHAR(pc_ts), itemmake[ts].Mat1id); 
							itemmake[ts].has2 = getamount(DEREF_P_CHAR(pc_ts), itemmake[ts].Mat2id);
							itemmake[ts].coloring = coloring; // Magius(CHE) §
							if (coloring>-1)
							{
								itemmake[ts].newcolor1 = memcolor1; // Magius(CHE) §
								itemmake[ts].newcolor2 = memcolor2; // Magius(CHE) §
							}
							pos = ftell(scpfile);
							closescript();
							Skills->MakeMenu(ts, makenumber(0), makenumber(1));
							if (ttype)
								openscript("ntrigrs.scp");
							else 
								openscript("wtrigrs.scp");
							fseek(scpfile, pos, SEEK_SET);
							strcpy((char*)script1, "DUMMY");
						}
						else if (!(strcmp("MISC", (char*)script1)))  // Perform a miscellaneous function
						{
							if (!(strcmp("bank", strlwr((char*)script2))))
								openbank(ts, DEREF_P_CHAR(pc_ts));
							if (!(strcmp("ware", strlwr((char*)script2))))
								openspecialbank(ts, DEREF_P_CHAR(pc_ts));// AntiChrist
							if (!(strcmp("balance", strlwr((char*)script2))))
							{
								sprintf(sect, "You have %i gp in your bank account!", pc_ts->CountBankGold());
								npctalk(ts, ti, sect, 0);
							}
						}
						else if (!(strcmp("MSG", (char*)script1)))  // Display a message when trigger is activated
						{
							if ((pc_ts->isPlayer()))
							{
								sysmessage(ts, script2);
							}
						}
						else if (!(strcmp("MAN", (char*)script1)))  // Do math on players mana
						{
							j = str2num(script2);
							if ((j < 0) &&(pc_ts->mn < abs(j)))
								
							{
								sysmessage(ts, "Your mind is too tired to do that.");
								closescript();
								return;
							}
							pc_ts->mn += j;
							if (pc_ts->mn>pc_ts->in)
								pc_ts->mn = pc_ts->in;
							updatestats(DEREF_P_CHAR(pc_ts), 1);
						}
						else if (!(strcmp("MEMCOLOR", (char*)script1)))  // Store the item color in memory by Magius(CHE) §
						{
							P_ITEM p = NULL;
							if (!strcmp((char*)script2, "EMPTY"))
							{
								coloring=-1;
								closescript();
								return;
							}
							else if (!strcmp((char*)script2, "REQ"))
								p = pi_evti;
							else if (!strcmp((char*)script2, "NEED"))
								p = pi_needitem;
							if (p != NULL)
							{
								memcolor1 = p->color1;
								memcolor2 = p->color2;
								coloring = 1;
							}
							else 
							{
								clConsole.send("WOLFPACK: Error in Trigger script.\n Check MEMCOLOR %s in SECTION ", script2);
								if (ttype == 1)
									clConsole.send("%i.\n", trig);
								else 
									clConsole.send("...x%x%x...\n", buffer[ts][0x11], buffer[ts][0x12]);
								clConsole.send(" Make sure that it was after %s.\n", script2);
							}
						}
						break;
					case 'N':
						if (!(strcmp("NPCACT", (char*)script1)))  // Make player perform an action
						{
							cline = &script2[0];
							splitline();
							npcaction(ti, hexnumber(0));
						}
						else if (!(strcmp("NADD", (char*)script1)))  // Add a NPC at given location - AntiChrist -- Fixed here by Magius(CHE) §
						{// Usage: NADD <npc_number> <life_in_seconds>
							// uiCompleted=0;
							cline = &script2[0];
							splitline();
							unsigned int uiNpcNum = makenumber(0);
							unsigned int uiTimer = makenumber(1);
							/*unsigned int uiTamed = makenumber(2); */
							
							triggerx = pc_ts->pos.x + 1;
							triggery = pc_ts->pos.y + 1;
							triggerz = pc_ts->pos.z;
							
							pos = ftell(scpfile);
							closescript();
							Npcs->AddRespawnNPC(ti, uiNpcNum, 1);
							npcnum = triggerx;
							
							if (uiTimer>0)
							{// if we have a timer
								chars[npcnum].summontimer = uiCurrentTime +(uiTimer*MY_CLOCKS_PER_SEC);
							}
							
							triggerx = 0;
							if (ttype)
								openscript("ntrigrs.scp"); // Added by Magius(CHE) §
							else 
								openscript("wtrigrs.scp");  // Added by Magius(CHE) §
							fseek(scpfile, pos, SEEK_SET);
							strcpy((char*)script1, "DUMMY");
						}
						else if (!(strcmp("NEWOWNER", (char*)script1)))  // Set ownership of NPC
						{
							if (pi_itemnum != NULL)
							{// item
								p = DEREF_P_CHAR(pc_ts);
								pi_itemnum->SetOwnSerial(pc_ts->serial);
							}
							if (npcnum>-1)
							{// char
								p = DEREF_P_CHAR(pc_ts);
								chars[npcnum].SetOwnSerial(pc_ts->serial);
							}
						}
						else if (!(strcmp("NEEDCOLOR", (char*)script1)))  // Set the color check on NEEDED item by Magius(CHE) §
						{
							cline = &script2[0];
							splitline();
							clr1 = hexnumber(0);
							clr2 = hexnumber(1);
							j = makenumber(2);
							if (pi_needitem == NULL)
							{
								closescript();
								return;
							}
							if (clr1 != pi_needitem->color1 || clr2 != pi_needitem->color2)
							{
								if (strlen(fmsg))
									sysmessage(ts, fmsg);
								else 
								{
									sysmessage(ts, "You need another type of %s!", tempname3);
								}
								closescript();
								return;
							}
							if (j>0)
							{
								pc_ts->targtrig = j;
								closescript();
								triggerwitem(ts, ti, 1);
								return;
							}
						}
						else if (!(strcmp("NEED", (char*)script1)))  // The item here is required and will be removed
						{
							p = packitem(DEREF_P_CHAR(pc_ts));
							vector<SERIAL> vecContainer = contsp.getData(items[p].serial);
							unsigned int i;
							for (i = 0; i < vecContainer.size(); i++)
							{
								P_ITEM pi = FindItemBySerial(vecContainer[i]);
								sprintf(sect, "x%x%x", pi->id1, pi->id2);
								if (strstr((char*)script2, sect))
								{
									pi_needitem = pi;
									break;
								}
							}
							if (pi_needitem < 0)
							{
								if (strlen(fmsg))
									sysmessage(ts, fmsg); // Added by Magius(CHE)
								else 
									sysmessage(ts, "You don't have the neccessary supplies to do that."); // Changed by Magius(CHE)
								closescript();
								return;
							}
							else 
							{
								// Get Temporany Name of the NEED Item - Magius(CHE) §
								if (items[i].name[0] != '#') // Get Temporany Name of the NEED Item - Magius(CHE) §
									strcpy(tempname3, items[i].name);
								else 
								{
									Map->SeekTile(items[i].id(), &tile);
									strcpy(tempname3, (char*)tile.name);
								}
								// End Get Temporany Name of the NEED Item - Magius(CHE) §
							}
						}
						else if ((!(strcmp("NEWTYPE", (char*)script1))) ||(!(strcmp("SETTYPE", (char*)script1))))  // Set active item type
						{
							if (pi_itemnum != NULL)
								pi_itemnum->type = str2num(script2);
						}
						else if (!(strcmp("NEWNAME", (char*)script1)))  // Give the new item/npc a name
						{
							if (pi_itemnum != NULL)
							{
								strcpy(pi_itemnum->name, (char*)script2);
							}
							if (npcnum>-1)
							{
								strcpy(chars[npcnum].name, (char*)script2);
							}
						}
						else if (!(strcmp("NEWTRIG", (char*)script1)))  // Give the new item/npc a dynamic trigger number
						{
							if (pi_itemnum != NULL)
							{
								pi_itemnum->trigger = str2num(script2);
							}
							if (npcnum>-1)
							{
								chars[npcnum].trigger = str2num(script2);
							}
						}
						else if (!(strcmp("NEWWORD", (char*)script1)))  // Give the new npc a triggerword
						{
							if (npcnum>-1)
							{
								strcpy(chars[npcnum].trigword, (char*)script2);
							}
						}
						else if (!(strcmp("NEWSPEECH", (char*)script1)))  // Give the new npc a new spech -- MAgius(CHE) §
						{
							if (npcnum>-1)
							{
								chars[npcnum].speech = str2num(script2);
							}
						}
						break;
					case 'O':
						if (!strcmp("OPENGUMP", (char*)script1))
							Gumps->Menu(ts, str2num(script2), -1);
						break;
						
					case 'P':
						if (!(strcmp("PUT", (char*)script1)))  // Send player to X Y Z when triggered
						{
							cline = &script2[0];
							splitline();
							
							pc_ts->pos.x = makenumber(0);
							pc_ts->pos.y = makenumber(1);
							pc_ts->pos.z = makenumber(2);
							teleport(DEREF_P_CHAR(pc_ts));
						}
						break;
					case 'R':
						if (!(strcmp("REQCOLOR", (char*)script1)))  // Set the color check on REQUIRED item by Magius(CHE) §
						{
							cline = &script2[0];
							splitline();
							clr1 = hexnumber(0);
							clr2 = hexnumber(1);
							j = makenumber(2);
							if (pi_evti == NULL)
							{
								closescript();
								return;
							}
							if (clr1 != pi_evti->color1 || clr2 != pi_evti->color2)
							{
								if (strlen(fmsg))
									sysmessage(ts, fmsg);
								else 
								{
									sysmessage(ts, "You need another type of %s!", tempname2);
								}
								closescript();
								return;
							}
							if (j>0)
							{
								pc_ts->targtrig = j;
								closescript();
								triggerwitem(ts, ti, 1);
								return;
							}
						}
						else if (!(strcmp("RAND", (char*)script1)))  // Does a random check
						{
							cline = &script2[0];
							splitline();
							if (makenumber(0) != 0)
								p = (rand()%(makenumber(0))) + 1;
							else 
								p = 1;
							
							if (p>makenumber(1))
							{
								if (strlen(fmsg))
									sysmessage(ts, fmsg);
								closescript();
								return;
							}
						}
						else if (!(strcmp("REQ", (char*)script1)))  // Check if envoked by certain item. Added By Magius(CHE) to fix Targ trigger
						{
							sprintf(sect, "x%x%x", pc_ts->envokeid1, pc_ts->envokeid2);
							if (!strstr((char*)script2, sect))
							{
								sysmessage(ts, "That didn't seem to work.");
								closescript();
								return;
							}
							else 
							{
								pi_evti = MAKE_ITEM_REF(pc_ts->envokeitem);
								if (items[pc_ts->envokeitem].name[0] != '#') // Get Temporany Name of the REQUIRED Item - Magius(CHE)
									sprintf(tempname2, "%s", items[pc_ts->envokeitem].name);
								else 
								{
									Map->SeekTile(pi_evti->id(), &tile);
									strcpy(tempname2, (char*)tile.name);
								}
							} // End Addon
						}
						break;
					case 'S':
						if (!(strcmp("SND", (char*)script1)))  // Play a sound when trigger is activated
						{
							cline = &script2[0];
							splitline();
							soundeffect2(ti, hexnumber(0), hexnumber(1));
						}
						else if (!(strcmp("SETTRG", (char*)script1)))  // Set npcs trigger to new trigger
						{
							if (ti>-1)
								chars[ti].trigger = str2num(script2);
						}
						else if (!(strcmp("SPEECH", (char*)script1)))  // Set the triggered npc a new spech -- Magius(CHE) §
						{
							if (ti>-1)
							{
								chars[ti].speech = str2num(script2);
							}
						}
						// End NPC Triggers
						else if (!(strcmp("SETID", (char*)script1)))  // Set chars id to new id
						{
							if (ti>-1)
							{
								cline = &script2[0];
								splitline();
								chars[ti].id1 = hexnumber(0);
								chars[ti].id2 = hexnumber(1);
								chars[ti].xid1 = hexnumber(0);
								chars[ti].xid2 = hexnumber(1);
								for (j = 0; j < now; j++)
									if (perm[j] && inrange1p(currchar[j], ti))
										updatechar(ti);
							}
						}
						else if (!(strcmp("STAM", (char*)script1)))  // Do math on players stamina
						{
							j = str2num(script2);
							
							if ((j < 0) &&(pc_ts->stm < abs(j)))
								
							{
								sysmessage(ts, "You are too tired to do that.");
								closescript();
								return;
							}
							pc_ts->stm += j;
							if (pc_ts->stm>pc_ts->effDex())
								pc_ts->stm = pc_ts->effDex();
							updatestats(DEREF_P_CHAR(pc_ts), 2);
						}
						else if (!(strcmp("STR", (char*)script1)))  // Do math on players strength
						{
							j = str2num(script2);
							if (j < 0)
							{
								sysmessage(ts, "You lost strength!");
							}
							else 
							{
								sysmessage(ts, "You are getting stronger!");
							}
							pc_ts->st += j;
							if (pc_ts->st < 1)
								pc_ts->st = 1;
						}
						else if (!(strcmp("SETWORD", (char*)script1)))  // Sets the trigger word of an NPC
						{
							if (ti>-1)
								strcpy(chars[ti].trigword, script2);
						}
						else if (!(strcmp("SKL", (char*)script1)))  // Do math on the players skill
						{
							cline = &script2[0];
							splitline();
							p = makenumber(0);
							j = makenumber(1);
							
							if (j)
							{
								if (pc_ts->baseskill[p] < 1000)
								{
									pc_ts->baseskill[p] += j;
									if (pc_ts->baseskill[p]>1000)
										pc_ts->baseskill[p] = 1000;
								}
							}
							else 
							{
								Skills->AdvanceSkill(DEREF_P_CHAR(pc_ts), p, 1);
							}
						}
						else if (!(strcmp("SETOWNER", (char*)script1)))  // Set ownership of NPC
						{
							if (ti>-1)
							{
//								p = currchar[ts];
								chars[ti].SetOwnSerial(pc_ts->serial);
							}
						}
						break;
					case 'T':
						if (!(strcmp("TALK", (char*)script1)))  // the triggered NPC now talking! --- by Magius(CHE) §
						{
							if (ti>-1 && strlen((char*)script2)>0)
								npctalk(ts, ti, (char*)script2, 0);
						}
						break;
					case 'U':
						if (!(strcmp("USEUP", (char*)script1)))  // The item here is required and will be removed
						{
							P_ITEM pPack = Packitem(pc_ts);
							if (pPack != NULL) // lb
								if (pi_needitem != NULL)
								{
									vector<SERIAL> vecContainer = contsp.getData(pPack->serial);
									unsigned int i;
									for (i = 0; i < vecContainer.size(); i++)
									{
										P_ITEM pi = FindItemBySerial(vecContainer[i]);
										sprintf(sect, "x%x%x", pi->id1, pi->id2);
										if (strstr((char*)script2, sect))
										{
											pi_needitem = pi;
											break;
										}
									}
								}
							if (pi_needitem == NULL)
							{
								sysmessage(ts, "It appears as though you have insufficient supplies to make that with.");
								closescript();
								return;
							}
							else 
							{
								if (pi_needitem->amount>1)
									pi_needitem->amount--;
								else
									Items->DeleItem(pi_needitem);
							}
						}
						break;
					default:
						break;
				}
		}
  }
  while ((script1[0] != '}')  &&(++loopexit < MAXLOOPS));
  closescript();
}

int checkenvoke(char eid1, char eid2)
{
	FILE *envokefile;
	char buf[1024];
	
	if (!(envokefile = fopen("envoke.scp", "r")))
	{
		fprintf(stderr, "Cannot open envoke.scp: %s", strerror(errno));
		exit(1);
	}
	unsigned long loopexit = 0;
	while ((fgets(buf, sizeof(buf), envokefile))  &&(++loopexit < MAXLOOPS))
	{
		sprintf((char*)temp, "x%x%xx", eid1, eid2);      // antichrist
		sprintf((char*)temp2, "x%x%x ", eid1, eid2);	  // antichrist
		if ((strstr(buf, (char*)temp) > 0) || (strstr(buf, (char*)temp2) > 0))
		{
			fclose(envokefile);
			return 1;
		}
	}
	fclose(envokefile);
	
	return 0;
}
