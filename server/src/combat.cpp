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

// Platform INcludes
#include "platform.h"

// Wolfpack Includes

#include "wolfpack.h"
#include "itemid.h"
#include "SndPkg.h"
#include "trigger.h"
#include "utilsys.h"
#include "guildstones.h"
#include "combat.h"
#include "srvparams.h"

#include "debug.h"
#undef  DBGFILE
#define DBGFILE "combat.cpp"

#define SWINGAT (unsigned int)1.75 * MY_CLOCKS_PER_SEC // changed from signed to unsigned, LB

int cCombat::GetBowType(P_CHAR pc)
{
	unsigned int ci=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(pc->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->layer==1 || pi->layer==2)
		{
			switch( pi->id() )
			{
			case 0x13B1:
			case 0x13B2:	return 1;	// bows
			case 0x0F4F:
			case 0x0F50:	return 2;	// crossbow
			case 0x13FC:
			case 0x13FD:	return 3;	// heavy xbow
			}
		}
	}
	return 0;
}

void cCombat::ItemCastSpell(UOXSOCKET s, P_CHAR pc, P_ITEM pi)//S=Socket c=Char # Target i=Item //Itemid
{
	if(!pi) return;
	P_CHAR pc_currchar = currchar[s];
	unsigned short int spellnum=((pi->morex*8)-8)+pi->morey;
	unsigned short int tempmana=pc_currchar->mn;//Save their mana so we can give it back.
	unsigned short int tempmage=pc_currchar->skill[MAGERY];//Easier than writing new functions for all these spells


	if(pi->type!=15) return;
	
	if(pi->morez<=0) return;
	
	switch(spellnum)
	{
	case 1:  Magic->ClumsySpell(pc_currchar,pc); break; //LB
	case 3:  Magic->FeebleMindSpell(pc_currchar,pc); break; //LB
	case 5:	 Magic->MagicArrow(pc_currchar,pc);		break; // lB
	case 8:  Magic->WeakenSpell(pc_currchar,pc); break; //LB
	case 18: Magic->FireballSpell(pc_currchar,pc); break; //LB
	case 22: Magic->HarmSpell(pc_currchar,pc); break; //LB
	case 27: Magic->CurseSpell(pc_currchar,pc); break; //LB
	case 30: Magic->LightningSpell(pc_currchar,pc); break; //lb
	case 37: Magic->MindBlastSpell(pc_currchar,pc); break;
	case 38: Magic->ParalyzeSpell(pc_currchar,pc);	break; //lb
	case 42: Magic->EnergyBoltSpell(pc_currchar,pc); break;
	case 43: Magic->ExplosionSpell(pc_currchar,pc); break;
	case 51: Magic->FlameStrikeSpell(pc_currchar,pc); break;
	default:
		staticeffect(pc_currchar, 0x37, 0x35, 0, 30);
		soundeffect2(pc_currchar, 0x005C);
		break;
	}
	pc_currchar->mn+=tempmana;
	pc_currchar->skill[MAGERY]=tempmage;
	if(pc_currchar->in<pc_currchar->mn) pc_currchar->mn=pc_currchar->in;//Shouldn't happen, but just in case;
	updatestats(pc_currchar, 1);
	
	pi->morez--;
	if(pi->morez==0)//JUST lost it's charge....
		sysmessage(s, tr("This item is out of charges."));
}

void CheckPoisoning(UOXSOCKET sd, P_CHAR pc_attacker, P_CHAR pc_defender)
{
	if ((pc_attacker->poison)&&(pc_defender->poisoned<pc_attacker->poison))
	{
		if (rand()%3==0) // || fightskill==FENCING)//0 1 or 2 //fencing always poisons :) - AntiChrist
		{
			pc_defender->poisoned=pc_attacker->poison;
			pc_defender->poisontime=uiCurrentTime+(MY_CLOCKS_PER_SEC*(40/pc_defender->poisoned)); // a lev.1 poison takes effect after 40 secs, a deadly pois.(lev.4) takes 40/4 secs - AntiChrist
			pc_defender->poisonwearofftime=pc_defender->poisontime+(MY_CLOCKS_PER_SEC*SrvParams->poisonTimer()); //wear off starts after poison takes effect - AntiChrist
			if (sd != -1) 
			{
				impowncreate(sd, pc_defender, 1); //Lb, sends the green bar ! 
				sysmessage(sd, tr("You have been poisoned!"));//AntiChrist 
			}
		}
	}
}

// This checks LineOfSight before calling CombatHit (Duke, 10.7.2001)
void cCombat::CombatHitCheckLoS(P_CHAR pAttacker, unsigned int currenttime)
{
	P_CHAR pDefender = FindCharBySerial(pAttacker->swingtarg);
	if ( pDefender == NULL ) return;
	UOXSOCKET s1=calcSocketFromChar(pAttacker);

	unsigned short los=line_of_sight(s1,pAttacker->pos, pDefender->pos,	WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING);

	CombatHit(pAttacker, pDefender, currenttime, los);
}

