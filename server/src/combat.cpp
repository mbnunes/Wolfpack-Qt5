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
#include "utilsys.h"
#include "guildstones.h"
#include "mapobjects.h"
#include "combat.h"
#include "srvparams.h"
#include "skills.h"
#include "classes.h"
#include "network.h"
#include "network/uosocket.h"
#include "territories.h"

#include "debug.h"
#undef  DBGFILE
#define DBGFILE "combat.cpp"

#define SWINGAT (unsigned int)1.75 * MY_CLOCKS_PER_SEC // changed from signed to unsigned, LB

int cCombat::GetBowType(P_CHAR pc)
{
	unsigned int ci=0;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(pc->serial);
	for ( ; ci < vecContainer.size(); ++ci)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if (pi && ( pi->layer() == 1 || pi->layer() == 2 ) )
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

void cCombat::ItemCastSpell( P_CHAR pAttacker, P_CHAR pDefender, P_ITEM pItem )
{
	if( !pAttacker || !pDefender || !pItem )
		return;

	UINT16 spellnum = ( ( pItem->morex * 8 ) - 8 ) + pItem->morey;
	UINT16 tempmana = pAttacker->mn(); //Save their mana so we can give it back.
	UINT16 tempmage = pAttacker->skill( MAGERY ); //Easier than writing new functions for all these spells

	if( pItem->type() != 15 || pItem->morez <= 0 )
		return;
	
	switch( spellnum )
	{
	case 1:  Magic->ClumsySpell( pAttacker, pDefender );		break;
	case 3:  Magic->FeebleMindSpell( pAttacker, pDefender );	break;
	case 5:	 Magic->MagicArrow( pAttacker, pDefender );			break;
	case 8:  Magic->WeakenSpell( pAttacker, pDefender );		break;
	case 18: Magic->FireballSpell( pAttacker, pDefender );		break;
	case 22: Magic->HarmSpell( pAttacker, pDefender );			break;
	case 27: Magic->CurseSpell( pAttacker, pDefender );			break;
	case 30: Magic->LightningSpell( pAttacker, pDefender );		break;
	case 37: Magic->MindBlastSpell( pAttacker, pDefender );		break;
	case 38: Magic->ParalyzeSpell( pAttacker, pDefender );		break;
	case 42: Magic->EnergyBoltSpell( pAttacker, pDefender );	break;
	case 43: Magic->ExplosionSpell( pAttacker, pDefender );		break;
	case 51: Magic->FlameStrikeSpell( pAttacker, pDefender );	break;
	default:
		staticeffect( pAttacker, 0x37, 0x35, 0, 30 );
		pAttacker->soundEffect( 0x5C );
		break;
	}

	pAttacker->setMn(pAttacker->mn() + tempmana);
	pAttacker->setSkill( MAGERY, tempmage );
	
	if( pAttacker->in() < pAttacker->mn() )
		pAttacker->setMn( pAttacker->in() );
	
	pItem->morez--;

	if( pAttacker->socket() )
	{
		pAttacker->socket()->updateMana();

		if( pItem->morez == 0 )
			pAttacker->socket()->sysMessage( tr( "This item is out of charges." ) );
	}
}

void CheckPoisoning( cUOSocket *socket, P_CHAR pc_attacker, P_CHAR pc_defender)
{
	if( pc_attacker->poison() && ( pc_defender->poisoned() < pc_attacker->poison() ) )
	{
		if( rand()%3 == 0 ) // || fightskill==FENCING)//0 1 or 2 //fencing always poisons :) - AntiChrist
		{
			pc_defender->setPoisoned(pc_attacker->poison());
			pc_defender->setPoisontime(uiCurrentTime+(MY_CLOCKS_PER_SEC*(40/pc_defender->poisoned()))); // a lev.1 poison takes effect after 40 secs, a deadly pois.(lev.4) takes 40/4 secs - AntiChrist
			pc_defender->setPoisonwearofftime(pc_defender->poisontime()+(MY_CLOCKS_PER_SEC*SrvParams->poisonTimer())); //wear off starts after poison takes effect - AntiChrist
			pc_defender->resend( false );

			if( socket )
				socket->sysMessage( tr("You have been poisoned!" ) );
		}
	}
}

// This checks LineOfSight before calling CombatHit
void cCombat::CombatHitCheckLoS( P_CHAR pAttacker, UINT32 currenttime )
{
	P_CHAR pDefender = FindCharBySerial( pAttacker->swingtarg() );
	if( !pDefender ) 
		return;

	UINT16 los = lineOfSight( pAttacker->pos, pDefender->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING );

	CombatHit( pAttacker, pDefender, currenttime, los );
}

// CombatHit now expects that LineOfSight has been checked before (Duke, 10.7.2001)
void cCombat::CombatHit(P_CHAR pc_attacker, P_CHAR pc_defender, unsigned int currenttime, short los )
{
	if( !pc_defender || !pc_attacker )
		return;

	unsigned short fightskill = Skills->GetCombatSkill(pc_attacker), bowtype = Combat->GetBowType(pc_attacker),splitnum,splitcount,hitin;
	INT32 basedamage = 0;
	INT32 damage = 0;
	signed int x;
	// Magius(CHE) - For armour absorbtion system
	int maxabs, maxnohabs, tmpj;


	P_ITEM pWeapon = pc_attacker->getWeapon(); // get the weapon item only once

	if( pWeapon && !( rand() % 50 )	// a 2 percent chance
		&& pWeapon->type() !=9)	// but not for spellbooks
	{
		pWeapon->setHp( pWeapon->hp() - 1 ); //Take off a hit point
		if( pWeapon->hp() <= 0 )
		{
			pc_attacker->message( tr( "Your weapon has been destroyed" ) );
			Items->DeleItem( pWeapon );
		}
	}

	pc_attacker->setSwingTarg( -1 );
	if( ( chardist( pc_attacker, pc_defender ) > 1 && fightskill != ARCHERY ) || !los )
		return;

	if( pc_defender->isInvul() ) 
		return;

	bool hit = Skills->CheckSkill( pc_attacker, fightskill, 0, 1000 );  // This is the mandatory skillcheck
	
	// We missed our target
	if( !hit )
	{
		// Display a message to both players that the 
		// swing dind't hit
		// NOTE: There should be a random chance that this
		// message appears *or* a flag to set
		if( pc_attacker->socket() )
			pc_attacker->socket()->sysMessage( tr( "You miss %1" ).arg( pc_defender->name.c_str() ) );

		if( pc_defender->socket() )
			pc_defender->socket()->sysMessage( tr( "%1 misses you" ).arg( pc_attacker->name.c_str() ) );

		if( pc_attacker->isPlayer() )
			doMissedSoundEffect( pc_attacker );

		if( ( fightskill == ARCHERY ) && ( los ) )
		{
			if( RandomNum( 1, 3 ) == 3 ) // 1/3 chance
			{
				short id = 0x1BFB;	// bolts
				if( bowtype == 1 )
					id=0x0F3F;		// arrows

				P_ITEM pAmmo = Items->SpawnItem( pc_defender, 1, "#", 1, id, 0, 0);
				if( pAmmo )
				{
					pAmmo->moveTo( pc_defender->pos );
					pAmmo->priv = 1;
					pAmmo->update();
				}
			}
		}

		return;
	}

	// ==== HIT SOUNDS
	if( pc_defender->id() == 0x191 )
		pc_defender->soundEffect( 0x14b );
	else if( pc_defender->id() == 0x190 ) 
		pc_defender->soundEffect( 0x156 );
	else	
		playmonstersound( pc_defender, pc_defender->id(), SND_DEFEND );
			
		// ==== POISONING
		if( pWeapon && ( pWeapon->poisoned > 0 ) )
		{
			   pc_defender->setPoisoned( pWeapon->poisoned );
			   pc_defender->setPoisontime( uiCurrentTime + ( MY_CLOCKS_PER_SEC * ( 40 / pc_defender->poisoned() ) ) ); // a lev.1 poison takes effect after 40 secs, a deadly pois.(lev.4) takes 40/4 secs
		       pc_defender->setPoisonwearofftime( pc_defender->poisontime() + ( MY_CLOCKS_PER_SEC * SrvParams->poisonTimer() ) ); //wear off starts after poison takes effect
		}

		CheckPoisoning( pc_defender->socket(), pc_attacker, pc_defender ); // attacker poisons defender
		CheckPoisoning( pc_attacker->socket(), pc_defender, pc_attacker ); // and vice versa

		// ==== UNFREEZE
		if( ( pc_defender->effDex() > 0 ) )
//			pc_defender->priv2 &= 0xFD;
			pc_defender->setPriv2( pc_defender->priv2() & 0xFD );
			

		// ==== ENCHANTED ITEMS
		if( ( fightskill != WRESTLING ) && los )
				Combat->ItemSpell( pc_attacker, pc_defender );
			
		// Calc base damage
		if( ( fightskill != WRESTLING ) || pc_attacker->isNpc() )
			basedamage = Combat->CalcAtt( pc_attacker ); 
		else if( ( pc_attacker->skill( WRESTLING ) / 100 ) > 0 )
		{
			basedamage = rand() % ( pc_attacker->skill( WRESTLING ) / 100 );
		}
		else 
			basedamage = rand() % 2;
	
		if( pc_attacker->isPlayer() && ( fightskill != WRESTLING ) )
		{ 
			if( pWeapon->racehate() != 0 && pc_defender->race() != 0 ) //-Fraz- Racehating combat
			{
				if (pWeapon->racehate() == pc_defender->race() )
				{
					basedamage *= 2;
					if( pc_defender->isPlayer() )
					{
						if( pc_defender->socket() )
							pc_defender->socket()->sysMessage( tr( "You scream in agony from being hit by the accursed metal!" ) );

						if( pc_defender->id() == 0x191 )
							pc_defender->soundEffect( 0x152 );
						else if( pc_defender->id() == 0x190 )
							pc_defender->soundEffect( 0x157 );

					}	// can add a possible effect below here for npc's being hit
				}		
			}
		}

		Skills->CheckSkill( pc_attacker, TACTICS, 0, 1000 );
		damage=(int)(basedamage*((pc_attacker->skill(TACTICS)+500.0)/1000.0)); // Add Tactical bonus
		damage=damage+(int)((basedamage*(pc_attacker->st() /500.0))); // Add Strength bonus

			//===== BONUS DAMAGE FOR ANATOMY
			//Anatomy = 100 -> Bonus +20% Damage
			if (Skills->CheckSkill(pc_attacker, ANATOMY, 0, 1000))
			{
				float multiplier=(((pc_attacker->skill(ANATOMY)*20)/1000.0f)/100.0f)+1;
				damage=(int)  (damage * multiplier);
			}

			//===== BONUS DEFENSE FOR TACTICS
			// Tactics = 100 -> Bonus -20% Damage
			float multiplier=1-(((pc_defender->skill(TACTICS)*20)/1000.0f)/100.0f);
			damage = (int)(damage * multiplier);

			//===== BONUS DEFENSE FOR PARRY+SHIELD
			P_ITEM pShield = pc_defender->getShield();
			if( pShield )
			{
				if (Skills->CheckSkill(pc_defender, PARRYING, 0, 1000)) // chance to block with shield
				{
					if( pShield->def != 0 )
						damage -= rand() % pShield->def; // damage absorbed by shield

					if( rand() % 2 ) 
						pShield->setHp( pShield->hp() - 1 ); //Take off a hit point

					if( pShield->hp() <= 0 )
					{
						if( pc_defender->socket() )
							pc_defender->socket()->sysMessage( tr( "Your shield has been destroyed" ) );

						Items->DeleItem(pShield);
					}
				}
			}

			// Armor destruction and sped up by hitting with maces should go in here somewhere 
			// According to lacation of body hit Id imagine -Frazurbluu- **NEEDS ADDED**
			x=rand()%100;// determine area of body hit
			QString cMessage;

			if( !SrvParams->combatHitMessage() )
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
				hitin = rand()%2;
				if (x<=44)
				{
					x=1;       // body
					switch (hitin)
					{
					case 1:
						//later take into account dir facing attacker during battle
						if (damage < 10)		cMessage = "hits you in your Chest!";
						else if (damage >=10)	cMessage = "lands a terrible blow to your Chest!";
						break;
					case 2:
						if (damage < 10)		cMessage = "lands a blow to your Stomach!";
						else if (damage >=10)	cMessage = "knocks the wind out of you!";
						break;
					default:
						if (damage < 10)		cMessage = "hits you in your Ribs!";
						else if (damage >=10)	cMessage = "has broken your Rib?!";
					}
				}
				else if (x<=58)
				{
					if (damage > 1)
					{
						x=2;  // arms
						switch (hitin)
						{
						case 1:		cMessage = tr( "hits you in Left Arm!" );	break;
						case 2:		cMessage = tr( "hits you in Right Arm!" );	break;
						default:	cMessage = tr( "hits you in Right Arm!" );
						}
					}
				}
				else if (x<=72)
				{
					x=3;  // head
					switch (hitin)
					{
					case 1:
						if (damage < 10)		cMessage = "hits you you straight in the Face!";
						else if (damage >=10)	cMessage = "lands a stunning blow to your Head!";
						break;
					case 2:
						if (damage < 10)		cMessage = "hits you to your Head!";
						else if (damage >=10)	cMessage = "smashed a blow across your Face!";
						break;
					default:
						if (damage < 10)		cMessage = "hits you you square in the Jaw!";
						else if (damage >=10)	cMessage = "lands a terrible hit to your Temple!";
					}
				}
				else if (x<=86) 
				{
					x=4;  // legs
					switch (hitin)
					{
					case 1:		cMessage = "hits you in Left Thigh!";	break;
					case 2:		cMessage = "hits you in Right Thigh!";	break;
					default:	cMessage = "hits you in Groin!";
					}
				}
				else if (x<=93)
				{
					x=5;  // neck
					cMessage = "hits you to your Throat!";
				}
				else
				{
					x=6;  // hands
					switch (hitin)
					{
					case 1:
						if (damage > 1) cMessage = tr("hits you in Left Hand!");
						break;
					case 2:
						if (damage > 1) cMessage = tr("hits you in Right Hand!");
						break;
					default:
						if (damage > 1) cMessage = tr("hits you in Right Hand!");
					}
				}
			}

			x = CalcDef( pc_defender, x );
			
			// Magius(CHE) - For armour absorbtion system
			maxabs = 20; //
						 // there are monsters with DEF >20, this makes them undefeatable
			maxnohabs = 100;
			
			if( SrvParams->maxAbsorbtion() > 0 )
			{
				maxabs = SrvParams->maxAbsorbtion();
			}

			if (SrvParams->maxnohabsorbtion() > 0)
			{
				maxnohabs = SrvParams->maxnohabsorbtion();
			}		

			if( !ishuman( pc_defender ) ) 
				maxabs = maxnohabs;

			tmpj = (int)( damage * x ) / maxabs; // Absorbtion by Magius(CHE)
			damage -= tmpj;
			if( damage < 0 )
				damage=0;

			if( pc_defender->isPlayer() )
				damage /= SrvParams->npcdamage(); // Rate damage against other players
			
			// End Armour Absorbtion by Magius(CHE) (See alse reactive armour spell damage)

			if( pc_attacker->isPlayer() )
				ItemCastSpell( pc_attacker, pc_defender, pWeapon );

			// when hitten and damage >1, defender fails if casting a spell!
			// Thats not really good, better make a check versus int+magic
			if( damage > 1 && pc_defender->isPlayer() )//only if damage>1 and against a player
			{
				// TODO: Implement spell apruption
				/*if(pc_defender->casting() && currentSpellType[s2]==0 )
				{//if casting a normal spell (scroll: no concentration loosen)
					Magic->SpellFail(s2);
					currentSpellType[s2]=0;
					pc_defender->setSpell(-1);
					pc_defender->setCasting(false);
					pc_defender->setSpelltime(0);
					pc_defender->priv2 &= 0xfd; // unfreeze, bugfix LB
				}*/
			}

			if( damage > 0 )
			{
				// Now we know that damage has actually been dealt
				// So we may now display the where-hit message
				if( !cMessage.isEmpty() )
				{
					if( pc_defender->socket() )
						pc_defender->socket()->sysMessage( QString( "%1 %2" ).arg( pc_attacker->name.c_str() ).arg( cMessage ) );
				}

				//===== REACTIVE ARMOR
				if( pc_defender->ra() )
				{
					// RA may need a rewrite to be more OSI standard here
					// Its said 80% deflected 10% to attacker / 10% defender gotta check special effects
					int damage1;
					damage1=(int)( damage*(pc_defender->skill(MAGERY)/2000.0));
//					pc_defender->hp -= damage - damage1;
					pc_defender->setHp( pc_defender->hp() - damage - damage1 );
					if (pc_defender->isNpc()) damage1 = damage1 * SrvParams->npcdamage();
//					pc_attacker->hp -= damage1;  // Remove damage from attacker
					pc_attacker->setHp( pc_attacker->hp() - damage1 );
					staticeffect(pc_defender, 0x37, 0x4A, 0, 15); //RA effect
					
					if ((fightskill==MACEFIGHTING) && (IsSpecialMace(pWeapon->id()))) // Stamina Loss
					{ 
						//pc_attacker->stm-=3+(rand()%4);
					}

					if( ( fightskill == FENCING ) && ( IsFencing2H( pWeapon->id() ) ) ) // Paralyzing
					{ 
						//will call the combat caused paralyzation **NEED TO DO**
					}

					if ((fightskill==SWORDSMANSHIP) && (IsAxe(pWeapon->id())))// Concussion Hit
					{ 
						//will call the combat caused concussion (loss of int for 30 secs) **NEED TO DO**
						// for now make it subtract mana
						//pc_attacker->mn-=(pc_attacker->mn/2);
					}

					pc_attacker->updateHealth();
				}
				else 
				{	
					//===== DEAL DAMAGE (!)
//					pc_defender->hp -= damage; // Remove damage from defender only apply special hits to non-npc's
					pc_defender->setHp( pc_defender->hp() - damage );
					if ((fightskill==MACEFIGHTING) && (IsSpecialMace(pWeapon->id())) && (pc_defender->isPlayer()))// Stamina Loss -Fraz-
					{ 
//						pc_defender->stm-=3+(rand()%3);
						pc_defender->setStm( pc_defender->stm() - 3+(rand()%3) );
					}

					if ((fightskill==FENCING) && (IsFencing2H(pWeapon->id())) && (pc_defender->isPlayer()))// Paralyzing
					{ 
						tempeffect(pc_attacker, pc_defender, 44, 0, 0, 0);
						pc_attacker->message( tr( "You delivered a paralyzing blow" ) );
					}

					if( ( fightskill == SWORDSMANSHIP ) && ( IsAxe( pWeapon->id() ) ) && ( pc_defender->isPlayer() ) )// Concussion Hit
					{ 
						tempeffect(pc_attacker, pc_defender, 45, 0, 0, 0);
					}
				}

				//===== BLOOD ( This *needs* a blood-color check)
				if( RandomNum( 1, 10 ) == 1 ) // 10% chance
				{
	               UINT16 id = 0x122c;

	               if( damage > 50 )
					   id = 0x122a;

				   else if( damage > 40 )
					   id = 0x122d;

	               else if( damage > 30 )
					   id = 0x122e;

	               else if( damage > 20 )
					   id = 0x122b;

				   else if( damage < 10 )
					   id = 0x1645;

				   P_ITEM pBlood = Items->SpawnItem( pc_defender, 1, "#", 0, id, 0, 0 );
				   
				   if( pBlood )
				   {
					  pBlood->moveTo( pc_defender->pos );
					  pBlood->priv = 1;
					  pBlood->setGMMovable(); // Moveable by GM
					  pBlood->update();
					  pBlood->decaytime = ( 8 * MY_CLOCKS_PER_SEC ) + uiCurrentTime; // Will stay 8 secs
				   }
				}

				//===== SPLITTING NPCS
				if ((pc_defender->split()>0)&&(pc_defender->hp()>=1))
				{
					if (rand()%100<=pc_defender->splitchnc())
					{
						if (pc_defender->split()==1) splitnum=1;
						else splitnum=rand()%pc_defender->split()+1;
						
						for (splitcount=0;splitcount<splitnum;splitcount++)
							Npcs->Split(pc_defender);
					}
				}
			}

		/* 
			Concept note( Sebastian@hartte.de - darkstorm ):
			We deal the damage in the moment the swing animation
			starts, this is *bad* we shoudl determine a given
			amount of time it will take for the swing animation 
			to complete and *then* deal the damage if the user is 
			still in range (srvparams option!)
		*/
//		pc_defender->hp = QMAX( 0, pc_defender->hp );
		pc_defender->setHp( QMAX( 0, pc_defender->hp() ) );
		//===== RESEND HEALTH BAR(S)
		pc_defender->updateHealth();

		//===== DAMAGE ANIMATION
		// Only show it when damage has been dealt at all.
		if( pc_defender->id() >= 0x0190 && damage )
		{
			if( !pc_defender->onHorse() ) 
				pc_defender->action( 0x14 );
		}

		//===== SOUNDEFFECTS
		if( pc_attacker->isPlayer() )
			if( ( fightskill == ARCHERY && los) || fightskill != ARCHERY )
				doSoundEffect( pc_attacker, fightskill, pWeapon );
}

