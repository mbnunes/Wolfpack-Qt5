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
#include "typedefs.h"
#include "wpdefmanager.h"

#include "debug.h"
#undef  DBGFILE
#define DBGFILE "combat.cpp"

#define SWINGAT (unsigned int)1.75 * MY_CLOCKS_PER_SEC // changed from signed to unsigned, LB

namespace Combat 
{
	/*!

		Check for the weaponskill required by this weapon by checking it's type.

	*/
	UI16 weaponSkill( P_ITEM pi )
	{
		if( !pi )
			return WRESTLING;

		switch( pi->type() )
		{
			// 1001: Sword Weapons (Swordsmanship)
			// 1002: Axe Weapons (Swordsmanship + Lumberjacking)
			case 1001:			
			case 1002:
				return SWORDSMANSHIP;
				break;
	
			// 1003: Macefighting (Staffs)
			// 1004: Macefighting (Maces/WarHammer)
			case 1003:			
			case 1004:
				return MACEFIGHTING;
				break;
	
			// 1005: Fencing
			case 1005:
				return FENCING;
				break;
	
			// 1006: Bows
			// 1007: Crossbows
			case 1006:
			case 1007:
				return ARCHERY;
				break;

			default:
				return WRESTLING;
		};
	}

	/*!
		Check for the bow-type of this item by evaluating it's type.
		BOW for a bow.
		XBOW for a crossbow.
	*/
	enBowTypes bowType( P_ITEM pi )
	{
		if( !pi )
			return INVALID_BOWTYPE;

		switch( pi->type() )
		{
			case 1006:
				return BOW;

			case 1007:
				return XBOW;

			default:
				return INVALID_BOWTYPE;
		}
	}

	/*!
		This is called when we finish to swing our weapon.
		It basically checks for LoS and distance
		and then calls hit().
	*/
	void checkandhit( P_CHAR pAttacker )
	{
		P_CHAR pDefender = FindCharBySerial( pAttacker->swingtarg() );
		pAttacker->setSwingTarg( INVALID_SERIAL );

		if( !pDefender || pDefender->isInvul() )
		{
			if( pAttacker->socket() )
				pAttacker->socket()->sysMessage( tr( "You can't hit your target." ) );

			return;
		}

		// Can we see our target. 
		// I don't know what the +z 13 means...
		bool los = lineOfSight( pAttacker->pos + Coord_cl( 0, 0, 13 ), pDefender->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING );

		hit( pAttacker, pDefender, los );
	}