// CombatHit now expects that LineOfSight has been checked before (Duke, 10.7.2001)
void cCombat::CombatHit(P_CHAR pc_attacker, P_CHAR pc_deffender, unsigned int currenttime, short los)
{
	char hit;

	if (pc_deffender == NULL || pc_attacker == NULL)
		return;

	UOXSOCKET s1=calcSocketFromChar(pc_attacker), s2=calcSocketFromChar(pc_deffender);
	unsigned short fightskill = Skills->GetCombatSkill(pc_attacker), bowtype = Combat->GetBowType(pc_attacker),splitnum,splitcount,hitin;
	unsigned int basedamage;
	int damage; // removed from unsigne by Magius(CHE)
	signed int x;
	// Magius(CHE) - For armour absorbtion system
	int maxabs, maxnohabs, tmpj;


	P_ITEM pWeapon=pc_attacker->getWeapon();// get the weapon item only once

	if (pWeapon && !(rand()%50)	// a 2 percent chance (Duke, 07.11.2000)
		&& pWeapon->type !=9)	// but not for spellbooks (Duke, 09/10/00)
	{
		pWeapon->hp--; //Take off a hit point
		if(pWeapon->hp<=0)
		{
			sysmessage(s1, tr("Your weapon has been destroyed"));
			if ((pWeapon->trigon==1) && (pWeapon->layer>0))// -Frazurbluu- Trigger Type 2 is my new trigger type *-
			{
				Trig->triggerwitem(s1, pWeapon, 1); // trigger is fired when item destroyed
			}				
			Items->DeleItem(pWeapon);
		}
	}

	// End here - Magius(CHE) - For armour absorbtion system



	pc_attacker->swingtarg=-1;

	if((chardist(pc_attacker, pc_deffender)>1 && fightskill!=ARCHERY) || !los) return;
	if(pc_deffender->isNpc() && pc_deffender->isInvul()) return; // ripper


	hit=Skills->CheckSkill(pc_attacker, fightskill, 0, 1000);  // increase fighting skill for attacker and defender
	if (!hit)
	{
		if (pc_attacker->isPlayer())
			doMissedSoundEffect(pc_attacker);
		if ((fightskill==ARCHERY)&&(los))
		{
			if (rand()%3-1)//-1 0 or 1
			{
				short id=0x1BFB;	// bolts
				if (bowtype==1)
					id=0x0F3F;		// arrows

				P_ITEM pAmmo=Items->SpawnItem(pc_deffender,1,"#",1,id,0,0);
				if(pAmmo)
				{
					pAmmo->moveTo(pc_deffender->pos);
					pAmmo->priv=1;
					RefreshItem(pAmmo);
				}
			}
		}
	}
	else
	{
		if (!pc_deffender->isInvul())
		{
			if (pc_deffender->xid==0x0191) soundeffect2(pc_deffender,0x014b);
			else if (pc_deffender->xid==0x0190) soundeffect2(pc_deffender,0x0156);
			playmonstersound(pc_deffender, pc_deffender->id(), SND_DEFEND);
			//AntiChrist -- for poisoned weapons
			if((pWeapon) && (pWeapon->poisoned>0))
			{
				   pc_deffender->poisoned=pWeapon->poisoned;
				   pc_deffender->poisontime=uiCurrentTime+(MY_CLOCKS_PER_SEC*(40/pc_deffender->poisoned)); // a lev.1 poison takes effect after 40 secs, a deadly pois.(lev.4) takes 40/4 secs - AntiChrist
			       pc_deffender->poisonwearofftime=pc_deffender->poisontime+(MY_CLOCKS_PER_SEC*SrvParams->poisonTimer()); //wear off starts after poison takes effect - AntiChrist
			}
			CheckPoisoning(s2, pc_attacker, pc_deffender);	// attacker poisons defender
			CheckPoisoning(s1, pc_deffender, pc_attacker); // and vice versa

			if ((pc_deffender->effDex()>0)) pc_deffender->priv2&=0xFD;	// unfreeze

			if (fightskill!=WRESTLING && los)
				Combat->ItemSpell(pc_attacker, pc_deffender);
			
			if (fightskill!=WRESTLING || pc_attacker->isNpc())
				basedamage=Combat->CalcAtt(pc_attacker); // Calc base damage
			else
			{
				if ((pc_attacker->skill[WRESTLING]/100) > 0) 
				{
					if (pc_attacker->skill[WRESTLING]/100!=0)
						basedamage=rand()%(pc_attacker->skill[WRESTLING]/100);
					else basedamage=0;
				}
				else basedamage=rand()%2;
			}

			if((pc_attacker->isPlayer()) && (fightskill!=WRESTLING))
			{ 
				if (pWeapon->racehate != 0 && pc_deffender->race != 0)//-Fraz- Racehating combat
				{
					if (pWeapon->racehate==pc_deffender->race)
					{
						basedamage *=2;
							if(pc_deffender->isPlayer())
							{
								sysmessage(s2, tr("You scream in agony from being hit by the accursed metal!"));
								if (pc_deffender->xid == 0x0191) soundeffect2(pc_deffender,0x0152);
								else if (pc_deffender->xid==0x0190) soundeffect2(pc_deffender,0x0157);
							}// can add a possible effect below here for npc's being hit
					}
							
				}
			}
			Skills->CheckSkill(pc_attacker, TACTICS, 0, 1000);
			damage=(int)(basedamage*((pc_attacker->skill[TACTICS]+500.0)/1000.0)); // Add Tactical bonus
			damage=damage+(int)((basedamage*(pc_attacker->st/500.0))); // Add Strength bonus

			//Adds a BONUS DAMAGE for ANATOMY
			//Anatomy=100 -> Bonus +20% Damage - AntiChrist (11/10/99)
			if (Skills->CheckSkill(pc_attacker, ANATOMY, 0, 1000))
			{
				float multiplier=(((pc_attacker->skill[ANATOMY]*20)/1000.0f)/100.0f)+1;
				damage=(int)  (damage * multiplier);
			}
			//Adds a BONUS DEFENCE for TACTICS
			//Tactics=100 -> Bonus -20% Damage - AntiChrist (11/10/99)
			float multiplier=1-(((pc_deffender->skill[TACTICS]*20)/1000.0f)/100.0f);
			damage=(int)  (damage * multiplier);
			P_ITEM pShield=pc_deffender->getShield();
			if(pShield)
			{
				if (Skills->CheckSkill(pc_deffender, PARRYING, 0, 1000))// chance to block with shield
				{
					if (pShield->def!=0) damage-=rand()%(pShield->def);// damage absorbed by shield
					if(rand()%2) pShield->hp--; //Take off a hit point
					if(pShield->hp<=0)
					{
						sysmessage(s2, tr("Your shield has been destroyed"));
						if ((pShield->trigon==1) && (pShield->layer >0))// -Frazurbluu- Trigger Type 2 is my new trigger type *-
						{
							Trig->triggerwitem(s2, pShield, 1); // trigger is fired when item destroyed
						}	
						Items->DeleItem(pShield);
					}
				}
			}
			// Armor destruction and sped up by hitting with maces should go in here somewhere 
			// According to lacation of body hit Id imagine -Frazurbluu- **NEEDS ADDED**
			x=rand()%100;// determine area of body hit
			if (!SrvParams->combatHitMessage())
			{
				if (x<=44) x=1; // body
				else if (x<=58) x=2; // arms
				else if (x<=72) x=3; // head
				else if (x<=86) x=4; // legs
				else if (x<=93) x=5; // neck
				else x=6; // hands
			}
			else
			{
				temp[0] = '\0';
				hitin = rand()%2;
				if (x<=44)
				{
					x=1;       // body
					switch (hitin)
					{
					case 1:
						//later take into account dir facing attacker during battle
						if (damage < 10) strcpy(temp, "hits you in your Chest!");
						else if (damage >=10) strcpy(temp, "lands a terrible blow to your Chest!");
						break;
					case 2:
						if (damage < 10) strcpy(temp, "lands a blow to your Stomach!");
						else if (damage >=10) strcpy(temp, "knocks the wind out of you!");
						break;
					default:
						if (damage < 10) strcpy(temp, "hits you in your Ribs!");
						else if (damage >=10) strcpy(temp, "broken your Rib?!");
					}
				}
				else if (x<=58)
				{
					if (damage > 1)
					{
						x=2;  // arms
						switch (hitin)
						{
						case 1:	strcpy(temp, "hits you in Left Arm!");	break;
						case 2:	strcpy(temp, "hits you in Right Arm!");	break;
						default:strcpy(temp, "hits you in Right Arm!");
						}
					}
				}
				else if (x<=72)
				{
					x=3;  // head
					switch (hitin)
					{
					case 1:
						if (damage < 10) strcpy(temp, "hits you you straight in the Face!");
						else if (damage >=10) strcpy(temp, "lands a stunning blow to your Head!");
						break;
					case 2:
						if (damage < 10) strcpy(temp, "hits you to your Head!"); //kolours - (09/19/98)
						else if (damage >=10) strcpy(temp, "smashed a blow across your Face!");
						break;
					default:
						if (damage < 10) strcpy(temp, "hits you you square in the Jaw!");
						else if (damage >=10) strcpy(temp, "lands a terrible hit to your Temple!");
					}
				}
				else if (x<=86) 
				{
					x=4;  // legs
					switch (hitin)
					{
					case 1:	strcpy(temp, "hits you in Left Thigh!");	break;
					case 2:	strcpy(temp, "hits you in Right Thigh!");	break;
					default:strcpy(temp, "hits you in Groin!");
					}
				}
				else if (x<=93)
				{
					x=5;  // neck
					strcpy(temp, "hits you to your Throat!");
				}
				else
				{
					x=6;  // hands
					switch (hitin)
					{
					case 1:
						if (damage > 1) strcpy(temp, tr("hits you in Left Hand!").latin1());
						break;
					case 2:
						if (damage > 1) strcpy(temp, tr("hits you in Right Hand!").latin1());
						break;
					default:
						if (damage > 1) strcpy(temp, tr("hits you in Right Hand!").latin1());
					}
				}

				sprintf((char*)temp2,"%s %s",pc_attacker->name.c_str(), temp);
				if (pc_deffender->isPlayer() && s2!=-1) sysmessage(s2, (char*)temp2); //kolours -- hit display
			}
			x = CalcDef(pc_deffender,x);
			
			// Magius(CHE) - For armour absorbtion system
			maxabs = 20; //
			           // there are monsters with DEF >20, this makes them undefeatable
			maxnohabs=100;
			if (SrvParams->maxAbsorbtion() > 0)
			{
				maxabs = SrvParams->maxAbsorbtion();
			}
			if (SrvParams->maxnohabsorbtion() > 0)
			{
				maxnohabs = SrvParams->maxnohabsorbtion();
			}		
			if (!ishuman(pc_deffender)) maxabs=maxnohabs;
			tmpj=(int) (damage*x)/maxabs; // Absorbtion by Magius(CHE)
			damage -= tmpj;
			if (damage<0) damage=0;
			if (pc_deffender->isPlayer()) damage /= SrvParams->npcdamage(); // Rate damage against other players
			// End Armour Absorbtion by Magius(CHE) (See alse reactive armour spell damage)

			if (pc_attacker->isPlayer())//Zippy
				ItemCastSpell(s1, pc_deffender,pWeapon);

			//AntiChrist - 26/10/99
			//when hitten and damage >1, defender fails if casting a spell!
			if(damage>1 && pc_deffender->isPlayer())//only if damage>1 and against a player
			{
				if(pc_deffender->casting && currentSpellType[s2]==0 )
				{//if casting a normal spell (scroll: no concentration loosen)
					currentSpellType[s2]=0;
					pc_deffender->spell=-1;
					pc_deffender->casting=0;
					pc_deffender->spelltime=0;
					pc_deffender->priv2 &= 0xfd; // unfreeze, bugfix LB
					Magic->SpellFail(s2);
				}
			}

			if(damage>0)
			{
				if (pc_deffender->ra) // For reactive armor spell
				{
					// -Frazurbluu- RA may need a rewrite to be more OSI standard here
					// Its said 80% deflected 10% to attacker / 10% defender gotta check special effects
					int damage1;
					damage1=(int)( damage*(pc_deffender->skill[MAGERY]/2000.0));
					pc_deffender->hp -= damage-damage1;
					if (pc_deffender->isNpc()) damage1 = damage1 * SrvParams->npcdamage(); // by Magius(CHE)
					pc_attacker->hp -= damage1;  // Remove damage from attacker
					staticeffect(pc_deffender, 0x37, 0x4A, 0, 15);//RA effect - AntiChrist (9/99)
					if ((fightskill==MACEFIGHTING) && (IsSpecialMace(pWeapon->id())))// Stamina Loss -Fraz-
					{ 
						//pc_attacker->stm-=3+(rand()%4);
					}
					if ((fightskill==FENCING) && (IsFencing2H(pWeapon->id())))// Paralyzing -Fraz-
					{ 
						//will call the combat caused paralyzation **NEED TO DO**
					}
					if ((fightskill==SWORDSMANSHIP) && (IsAxe(pWeapon->id())))// Concussion Hit -Fraz-
					{ 
						//will call the combat caused concussion (loss of int for 30 secs) **NEED TO DO**
						// for now make it subtract mana
						//pc_attacker->mn-=(pc_attacker->mn/2);
					}
					updatestats(pc_attacker, 0);
				}
				else 
				{	// -Fraz- Now needs adjusted to happen on a skill percentage 
					pc_deffender->hp-=damage; // Remove damage from defender only apply special hits to non-npc's
					if ((fightskill==MACEFIGHTING) && (IsSpecialMace(pWeapon->id())) && (pc_deffender->isPlayer()))// Stamina Loss -Fraz-
					{ 
						pc_deffender->stm-=3+(rand()%3);
					}
					if ((fightskill==FENCING) && (IsFencing2H(pWeapon->id())) && (pc_deffender->isPlayer()))// Paralyzing -Fraz-
					{ 
						tempeffect(pc_attacker, pc_deffender, 44, 0, 0, 0);
						sysmessage(s1, tr("You delivered a paralyzing blow"));
											}
					if ((fightskill==SWORDSMANSHIP) && (IsAxe(pWeapon->id())) && (pc_deffender->isPlayer()))// Concussion Hit -Fraz-
					{ 
						tempeffect(pc_attacker, pc_deffender, 45, 0, 0, 0);
						//pc_attacker->mn-=(pc_attacker->mn/2); //-Fraz- temp use of this for concussion
					}
					updatestats((pc_deffender), 0);
				}
				// blood shred by blackwind
				if (damage>10)
				{
	               short id = 0x122c;	
	               if (damage>50) id=0x122a;
				   else if (damage>40) id=0x122d;
	               else if (damage>30) id=0x122e;
	               else if (damage>20) id=0x122b;
				   P_ITEM pBlood = Items->SpawnItem(pc_deffender, 1, "#", 0, id, 0, 0);
				   if (pBlood)
				   {
					  pBlood->moveTo(pc_deffender->pos);
					  pBlood->priv = 1;
					  pBlood->setGMMovable(); //Moveable by GM
					  RefreshItem(pBlood);
					  pBlood->decaytime = (SrvParams->decayTime()/2)*MY_CLOCKS_PER_SEC+uiCurrentTime;
				   }
				}

				/////////  For Splitting NPCs ///  McCleod
				if ((pc_deffender->split>0)&&(pc_deffender->hp>=1))
				{
					if (rand()%100<=pc_deffender->splitchnc)
					{
						if (pc_deffender->split==1) splitnum=1;
						else splitnum=rand()%pc_deffender->split+1;
						
						for (splitcount=0;splitcount<splitnum;splitcount++)
							Npcs->Split(pc_deffender);
					}
				}
				////////      End of spliting NPCs
			}
			if (pc_attacker->isPlayer())
				if((fightskill==ARCHERY && los)|| fightskill!=ARCHERY)
					doSoundEffect(pc_attacker, fightskill, pWeapon);
			if (pc_deffender->hp<0) pc_deffender->hp=0;
			updatestats((pc_deffender), 0);
			x = pc_deffender->id();
			if (x>=0x0190)
			{
				if (!pc_deffender->onhorse) 
					npcaction(pc_deffender, 0x14);
			}
		}
	}
}

