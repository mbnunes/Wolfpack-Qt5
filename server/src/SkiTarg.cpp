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
#include "maps.h"
#include "network.h"
#include "wpdefmanager.h"
#include "makemenus.h"
#include "skills.h"
#include "network/uosocket.h"

#undef DBGFILE
#define DBGFILE "skiTarg.cpp"
#include "debug.h"

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
//			sysmessage(s, tr("You cut some cloth into bandages, and put it in your backpack") );
			P_ITEM pi_c = Items->SpawnItem(s,pc_currchar,amt,"#",0,0x0E,0x21,col1,1,1);
			if(pi_c == NULL) return;
			// need to set amount and weight and pileable, note: cannot set pilable while spawning item -Fraz-
			pi_c->setWeight( 10 );
			pi_c->setAtt(9);
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

/*
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
		if (!SrvParams->bandageInCombat() && (pp->war() || ph->war()))
		{
			P_CHAR pc_attacker = FindCharBySerial(ph->attacker()); // Ripper...cant heal while in a fight
			if ( (pc_attacker != NULL) && pc_attacker->war())
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
		
		if (pp->dead())
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
		ph->setObjectDelay( SetTimerSec(ph->objectdelay,SrvParams->objectDelay() + SrvParams->bandageDelay()) );
		pib->ReduceAmount(1);
	}
}
*/

/*
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
*/

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
		if ( piTarg->container() != pPack
			|| piClick->container() != pPack )
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
			if( !pc_currchar->checkSkill( TINKERING, minskill, 1000 ) )
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