	/*!
		This function is responsible for calculating
		the to-hit chance and dealing the right 
		amount of damage. This is called *after* 
		we completed a swing.
	*/
	void hit( P_CHAR pAttacker, P_CHAR pDefender, bool los )
	{
		SI32 totaldamage = 0;

		// Get the weapon the attacker is wearing.
		P_ITEM pWeapon = pAttacker->getWeapon();
		UINT16 wSkill = pWeapon ? pWeapon->getWeaponSkill() : WRESTLING;
		enBowTypes bowtype = bowType( pWeapon );

		// We simply can't see our target.
		if( !los || ( wSkill != ARCHERY && pAttacker->pos.distance( pDefender->pos ) > 1 ) )
			return;

		// There is a 50% chance that
		// our weapon will be damaged.
		if( wSkill != WRESTLING && pWeapon )
			if( pWeapon->wearOut() ) // Our weapon has been destroyed
				wSkill = WRESTLING;

		// For Bows and Crossbows, 
		// check if the user has enough ammunition
		P_ITEM pAmmo = NULL;

		if( wSkill == ARCHERY )
		{
			UINT16 id = ( bowtype == BOW ) ? 0xF3F : 0x1BFB;

			P_ITEM pBackpack = pAttacker->getBackpack();

			if( !pBackpack || !pBackpack->DeleteAmount( 1, id ) )
			{
				// We have to be careful here.
				// We could be spammed with those messages.
				if( pAttacker->socket() )
					pAttacker->socket()->sysMessage( tr( "You are out of ammunition!" ) );

				return;
			}
		}

		UINT16 minskill, maxskill;
		
		// If we are wearing a weapon, let the skill-check
		// depend on the power of the weapon automatically.		
		if( wSkill != WRESTLING && pWeapon )
		{
			// Maximum 99% minimum skill
			minskill = QMIN( 990, pWeapon->lodamage() * 10 );
			maxskill = pWeapon->hidamage() * 10;
		}
		// For wrestling a simple 0-100 check is made.
		else
		{
			minskill = 0;
			maxskill = 1000;
		}

		// Gain in the three skills we use during combat.
		pAttacker->checkSkill( wSkill, minskill, maxskill );
		pAttacker->checkSkill( TACTICS, minskill, maxskill );
		pAttacker->checkSkill( ANATOMY, minskill, maxskill );

		// Calculate the defense Skill
		P_ITEM pDefWeapon = pDefender->getWeapon();
		UINT16 dSkill = pDefWeapon ? pDefWeapon->getWeaponSkill() : WRESTLING;
		
		// You can't defend yourself with a bow in your hand
		if( dSkill == ARCHERY )
		{
			dSkill = WRESTLING;
			pDefWeapon = 0; // So we dont get accounted for the weapon-strength
		}

		// Do the same check like we did	
		if( dSkill != WRESTLING && pDefWeapon )
		{
			// Maximum 99% minimum skill
			minskill = QMIN( 990, pDefWeapon->lodamage() * 10 );
			maxskill = pDefWeapon->hidamage() * 10;
		}
		else
		{
			minskill = 0;
			maxskill = 1000;
		}

		// Check skills for the defender
		pDefender->checkSkill( dSkill, minskill, maxskill );
		pDefender->checkSkill( TACTICS, minskill, maxskill );

		// Now Calculate the Combat Ability of the Defender
		// This is either the used skill OR for wrestling:
		// wrestling skill *or* evasion skill
		UINT16 dEvasion = QMIN( 120, pDefender->skill( dSkill ) / 10 ); // Up to 120

		// If we are unarmed there could be a chance that we can 
		// evade the blow by using EVALINT + ANATOMY
		if( dSkill == WRESTLING )
			dEvasion = QMAX( dEvasion, QMIN( 120, ( pDefender->skill( EVALUATINGINTEL ) + pDefender->skill( ANATOMY ) + 200 ) / 20 ) );

		// Now we have to calculate hitChance
		// Hit Chance = ( Attacker's Combat Ability + 50 ) ÷ ( [Defender's Combat Ability + 50] x 2 )
		double hitChance = ( pAttacker->skill( wSkill ) / 10 ) + 50;
		hitChance /= ( dEvasion + 50 ) * 2;
		hitChance = floor( hitChance * 100 );

		pAttacker->message( QString( "Your chance to hit %1 is %2" ).arg( pDefender->name ).arg( hitChance ) );

		// Check if we missed
		if( RandomNum( 1, 100 ) > hitChance )
		{
			// Display a message to both players that the 
			// swing didn't hit
			// NOTE: There should be a random chance that this
			// message appears *or* a flag to set
			if( pAttacker->socket() )
				pAttacker->socket()->sysMessage( tr( "You miss %1" ).arg( pDefender->name ) );

			if( pDefender->socket() )
				pDefender->socket()->sysMessage( tr( "%1 misses you" ).arg( pAttacker->name ) );

			// If we missed using a Bow or Crossbow we 
			// could leave the ammunition at the feets of 
			// our target. 
			if( wSkill == ARCHERY )
			{
				if( pAmmo && RandomNum( 1, 3 ) == 1 ) // 1/3 chance
				{
					pAmmo->moveTo( pDefender->pos );
					pAmmo->priv = 1;
					pAmmo->startDecay();
					pAmmo->update();
				}
			}

			return;
		}

		// If we used a poisoned Weapon to deal 
		// damage, apply the poison here
		if( pWeapon && ( pWeapon->poisoned > 0 ) )
		{
			   pDefender->setPoisoned( pWeapon->poisoned );

			   // a lev.1 poison takes effect after 40 secs, a deadly pois.(lev.4) takes 40/4 secs
			   pDefender->setPoisontime( uiCurrentTime + ( MY_CLOCKS_PER_SEC * ( 40 / pDefender->poisoned() ) ) );
			   pDefender->setPoisonwearofftime( pDefender->poisontime() + ( MY_CLOCKS_PER_SEC * SrvParams->poisonTimer() ) );
		}

		// If we item we used was enchantet using 
		// some kind of spell, it's now time to
		// apply the spell effect. This feature
		// can only be used when magic has been
		// rewritten.

		// Calc base damage
		double damage;

		// If we wear a weapon, use that value,
		if( pWeapon )
			damage = RandomNum( pWeapon->lodamage(), pWeapon->hidamage() );

		// Modify the damaged based on the weapon skill
		// I did not find this in the OSI specs
		//damage = (SI32)ceil( (float)damage * (float)pAttacker->skill( wSkill ) / 1000.0f );
		
		// Fall back to lodamage/hidamage
		else if( pAttacker->lodamage() != 0 && pAttacker->hidamage() != 0 )
			// set basedamage to random value of weapon lo and hidamage
			damage = RandomNum( pAttacker->lodamage(), pAttacker->hidamage() );

		// otherwise use the WRESTLING skill
		else
			damage = RandomNum( 0, QMAX( 1, pAttacker->skill( WRESTLING ) / 50 ) );

		// Boni to damage:

		// Tactics (% of Base Damage is Tactics + 50)
		damage = damage * ( floor( pAttacker->skill( TACTICS ) / 10 ) + 50 ) / 100;

		// Strength (add 1/5 of strength * damage)
		damage += damage * ( pAttacker->st() / 500 );

		// Anatomy (add 1/5 of anatomy * damage)
		damage += damage * ( pAttacker->skill( ANATOMY ) / 500 );

		// Anatomy GM gives another +10%
		if( pAttacker->baseSkill( ANATOMY ) >= 1000 )
			damage += damage * 0.10;
			
		// If we are using an axe then add the lumberjacking bonus
		if( pWeapon && pWeapon->type() == 1002 )
		{
			// Lumberjacking (add 1/5 of lumberjacking * damage)
			damage += damage * ( pAttacker->skill( LUMBERJACKING ) / 500 );

			// Lumberjacking GM gives another +10%
			if( pAttacker->baseSkill( LUMBERJACKING ) >= 1000 )
				damage += damage * 0.10;
		}

		pAttacker->message( QString( "You deal %1 points of damage" ).arg( damage ) );

		// Parrying with shield
		/*P_ITEM pShield = pDefender->leftHandItem();
		if( pShield && IsShield( pShield->id() ) )
		{
			pDefender->checkSkill( PARRYING, (int)floor( ( (float)accuracy / 100.0f) * (float)pAttacker->skill( fightskill ) ), 1000 );
			// % Chance of Blocking= Parrying Skill ÷ 2
			if( pDefender->skill( PARRYING ) / 2 >= RandomNum( 0, 1000 ) )
			{
				// damage absorbed by shield
				if( pShield->def != 0 )
					damage -= RandomNum( 0, pShield->def ); 
					pShield->wearOut();
			}
		}*/

		/*SI32 bodyhit = RandomNum( 0, accuracy );
		enBodyParts bodypart;
		if( bodyhit > 100 )
			bodyhit = 100;

		if( bodyhit <= 14 )
			bodypart = LEGS;
		else if( bodyhit <= 58 )
			bodypart = BODY;
		else if( bodyhit <= 72 )
			bodypart = ARMS;
		else if( bodyhit <= 79 )
			bodypart = HANDS;
		else if( bodyhit <= 86 )
			bodypart = NECK;
		else if( bodyhit <= 100 )
			bodypart = HEAD;

		// Damage Absorbed= Random value between of 1/2 AR to full AR of Hit Location's piece of armor.
		SI32 ar = pDefender->calcDefense( bodypart, true );
		damage -= RandomNum( ar / 2, ar );
		
		//When we reached this point, nothing will affect the damage anymore.
		//If damage is <= 0, the defender was successful, so lets break this iteration here...
		if( damage <= 0 )
		{
				if( pAttacker->socket() )
					pAttacker->socket()->sysMessage( tr("Your attack has been parried!") );
				if( pDefender->socket() )
					pDefender->socket()->sysMessage( tr("You parried the blow!") );
				playMissedSoundEffect( pAttacker );
				++it;
				continue;
			}

			QString defMessage = (char*)0;
			QString attMessage = (char*)0;

			// Generate the messages according to bodypart and damage
			switch( bodypart )
			{
			case LEGS:
				switch( RandomNum( 0, 2 ) )
				{
				case 0:
					defMessage = tr( "%1 hits you in the left thigh!" );
					attMessage = tr( "You hit %1 in the left thigh!" );
					break;
				case 1:
					defMessage = tr( "%1 hits you in the right thigh!" );
					attMessage = tr( "You hit %1 in the right thigh!" );
					break;
				case 2:
					defMessage = tr( "%1 hits you in the groin!" );
					attMessage = tr( "You hit %1 in the groin!" );
					break;
				}
				break;
			case BODY:
				switch( RandomNum( 0, 2 ) )
				{
				case 0:
					if( damage < 10 )
					{
						defMessage = tr( "%1 hits you in your chest!" );
						attMessage = tr( "You hit %1 in the chest!" );
					}
					else
					{
						defMessage = tr( "%1 lands a terrible blow to your chest!" );
						attMessage = tr( "You land a terrible blow to %1's chest!" );
					}
					break;
				case 1:
					if( damage < 10 )
					{
						defMessage = tr( "%1 lands a blow to your stomach!" );
						attMessage = tr( "You land a blow to %1's stomach!" );
					}
					else
					{
						defMessage = tr( "%1 knocks the wind out of you!" );
						attMessage = tr( "You knock the wind out of %1!" );
					}
					break;
				case 2:
					if( damage < 10 )
					{
						defMessage = tr( "%1 hits you in your ribs!" );
						attMessage = tr( "You hit %1 in the ribs!" );
					}
					else
					{
						defMessage = tr( "%1 has broken your Rib?!" );
						attMessage = tr( "It sounds as if you have broken one of %1's ribs!" );
					}
					break;
				}
				break;
			case ARMS:
				switch( RandomNum( 0, 1 ) )
				{
				case 0:
					defMessage = tr( "%1 hits you in your left arm!" );
					attMessage = tr( "You hit %1 in the left arm!" );
					break;
				case 1:
					defMessage = tr( "%1 hits you in your right arm!" );
					attMessage = tr( "You hit %1 in the right arm!" );
					break;
				}
				break;
			case HANDS:
				switch( RandomNum( 0, 1 ) )
				{
				case 0:
					defMessage = tr( "%1 hits you at your left hand!" );
					attMessage = tr( "You hit %1 at the left hand!" );
					break;
				case 1:
					defMessage = tr( "%1 hits you at your right hand!" );
					attMessage = tr( "You hit %1 at the right hand!" );
					break;
				}
				break;
			case NECK:
				defMessage = tr( "%1 hits you at the throat!" );
				attMessage = tr( "You hit %1 at the throat!" );
				break;
			case HEAD:
				if( fightskill == ARCHERY && accuracy >= 100 && pAttacker->skill( ARCHERY ) >= 1000 && RandomNum( 0, 4 ) == 0 )
				{
					// GM Archer head shot ...
					damage = pDefender->st();
					bodypart = DEADLY;
					attMessage = tr( "Your shot hits %1 directly between the eyes!" );
					defMessage = tr( "%1 hits you directly between the eyes!" );
				}
				else
				{
					switch( RandomNum( 0, 2 ) )
					{
					case 0:
						if( damage < 10 )
						{
							defMessage = tr( "%1 hits you straight in the face!" );
							attMessage = tr( "You hit %1 straight in the face!" );
						}
						else 
						{
							defMessage = tr( "%1 lands a stunning blow to your head!" );
							attMessage = tr( "You land a stunning blow to %1's head!" );
						}
						break;
					case 1:
						if( damage < 10 )
						{
							defMessage = tr( "%1 hits you at the head!" );
							attMessage = tr( "You hit %1 at the head!" );
						}
						else
						{
							defMessage = tr( "%1 smashed a blow across your face!" );
							attMessage = tr( "You smash a blow across %1's face!" );
						}
						break;
					case 2:
						if( damage < 10 )
						{
							defMessage = tr( "%1 hits you square in the jaw!" );
							attMessage = tr( "You hit %1 square in the jaw!" );
						}
						else
						{
							defMessage = tr( "%1 lands a terrible hit to your temple!" );
							attMessage = tr( "You land a terrible hit at %1's temple!" );
						}
						break;
					}
				}
				break;
			}

			if( pAttacker->socket() )
				pAttacker->socket()->sysMessage( attMessage.arg( pDefender->name.latin1() ) );
			if( pDefender->socket() )
				pDefender->socket()->sysMessage( defMessage.arg( pAttacker->name.latin1() ) );

			// Stamina Loss
			if( ( fightskill == MACEFIGHTING ) && ( IsSpecialMace( pItem->id() ) ) && pDefender->isPlayer() )
			{ 
				pDefender->setStm( pDefender->stm() - RandomNum( 3, 6 ) );
			}

			// Paralyzing
			if( ( fightskill == FENCING ) && ( IsFencing2H( pItem->id() ) ) && pDefender->isPlayer() )
			{ 
				tempeffect( pAttacker, pDefender, 44, 0, 0, 0 );
				if( pAttacker->socket() )
					pAttacker->socket()->sysMessage( tr( "You delivered a paralyzing blow" ) );
			}

			// Concussion Hit
			if( ( fightskill == SWORDSMANSHIP ) && ( IsAxe( pItem->id() ) ) && pDefender->isPlayer() )
			{ 
				tempeffect( pAttacker, pDefender, 45, 0, 0, 0 );
			}

			//===== SOUNDEFFECTS
			if( pAttacker->isPlayer() )
				if( ( fightskill == ARCHERY && los) || fightskill != ARCHERY )
					playSoundEffect( pAttacker, fightskill, pItem );

			totaldamage += damage;

			++it;
		}

		// ==== HIT SOUNDS
		if( pDefender->id() == 0x191 )
		{
			UI16 sound = hex2dec( DefManager->getRandomListEntry( "SOUNDS_COMBAT_HIT_HUMAN_FEMALE" ) ).toUShort();
			if( sound > 0 )
				pDefender->soundEffect( sound );
			else
				pDefender->soundEffect( 0x14b );
		}
		else if( pDefender->id() == 0x190 )
		{
			UI16 sound = hex2dec( DefManager->getRandomListEntry( "SOUNDS_COMBAT_HIT_HUMAN_MALE" ) ).toUShort();
			if( sound > 0 )
				pDefender->soundEffect( sound );
			else
				pDefender->soundEffect( 0x156 );
		}
		else	
			playmonstersound( pDefender, pDefender->id(), SND_DEFEND );

		// attacker poisons defender
		// and vice versa
		pAttacker->applyPoison( pDefender );
		pDefender->applyPoison( pAttacker );

		// ==== UNFREEZE
		if( ( pDefender->effDex() > 0 ) )
			pDefender->setPriv2( pDefender->priv2() & 0xFD );
			

		// when hitten and damage >1, defender fails if casting a spell!
		// Thats not really good, better make a check versus int+magic
		if( totaldamage > 1 && pDefender->isPlayer() )//only if damage>1 and against a player
		{
			// TODO: Implement spell apruption
			//if(pc_defender->casting() && currentSpellType[s2]==0 )
			//{//if casting a normal spell (scroll: no concentration loosen)
			//	Magic->SpellFail(s2);
		//		currentSpellType[s2]=0;
		//		pc_defender->setSpell(-1);
		//		pc_defender->setCasting(false);
		//		pc_defender->setSpelltime(0);
		//		pc_defender->priv2 &= 0xfd; // unfreeze, bugfix LB
		//	}
		}

		//===== REACTIVE ARMOR
		if( pDefender->ra() )
		{
			// lets reflect (MAGERY/2)% of the damage
			SI32 reflectdamage = (SI32)floor( (float)totaldamage * (float)pDefender->skill( MAGERY ) / 2000.0f );
			totaldamage -= reflectdamage;

			pAttacker->setHp( pAttacker->hp() - reflectdamage );
			if( pAttacker->hp() <= 0 )
				pAttacker->kill();

			staticeffect( pDefender, 0x37, 0x4A, 0, 15 ); //RA effect
			pAttacker->updateHealth();
		}

		//===== DEAL DAMAGE
		pDefender->setHp( pDefender->hp() - totaldamage );

		//===== BLOOD ( This *needs* a blood-color check)
		if( RandomNum( 1, 10 ) == 1 ) // 10% chance
		{
	       UINT16 id = 0x122c;

	       if( totaldamage > 50 )
			   id = 0x122a;

		   else if( totaldamage > 40 )
			   id = 0x122d;

	       else if( totaldamage > 30 )
			   id = 0x122e;

	       else if( totaldamage > 20 )
			   id = 0x122b;

		   else if( totaldamage < 10 )
			   id = 0x1645;

		   P_ITEM pBlood = Items->SpawnItem( pDefender, 1, "#", 0, id, 0, 0 );
		   
		   if( pBlood )
		   {
			  pBlood->moveTo( pDefender->pos );
			  pBlood->priv = 1;
			  pBlood->setGMMovable(); // Moveable by GM
			  pBlood->update();
			  pBlood->decaytime = ( 8 * MY_CLOCKS_PER_SEC ) + uiCurrentTime; // Will stay 8 secs
		   }
		}

		//===== SPLITTING NPCS
		UI08 splitnum = 0;
		if( ( pDefender->split() > 0 ) && ( pDefender->hp() >= 1 ) )
		{
			if( RandomNum( 0, 100 ) <= pDefender->splitchnc() )
			{
				if( pDefender->split() == 1 ) 
					splitnum = 1;
				else 
					splitnum = RandomNum( 1, pDefender->split() );
				
				for( UI08 splitcount = 0; splitcount < splitnum; splitcount++ )
					cCharStuff::Split( pDefender );
			}
		}

		//	Concept note( Sebastian@hartte.de - darkstorm ):
		//	We deal the damage in the moment the swing animation
		//	starts, this is *bad* we shoudl determine a given
		//	amount of time it will take for the swing animation 
		//	to complete and *then* deal the damage if the user is 
		//	still in range (srvparams option!)
		pDefender->setHp( QMAX( 0, pDefender->hp() ) );
		//===== RESEND HEALTH BAR(S)
		pDefender->updateHealth();

		//===== DAMAGE ANIMATION
		// Only show it when damage has been dealt at all.
		if( pDefender->id() >= 0x0190 && totaldamage > 0 )
		{
			if( pDefender->atLayer( cChar::Mount ) ) 
				pDefender->action( 0x14 );
		}*/
	}

