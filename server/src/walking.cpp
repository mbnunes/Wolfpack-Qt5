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
//	Wolfpack Homepage: http://www.wpdev.com/
//========================================================================================


#include "walking.h"
#undef  DBGFILE
#define DBGFILE "walking.cpp"

///////////////
// Name:	getLeftDir, getRightDir
// history:	by Duke, 14.7.2001
// Purpose:	calculates the adjacent direction
//
int getLeftDir(int dir)	// counterclockwise
{
	dir &= 7;	// make sure it's valid
	return dir==0 ? 7 : dir-1;
}
int getRightDir(int dir)	// clockwise
{
	dir &= 7;
	return dir==7 ? 0 : dir+1;
}

///////////////
// Name:	getXYfromDir
// history:	by Duke, 14.7.2001
// Purpose:	calculates and changes the given coords one step into the given direction
//
void getXYfromDir(int dir, int *x, int *y)
{
	switch(dir&0x07)
	{
	case 0: (*y)--;				break;
	case 1: (*x)++; (*y)--;		break;
	case 2: (*x)++;				break;
	case 3: (*x)++; (*y)++;		break;
	case 4: (*y)++;				break;
	case 5: (*x)--; (*y)++;		break;
	case 6: (*x)--;				break;
	case 7: (*x)--; (*y)--;		break;
	}
}

///////////////
// Name:	WalkHandle
// history:	cut from walking() by Duke, 27.10.2000
// Purpose:	dummy
//
bool WalkHandle(UOXSOCKET s, CHARACTER c)
{
	return true;
}

///////////////
// Name:	WalkHandleAllowance
// history:	cut from walking() by Duke, 27.10.2000
// Purpose:	checks if the Char is allowed to move at all i.e. not frozwn, overloaded...
//
bool WalkHandleAllowance(UOXSOCKET s, CHARACTER c, int sequence)
{

	P_CHAR pc = MAKE_CHARREF_LRV(c, false);
	
	if (pc->stablemaster_serial>0) return false; // shouldnt be called for stabled pets, just to be on the safe side

	if(s!=-1)
	{
		if ((walksequence[s]+1!=sequence)&&(sequence!=256))
		{
			deny(s, c, sequence);  
			return false;
		}
	}

	if (!pc->dead && pc->isPlayer() && !(pc->isGMorCounselor()))
	{
		if (!Weight->CheckWeight(c, s) || (pc->stm<2)) // this has to executes, even if s==-1, thus that many !=-1's
		{
			if (s!=-1)
			{
			  sysmessage(s, "You are too fatigued to move.");
			  walksequence[s]=-1;
			  teleport(c);
			  return false;
			}
		}
	}

	// dont put a if s!=-1 here !!!
	if (pc->priv2&2) // lord binary !!! 
	{			
		teleport(c);
		if (s>-1)
		{
			if (pc->casting)
				sysmessage(s, "You cannot move while casting.");
		    else
				sysmessage(s, "You are frozen and cannot move.");
		}
		return false;
	} 
	return true;
}

///////////////
// Name:	WalkHandleRunning
// history:	cut from walking() by Duke, 27.10.2000
// Purpose:	handles running, stealth etc.
//
bool WalkHandleRunning(UOXSOCKET s, CHARACTER c, int dir)
{
	P_CHAR pc = MAKE_CHARREF_LRV(c, true);
	if (dir&0x80) 
	{ //AntiChrist -- if running
		//AntiChrist - we have to unhide always if running
		pc->unhide();
		//Running stamina - AntiChrist
		pc->regen2=uiCurrentTime+(SrvParms->staminarate*MY_CLOCKS_PER_SEC); //Don't regenerate stamina while running
		pc->running++;

		// Horse Stamina loss bug Fixed by blackwind. 
		// Without loss of stamina players will be able to runaway for ever..
		if (!pc->dead)
		{
			if(((!pc->onhorse && pc->running>(SrvParms->runningstaminasteps)*2)) ||
				(pc->onhorse && pc->running>(SrvParms->runningstaminasteps*2)*2) )
			{ //The first *2 it's because i noticed that a step(animation) correspond to 2 walking calls
				pc->running=0;
				pc->stm--;
				updatestats(c,2);
			}
		}
	}
	else 
	{
		pc->running=0;
		if(pc->stealth>-1) 
		{ //AntiChrist - Stealth
			pc->stealth++;
			if(pc->stealth > ((SrvParms->maxstealthsteps*pc->skill[STEALTH])/1000))
			{
				pc->unhide();
			}
		}
		else
		{
			pc->unhide();
		}
	}
	/*if(Skills->GetCombatSkill(c) ==ARCHERY)  // -Frazurbluu- add in changes for archery skill, and dexterity
    {                                        //  possibly weapon speed?? maybe not, cause crossbows notta running shooting   
            if (chars[c].targ!= -1)
            {
                   if(chars[c].timeout>= uiCurrentTime)
                           chars[c].timeout= uiCurrentTime + (3*CLOCKS_PER_SEC);// need to tweek the delay and have check dex

            }
    }*/
	return true;
}

///////////////
// Name:	WalkCollectBlockers
// history:	cut from walking() by Duke, 20.11.2000
// Purpose:	Collects Landscape plus static and dynamic items in an array
//
// Remark LB 14'th nov 2000: will rewrite this stuff when I have time again for wp-project...
// dont delete the rem'd out clConsole.send's in meanwhile, thx !
//
void WalkCollectBlockers(UOXSOCKET s, CHARACTER c)
{
	P_CHAR pc = MAKE_CHARREF_LR(c);	
	xycount=0;

	int mapid = 0;
	signed char mapz = Map->AverageMapElevation(pc->pos.x, pc->pos.y, mapid);
	if (mapz != illegal_z)
	{
		land_st land;
		Map->SeekLand(mapid, &land);
		
		xyblock[xycount].type=0;
		xyblock[xycount].basez = mapz;
		xyblock[xycount].id = mapid;
		xyblock[xycount].flag1=land.flag1;
		xyblock[xycount].flag2=land.flag2;
		xyblock[xycount].flag3=land.flag3;
		xyblock[xycount].flag4=land.flag4;
		xyblock[xycount].height=0;
		xyblock[xycount].weight=255;
		xycount++;
	}

	unsigned int StartGrid=mapRegions->StartGrid(pc->pos.x,pc->pos.y);
	unsigned int getcell=mapRegions->GetCell(pc->pos.x,pc->pos.y);
	unsigned int increment=0, checkgrid, a;
	for (checkgrid=StartGrid+(increment*mapRegions->GetColSize());increment<3;increment++, checkgrid=StartGrid+(increment*mapRegions->GetColSize()))
	{
		for (a=0;a<3;a++)
		{
			vector<SERIAL> vecEntries = mapRegions->GetCellEntries(checkgrid+a);
			for ( unsigned int k = 0; k < vecEntries.size(); k++)
			{
				P_ITEM pi = FindItemBySerial(vecEntries[k]);
				if (pi != NULL)
				{
					if (pi->id1<0x40)
					{
						if ((pi->pos.x==pc->pos.x)&&(pi->pos.y==pc->pos.y))
						{
							if (pi->trigger!=0)
							{
								if ((pi->trigtype==1)&&(!pc->dead))
								{
									if (pi->disabled<=uiCurrentTime)//AntiChrist
									{
										triggerwitem(s,DEREF_P_ITEM(pi),1);  //When player steps on a trigger
									}
								}
							}
							tile_st tile;
							Map->SeekTile(pi->id(), &tile);
							xyblock[xycount].type=1;
							xyblock[xycount].basez=pi->pos.z;
							xyblock[xycount].id=pi->id();
							xyblock[xycount].flag1=tile.flag1;
							xyblock[xycount].flag2=tile.flag2;
							xyblock[xycount].flag3=tile.flag3;
							xyblock[xycount].flag4=tile.flag4;
							xyblock[xycount].height=tile.height;
							xyblock[xycount].weight=tile.weight;
							xycount++;
						}
					}
					else
					{
						if (
							(abs(pi->pos.x-pc->pos.x)<=BUILDRANGE)&&
							(abs(pi->pos.y-pc->pos.y)<=BUILDRANGE)
							)
						{
							UOXFile *mfile = NULL;
							SI32 length = 0;		// should be SI32, not long
							Map->SeekMulti(pi->id()-0x4000, &mfile, &length);
							length=length/MultiRecordSize;
							if (length == -1 || length>=17000000)//Too big... bug fix hopefully (Abaddon 13 Sept 1999)
					//								if (length == -1)
							{
								//clConsole.send("walking() - Bad length in multi file. Avoiding stall.\n");
								length = 0;
							}
							int j;
							for (j=0;j<length;j++)
							{
								st_multi multi;
								mfile->get_st_multi(&multi);
								if (multi.visible && (pi->pos.x+multi.x == pc->pos.x) && (pi->pos.y+multi.y == pc->pos.y))
								{
									tile_st tile;
									Map->SeekTile(multi.tile, &tile);
									xyblock[xycount].type=2;
									xyblock[xycount].basez=multi.z+pi->pos.z;
									xyblock[xycount].id=multi.tile;
									xyblock[xycount].flag1=tile.flag1;
									xyblock[xycount].flag2=tile.flag2;
									xyblock[xycount].flag3=tile.flag3;
									xyblock[xycount].flag4=tile.flag4;
									xyblock[xycount].height=tile.height;
									xyblock[xycount].weight=255;
									xycount++;
								}
							}
						}
					}
				}
			}
		}
	}

	MapStaticIterator msi(pc->pos.x, pc->pos.y);
	staticrecord *stat;
	unsigned long loopexit = 0;
	while ( (stat = msi.Next())  && (++loopexit < MAXLOOPS))
	{
		//clConsole.send("staticr[X] type=%d, id=%d\n", 2, stat->itemid);
		tile_st tile;
		msi.GetTile(&tile);
		xyblock[xycount].type=2;
		xyblock[xycount].basez=stat->zoff;
		xyblock[xycount].id=stat->itemid;
		xyblock[xycount].flag1=tile.flag1;
		xyblock[xycount].flag2=tile.flag2;
		xyblock[xycount].flag3=tile.flag3;
		xyblock[xycount].flag4=tile.flag4;
		xyblock[xycount].height=tile.height;
		xyblock[xycount].weight=255;
		xycount++;
	}								
}

