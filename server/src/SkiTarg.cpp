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

// Wolfpack Includes
#include "wolfpack.h"
#include "itemid.h"
#include "SndPkg.h"
#include "guildstones.h"
#include "combat.h"
#include "regions.h"
#include "srvparams.h"
#include "tilecache.h"
#include "classes.h"
#include "mapstuff.h"
#include "network.h"
#include "wpdefmanager.h"
#include "makemenus.h"
#include "skills.h"
#include "network/uosocket.h"

#undef DBGFILE
#define DBGFILE "skiTarg.cpp"
#include "debug.h"

P_ITEM Check4Pack(UOXSOCKET s)
{
	P_CHAR pc_currchar = currchar[s];
	P_ITEM pi_pack = Packitem(pc_currchar);
	if (pi_pack == NULL)
	{
		sysmessage(s, tr("Time to buy a backpack") );
	}
	return pi_pack;
}

bool CheckInPack(UOXSOCKET s, PC_ITEM pi)
{
	P_ITEM pPack=Check4Pack(s);
	if (!pPack) return false;
	if (pi->contserial!=pPack->serial)
	{
		sysmessage(s,tr("You can't use material outside your backpack") );
		return false;
	}
	return true;
}

static bool ForgeInRange(int s)
{
/*	P_CHAR pc = currchar[s];
	bool rc = false;

	unsigned int StartGrid=mapRegions->StartGrid(pc->pos);
	unsigned int increment=0, checkgrid, a;
#pragma note("Replace by Region Item iterator")
	for (checkgrid=StartGrid+(increment*mapRegions->GetColSize());increment<3;increment++, checkgrid=StartGrid+(increment*mapRegions->GetColSize()))
	{
		for (a=0;a<3;a++)
		{
			cRegion::raw vecEntries = mapRegions->GetCellEntries(checkgrid+a);
			cRegion::rawIterator it = vecEntries.begin();
			for (; it != vecEntries.end(); ++it)
			{
				P_ITEM pi = FindItemBySerial(*it);
				if (pi != NULL)
					if(IsForge(pi->id()))
						if(iteminrange(s,pi,3))
							rc = true;
			}
		}
	}
	return rc;*/
	return true;
}

static bool AnvilInRange(int s)
{
/*	P_CHAR pc = currchar[s];
	bool rc = false;

	unsigned int StartGrid=mapRegions->StartGrid(pc->pos);
	unsigned int increment=0, checkgrid, a;
#pragma note("Replace by region item iterator")
	for (checkgrid=StartGrid+(increment*mapRegions->GetColSize());increment<3;increment++, checkgrid=StartGrid+(increment*mapRegions->GetColSize()))
	{
		for (a=0;a<3;a++)
		{
			cRegion::raw vecEntries = mapRegions->GetCellEntries(checkgrid+a);
			cRegion::rawIterator it = vecEntries.begin();
			for (; it != vecEntries.end(); ++it)
			{
				P_ITEM pi = FindItemBySerial(*it);
				if (pi != NULL)
					if(IsAnvil(pi->id()))
						if(iteminrange(s,pi,3))
							rc = true;
			}
		}
	}
	return rc;*/
	return true;
}

////////////////////
// name:	AnvilTarget2
// history:	by Duke, 28 March 2000
// Purpose:	little helper function for cSkills::Smith()
//			checks for anvil in reach and enough material
//			and invokes appropriate Makemenu
//
static void AnvilTarget2(int s,				// socket #
						 P_ITEM pi,			// material item
						 int ma,			// minimum amount
						 int mm,			// makemenu # to invoke from create.scp
						 char* matname)		// name of the metal
{
/*	if (!AnvilInRange(s))
		
	else
	{
		P_CHAR pc_currchar = currchar[s];
		P_ITEM pi_pack = Packitem(pc_currchar);
		if (pi_pack == NULL) 
			return;

		int amt = pi_pack->CountItems( pi->id(), pi->color());
		if ((itemmake[s].has=amt) < ma)
		{
			sysmessage(s, tr("You don't have enough %1 ingots to make anything.").arg(matname) );
		}
		else
			Skills->MakeMenu(s,mm,BLACKSMITHING);
	}*/
}

void cSkills::TasteIDTarget(int s)
{
/*	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	P_CHAR pc_currchar = currchar[s];
	if (pi && !pi->isLockedDown()) // Ripper
	{
		if(!( pi->type()==19 || pi->type()==14))
		{
			sysmessage(s, tr("You cant taste that!") );
			return;
		}
		if (!CheckSkill(pc_currchar, TASTEID, 0, 250))
		{
			sysmessage(s, tr("You can't quite tell what this item is...") );
		}
		else
		{
			if( pi->corpse() )
			{
				sysmessage(s, tr("You have to use your forensics evalutation skill to know more on this corpse.") );
				return;
			}
			
			// Identify Item by Antichrist // Changed by MagiusCHE)
			if (CheckSkill(pc_currchar, TASTEID, 250, 500))
				if (pi->name2() != "#")
					pi->setName( pi->name2() ); // Item identified! -- by Magius(CHE)
				
				// ANTICHRIST -- FOR THE "#" BUG -- now you see the real name
				if( pi->name() == "#" ) 
					pi->getName(temp2);
				else 
					strcpy( (char*)temp2, pi->name().ascii() );

				sysmessage(s, tr("You found that this item appears to be called: %1").arg(temp2)  );
				
				if (CheckSkill(pc_currchar, TASTEID, 250, 500))
				{
					if((pi->poisoned>0) || (pi->morex==4 && pi->morey==6 && pi->morez==1))
						sysmessage(s,tr("This item is poisoned!") );
					else
						sysmessage(s,tr("This item shows no poison.") );
					
					// Show Creator by Magius(CHE)
					if (CheckSkill(pc_currchar, TASTEID, 250, 500))
					{
						if (pi->creator.size()>0)
						{
							if (pi->madewith>0) 
								sysmessage(s, tr("It is %1 by %2").arg(skill[pi->madewith-1].madeword).arg(pi->creator.c_str()) ); // Magius(CHE)
							else if (pi->madewith<0) sysmessage(s, tr("It is %1 by %2").arg(skill[0-pi->madewith-1].madeword).arg(pi->creator.c_str()) ); // Magius(CHE)
							else sysmessage(s, tr("It is made by %1").arg(pi->creator.c_str()) ); // Magius(CHE)
						} else sysmessage(s, tr("You don't know its creator!") );
					} else sysmessage(s, tr("You can't know its creator!") );
				}
		}
	}*/
}