static void NpcSpellAttack(P_CHAR pc_attacker, P_CHAR pc_defender, unsigned int currenttime, int los)
{
	if (pc_attacker->spatimer<=currenttime)
	{
		int spattacks = numbitsset( pc_attacker->spattack );

		if (!pc_defender->dead && chardist(pc_attacker, pc_defender) < SrvParams->attack_distance() && spattacks > 0 )
		{
			if (los)
			{																	
				int spattackbit=rand()%(spattacks) + 1;
				switch(whichbit(pc_attacker->spattack, spattackbit))
				{
				case 1:
					if (pc_attacker->mn>=4)
					{
						npcaction(pc_attacker, 6);
						Magic->MagicArrow(pc_attacker, pc_defender);
					}
					break; 
				case 2:
					if (pc_attacker->mn>=6)
					{
						npcaction(pc_attacker, 6);
						Magic->HarmSpell(pc_attacker, pc_defender);
					}
					break; //lb
				case 3:
					if (pc_attacker->mn>=4)
					{
						npcaction(pc_attacker, 6);
						Magic->ClumsySpell(pc_attacker, pc_defender);										
					}
					break; //LB
				case 4:
					if (pc_attacker->mn>=4)
					{
						npcaction(pc_attacker, 6);
						Magic->FeebleMindSpell(pc_attacker, pc_defender);
					}
					break; //LB
				case 5:
					if (pc_attacker->mn>=4)
					{
						npcaction(pc_attacker, 6);
						Magic->WeakenSpell(pc_attacker, pc_defender);
					}
					break; //LB
				case 6:
					if (pc_attacker->mn>=9)
					{
						npcaction(pc_attacker, 6);
						Magic->FireballSpell(pc_attacker, pc_defender);
					}
					break; //LB
				case 7:
					if (pc_attacker->mn>=11)
					{
						npcaction(pc_attacker, 6);
						Magic->CurseSpell(pc_attacker, pc_defender);
					}
					break; //LB
				case 8:
					if (pc_attacker->mn>=11)
					{
						npcaction(pc_attacker, 6);
						Magic->LightningSpell(pc_attacker, pc_defender);
					}
					break; //lb
				case 9:
					if (pc_attacker->mn>=14)
					{
						npcaction(pc_attacker, 6);
						Magic->ParalyzeSpell(pc_attacker, pc_defender);
					}
					break; //lb
				case 10:
					if (pc_attacker->mn>=14)
					{
						npcaction(pc_attacker, 6);
						Magic->MindBlastSpell(pc_attacker, pc_defender);
					}
					break;
				case 11:
					if (pc_attacker->mn>=20)
					{
						npcaction(pc_attacker, 6);
						Magic->EnergyBoltSpell(pc_attacker, pc_defender);
					}
					break;
				case 12:
					if (pc_attacker->mn>=20)
					{
						npcaction(pc_attacker, 6);
						Magic->ExplosionSpell(pc_attacker, pc_defender);
					}
					break;
				case 13:
					if (pc_attacker->mn>=40)
					{
						npcaction(pc_attacker, 6);
						Magic->FlameStrikeSpell(pc_attacker, pc_defender);
					}
					break;
				case 14:
					npcaction(pc_attacker, 6);
					Magic->PFireballTarget(pc_attacker, pc_defender, 10);
					break;
				case 15:
					npcaction(pc_attacker, 6);
					Magic->PFireballTarget(pc_attacker, pc_defender, 20);
					break;
				case 16:
					npcaction(pc_attacker, 6);
					Magic->PFireballTarget(pc_attacker, pc_defender, 40);
					break;
				}
			}			
		}
		pc_attacker->spatimer=currenttime+(pc_attacker->spadelay*MY_CLOCKS_PER_SEC); //LB bugkilling
	}
}				