///////////////
// Name:	WalkEvaluateBlockers
// history:	cut from walking() by Duke, 20.11.2000
// Purpose:	Decides if something in the array blocks the walker
//
// Remark LB 14'th nov 2000: will rewrite this stuff when I have time again for wp-project...
// dont delete the rem'd out clConsole.send's in meanwhile, thx !
//
void WalkEvaluateBlockers(UOXSOCKET s, CHARACTER c, signed char *pz, signed char *pdispz)
{
	signed char z, oldz, seekz,dispz;
	int num;
	int i;
	char gmbody;
	z=-128;

	P_CHAR pc = MAKE_CHARREF_LR(c);

	oldz=pc->pos.z;
	
	/*if (
	((pc->id1==0x03)&&(pc->id2==0xDB)) ||
	((pc->id1==0x01)&&(pc->id2==0x92)) ||
	((pc->id1==0x01)&&(pc->id2==0x93))
	) gmbody=1; else gmbody=0;*/
	//AntiChrist - with the old way..polymorphed gms are like normal players!
	//Why it shouldn't check for priv&1?! well...let's see if there
	//is some side effect using the priv&1 method
	if(pc->isGM()) gmbody=1; else gmbody=0;
	if(pc->dead) gmbody=1; else gmbody=0;//ghosts can walk trhou doors
	
	unsigned long loopexit=0;
	
	
	do
	{
		seekz=127;
		num=-1;
		for (i=0;i<xycount;i++)
		{
			if ((xyblock[i].basez+xyblock[i].height)<seekz)
			{
				num=i;
				seekz=xyblock[i].basez+xyblock[i].height;
			}
		}
		
		if (num==-1)
		{
#ifdef DEBUG
			clConsole.send("(walking) Error?\n");
#endif
			xycount=0;
			z=-128;
		}
		else
		{
			// clConsole.send("xyBaseZ: %i, xyHeight: %i, xyType: %i\n", xyblock[num].basez, xyblock[num].height, xyblock[num].type);
			// clConsole.send("xyid: %i, oldz: %i, cpriv: %i cpriv2: %i\n", xyblock[num].id, oldz, pc->priv, pc->priv2);
			// clConsole.send("xyflag1: %i, xyflag2: %i, xyflag3: %i, xyflag4: %i\n", xyblock[num].flag1,xyblock[num].flag2,xyblock[num].flag3,xyblock[num].flag4);
			// clConsole.send("gmbody: %i, xyweight: %i\n",gmbody,xyblock[num].weight);
			
			//clConsole.send("seekz: %i\n",seekz);
			
			//if((pc->account!=-1))
			//{
			//	clConsole.send("flag1, b1:%i b2:%i b3:%i b4:%i b5:%i b6:%i b7:%i b8:%i\n ",xyblock[num].flag1&0x1,xyblock[num].flag1&0x2,xyblock[num].flag1&0x4,xyblock[num].flag1&0x8,xyblock[num].flag1&0x10,xyblock[num].flag1&0x20,xyblock[num].flag1&0x40,xyblock[num].flag1&0x80);
			//	clConsole.send("flag2, b1:%i b2:%i b3:%i b4:%i b5:%i b6:%i b7:%i b8:%i\n ",xyblock[num].flag2&0x1,xyblock[num].flag2&0x2,xyblock[num].flag2&0x4,xyblock[num].flag2&0x8,xyblock[num].flag2&0x10,xyblock[num].flag2&0x20,xyblock[num].flag2&0x40,xyblock[num].flag2&0x80);
			//	clConsole.send("flag3, b1:%i b2:%i b3:%i b4:%i b5:%i b6:%i b7:%i b8:%i\n ",xyblock[num].flag3&0x1,xyblock[num].flag3&0x2,xyblock[num].flag3&0x4,xyblock[num].flag3&0x8,xyblock[num].flag3&0x10,xyblock[num].flag3&0x20,xyblock[num].flag3&0x40,xyblock[num].flag3&0x80);
			//	clConsole.send("flag4, b1:%i b2:%i b3:%i b4:%i b5:%i b6:%i b7:%i b8:%i\n ",xyblock[num].flag4&0x1,xyblock[num].flag4&0x2,xyblock[num].flag4&0x4,xyblock[num].flag4&0x8,xyblock[num].flag4&0x10,xyblock[num].flag4&0x20,xyblock[num].flag4&0x40,xyblock[num].flag4&0x80);
			//}
			
			
			if (xyblock[num].type!=0)
			{
				if (xyblock[num].height==0) xyblock[num].height++;
				if ((!(xyblock[num].id==1))&&(xyblock[num].basez<=oldz+MaxZstep))
				{
					if ((xyblock[num].flag1&0x80)&&(!((pc->isGM())||(pc->dead))))
					{
						//clConsole.send("A\n");
						z=-128;
					}
					else
					{
						if (gmbody)
						{
							if ( ((xyblock[num].weight==255)&&(!(pc->priv2&1))) || (xyblock[num].type==2) )
								if ( (xyblock[num].weight==255) || (xyblock[num].type==2) )
								{
									if (xyblock[num].flag2&0x04)
									{													
										if (xyblock[num].basez<oldz+MaxZstep)
										{
											//clConsole.send("x3: seekz: %i\n",seekz);
											z=xyblock[num].basez+xyblock[num].height;
											dispz=xyblock[num].basez+(xyblock[num].height/2);
											
										}
										else
										{
											if ((pc->isGM())||(pc->dead)||(xyblock[num].flag4&0x20))
											{
												//clConsole.send("x4: seekz: %i\n",seekz);
												dispz=z=xyblock[num].basez;
											}
											else
											{
												//clConsole.send("B\n");
												z=-128;
											}
										}
									}
									else
									{
										if (xyblock[num].basez+xyblock[num].height<oldz+MaxZstep)
										{
											//clConsole.send("x1: seekz: %i\n",seekz);
											dispz=z=xyblock[num].basez+xyblock[num].height;
										}
										else
										{
											if ((pc->isGM())||(pc->dead)||(xyblock[num].flag4&0x20))
											{
												//clConsole.send("x2: seekz: %i\n",seekz);
												dispz=z=xyblock[num].basez;
											}
											else
											{
												z=-128;
											}
										}
									}
								}
						}
						else
						{
							//clConsole.send("xyblock[num].flag1=%i xyblock[num].flag2=%i chars[c]priv=%i\n",xyblock[num].flag1, xyblock[num].flag2, pc->priv);
							if ((xyblock[num].flag1&0x40)/*&&(!(xyblock[num].flag1&1)))*/&&(!(pc->isGM()))) // lb, castle walk bugfix without sideffects with very.
								
							{
								//clConsole.send("D (ID=%x)\n", xyblock[num].id);
								if (xyblock[num].basez+xyblock[num].height>pc->pos.z)
									z=-128;
							}
							else
							{
								if ((xyblock[num].flag2&4))
								{
									//clConsole.send("ladder-z: %i height: %i\n", xyblock[num].basez,xyblock[num].height);
									if (xyblock[num].basez<oldz+MaxZstep)
									{
										//clConsole.send("ladder-z2: %i height: %i seekz: %i\n", xyblock[num].basez,xyblock[num].height,seekz);
										z=xyblock[num].basez+xyblock[num].height;
										dispz=xyblock[num].basez+(xyblock[num].height/2);
									}
									else
									{													
										if (pc->isGM())
											
										{
											//clConsole.send("x4: seekz: %i\n",seekz);
											
											dispz=z=xyblock[num].basez;
										}
										else
										{
											//clConsole.send("E\n");
											z=-128;
										}
									}
								}
								else
								{
									if (xyblock[num].basez+xyblock[num].height<oldz+MaxZstep)
									{
										//clConsole.send("x5: seekz: %i base: %i height: %i oldz: %i\n",seekz, xyblock[num].basez, xyblock[num].height, oldz);
										dispz=z=xyblock[num].basez+xyblock[num].height;
									}
									else
									{
										if (pc->isGM())
										{
											// clConsole.send("x6: seekz: %i\n",seekz);
											dispz=z=xyblock[num].basez;
										}
										else
										{
											if (xyblock[num].flag2&0x20)
											{
												//clConsole.send("F: %x %x %x %x\n", xyblock[num].flag1, xyblock[num].flag2, xyblock[num].flag3, xyblock[num].  flag4);
												z=-128;
											}
										}
									}
								}
							}
						}
					}
				}
			}
			else
			{
				if (((xyblock[num].flag1&0x80)||(xyblock[num].flag1&0x40))&&(!(pc->isGM())))
				{
					//clConsole.send("G\n");
					if(s==-1) z=-128; // changed from c to s, LB 16.9.99
				}
				else
				{
					if ((z==-128)||(xyblock[num].basez+xyblock[num].height<oldz+MaxZstep))
					{
						//clConsole.send("x7: seekz: %i\n",seekz);
						dispz=z=xyblock[num].basez;
					}
				}
			}
			
			if (xycount==0) break;
			memcpy(&xyblock[num],&xyblock[xycount-1],sizeof(unitile_st));
			
			xycount--;
		}
	}
	while (xycount>0  && (++loopexit < MAXLOOPS) );
	
	*pz=z; *pdispz=dispz;
}