void cSkills::TreeTarget( int s )
{
/*	int lumber=0;
	int px,py,cx,cy;
	static unsigned int logtime[max_res_x][max_res_y];//see mine for values...they were 1000 also here
	static int logamount[max_res_x][max_res_y];
	int a, b, c;
	long int curtime=uiCurrentTime;
	P_CHAR pc = currchar[s];

	//AntiChrist
	//Logging stamina
	if (SrvParams->logstamina()<0 && abs(SrvParams->logstamina())>pc->stm)
	{
		sysmessage(s, tr("You are too tired to chop.") );
		return;
	}

	pc->stm+=SrvParams->logstamina();
	if(pc->stm<0) pc->stm=0;
	if(pc->stm>pc->effDex()) pc->stm=pc->effDex();
	updatestats(pc,2);
	
	if(logtime[0][0]==0)//First time done since server started
	{
		logtime[0][0]=17;//lucky number ;-)
		logamount[0][0]=SrvParams->logs();
		LogMessage("Lumberjacking startup, setting tree values and times...");
		
		//for(a=1;a<410;a++)
		for(a=1;a<max_res_x;a++)//AntiChrist bug fix
		{
			for(b=1;b<max_res_y;b++)
			{
				logamount[a][b]=SrvParams->logs();
				SetTimerSec(&logtime[a][b],SrvParams->logtime());
			}
		}
		LogMessage("Done.");
	}

	//AntiChrist
	if (buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) 
	return; // Test if use canceled the mining request

	//AntiChrist
	px=((buffer[s][0x0b]<<8)+(buffer[s][0x0c]%256));
	py=((buffer[s][0x0d]<<8)+(buffer[s][0x0e]%256));
	// amount=world_resource_check(px,py,4,1); working... but not finished
	cx=abs(pc->pos.x-px);
	cy=abs(pc->pos.y-py);
	if(!((cx<=5)&&(cy<=5)))
	{
		sysmessage(s, tr("You are to far away to reach that") );
		return;
	}

	a=pc->pos.x/SrvParams->logarea(); //Zippy
	b=pc->pos.y/SrvParams->logarea();
		
	if(a>=max_res_x || b>=max_res_y) return;
	
	if(logtime[a][b]<=curtime)
	{
		for(c=0;c<SrvParams->logs();c++)//Find howmany 10 min periods have been by, give 1 more for each period.
		{
			if((logtime[a][b]+(c*SrvParams->logtime()*MY_CLOCKS_PER_SEC))<=curtime && logamount[a][b]<SrvParams->logs())
				logamount[a][b]+=SrvParams->lograte();//AntiChrist
			else break;
		}
		SetTimerSec(&logtime[a][b],SrvParams->logtime());
	}
	
	if(logamount[a][b]>SrvParams->logs()) logamount[a][b]=SrvParams->logs();
	
	if(logamount[a][b]<=0)
	{
		sysmessage(s, tr("There is no more wood here to chop.") );
		return;
	}
	
	P_ITEM pi_pack = Packitem(pc);
	if (pi_pack == NULL) {sysmessage(s,tr("No backpack to store logs") ); return; } //LB
	
	if (pc->onHorse()) 
		action(s,0x1C);
	else 
		action(s,0x0D);
	//soundeffect(s,0x01,0x3E);
	
	if (!Skills->CheckSkill(pc, LUMBERJACKING, 0, 1000)) 
	{
		sysmessage(s, tr("You chop for a while, but fail to produce any usable wood.") );
		if(logamount[a][b]>0 && rand()%2==1) logamount[a][b]--;//Randomly deplete resources even when they fail 1/2 chance you'll loose wood.
		return;
	}
	
	if(logamount[a][b]>0) logamount[a][b]--;
	
	if ((buffer[s][1]==1 || buffer[s][1]==0)&&(buffer[s][2]==0)
		&&(buffer[s][3]==1)&&(buffer[s][4]==0))
	{
		//
		//29/11/99
		//
		//AntiChrist - new implementation of triggered logging.
		//
		//Log creation is now done all by trigger
		//so that we can create new scripted minerals.
		//(put resource.logtrigger=0 to disable this feature
		//and use standard logging!)
		//
		if(SrvParams->logtrigger()>0)
		{
			//pc->targtrig=resource.logtrigger;
		} else
		{//normal mining skill
			
			P_ITEM pi_c = Items->SpawnItem(s, pc, 10, "#", 1, 0x1B, 0xE0, 0, 1, 1);
			if(pi_c == NULL) return;//AntiChrist to prevent crashes
			if (pi_c->amount() > 10) sysmessage(s, tr("You place more logs in your pack.") );
			else sysmessage(s, tr("You place some logs in your pack.") );
			
			lumber=1;
		}
	}*/
}