// Formulas take from OSI's combat formulas
// attack speed should be determined here.
// attack speed = 15000 / ((DEX+100) * weapon speed)
static void SetWeaponTimeout(P_CHAR Attacker, P_ITEM Weapon)
{
	int x,j;
	
	if (Weapon) 
	{ 
		if (Weapon->spd==0) Weapon->spd=35;
		x = (15000*MY_CLOCKS_PER_SEC) / ((Attacker->effDex()+100) * Weapon->spd); //Calculate combat delay
	}
	else 
	{
		if(Attacker->skill[WRESTLING]>200) j = 35;
		else if(Attacker->skill[WRESTLING]>400) j = 40;
		else if(Attacker->skill[WRESTLING]>600) j = 45;
		else if(Attacker->skill[WRESTLING]>800) j = 50;
		else j = 30;
		x = (15000*MY_CLOCKS_PER_SEC) / ((Attacker->effDex()+100) * j);
	}
	Attacker->timeout=uiCurrentTime+x;
}

void cCombat::DoCombatAnimations(P_CHAR pc_attacker, P_CHAR pc_defender, int fightskill, int bowtype, int los)
{
	short id = pc_attacker->id();
	int cc,aa;
	if (id<0x0190)
	{
		aa=4+(rand()%3); // bugfix, LB, some creatures dont have animation #4
		cc=(creatures[id].who_am_i)&0x2; // anti blink bit set ?
		if (cc==2)
		{
			aa++;
			if (id==5) // eagles need special treatment
			{
				switch (RandomNum(0, 2))
				{
				case 0: aa=0x1;  break;
				case 1: aa=0x14; break;
				case 2: aa=0x4;  break;
				}
			}
		}
		npcaction(pc_attacker,aa); 
		playmonstersound(pc_attacker, pc_attacker->id(), SND_ATTACK);
	}
	else if (pc_attacker->onhorse)
	{
		CombatOnHorse(pc_attacker);	// determines weapon in hand and runs animation kolours (09/19/98)
	}
	else
	{
		CombatOnFoot(pc_attacker);	// determines weapon in hand and runs animation kolours (09/19/98)
	}

	if (fightskill==ARCHERY)
	{
		if (los)
		{
			if (bowtype==1)
			{
				delequan(pc_attacker, 0x0F3F, 1, NULL);
				movingeffect3(pc_attacker, pc_defender, 0x0F, 0x42, 0x08, 0x00, 0x00,0,0,0,0);
			}
			else
			{
				delequan(pc_attacker, 0x1BFB, 1, NULL);
				movingeffect3(pc_attacker, pc_defender, 0x1B, 0xFE, 0x08, 0x00, 0x00,0,0,0,0);
			}
		}
	}
}

