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

// Platform Includes
#include "platform.h"

#include "Timing.h"
#include "worldmain.h"
#include "walking2.h"
#include "TmpEff.h"
#include "combat.h"
#include "regions.h"
#include "srvparams.h"

// Library Includes
#include "qdatetime.h"

#undef  DBGFILE
#define DBGFILE "Timing.cpp"

void checktimers() // Check shutdown timers
{
	register unsigned int tclock = uiCurrentTime;
	overflow = ( lclock > tclock );
	if (endtime)
	{
		if (endtime <= tclock) keeprun = 0;
	}
	lclock = tclock;
}

void do_lsd(UOXSOCKET s)
{
	P_CHAR pc_currchar = currchar[s];
	if (rand()%15==0)
	{
		int c1,c2,color,ctr=0,b,xx,yy,di,icnt=0;
		signed char zz;
		int StartGrid=mapRegions->StartGrid(pc_currchar->pos);
		unsigned int increment=0;
		for (unsigned int checkgrid=StartGrid+(increment*mapRegions->GetColSize());increment<3;increment++, checkgrid=StartGrid+(increment*mapRegions->GetColSize()))
		{
			for (int a=0;a<3;a++)
			{
				cRegion::raw vecEntries = mapRegions->GetCellEntries(checkgrid + a);
				cRegion::rawIterator it = vecEntries.begin();
				for ( ; it != vecEntries.end(); ++it )
				{
					P_ITEM pi = FindItemBySerial(*it);
					P_CHAR mapchar = FindCharBySerial(*it);
					if (pi != NULL)
					{
						 color = pi->color(); // fetch item's color
						 if (rand()%44==0) color+=pi->pos.x-pi->pos.y; else
						 color+=pc_currchar->pos.x+pc_currchar->pos.y;
						 color+=rand()%3; // add random "noise"
						 ctr++;
						 // lots of color consistancy checks
						 color=color%0x03E9;
							 c1=color>>8;
						 c2=color%256;
						 if ((((c1<<8)+c2)<0x0002) || (((c1<<8)+c2)>0x03E9) )
						 {
							c1=0x03;
							c2=0xE9;
						 }
						 b=((((c1<<8)+c2)&0x4000)>>14)+((((c1<<8)+c2)&0x8000)>>15);
						 if (b)
						 {
							c1=0x1;
							c2=rand()%255;
						 }
	
						 if (rand()%10==0) zz=pi->pos.z+rand()%33; else zz=pi->pos.z;
						 if (rand()%10==0) xx=pi->pos.x+rand()%3; else xx=pi->pos.x;
						 if (rand()%10==0) yy=pi->pos.y+rand()%3; else yy=pi->pos.y;
						 di = itemdist(pc_currchar, pi);
						 if (di<13) if (rand()%7==0)
						 {
							icnt++;
							if (icnt%10==0 || icnt<10) 
								senditem_lsd(s, pi, c1, c2, xx, yy, zz); // attempt to cut packet-bombing by this thing
						 }
	
					}// end of if item
					else if (mapchar != NULL)// character
					{
						di = chardist(pc_currchar, mapchar);
						if (di<10) if (rand()%10==0)
						{
							icnt++;
							if (icnt%10==0 || icnt<10) 
								sendperson_lsd(s, mapchar, c1, c2); // attempt to cut packet-bombing by this thing
						}
					}
				}
			}
		}	

		if (rand()%33==0)
		{
			if (rand()%10>3) soundeffect5(s, 0x00, 0xF8); // lsd sound :)
			else
			{
				int snd=rand()%19;
				if (snd>9) soundeffect5(s,0x01,snd-10);
				else soundeffect5(s,0,246+snd);
			}	
		}
	}
}

void restockNPC(unsigned int currenttime, P_CHAR pc_i)
{
	unsigned int a, b;

	if (SrvParams->shopRestock()==1 && (shoprestocktime<=currenttime || overflow))
	{
		vector<SERIAL> vecContainer = contsp.getData(pc_i->serial);
		for ( a = 0; a < vecContainer.size(); a++ )
		{
			const PC_ITEM pici = FindItemBySerial(vecContainer[a]);
			if (pici != NULL)
			{
				if(pici->layer==0x1A && pc_i->shop) //morrolan item restock fix
				{
					vector<SERIAL> vecContainer2 = contsp.getData(pici->serial);
					for (b=0;b<vecContainer2.size();b++)
					{
						const P_ITEM pic = FindItemBySerial(vecContainer2[b]);
						if (pic != NULL)
						{
							if (pic->restock)
							{
								int tmp=min(pic->restock, (pic->restock/2)+1);
								pic->amount += tmp;
								pic->restock -= tmp;
							}
							// MAgius(CHE): All items in shopkeeper need a new randomvaluerate.
							if (SrvParams->trade_system()==1)
								StoreItemRandomValue(pic, calcRegionFromXY(pc_i->pos.x, pc_i->pos.y));// Magius(CHE) (2)
						}
					}// for b
				}
			}
		}//for a
	}//if time
}