void cSkills::GraveDig(int s) // added by Genesis 11-4-98
{
	int	nAmount, nFame;
	char iID=0;
	QString listSect;
	P_ITEM ci = NULL;
	
	P_CHAR pc = currchar[s];

	Karma(pc, NULL,-2000); // Karma loss no lower than the -2 pier
	
	if(pc->onHorse())
		action(s,0x1A);
	else
		action(s,0x0b);
	//soundeffect(s,0x01,0x25);
	if(!Skills->CheckSkill(pc, MINING, 0, 800)) 
	{
		sysmessage(s, tr("You sifted through the dirt and found nothing.") );
		return;
	}
	
	nFame = pc->fame;
	if(pc->onHorse())
		action(s,0x1A);
	else
		action(s,0x0b);
	//soundeffect(s,0x01,0x25);
	int nRandnum=rand()%13;
	switch(nRandnum)
	{
	case 2:
		listSect = DefManager->getRandomListEntry( "COMMON_UNDEAD_LOWLVL" );
		if( !listSect.isEmpty() )
		{
			Npcs->createScriptNpc( s, NULL, listSect );
			sysmessage(s, tr("You have disturbed the rest of a vile undead creature.") );
		}	
		break;
	case 4:
		listSect = DefManager->getRandomListEntry( "COMMON_ARMOR_AND_SHIELDS" );
		if( !listSect.isEmpty() )
		{
			ci = Items->createScriptItem( listSect );
			if( ci != NULL )
			{
				if((ci->id()>=7026)&&(ci->id()<=7035))
					sysmessage(s, tr("You unearthed an old shield and placed it in your pack") );
				else
					sysmessage(s, tr("You have found an old piece armor and placed it in your pack.") );
			}
		}
		break;
	case 5:
		//Random treasure between gems and gold
		nRandnum=rand()%2;
		if(nRandnum)
		{ // randomly create a gem and place in backpack
			QString listSect = DefManager->getRandomListEntry( "COMMON_GEMS" );
			if( !listSect.isEmpty() )
			{
				Items->createScriptItem( listSect );
				sysmessage(s, tr("You place a gem in your pack.") );
			}
		}
		else
		{ // Create between 1 and 15 goldpieces and place directly in backpack
			nAmount=1+(rand()%15);
			//addgold(s,nAmount);
			//goldsfx(s,nAmount);
			if (nAmount==1)
				sysmessage(s, tr("You unearthed %1 gold coin.").arg(nAmount) );
			else
				sysmessage(s, tr("You unearthed %1 gold coins.").arg(nAmount) );
		}
		break;
	case 6:
		if(nFame<500)
			listSect = DefManager->getRandomListEntry( "COMMON_UNDEAD_LOWLVL" );
		else
			listSect = DefManager->getRandomListEntry( "COMMON_UNDEAD_MEDLVL" );
		if( !listSect.isEmpty() )
		{
			Npcs->createScriptNpc( s, NULL, listSect );
			sysmessage(s, tr("You have disturbed the rest of a vile undead creature.") );
		}	
		break;
	case 8:
		listSect = DefManager->getRandomListEntry( "COMMON_WEAPONS" );
		if( !listSect.isEmpty() )
		{
			Items->createScriptItem( listSect );
			sysmessage(s, tr("You unearthed a old weapon and placed it in your pack.") );
		}
		break;
	case 10:
		if(nFame<1000)
			listSect = DefManager->getRandomListEntry( "COMMON_UNDEAD_MEDLVL" );
		else
			listSect = DefManager->getRandomListEntry( "COMMON_UNDEAD_HILVL" );
		if( !listSect.isEmpty() )
		{
			Npcs->createScriptNpc( s, NULL, listSect );
			sysmessage(s, tr("You have disturbed the rest of a vile undead creature.") );
		}	
		break;
	case 12:
		if(nFame>1000)
			listSect = DefManager->getRandomListEntry( "COMMON_UNDEAD_HILVL" );
		else
			listSect = DefManager->getRandomListEntry( "COMMON_UNDEAD_MEDLVL" );
		if( !listSect.isEmpty() )
		{
			Npcs->createScriptNpc( s, NULL, listSect );
			sysmessage(s, tr("You have disturbed the rest of a vile undead creature.") );
		}	
		break;
	default:
		nRandnum=rand()%2;
		switch(nRandnum)
		{
			case 1:
				nRandnum=rand()%12;
				switch(RandomNum(0, 11))
				{
					case 0: iID=0x11; break;
					case 1: iID=0x12; break;
					case 2: iID=0x13; break;
					case 3: iID=0x14; break;
					case 4: iID=0x15; break;
					case 5: iID=0x16; break;
					case 6: iID=0x17; break;
					case 7: iID=0x18; break;
					case 8: iID=0x19; break;
					case 9: iID=0x1A; break;
					case 10: iID=0x1B; break;
					case 11: iID=0x1C; break;
				}
				Items->SpawnItem(s, pc, 1, NULL, 0, 0x1b, iID, 0x00, 1, 1);
				sysmessage(s, tr("You have unearthed some old bones and placed them in your pack.") );
				break;
			default: // found an empty grave
				sysmessage(s, tr("This grave seems to be empty.") );
		}
	}
}

///////////////////////
// name:	SmeltOre2
// history:	by Duke, 31 March 2000
// Purpose: helper function for cSkills::SmeltOre()
//			checks if the player's mining skill meets the requirements
//			if yes, he'll get the specified ingots and appropriate messages
// Remarks: CheckSkill does NOT use minskill (cause it was like that before)
//
static void SmeltOre2(	int s,					// current char's socket #
						int minskill,			// minimum skill required for ore color
						short id,short color,	// item ID & color of ingot to be created
						char *orename)
{
	P_CHAR pc_currchar = currchar[s];

//	int smi = pc_currchar->smeltitem;		// index of ore item
	const P_ITEM pi = FindItemBySerial(pc_currchar->smeltitem());	// on error return
	if ( pi == NULL )
		return;


	if (pc_currchar->skill(MINING) < minskill)
	{
		sysmessage(s, tr("You have no idea what to do with this strange ore") );
		return;					
	}
	if(!Skills->CheckSkill(pc_currchar, MINING, 0, 1000))
	{
		if( pi->amount() == 1 )
		{
			sysmessage(s, tr("Your hand slips and the last of your materials are destroyed.") );
			Items->DeleItem(pi);
		}
		else
		{
			sysmessage(s, tr("Your hand slips and some of your materials are destroyed.") );
			pi->setAmount( pi->amount() / 2 );
			pi->update();
		}
	}
	else
	{
		char tmp[100];
		int numore = pi->amount() * 2;			// one ore gives two ingots
		sprintf(tmp,"%s Ingot",orename);
		
		cItem* Ingot = Items->SpawnItem(pc_currchar, numore, tmp, 1, id, color, 1);
		if (Ingot)
		{
			Ingot->setWeight( 2 ); // Thats 0.2 stone
			Ingot->update();
		}

		sysmessage(s, tr("You have smelted your ore") );
		sysmessage(s, tr("You place some %1 ingots in your pack.").arg((char)tolower(*orename) + QString(orename+1)));
		Items->DeleItem(pi);
	}
}

//////////////////////////
// Function:	SmeltOre
// History:		unknown, colored ore by Cork,
//				31 March 2000 totally revamped by Duke
//				16.9.2000 removed array access and revamped a bit more (Duke)
//
// Purpose:		checks if targeted item is a forge and in range
//				then executes the smelting function with the appropriate
//				minskill and ingot type
// Remarks:		NOTE: ingot color is different from ore color for gold, silver & copper!