	void combat( P_CHAR pAttacker )
	{
		// We are either not fighting or dont have a target
		if( !pAttacker || pAttacker->free || !pAttacker->war() || ( pAttacker->targ() == INVALID_SERIAL ) )
			return;

		P_CHAR pDefender = FindCharBySerial( pAttacker->targ() );

		// We are at war but our target's gone out of range
		if( !pDefender || pDefender->free || ( pDefender->isPlayer() && !pDefender->socket() ) || pDefender->isHidden() )
		{
			pAttacker->setTimeOut(0);
			if( pDefender )
				pDefender->setAttacker(INVALID_SERIAL);
			pAttacker->setTarg(INVALID_SERIAL);
			pAttacker->resetAttackFirst();

			// Send a warmode update
			pAttacker->resend( false );
			
/*			// Update the little button on the paperdoll
			pAttacker->setWar( false );
			if( pAttacker->socket() )
			{
				cUOTxWarmode warmode;
				warmode.setStatus( 0 );
				pAttacker->socket()->send( &warmode );
			}*/

			return;
		}

		// We can be sure here that pc_defender is a valid hit-target
		if( pAttacker->isNpc() || pAttacker->socket() )
		{	
/*			// Too far above or below us
			if( pAttacker->pos.z > ( pDefender->pos.z + 10 ) ) 
				return;

			if( pAttacker->pos.z < ( pDefender->pos.z - 10 ) ) 
				return;
			?? lets check lineofsight
			*/
			
			if( ( pDefender->isNpc() && pDefender->npcaitype() != 17 ) || ( pDefender->socket() && !pDefender->dead() ) )
			{
				if( !pAttacker->inRange( pDefender, SrvParams->attack_distance() ) )
				{
					// Guards to scotty-beam-me-up
					if( pAttacker->npcaitype() == 4 && pAttacker->inGuardedArea() )
					{
						pAttacker->removeFromView( false );
						pAttacker->moveTo( pDefender->pos );
						pAttacker->resend( false );
						
						pAttacker->soundEffect( 0x1FE );
						staticeffect( pAttacker, 0x37, 0x2A, 0x09, 0x06 );
						pAttacker->talk( tr("Halt, scoundrel!"), -1, 0, true );
					}
					else if( pAttacker->isNpc() ) // Any other NPC
					{
						pAttacker->setTarg(INVALID_SERIAL);
						pAttacker->setTimeOut(0);
						
						P_CHAR pc = FindCharBySerial( pAttacker->attacker() );
						if( pc )
						{
							pc->resetAttackFirst();
							pc->setAttacker(INVALID_SERIAL);
						}
						pAttacker->setAttacker(INVALID_SERIAL);
						pAttacker->resetAttackFirst();

						if( pAttacker->isNpc() && pAttacker->npcaitype() != 17 && !pAttacker->dead() && pAttacker->war() )
							pAttacker->toggleCombat();
					}
				}
				else
				{
					if( pAttacker->targ() == INVALID_SERIAL )
					{
						pDefender->fight( pAttacker );
						pDefender->setAttackFirst();
						pAttacker->fight( pDefender );
						pAttacker->resetAttackFirst();
						UI32 x = ( ( ( 100-pAttacker->effDex() ) * MY_CLOCKS_PER_SEC ) / 25 ) + ( 1 * MY_CLOCKS_PER_SEC ); //Yet another attempt.
						pAttacker->setTimeOut(uiCurrentTime + x);
						return;
					}
					if( isTimerOk( pAttacker ) )
					{
						bool los = lineOfSight( pAttacker->pos + Coord_cl( 0, 0, 13 ), pDefender->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING );
						// we only check for bows, so right hand is enough
						P_ITEM pRightItem = pAttacker->rightHandItem();
						UI16 fightskill = weaponSkill( pRightItem );
						enBowTypes bowtype = bowType( pRightItem );
						bool mayAttack = false;

						if( fightskill == ARCHERY )
						{
							if( los )
							{
								int arrowsquant;

								if( bowtype == BOW ) 
									arrowsquant = pAttacker->CountItems( 0x0F3F );
								else
									arrowsquant = pAttacker->CountItems( 0x1BFB );

								if( arrowsquant > 0 )
									mayAttack = true;
							}
						}
						else if( pAttacker->inRange( pDefender, 1 ) )
							mayAttack = true;
						
						if( mayAttack )
						{
							// - Do stamina maths -
							if( abs( SrvParams->attackstamina() ) > 0 && !pAttacker->isGM() )
							{
								if( ( SrvParams->attackstamina() < 0 ) && ( pAttacker->stm() < abs( SrvParams->attackstamina() ) ) )
								{
									if( pAttacker->socket() )
										pAttacker->socket()->sysMessage( tr( "You are too tired to attack." ) );
									
									setWeaponTimeout( pAttacker, pAttacker->leftHandItem() );
									setWeaponTimeout( pAttacker, pAttacker->rightHandItem() );
									return;
								}

								pAttacker->setStm( pAttacker->stm() + SrvParams->attackstamina() );
								if( pAttacker->stm() > pAttacker->effDex() )
									pAttacker->setStm( pAttacker->effDex() );
								
								if( pAttacker->stm() < 0 )
									pAttacker->setStm(0);

								// Send the changed stamina
								if( pAttacker->socket() )
									pAttacker->socket()->updateStamina();
							}
							
							doCombatAnimations( pAttacker, pDefender, fightskill, bowtype, los );

							if( ( pAttacker->inRange( pDefender, 2 ) || ( fightskill == ARCHERY ) ) && !( pAttacker->npcaitype()==4) )
							{
								if( los )
								{
									pDefender->fight( pAttacker );
									pDefender->setAttackFirst();
									pAttacker->fight( pDefender );
									pAttacker->resetAttackFirst();
								}
							}

							if( pAttacker->timeout2() > uiCurrentTime )
								return;

							// A tempeffect is needed here eventually
							// An Arrow doesnt hit its target immedeately..
							if( fightskill == ARCHERY )
								hit( pAttacker, pDefender, los );
							else
								pAttacker->setSwingTarg( pDefender->serial );
						}

						setWeaponTimeout( pAttacker, pRightItem );
						setWeaponTimeout( pAttacker, pAttacker->leftHandItem() );
						pAttacker->setTimeOut2(pAttacker->timeout()); // set shotclock memory

						if( !pDefender->isInvul() )
						{
	//						NpcSpellAttack( pAttacker,pDefender,currenttime,los );
						}

/*						if( fightskill != ARCHERY)
						{
							hit( pAttacker, pDefender, los );
						}*/
					}
				}			

				// Our target finally died.
				if( pDefender->hp() < 1 ) //Highlight //Repsys
				{
					if( ( pAttacker->npcaitype() == 4 || pAttacker->npcaitype() == 9 ) && pDefender->isNpc() )
					{
						pDefender->action( 0x15 );					
						PlayDeathSound( pDefender );					
						cCharStuff::DeleteChar( pDefender ); //Guards, don't give body
					}
					else
					{
						pDefender->kill();
					}
					
					//murder count \/				
					if( ( pAttacker->isPlayer() ) && ( pDefender->isPlayer() ) ) //Player vs Player
					{
						if( pDefender->isInnocent() && GuildCompare( pAttacker, pDefender ) == 0 )
						{
							pAttacker->setKills( pAttacker->kills() + 1 );

							if( pAttacker->socket() )
							{
								pAttacker->socket()->sysMessage( tr( "You have killed %1 innocent people." ).arg( pAttacker->kills() ) );

								if( pAttacker->kills() == SrvParams->maxkills() + 1 )
									pAttacker->socket()->sysMessage( tr("You are now a murderer!") );
							}
						}
						
						if( SrvParams->pvpLog() )
						{
							sprintf((char*)temp,"%s was killed by %s!\n",pDefender->name.latin1(), pAttacker->name.latin1());
							savelog((char*)temp,"PvP.log");
						}
					}

					if( pAttacker->isNpc() )
						pAttacker->toggleCombat();
				}
			}
		}
	}