void cCombat::DoCombat(P_CHAR pc_attacker, unsigned int currenttime)
{
	int x, bowtype=0;// spamanachecking, tmp;

	if ( pc_attacker == NULL )
		return;
	if (pc_attacker->free) return;
	P_ITEM pWeapon=pc_attacker->getWeapon();

	P_CHAR pc_defender = FindCharBySerial(pc_attacker->targ);
	/*if (pc_attacker->priv2&2) //The char is paralyzed 
	{ 
        sysmessage(calcSocketFromChar(pc_attacker), "You are frozen and cannot attack."); 
        return; 
	}*/
	if ((pc_defender == NULL) || (pc_defender->isPlayer() && !online(pc_defender) || pc_defender->isHidden()) && pc_attacker->war)
	{
		pc_attacker->war=false; // LB
		pc_attacker->timeout=0;
		pc_attacker->attacker = INVALID_SERIAL;
		pc_attacker->resetAttackFirst();
		return;
	}
	if (( pc_attacker->isNpc() || online(pc_attacker) ) && pc_defender != NULL )
	{			
		if (pc_defender == NULL) return;
		if (pc_defender->free) return;
		if (pc_attacker->dispz > (pc_defender->dispz +10)) return;//FRAZAI
		if (pc_attacker->dispz < (pc_defender->dispz -10)) return;//FRAZAI
		
		if ((pc_defender->isNpc() && pc_defender->npcaitype!=17) || (online(pc_defender) && !pc_defender->dead) ) // ripper		
		{
			if (chardist( pc_attacker, pc_defender ) > SrvParams->attack_distance())
			{
				if (pc_attacker->npcaitype==4 && pc_attacker->inGuardedArea()) // changed from 0x40 to 4, LB
				{
					pc_attacker->moveTo(pc_defender->pos);
					
					teleport(pc_attacker);
					soundeffect2(pc_attacker, 0x01FE); // crashfix, LB
					staticeffect(pc_attacker, 0x37, 0x2A, 0x09, 0x06);
					npctalkall(pc_attacker,"Halt, scoundrel!",1);
				}
				else 
				{ // else -> npcaityes != 4
					pc_attacker->targ = INVALID_SERIAL;
					pc_attacker->timeout=0;
					P_CHAR pc = FindCharBySerial(pc_attacker->attacker);
					if (pc != NULL)
					{
						pc->resetAttackFirst();
						pc->attacker = INVALID_SERIAL; // lb crashfix
					}
					pc_attacker->attacker=INVALID_SERIAL;
					pc_attacker->resetAttackFirst();
					if (pc_attacker->isNpc() && pc_attacker->npcaitype!=17 && !pc_attacker->dead && pc_attacker->war)
						npcToggleCombat(pc_attacker); // ripper
				}
			}
			else
			{
				if (pc_attacker->targ == INVALID_SERIAL)
				{
					npcsimpleattacktarget(pc_attacker, pc_defender);
					x=(((100-pc_attacker->effDex())*MY_CLOCKS_PER_SEC)/25)+(1*MY_CLOCKS_PER_SEC); //Yet another attempt.
					pc_attacker->timeout=currenttime+x;
					return;
				}
				if (Combat->TimerOk(pc_attacker))
				{
					int los = line_of_sight(-1, pc_attacker->pos, pc_defender->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING);
					UOXSOCKET s1 = calcSocketFromChar(pc_attacker);
					int fightskill=Skills->GetCombatSkill(pc_attacker);
					x=0;
					if (fightskill==ARCHERY)
					{
						if (los)
						{
							int arrowsquant;
							bowtype=Combat->GetBowType(pc_attacker);
							if (bowtype==1) 
								arrowsquant=getamount(pc_attacker, 0x0F3F);
							else 
								arrowsquant=getamount(pc_attacker, 0x1BFB);
							if (arrowsquant>0)
								x=1;
							//else
							//	sysmessage(s1, "You are out of ammunitions!"); //-Fraz- this message can cause problems removed
						}
					}
					if ( chardist( pc_attacker, pc_defender )<2 && fightskill!=ARCHERY ) x=1;
					if (x)
					{
						// - Do stamina maths - AntiChrist (6) -
						if(abs(SrvParams->attackstamina())>0 && !pc_attacker->isGM())
						{
							if((SrvParams->attackstamina()<0)&&(pc_attacker->stm<abs(SrvParams->attackstamina())))
							{
								sysmessage(s1, tr("You are too tired to attack."));
								SetWeaponTimeout(pc_attacker, pWeapon);
								return;
							}
							pc_attacker->stm += SrvParams->attackstamina();
							if (pc_attacker->stm>pc_attacker->effDex()) pc_attacker->stm=pc_attacker->effDex();
							if (pc_attacker->stm<0) pc_attacker->stm=0;
							updatestats((pc_attacker),2); //LB, crashfix, was currchar[a]
							// --------ATTACK STAMINA END ------
						}
						
						DoCombatAnimations( pc_attacker, pc_defender, fightskill, bowtype, los);

						if (((chardist( pc_attacker, pc_defender )<2)||(fightskill==ARCHERY))&&!(pc_attacker->npcaitype==4)) // changed from 0x40 to 4
                        {
							if (los)
							{
								npcsimpleattacktarget(pc_attacker, pc_defender);
							}
						}
						if (pc_attacker->timeout2 > uiCurrentTime) return; //check shotclock memory-BackStab
						if (fightskill==ARCHERY)
							Combat->CombatHit(pc_attacker, pc_defender, currenttime,los);
						else
							pc_attacker->swingtarg = pc_defender->serial;
					}

					SetWeaponTimeout(pc_attacker, pWeapon);
					pc_attacker->timeout2=pc_attacker->timeout; // set shotclock memory-BackStab
					if (!pc_defender->isInvul())
					{
						NpcSpellAttack(pc_attacker,pc_defender,currenttime,los);
					}
					if (fightskill!=ARCHERY)
					{
						Combat->CombatHit(pc_attacker, pc_defender, currenttime, los); // LB !!!
						//return; // Ripper	???? (Duke)
					}
				}
			}			
			if (pc_defender->hp<1)//Highlight //Repsys
			{
				if(pc_attacker->npcaitype==4 && pc_defender->isNpc())
				{
					npcaction(pc_defender, 0x15);
					
					PlayDeathSound(pc_defender);
					
					Npcs->DeleteChar(pc_defender);//Guards, don't give body
				}
				else
				{
					deathstuff(pc_defender);
				}
				//murder count \/
				
				if ((pc_attacker->isPlayer())&&(pc_defender->isPlayer()))//Player vs Player
				{
					if(pc_defender->isInnocent() && GuildCompare(pc_attacker, pc_defender )==0 )
					{
						++pc_attacker->kills;
						UOXSOCKET attacker_socket = calcSocketFromChar(pc_attacker);
						sysmessage(attacker_socket, tr("You have killed %1 innocent people.").arg(pc_attacker->kills));
						//clConsole.send("DEBUG %s's kills are now -> %i\n",pc_attacker->name,pc_attacker->kills);
						if (pc_attacker->kills==SrvParams->maxkills()+1)
							sysmessage(attacker_socket, tr("You are now a murderer!"));
					}
					
					if (SrvParams->pvpLog())
					{
						sprintf((char*)temp,"%s was killed by %s!\n",pc_defender->name.c_str(), pc_attacker->name.c_str());
						savelog((char*)temp,"PvP.log");
					}
					
				}
				npcToggleCombat(pc_attacker);
				return; // LB
			}
		}
	}
}