void cSkills::SmeltOre(int s)
{
	/*P_CHAR pc_currchar = currchar[s];

	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (pi && !pi->isLockedDown()) // Ripper
	{
		if(	IsForge(pi->id()) )
		{
			if(!iteminrange(s,pi,3))		//Check if the forge is in range
				sysmessage(s, tr("You cant smelt here.") );
			else
			{
				P_ITEM pix = FindItemBySerial(pc_currchar->smeltitem());	// on error return
				if ( pix == NULL)
					return;
				switch ( pix->color() )
				{
					case 0x0000:	SmeltOre2(s,   0, 0x1BF2, 0x0961,"Iron");	break;
					case 0x0466:	SmeltOre2(s, 850, 0x1BF2, 0x0466,"Golden");	break;
					case 0x046E:	SmeltOre2(s, 750, 0x1BF2, 0x046E,"Copper");	break;
					case 0x0961:	SmeltOre2(s, 790, 0x1BF2, 0x0000,"Silver");	break;
					case 0x0150:	SmeltOre2(s, 900, 0x1BF2, 0x0150,"Agapite");break;
					case 0x0386:	SmeltOre2(s, 650, 0x1BF2, 0x0386,"Shadow");	break;
					case 0x022f:	SmeltOre2(s, 950, 0x1BF2, 0x022F,"Verite");	break;
					case 0x02e7:	SmeltOre2(s, 800, 0x1BF2, 0x02E7,"Bronze");	break;
					case 0x02c3:	SmeltOre2(s, 700, 0x1BF2, 0x02C3,"Merkite");break;
					case 0x0191:	SmeltOre2(s, 990, 0x1BF2, 0x0191,"Mythril");break;
					default:		
						LogError("switch reached default");
				}
			}
		}
	}

	pc_currchar->setSmeltItem( INVALID_SERIAL );*/
}

void cSkills::Wheel(int s, int mat)//Spinning wheel
{
	/*int tailme=0;
	P_CHAR pc_currchar = currchar[s];
	
	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (!pi) return;
	
	if( pi->id() >= 0x10A4 || pi->id() <= 0x10A6 )
	{
		if(iteminrange(s,pi,3))
		{
			if (!Skills->CheckSkill(currchar[s],TAILORING, 0, 1000)) 
			{
				sysmessage(s, tr("You failed to spin your material.") );
				return;
			}
			sysmessage(s, tr("You have successfully spun your material.") );

//			int ti = pc_currchar->tailitem;
			const P_ITEM pti = FindItemBySerial(pc_currchar->tailitem());	// on error return
			
			if (mat==YARN)
			{
				pti->setName( "#" );
				pti->setId(0x0E1D);
				pti->setAmount( pti->amount() * 3 );
			}
			else if (mat==THREAD)
			{
				pti->setName( "#" );
				pti->setId(0x0FA0);
				pti->setAmount( pti->amount() * 3 );
			}

			pti->priv |= 0x01;
			pti->update();
			tailme=1;
		}
	}
	pc_currchar->setTailItem( INVALID_SERIAL );
	if(!tailme) sysmessage(s,tr("You cant tailor here.") );*/
}

void cSkills::Loom(int s)
{
/*	int tailme=0;
	P_CHAR pc_currchar = currchar[s];
	
	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (pi && !pi->isLockedDown()) // Ripper
	{
		if ( pi->id() >= 0x105F && pi->id() <= 0x1066 )
		{
			if(iteminrange(s,pi,3))
			{
//				int ti = pc_currchar->tailitem;
				const P_ITEM pti = FindItemBySerial(pc_currchar->tailitem());	// on error return
				if (pti == NULL)
					return;
				if( pti->amount() < 5 )
				{
					sysmessage(s, tr("You do not have enough material to make anything!") );
					return;
				}

				if (!Skills->CheckSkill(pc_currchar,TAILORING, 300, 1000)) 
				{
					sysmessage(s, tr("You failed to make cloth.") );
					sysmessage(s, tr("You have broken and lost some material!") );
					
					if ( pti->amount() != 0 ) 
						pti->ReduceAmount( 1 + ( rand() % ( pti->amount() ) ) );
					else 
						pti->ReduceAmount( 1 );

					// It's auto-deleted
					pti->update();
					return;
				}
				
				if( pti->id()==0x0E1E || pti->id()==0x0E1D || pti->id()==0x0E1F )	// yarn
				{
					sysmessage(s, tr("You have made your cloth.") );

					pti->setName( "#" );
					pti->setId(0x175D);
					pti->priv |= 0x01;
					pti->setAmount( static_cast<unsigned short> ( pti->amount() * 0.25 ) );
				}
				else if( pti->id()==0x0FA0 || pti->id()==0x0FA1 )	// thread
				{
					sysmessage(s, tr("You have made a bolt of cloth.") );

					pti->setName( "#" );
					pti->setId(0x0F95);
					pti->priv |= 1;
					pti->setAmount( static_cast<unsigned short> ( pti->amount() * 0.25 ) );
				}
				pti->update();
				tailme=1;
			}
		}
	}
	pc_currchar->setTailItem( INVALID_SERIAL );
	if(!tailme) 
		sysmessage(s, tr("You cant tailor here.") );*/
}

void cSkills::MakeDough(int s)
{
/*	bool tailme = false;
	P_CHAR pc_currchar = currchar[s];
	
	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (pi && !pi->isLockedDown()) // Ripper
	{
		if(pi->id()==0x103A)
		{
			if(iteminrange(s,pi,3))
			{
				if (!Skills->CheckSkill(pc_currchar,COOKING, 0, 1000)) 
				{
					sysmessage(s, tr("You failed to mix, and spilt your water.") );
					return;
				}
				sysmessage(s, tr("You have mixed very well to make your dough.") );
				
				const P_ITEM pti=FindItemBySerial(pc_currchar->tailitem());	// on error return
				if ( pti == NULL)
					return;
				pti->setName( "#" );
				
				pti->setId(0x103D);
				pti->priv |= 0x01;
				pti->setAmount( pti->amount() * 2 );
				
				pti->update();
				tailme = true;
			}
		}
	}
	pc_currchar->setTailItem( INVALID_SERIAL );
	if(!tailme) 
		sysmessage(s, tr("You cant mix here.") );*/
}

void cSkills::MakePizza(int s)
{
	/*int tailme=0;
	P_CHAR pc_currchar = currchar[s];
	
	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (pi && !pi->isLockedDown()) // Ripper
	{
		if(pi->id()==0x103D)
		{
			if(iteminrange(s,pi,3))
			{
				if (!Skills->CheckSkill(pc_currchar,COOKING, 0, 1000)) 
				{
					sysmessage(s, tr("You failed to mix.") );
					Items->DeleItem(pi);
					return;
				}
				sysmessage(s, tr("You have made your uncooked pizza, ready to place in oven.") );
				
				const P_ITEM pti = FindItemBySerial(pc_currchar->tailitem());	// on error return
				if ( pti == NULL )
					return;
				pti->setName( "#" );
				
				pti->setId(0x1083);
				pti->priv |= 0x01;
				pti->setAmount( pti->amount() * 2 );
				
				pti->update();
				tailme = 1;
			}
		}
	}
	pc_currchar->setTailItem( INVALID_SERIAL);
	if(!tailme) 
		sysmessage(s, tr("You cant mix here.") );*/
}

