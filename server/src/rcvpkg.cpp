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
	UOXSOCKET s=ps->GetSocket();
	CHARACTER cc = ps->GetCurrChar();
	P_CHAR pc_currchar = MAKE_CHARREF_LR(cc);
	int j;

	int serial=calcserial(buffer[s][1],buffer[s][2],buffer[s][3],buffer[s][4]);
	if(serial == INVALID_SERIAL) return;
//	int i = calcCharFromSer( serial );
	P_CHAR pc_i = FindCharBySerPtr(&buffer[s][1]);
	if(pc_i == NULL) return;	//to avoid problems
	
	if(pc_currchar->dead)//AntiChrist stuff
	{
		if(SrvParms->persecute)
		{//start persecute stuff - AntiChrist
			pc_currchar->targ = DEREF_P_CHAR(pc_i);
			if(pc_currchar->targ==-1) return;
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
		pc_currchar->targ=calcCharFromSer(serial);
		pc_currchar->unhide();
		pc_currchar->disturbMed(s);

		
		if(pc_i->dead || pc_i->hp<=0)//AntiChrist
		{
			sysmessage(s,"That person is already dead!");
			return;
		}
		
		if (pc_i->npcaitype==17)//PlayerVendors
		{
			sprintf((char*)temp, "%s cannot be harmed.",pc_i->name);
			sysmessage(s, (char*)temp);
			return;
		}
		
		SndAttackOK(s, pc_i->serial);	//keep the target highlighted       
		if (!(pc_i->targ==-1))
		{
			pc_i->attacker = pc_currchar->serial;
			pc_i->resetAttackFirst();
		}
		pc_currchar->setAttackFirst();
		pc_currchar->attacker = pc_i->serial;
 
		pc_currchar->dir = chardir(DEREF_P_CHAR(pc_currchar),DEREF_P_CHAR(pc_i));	// turn to attacker, LB (& Duke)

		updatechar(DEREF_P_CHAR(pc_currchar));

		if( pc_i->guarded )
		{
			AllCharsIterator iter_char;
			for (iter_char.Begin(); iter_char.GetData() != NULL; iter_char++)
			{
				P_CHAR toCheck = iter_char.GetData();
				if (pc_i->Owns(toCheck) && toCheck->npcaitype == 32 && chardist( DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(toCheck) )<= 10 )
				{
					npcattacktarget( DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(toCheck) );
				}
			}
		}

		if (pc_i->inGuardedArea() && SrvParms->guardsactive)
		{
			if (pc_i->isPlayer() && pc_i->isInnocent() && Guilds->Compare( DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_i) )==0) //REPSYS
			{
				criminal( DEREF_P_CHAR(pc_currchar) );
				Combat->SpawnGuard(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_i) ,pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);
			}
			else if( pc_i->isNpc() && pc_i->isInnocent() && !pc_i->isHuman() && pc_i->npcaitype!=4 )
			{
				criminal( DEREF_P_CHAR(pc_currchar) );
				Combat->SpawnGuard(DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_i), pc_currchar->pos.x,pc_currchar->pos.y,pc_currchar->pos.z);
			}
			else if( pc_i->isNpc() && pc_i->isInnocent() && pc_i->isHuman() && pc_i->npcaitype!=4 )
			{
				npctalkall(DEREF_P_CHAR(pc_i), "Help! Guards! I've been attacked!", 1);
				criminal( DEREF_P_CHAR(pc_currchar) );
				callguards(DEREF_P_CHAR(pc_i));
			}
			else if( pc_i->isNpc() && pc_i->npcaitype==4)
			{
				criminal( DEREF_P_CHAR(pc_currchar) );
				npcattacktarget(DEREF_P_CHAR(pc_i), DEREF_P_CHAR(pc_currchar));
			}
			else if ((pc_i->isNpc() || pc_i->tamed) && !pc_i->war && pc_i->npcaitype!=4) // changed from 0x40 to 4, cauz 0x40 was removed LB
			{
				npcToggleCombat(DEREF_P_CHAR(pc_i));
				pc_i->npcmovetime=(unsigned int)(uiCurrentTime+(double)(NPCSPEED*MY_CLOCKS_PER_SEC)); //*16));
			}
			else
			{
				pc_i->npcmovetime=(unsigned int)(uiCurrentTime+(double)(NPCSPEED*MY_CLOCKS_PER_SEC)); //*16));
			}
			
			sprintf((char*)temp, "You see %s attacking %s!", pc_currchar->name, pc_i->name);
			
			for (j=0;j<now;j++)
			{
				if((inrange1(s, j) && perm[j]) && (s!=j))
				{
					pc_i->emotecolor1=0x00;
					pc_i->emotecolor2=0x26;
					npcemote(j, DEREF_P_CHAR(pc_currchar), (char*)temp,1);
				}
			}
		}
		else	// not a guarded area
		{
			if (pc_i->isInnocent())
			{
				if (pc_i->isPlayer() && Guilds->Compare( DEREF_P_CHAR(pc_currchar), DEREF_P_CHAR(pc_i) )==0)
				{
					criminal(DEREF_P_CHAR(pc_currchar));
				}
				else if (pc_i->isNpc() && pc_i->tamed)
				{
					criminal(DEREF_P_CHAR(pc_currchar));
					npcattacktarget(DEREF_P_CHAR(pc_i),DEREF_P_CHAR(pc_currchar));
				}
				else if (pc_i->isNpc())
				{
					criminal(DEREF_P_CHAR(pc_currchar));
					npcattacktarget(DEREF_P_CHAR(pc_i),DEREF_P_CHAR(pc_currchar));
					if (pc_i->isHuman() )
					{
						npctalkall(DEREF_P_CHAR(pc_i), "Help! Guards! Tis a murder being commited!", 1);
					}
				}
			}
		}
	}
}