///////////////
// Name:	WalkHandleBlocking
// history:	cut from walking() by Duke, 27.10.2000
// Purpose:	Handles a 'real move' if the Char is not only changing direction
//
// Remark LB 14'th nov 2000: will rewrite this stuff when I have time again for wp-project...
// dont delete the rem'd out clConsole.send's in meanwhile, thx !

bool WalkHandleBlocking(UOXSOCKET s, CHARACTER c, int sequence, int dir, int oldx, int oldy)
{
	P_CHAR pc = MAKE_CHARREF_LRV(c, false);
	if (pc->isNpc())
		pc->setNextMoveTime(16); //reset move timer
		
		switch(dir&0x0F)
		{
		case '\x00' : pc->pos.y--;
			break;
		case '\x01' : { pc->pos.x++; pc->pos.y--; }
			break;
		case '\x02' : pc->pos.x++;
			break;
		case '\x03' : { pc->pos.x++; pc->pos.y++; }
			break;
		case '\x04' : pc->pos.y++;
			break;
		case '\x05' : { pc->pos.x--; pc->pos.y++; }
			break;
		case '\x06' : pc->pos.x--;
			break;
		case '\x07' : { pc->pos.x--; pc->pos.y--; }
			break;
		default:
			clConsole.send("ERROR: Fallout of switch statement without default. wolfpack.cpp, walking()\n"); //Morrolan
			clConsole.send("Caused by character %s. dir: %i dir&0x0f: %i dir-passed : %i dp&0x0f : %i\n",pc->name,pc->dir,pc->dir&0x0f,dir,dir&0x0f);
			// sysmessage(calcSocketFromChar(c), "Dont desparate, I'm working on that damned fu**** bug, Lord Binary");
			// pc->dir=0;
			// teleport(c);
			if (s!=-1) deny(s, c, sequence); // lb, crashfix
			return false;
		}

	WalkCollectBlockers(s, c);

	signed char z, dispz=0;

	WalkEvaluateBlockers(s, c, &z, &dispz);
	
	// check if player is banned from a house - crackerjack 8/12/99
	int j;

	if (pc->isPlayer()) // this is also called for npcs .. LB
	{
		j=HouseManager->GetHouseNum(pc);
		if(j>=0)
		{
			int b=House[j]->FindBan(pc)
			if(b>=0) 
			{
				sysmessage(s, "You are banned from that location.");
				pc->pos.x=House[j]->x2+1;
				pc->pos.y=House[j]->y2+1;
				teleport(c);
				return false;
			}
		}
	} // end of is_house
	 // end of is_multi
	// end of is player
	if (z==-128)
	{
		pc->pos.x=oldx;
		pc->pos.y=oldy;
		if (s!=-1) deny(s, c, sequence);
		return false;
	}
	
	int nowx2,nowy2;
	
	//Char mapRegions
	
	nowx2=pc->pos.x;
	nowy2=pc->pos.y;
	pc->pos.x=oldx;
	pc->pos.y=oldy; // we have to remove it with OLD x,y ... LB, very important
	pc->MoveTo(nowx2,nowy2,z);
	
	return true;
}