void genericCheck(P_CHAR pc, unsigned int currenttime)// Char mapRegions
{
	int c;
	
	if (pc == NULL)
		return;
	
	if (!(pc->dead))
	{
		if (pc->hp>pc->st)
		{
			pc->hp = pc->st;
			updatestats(pc, 0);
		}
		if (pc->stm>pc->effDex())
		{
			pc->stm = pc->effDex();
			updatestats(pc, 2);
		}
		if (pc->mn>pc->in)
		{
			pc->mn = pc->in;
			updatestats(pc, 1);
		}
		if ((pc->regen <= currenttime) || (overflow))
		{
			unsigned int interval = SrvParams->hitpointrate()*MY_CLOCKS_PER_SEC;
			if (pc->hp < pc->st && pc->hunger>3 || SrvParams->hungerRate() == 0)
			{
				for (c = 0; c < pc->st + 1; c++)
				{
					if (pc->regen + (c*interval) <= currenttime && pc->hp <= pc->st)
					{
						if (pc->skill[17] < 500)
							pc->hp++;
						else if (pc->skill[17] < 800)
							pc->hp += 2;
						else 
							pc->hp += 3;
						if (pc->hp>pc->st)
						{
							pc->hp = pc->st;
							break;
						}
						updatestats(pc, 0);
					}
				}
			}
			pc->regen = currenttime + interval;
		}
		if ((pc->regen2 <= currenttime) || (overflow))
		{
			unsigned int interval = SrvParams->staminarate()*MY_CLOCKS_PER_SEC;
			for (c = 0; c < pc->effDex() + 1; c++)
			{
				if (pc->regen2 + (c*interval) <= currenttime && pc->stm <= pc->effDex())
				{
					pc->stm++;
					if (pc->stm>pc->effDex())
					{
						pc->stm = pc->effDex();
						break;
					}
					updatestats(pc, 2);
				}
			}
			pc->regen2 = currenttime + interval;
		}
		// OSI Style Mana regeneration by blackwind
		// if (pc->in>pc->mn)  this leads to the 'mana not subtracted' bug (Duke)
			if ((pc->regen3 <= currenttime) || (overflow))
			{
				unsigned int interval = SrvParams->manarate()*MY_CLOCKS_PER_SEC;
				for(c=0;c<pc->in+1;c++)
				{
					if (pc->regen3 + (c*interval) <= currenttime && pc->mn <= pc->in)
					{
						pc->mn++;
						if (pc->med && pc->mn <= pc->mn2)
							pc->mn += 5;
						if (pc->mn>pc->in)
						{
							if (pc->med)
							{
								int s = calcSocketFromChar(pc);
								sysmessage(s, tr("You are at peace.") );
								pc->med = false;
							}
							pc->mn = pc->in;
							break;
						}
						updatestats(pc, 1);
					}
				}
				if (SrvParams->armoraffectmana())
				{
					// blackwind's osi style mana regeneration formula
					int ratio = ((100 + 50)/SrvParams->manarate());
					// 100 = Maximum skill (GM)
					// 50 = int affects mana regen (%50)
					int armorhandicap = ((Skills->GetAntiMagicalArmorDefence(pc) + 1) / SrvParams->manarate());
					int charsmeditsecs = (1 + SrvParams->manarate() - ((((pc->skill[MEDITATION] + 1)/10) + ((pc->in + 1) / 2)) / ratio));
					if (pc->med)
					{
						pc->regen3 = currenttime + ((armorhandicap + charsmeditsecs/2)* MY_CLOCKS_PER_SEC);
					}
					else
						pc->regen3 = currenttime + ((armorhandicap + charsmeditsecs)* MY_CLOCKS_PER_SEC);
				}
				else 
					pc->regen3 = currenttime + interval;
			}
			// end Mana regeneration
			if ((pc->hidden == 2) && ((pc->invistimeout <= currenttime) || (overflow)) && (!(pc->priv2&8)))
			{// only if not permanently hidden - AntiChrist
				pc->hidden = 0;
				pc->stealth=-1;
				updatechar(pc);
			}
	}
	if (pc->hp <= 0 && !pc->dead)
		deathstuff(pc);
}

