#include "wolfpack.h"
#include "races.h"

vector<cRaces *> Races;

void cRaceManager::LoadRaceFile()
{
	int racecount=0;
	wscfile=fopen("races.scp", "r");
	if (wscfile == NULL) 
	{
		return;
	}
	do
	{
		readw2();
		if(!(strcmp((char*)script1,"SECTION")))
		{
			racecount=Races.size();
			Races.resize(Races.size()+1);
			Races[racecount]=new cRaces;
		}
		else
		{
			switch(script1[0])
			{
			case 'B':
			case 'b':
				if(!strcmp((char*)script1, "BEARDREQ"))
					Races[racecount]->BeardReq=str2num(script2);
				break;
			case 'C':
			case 'c':
				if(!strcmp((char*)script1, "CANUSESKILL"))
					Races[racecount]->CanUseSkill.push_back(str2num(script2));
				if(!strcmp((char*)script1, "CANTUSEITEM"))
					Races[racecount]->CantUseItem.push_back(str2num(script2));
				break;
			case 'D':
			case 'd':
				if(!strcmp((char*)script1, "DEXCAP"))
					Races[racecount]->DexCap=str2num(script2);
				break;
			case 'G':
			case 'g':
				if(!strcmp((char*)script1, "GENDER"))
					Races[racecount]->Gender=str2num(script2);
				break;
			case 'H':
			case 'h':
				if(!strcmp((char*)script1, "HAIRREQ"))
					Races[racecount]->HairReq=str2num(script2);
				if(!strcmp((char*)script1, "HAIRBEARDLIST"))
					strcpy(Races[racecount]->HairBeardList,script2);
				break;
			case 'I':
			case 'i':
				if(!strcmp((char*)script1, "ISPLAYERRACE"))
					Races[racecount]->IsPlayerRace=str2num(script2);
				if(!strcmp((char*)script1, "INTCAP"))
					Races[racecount]->IntCap=str2num(script2);
				if(!strcmp((char*)script1, "IMUNETOSPELL"))
					Races[racecount]->ImuneToSpell.push_back(str2num(script2));
				break;
			case 'N':
			case 'n':
				if(!strcmp((char*)script1, "NIGHTSIGHT"))
					Races[racecount]->NightSight=str2num(script2);
				if(!strcmp((char*)script1, "NOHAIR"))
					Races[racecount]->NoHair=str2num(script2);
				if(!strcmp((char*)script1, "NOBEARD"))
					Races[racecount]->NoBeard=str2num(script2);
				break;
			case 'R':
			case 'r':
				if(!strcmp((char*)script1, "RACENAME"))
					strcpy(Races[racecount]->RaceName,script2);
				if(!strcmp((char*)script1, "RACEALLY"))
					Races[racecount]->RacialAlly.push_back(str2num(script2));
				if(!strcmp((char*)script1, "RACEENEMY"))
					Races[racecount]->RacialEnemy.push_back(str2num(script2));
				break;
			case 'S':
			case 's':
				if(!strcmp((char*)script1, "STARTINT"))
					Races[racecount]->StartInt=str2num(script2);
				if(!strcmp((char*)script1, "STARTDEX"))
					Races[racecount]->StartDex=str2num(script2);
				if(!strcmp((char*)script1, "STARTSTR"))
					Races[racecount]->StartStr=str2num(script2);
				if(!strcmp((char*)script1, "STRCAP"))
					Races[racecount]->StrCap=str2num(script2);
				if(!strcmp((char*)script1, "SKINLIST"))
					strcpy(Races[racecount]->SkinList,script2);
				break;
			case 'V':
			case 'v':
				if(!strcmp((char*)script1, "VISRANGE"))
					Races[racecount]->VisRange=str2num(script2);
				break;
			default:
				break;
			}
		}
	} while (strcmp((char*)script1,"EOF") && !feof(wscfile));
	fclose(wscfile);
}