///////////////
// Name:	WalkSendToPlayers
// history:	cut from walking() by Duke, 27.10.2000
// Purpose:	checks if the Char is allowed to move at all i.e. not frozwn, overloaded...
//
bool WalkSendToPlayers(UOXSOCKET s, CHARACTER c, int dir, int oldx, int oldy, int newx, int newy)
{
	P_CHAR pc = MAKE_CHARREF_LRV(c, true);
	for (int i = 0; i < now; i++)
	{
		if (perm[i])
		{
			if ((inrange1p(c, currchar[i])) && (perm[i]))
			{
				if (
					(((abs(newx - chars[currchar[i]].pos.x) == Races[pc->race]->VisRange) || (abs(newy - chars[currchar[i]].pos.y) == Races[pc->race]->VisRange)) &&
					((abs(oldx - chars[currchar[i]].pos.x)>Races[pc->race]->VisRange) || (abs(oldy - chars[currchar[i]].pos.y)>Races[pc->race]->VisRange))) ||
					((abs(newx - chars[currchar[i]].pos.x) == Races[pc->race]->VisRange) && (abs(newy - chars[currchar[i]].pos.y) == Races[pc->race]->VisRange))
					)
				{
					impowncreate(i, c, 1);
				}
				else
					//    if ((abs(newx-chars[currchar[i]].x)<VISRANGE)||(abs(newy-chars[currchar[i]].y)<VISRANGE))
				{
					bool sendit;
					if ((pc->isHidden() || (pc->dead && !pc->war && !chars[currchar[s]].dead)) && c != currchar[i] && !chars[currchar[i]].isGM() )
						sendit = false;
					else 
						sendit = true; // the freaking ( for the priv && is important ... stupid c[programmers]
					// clConsole.send("notyourself %i walker-hidden: %i sendit %i send-to-priv %i \n",c!=currchar[i],pc->hidden,sendit,chars[currchar[i]].priv&0x01);
					
					if (sendit)
					{
						extmove[1] = pc->ser1;
						extmove[2] = pc->ser2;
						extmove[3] = pc->ser3;
						extmove[4] = pc->ser4;
						extmove[5] = pc->id1;
						extmove[6] = pc->id2;
						extmove[7] = pc->pos.x >> 8;
						extmove[8] = pc->pos.x%256;
						extmove[9] = pc->pos.y >> 8;
						extmove[10] = pc->pos.y%256;
						extmove[11] = pc->dispz;
						extmove[12] = dir;
						
						// running stuff
						if (pc->isNpc() && pc->war) // Skyfire
						{
							extmove[12] = dir | 0x80;
							// Xsend(i, extmove, 17);
						}
						if (pc->isNpc() && pc->ftarg!=-1)
						{
							extmove[12] = dir | 0x80;
							// Xsend(i, extmove, 17);
						}
						
						//** Lb's flying creatures stuff ***/
						
						if (pc->isNpc())
						{
							int b, cr, d;
							b = (pc->id1 << 8) + pc->id2;
							cr = (creatures[b].who_am_i)&0x1; // can it fly ?
							if (cr == 1)
							{
								if (pc->fly_steps>0)
								{
									pc->fly_steps--;
									extmove[12] |= 0x80; // run mode = fly for that ones that can fly
								}
								else
								{
									if (fly_p != 0)
										d = rand()%fly_p;
									else 
										d = 0;
									if (d == 0) 
									{
										if (fly_steps_max != 0)
											pc->fly_steps = (rand()%fly_steps_max) + 2; 
										else 
											pc->fly_steps += 2;
									}
								}
							}
						}
						
						
						ShortToCharPtr(pc->skin, &extmove[13]);
						P_CHAR pc_check = MAKE_CHAR_REF(currchar[i]);
						if (pc->war)
							extmove[15] = 0x40;
						else 
							extmove[15] = 0x00;
						if (pc->isHidden())
							extmove[15] |= 0x80;
						if (pc->dead && !pc->war)
							extmove[15] |= 0x80; // Ripper
						if (pc->poisoned)
							extmove[15] |= 0x04; // AntiChrist -- thnx to SpaceDog
						int guild,race;
						guild = Guilds->Compare(c, currchar[i]);
						race = RaceManager->CheckRelation(pc,pc_check);
						if (guild == 1 || race == 1)// Same guild (Green)
							extmove[16] = 2;
						else if (guild == 2 || race == 2) // Enemy guild.. set to orange
							extmove[16] = 5;
						else
						{
							if (pc->isMurderer())
								extmove[16] = 6;		// If a bad, show as red.
							else if (pc->isInnocent())
								extmove[16] = 1;		// If a good, show as blue.
							else if (pc->flag == 0x08)
								extmove[16] = 2;		// green (guilds)
							else if (pc->flag == 0x10)
								extmove[16] = 5;		// orange (guilds)
							else  
								extmove[16] = 3;		// grey
						}
						if (currchar[i] != c)
							Xsend(i, extmove, 17);
					} // end of it sendit
		  }
	  }
    }
  }
  return true;
}

///////////////
// Name:	WalkHandleCharsAtNewPos
// history:	cut from walking() by Duke, 27.10.2000
// Purpose:	sends the newly visible Chars to the screen and checks for shoving
//
bool WalkHandleCharsAtNewPos(UOXSOCKET s, CHARACTER c, int oldx, int oldy, int newx, int newy)
{
	P_CHAR pc = MAKE_CHARREF_LRV(c, true);
	cRegion::RegionIterator4Chars ri(pc->pos);
	for (ri.Begin(); ri.GetData() != ri.End(); ri++)
	{
		P_CHAR pc_i = ri.GetData();
		if (pc_i != NULL)
		{
			if (pc_i->stablemaster_serial<=0) // dont send stabled pets to let them appear invisible
			{

			//clConsole.send("DEBUG: Mapchar %i [%i]\n",mapchar,mapitem);
					 
			if ((online(DEREF_P_CHAR(pc_i))||pc_i->isNpc())||(pc->isGM()))//Let GMs see logged out players
			{
				if (
					(((abs(newx-pc_i->pos.x)==Races[pc_i->race]->VisRange)||(abs(newy-pc_i->pos.y)==Races[pc_i->race]->VisRange)) &&
					((abs(oldx-pc_i->pos.x)>Races[pc_i->race]->VisRange)||(abs(oldy-pc_i->pos.y)>Races[pc_i->race]->VisRange))) ||
					((abs(newx-pc_i->pos.x)==Races[pc_i->race]->VisRange)&&(abs(newy-pc_i->pos.y)==Races[pc_i->race]->VisRange))
					)
				{
					impowncreate(s, DEREF_P_CHAR(pc_i), 1);
				}
			}
			if (!(
				((pc->id1==0x03)&&(pc->id2==0xDB)) ||
				((pc->id1==0x01)&&(pc->id2==0x92)) ||
				((pc->id1==0x01)&&(pc->id2==0x93)) ||
				(pc->isGMorCounselor())
				))
			{
				if (DEREF_P_CHAR(pc_i)!=c && (online(DEREF_P_CHAR(pc_i)) || pc_i->isNpc()))
				{
					if (pc_i->pos.x==pc->pos.x && pc_i->pos.y==pc->pos.y && pc_i->pos.z==pc->pos.z)
					{
						if (pc_i->isHidden() && !pc_i->dead && (!(pc_i->isInvul())) &&(!(pc_i->isGM())))
						{
							if (s!=-1) sysmessage(s, "Being perfectly rested, you shoved something invisible out of the way.");
								pc->stm=max(pc->stm-4, 0);
								updatestats(c, 2);  // arm code
							}
							else if (!pc_i->isHidden() && !pc_i->dead && (!(pc_i->isInvul())) &&(!(pc_i->isGM()))) // ripper..GMs and ghosts dont get shoved.)
							{
								if (s!=-1) sysmessage(s,"Being perfectly rested, you shove %s out of the way.", pc_i->name);
									  
								pc->stm=max(pc->stm-4, 0);
								updatestats(c, 2);  // arm code
							}
							else if(!pc_i->isGMorCounselor() && !pc_i->isInvul())//A normal player (No priv1(Not a gm))
							{
								if (s!=-1) sysmessage(s, "Being perfectly rested, you shove something invisible out of the way.");
								pc->stm=max(pc->stm-4, 0);
								updatestats(c, 2);  // arm code
							}
						}
					}
				}
			}
		}
	}
	return true;
}