/*
* I decided to base this on how OSI will be changing detect hidden. 
* The greater your skill, the more of a range you can detect from target position.
* Hiders near the center of the detect circle are easier to detect than ones near
* the edges of the detect circle. Also low skill detecters cannot find high skilled
* hiders as easily as low skilled hiders.
*/

void cSkills::EnticementTarget1(UOXSOCKET s)
{
	P_CHAR pc = FindCharBySerPtr(buffer[s]+7);
	if( pc == NULL ) return;

	P_CHAR pc_currchar = currchar[s];
	P_ITEM inst = GetInstrument( pc->socket() );
	if (inst == NULL) 
	{
		sysmessage(s, tr("You do not have an instrument to play on!") );
		return;
	}
	if ( pc->isInvul() || pc->shop() || // invul or shopkeeper
		pc->npcaitype()==0x01 || // healer
		pc->npcaitype()==0x04 || // tele guard
		pc->npcaitype()==0x06 || // chaos guard
		pc->npcaitype()==0x07 || // order guard
		pc->npcaitype()==0x09)   // city guard
	{
		sysmessage(s, tr(" You cant entice that npc!") );
		return;
	}
	if (pc->inGuardedArea())
	{
		sysmessage(s, tr(" You cant do that in town!") );
		return;
	}
	addid1[s]=buffer[s][7];
	addid2[s]=buffer[s][8];
	addid3[s]=buffer[s][9];
	addid4[s]=buffer[s][10];

	if (pc->isPlayer())
		sysmessage(s, tr("You cannot entice other players.") );
	else
	{
		target(s, 0, 1, 0, 82, (char*)tr("You play your music, luring them near. Whom do you wish them to follow?").latin1() );
		PlayInstrumentWell(pc_currchar->socket(), inst);
	}
}

void cSkills::EnticementTarget2(UOXSOCKET s)
{
	P_CHAR pc = FindCharBySerPtr(buffer[s]+7);
	if( pc == NULL ) return;
	P_CHAR pc_currchar = currchar[s];
	P_ITEM inst = GetInstrument( pc->socket() );
	if (inst == NULL) 
	{
		sysmessage(s, tr("You do not have an instrument to play on!") );
		return;
	}
	int res1 = CheckSkill(pc_currchar, ENTICEMENT, 0, 1000);
	int res2 = CheckSkill(pc_currchar, MUSICIANSHIP, 0, 1000);
	if (res1 && res2)
	{
		P_CHAR pc_target = FindCharBySerial(calcserial(addid1[s], addid2[s], addid3[s], addid4[s]));
		if ( pc_target == NULL ) return;
		pc_target->ftarg = pc->serial;
		pc_target->npcWander = 1;
		sysmessage(s, tr("You play your hypnotic music, luring them near your target.") );
		PlayInstrumentWell(pc_currchar->socket(), inst);
	}
	else 
	{
		sysmessage(s, tr("Your music fails to attract them.") );
		PlayInstrumentPoor(pc_currchar->socket(), inst);
	}
}

//////////////////////////
// name:	AlchemyTarget
// history: unknown, revamped by Duke,21.04.2000
// Purpose:	checks for valid reg and brings up gumpmenu to select potion
//			This is called after the user dblclicked a mortar and targeted a reg

void cSkills::AlchemyTarget(int s)
{
	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (!pi) return;
	
	switch (pi->id())
	{
/*	case 0x0F7B: ShowMenu( s, 991 );break;	// Agility,
	case 0x0F84: ShowMenu( s, 992 );break;	// Cure, Garlic
	case 0x0F8C: ShowMenu( s, 993 );break;	// Explosion, Sulfurous Ash
	case 0x0F85: ShowMenu( s, 994 );break;	// Heal, Ginseng
	case 0x0F8D: ShowMenu( s, 995 );break;	// Night sight
	case 0x0F88: ShowMenu( s, 996 );break;	// Poison, Nightshade
	case 0x0F7A: ShowMenu( s, 997 );break;	// Refresh, 
	case 0x0F86: ShowMenu( s, 998 );break;	// Strength,
	case 0x0E9B: break;	// Mortar*/
	default:
		if ( pi->id()>=0x1B11 && pi->id()<=0x1B1C )
		{
			MakeNecroReg(s,pi,pi->id());
			sysmessage(s, tr("You grind some bone into powder.") );
		}
		else
			sysmessage(s, tr("That is not a valid reagent.") );
	}
}

void cSkills::CreateBandageTarget(int s)//-Frazurbluu- rewrite of tailoring to current OSI
{
	const P_ITEM pi = FindItemBySerPtr(buffer[s]+7);
	short int amt=0;
	P_CHAR pc_currchar = currchar[s];

	if (pi && !pi->isLockedDown()) // Ripper
	{
		unsigned short col1 = pi->color(); //-Frazurbluu- added color retention for bandage cutting from cloth

		if ((IsCloth(pi->id()) && (IsCutCloth(pi->id()))))
		{
			amt = pi->amount();  //-Frazurbluu- changed to reflect current OSI 
			//soundeffect(s,0x02,0x48);
			sysmessage(s, tr("You cut some cloth into bandages, and put it in your backpack") );
			P_ITEM pi_c = Items->SpawnItem(s,pc_currchar,amt,"#",0,0x0E,0x21,col1,1,1);
			if(pi_c == NULL) return;
			// need to set amount and weight and pileable, note: cannot set pilable while spawning item -Fraz-
			pi_c->setWeight( 10 );
			pi_c->att=9;
			pi_c->setAmount( amt );
			pi_c->update();
			Items->DeleItem(pi);
			return;
		}	
		if( IsBoltOfCloth(pi->id()) )
		{
			if (pi->amount()>1)
				amt=(pi->amount()*50);//-Frazurbluu- changed to reflect current OSI 
			else
				amt=50;
			//soundeffect(s,0x02,0x48);
			P_ITEM pi_c = Items->SpawnItem(s,pc_currchar,1,"cut cloth",0,0x17,0x66,col1,1,1);
			if(pi_c == NULL) return;
			pi_c->setWeight( 10 );
			pi_c->setAmount( amt );
			pi_c->update();
			Items->DeleItem(pi);
			return;
		}
		if( IsHide(pi->id()) )
		{
			amt = pi->amount();
			//soundeffect(s,0x02,0x48);
			P_ITEM pi_c = Items->SpawnItem(s,pc_currchar,1,"leather piece",0,0x10,0x67,col1,1,1);
			if( !pi_c ) 
				return;
			pi_c->setWeight( 100 );
			pi_c->setAmount( amt );
			pi_c->update();
			Items->DeleItem(pi);
			return;
		}
		sysmessage(s, tr("You cannot cut anything from that item.") );
	}
}

