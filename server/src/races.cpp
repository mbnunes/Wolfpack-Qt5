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
#include "races.h"

void cRaces::LoadRaceFile()
{
	int racecount = 0;
	wscfile = fopen("races.scp", "r");
	if (wscfile == NULL) 
	{
		return;
	}
	do
	{
		readw2();
		if(!(strcmp((char*)script1,"SECTION")))
		{
			cRace dummy;
			races.push_back(dummy);
			racecount = races.size() - 1;
		}
		else if( racecount < races.size())
		{
			switch(script1[0])
			{
			case 'B':
			case 'b':
				if(!strcmp((char*)script1, "BEARDREQ"))
					races[racecount].BeardReq = str2num(script2);
				else if(!strcmp((char*)script1, "BASEAR"))
					races[racecount].BaseAR = str2num(script2);
				else if(!strcmp((char*)script1, "BESTATTACKSKILL"))
				{
					if(!strcmp((char*)script2, "ARCHERY"))
						races[racecount].BestFightSkill = ARCHERY;
					else if(!strcmp((char*)script2, "SWORDSMANSHIP"))
						races[racecount].BestFightSkill = SWORDSMANSHIP;
					else if(!strcmp((char*)script2, "MACEFIGHTING"))
						races[racecount].BestFightSkill = MACEFIGHTING;
					else if(!strcmp((char*)script2, "FENCING"))
						races[racecount].BestFightSkill = FENCING;
					else
						races[racecount].BestFightSkill = SWORDSMANSHIP;
				}
				break;
			case 'C':
			case 'c':
				if(!strcmp((char*)script1, "CANUSESKILL"))
					races[racecount].CanUseSkill.push_back(str2num(script2));
				else if(!strcmp((char*)script1, "CANTUSEITEM"))
					races[racecount].CantUseItem.push_back(str2num(script2));
				break;
			case 'D':
			case 'd':
				if(!strcmp((char*)script1, "DEXCAP"))
					races[racecount].DexCap = str2num(script2);
				break;
			case 'G':
			case 'g':
				if(!strcmp((char*)script1, "GENDER"))
					races[racecount].Gender = str2num(script2);
				break;
			case 'H':
			case 'h':
				if(!strcmp((char*)script1, "HAIRREQ"))
					races[racecount].HairReq = str2num(script2);
				else if(!strcmp((char*)script1, "HAIRBEARDLIST"))
					races[racecount].HairBeardList = script2;
				break;
			case 'I':
			case 'i':
				if(!strcmp((char*)script1, "ISPLAYERRACE"))
					races[racecount].IsPlayerRace=str2num(script2);
				else if(!strcmp((char*)script1, "INTCAP"))
					races[racecount].IntCap=str2num(script2);
				else if(!strcmp((char*)script1, "IMUNETOSPELL"))
					races[racecount].ImuneToSpell.push_back(str2num(script2));
				break;
			case 'N':
			case 'n':
				if(!strcmp((char*)script1, "NIGHTSIGHT"))
					races[racecount].NightSight=str2num(script2);
				else if(!strcmp((char*)script1, "NOHAIR"))
					races[racecount].NoHair=str2num(script2);
				else if(!strcmp((char*)script1, "NOBEARD"))
					races[racecount].NoBeard=str2num(script2);
				break;
			case 'R':
			case 'r':
				if(!strcmp((char*)script1, "RACENAME"))
					races[racecount].RaceName = script2;
				else if(!strcmp((char*)script1, "RACEALLY"))
					races[racecount].RacialAlly.push_back(str2num(script2));
				else if(!strcmp((char*)script1, "RACEENEMY"))
					races[racecount].RacialEnemy.push_back(str2num(script2));
				break;
			case 'S':
			case 's':
				if(!strcmp((char*)script1, "STARTINT"))
					races[racecount].StartInt=str2num(script2);
				else if(!strcmp((char*)script1, "STARTDEX"))
					races[racecount].StartDex=str2num(script2);
				else if(!strcmp((char*)script1, "STARTSTR"))
					races[racecount].StartStr=str2num(script2);
				else if(!strcmp((char*)script1, "STRCAP"))
					races[racecount].StrCap=str2num(script2);
				else if(!strcmp((char*)script1, "SKINLIST"))
					races[racecount].SkinList = script2;
				else if(!strcmp((char*)script1, "STARTX"))
					races[racecount].startpos.x=str2num(script2);
				else if(!strcmp((char*)script1, "STARTY"))
					races[racecount].startpos.y=str2num(script2);
				else if(!strcmp((char*)script1, "STARTZ"))
					races[racecount].startpos.z=str2num(script2);
				break;
			case 'V':
			case 'v':
				if(!strcmp((char*)script1, "VISRANGE"))
					races[racecount].VisRange = str2num(script2);
				break;
			default:
				break;
			}
		}
	} while (strcmp((char*)script1,"EOF") && !feof(wscfile));
	fclose(wscfile);
}