///////////////
// Name:	WalkHandleItemsAtNewPos
// history:	cut from walking() by Duke, 27.10.2000
// Purpose:	sends the newly visible items to the screen and checks for item effects
//
bool WalkHandleItemsAtNewPos(UOXSOCKET s, CHARACTER c, int oldx, int oldy, int newx, int newy)
{
	int start_x, scan_x,scan_y;

	P_CHAR pc = MAKE_CHARREF_LRV(c, true);
	
	P_ITEM pi;
	unsigned long loopexit=0;

	unsigned int StartGrid=mapRegions->StartGrid(pc->pos.x, pc->pos.y);
	unsigned int getcell=mapRegions->GetCell(pc->pos.x,pc->pos.y);
	unsigned int increment=0, checkgrid, a;
	for (checkgrid=StartGrid+(increment*mapRegions->GetColSize());increment<3;increment++, checkgrid=StartGrid+(increment*mapRegions->GetColSize()))
	{
		for (a=0;a<3;a++)
		{
			vector<SERIAL> vecEntries = mapRegions->GetCellEntries(checkgrid+a);
			for ( unsigned int k = 0; k < vecEntries.size(); k++)
			{
				pi = FindItemBySerial(vecEntries[k]);
				if (pi != NULL)
				{
					if (pi->pos.x==pc->pos.x && pi->pos.y==pc->pos.y && pi->pos.z==pc->pos.z)
					{
						if ( pi->id()==0x3996 || pi->id()==0x398C )		//Fire Field
						{
							if (!Magic->CheckResist(-1, c, 4))
							{						
								Magic->MagicDamage(c, pi->morex/300);
							}
							soundeffect2(c, 2, 8);
						}
				
						if ( pi->id()==0x3915 || pi->id()==0x3920 )		//Poison field
						{
							if (!Magic->CheckResist(-1, c, 5))
							{						
								Magic->PoisonDamage(c,1);
							}
							soundeffect2(c, 2, 8);
						}
			
						if ( pi->id()==0x3979 || pi->id()==0x3967 )		//Para Field
						{
							if (!Magic->CheckResist(-1, c, 6))
							{
								tempeffect(c, c, 1, 0, 0, 0);
							}
							soundeffect2(c, 0x02, 0x04);
						}
					}
		
					if ( pi->id()>=0x407C && pi->id()<=0x407E )
					{
						int di=dist(pi->pos, pc->pos);
						if (di<=BUILDRANGE && di>=VISRANGE)								
						{			
							senditem(s, pi);
						}
			
					}	
		
		
					// Item-resend-rewritten by Lord Binary, 4.Nov.1999
					// the old code send about 500 items per new step (if screen is full of items)
					// though there are only max 32 that are really new.
					// this was a bandwidth waste and causzed client freezes and slowdown in extreme cases
					// plz dont touch, its VERY WELL tested.
					// it could be written a bit more compact, but that has no  influence on performance
					// so again, dont touch :)							
		
					switch (pc->dir)
					{
					case 7: // NW
						start_x=pc->pos.x-14;
						scan_y=pc->pos.y+2;								      
			
						for (scan_x=start_x;scan_x<=start_x+15;scan_x++) // find all items that have to be resend
						{
							if (pi->pos.x==scan_x || pi->pos.x==scan_x+1)
							{
								if (pi->pos.y==scan_y)
								{
									senditem(s,pi);
								}
							}
							scan_y--;
						}
						break;
			
					case 3: // SE
						start_x=pc->pos.x-2;
						scan_y=pc->pos.y+14;
			
						for (scan_x=start_x;scan_x<=start_x+15;scan_x++) // find all items that have to be resend
						{
							if (pi->pos.x==scan_x || pi->pos.x==scan_x+1)
							{
								if (pi->pos.y==scan_y)
								{		
									senditem(s,pi);
								}
							}
							scan_y--;
						}									   
						break;
			
					case 5: // SW
						start_x=pc->pos.x-14;
						scan_y=pc->pos.y+2;
			
						for (scan_x=start_x;scan_x<=start_x+15;scan_x++) // find all items that have to be resend
						{
							if (pi->pos.x==scan_x)
							{
								if (pi->pos.y==scan_y || pi->pos.y==scan_y+1)
								{											  
									senditem(s,pi);
								}
							}
							scan_y++;
						}
						break;
			
			
					case 1: // NE
						start_x=pc->pos.x+2;
						scan_y=pc->pos.y-14;
			
						for (scan_x=start_x;scan_x<=start_x+15;scan_x++) // find all items that have to be resend
						{
							if (pi->pos.x==scan_x)
							{
								if (pi->pos.y==scan_y || pi->pos.y==scan_y+1)
								{		  
									senditem(s,pi);
								}
							}
							scan_y++;
						}
			
						break;
			
					case 4: //S
						start_x=pc->pos.x-14;
						scan_y=pc->pos.y+2;
			
						for (scan_x=start_x;scan_x<=start_x+15;scan_x++) // find all items that have to be resend
						{
							if (pi->pos.x==scan_x)
							{
								if (pi->pos.y==scan_y)
								{											  										   
									senditem(s,pi);
								}
							}
						scan_y++;
						}
			
						start_x=pc->pos.x-2;
						scan_y=pc->pos.y+14;
			
						for (scan_x=start_x;scan_x<=start_x+15;scan_x++) // find all items that have to be resend
						{
							if (pi->pos.x==scan_x)
							{
								if (pi->pos.y==scan_y)
								{				   
									senditem(s,pi);
								}
							}
							scan_y--;
						}		
			
						break;
			
					case 0: // N
						start_x=pc->pos.x-14;
						scan_y=pc->pos.y+2;
			
						for (scan_x=start_x;scan_x<=start_x+15;scan_x++) // find all items that have to be resend
						{
							if (pi->pos.x==scan_x)
							{
								if (pi->pos.y==scan_y)
								{                                            
									senditem(s,pi);
								}
							}	
							scan_y--;
						}
			
						start_x=pc->pos.x+2;
						scan_y=pc->pos.y-14;
			
						for (scan_x=start_x;scan_x<=start_x+15;scan_x++) // find all items that have to be resend
						{
							if (pi->pos.x==scan_x)
							{
								if (pi->pos.y==scan_y)
								{
									senditem(s,pi);
								}
							}
							scan_y++;
						}
						break;
			
					case 2: // E
						start_x=pc->pos.x+2;
						scan_y=pc->pos.y-14;
			
						for (scan_x=start_x;scan_x<=start_x+15;scan_x++) // find all items that have to be resend
						{
							if (pi->pos.x==scan_x)
							{
								if (pi->pos.y==scan_y)
								{										  											  
									senditem(s,pi);
								}
							}
							scan_y++;
						}
			
						start_x=pc->pos.x-2;
						scan_y=pc->pos.y+14;
			
						for (scan_x=start_x;scan_x<=start_x+15;scan_x++) // find all items that have to be resend
						{
							if (pi->pos.x==scan_x)
							{
								if (pi->pos.y==scan_y)
								{		
									senditem(s,pi);
								}
							}
							scan_y--;
						}									   
						break;
			
					case 6:
						start_x=pc->pos.x-14;
						scan_y=pc->pos.y+2;
			
						for (scan_x=start_x;scan_x<=start_x+15;scan_x++) // find all items that have to be resend
						{
							if (pi->pos.x==scan_x)
							{
								if (pi->pos.y==scan_y)
								{											  										  
									senditem(s,pi);
								}
							}
							scan_y++;
						}
			
						start_x=pc->pos.x-14;
						scan_y=pc->pos.y+2;
					
						for (scan_x=start_x;scan_x<=start_x+15;scan_x++) // find all items that have to be resend
						{
							if (pi->pos.x==scan_x)
							{
								if (pi->pos.y==scan_y)
								{		   
									senditem(s,pi);
								}
							}
							scan_y--;
						}
			
						break;
			
					default: // if dir is screwed for some resason send away the items for all directins
						int di = dist(pi->pos, pc->pos);
						if (di==11 || di==12)								
						{								       
							senditem(s, pi);
						}
					} // end of switch
					
					// end of LB's new item-resend							 
				}
			}
		}
	}
	return true;
}


void WalkingHandleRainSnow(UOXSOCKET s, CHARACTER c)
{

  int i;
  P_CHAR pc = MAKE_CHARREF_LR(c);

  /********* LB's no rain & snow in buildings stuff ***********/
  if (pc->isPlayer() && online(c)) // check for being in buildings (for weather) only for PC's
  if(wtype!=0) // check only neccasairy if it rains or snows ...
  {
	int y=calcSocketFromChar(c);
    int j=indungeon(c); // dung-check
    i=Map->StaticTop(pc->pos.x,pc->pos.y,pc->pos.z); // static check

	// dynamics-check
	int x=Map->DynamicElevation(pc->pos.x,pc->pos.y,pc->pos.z);
	if (x!=-127) if (Boats->GetBoat(y)!=-1) x=-127; // check for dynamic buildings except boats
	if (x==1 || x==0) x=-127; // 1 seems to be the multi-borders
	// bugfix LB
	
	int kk=noweather[s];    
	if (j || i || x!=-127 ) noweather[s]=1; else noweather[s]=0; // no rain & snow in static buildings+dungeons;
	if (kk-noweather[s]!=0) /*weather(y,0);*/ j=j; // iff outside-inside changes resend weather ...
	// needs to be de-rem'd if weather is available again
  }
  // end
}