////////////////////////
// name:	HealingSkillTarget
// history: unknown, revamped by Duke, 4.06.2000

void cSkills::HealingSkillTarget(UOXSOCKET s)
{
	signed short tempshort;

	P_ITEM pib = FindItemBySerial(addmitem[s]);	// item index of bandage
	
	P_CHAR pp = FindCharBySerPtr(buffer[s]+7); // pointer to patient
	if (pp != NULL)
	{
		P_CHAR ph = currchar[s];	// points to the healer
		if (!SrvParams->bandageInCombat() && (pp->war || ph->war))
		{
			P_CHAR pc_attacker = FindCharBySerial(ph->attacker); // Ripper...cant heal while in a fight
			if ( (pc_attacker != NULL) && pc_attacker->war)
			{
				sysmessage(s, tr("You can`t heal while in a fight!") );
				return;
			}
		}
		if(ph->pos.distance(pp->pos)>5)
		{
			sysmessage(s, tr("You are not close enough to apply the bandages.") );
			return;
		}
		if ((ph->isInnocent()) &&(ph->serial != pp->serial))
		{
	       if ((pp->crimflag()>0) ||(pp->isMurderer()))
		   {
		       criminal(ph);
		   }
		}
		
		if (pp->dead)
		{
			if (ph->skill(HEALING) < 800 || ph->skill(ANATOMY) < 800)
				sysmessage(s, tr("You are not skilled enough to resurrect") );
			else
			{
				int reschance = static_cast<int>((ph->baseSkill(HEALING)+ph->baseSkill(ANATOMY))*0.17);
				int rescheck=RandomNum(1,100);
				if (CheckSkill((ph),HEALING,800,1000) && CheckSkill((ph),ANATOMY,800,1000) && reschance<=rescheck)
					sysmessage(s, tr("You failed to resurrect the ghost") );
				else
				{
					Targ->NpcResurrectTarget(ph);
					sysmessage(s, tr("Because of your skill, you were able to resurrect the ghost.") );
				}
			}
			return;
		}
		
		
		if (pp->poisoned()>0)
		{
			if ( pp->isHuman() )
			{
				if (ph->skill(HEALING)<600 || ph->skill(ANATOMY)<600)
				{
					sysmessage(s, tr("You are not skilled enough to cure poison.") );
					sysmessage(s, tr("The poison in your target's system counters the bandage's effect.") );
				}
				else
				{
					int curechance = static_cast<int>((ph->baseSkill(HEALING)+ph->baseSkill(ANATOMY))*0.67);
					int curecheck=RandomNum(1,100);
					CheckSkill((ph),HEALING,600,1000);
					CheckSkill((ph),ANATOMY,600,1000);
					if(curechance<=curecheck)
					{
						pp->setPoisoned(0);
						sysmessage(s, tr("Because of your skill, you were able to counter the poison.") );
					}
					else
						sysmessage(s, tr("You fail to counter the poison") );
					pib->ReduceAmount(1);
				}
				return;
			}
			else
			{
		        if (ph->baseSkill(VETERINARY)<=600 || ph->baseSkill(ANIMALLORE)<=600)
				{
					sysmessage(s, tr("You are not skilled enough to cure poison."));
					sysmessage(s, tr("The poison in your target's system counters the bandage's effect."));
				}
				else
				{
					if (CheckSkill(ph,VETERINARY,600,1000) &&
						CheckSkill(ph,ANIMALLORE,600,1000))
					{
						pp->setPoisoned(0);
						sysmessage(s, tr("Because of your skill, you were able to counter the poison."));
					}
					else
					{
						sysmessage(s, tr("You fail to counter the poison"));
          				pib->ReduceAmount(1);
        			}
				}
			}
			return;
		}

		if(pp->hp() == pp->st() )
		{
			sysmessage(s, tr("That being is not damaged") );
			return;
		}
		
		if(pp->isHuman()) //Used on human
		{
			if (!CheckSkill((ph),HEALING,0,1000))
			{
				sysmessage(s, tr("You apply the bandages, but they barely help!") );
//				pp->hp++;
				tempshort = pp->hp();
				pp->setHp( ++tempshort );
			}
			else
			{
				int healmin = (((ph->skill(HEALING)/5)+(ph->skill(ANATOMY)/5))+3); //OSI's formula for min amount healed (Skyfire)
				int healmax = (((ph->skill(HEALING)/5)+(ph->skill(ANATOMY)/2))+10); //OSI's formula for max amount healed (Skyfire)
				int j=RandomNum(healmin,healmax);
				//int iMore1 = min(pp->st, j+pp->hp)-pp->hp;
				if(j>(pp->st() -pp->hp()))
					j=(pp->st() -pp->hp());
				if(pp->serial==ph->serial)
					tempeffect(ph, ph, 35, j, 0, 15, 0);//allow a delay
				else 
					tempeffect(ph, ph, 35, j, 0, 5, 0);// added suggestion by Ramases //-Fraz- must be checked
			}
		}
		else //Bandages used on a non-human
		{
			if (!CheckSkill((ph),VETERINARY,0,1000))
				sysmessage(s, tr("You are not skilled enough to heal that creature.") );
			else
			{
				int healmin = (((ph->skill(HEALING)/5)+(ph->skill(VETERINARY)/5))+3); //OSI's formula for min amount healed (Skyfire)
				int healmax = (((ph->skill(HEALING)/5)+(ph->skill(VETERINARY)/2))+10); //OSI's formula for max amount healed (Skyfire)
				int j = RandomNum(healmin, healmax);
				// khpae
				pp->setHp((pp->st() > (pp->hp() + j)) ? (pp->hp() + j) : pp->st());
				updatestats(pp, 0);
				sysmessage(s, tr("You apply the bandages and the creature looks a bit healthier.") );
			}
		}
		SetTimerSec(&ph->objectdelay,SrvParams->objectDelay() + SrvParams->bandageDelay());
		pib->ReduceAmount(1);
	}
}