void checkPC(P_CHAR pc, unsigned int currenttime)//Char mapRegions
{
	int y,x, timer;//, valid=0;
	char t[120];

	if ( pc == NULL ) return;

	UOXSOCKET s = calcSocketFromChar(pc);//Only calc socket once!

	Magic->CheckFieldEffects2(currenttime, pc, 1);//Lag fix
	if (!pc->dead && pc->swingtarg==-1 )
		Combat->DoCombat(pc, currenttime);
	else if(!pc->dead && (pc->swingtarg>=0 && pc->timeout<=currenttime))
		Combat->CombatHitCheckLoS(pc, currenttime);

/*	if (wtype==1 && raindroptime<=currenttime && !noweather[s]) // implment. of xuri's raindrop idea, LB
	{
		switch(rand()%3)
		{
		case 0:soundeffect2(i,0x00,0x24); // fall-throughs intentional !!
		case 1:soundeffect2(i,0x00,0x23);
		 case 2:soundeffect2(i,0x00,0x22);
		}
	} */

	if (pc->smoketimer>currenttime)
	{
		if (pc->smokedisplaytimer<=currenttime)
		{
			pc->smokedisplaytimer=currenttime+5*MY_CLOCKS_PER_SEC;
			staticeffect(pc, 0x37, 0x35, 0, 30);
			soundeffect2(pc, 0x002B);
			switch( RandomNum(0, 6) )
			{
			 case 0:	npcemote(s, pc, "*Drags in deep*",1 );				break;
			 case 1:	npcemote(s, pc, "*Coughs*",1 );						break;
			 case 2:	npcemote(s, pc, "*Retches*",1 );					break;
			 case 3:	npcemote(s, pc, "*Hacking cough*",1 );				break;
			 case 4:	npcemote(s, pc, "*Sighs in contentment*",1 );		break;
			 case 5:	npcemote(s, pc, "*Puff puff*",1 );					break;
			 case 6:	npcemote(s, pc, "Wheeeee!!! Xuri's smoking!",1 );	break;
			 default:	break;
			}
		}
	}

	if (LSD[s]) do_lsd(s); //LB's LSD potion-stuff

	if (pc->isPlayer() && online(pc) && pc->squelched==2)
	{
		if (pc->mutetime!=-1)
		{
			if (pc->mutetime<=currenttime||overflow)
			{
				pc->squelched=0;
				pc->mutetime=-1;
				sysmessage(s, tr("You are no longer squelched!") );
			}
		}
	}

	if (pc->isPlayer() && online(pc))
	{
		if ( pc->crimflag > 0 && ( pc->crimflag <= currenttime || overflow ) &&  pc->isCriminal() )//AntiChrist
		{
			sysmessage(s, tr("You are no longer a criminal.") );
			pc->crimflag=-1;
			pc->setInnocent();
		}
		if (pc->murderrate<currenttime)//AntiChrist
		{
			if (pc->kills>0)
				pc->kills--;
			if ((pc->kills==SrvParams->maxkills())&&(SrvParams->maxkills()>0))
				sysmessage(s, tr("You are no longer a murderer.") );
			pc->murderrate=(SrvParams->murderdecay()*MY_CLOCKS_PER_SEC)+currenttime;//AntiChrist
		}
		setcharflag(pc);//AntiChrist
	}

	if (pc->isPlayer() && pc->casting)//PC casting a spell
	{
		pc->nextact--;
		if (pc->spelltime<=currenttime||overflow)//Spell is complete target it.
		{
			Magic->AfterSpellDelay( s, pc );
		}
		else if (pc->nextact<=0)//redo the spell action
		{
			pc->nextact=75;
			impaction(s, pc->spellaction);
		}
	}

	if(SrvParams->bgSound()>=1)
	{
		timer = SrvParams->bgSound() * 100;
		if ( timer == 0 ) timer = 1;
		if( online(pc) && pc->isPlayer() && !pc->dead && ( (rand()%(timer) ) == (timer/2))) 
			bgsound(pc);
	}
	if( pc->spiritspeaktimer > 0 && pc->spiritspeaktimer <= uiCurrentTime)
		pc->spiritspeaktimer = 0;

	
	// Blackwinds Jail stuff.
	if (pc->cell>0)
	{
		if ((pc->jailtimer>0) && (pc->jailtimer <= uiCurrentTime))
		{
			sysmessage(s, tr("Your jail time is over!") );
			
			if (pc != NULL)
			{
				if(pc->cell==0)
				{			
					sysmessage(s, tr("You're not in jail already ? Please report to GM") );
					pc->jailtimer=0;
					sprintf((char*)temp,"%i cause bug in jail system.",pc->account);
					savelog((char*)temp,"server.log");
				}
				else
				{
					jails[pc->cell].occupied = false;
					pc->moveTo(jails[pc->cell].oldpos);
					pc->cell=0;
					pc->jailsecs=0;
					pc->jailtimer=0;
					pc->priv2=0;
					teleport(pc);
					
					sprintf((char*)temp,"%s is auto-released from jail \n",pc->name.c_str());
					savelog((char*)temp,"server.log");
					
					sysmessage(s, tr("You are released.") );
				}
			}
			
		}
	}
	

		// LB, changed to seconds instead of crappy #of checks, 21/9/99
	if(pc->trackingtimer > currenttime && online(pc))
	{
		if(pc->trackingdisplaytimer<=currenttime)
		{
			pc->trackingdisplaytimer=currenttime+SrvParams->redisplaytime()*MY_CLOCKS_PER_SEC;
			Skills->Track(pc);
		}
	} else
	{
		if (pc->trackingtimer>(currenttime/10)) // dont send arrow-away packet all the time
		{
			pc->trackingtimer=0;
			unsigned char arrow[7] = {0xBA, 0,};
			P_CHAR pc_trackingTarget = FindCharBySerial(pc->trackingtarget);
			arrow[0]='\xBA';
			arrow[1]=0;
			arrow[2]=(pc_trackingTarget->pos.x-1)>>8;
			arrow[3]=(pc_trackingTarget->pos.x-1)%256;
			arrow[4]=pc_trackingTarget->pos.y>>8;
			arrow[5]=pc_trackingTarget->pos.y%256;
			Xsend(s,arrow,6);
		}
	}

	if (SrvParams->hungerRate() > 1 && (pc->hungertime<=currenttime || overflow))
	{
		if (!pc->isGMorCounselor() && pc->hunger) pc->hunger--; //Morrolan GMs and Counselors don't get hungry

		switch(pc->hunger)
		{
		case 6: break; //Morrolan
		case 5: sysmessage(s, tr("You are still stuffed from your last meal") );	break;
		case 4:	sysmessage(s, tr("You are not very hungry but could eat more") );	break;
		case 3:	sysmessage(s, tr("You are feeling fairly hungry") );				break;
		case 2:	sysmessage(s, tr("You are extremely hungry") );						break;
		case 1:	sysmessage(s, tr("You are very weak from starvation") );			break;
		case 0:
			if (!pc->isGMorCounselor())
				sysmessage(s, tr("You must eat very soon or you will die!") );
			break;
		}
		pc->hungertime = currenttime+(SrvParams->hungerRate() * MY_CLOCKS_PER_SEC); // Bookmark
	}
	if (((hungerdamagetimer<=currenttime)||(overflow))&&(SrvParams->hungerDamage()>0)) // Damage them if they are very hungry
	{
		hungerdamagetimer=currenttime+(SrvParams->hungerDamageRate()*MY_CLOCKS_PER_SEC); /** set new hungertime **/
		if (pc->hp > 0 && pc->hunger<2 && !pc->isGMorCounselor() && !pc->dead)
		{
			sysmessage(s, tr("You are starving !") );
			pc->hp -= SrvParams->hungerDamage();
			updatestats(pc, 0);
			if(pc->hp<=0)
			{
				sysmessage(s, tr("You have died of starvation") );
				deathstuff(pc);
			}
		}
	}

	// new math + poison wear off timer added by lord binary !

	if ( pc->poisoned && (online(pc) || pc->isNpc()) && !pc->isInvul() )
	{
		if (pc->poisontime<=currenttime || (overflow))
		{
			if (pc->poisonwearofftime>currenttime) // lb, makes poison wear off pc's
			{
				switch (pc->poisoned)
				{
				case 1:
					pc->poisontime=currenttime+(5*MY_CLOCKS_PER_SEC);
					if ( pc->poisontxt<=currenttime || (overflow))
					{
						pc->poisontxt=currenttime+(10*MY_CLOCKS_PER_SEC);
						sprintf(t,"* %s looks a bit nauseous *", pc->name.c_str());
						pc->emotecolor = 0x0026;//buffer[s][4];
						npcemoteall(pc,t,1);
					}
				 
					pc->hp -= max(((pc->hp)*RandomNum(5,15))/100, RandomNum(0,1) ); // between 0% and 10% of player's hp 
 
					updatestats(pc, 0);
					break;
				case 2:
					pc->poisontime=currenttime+(4*MY_CLOCKS_PER_SEC);
					if ((pc->poisontxt<=currenttime)||(overflow))
					{
						pc->poisontxt=currenttime+(10*MY_CLOCKS_PER_SEC);
						sprintf(t,"* %s looks disoriented and nauseous! *",pc->name.c_str());
						pc->emotecolor = 0x0026;//buffer[s][4];
						npcemoteall(pc,t,1);
					}
					
					pc->hp -= max(((pc->hp)*RandomNum(10,20))/100, RandomNum(0,1)); //between 10% and 20% of player's hp
 
					updatestats(pc, 0);
					break;
				case 3:
					pc->poisontime=currenttime+(3*MY_CLOCKS_PER_SEC);
					if ( pc->poisontxt <= currenttime ||(overflow))
					{
						pc->poisontxt = currenttime+(10*MY_CLOCKS_PER_SEC);
						sprintf(t,"* %s is in severe pain! *", pc->name.c_str());
						pc->emotecolor = 0x0026;//buffer[s][4];
						npcemoteall(pc,t,1);
					}
					x=RandomNum(1,3);
					y=RandomNum(5,10);
					y=10;
					
					pc->hp -= max(((pc->hp)*RandomNum(20,30))/100, RandomNum(0,1)); //between 20% and 30% of player's hp 

					updatestats(pc, 0);
					break; // lb !!!
				case 4:
					pc->poisontime = currenttime+(3*MY_CLOCKS_PER_SEC);
					if ( pc->poisontxt <= currenttime || (overflow))
					{
						pc->poisontxt=currenttime+(10*MY_CLOCKS_PER_SEC);
						sprintf(t,"* %s looks extremely weak and is wrecked in pain! *", pc->name.c_str());
						pc->emotecolor = 0x0026;//buffer[s][4];
						npcemoteall(pc,t,1);
					}

					x = RandomNum(3,6);
					y = 20;
				
					pc->hp -= max(((pc->hp)*RandomNum(30,40))/100, 1); //between 30% and 40% of player's hp 
					updatestats(pc, 0);
					break;

				default:
					clConsole.send("ERROR: Fallout of switch statement without default. wolfpack.cpp, checkPC()\n"); //Morrolan
					pc->poisoned=0;
					return;
				}
				if (pc->hp<1)
				{
					deathstuff(pc);
					sysmessage(s, tr("The poison has killed you.") );
				}
			} // end switch
		} // end if poison-wear off-timer
	} // end if poison-damage timer

	if ( pc->poisoned && pc->poisonwearofftime<=currenttime && online(pc) )
	{
		pc->poisoned = 0;
		impowncreate(s, pc, 1); // updating to blue stats-bar ...
		sysmessage(s, tr("The poison has worn off.") );
	}
	if( pc->onhorse )
	{
		P_ITEM pHorse = pc->GetItemOnLayer(0x19);
		if(!pHorse)
		{
			pc->onhorse = false;	// turn it off, we aren't on one because there's no item!
			return;
		}
		else
		{
			if( pHorse->decaytime != 0 && ( pHorse->decaytime <= uiCurrentTime || overflow ) )
			{
				pc->onhorse = false;
				Items->DeleItem( pHorse );
			}
		}
	}
}