void WalkingHandleGlowingItems(P_CHAR pc)
{
	int i;
	if (pc == NULL)
		return;
	if (online(DEREF_P_CHAR(pc)))
	{
		int serial,serhash,ci;
		serial=pc->serial;
		serhash=serial%HASHMAX;
		for (ci=0;ci<glowsp[serhash].max;ci++)
		{
			i=glowsp[serhash].pointer[ci];
			if (i!=-1)
			{
				if (items[i].free==0)
				{
					pc->glowHalo(&items[i]);
				}
			}
		}
	}
}


void walking(int c, int dir, int sequence)
{	
	int newx, newy;

	P_CHAR pc = MAKE_CHARREF_LR(c);
	
	UOXSOCKET s=calcSocketFromChar(pc);

	if (!WalkHandleAllowance(s,DEREF_P_CHAR(pc),sequence))		// check sequence, frozen, weight etc.
		return;

	WalkHandleRunning(s,c,dir);

	int oldx=pc->pos.x;
	int oldy=pc->pos.y;

	if ((dir&0x0F)==pc->dir)
	{
		if (!WalkHandleBlocking(s,DEREF_P_CHAR(pc),sequence,dir, oldx, oldy))
			return;
	}

	if (s!=-1)
	{
		char walkok[4]="\x22\x00\x01";
		walkok[1]=buffer[s][2];
		walkok[2]=0x41;
		if (pc->isHidden()) walkok[2]=0x00;
		Xsend(s, walkok, 3);
		walksequence[s]=sequence;
		if (walksequence[s]==255) walksequence[s]=0;
	}
  
	newx=pc->pos.x;
	newy=pc->pos.y;

	WalkSendToPlayers(s,DEREF_P_CHAR(pc),dir, oldx, oldy, newx, newy);

	if (dir>-1 && (dir&0x0F)<8) { pc->dir=(dir&0x0F);}
	else { clConsole.send("dir-screwed : %i\n",dir); }

	if (s!=-1)
	{
		WalkHandleCharsAtNewPos(s, DEREF_P_CHAR(pc), oldx, oldy, newx, newy);
		WalkHandleItemsAtNewPos(s, DEREF_P_CHAR(pc), oldx, oldy, newx, newy);
	}
  
	if ((pc->pos.x!=oldx)||(pc->pos.y!=oldy))
	{
        if (pc->isPlayer() || pc->questType || pc->tamed)
            objTeleporters(c); 
            teleporters(c);
	}

	// WalkingHandleRainSnow(s,c); // while rain and snow are disabled its a waste of CPU cycles

	WalkingHandleGlowingItems(pc);
    
    Magic->GateCollision(DEREF_P_CHAR(pc));

    checkregion(DEREF_P_CHAR(pc)); 
	
	if(Skills->GetCombatSkill(DEREF_P_CHAR(pc)) ==ARCHERY)  // -Frazurbluu- add in changes for archery skill, and dexterity
    {                                        //  possibly weapon speed?? maybe not, cause crossbows notta running shooting   
		if (pc->targ!= -1)
        {
            if(pc->timeout>= uiCurrentTime)
               pc->timeout= uiCurrentTime + (3*CLOCKS_PER_SEC);

		}
    } 
}



void walking2(CHARACTER s) // Only for switching to combat mode
{
	int i, sendit;

	P_CHAR pc_s = MAKE_CHARREF_LR(s);

	for (i = 0; i < now; i++)
	{
		if (perm[i])
		{
			if (inrange1p(s, currchar[i]))  // perm[i] already tested.
			{
				if ((pc_s->isHidden() || (pc_s->dead && !pc_s->war)) && s != currchar[i] && !chars[currchar[i]].isGM())
				{
					if (!chars[currchar[i]].dead)
					{
						LongToCharPtr(pc_s->serial, &removeitem[1]);
						Xsend(i, removeitem, 5);
						sendit = 0;
					}
					else
						sendit = 1;
				}
				else 
					sendit = 1; // LB 9-12-99 client 1.26.2 fix
				
				if (sendit)
				{
					P_CHAR pc_check = MAKE_CHAR_REF(currchar[i]);
					extmove[1] = pc_s->ser1;
					extmove[2] = pc_s->ser2;
					extmove[3] = pc_s->ser3;
					extmove[4] = pc_s->ser4;
					extmove[5] = pc_s->id1;
					extmove[6] = pc_s->id2;
					extmove[7] = pc_s->pos.x >> 8;
					extmove[8] = pc_s->pos.x%256;
					extmove[9] = pc_s->pos.y >> 8;
					extmove[10] = pc_s->pos.y%256;
					extmove[11] = pc_s->dispz;
					extmove[12] = pc_s->dir&0x7F;
					
					// running stuff
					int dir = pc_s->dir;
					if (pc_s->isNpc() && pc_s->war) // Skyfire
					{
						extmove[12] = dir | 0x80;
						Xsend(i, extmove, 17);
					}
					if (pc_s->isNpc() && pc_s->ftarg!=-1)
					{
						extmove[12] = dir | 0x80;
						Xsend(i, extmove, 17);
					}
					
					ShortToCharPtr(pc_s->skin, &extmove[13]);
					
					if (pc_s->war)
						extmove[15] = 0x40;
					else 
						extmove[15] = 0x00;
					if (pc_s->isHidden())
						extmove[15] |= 0x80;
					if (pc_s->poisoned)
						extmove[15] |= 0x04; // AntiChrist -- thnx to SpaceDog
					if (pc_s->kills >= 4)
						extmove[16] = 6; // ripper
					int guild, race;
					guild = Guilds->Compare(s, currchar[i]);
					race = RaceManager->CheckRelation(pc_s,pc_check);
					if (guild == 1 || race == 1)// Same guild (Green)
						extmove[16] = 2;
					else if (guild == 2 || race == 2) // Enemy guild.. set to orange
						extmove[16] = 5;
					else 
					{
						if (pc_s->isMurderer())      // show red
							extmove[16] = 6;
						else if (pc_s->isInnocent()) // show blue
							extmove[16] = 1;
						else if (pc_s->flag == 0x08) // show green
							extmove[16] = 2;
						else if (pc_s->flag == 0x10) // show orange
							extmove[16] = 5;
						else 
							extmove[16] = 3;            // show grey
					}
					// end of if sendit
					
					if (!pc_s->war) // we have to execute this no matter if invisble or not LB
					{
						pc_s->attacker = INVALID_SERIAL;
						pc_s->targ=-1;
					}
					
					if (sendit)
						Xsend(i, extmove, 17);
				}
			}
		}
	}
}

int npcSelectDir(P_CHAR pc_i, int j)
{
	if (pc_i->blocked)
	{
		short x=0;
		if (j/2.0!=j/2)
			x=1;
		if (pc_i->blocked<=2)
			j =pc_i->dir2;	/* =(j-2-x)%8; //works better  ????*/
		else
		{
			if (rand()%2) j=pc_i->dir2 = static_cast<char>((j-2-x)%8);
			else j=pc_i->dir2 = static_cast<char>((j+2+x)%8);
		}
	}
	if (j<0)
		j=rand()%8;
	return j;
}

int npcSelectDirWarOld(P_CHAR pc_i, int j)
{
	if (pc_i->blocked)
	{
		short x=0;
		if (j/2.0!=j/2)
			x=1;
		if (pc_i->blocked<=2)
			j =pc_i->dir2 =(j-2-x)%8; //works better  ????
		else
		{
			if (rand()%2) j=pc_i->dir2=(j-2-x)%8;
			else j=pc_i->dir2=(j+2+x)%8;
		}
	}
	if (j<0)
		j=rand()%8;
	return j;
}