	// Formulas take from OSI's combat formulas
	// attack speed should be determined here.
	// attack speed = 15000 / ((DEX+100) * weapon speed)
	void setWeaponTimeout( P_CHAR pAttacker, P_ITEM pWeapon )
	{
		UI32 x,j;
		
		if( pWeapon ) 
		{ 
			if( pWeapon->speed() == 0 ) 
				pWeapon->setSpeed( 35 );

			// Attack Speed= 15,000 ÷ ( [Stamina +100] x Weapon Speed )
			x = (15000*MY_CLOCKS_PER_SEC) / ((pAttacker->stm()+100) * pWeapon->speed() ); 
		}
		else 
		{
			if( pAttacker->skill(WRESTLING) > 200 ) j = 35;
			else if( pAttacker->skill(WRESTLING) > 400 ) j = 40;
			else if( pAttacker->skill(WRESTLING) > 600 ) j = 45;
			else if( pAttacker->skill(WRESTLING) > 800 ) j = 50;
			else j = 30;
			x = (15000*MY_CLOCKS_PER_SEC) / ((pAttacker->effDex()+100) * j);
		}
		pAttacker->setTimeOut(uiCurrentTime + x);
	}

	void doCombatAnimations( P_CHAR pAttacker, P_CHAR pDefender, UI16 fightskill, enBowTypes bowtype, bool los )
	{
		// make sure attacker is facing the right direction
		pAttacker->turnTo( pDefender );

		UINT16 id = pAttacker->id();


		// Monsters receive special treatment
		if( id < 0x0190 )
		{
			UINT8 action = 4 + RandomNum( 0, 2 ); // some creatures dont have animation #4
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

			pAttacker->action( action );
			playmonstersound( pAttacker, pAttacker->id(), SND_ATTACK );
		}
		else if (pAttacker->atLayer(cChar::Mount))
		{
			doHorseCombatAnimation( pAttacker ); // determines weapon in hand and runs animation
		}
		else
		{
			doFootCombatAnimation( pAttacker );	// determines weapon in hand and runs animation kolours
		}

		// Show flying arrows if archery was used
		if( fightskill == ARCHERY )
		{
			if( los )
			{
				if( bowtype == BOW )
				{
					P_ITEM pBackpack = pAttacker->getBackpack();
					pBackpack->DeleteAmount( 1, 0x0F3F );
					movingeffect3( pAttacker, pDefender, 0x0F, 0x42, 0x08, 0x00, 0x00,0,0,0,0 );
				}
				else
				{
					P_ITEM pBackpack = pAttacker->getBackpack();
					pBackpack->DeleteAmount( 1, 0x1BFB );
					movingeffect3( pAttacker, pDefender, 0x1B, 0xFE, 0x08, 0x00, 0x00,0,0,0,0 );
				}
			}
		}
	}