int cCombat::CalcAtt(P_CHAR pc_p) // Calculate total attack powerer
{
	int total = 0, j;
	
	if ( pc_p == NULL ) return 0;

	if(pc_p->isNpc()) 
	{
		//Compatibility for use with older scripts
		if((pc_p->lodamage==0)&&(pc_p->hidamage==0)) return(pc_p->att);
		
		//Code for new script commands
		if(pc_p->lodamage==pc_p->hidamage) //Avoids divide by zero error
		{
			return(pc_p->lodamage);
		}
		if(!(pc_p->hidamage<=2))
		{
			j=pc_p->hidamage-pc_p->lodamage; 
			if (j!=0) total=(rand()%(j))+pc_p->lodamage; else total=pc_p->lodamage;
		}
		
		if(total<1) total=1;
		return(total);
	}
	
	unsigned int ci = 0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(pc_p->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi->layer==1 || pi->layer==2)
		{
			if (pi->lodamage == pi->hidamage) total+=pi->lodamage;
			else if ((pi->lodamage>0)&&(pi->hidamage>0))
			{
				if (pi->hidamage-pi->lodamage!=0) total+=(rand()%(pi->hidamage-pi->lodamage))+(pi->lodamage); 
				else total+=pi->lodamage;
			}
			else if ((pi->lodamage==0)&&(pi->hidamage==0)) total+=pi->att;
		}
	}
	if (total<1) total=1;
	return total;
}