void checkNPC(P_CHAR pc, unsigned int currenttime)//Char mapRegions
{
	if (pc == NULL)
		return;
	if (pc->stablemaster_serial != INVALID_SERIAL) return;

	int pcalc;
	char t[120];

	Npcs->CheckAI(currenttime, pc);//Lag fix
	Movement->NpcMovement(currenttime, pc);//Lag fix
	setcharflag(pc);
	if (!pc->dead && pc->swingtarg==-1 )
		Combat->DoCombat(pc, currenttime);
	else if(!pc->dead && (pc->swingtarg>=0 && pc->timeout<=currenttime))
		Combat->CombatHitCheckLoS(pc,currenttime);

	Magic->CheckFieldEffects2(currenttime, pc, 0);//Lag fix

	restockNPC(currenttime, pc);

	if (!pc->free) //bud
	{
		if ((pc->disabled>0)&&((pc->disabled<=currenttime)||(overflow)))
		{
			pc->disabled=0;
		}
		if (pc->summontimer<=currenttime||(overflow))
		{
			if(pc->summontimer>0)
			{
				// Dupois - Added Dec 20, 1999
				// QUEST expire check - after an Escort quest is created a timer is set
				// so that the NPC will be deleted and removed from the game if it hangs around
				// too long without every having its quest accepted by a player so we have to remove
				// its posting from the message board before icing the NPC
				// Only need to remove the post if the NPC does not have a follow target set
				if ( (pc->questType==ESCORTQUEST) && (pc->ftarg == INVALID_SERIAL) )
				{
					MsgBoardQuestEscortRemovePost( pc );
					MsgBoardQuestEscortDelete( pc );
					return;
				}
				// Dupois - End

				soundeffect2(pc, 0x01FE);
				pc->dead=true;
				Npcs->DeleteChar(pc);
				return;
			}
		}
	}

//	if ((pc->fleeat==0)) pc->fleeat=SrvParms->npc_base_fleeat;
//	if ((pc->reattackat==0)) pc->reattackat=SrvParms->npc_base_reattackat;

	if (pc->npcWander!=5 &&	pc->hp < pc->st*pc->fleeat/100)
	{
		pc->oldnpcWander = pc->npcWander;
		pc->npcWander=5;
		pc->setNextMoveTime();
	}

	if (pc->npcWander==5 &&	pc->hp > pc->st*pc->reattackat/100)
	{
		pc->npcWander = pc->oldnpcWander;
		pc->setNextMoveTime();

		pc->oldnpcWander=0; // so it won't save this at the wsc file
	}
	// end of flee code

	// new poisoning code, Lord Binary
	if (pc->poisoned && !(pc->isInvul()) )
	{
		if ((pc->poisontime<=currenttime)||(overflow))
		{
			if (pc->poisonwearofftime>currenttime) // lb, makes poison wear off pc's
			{
				switch (pc->poisoned)
				{
				case 1:
					pc->poisontime=currenttime+(5*MY_CLOCKS_PER_SEC);
					if ((pc->poisontxt<=currenttime)||(overflow))
					{
						pc->poisontxt=currenttime+(10*MY_CLOCKS_PER_SEC);
						sprintf(t,"* %s looks a bit nauseous *",pc->name.c_str());
						pc->emotecolor = 0x0026;//buffer[s][4];
						npcemoteall(pc,t,1);
					}
					pc->hp -= RandomNum(1,2);
					updatestats(pc, 0);
					break;
				case 2:
					pc->poisontime=currenttime+(4*MY_CLOCKS_PER_SEC);
					if ((pc->poisontxt<=currenttime)||(overflow))
					{
						pc->poisontxt=currenttime+(10*MY_CLOCKS_PER_SEC);
						sprintf(t,"* %s looks disoriented and nauseous! *",pc->name.c_str());
						pc->emotecolor = 0x0026; //buffer[s][4];
						npcemoteall(pc,t,1);
					}

					pcalc = ( ( pc->hp * RandomNum(2,5) ) / 100) + RandomNum(0,2); // damage: 1..2..5% of hp's+ 1..2 constant
					pc->hp -= pcalc;
					updatestats(pc, 0);
					break;
				case 3:
					pc->poisontime=currenttime+(3*MY_CLOCKS_PER_SEC);
					if ((pc->poisontxt<=currenttime)||(overflow))
					{
						pc->poisontxt=currenttime+(10*MY_CLOCKS_PER_SEC);
						sprintf(t,"* %s is in severe pain! *",pc->name.c_str());
						pc->emotecolor = 0x0026;//buffer[s][4];
						npcemoteall(pc,t,1);
					}
					pcalc=( ( pc->hp * RandomNum(5,10) ) / 100 ) + RandomNum(1,3); // damage: 5..10% of hp's+ 1..2 constant
					pc->hp -= pcalc;
					updatestats(pc, 0);
					break; // lb !!!
				case 4:
					pc->poisontime=currenttime+(3*MY_CLOCKS_PER_SEC);
					if ((pc->poisontxt<=currenttime)||(overflow))
					{
						pc->poisontxt=currenttime+(10*MY_CLOCKS_PER_SEC);
						sprintf(t,"* %s looks extremely weak and is wrecked in pain! *",pc->name.c_str());
						pc->emotecolor = 0x0026;//buffer[s][4];
						npcemoteall(pc,t,1);
					}

					pcalc=( (pc->hp * RandomNum(10,15) ) / 100 ) + RandomNum(3,6); // damage:10 to 15% of hp's+ 3..6 constant, quite deadly <g>
					pc->hp -= pcalc;
					updatestats(pc, 0);
					break;
				default:
					clConsole.send("ERROR: Fallout of switch statement without default. wolfpack.cpp, checkNPC()\n"); //Morrolan
					pc->poisoned=0;
					return;
				}
				if (pc->hp<1)
				{
					deathstuff(pc);
				}
			} // end switch
		} // end if poison-wear off-timer
	} // end if poison-damage timer

	if ((pc->poisonwearofftime<=currenttime))
	{
		if ((pc->poisoned))
		{
			pc->poisoned=0;
			impowncreate(calcSocketFromChar(pc), pc, 1); // updating to blue stats-bar ...
		}
	}

	//hunger code for npcs
	if (SrvParams->hungerRate()>1 && (pc->hungertime<=currenttime || overflow))
	{
		t[0] = '\0';

		if (pc->hunger) pc->hunger--; //Morrolan GMs and Counselors don't get hungry

		if(pc->tamed && pc->npcaitype!=17)
		{//if tamed let's display his hungry status
			switch(pc->hunger)
			{
			case 6:
			case 5:	break;
			case 4:	sprintf(t,"* %s looks a little hungry *",pc->name.c_str());			break;
			case 3:	sprintf(t,"* %s looks fairly hungry *",pc->name.c_str());			break;
			case 2:	sprintf(t,"* %s looks extremely hungry *",pc->name.c_str());		break;
			case 1:	sprintf(t,"* %s looks weak from starvation *",pc->name.c_str());	break;
			case 0:
				//maximum hunger - untame code - AntiChrist
				//pet release code here
				if(pc->tamed)
				{
					pc->ftarg = INVALID_SERIAL;
					pc->npcWander=2;
					pc->tamed = false;
					if(pc->ownserial!=-1) 
						pc->SetOwnSerial(-1);
					sprintf((char*)temp, "* %s appears to have decided that it is better off without a master *", pc->name.c_str());
					npctalkall(pc, (char*)temp,0);
					{
						soundeffect2(pc, 0x01FE);
						if(SrvParams->tamedDisappear()==1)
						Npcs->DeleteChar(pc);
					}
				}
				//sprintf(t,"* %s must eat very soon or he will die! *",pc->name);
				break;
			}

			if(strlen(t))
			{//display message ( if there's one
				pc->emotecolor = 0x0026;//buffer[s][4];
				npcemoteall(pc,t,1);
			}
		}//if tamed
		pc->hungertime=currenttime+(SrvParams->hungerRate()*MY_CLOCKS_PER_SEC); // Bookmark
	}//if hungerrate>1
}

