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
Module : necro.cpp
Purpose: store all necromancy related functions
Created: Genesis 11-12-1998
History: None
*/

#include "wolfpack.h"
#include "SndPkg.h"
#include "debug.h"
#include "network.h"

#undef  DBGFILE
#define DBGFILE "necro.cpp"

void vialtarget(int nSocket) // bug & crashfixed by LB 25 september 1999
{
	P_ITEM Vial = FindItemBySerial(addmitem[nSocket]);
	if (!Vial) return; // should never happen
	
//	int nTargetID=-1;
	
	
//	int cc = currchar[nSocket];
	cChar* Player = currchar[nSocket];

	cItem* Weapon = Player->getWeapon(); // search for a dagger in the players hand
	if (!Weapon || !IsDagger(Weapon->id()) )
	{
		sysmessage(nSocket,"You do not have a dagger equipped.");
		return;
	}
	
	int serial=LongFromCharPtr(buffer[nSocket]+7);
	if (isCharSerial(serial))
	{
		cChar* Victim = FindCharBySerial(serial);
		if (!Victim)
			return;
		Vial->more1=0;
		if(!Victim->npc)
		{
			// checkskill hmmm what skill/s has/have to added here LB ...
			
			if( Victim->isSameAs(Player) )
			{
				if(Victim->hp<=10)
				{
					sysmessage(nSocket,"You are too wounded to continue.");
					return;
				}
				sysmessage(nSocket,"You prick your finger and fill the vial.");
			}
			else
			{
				if (Player->dist(Victim) > 2)
				{
					sysmessage(nSocket,"That individual is not anywhere near you.");
					return;
				}
				if (Victim->npc)
				{
					if( Victim->id() == 0x000c || (Victim->id()>=0x003b && Victim->id()<=0x003d) )
						Vial->more1=1;
					// Guard be summuned if in town and good npc
					// if good flag criminal
					// if evil npc attack necromancer but don't flag criminal
				}
				else
				{
					sprintf(temp,"%s has pricked you with a dagger and sampled your blood.",Player->name.c_str());
					sysmessage(calcSocketFromChar(Victim),temp);
					// flag criminal						
				}
				Karma(Player,Victim,(0-(Victim->karma)));
			}
			Victim->hp -= (rand()%6)+2;
			MakeNecroReg(nSocket,Vial,0x0E24);
		}
	}
	else
	{
		cItem* Corpse = FindItemBySerial(serial);
		if (!Corpse)
			return;
		if( !Corpse->corpse() )
			sysmessage(nSocket,"That is not a person or a corpse!");
		else
		{
			Vial->more1=Corpse->more1;
			Karma(Player, NULL,-1000);
			if (Corpse->more2<4)
			{
				sysmessage(nSocket,"You take a sample of blood from the corpse.");
				MakeNecroReg(nSocket,Vial,0x0E24);
				Corpse->more2++;
			}
			else
				sysmessage(nSocket,"You examine the corpse but, decide any further blood samples would be too contaminated.");
		}
	}
}

void MakeNecroReg(int nSocket, P_ITEM pMat, short id)
{
	P_ITEM pItem = NULL;
	P_CHAR pc_currchar = currchar[nSocket];

	if( id>=0x1B11 && id<=0x1B1C ) // Make bone powder.
	{
		sprintf((char*)temp,"%s is grinding some bone into powder.", pc_currchar->name.c_str());
		npcemoteall(pc_currchar, (char*)temp,1);
		tempeffect(pc_currchar, pc_currchar, 9, 0, 0, 0);
		tempeffect(pc_currchar, pc_currchar, 9, 0, 3, 0);
		tempeffect(pc_currchar, pc_currchar, 9, 0, 6, 0);
		tempeffect(pc_currchar, pc_currchar, 9, 0, 9, 0);
		pItem = Items->SpawnItem(nSocket, pc_currchar, 1, "bone powder", 1, 0x0F, 0x8F, 0, 1, 1);
		if(pItem == NULL) return;//AntiChrist to preview crashes
		pItem->morex = 666;
		pItem->more1=1; // this will fill more with info to tell difference between ash and bone
		Items->DeleItem(pMat);
		
	}
	if( id==0x0E24 ) // Make vial of blood.
	{
		if(pMat->more1==1)
		{
			pItem = Items->SpawnItem(nSocket, pc_currchar,1,"#",1,0x0F,0x82,0,1,1);
			if(pItem==NULL) return;//AntiChrist to preview crashes
			pItem->value=15;
			pItem->morex=666;
		}
		else
		{
			pItem = Items->SpawnItem(nSocket, pc_currchar,1,"#",1,0x0F,0x7D,0,1,1);
			if(pItem==NULL) return;//AntiChrist to preview crashes
			pItem->value=10;
			pItem->morex=666;
		}
		pMat->ReduceAmount(1);
	}
}