class cWalkingAI
{
private:
	short results;
	int startX, startY;
	int targetX, targetY;
	int zz;
	int c;
public:
	void Init(P_CHAR pNpc, int desiredDir, int tx, int ty)
	{
		results = 0;
		startX = pNpc->pos.x;
		startY = pNpc->pos.y;
		targetX = tx;
		targetY = ty;
		zz = pNpc->pos.z;
		c = DEREF_P_CHAR(pNpc);	// just needed for call to validNPCMove()
	}

	short dirBitChecked(int dir) { int i=(1<<dir);
	return i; }
	//return (1<<dir); }
	short dirBitBlocked(int dir) { return (1<<(dir+8)); }

	bool TileBlocks(int dir)		// check if tile blocks
	{
		if (results & dirBitChecked(dir))	// avoid double checking
			return (results & dirBitBlocked(dir)) ? true : false;
		int chkX = startX;
		int chkY = startY;
		getXYfromDir(dir,&chkX,&chkY);		// get coords of the location we want to walk
		results |= dirBitChecked(dir);
		if (!validNPCMove(chkX, chkY, zz, c))
			results |= dirBitBlocked(dir);
		return (results & dirBitBlocked(dir)) ? true : false;
	}

	bool DirBlocks(int dir)		// check if we can reach tile, return true if blocked
	{
		if (TileBlocks(dir))
			return true;		// the tile itself is blocking
		if (!(dir & 1))			// square ie. N,S,W or E
			return false;		// perfect! we can just go where we want to go
		else					// diagonal ie. NE,SE,SW or SE ??
		{						// We'll have to check the adjacent tiles too
			if (!TileBlocks(getLeftDir(dir)))
				return false;
			if (!TileBlocks(getRightDir(dir)))
				return false;
		}
		return true;
	}
	
	double dist(int xa, int ya, int xb, int yb) // Distance between position a and b
	{
		return hypot( abs(xa-xb), abs(ya-yb) ); // hypot is POSIX standard
	}
	
	double distToTarget(int dir)
	{
		int chkX1 = startX;
		int chkY1 = startY;
		getXYfromDir(dir,&chkX1,&chkY1);		// get coords of the location we want to check
		return dist(startX,startY,chkX1,chkY1);
	}

	int ClosestDir(int dir1, int dir2)
	{
		if ( distToTarget(dir1) < distToTarget(dir2) )
			return dir1;
		else
			return dir2;
	}

	int PreferredDir(int dir1, int dir2)
	{
		int dir1stTry = ClosestDir(dir1,dir2);
		if (!DirBlocks(dir1stTry))
			return dir1stTry;
		int dir2ndTry = (dir1stTry==dir1 ? dir2 : dir1);
		if (!DirBlocks(dir2ndTry))
			return dir2ndTry;
		return -1;
	}

	int SelectDirWar(int desiredDir)
	{
		int dir = -1;
		// step 1: let's see if we can just go where we want to go
		if (!DirBlocks(desiredDir))
			return desiredDir;				// perfect!

		
		// step 2: the direct way is blocked, find another way...
		int dirLeft1  = getLeftDir (desiredDir);
		int dirRight1 = getRightDir(desiredDir);

		dir = PreferredDir(dirLeft1,dirRight1);
		if (dir != -1)
			return dir;

		
		// step 3: let's try to go sideways...
		int dirLeft2  = getLeftDir (dirLeft1);
		int dirRight2 = getRightDir(dirRight1);

		dir = PreferredDir(dirLeft2,dirRight2);
		if (dir != -1)
			return dir;

		
		// step 4: let's try to go backwards...
		int dirLeft3  = getLeftDir (dirLeft2);
		int dirRight3 = getRightDir(dirRight2);

		dir = PreferredDir(dirLeft3,dirRight3);
		if (dir != -1)
			return dir;


		// step 5: only one place left to go...
		int opposite  = getLeftDir (dirLeft3);
		if (!DirBlocks(opposite))
			return opposite;

		return -1;				// we are totally blocked !
	}
	
};