int cCombat::CalcDef(P_CHAR pc,int x) // Calculate total defense power 
{ 
	P_ITEM pj=NULL; 
	P_ITEM pShield = pc->getShield(); 
	unsigned int total=pc->def; 
	if (x==0) // -Fraz- added parrying skill bonuses
	{ 
		if (pShield)
			total+=(((pc->skill[PARRYING]*pShield->def)/200)+1); // Updated to OSI standars (Skyfire)
	} 		//Displayed AR = ((Parrying Skill * Base AR of Shield) ÷ 200) + 1
	if (pc->skill[PARRYING]==1000) 
		total+=5; // gm parry bonus. 
	if (ishuman(pc)) // Added by Magius(CHE) 
	{ 
		unsigned int ci=0; 
		P_ITEM pi; 
		vector<SERIAL> vecContainer = contsp.getData(pc->serial);
		for ( ci = 0; ci < vecContainer.size(); ci++)
		{
			pi = FindItemBySerial(vecContainer[ci]);
			if (pi != NULL && pi->layer>1 && pi->layer<25) 
			{ 
				//blackwinds new stuff 
				int armordef=0; 
				if (pi->def>0) 
				{ 
					float armorhpperc=((float)pi->hp/((float)pi->maxhp/100)); 
					armordef=(int)(((float)pi->def/100)*armorhpperc); 
				} 

				if (x==0) 
					total+=pi->def; 
				else 
				{ 
					switch (pi->layer) 
					{ 
					case 5: 
					case 13: 
					case 17: 
					case 20: 
					case 22: 
						if (x==1 && pi->def>total) 
						{ 
							total=pi->def; 
							pj=pi; 
						} 
						break; 
					case 19: 
						if (x==2 && pi->def>total) 
						{ 
							total=pi->def; 
							pj=pi; 
						} 
						break; 
					case 6: 
						if (x==3 && pi->def>total) 
						{ 
							total=pi->def; 
							pj=pi; 
						} 
						break; 
					case 3: 
					case 4: 
					case 12: 
					case 23: 
					case 24: 
						if (x==4 && pi->def>total) 
						{ 
							total=pi->def; 
							pj=pi; 
						} 
						break; 
					case 10: 
						if (x==5 && pi->def>total) 
						{ 
							total=pi->def; 
							pj=pi; 
						} 
						break; 
					case 7: 
						if (x==6 && pi->def>total) 
						{ 
							total=pi->def; 
							pj=pi; 
						} 
						break; 
					default: 
						break; 
					} 
				} 
			} 
		} 
	} else pj=NULL; 

	UOXSOCKET k=calcSocketFromChar(pc); 
	if (pj && k!=-1) 
	{ 
		//AntiChrist - don't damage hairs, beard and backpack 
		//important! this sometimes cause backpack destroy! 
		// 
		if(pj->layer!=0x0B && pj->layer!=0x10 && pj->layer!=0x15) // bugfix lB,was 0x15, 0x15,0x15 !! 
		{ 
			if((rand()%2)==0) 
				pj->hp--; //Take off a hit point 
			if(pj->hp<=0) 
			{ 
				sysmessage(k, tr("Your %1 has been destroyed").arg(pj->getName().c_str()));
				pc->removeItemBonus(pj);	// remove BONUS STATS given by equipped special items
				//-Frazurbluu-  need to have tactics bonus removed also
				if ((pj->trigon==1) && (pj->layer >0))// -Frazurbluu- Trigger Type 2 is my new trigger type *-
				{
					Trig->triggerwitem(k, pj, 1); // trigger is fired when item destroyed
				}
				Items->DeleItem(pj);		 
			}
			statwindow(k, currchar[k]);
		}
	}
	if (total < 2) total = 2;
	return total;
}

int cCombat::TimerOk(P_CHAR pc)
{
	int d=0;
	if (pc == NULL)
		return 0;
	//if (chars[c].timeout<uiCurrentTime) d=1;
	//---BackStab-- Tab-Machinegun stopper- meet both timeout requirements---------- 
    if(( pc->timeout < uiCurrentTime )&&(pc->timeout2 < uiCurrentTime)) d=1;
	if (overflow) d=1;
	return d;
}

//play animation for weapon in hand during combat on a horse //kolours - modularized (09/20/98)
void cCombat::CombatOnHorse(P_CHAR pc)
{
	if ( pc == NULL ) return;
	P_ITEM pWeapon=pc->getWeapon();

	// tweak for better fight animations if on Lama, ostards, kirin, seahorse, or ridgeback
	// LB (04-02-2002)
	

	P_ITEM pHorseItem=pc->GetItemOnLayer(0x19);	

	int base;
	switch(pHorseItem->id2)
	{
	    case 0xab: // llamas
		case 0xa6:
			base=0x2b;
			break;

		case 0xa3:  // ostards 
		case 0xa4:
		case 0xa5:
		case 0xac:
			base=0x35;
			break;

		case 0xad: // kirin
			base=0x3f;
			break;

		case 0xb3: // sea horse
			base=0x49;
			break;

		case 0xb8: //ridgeback
			base=0x53;
			break;

	    default:
			base = 0x17; // default (horse) base animation
			break;
	}
	if (pWeapon)
	{
		short id = pWeapon->id();

		if (IsBow(id))
		{
			npcaction(pc, 0x1B);
			return;
		}
		if (IsCrossbow(id) || IsHeavyCrossbow(id))
		{
			npcaction(pc, 0x1C);
			return;
		}

		if(  IsSword(id) || IsMaceType(id) ||
			(id ==0x0FB4 || id ==0x0FB5) || // sledge hammer
			IsFencing1H(id) )
		{
			npcaction(pc, 0x1A);
			return;
		}
		if ( IsAxe(id) || IsFencing2H(id) )
		{
			npcaction(pc, 0x1D); //2Handed
			return;
		}
	} // end of !=-1
	else
	{
		npcaction(pc, 0x1A); //fist fighting
		return;
	}
}

//play animation for weapon in hand for combat on foot //kolours - (09/20/98)
void cCombat::CombatOnFoot(P_CHAR pc)
{
	 if ( pc == NULL ) return;
	 P_ITEM pWeapon=pc->getWeapon();
	int m = rand()%4; //randomize the animation sequence

	if (pWeapon)
	{
		short id = pWeapon->id();

		if (IsBow(id))
		{
			npcaction(pc, 0x12); //bow
			return;
		}
		if (IsCrossbow(id) || IsHeavyCrossbow(id))
		{
			npcaction(pc, 0x13); //crossbow - regular
			return;
		}
		if (IsSword(id))
		{
			switch (m) //swords
			{
			case 0:		npcaction(pc, 0x0D);	return; //side swing
			case 1:		npcaction(pc, 0x0A);	return; //poke
			default:	npcaction(pc, 0x09);	return; //top-down swing
			}
		}
		if (IsMace1H(id))
		{
			switch (m) //maces
			{
			case 0:		npcaction(pc, 0x0D);	return;	//side swing
			default:	npcaction(pc, 0x09);	return; //top-down swing
			}
		}
		if (IsMace2H(id) || IsAxe(id))
		{
			switch (m)
			{
			case 0:		npcaction(pc, 0x0D);	return; //2H top-down
			case 1:		npcaction(pc, 0x0C);	return; //2H swing
			default:	npcaction(pc, 0x0D);	return; //2H top-down
			}
		}
		if (IsFencing1H(id))	// one handed fencing
		{
			switch (m) //fencing
			{
			case 0:		npcaction(pc, 0x09);	return; //top-down
			case 1:		npcaction(pc, 0x0D);	return; //side-swipe
			default:	npcaction(pc, 0x0A);	return; //default: poke
			}
		}
		if (IsFencing2H(id))	//pitchfork & spear
		{
			switch (m) //pitchfork
			{
			case 0:		npcaction(pc, 0x0D);	return; //top-down
			default:	npcaction(pc, 0x0E);	return; //default: 2-handed poke
			}
		}
	} // end of !=-1
	else
	{
		switch (m) //fist fighting
		{
		case 0:		npcaction(pc, 0x0A);	return; //fist straight-punch
		case 1:		npcaction(pc, 0x09);	return; //fist top-down
		default:	npcaction(pc, 0x1F);	return; //default: //fist over-head
		}
	}
}


