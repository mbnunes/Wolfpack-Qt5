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
#include "mapobjects.h"
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
		P_ITEM pPack = currchar[s]->getBackpack();
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