	bool isTimerOk( P_CHAR pc )
	{
		if( !pc )
			return false;

		if( ( pc->timeout() < uiCurrentTime ) && ( pc->timeout2() < uiCurrentTime ) ) 
			return true;
	
		return false;
	}

	// play animation for weapon in hand during combat on a horse
	void doHorseCombatAnimation( P_CHAR pc )
	{
		if( !pc ) 
			return;

		P_ITEM pWeapon = pc->rightHandItem();
		if( !pWeapon )
			pWeapon = pc->leftHandItem();

		P_ITEM pHorseItem = pc->GetItemOnLayer( 0x19 );	

		int base;
		switch( pHorseItem->id() & 0x00FF )
		{
			case 0xab: // llamas
			case 0xa6:
				base = 0x2b;
				break;

			case 0xa3:  // ostards 
			case 0xa4:
			case 0xa5:
			case 0xac:
				base = 0x35;
				break;
			case 0xad: // kirin
				base = 0x3f;
				break;


			case 0xb3: // sea horse
				base = 0x49;
				break;
			case 0xb8: //ridgeback
				base = 0x53;
				break;
			default:
				base = 0x17; // default (horse) base animation
				break;
		}
		if( pWeapon )
		{
			short id = pWeapon->id();


			if( IsBow( id ) )
				pc->action( 0x1B );
			else if( IsCrossbow( id ) || IsHeavyCrossbow( id ) )
				pc->action( 0x1C );
			else if( IsSword( id ) || IsMaceType( id ) ||
				( id == 0x0FB4 || id == 0x0FB5 ) || // sledge hammer
				IsFencing1H( id ) )
				pc->action( 0x1A );
			else if( IsAxe( id ) || IsFencing2H( id ) )
				pc->action( 0x1D );
		}
		else
		{
			pc->action( 0x1A );
		}
	}