void cRaces::SetRace(P_CHAR pc, int race)
{
	int so = calcSocketFromChar(pc);
	short colorlist = addrandomcolor(pc,(char*)Races[race]->HairBeardList.c_str());

	pc->race=race;

	pc->skin=addrandomcolor(pc, (char*)Races[race]->SkinList.c_str());

	if(Races[race]->NoHair)
	{
		vector<SERIAL> vecContainer = contsp.getData(pc->serial);
		for (unsigned int ci=0;ci<vecContainer.size();ci++)
		{
			P_ITEM pi = FindItemBySerial(vecContainer[ci]);
			if (pi != NULL)
				if ((pi->contserial == pc->serial) && (pi->layer==0x0B))
				{
					Items->DeleItem(pi);
				}
		}
	}

	if(Races[race]->NoBeard)
	{
		vector<SERIAL> vecContainer = contsp.getData(pc->serial);
		for (unsigned int ci=0;ci<vecContainer.size();ci++)
		{
			P_ITEM pi = FindItemBySerial(vecContainer[ci]);
			if (pi != NULL)
				if ((pi->layer==0x10) && (pi->contserial==pc->serial))
				{
					Items->DeleItem(pi);
				}
		}
	}

	if(Races[race]->BeardReq>0)
	{
		if((Races[race]->BeardReq==1 && pc->id2==0x90) || (Races[race]->BeardReq==2 && pc->id2==0x91) || (Races[race]->BeardReq==3))
		{
			vector<SERIAL> vecContainer = contsp.getData(pc->serial);
			for (unsigned int ci=0;ci<vecContainer.size();ci++)
			{
				P_ITEM pi = FindItemBySerial(vecContainer[ci]);
				if ( pi != NULL)
					if ((pi->layer==0x10) && (pi->contserial==pc->serial))
					{
						Items->DeleItem(pi);
					}
			}
			int beardstyle=RandomBeardStyle();
			P_ITEM pi = Items->SpawnItem(so,pc,1, "#", 0, 0x20, beardstyle, 0x0462,0,0);
			if(pi == NULL) return;//AntiChrist to preview crashes
			pi->setColor(colorlist);
			pi->SetContSerial(pc->serial);
			pi->layer=0x10;
		}
	}

	if(Races[race]->HairReq>0)
	{
		if((Races[race]->HairReq==1 && pc->id2==0x90) || (Races[race]->HairReq==2 && pc->id2==0x91)|| (Races[race]->HairReq==3))
		{
			vector<SERIAL> vecContainer = contsp.getData(pc->serial);
			for (unsigned int ci = 0; ci < vecContainer.size(); ci++)
			{
				P_ITEM pi = FindItemBySerial(vecContainer[ci]);
				if (pi != NULL)
					if ((pi->layer == 0x0B) && (pi->contserial == pc->serial))
					{
						Items->DeleItem(pi);
					}
			}
			int hairstyle = RandomHairStyle();
			P_ITEM pi = Items->SpawnItem( so, pc, 1, "#", 0, 0x20, hairstyle, 0x0462, 0, 0 );
			if (pi == NULL)
				return;
			pi->setColor(colorlist);
			pi->SetContSerial(pc->serial);
			pi->layer=0x0B;
		}
	}

	if(Races[race]->NightSight)
		pc->fixedlight=3;

	if(Races[race]->StartStr>0)
	{
		pc->st = Races[race]->StartStr;
		pc->st2 = Races[race]->StartStr;
	}

	if(Races[race]->StartInt>0)
	{
		pc->in = Races[race]->StartInt;
		pc->in2 = Races[race]->StartInt;
	}

	if(Races[race]->StartDex>0)
		pc->setDex(Races[race]->StartDex);

	if(Races[race]->BaseAR>0)
		pc->def+=Races[race]->BaseAR;
	
	if(Races[race]->startpos.x>0)
		pc->pos.x=Races[race]->startpos.x;

	if(Races[race]->startpos.y>0)
		pc->pos.y=Races[race]->startpos.y;

	if(Races[race]->startpos.z>0)
		pc->pos.z=Races[race]->startpos.z;

	updatechar(pc);
	statwindow(so, pc);
	teleport(pc);
}

int cRaces::RandomHairStyle()
{
	switch( RandomNum(0,9) )
	{
	case 0:		return 0x3B;
	case 1:		return 0x3D;
	case 2:		return 0x44;
	case 3:		return 0x45;
	case 4:		return 0x46;
	case 5:		return 0x47;
	case 6:		return 0x48;
	case 7:		return 0x49;
	case 8:		return 0x4A;
	case 9:		return 0x44;
	default:	return 0x47;
	}
}

int cRaces::RandomBeardStyle()
{
	switch( RandomNum(0,7) )
	{
	case 0:		return 0x3B;
	case 1:		return 0x3E;
	case 2:		return 0x3F;
	case 3:		return 0x40;
	case 4:		return 0x41;
	case 5:		return 0x4B;
	case 6:		return 0x4C;
	case 7:		return 0x4D;
	default:	return 0x47;
	}
}

int cRaces::CheckRelation(P_CHAR pc_1, P_CHAR pc_2)
{
	if(pc_1 == pc_2)
		return 0;
	if(pc_1->isNpc() || pc_2->isNpc())
		return 0;

	int race1 = pc_1->race;
	int race2 = pc_2->race;
	if(race1 == race2)
		return 1;
	unsigned int rc;
	for(rc = 0; rc < Races[race1]->RacialAlly.size(); rc++)
		if(Races[race1]->RacialAlly[rc] == race2)
			return 1;
	for(rc = 0; rc < Races[race1]->RacialEnemy.size(); rc++)
		if(Races[race1]->RacialEnemy[rc] == race2)
			return 2;
	return 3;
}

bool cRace::CheckSkillUse(int skillnum)
{
	unsigned int skillcount;
	for(skillcount = 0; skillcount != CanUseSkill.size(); skillcount++)
		if(CanUseSkill[skillcount] == skillnum)
			return true;
	return false;
}

bool cRace::CheckSpellImune(int spellnum)
{
	unsigned int spellcount;
	for(spellcount = 0; spellcount != ImuneToSpell.size(); spellcount++)
		if(ImuneToSpell[spellcount] == spellnum)
			return true;
	return false;
}

bool cRace::CheckItemUse(int itemnum)
{
	unsigned int itemcount;
	for(itemcount = 0; itemcount != CantUseItem.size(); itemcount++)
		if(CantUseItem[itemcount] == itemnum)
			return false;
	return true;
}