void checkauto() // Check automatic/timer controlled stuff (Like fighting and regeneration)
{
	//int k;
	unsigned int i;
	register unsigned int currenttime = uiCurrentTime;
	static unsigned int checkspawnregions = 0;
	static unsigned int checknpcs = 0;
	static unsigned int checktamednpcs = 0;
	static unsigned int checknpcfollow = 0;
	static unsigned int checkitemstime = 0;
	static unsigned int lighttime = 0;
	static unsigned int htmltime = 0;
	static unsigned int housedecaytimer = 0;
	static unsigned int freeUnusedMemory = 0;

	//static unsigned int repairworldtimer=0;

	if (housedecaytimer<=currenttime)
	{
		//////////////////////
		///// check_houses
		/////////////////////
		if(SrvParams->housedecay_secs() != -1)
			int check_house_decay();

		////////////////////
		// check stabling
		///////////////////

		unsigned long int diff;

		//char * t;
		//t[-1000000]=0;
		//strcpy(0, "lala"); // x-wolf test :)

		AllCharsIterator iter_char;
		   
		for (iter_char.Begin(); !iter_char.atEnd(); ++iter_char)
		{
			P_CHAR pc = iter_char.GetData();
			if (pc->npc_type == 1)
			{
				vector<SERIAL> pets( stablesp.getData(pc->serial) );
				unsigned int ci;
				for (ci = 0; ci < pets.size();ci++)
				{
					P_CHAR pc_pet = FindCharBySerial(pets[ci]);
					if (pc_pet != NULL)
					{
						diff = (getNormalizedTime() - pc_pet->timeused_last) / MY_CLOCKS_PER_SEC;
						pc_pet->time_unused+=diff;
						//clConsole.send("stabling-check debug-name: %s\n",chars[i].name);
					}
				}
			}
		}
		housedecaytimer=uiCurrentTime+MY_CLOCKS_PER_SEC*60*30; // check only each 30 minutes
	}


	if(checkspawnregions<=currenttime && SrvParams->spawnRegionCheckTime() != -1)//Regionspawns
	{
		for(i=1;i<spawnregion.size();i++)
		{
			//if (spawnregion[i].max) continue;
			if(spawnregion[i].nexttime<=currenttime)
			{
				spawnregion[i].nexttime=currenttime+(MY_CLOCKS_PER_SEC*60*RandomNum(spawnregion[i].mintime,spawnregion[i].maxtime));
				if (doregionspawn(i)) break;
			}
		}
		checkspawnregions=uiCurrentTime+SrvParams->spawnRegionCheckTime()*MY_CLOCKS_PER_SEC;//Don't check them TOO often (Keep down the lag)
	}

	if(SrvParams->html()>0 && (htmltime<=currenttime || overflow))
	{
			updatehtml();
			htmltime=currenttime+(SrvParams->html()*MY_CLOCKS_PER_SEC);
	}

	if (saveinterval != 0)
	{
		if (autosaved == 0)
		{
			autosaved = 1;
			time( (time_t*) (&oldtime)) ;
		}
		time( (time_t*) ( &newtime)) ;

		if (dosavewarning==1)
		if (difftime(newtime,oldtime)==saveinterval-10) 
		{
		   sysbroadcast("World will be saved in 10 seconds..");
		   dosavewarning = 0;
		}

		if (difftime(newtime, oldtime)>=saveinterval || cwmWorldState->Saving() )
		{
			autosaved = 0;
			cwmWorldState->savenewworld(0);
			dosavewarning = 1;
		}

		// Dupois - Added Dec 20, 1999
		//			After an automatic world save occurs, lets check to see if
		//			anyone is online (clients connected). If nobody is connected
		//			Lets do some maintenance on the bulletin boards.
	/*		if( !now )
		{
				clConsole.send("WOLFPACK: No players currently online. Starting bulletin board maintenance.\n" );
				savelog( "Bulletin Board Maintenance routine running (AUTO)\n", "server.log" );
			MsgBoardMaintenance();
		}
		*/
	}

	//Time functions
	if (uotickcount<=currenttime||(overflow))
	{
		uoTime.addSecs(1);
		uotickcount = currenttime + SrvParams->secondsPerUOMinute()*MY_CLOCKS_PER_SEC;
	}

	if(lighttime<=currenttime || (overflow))
	{
		doworldlight(); //Changes lighting, if it is currently time to.
		int i;
		for (i = 0; i < now; i++) 
			if (online(currchar[i])) 
				dolight(i, SrvParams->worldCurrentLevel()); // bandwidth fix, LB
		lighttime=currenttime+30*MY_CLOCKS_PER_SEC;
	}
	static unsigned int itemlooptime = 0;
    if(itemlooptime <=currenttime || (overflow))
	{
       itemlooptime = currenttime+5*MY_CLOCKS_PER_SEC;
	   AllItemsIterator iterItems;
       for( iterItems.Begin(); !iterItems.atEnd(); ++iterItems ) // Ripper...so spawners get set to nodecay.
	   {
			P_ITEM pi = iterItems.GetData();
			if(pi != NULL)
			{
				// lets make sure they are spawners.
				if ((pi->type>=61 && pi->type<=65) || (pi->type==69) || (pi->type==125))
				{
					// set to nodecay and refresh.
	                pi->priv=0;
		            RefreshItem(pi);
				 }
			}
		}
	}
	for(i=0;i<now;i++)
	{
		if (online(currchar[i]) && currchar[i]->account()==acctno[i])
		{

			genericCheck(currchar[i], currenttime);
			checkPC(currchar[i], currenttime);

			int	StartGrid=mapRegions->StartGrid(currchar[i]->pos);

			unsigned int increment=0;
			for (unsigned int checkgrid=StartGrid+(increment*mapRegions->GetColSize());increment<3;increment++, checkgrid=StartGrid+(increment*mapRegions->GetColSize()))
			{
				for (int a=0;a<3;a++)
				{
					cRegion::raw vecEntries( mapRegions->GetCellEntries(checkgrid + a) );
					cRegion::rawIterator it = vecEntries.begin();
					for (; it != vecEntries.end(); ++it)
					{
						P_CHAR mapchar = FindCharBySerial(*it);
						P_ITEM mapitem = FindItemBySerial(*it);
						if (mapchar != NULL)
						{//Instalog //AntiChrist
								//AntiChrist
								//so we can use two different speeds
								//for checking tamed and non-tamed npcs!
								//players WANT to have faster tamed npcs....
							if( (!mapchar->tamed && checknpcs<=currenttime) || (mapchar->tamed && checktamednpcs<=currenttime) || (mapchar->npcWander==1 && checknpcfollow<=currenttime) || overflow)
							{
								if (mapchar->isNpc()) 
									genericCheck(mapchar, currenttime); // lb, lagfix
								if (chardist(currchar[i], mapchar)<=24 && mapchar->isNpc()) //Morrolan tweak from 30 to 24 tiles
									checkNPC(mapchar, currenttime);
								else if (mapchar->isPlayer() &&
									Accounts->GetInWorld(mapchar->account()) == mapchar->serial && mapchar->logout>0 &&
									(mapchar->logout<=currenttime || (overflow)))
								{										
									Accounts->SetOffline(mapchar->account());
									mapchar->logout = 0;
									updatechar(mapchar);
								}
							}
						}
						else if (mapitem != NULL && (checkitemstime<=currenttime||(overflow))) // moroallan !!! -1 is important !!!
						{//BugFix
							Items->RespawnItem(currenttime, mapitem);
							if(mapitem->type == 51 || mapitem->type == 52) // LB !!!!
							{
								if(mapitem->gatetime<=currenttime) // LB !!!
								{
									unsigned int k;
									for (k=0;k<2;k++) 
										Items->DeleItem(mapitem); // bugfix for items disappearing
								}
							}
							Items->DecayItem(currenttime, mapitem);
							if (mapitem->type==88 && mapitem->morey<25 )
							{
								if (itemdist(currchar[i], mapitem)<=mapitem->morey)
								{
									if (RandomNum(1,100)<=mapitem->morez)
										soundeffect4(mapitem,i, mapitem->morex>>8, mapitem->morex%256);
								}
							}
						} else if (mapitem != NULL) {//Boats
							if(mapitem->type==117 &&
								(mapitem->type2==1 || mapitem->type2==2)&&
								(mapitem->gatetime<=currenttime||overflow))
							{
								if (mapitem->type2==1) 
									Boats->Move(i, mapitem->dir, mapitem);
								else 
								{
									int dir = mapitem->dir+4;
									if (dir>7) dir-=8; // LB, BUGKILLING !!!
									Boats->Move(i, dir, mapitem);
								}
								mapitem->gatetime=(unsigned int)(currenttime + (double)(SrvParams->boatSpeed()*MY_CLOCKS_PER_SEC));
							}	
						}
					}
				}
			}
		}
	}//for i<now

	AllTmpEff->Check();

	if ( freeUnusedMemory <= currenttime )
	{
		cItemsManager::getInstance()->purge();
		cCharsManager::getInstance()->purge();
		freeUnusedMemory = currenttime + MY_CLOCKS_PER_SEC*60*40; // check only each 40 minutes
	}

	if(checknpcs<=currenttime) checknpcs=(unsigned int)((double)(SrvParams->checkNPCTime()*MY_CLOCKS_PER_SEC+currenttime)); //lb
	if(checktamednpcs<=currenttime) checktamednpcs=(unsigned int)((double) currenttime+(SrvParams->checkTammedTime()*MY_CLOCKS_PER_SEC)); //AntiChrist
	if(checknpcfollow<=currenttime) checknpcfollow=(unsigned int)((double) currenttime+(SrvParams->checkFollowTime()*MY_CLOCKS_PER_SEC)); //Ripper
	if(checkitemstime<=currenttime) checkitemstime=(unsigned int)((double)(SrvParams->checkItemTime()*MY_CLOCKS_PER_SEC+currenttime)); //lb
	//if(shoprestocktime<=currenttime) shoprestocktime=currenttime+(shoprestockrate*60*MY_CLOCKS_PER_SEC);
	if(SrvParams->shopRestock()==1 && shoprestocktime<=currenttime)
	{
		shoprestocktime=currenttime+(shoprestockrate*60*MY_CLOCKS_PER_SEC);
		Trade->restock(0);
	}
	if (nextnpcaitime <= currenttime)
		nextnpcaitime = (unsigned int)((double) currenttime + (SrvParams->checkAITime()*MY_CLOCKS_PER_SEC)); // lb
	if (nextfieldeffecttime <= currenttime)
		nextfieldeffecttime = (unsigned int)((double) currenttime + (0.5*MY_CLOCKS_PER_SEC));
	if (nextdecaytime <= currenttime)
		nextdecaytime = currenttime + (15*MY_CLOCKS_PER_SEC); // lb ...
	if (SrvParams->autoAccountReload() > 0 && Accounts->lasttimecheck + (SrvParams->autoAccountReload()*60*MY_CLOCKS_PER_SEC) <= currenttime)
		Accounts->CheckAccountFile();

	// Do Eclipse stuff.. /blackwind 
/*	if (SrvParms->eclipsetimer>0) 
	{ 
		if (SrvParms->eclipsetimer <= uiCurrentTime) 
		{ 
			SrvParms->eclipsetimer = (unsigned int)((double) uiCurrentTime +(ECLIPSETIMER*MY_CLOCKS_PER_SEC)); 
			if (SrvParms->eclipsemode)
				SrvParams->worldCurrentLevel()--;
			else 
				SrvParams->worldCurrentLevel()++; 
			setabovelight(SrvParams->worldFixedLevel()); 
			if ((SrvParams->worldFixedLevel()>20) ||(SrvParams->worldFixedLevel() < 1)) 
			{ 
				SrvParms->eclipsetimer = 0; 
				server_data.eclipsemode=!server_data.eclipsemode; 
			} 
		} 
	} // end eclipse / blackwind..
*/
}