	// play animation for weapon in hand for combat on foot
	void doFootCombatAnimation( P_CHAR pc )
	{
		if( !pc )
			return;

		P_ITEM pWeapon = pc->rightHandItem();
		if( !pWeapon )
			pWeapon = pc->leftHandItem();

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

	// play the "MISSED" sound effect
	void playMissedSoundEffect( P_CHAR pChar )
	{
		UI16 soundId = hex2dec( DefManager->getRandomListEntry( "SOUNDS_COMBAT_MISSED" ) ).toUShort();
		if( soundId == 0 )
		{
			switch( RandomNum( 1, 3 ) )
			{
			case 1: soundId = 0x238; break;
			case 2: soundId = 0x239; break;
			case 3: soundId = 0x23A; break;
			};
		}

		pChar->soundEffect( soundId );
	}

	void playSoundEffect( P_CHAR pc, UI16 fightskill, P_ITEM pWeapon )
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

		switch( fightskill )
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

	void spawnGuard( P_CHAR pOffender, P_CHAR pCaller, const Coord_cl &pos )
	{
		if( !pOffender || !pCaller )
			return;
		
		if( pOffender->dead() || pCaller->dead())
			return;

		cTerritory* pRegion = pCaller->region();

		if( pRegion == NULL )
			return;
		
		if( pRegion->isGuarded() && SrvParams->guardsActive() && !pOffender->isInvul() )
		{
			P_CHAR pGuard = cCharStuff::createScriptNpc( pRegion->getGuardSect(), pos );
			
			if ( !pGuard ) 
				return;
			
			pGuard->setNpcAIType( 4 );
			pGuard->setAttackFirst();
			pGuard->setAttacker(pOffender->serial);
			pGuard->setTarg(pOffender->serial);
			pGuard->setNpcWander(2);  // set wander mode
			pGuard->toggleCombat();
			pGuard->setNextMoveTime();
			pGuard->setSummonTimer( ( uiCurrentTime + (MY_CLOCKS_PER_SEC*25) ) );    
			
			pGuard->soundEffect( 0x1FE );
			staticeffect( pGuard, 0x37, 0x2A, 0x09, 0x06 );

			pGuard->resend( false ); // Update warmode status to other players

			// 50% talk chance
			switch( RandomNum( 0, 4 ) )
			{
				case 0:		pGuard->talk( tr( "Thou shalt regret thine actions, swine!" ), -1, 0, true );	break;
				case 1:		pGuard->talk( tr( "Death to all Evil!" ), -1, 0, true );						break;
			}
		}
	}
}

#pragma note("convert old cCombat::ItemCastSpell to python(?)")
/* ==> PYTHON
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
}*/

#pragma note("Reimplement NpcSpellAttack with cMagic!")
/*
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
*/
/*

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
    }*//*
}


*/