void cSkills::ItemIdTarget(int s)
{
	P_CHAR pc_currchar = currchar[s];
	const P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (pi && !pi->isLockedDown()) // Ripper
	{
		if (!CheckSkill(pc_currchar, ITEMID, 0, 250))
		{
			sysmessage(s, tr("You can't quite tell what this item is...") );
		}
		else
		{
			if( pi->corpse() )
			{
				sysmessage(s, tr("You have to use your forensics evalutation skill to know more on this corpse.") );
				return;
			}

			// Identify Item by Antichrist // Changed by MagiusCHE)
			if (CheckSkill(pc_currchar, ITEMID, 250, 500))
				if (pi->name2() == "#") 
					pi->setName( pi->name2() );

			if( pi->name() == "#" ) 
				pi->getName(temp2);
			else 
				strcpy((char*)temp2, pi->name().ascii() );
			sysmessage(s, tr("You found that this item appears to be called: %1").arg(temp2) );

			// Show Creator by Magius(CHE)
			if (CheckSkill(pc_currchar, ITEMID, 250, 500))
			{
				if (pi->creator.size()>0)
				{
					if (pi->madewith>0) sprintf((char*)temp2, tr("It is %1 by %2").arg(skill[pi->madewith-1].madeword).arg(pi->creator.c_str()) ); // Magius(CHE)
					else if (pi->madewith<0) sprintf((char*)temp2, tr("It is %1 by %2").arg(skill[0-pi->madewith-1].madeword).arg(pi->creator.c_str()) ); // Magius(CHE)
					else sprintf((char*)temp2, tr("It is made by %1").arg(pi->creator.c_str()) ); // Magius(CHE)
				} else strcpy((char*)temp2, tr("You don't know its creator!") );
			} else strcpy((char*)temp2, tr("You can't know its creator!") );
			sysmessage(s, (char*)temp2);
			// End Show creator

			if (!CheckSkill(pc_currchar, ITEMID, 250, 500))
			{
				sysmessage(s, tr("You can't tell if it is magical or not.") );
			}
			else
			{
				if(pi->type()!=15)
				{
					sysmessage(s, tr("This item has no hidden magical properties.") );
				}
				else
				{
					if (!CheckSkill(pc_currchar, ITEMID, 500, 1000))
					{
						sysmessage(s, tr("This item is enchanted with a spell, but you cannot determine which") );
					}
					else
					{
						if (!CheckSkill(pc_currchar, ITEMID, 750, 1100))
						{
							sysmessage(s, tr("It is enchanted with the spell %1, but you cannot determine how many charges remain.").arg(spellname[(8*(pi->morex-1))+pi->morey-1]) );
						}
						else
						{
							sysmessage(s, tr("It is enchanted with the spell %1, and has %2 charges remaining.").arg(spellname[(8*(pi->morex-1))+pi->morey-1]).arg(pi->morez) );
						}
					}
				}
			}
		}
	}
}



//////////////////////////////////
// name:	cTinkerCombine
// history: by Duke, 3.06.2000
// Purpose: handles the combining of two (tinkering-)items after the user
//			dclicked one and then targeted the second item
//
//			It's a base class for three derived classes that handle the
//			old interfaces

class cTinkerCombine	// Combining tinkering items
{
protected:
	char* failtext;
	unsigned char badsnd1;
	unsigned char badsnd2;
	short itembits;
	short minskill;
	short id2;
public:
	cTinkerCombine(short badsnd=0x51, char *failmsg="You break one of the parts.")
	{
		badsnd1 = static_cast<unsigned char>(badsnd>>8);
		badsnd2 = static_cast<unsigned char>(badsnd&0x00FF);
		failtext=failmsg;
		itembits=0;
		minskill=100;
	}
	/*
	virtual void delonfail(SOCK s)		{deletematerial(s, itemmake[s].needs/2);}
	virtual void delonsuccess(SOCK s)	{deletematerial(s, itemmake[s].needs);}
	virtual void failure(SOCK s)		{delonfail(s);playbad(s);failmsg(s);}
	*/
	virtual void failmsg(int s)			{sysmessage(s,failtext);}
	virtual void playbad(int s)			{/*soundeffect(s,badsnd1,badsnd2);*/}
	virtual void playgood(int s)		{/*soundeffect(s,0,0x2A);*/}
	virtual void checkPartID(short id)	{;}
	virtual bool decide()				{return (itembits == 3) ? true : false;}
	virtual void createIt(int s)		{;}
	static cTinkerCombine* factory(short combinetype);
	virtual void DoIt(int s)
	{
		P_ITEM piClick = FindItemBySerial( calcserial(addid1[s], addid2[s], addid3[s], addid4[s]) );
		if( piClick == NULL )
		{
			sysmessage( s, "Original part no longer exists" );
			return;
		}
		
		const P_ITEM piTarg=FindItemBySerPtr(buffer[s]+7);
		if (piTarg==NULL || piTarg->isLockedDown())
		{
			sysmessage(s, tr("You can't combine these."));
			return;
		}
		
		// make sure both items are in the player's backpack
		P_ITEM pPack=Check4Pack(s);
		if (pPack==NULL) return;
		if ( piTarg->contserial!=pPack->serial
			|| piClick->contserial!=pPack->serial)
		{
			sysmessage(s,tr("You can't use material outside your backpack") );
			return;
		}
		
		// make sure the parts are of correct IDs AND they are different
		checkPartID( piClick->id() );
		checkPartID( piTarg->id() );
		if (!decide())
			sysmessage(s, tr("You can't combine these.") );
		else
		{
			P_CHAR pc_currchar = currchar[s];

			if (pc_currchar->skill(TINKERING)<minskill)
			{
				sysmessage(s, tr("You aren't skilled enough to even try that!") );
				return;
			}
			if( !Skills->CheckSkill( pc_currchar, TINKERING, minskill, 1000 ) )
			{
				failmsg(s);
				P_ITEM piLoser= rand()%2 ? piTarg : piClick;
				piLoser->ReduceAmount(1);
				playbad(s);
			}
			else
			{
				sysmessage(s, tr("You combined the parts") );
				piClick->ReduceAmount(1);
				piTarg->ReduceAmount(1);		// delete both parts 
				createIt(s);						// spawn the item
				playgood(s);
			}
		}
	}
};

class cTinkCreateAwG : public cTinkerCombine
{
public:
	cTinkCreateAwG() : cTinkerCombine() {}
	virtual void checkPartID(short id)
	{
		if (id==0x105B || id==0x105C) itembits |= 0x01; // axles
		if (id==0x1053 || id==0x1054) itembits |= 0x02; // gears
	}
	virtual void createIt(int s)
	{
		Items->SpawnItem(s, currchar[s],1,"an axle with gears",1,0x10,0x51,0,1,1);
	}
};

