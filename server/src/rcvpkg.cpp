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
//#include "itemid.h"
#include "SndPkg.h"

#undef DBGFILE
#define DBGFILE "rcvpkg.cpp"
#include "debug.h"

void RcvAttack(P_CLIENT ps)
{
	UOXSOCKET s = ps->GetSocket();
	P_CHAR pc_currchar = ps->getPlayer();
	int j;

	SERIAL serial=calcserial(buffer[s][1],buffer[s][2],buffer[s][3],buffer[s][4]);
	if(serial == INVALID_SERIAL) return;
	P_CHAR pc_i = FindCharBySerPtr(&buffer[s][1]);
	if(pc_i == NULL) return;	//to avoid problems
	
	if(pc_currchar->dead)//AntiChrist stuff
	{
		if(SrvParms->persecute)
		{//start persecute stuff - AntiChrist
			pc_currchar->targ = pc_i->serial;
			if(pc_currchar->targ==INVALID_SERIAL) return;
			else Skills->Persecute(s);
			return;
		} else
		{
			sysmessage(s,"You are dead and cannot do that.");
			return;
		}
	}
	if(pc_currchar->cell>0)
	{
		sysmessage(s,"There is no fighting in the jail cells!");
		return;
	}

	if(!pc_currchar->dead)
	{
		pc_currchar->targ = serial;
		pc_currchar->unhide();
		pc_currchar->disturbMed(s);

		
		if(pc_i->dead || pc_i->hp<=0)//AntiChrist
		{
			sysmessage(s,"That person is already dead!");
			return;
		}
		
		if (pc_i->npcaitype==17)//PlayerVendors
		{
			sprintf((char*)temp, "%s cannot be harmed.",pc_i->name.c_str());
			sysmessage(s, (char*)temp);
			return;
		}
		
		SndAttackOK(s, pc_i->serial);	//keep the target highlighted       
		if (!(pc_i->targ == INVALID_SERIAL))
		{
			pc_i->attacker = pc_currchar->serial;
			pc_i->resetAttackFirst();
		}
		pc_currchar->setAttackFirst();
		pc_currchar->attacker = pc_i->serial;
 
		pc_currchar->dir = chardir(pc_currchar, pc_i);	// turn to attacker, LB (& Duke)

		updatechar(pc_currchar);

		if( pc_i->guarded )
		{
			AllCharsIterator iter_char;
			for (iter_char.Begin(); !iter_char.atEnd(); iter_char++)
			{
				P_CHAR toCheck = iter_char.GetData();
				if (pc_i->Owns(toCheck) && toCheck->npcaitype == 32 && chardist( pc_currchar, toCheck )<= 10 )
				{
					npcattacktarget( pc_currchar, toCheck );
				}
			}
		}

		if (pc_i->inGuardedArea() && SrvParms->guardsactive)
		{
			if (pc_i->isPlayer() && pc_i->isInnocent() && Guilds->Compare( pc_currchar, pc_i )==0) //REPSYS
			{
				criminal( pc_currchar );
				Combat->SpawnGuard(pc_currchar, pc_i ,pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);
			}
			else if( pc_i->isNpc() && pc_i->isInnocent() && !pc_i->isHuman() && pc_i->npcaitype!=4 )
			{
				criminal( pc_currchar );
				Combat->SpawnGuard(pc_currchar, pc_i, pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);
			}
			else if( pc_i->isNpc() && pc_i->isInnocent() && pc_i->isHuman() && pc_i->npcaitype!=4 )
			{
				npctalkall(pc_i, "Help! Guards! I've been attacked!", 1);
				criminal( pc_currchar );
				callguards(pc_i);
			}
			else if( pc_i->isNpc() && pc_i->npcaitype==4)
			{
				criminal( pc_currchar );
				npcattacktarget(pc_i, pc_currchar);
			}
			else if ((pc_i->isNpc() || pc_i->tamed) && !pc_i->war && pc_i->npcaitype!=4) // changed from 0x40 to 4, cauz 0x40 was removed LB
			{
				npcToggleCombat(pc_i);
				pc_i->npcmovetime=(unsigned int)(uiCurrentTime+(double)((NPCSPEED*MY_CLOCKS_PER_SEC)/5)); //*16));
			}
			else
			{
				pc_i->npcmovetime=(unsigned int)(uiCurrentTime+(double)((NPCSPEED*MY_CLOCKS_PER_SEC)/5)); //*16));
			}
			
			sprintf((char*)temp, "You see %s attacking %s!", pc_currchar->name.c_str(), pc_i->name.c_str());
			
			for (j=0;j<now;j++)
			{
				if((inrange1(s, j) && perm[j]) && (s!=j))
				{
					pc_i->emotecolor1=0x00;
					pc_i->emotecolor2=0x26;
					npcemote(j, pc_currchar, (char*)temp,1);
				}
			}
		}
		else	// not a guarded area
		{
			if (pc_i->isInnocent())
			{
				if (pc_i->isPlayer() && Guilds->Compare( pc_currchar, pc_i )==0)
				{
					criminal( pc_currchar );
				}
				else if (pc_i->isNpc() && pc_i->tamed)
				{
					criminal( pc_currchar );
					npcattacktarget(pc_i, pc_currchar);
				}
				else if (pc_i->isNpc())
				{
					criminal( pc_currchar );
					npcattacktarget(pc_i, pc_currchar);
					if (pc_i->isHuman() )
					{
						npctalkall(pc_i, "Help! Guards! Tis a murder being commited!", 1);
					}
				}
			}
		}
	}
}