//s: char#
void cCombat::SpawnGuard(P_CHAR pc_offender, P_CHAR pc_caller, int x, int y, signed char z)
{
	int t;
//	if (i < 0 || i >= cmem || s < 0 || s >= cmem)
//		return;
	
//	P_CHAR pc_offender = MAKE_CHARREF_LR(s);
//	P_CHAR pc_caller   = MAKE_CHARREF_LR(i);
	if ( pc_offender == NULL || pc_caller == NULL)
		return;
	
	if (!pc_caller->inGuardedArea())
		return;
	
	if (pc_offender->dead || pc_caller->dead)
		return; // AntiChrist
	
	if (SrvParams->guardsActive() && !pc_offender->isInvul())
	{
        t = region[pc_caller->region].guardnum[(rand()%10)];
		P_CHAR pc_guard = Npcs->AddNPCxyz(calcSocketFromChar(pc_offender), t, 0, x, y, z);
		if ( pc_guard == NULL ) return;
		
		pc_guard->npcaitype = 4; // CITY GUARD, LB, bugfix, was 0x40 -> not existing
		pc_guard->setAttackFirst();
		pc_guard->attacker = pc_offender->serial;
		pc_guard->targ = pc_offender->serial;
		pc_guard->npcWander = 2;  // set wander mode Tauriel
		npcToggleCombat(pc_guard);
		pc_guard->npcmovetime =(unsigned int)(getNormalizedTime() +(double)((NPCSPEED*MY_CLOCKS_PER_SEC)/5));
		pc_guard->summontimer =(getNormalizedTime() +(MY_CLOCKS_PER_SEC*25));    
		
		soundeffect2(pc_guard, 0x01FE);  // Tauriel 1-9-99 changed to stop crashing used to call soundeffect (expeted socket)
		staticeffect(pc_guard, 0x37, 0x2A, 0x09, 0x06);
		
		updatechar(pc_guard);
		switch (RandomNum(0,1))
		{
		case 0:		npctalkall(pc_guard, "Thou shalt regret thine actions, swine!", 1);	break;
		case 1:		npctalkall(pc_guard, "Death to all Evil!", 1);							break;
		}
	}
}

void cCombat::ItemSpell(cChar* Attacker, cChar* Defender)
{
	if (Attacker->npc)			// npcs can't use casting weapons right now (Duke)
		return;
	currentSpellType[calcSocketFromChar(Attacker)]=2;
	unsigned int ci;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(Attacker->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (((pi->layer==1 && pi->type!=9) || (pi->layer==2)))
		{
			if (pi->offspell && (pi->att||pi->hidamage) && pi->type == 15)
			{
				switch(pi->offspell)
				{
				case 1:	Magic->ClumsySpell(Attacker,Defender, false);			break;
				case 2:	Magic->FeebleMindSpell(Attacker, Defender, false);		break;
				case 3:	Magic->MagicArrow(Attacker,Defender, false);			break;
				case 4:	Magic->WeakenSpell(Attacker,Defender, false);			break;
				case 5:	Magic->HarmSpell(Attacker,Defender, false);				break;
				case 6:	Magic->FireballSpell(Attacker,Defender, false);			break;
				case 8:	Magic->CurseSpell(Attacker,Defender, false);			break;
				case 9:	Magic->LightningSpell(Attacker,Defender, false);		break;
				case 11:Magic->MindBlastSpell(Attacker,Defender, false);		break;
				case 12:Magic->ParalyzeSpell(Attacker,Defender, false);			break;
				case 14:Magic->ExplosionSpell(Attacker,Defender, false);		break;
				case 15:Magic->FlameStrikeSpell(Attacker, Defender, false);		break;
				default:
					LogErrorVar("invalid offspell value %i",pi->offspell);
				}
				pi->morez--;
				if (pi->morez == 0)
				{
					pi->type = pi->type2;
					pi->morex = 0;
					pi->morey = 0;
					pi->offspell = 0;
				}
			}
			return;
		}
    }
}

//AntiChrist - do the sound effect ( only if HITTEN! )
void cCombat::doSoundEffect(P_CHAR pc, int fightskill, P_ITEM pWeapon)
{
	bool heavy=false;
	int a=rand()%4;

	//check if a heavy weapon
	if (pWeapon && IsAxe(pWeapon->id()))
		heavy=true;

	if(heavy)
	{
		if (a==0 || a==1) soundeffect2(pc, 0x0236);
		else soundeffect2(pc, 0x0237);
		return;
	}		

	switch(fightskill)
	{
	case ARCHERY:
		soundeffect2(pc, 0x0234);
		break;
	case FENCING:
	case SWORDSMANSHIP:
		if (a==0 || a==1) 
			soundeffect2(pc, 0x023B);
		else 
			soundeffect2(pc, 0x023C);
		break;
	case MACEFIGHTING:
		if (a==0 || a==1) 
			soundeffect2(pc, 0x0232);
		else if (a==2) 
			soundeffect2(pc, 0x0139);
		else 
			soundeffect2(pc, 0x0233);
		break;
	case WRESTLING:
		if (a==0) soundeffect2(pc, 0x0135);
		else if (a==1) soundeffect2(pc, 0x0137);
		else if (a==2) soundeffect2(pc, 0x013D);
		else soundeffect2(pc, 0x013B);
		break;
	default:
		soundeffect2(pc, 0x013D);
	}
}


//AntiChrist - do the "MISSED" sound effect
void cCombat::doMissedSoundEffect(P_CHAR pc)
{
	int a=rand()%3;

	if (a==0) soundeffect2(pc, 0x0238);
	else if (a==1) soundeffect2(pc, 0x0239);
	else soundeffect2(pc, 0x023A);
}