class cTinkCreateParts : public cTinkerCombine
{
public:
	cTinkCreateParts() : cTinkerCombine() {}
	virtual void checkPartID(short id)
	{
		if (id==0x1051 || id==0x1052) itembits |= 0x01; // axles with gears
		if (id==0x1055 || id==0x1056) itembits |= 0x02; // hinge
		if (id==0x105D || id==0x105E) itembits |= 0x04; // springs
	}
	virtual bool decide()
	{
		if (itembits == 3) {id2=0x59; minskill=300; return true;}	// sextant parts
		if (itembits == 5) {id2=0x4F; minskill=400; return true;}	// clock parts
		return false;
	}
	virtual void createIt(int s)
	{
		 char sztemp[15] ;
		if (id2 == 0x4F)
		  strcpy(sztemp,"clock parts") ;
		else
		  strcpy(sztemp,"sextant parts") ;
		char *pn = sztemp ;
		//#else
		//char *pn = (id2==0x4F) ? "clock parts" : "sextant parts";
		//#endif
		Items->SpawnItem(s, currchar[s],1,pn,1,0x10,id2,0,1,1);
	}
};

class cTinkCreateClock : public cTinkerCombine
{
public:
	cTinkCreateClock() : cTinkerCombine() {}
	virtual void checkPartID(short id)
	{
		if (id==0x104D || id==0x104E) itembits |= 0x01; // clock frame
		if (id==0x104F || id==0x1050) itembits |= 0x02; // clock parts
	}
	virtual bool decide()   {minskill=600; return cTinkerCombine::decide();}
	virtual void createIt(int s)
	{
		Items->SpawnItem(s,currchar[s],1,"clock",0,0x10,0x4B,0,1,1);
	}
};

#define cTC_AwG		11
#define cTC_Parts	22
#define cTC_Clock	33

cTinkerCombine* cTinkerCombine::factory(short combinetype)
{
	switch (combinetype)
	{
		case cTC_AwG:   return new cTinkCreateAwG();break;
		case cTC_Parts: return new cTinkCreateParts();break;	// clock/sextant parts
		case cTC_Clock: return new cTinkCreateClock();break;	// clock
		default:		return new cTinkerCombine();break;		// a generic handler
	}
	return NULL ;
}

void cSkills::TinkerAxel(int s)
{
	cTinkerCombine *ptc = cTinkerCombine::factory(cTC_AwG);
	ptc->DoIt(s);
}

void cSkills::TinkerAwg(int s)
{
	cTinkerCombine *ptc = cTinkerCombine::factory(cTC_Parts);
	ptc->DoIt(s);
}

void cSkills::TinkerClock(int s)
{
	cTinkerCombine *ptc = cTinkerCombine::factory(cTC_Clock);
	ptc->DoIt(s);
}

void cSkills::RepairTarget(UOXSOCKET s)
{ // Ripper..Repair items.
	P_CHAR pc = currchar[s];

	short smithing=pc->baseSkill(BLACKSMITHING);

	if (smithing < 500)
	{
		sysmessage(s, tr("* Your not skilled enough to repair items.*") );
		return;
	}

	P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	if (pi && !pi->isLockedDown())
	{
		if (!CheckInPack(s,pi)) 
			return;
		if( !pi->hp() )
		{
			sysmessage(s, tr(" That item cant be repaired.") );
			return;
		}
		if(!AnvilInRange(s))
		{
			sysmessage(s,tr(" Must be closer to the anvil.") );;
			return;
		}
		if( pi->hp() >= pi->maxhp() )
		{
			sysmessage(s,tr(" That item is at full strength.") );
			return;
		}
		short dmg=4;	// damage to maxhp
		if		((smithing>=900)) dmg=1;
		else if ((smithing>=700)) dmg=2;
		else if ((smithing>=500)) dmg=3;

		if(Skills->CheckSkill((pc),BLACKSMITHING, 0, 1000))
		{
			pi->setMaxhp( pi->maxhp() - dmg );
			pi->setHp( pi->maxhp() );
			sysmessage(s, tr(" * the item has been repaired.*") );
		}
		else
		{
			pi->setHp( pi->hp() - 2 );
			pi->setMaxhp( pi->maxhp() - 1 );
			sysmessage(s, tr(" * You fail to repair the item. *") );;
			sysmessage(s, tr(" * You weaken the item.*") );;
		}
	}
}

//////////////////////////////////
// name:	SmeltItemTarget
// history: by Ripper
//			revamped by Duke, 30.11.2001
// Purpose: Smelting items.
//			Only items crafted by a (player-)Blacksmith can be smelted

void cSkills::SmeltItemTarget(UOXSOCKET s)
{
	unsigned short int sk=MINING;

	P_CHAR pc = currchar[s];

	cItem* pi=FindItemBySerPtr(buffer[s]+7);
	if (!pi) return; 
	if (!CheckInPack(s,pi)) return;

	// number of ingots you get depends on the weight 
	// from what i can tell it's 1 + 1 per stone
	int a = 1 + pi->stones();

	if (pi->isLockedDown() || pi->rank!=30 || (pi->smelt() < 1 || pi->smelt() > 10 ))
	{
		sysmessage(s, tr("You cant smelt that item!") );
		return;
	}
	if(!ForgeInRange(s))
	{
		sysmessage(s,tr("Must be closer to the forge.") );
		return;
	}
	if(pc->skill(sk)< 300 || ( pc->skill(sk)<500 && pi->smelt()!=1 ) )
	{
		sysmessage(s, tr("You aren't skilled enough to even try that!") );
		return;
	}
	if (Skills->CheckSkill((pc),sk, 0, 1000))
	{
		char* Name = NULL;
		unsigned short Color = pi->color();
		switch( pi->smelt() )
		{
		case 1:	Name="#";			 	Color=0x0961;	break;
		case 2:	Name="silver ingot"; 	Color=0x0000;	break;
		case 3:	Name="golden ingot"; 					break;
		case 4:	Name="agapite ingot";					break;
		case 5:	Name="shadow ingot"; 					break;
		case 6:	Name="mythril ingot";					break;
		case 7:	Name="bronze ingot"; 					break;
		case 8:	Name="verite ingot"; 					break;
		case 9:	Name="merkite ingot";					break;
		case 10:Name="copper ingot"; 					break;
		default:
			LogError("switch reached default");
			return;
		}
		cItem* Ingot = Items->SpawnItem(pc, a, Name, 1, 0x1BF2, Color, 1);
		if (Ingot)
		{
			Ingot->setWeight( 20 );	// that is 0.2 stone
			Ingot->update();
			sysmessage(s, tr("you smelt the item and place some ingots in your pack.") );
			Items->DeleItem(pi);
		}
	}
}