static void NpcSpellAttack( P_CHAR pc_attacker, P_CHAR pc_defender, unsigned int currenttime, int los )
{
	if( pc_attacker->spatimer <= currenttime )
	{
		int spattacks = numbitsset( pc_attacker->spattack );

		if (!pc_defender->dead() && chardist(pc_attacker, pc_defender) < SrvParams->attack_distance() && spattacks > 0 )
		{
			if (los)
			{																	
				int spattackbit=rand()%(spattacks) + 1;
				switch(whichbit(pc_attacker->spattack, spattackbit))
				{
				case 1:
					if (pc_attacker->mn()>=4)
					{
						npcaction(pc_attacker, 6);
						Magic->MagicArrow(pc_attacker, pc_defender);
					}
					break; 
				case 2:
					if (pc_attacker->mn()>=6)
					{
						npcaction(pc_attacker, 6);
						Magic->HarmSpell(pc_attacker, pc_defender);
					}
					break; //lb
				case 3:
					if (pc_attacker->mn()>=4)
					{
						npcaction(pc_attacker, 6);
						Magic->ClumsySpell(pc_attacker, pc_defender);										
					}
					break; //LB
				case 4:
					if (pc_attacker->mn()>=4)
					{
						npcaction(pc_attacker, 6);
						Magic->FeebleMindSpell(pc_attacker, pc_defender);
					}
					break; //LB
				case 5:
					if (pc_attacker->mn()>=4)
					{
						npcaction(pc_attacker, 6);
						Magic->WeakenSpell(pc_attacker, pc_defender);
					}
					break; //LB
				case 6:
					if (pc_attacker->mn()>=9)
					{
						npcaction(pc_attacker, 6);
						Magic->FireballSpell(pc_attacker, pc_defender);
					}
					break; //LB
				case 7:
					if (pc_attacker->mn()>=11)
					{
						npcaction(pc_attacker, 6);
						Magic->CurseSpell(pc_attacker, pc_defender);
					}
					break; //LB
				case 8:
					if (pc_attacker->mn()>=11)
					{
						npcaction(pc_attacker, 6);
						Magic->LightningSpell(pc_attacker, pc_defender);
					}
					break; //lb
				case 9:
					if (pc_attacker->mn()>=14)
					{
						npcaction(pc_attacker, 6);
						Magic->ParalyzeSpell(pc_attacker, pc_defender);
					}
					break; //lb
				case 10:
					if (pc_attacker->mn()>=14)
					{
						npcaction(pc_attacker, 6);
						Magic->MindBlastSpell(pc_attacker, pc_defender);
					}
					break;
				case 11:
					if (pc_attacker->mn()>=20)
					{
						npcaction(pc_attacker, 6);
						Magic->EnergyBoltSpell(pc_attacker, pc_defender);
					}
					break;
				case 12:
					if (pc_attacker->mn()>=20)
					{
						npcaction(pc_attacker, 6);
						Magic->ExplosionSpell(pc_attacker, pc_defender);
					}
					break;
				case 13:
					if (pc_attacker->mn()>=40)
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
static void SetWeaponTimeout( P_CHAR Attacker, P_ITEM Weapon )
{
	int x,j;
	
	if (Weapon) 
	{ 
		if( Weapon->speed() == 0 ) 
			Weapon->setSpeed( 35 );

		x = (15000*MY_CLOCKS_PER_SEC) / ((Attacker->effDex()+100) * Weapon->speed() ); //Calculate combat delay
	}
	else 
	{
		if(Attacker->skill(WRESTLING)>200) j = 35;
		else if(Attacker->skill(WRESTLING)>400) j = 40;
		else if(Attacker->skill(WRESTLING)>600) j = 45;
		else if(Attacker->skill(WRESTLING)>800) j = 50;
		else j = 30;
		x = (15000*MY_CLOCKS_PER_SEC) / ((Attacker->effDex()+100) * j);
	}
	Attacker->timeout=uiCurrentTime+x;
}

void cCombat::DoCombatAnimations( P_CHAR pc_attacker, P_CHAR pc_defender, int fightskill, int bowtype, int los )
{
	// Check that pc_attacker is facing the right direction
	pc_attacker->turnTo( pc_defender );

	UINT16 id = pc_attacker->id();

	// Monsters recieve special treatment
	if( id < 0x0190 )
	{
		UINT8 action = 4 + RandomNum( 0, 3 ); // some creatures dont have animation #4
		if( creatures[id].who_am_i & 0x2 ) // anti blink bit set ?
		{
			action++;

			if( id == 5 ) // eagles need special treatment
			{
				switch( RandomNum( 0, 2 ) )
				{
				case 0: action = 0x1;  break;
				case 1: action = 0x14; break;
				case 2: action = 0x4;  break;
				}
			}
		}

		pc_attacker->action( action );
		playmonstersound( pc_attacker, pc_attacker->id(), SND_ATTACK );
	}
	else if (pc_attacker->onHorse())
	{
		CombatOnHorse( pc_attacker ); // determines weapon in hand and runs animation
	}
	else
	{
		CombatOnFoot( pc_attacker );	// determines weapon in hand and runs animation kolours
	}

	// Show flying arrows if archery was used
	if( fightskill == ARCHERY )
	{
		if( los )
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

void cCombat::DoCombat( P_CHAR pc_attacker, unsigned int currenttime )
{
	// We are either not fighting or dont have a target
	if( !pc_attacker || pc_attacker->free || !pc_attacker->war || ( pc_attacker->targ == INVALID_SERIAL ) )
		return;

	int x, bowtype=0; // spamanachecking,tmp;
	unsigned int tempuint;
	P_ITEM pWeapon = pc_attacker->getWeapon();

	P_CHAR pc_defender = FindCharBySerial( pc_attacker->targ );

	// We are at war but our target's gone out of reach
	if( !pc_defender || pc_defender->free || ( pc_defender->isPlayer() && !pc_defender->socket() ) || pc_defender->isHidden() )
	{
		pc_attacker->war = false;
		pc_attacker->timeout = 0;
		pc_attacker->attacker = INVALID_SERIAL;
		pc_attacker->resetAttackFirst();

		// Send a warmode update
		pc_attacker->resend( false );
		
		// Update the little button on the paperdoll
		if( pc_attacker->socket() )
		{
			cUOTxWarmode warmode;
			warmode.setStatus( 0 );
			pc_attacker->socket()->send( &warmode );
		}

		return;
	}

	// We can be sure here that pc_defender is a valid hit-target
	if( pc_attacker->isNpc() || pc_attacker->socket() )
	{	
		// Too far above or below us
		if( pc_attacker->pos.z > ( pc_defender->pos.z + 10 ) ) 
			return;

		if( pc_attacker->pos.z < ( pc_defender->pos.z - 10 ) ) 
			return;
		
		if( ( pc_defender->isNpc() && pc_defender->npcaitype() !=17 ) || ( pc_defender->socket() && !pc_defender->dead() ) )
		{
			if( !pc_attacker->inRange( pc_defender, SrvParams->attack_distance() ) )
			{
				// Guards to scotty-beam-me-up
				if( pc_attacker->npcaitype() == 4 && pc_attacker->inGuardedArea() )
				{
					pc_attacker->removeFromView( false );
					pc_attacker->moveTo( pc_defender->pos );
					pc_attacker->resend( false );
					
					pc_attacker->soundEffect( 0x1FE );
					staticeffect(pc_attacker, 0x37, 0x2A, 0x09, 0x06);
					pc_attacker->talk( tr("Halt, scoundrel!"), -1, 0, true );
				}
				else if( pc_attacker->isNpc() ) // Any other NPC
				{
					pc_attacker->targ = INVALID_SERIAL;
					pc_attacker->timeout=0;
					
					P_CHAR pc = FindCharBySerial( pc_attacker->attacker );
					if( pc )
					{
						pc->resetAttackFirst();
						pc->attacker = INVALID_SERIAL;
					}
					pc_attacker->attacker = INVALID_SERIAL;
					pc_attacker->resetAttackFirst();
					if (pc_attacker->isNpc() && pc_attacker->npcaitype()!=17 && !pc_attacker->dead() && pc_attacker->war)
						pc_attacker->toggleCombat();
				}
			}
			else
			{
				if( pc_attacker->targ == INVALID_SERIAL )
				{
					pc_defender->fight( pc_attacker );
					pc_defender->setAttackFirst();
					pc_attacker->fight( pc_defender );
					pc_attacker->resetAttackFirst();
					x=( ( ( 100-pc_attacker->effDex() ) * MY_CLOCKS_PER_SEC ) / 25 ) + ( 1 * MY_CLOCKS_PER_SEC ); //Yet another attempt.
					pc_attacker->timeout=currenttime+x;
					return;
				}
				if( Combat->TimerOk( pc_attacker ) )
				{
					int los = lineOfSight( pc_attacker->pos, pc_defender->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING );
					int fightskill = Skills->GetCombatSkill(pc_attacker);
					bool mayAttack = false;

					if( fightskill == ARCHERY )
					{
						if( los )
						{
							int arrowsquant;
							bowtype=Combat->GetBowType( pc_attacker );

							if( bowtype == 1 ) 
								arrowsquant = getamount( pc_attacker, 0x0F3F );
							else
								arrowsquant = getamount( pc_attacker, 0x1BFB );

							if( arrowsquant > 0 )
								mayAttack = true;
						}
					}
					else if( pc_attacker->inRange( pc_defender, 1 ) )
						mayAttack = true;
					

					if( mayAttack )
					{
						// - Do stamina maths -
						if( abs( SrvParams->attackstamina() ) > 0 && !pc_attacker->isGM() )
						{
							if( (SrvParams->attackstamina() < 0 ) && ( pc_attacker->stm() < abs( SrvParams->attackstamina() ) ) )
							{
								if( pc_attacker->socket() )
									pc_attacker->socket()->sysMessage( tr( "You are too tired to attack." ) );
								
								SetWeaponTimeout( pc_attacker, pWeapon );
								return;
							}

//							pc_attacker->stm += SrvParams->attackstamina();
							pc_attacker->setStm( pc_attacker->stm() + SrvParams->attackstamina() );
							if( pc_attacker->stm() > pc_attacker->effDex() )
								pc_attacker->setStm( pc_attacker->effDex() );
							
							if( pc_attacker->stm() < 0 )
								pc_attacker->setStm(0);

							// Send the changed stamina
							if( pc_attacker->socket() )
								pc_attacker->socket()->updateStamina();
						}
						
						DoCombatAnimations( pc_attacker, pc_defender, fightskill, bowtype, los );

						if( ( ( chardist( pc_attacker, pc_defender )<2)||(fightskill==ARCHERY))&&!(pc_attacker->npcaitype()==4))
                        {
							if (los)
							{
								pc_defender->fight( pc_attacker );
								pc_defender->setAttackFirst();
								pc_attacker->fight( pc_defender );
								pc_attacker->resetAttackFirst();
							}
						}

						if( pc_attacker->timeout2 > uiCurrentTime )
							return;

						// A tempeffect is needed here eventually
						// An Arrow doesnt hit its target immedeately..
						if( fightskill == ARCHERY )
							Combat->CombatHit( pc_attacker, pc_defender, currenttime, los );
						else
							pc_attacker->setSwingTarg( pc_defender->serial );
					}

					SetWeaponTimeout(pc_attacker, pWeapon);
					pc_attacker->timeout2=pc_attacker->timeout; // set shotclock memory-BackStab

					if( !pc_defender->isInvul() )
					{
						NpcSpellAttack( pc_attacker,pc_defender,currenttime,los );
					}

					if( fightskill != ARCHERY)
					{
						Combat->CombatHit( pc_attacker, pc_defender, currenttime, los );
					}
				}
			}			

			// Our target finally died.
			if( pc_defender->hp() < 1 ) //Highlight //Repsys
			{
				if((pc_attacker->npcaitype() == 4 || pc_attacker->npcaitype() == 9) && pc_defender->isNpc())
				{
					pc_defender->action( 0x15 );					
					PlayDeathSound( pc_defender );					
					Npcs->DeleteChar( pc_defender ); //Guards, don't give body
				}
				else
				{
					pc_defender->kill();
				}
				
				//murder count \/				
				if ((pc_attacker->isPlayer())&&(pc_defender->isPlayer()))//Player vs Player
				{
					if(pc_defender->isInnocent() && GuildCompare(pc_attacker, pc_defender )==0 )
					{
//						++pc_attacker->kills;
						tempuint = pc_attacker->kills();
						pc_attacker->setKills( ++tempuint );


						if( pc_attacker->socket() )
						{
							pc_attacker->socket()->sysMessage( tr( "You have killed %1 innocent people." ).arg( pc_attacker->kills() ) );

							if( pc_attacker->kills() == SrvParams->maxkills() + 1 )
								pc_attacker->socket()->sysMessage( tr("You are now a murderer!") );
						}
					}
					
					if( SrvParams->pvpLog() )
					{
						sprintf((char*)temp,"%s was killed by %s!\n",pc_defender->name.c_str(), pc_attacker->name.c_str());
						savelog((char*)temp,"PvP.log");
					}
				}

				pc_attacker->toggleCombat();
			}
		}
	}
}

int cCombat::CalcAtt( P_CHAR pc_p ) // Calculate total attack powerer
{
	int total = 0, j;
	
	if ( pc_p == NULL ) return 0;

	if( pc_p->isNpc() ) 
	{
		//Compatibility for use with older scripts
		if((pc_p->lodamage==0)&&(pc_p->hidamage==0)) return(pc_p->att);
		
		//Code for new script commands
		if(pc_p->lodamage==pc_p->hidamage) //Avoids divide by zero error
		{
			return(pc_p->lodamage());
		}
		if(!(pc_p->hidamage()<=2))
		{
			j = pc_p->hidamage() - pc_p->lodamage(); 
			if (j!=0) total=(rand()%(j))+pc_p->lodamage(); else total=pc_p->lodamage();
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
		if( pi->layer() == 1 || pi->layer() == 2 )
		{
			if (pi->lodamage() == pi->hidamage()) 
				total += pi->lodamage();
			else if ((pi->lodamage()>0)&&(pi->hidamage()>0))
			{
				if (pi->hidamage()-pi->lodamage()!=0) 
					total += (rand()%(pi->hidamage()-pi->lodamage()))+(pi->lodamage());
				else total+=pi->lodamage();
			}
			else if ((pi->lodamage()==0)&&(pi->hidamage()==0)) 
				total += pi->att;
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
			total+=(((pc->skill(PARRYING)*pShield->def)/200)+1); // Updated to OSI standars (Skyfire)
	} 		//Displayed AR = ((Parrying Skill * Base AR of Shield) ÷ 200) + 1
	if (pc->skill(PARRYING)>=1000) 
		total+=5; // gm parry bonus. 
	if (ishuman(pc)) // Added by Magius(CHE) 
	{ 
		unsigned int ci=0; 
		P_ITEM pi; 
		vector<SERIAL> vecContainer = contsp.getData(pc->serial);
		for ( ci = 0; ci < vecContainer.size(); ci++)
		{
			pi = FindItemBySerial(vecContainer[ci]);
			if( pi != NULL && pi->layer() > 1 && pi->layer() < 25 ) 
			{ 
				//blackwinds new stuff 
				int armordef=0; 
				if (pi->def>0) 
				{ 
					float armorhpperc = ( (float)pi->hp()/((float)pi->maxhp()/100)); 
					armordef=(int)(((float)pi->def/100)*armorhpperc); 
				} 

				if (x==0) 
					total+=pi->def; 
				else 
				{ 
					switch( pi->layer() ) 
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

	// TODO: MOVE THAT TO THE DAMN APPROPIATE FUNCTION !! THIS DAMGES EVERYTHING WHEN JUST CALCULATING

	cUOSocket *mSock = pc->socket();
	if( pj && mSock ) 
	{ 
		// don't damage hairs, beard and backpack 
		// important! this sometimes cause backpack destroy! 
		if( pj->layer() != 0x0B && pj->layer() != 0x10 && pj->layer() != 0x15 )
		{
			//Take off a hit point
			if( !( rand() % 2 ) )
				pj->setHp( pj->hp() - 1 ); 
			if( pj->hp() <= 0 ) 
			{ 
				mSock->sysMessage( tr( "Your %1 has been destroyed" ).arg( pj->getName() ) );
				pc->removeItemBonus( pj ); // remove BONUS STATS given by equipped special items

				Items->DeleItem( pj );
			}
		}
	}
	if (total < 2) total = 2;
	return total;
}

int cCombat::TimerOk(P_CHAR pc)
{
	int d = 0;
	
	if( !pc )
		return 0;

    if( ( pc->timeout < uiCurrentTime ) && ( pc->timeout2 < uiCurrentTime ) ) 
		d=1;
	
	if( overflow )
		d = 1;

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
	switch( pHorseItem->id()&0x00FF )
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
	}
	else
	{
		npcaction(pc, 0x1A); //fist fighting
		return;
	}
}

//play animation for weapon in hand for combat on foot //kolours - (09/20/98)
void cCombat::CombatOnFoot(P_CHAR pc)
{
	if( !pc )
		return;

	P_ITEM pWeapon = pc->getWeapon();

	if( pWeapon )
	{
		UINT16 id = pWeapon->id();

		if( IsBow( id ) )
		{
			pc->action( 0x12 ); // bow
			return;
		} 
		else if( IsCrossbow( id ) || IsHeavyCrossbow( id ) )
		{
			pc->action( 0x13 ); //crossbow - regular
			return;
		}
		else if( IsSword( id ) )
		{
			switch( RandomNum( 1, 3 ) ) //swords
			{
			case 1:		pc->action( 0x0D );	return; //side swing
			case 2:		pc->action( 0x0A );	return; //poke
			case 3:		pc->action( 0x09 );	return; //top-down swing
			}
		}
		else if( IsMace1H( id ) )
		{
			switch( RandomNum( 1,2 ) ) //maces
			{
			case 1:		pc->action( 0x0D );	return;	// side swing
			case 2:		pc->action( 0x09 );	return; // top-down swing
			}
		}
		else if( IsMace2H( id ) )
		{
			switch( RandomNum( 1, 3 ) )
			{
			case 1:		pc->action( 0x0C );	return; // 2H top-down
			case 2:		pc->action( 0x0D );	return; // 2H swing
			case 3:		pc->action( 0x0E );	return; // 2H jab
			}
		}
		else if( IsAxe( id ) )
		{
			switch( RandomNum( 1, 2 ) )
			{
			case 1:		pc->action( 0x0C );	return; // 2H top-down
			case 2:		pc->action( 0x0D );	return; // 2H swing
			}
		}
		else if (IsFencing1H(id))	// one handed fencing
		{
			switch( RandomNum( 1, 3 ) ) //fencing
			{
			case 1:		pc->action( 0x09 );	return; //top-down
			case 2:		pc->action( 0x0D );	return; //side-swipe
			case 3:		pc->action( 0x0A );	return; //default: poke
			}
		}
		else if( IsFencing2H( id ) ) // pitchfork & spear
		{
			switch( RandomNum( 1, 2) )	//pitchfork
			{
			case 1:		pc->action( 0x0D );	return; //top-down
			case 2:		pc->action( 0x0E );	return; //default: 2-handed poke
			}
		}
	}

	switch( RandomNum( 1, 3 ) ) //fist fighting
	{
		case 1:		pc->action( 0x0A );	return; //fist straight-punch
		case 2:		pc->action( 0x09 );	return; //fist top-down
		case 3:		pc->action( 0x1F );	return; //fist over-head
	}
}


//s: char#
void cCombat::SpawnGuard( P_CHAR pc_offender, P_CHAR pc_caller, const Coord_cl &pos )
{
	QString guardSect;

	if ( pc_offender == NULL || pc_caller == NULL)
		return;
	
	if (!pc_caller->inGuardedArea())
		return;
	
	if (pc_offender->dead() || pc_caller->dead())
		return; // AntiChrist

	cTerritory* Region = pc_caller->region;

	if( Region == NULL )
		return;
	
	if (SrvParams->guardsActive() && !pc_offender->isInvul())
	{
		P_CHAR pc_guard = Npcs->createScriptNpc( Region->getGuardSect(), pos );
		
		if ( !pc_guard ) 
			return;
		
		pc_guard->setNpcAIType( 4 );
		pc_guard->setAttackFirst();
		pc_guard->attacker = pc_offender->serial;
		pc_guard->targ = pc_offender->serial;
		pc_guard->npcWander = 2;  // set wander mode
		pc_guard->toggleCombat();
		pc_guard->setNextMoveTime();
		pc_guard->summontimer =(getNormalizedTime() +(MY_CLOCKS_PER_SEC*25));    
		
		pc_guard->soundEffect( 0x1FE );
		staticeffect( pc_guard, 0x37, 0x2A, 0x09, 0x06 );

		pc_guard->resend( false ); // Update warmode status to other players

		// 50% talk chance
		switch( RandomNum( 0, 4 ) )
		{
			case 0:		pc_guard->talk( tr( "Thou shalt regret thine actions, swine!" ), -1, 0, true ); break;
			case 1:		pc_guard->talk( tr( "Death to all Evil!" ), -1, 0, true );						break;
		}
	}
}

void cCombat::ItemSpell(cChar* Attacker, cChar* Defender)
{
	// Check this when magic is done
	/*if( Attacker->npc ) // npcs can't use casting weapons right now (Duke)
		return;
	currentSpellType[ calcSocketFromChar( Attacker ) ]=2;
	unsigned int ci;
	P_ITEM pi;
	vector<SERIAL> vecContainer = contsp.getData(Attacker->serial);
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if ( ( ( pi->layer() == 1 && pi->type() != 9 ) || (pi->layer() == 2 ) ) )
		{
			if( pi->offspell() && ( pi->att || pi->hidamage() ) && pi->type() == 15 )
			{
				switch( pi->offspell() )
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
					LogErrorVar("invalid offspell value %i",pi->offspell());
				}
				pi->morez--;
				if (pi->morez == 0)
				{
					pi->setType( pi->type2() );
					pi->morex = 0;
					pi->morey = 0;
					pi->setOffspell( 0 );
				}
			}
			return;
		}
    }*/
}

//AntiChrist - do the sound effect ( only if HITTEN! )
void cCombat::doSoundEffect( P_CHAR pc, int fightskill, P_ITEM pWeapon )
{
	bool heavy = false;
	int a = RandomNum( 0, 3 );

	//check if a heavy weapon
	if( pWeapon && IsAxe( pWeapon->id() ) )
		heavy = true;

	if( heavy )
	{
		if (a==0 || a==1) pc->soundEffect( 0x236 );
		else pc->soundEffect( 0x237 );
		return;
	}		

	switch(fightskill)
	{
	case ARCHERY:
		pc->soundEffect( 0x234);
		break;
	case FENCING:
	case SWORDSMANSHIP:
		if (a==0 || a==1) 
			pc->soundEffect( 0x23B );
		else 
			pc->soundEffect( 0x23C );
		break;
	case MACEFIGHTING:
		if ( a==0 || a==1 ) 
			pc->soundEffect( 0x232 );
		else if ( a==2 ) 
			pc->soundEffect( 0x139 );
		else 
			pc->soundEffect( 0x233 );
		break;
	case WRESTLING:
		if (a==0) pc->soundEffect( 0x135 );
		else if (a==1) pc->soundEffect( 0x137 );
		else if (a==2) pc->soundEffect( 0x13D );
		else pc->soundEffect( 0x13B );
		break;
	default:
		pc->soundEffect( 0x13D );
	}
}

// do the "MISSED" sound effect
void cCombat::doMissedSoundEffect( P_CHAR pChar )
{
	UINT16 soundId;
    
	switch( RandomNum( 1, 3 ) )
	{
	case 1: soundId = 0x238; break;
	case 2: soundId = 0x239; break;
	case 3: soundId = 0x23A; break;
	};

	pChar->soundEffect( soundId );
}