//NEW NPCMOVEMENT ZIPPY CODE STARTS HERE -- AntiChrist meging codes --
void npcMovement2(unsigned int currenttime, int i)//Lag fix
{
	register int k;
	
	int j, dnpctime, l;
	P_CHAR pc_i = MAKE_CHARREF_LR(i);
	
	dnpctime=0;
    if (pc_i->isNpc() && (pc_i->npcmovetime<=currenttime||(overflow)))
    {
	
		if (pc_i->war && pc_i->npcWander != 5) 
		{ // a very simple pathfinding algorithm
			P_CHAR pAtttacker = FindCharBySerial(pc_i->attacker);					
			if (pAttacker != NULL)
			{
				if ((chardist(DEREF_P_CHAR(pc_i), l)>1 || chardir(DEREF_P_CHAR(pc_i), l)!=pc_i->dir))
				{
					if (online(DEREF_P_CHAR(pAttacker)||pAttacker->isNpc()) // LB bugkilling, was online(calcsocket(l)
					{
						j=chardir(DEREF_P_CHAR(pc_i), pc_i->attacker);
#if 0
						// this code is disabled because there is a bug (?) in validNPCMove() calling DoesStaticBlock()
						// eg. 'brambles' are considered 'non-blocking' but the client doesn't let you go there !
						// thus the new code called here is running into the same probs as the old code :(
						// I have no clue yet :(  (Duke 18.7.01)
						cWalkingAI wai;
						wai.Init(pc_i, j, pAttacker->x, pAttacker->y);
						j=wai.SelectDirWar(j);
						if (j==-1) return;	// totally blocked. Don't walk.
#else
						j=npcSelectDirWarOld(pc_i,j);
#endif
						
						//					if (pc_i->npcWander==2) npcwalk(DEREF_P_CHAR(pc_i),j%8,2); //bugfix LB
						//					else if (pc_i->npcWander==3) npcwalk(DEREF_P_CHAR(pc_i),j%8,1); 
						//					else npcwalk(DEREF_P_CHAR(pc_i),j%8,0);
						// checking bounding box/circle in combat mode doesn't seem adequate (Duke)
						npcwalk(DEREF_P_CHAR(pc_i),j%8,0);
					}
				}
			} // end of l!=-1
		} 
		
		else
		{
			switch(pc_i->npcWander)
			{
			case 0: // No movement
				break;
			case 1: // Follow the follow target
				k=(pc_i->ftarg);
				if (k < 0 || k>cmem) return;

				//if (!((online(k)||chars[k].npc))) break;			
				
				if (chars[k].dead || chars[k].free || chars[k].disabled>uiCurrentTime) break; //antichrist
				/*if (chardist(i,k)>6) // so pets will teleport to owner
				{
                   pc_i->pos.x=chars[k].x;
                   pc_i->pos.y=chars[k].y;
                   pc_i->z=chars[k].z;
                   pc_i->dir=chars[k].dir;
                   teleport(i);
				}*/

				// Dupois - Added April 4, 1999
				// Has the Escortee reached the destination ??
				if ( (pc_i->ftarg!=-1) && (!chars[k].dead) && (pc_i->questDestRegion==pc_i->region) )
				{
					// Pay the Escortee and free the NPC
					MsgBoardQuestEscortArrive( DEREF_P_CHAR(pc_i), calcSocketFromChar( k ) );
				}
				// End - Dupois
				
				if (chardist(DEREF_P_CHAR(pc_i), k)>1 || chardir(DEREF_P_CHAR(pc_i), k)!=pc_i->dir)
				{
					j=chardir(i, k);
					j=npcSelectDir(pc_i,j);
					dnpctime=1;
					npcwalk(i,j%8,0);
				}
				break;
			case 2: // Wander freely, in a defined circle
				j=rand()%40;
				if (j<8 || j>32) dnpctime=1;
				if (j>7 && j<33) // Let's move in the same direction lots of the time.  Looks nicer.
					j=pc_i->dir;
				npcwalk(DEREF_P_CHAR(pc_i),j%8,2); // lb bugfix, was 0
				break;
			case 3: // Wander freely, within a defined box
				j=rand()%40;
				if (j<8 || j>32) dnpctime=1;
				if (j>7 && j<33) // Let's move in the same direction lots of the time.  Looks nicer.
					j=pc_i->dir;
				npcwalk(DEREF_P_CHAR(pc_i),j%8,1);
				break;
			case 4:  // wander freely, avoiding obstacles
				
				j=rand()%40;
				if (j<8 || j>32) dnpctime=1;
				if (j>7 && j<33) // Let's move in the same direction lots of the time.  Looks nicer.
					j=pc_i->dir;
				npcwalk(DEREF_P_CHAR(pc_i),j%8,0); 
				
				break;
			case 5: //FLEE!!!!!!
				{
					k=pc_i->targ;
					//if (!(online(k)||chars[k].npc)) break;
					if (k<0 || k>cmem) return;
					if (chardist(i, k)<15)
					{
						j=(chardir(DEREF_P_CHAR(pc_i), k)+4)%8;
						j=npcSelectDir(pc_i,j);
						npcwalk(DEREF_P_CHAR(pc_i),j%8,0);
					}
					break;
			default:
				clConsole.send("ERROR: Fallout of switch statement without default [%i]. walking.cpp, npcMovement2()\n",pc_i->npcWander); //Morrolan
				} // break; //Morrolan unnecessary ?
			}
		}

		//AntiChrist
		if(pc_i->npcWander==1)
			pc_i->npcmovetime=(unsigned int)(currenttime+double(NPCSPEED*MY_CLOCKS_PER_SEC*(dnpctime)/4)); //reset move timer
		else
			pc_i->npcmovetime=(unsigned int)(currenttime+double(NPCSPEED*MY_CLOCKS_PER_SEC*(dnpctime))); //reset move timer
	}
}

int checkBounds(P_CHAR pc, int newX, int newY, int type)
{
	int move=0;
	switch (type)
	{
	case 0: move=1;break;
	case 1: move=checkBoundingBox(newX, newY, pc->fx1, pc->fy1, pc->fz1, pc->fx2, pc->fy2);break;
	case 2: move=checkBoundingCircle(newX, newY, pc->fx1, pc->fy1, pc->fz1, pc->fx2);break;
	default: move=0;	// invalid type given
	}
	return move;
}

// Restructured & bugfixed by LB 17 Septemer 1999
// Thx to JM for help.

void npcwalk(CHARACTER i, int j, int type)   //type is npcwalk mode (0 for normal, 1 for box, 2 for circle)
{
	int x,y,s=i,b,cr,k;
	signed char z;
	char v1,v2,v3,valid,move;

	P_CHAR pc_i = MAKE_CHARREF_LR(i);

	x = pc_i->pos.x;
	y = pc_i->pos.y;
	z = pc_i->pos.z;
	//Bug Fix -- Zippy
	if(pc_i->priv2&2) return;//Frozen - Don't send them al the way to walking to check this, just do it here.

	//clConsole.send("  npcw-2 c-dir: %i j-dor: %i\n",pc_i->dir,j);


	//clConsole.send("callwe-w: %i t: %i dir: %i\n",caller,type,j);

	/////////// LB's flying creatures stuff, flying animation if they stand still ///////
  
	b=(pc_i->id1<<8)+pc_i->id2;
    cr=(creatures[b].who_am_i)&0x1; // can it fly ?  
	if (cr==1)
	{
		if (pc_i->fly_steps>0)
		{
			if (rand()%3==0) npcaction(s,0x13); // flying animation
		} 
	}
	
	/////////////////////////////////////////////////////////////////////////////////////

	//clConsole.send("name: %s j-dir:%i, c-dir:%i type:%i\n",pc_i->name,j,pc_i->dir,type);

	k=pc_i->dir;
	if (k==j)  // If we're moving, not changing direction
	{
		int newX = pc_i->pos.x;
		int newY = pc_i->pos.y;
		getXYfromDir(pc_i->dir,&newX,&newY);	// get coords of the location we want to walk

		switch(j) // Switch, based on the direction we're going to move.
		{
		case 0: // North
		case 2:
		case 4:
		case 6:
			valid=validNPCMove(newX, newY, z, i);
			if (valid)
			{
				move=checkBounds(pc_i, newX, newY, type);
				if (move)
					walking(i,j,256);
			}
			break;
			
		case 1: // Northeast
		case 3:
		case 5:
		case 7:
			v1 = validNPCMove(newX, newY, z, i); // check northeast
			if (!v1) valid=0;	// if it blocks -> no move 
			else				// if not, check if both, N & E are blocking. 
								// if so it blocks too, otherwise its ok.
			{
				int leftX = pc_i->pos.x;
				int leftY = pc_i->pos.y;
				getXYfromDir(getLeftDir(pc_i->dir),&leftX,&leftY);	// get coords of the left location we want to pass
				
				v2 =  validNPCMove(leftX, leftY, z, i);
				if (v2) valid=1; 
				else
				{ 
					int rightX = pc_i->pos.x;
					int rightY = pc_i->pos.y;
					getXYfromDir(getRightDir(pc_i->dir),&rightX,&rightY);	// get coords of the right location we want to pass

					v3= validNPCMove(rightX, rightY, z, i);
					if (v3) valid=1; else valid=0;
				}
			}
			
			  // LB remark: NW-block AND (N-block OR W-block) is the same more compact, BUT ...
			  // could be slower, depending on the compilers optimization.
              // I dont trust msvc, thus the "hand-optimization

			//clConsole.send("val: %i v1%i v2%i v3%i\n",valid,v1,v2,v3);

			if (valid)
			{
				move=checkBounds(pc_i, newX, newY, type);
				if (move)
				{ /*clConsole.send("calling w\n");*/walking(i,j,256);}
			}
			break;
		}
	}
	else 
	{        
		//clConsole.send("changing dir?\n");
		walking(i, j, 256);
	} 
}

int validNPCMove(int x, int y, signed char z, int s)
{
	const int getcell=mapRegions->GetCell(x,y);

	P_CHAR pc_s = MAKE_CHARREF_LRV(s, 0);

    pc_s->blocked++;
	vector<SERIAL> vecEntries = mapRegions->GetCellEntries(getcell);
    for ( unsigned int k = 0; k < vecEntries.size(); k++)
    {
		P_ITEM mapitem = FindItemBySerial(vecEntries[k]);
        if (mapitem != NULL)
        {
		    tile_st tile;
            Map->SeekTile(mapitem->id(), &tile);
            if (mapitem->pos.x==x && mapitem->pos.y==y && mapitem->pos.z+tile.height>z+1 && mapitem->pos.z<z+MaxZstep)
            {
                // bugfix found by JustMichael, moved by crackerjack
                // 8/2/99 makes code run faster too - one less loop :)
                if (mapitem->id()==0x3946 || mapitem->id()==0x3956) return 0;
                if (mapitem->id1<=2 || (mapitem->id()>=0x0300 && mapitem->id()<=0x03E2)) return 0;
                if (mapitem->id()>0x0854 && mapitem->id()<0x0866) return 0;
                
                if (mapitem->type==12)
                {
                    if (pc_s->isNpc() && (strlen(pc_s->title) > 0 || pc_s->npcaitype != 0))
                    {                            
                        // clConsole.send("doors!!!\n");
                        dooruse(-1, DEREF_P_ITEM(mapitem));
                        
                    }                                   
                    pc_s->blocked=0;
                    return 0;
                }
                
            }
        }
    }

	// experimental check for bad spawning/walking places, not optimized in any way (Duke, 3.9.01)
	int mapid = 0;
	signed char mapz = Map->AverageMapElevation(x, y, mapid);	// just to get the map-ID
	if (mapz != illegal_z)
	{
		if ((mapid >= 0x25A && mapid <= 0x261) ||	// cave wall
			(mapid >= 0x266 && mapid <= 0x26D) ||	// cave wall
			(mapid >= 0x2BC && mapid <= 0x2CB) )	// cave wall
			return 0;
		if ( mapid >= 0x0A8 && mapid <= 0x0AB) 	// water (ocean ?)
			return 0;
//		land_st land;
//		Map->SeekLand(mapid, &land);
	}
		
    // see if the map says its ok to move here
    if (Map->CanMonsterMoveHere(x, y, z))
    {
		pc_s->blocked = 0;
		return 1;
    }
    return 0;
}