void cRaceManager::SetRace(P_CHAR pc,int race)
{
	int i=0,n=0;
	int so=calcSocketFromChar(DEREF_P_CHAR(pc));
	short colorlist=addrandomcolor(DEREF_P_CHAR(pc),(char*)Races[race]->HairBeardList);

	pc->race=race;

	pc->skin=addrandomcolor(DEREF_P_CHAR(pc),(char*)Races[race]->SkinList);

	if(Races[race]->NoHair)
	{
		vector<SERIAL> vecContainer = contsp.getData(pc->serial);
		for (int ci=0;ci<vecContainer.size();ci++)
		{
			i=calcItemFromSer(vecContainer[ci]);
			if (i!=-1)
				if ((items[i].contserial==pc->serial) && (items[i].layer==0x0B))
				{
					Items->DeleItem(i);
				}
		}
	}

	if(Races[race]->NoBeard)
	{
		vector<SERIAL> vecContainer = contsp.getData(pc->serial);
		for (int ci=0;ci<vecContainer.size();ci++)
		{
			i=calcItemFromSer(vecContainer[ci]);
			if (i!=-1)
				if ((items[i].layer==0x10) && (items[i].contserial==pc->serial))
				{
					Items->DeleItem(i);
				}
		}
	}

	if(Races[race]->BeardReq>0)
	{
		if((Races[race]->BeardReq==1 && pc->id2==0x90) || (Races[race]->BeardReq==2 && pc->id2==0x91) || (Races[race]->BeardReq==3))
		{
			vector<SERIAL> vecContainer = contsp.getData(pc->serial);
			for (int ci=0;ci<vecContainer.size();ci++)
			{
				i=calcItemFromSer(vecContainer[ci]);
				if (i!=-1)
					if ((items[i].layer==0x10) && (items[i].contserial==pc->serial))
					{
						Items->DeleItem(i);
					}
			}
			int beardstyle=RandomBeardStyle();
			n=Items->SpawnItem(so,DEREF_P_CHAR(pc),1, "#", 0, 0x20, beardstyle, 0x04, 0x62,0,0);
			if(n==-1) return;//AntiChrist to preview crashes
			const P_ITEM pi=MAKE_ITEMREF_LR(n);	// on error return
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
			for (int ci=0;ci<vecContainer.size();ci++)
			{
				i=calcItemFromSer(vecContainer[ci]);
				if (i!=-1)
					if ((items[i].layer==0x0B) && (items[i].contserial==pc->serial))
					{
						Items->DeleItem(i);
					}
			}
			int hairstyle=RandomHairStyle();
			n=Items->SpawnItem(so,DEREF_P_CHAR(pc),1, "#", 0, 0x20, hairstyle, 0x04, 0x62,0,0);
			if(n==-1) return;//AntiChrist to preview crashes
			const P_ITEM pi=MAKE_ITEMREF_LR(n);	// on error return
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


	updatechar(DEREF_P_CHAR(pc));
	statwindow(so, DEREF_P_CHAR(pc));
}

int cRaceManager::RandomHairStyle()
{
	int i=0;
	i=RandomNum(0,9);
	switch( i )
	{
	case 0:
		return 0x3B;
	case 1:
		return 0x3D;
	case 2:
		return 0x44;
	case 3:
		return 0x45;
	case 4:
		return 0x46;
	case 5:
		return 0x47;
	case 6:
		return 0x48;
	case 7:
		return 0x49;
	case 8:
		return 0x4A;
	case 9:
		return 0x44;
	default:
		return 0x47;
	}
}

int cRaceManager::RandomBeardStyle()
{
	int i=0;
	i=RandomNum(0,7);
	switch( i )
	{
	case 0:
		return 0x3B;
	case 1:
		return 0x3E;
	case 2:
		return 0x3F;
	case 3:
		return 0x40;
	case 4:
		return 0x41;
	case 5:
		return 0x4B;
	case 6:
		return 0x4C;
	case 7:
		return 0x4D;
	default:
		return 0x47;
	}
}

int cRaceManager::CheckRelation(P_CHAR pc_1, P_CHAR pc_2)
{
	char temp[512]={' '};
	if(pc_1==pc_2)
		return 0;
	if(pc_1->isNpc())
		return 0;
	if(pc_2->isNpc())
		return 0; 
	int race1=pc_1->race;
	int race2=pc_2->race;
	if(race1==race2)
		return 1;
	int rc=0;
	for(rc=0;rc<Races[race1]->RacialAlly.size();rc++)
		if(Races[race1]->RacialAlly[rc] == race2)
			return 1;
	for(rc=0;rc<Races[race1]->RacialEnemy.size();rc++)
		if(Races[race1]->RacialEnemy[rc] == race2)
			return 2;
	return 3;
}

bool cRaces::CheckSkillUse(int skillnum)
{
	int skillcount=0;
	for(skillcount=0;skillcount!=CanUseSkill.size();skillcount++)
		if(CanUseSkill[skillcount] = skillnum)
			return true;
	return false;
}

bool cRaces::CheckSpellImune(int spellnum)
{
	int spellcount=0;
	for(spellcount=0;spellcount!=ImuneToSpell.size();spellcount++)
		if(ImuneToSpell[spellcount] = spellnum)
			return true;
	return false;
}

bool cRaces::CheckItemUse(int itemnum)
{
	int itemcount=0;
	for(itemcount=0;itemcount!=CantUseItem.size();itemcount++)
		if(CantUseItem[itemcount] = itemnum)
			return false;
	return true;
}