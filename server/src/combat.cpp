//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001-2003 by holders identified in authors.txt
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
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================

// Platform Includes
#include "platform.h"

// Wolfpack Includes

#include "wolfpack.h"
#include "itemid.h"
#include "utilsys.h"
#include "guildstones.h"
#include "mapobjects.h"
#include "combat.h"
#include "srvparams.h"
#include "skills.h"
#include "network.h"
#include "network/uosocket.h"
#include "network/uotxpackets.h"
#include "territories.h"
#include "typedefs.h"
#include "wpdefmanager.h"
#include "basechar.h"
#include "player.h"
#include "npc.h"
#include "chars.h"

#include "debug.h"

// System Includes
#include <math.h>

#undef  DBGFILE
#define DBGFILE "combat.cpp"

#define SWINGAT (unsigned int)1.75 * MY_CLOCKS_PER_SEC // changed from signed to unsigned, LB

namespace Combat 
{
	/*!
		Play the sound effect for a miss.
	*/
	void playMissedSoundEffect( P_CHAR pChar, UINT16 skill )
	{
		UINT16 id = 0;

		switch( skill )
		{
		case ARCHERY:			
			id = RandomNum( 0, 1 ) ? 0x233 : 0x238;
			break;

		// Wrestling Sounds are creature-dependant
		case WRESTLING:
			if( !pChar->isHuman() )
			{
				id = creatures[ pChar->bodyID() ].basesound + RandomNum( 0, 1 );
				break;
			}

		default:
			switch( RandomNum( 0, 2 ) )
			{
			case 0:
				id = 0x238;
				break;
			case 1:
				id = 0x239;
				break;
			case 2:
				id = 0x23a;
				break;
			}
			break;
		}

		pChar->soundEffect( id );
	}

	/*!
		Play the sound effect for a hit.
	*/
	void playSoundEffect( P_CHAR pChar, UINT16 skill, P_ITEM pWeapon )
	{
		UINT16 id = 0;

		if( !pWeapon )
			skill = WRESTLING;

		switch( skill )
		{
		case SWORDSMANSHIP:
			// Special 2 handed weapons
			if( pWeapon->twohanded() )
			{
				id = RandomNum( 0, 1 ) ? 0x236 : 0x237;
				break;
			}
		// We fall trough for normal swords here intentionally
		case FENCING:
			id = RandomNum( 0, 1 ) ? 0x23b : 0x23c;
			break;

		case ARCHERY:
			id = 0x234;
			break;

		case MACEFIGHTING:
			// This depends on the type of the mace
			// Warhammers have different sounds than staffs

			if( pWeapon->type() == 1003 ) // Staffs
				id = 0x233;
			else if( pWeapon->type() == 1004 ) // Maces/WarHammers
				id = 0x232;
			break;
		}
		
		// Use Wrestling Sounds
		if( !id )
		{
			switch( RandomNum( 0, 2 ) )
			{
				case 0:
					id = 0x135;
					break;
				case 1:
					id = 0x137;
					break;
				case 2:
					id = 0x13b;
					break;
			}
		}

		pChar->soundEffect( id );
	}

	/*!
		Play the soundeffect for getting hit.
	*/
	void playGetHitSoundEffect( P_CHAR pChar )
	{
		if( pChar->bodyID() == 0x191 )
		{
			UI16 sound = hex2dec( DefManager->getRandomListEntry( "SOUNDS_COMBAT_HIT_HUMAN_FEMALE" ) ).toUShort();
			if( sound > 0 )
				pChar->soundEffect( sound );
			else
				pChar->soundEffect( 0x14b );
		}
		else if( pChar->bodyID() == 0x190 )
		{
			UI16 sound = hex2dec( DefManager->getRandomListEntry( "SOUNDS_COMBAT_HIT_HUMAN_MALE" ) ).toUShort();
			if( sound > 0 )
				pChar->soundEffect( sound );
			else
				pChar->soundEffect( 0x156 );
		}
		else	
			playmonstersound( pChar, pChar->bodyID(), SND_DEFEND );
	}

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
			case 1006:				return BOW;
			case 1007:				return XBOW;
			default:				return INVALID_BOWTYPE;
		}
	}

	/*!
		This is called when we finish to swing our weapon.
		It basically checks for LoS and distance
		and then calls hit().
	*/
	void checkandhit( P_CHAR pAttacker )
	{
		P_CHAR pDefender = FindCharBySerial( pAttacker->swingTarget() );
		
		// We made our swing, so reset the target.
		pAttacker->setSwingTarget( INVALID_SERIAL );

		if( !pDefender || pDefender->isInvulnerable() )
		{
			if( pAttacker->objectType() == enPlayer )
			{
				P_PLAYER pp = dynamic_cast<P_PLAYER>(pAttacker);
				if( pp->socket() )
					pp->socket()->sysMessage( tr( "You can't hit your target." ) );
			}

			return;
		}

		if( pDefender->isDead() )
		{
			pAttacker->setSwingTarget( INVALID_SERIAL );
			pAttacker->setCombatTarget( INVALID_SERIAL );
			pAttacker->setNextHitTime( 0 );
			pAttacker->setAttackFirst( false );
			
			// Reset the target
			if( pAttacker->objectType() == enPlayer )
			{
				P_PLAYER pp = dynamic_cast<P_PLAYER>(pAttacker);
				if( pp->socket() )
				{
					cUOTxAttackResponse aResponse;
					aResponse.setSerial( INVALID_SERIAL );
					pp->socket()->send( &aResponse );
				}
			}

			return;
		}

		if( pDefender->isHidden() )
			return;

		// Can we see our target. 
		// I don't know what the +z 13 means...
		bool los = lineOfSight( pAttacker->pos() + Coord_cl( 0, 0, 13 ), pDefender->pos(), WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING );

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
		UINT16 oldStm = pDefender->stamina();

		// Get the weapon the attacker is wearing.
		P_ITEM pWeapon = pAttacker->getWeapon();
		UINT16 wSkill = pWeapon ? pWeapon->getWeaponSkill() : WRESTLING;
		enBowTypes bowtype = bowType( pWeapon );

		// We simply can't see our target.
		if( !los || ( wSkill != ARCHERY && pAttacker->dist( pDefender ) > 1 ) )
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
				if( pAttacker->objectType() == enPlayer )
				{
					P_PLAYER pp = dynamic_cast<P_PLAYER>(pAttacker);
					if( pp->socket() )
					{
						pp->socket()->sysMessage( tr( "You are out of ammunition!" ) );
					}
				}

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
		UINT16 dEvasion = QMIN( 120, pDefender->skillValue( dSkill ) / 10 ); // Up to 120

		// If we are unarmed there could be a chance that we can 
		// evade the blow by using EVALINT + ANATOMY
		if( dSkill == WRESTLING )
			dEvasion = QMAX( dEvasion, QMIN( 120, ( pDefender->skillValue( EVALUATINGINTEL ) + pDefender->skillValue( ANATOMY ) + 200 ) / 20 ) );

		// Now we have to calculate hitChance
		// Hit Chance = ( Attacker's Combat Ability + 50 ) ÷ ( [Defender's Combat Ability + 50] x 2 )
		double hitChance = ( pAttacker->skillValue( wSkill ) / 10 ) + 50;
		hitChance /= ( dEvasion + 50 ) * 2;
		hitChance = hitChance * 100;

		// Check if we missed
		if( RandomNum( 1, 100 ) > hitChance )
		{
			// Display a message to both players that the 
			// swing didn't hit
			// NOTE: There should be a random chance that this
			// message appears *or* a flag to set
			if( pAttacker->objectType() == enPlayer )
			{
				P_PLAYER pp = dynamic_cast<P_PLAYER>(pAttacker);
				if( pp->socket() )
					pp->socket()->sysMessage( tr( "You miss %1" ).arg( pDefender->name() ) );
			}

			if( pDefender->objectType() == enPlayer )
			{
				P_PLAYER pp = dynamic_cast<P_PLAYER>(pDefender);
				if( pp->socket() )
					pp->socket()->sysMessage( tr( "%1 misses you" ).arg( pAttacker->name() ) );
			}

			// If we missed using a Bow or Crossbow we 
			// could leave the ammunition at the feets of 
			// our target. 
			if( wSkill == ARCHERY )
			{
				if( pAmmo && RandomNum( 1, 3 ) == 1 ) // 1/3 chance
				{
					pAmmo->moveTo( pDefender->pos() );
					pAmmo->setPriv( 1 );
					pAmmo->startDecay();
					pAmmo->update();
				}
			}

			// Play the Miss Soundeffect
			playMissedSoundEffect( pAttacker, wSkill );

			return;
		}

		// If we used a poisoned Weapon to deal 
		// damage, apply the poison here
		if( pWeapon && ( pWeapon->poisoned() > 0 ) )
		{
			   pDefender->setPoisoned( pWeapon->poisoned() );

			   // a lev.1 poison takes effect after 40 secs, a isDeadly pois.(lev.4) takes 40/4 secs
			   pDefender->setPoisonTime( uiCurrentTime + ( MY_CLOCKS_PER_SEC * ( 40 / pDefender->poisoned() ) ) );
			   pDefender->setPoisonWearOffTime( pDefender->poisonTime() + ( MY_CLOCKS_PER_SEC * SrvParams->poisonTimer() ) );
		}

		// If we item we used was enchantet using 
		// some kind of spell, it's now time to
		// apply the spell effect. This feature
		// can only be used when magic has been
		// rewritten.

		// Calc base damage
		double damage;

		P_NPC pn = dynamic_cast<P_NPC>(pAttacker);

		// If we wear a weapon, use that value,
		if( pWeapon )
			damage = RandomNum( pWeapon->lodamage(), pWeapon->hidamage() );

		// Modify the damaged based on the weapon skill
		// I did not find this in the OSI specs
		//damage = (SI32)ceil( (float)damage * (float)pAttacker->skillValue( wSkill ) / 1000.0f );
		
		// Fall back to minDamage/maxDamage
		else if( pAttacker->objectType() == enNPC && pn->minDamage() != 0 && pn->maxDamage() != 0 )
		{
			// set basedamage to random value of weapon lo and maxDamage
			damage = RandomNum( pn->minDamage(), pn->maxDamage() );
		}

		// otherwise use the WRESTLING skill
		else
			damage = RandomNum( 0, QMAX( 1, pAttacker->skillValue( WRESTLING ) / 50 ) );

		// Boni to damage:

		// Tactics (% of Base Damage is Tactics + 50)
		damage = damage * ( pAttacker->skillValue( TACTICS ) / 10 + 50 ) / 100;

		// Strength (add 1/5 of strength * damage)
		damage += damage * ( pAttacker->strength() / 500 );

		// Anatomy (add 1/5 of anatomy * damage)
		damage += damage * ( pAttacker->skillValue( ANATOMY ) / 500 );

		// Anatomy GM gives another +10%
		if( pAttacker->skillValue( ANATOMY ) >= 1000 )
			damage += damage * 0.10;
			
		// If we are using an axe then add the lumberjacking bonus
		if( pWeapon && pWeapon->type() == 1002 )
		{
			// Lumberjacking (add 1/5 of lumberjacking * damage)
			damage += damage * ( pAttacker->skillValue( LUMBERJACKING ) / 500 );

			// Lumberjacking GM gives another +10%
			if( pAttacker->skillValue( LUMBERJACKING ) >= 1000 )
				damage += damage * 0.10;
		}		

		// Parrying with shield
		P_ITEM pShield = pDefender->leftHandItem();
		if( pShield && pShield->type() == 1008 )
		{
			// Do a parry check depending on the attackers skill
			pDefender->checkSkill( PARRYING, 0, pAttacker->skillValue( wSkill ) );

			// % Chance of Blocking = Parrying Skill ÷ 2
			if( pDefender->skillValue( PARRYING ) / 2 >= RandomNum( 0, 1000 ) )
			{
				// We successfully parried the blow
				if( pAttacker->objectType() == enPlayer )
				{
					P_PLAYER pp = dynamic_cast<P_PLAYER>(pAttacker);
					if( pp->socket() )
						pp->socket()->sysMessage( tr( "Your attack has been parried!" ) );
				}

				if( pDefender->objectType() == enPlayer )
				{
					P_PLAYER pp = dynamic_cast<P_PLAYER>(pDefender);
					if( pp->socket() )
						pp->socket()->sysMessage( tr( "You parried the blow!" ) );
				}

				if( wSkill != ARCHERY )
					damage -= pShield->def();
				else
					damage -= pShield->def() / 2;
			}
		}

		// Determine the body part we hit
		UINT32 bodyHit = RandomNum( 0, 100 );

		enBodyParts bodyPart;

		if( bodyHit <= 14 )
			bodyPart = LEGS;
		else if( bodyHit <= 58 )
			bodyPart = BODY;
		else if( bodyHit <= 72 )
			bodyPart = ARMS;
		else if( bodyHit <= 79 )
			bodyPart = HANDS;
		else if( bodyHit <= 86 )
			bodyPart = NECK;
		else
			bodyPart = HEAD;

		// Damage Absorbed: Random value between of 1/2 AR to full AR of Hit Location's piece of armor.
		// Calculate the AR rating for this spot of the body
		UINT16 ar = pDefender->calcDefense( bodyPart, true );
		damage -= RandomNum( ar / 2, ar );
		
		// When we reached this point, nothing will affect the damage anymore.
		// If Damage <= 0 that does NOT mean the defender parried the blow or
		// did something else.. It just means our hit stopped at the armor of
		// the defender. This does not mean we did a miss or something.

		QString defMessage, attMessage;

		// Define all messages for hit-areas

		static const char* hitLegs[] = {
			QT_TRANSLATE_NOOP("CombatHitMessages", "%1 hits you in the left thigh!"),		QT_TRANSLATE_NOOP("CombatHitMessages", "You hit %1 in the left thigh!"),
			QT_TRANSLATE_NOOP("CombatHitMessages", "%1 hits you in the right thigh!"),		QT_TRANSLATE_NOOP("CombatHitMessages", "You hit %1 in the right thigh!"),
			QT_TRANSLATE_NOOP("CombatHitMessages", "%1 hits you in the groin!"),			QT_TRANSLATE_NOOP("CombatHitMessages", "You hit %1 in the groin!")
		};

		static const char* hitBody[] = {
			QT_TRANSLATE_NOOP("CombatHitMessages", "%1 hits you in your chest!"),			QT_TRANSLATE_NOOP("CombatHitMessages", "You hit %1 in the chest!"),
			QT_TRANSLATE_NOOP("CombatHitMessages", "%1 lands a blow to your stomach!"),		QT_TRANSLATE_NOOP("CombatHitMessages", "You land a blow to %1's stomach!"),
			QT_TRANSLATE_NOOP("CombatHitMessages", "%1 hits you in your ribs!"),			QT_TRANSLATE_NOOP("CombatHitMessages", "You hit %1 in the ribs!")
		};

		static const char* hitBodyHard[] = {
			QT_TRANSLATE_NOOP("CombatHitMessages", "%1 lands a terrible blow to your chest!"),	QT_TRANSLATE_NOOP("CombatHitMessages", "You land a terrible blow to %1's chest!"),
			QT_TRANSLATE_NOOP("CombatHitMessages", "%1 knocks the wind out of you!"),			QT_TRANSLATE_NOOP("CombatHitMessages", "You knock the wind out of %1!"),
			QT_TRANSLATE_NOOP("CombatHitMessages", "%1 has broken your rib!"),					QT_TRANSLATE_NOOP("CombatHitMessages", "It sounds as if you have broken one of %1's ribs!")
		};

		static const char* hitArms[] = {
			QT_TRANSLATE_NOOP("CombatHitMessages", "%1 hits you in your left arm!"),		QT_TRANSLATE_NOOP("CombatHitMessages", "You hit %1 in the left arm!"),
			QT_TRANSLATE_NOOP("CombatHitMessages", "%1 hits you in your right arm!"),		QT_TRANSLATE_NOOP("CombatHitMessages", "You hit %1 in the right arm!")
		};

		static const char* hitHands[] = {
			QT_TRANSLATE_NOOP("CombatHitMessages", "%1 hits you at your left hand!"),		QT_TRANSLATE_NOOP("CombatHitMessages", "You hit %1 at the left hand!"),
			QT_TRANSLATE_NOOP("CombatHitMessages", "%1 hits you at your right hand!"),		QT_TRANSLATE_NOOP("CombatHitMessages", "You hit %1 at the right hand!")
		};

		static const char* hitNeck[] = {
			QT_TRANSLATE_NOOP("CombatHitMessages", "%1 hits you at the throat!"),			QT_TRANSLATE_NOOP("CombatHitMessages", "You hit %1 at the throat!"),
		};

		static const char* hitHead[] = {
			QT_TRANSLATE_NOOP("CombatHitMessages", "%1 hits you straight in the face!"),	QT_TRANSLATE_NOOP("CombatHitMessages", "You hit %1 straight in the face!"),
			QT_TRANSLATE_NOOP("CombatHitMessages", "%1 hits you at the head!"),				QT_TRANSLATE_NOOP("CombatHitMessages", "You hit %1 at the head!"),
			QT_TRANSLATE_NOOP("CombatHitMessages", "%1 hits you square in the jaw!"),		QT_TRANSLATE_NOOP("CombatHitMessages", "You hit %1 square in the jaw!")
		};

		static const char* hitHeadHard[] = {
			QT_TRANSLATE_NOOP("CombatHitMessages", "%1 lands a stunning blow to your head!"),	QT_TRANSLATE_NOOP("CombatHitMessages", "You land a stunning blow to %1's head!"),
			QT_TRANSLATE_NOOP("CombatHitMessages", "%1 smashed a blow across your face!"),		QT_TRANSLATE_NOOP("CombatHitMessages", "You smash a blow across %1's face!"),
			QT_TRANSLATE_NOOP("CombatHitMessages", "%1 lands a terrible hit to your temple!"),	QT_TRANSLATE_NOOP("CombatHitMessages", "You land a terrible hit at %1's temple!")
		};

		// Check if it has been a "hard" hit
		bool hardHit = ( pDefender->strength() * 0.1 <= damage );

		// Get the message for the defender and attacker
		switch( bodyPart )
		{
		case BODY:
			if( hardHit )
			{				
				defMessage = hitBody[ ( RandomNum( 0, 2 ) * 2 ) ];
				attMessage = hitBody[ ( RandomNum( 0, 2 ) * 2 ) + 1 ];
			}
			else
			{
				defMessage = hitBodyHard[ ( RandomNum( 0, 2 ) * 2 ) ];
				attMessage = hitBodyHard[ ( RandomNum( 0, 2 ) * 2 ) + 1 ];
			}
			break;

		case ARMS:
			defMessage = hitArms[ ( RandomNum( 0, 1 ) * 2 ) ];
			attMessage = hitArms[ ( RandomNum( 0, 1 ) * 2 ) + 1 ];
			break;

		case NECK:
			defMessage = hitNeck[0];
			attMessage = hitNeck[1];
			break;

		case HEAD:
			if( hardHit )
			{				
				defMessage = hitHead[ ( RandomNum( 0, 2 ) * 2 ) ];
				attMessage = hitHead[ ( RandomNum( 0, 2 ) * 2 ) + 1 ];
			}
			else
			{
				defMessage = hitHeadHard[ ( RandomNum( 0, 2 ) * 2 ) ];
				attMessage = hitHeadHard[ ( RandomNum( 0, 2 ) * 2 ) + 1 ];
			}
			break;

		case HANDS:
			defMessage = hitHands[ ( RandomNum( 0, 1 ) * 2 ) ];
			attMessage = hitHands[ ( RandomNum( 0, 1 ) * 2 ) + 1 ];
			break;

		case LEGS:
			defMessage = hitLegs[ ( RandomNum( 0, 2 ) * 2 ) ];
			attMessage = hitLegs[ ( RandomNum( 0, 2 ) * 2 ) + 1 ];
			break;
		}

		if( pAttacker->objectType() == enPlayer )
		{
			P_PLAYER pp = dynamic_cast<P_PLAYER>(pAttacker);
			if( pp->socket() )
				pp->socket()->sysMessage( qApp->translate("CombatHitMessages", attMessage.arg( pDefender->name() ) ) );
		}

		if( pDefender->objectType() == enPlayer )
		{
			P_PLAYER pp = dynamic_cast<P_PLAYER>(pDefender);
			if( pp->socket() )
				pp->socket()->sysMessage( qApp->translate("CombatHitMessages", defMessage.arg( pAttacker->name() ) ) );
		}

		// Macefighting Weapons (2handed only) 
		// Deal Stamina loss
		if( pWeapon && ( pWeapon->type() == 1003 || pWeapon->type() == 1004 ) && pWeapon->twohanded() )
			pDefender->setStamina( QMAX( 0, pDefender->stamina() - RandomNum( 3, 6 ) ) );

		// Paralyzing + Concussion Hit are disabled for now
		/*
		// Paralyzing
		if( ( fightskill == FENCING ) && ( IsFencing2H( pItem->id() ) ) && pDefender->isPlayer() )
		{ 
			tempeffect( pAttacker, pDefender, 44, 0, 0, 0 );
			if( pAttacker->socket() )
				pAttacker->socket()->sysMessage( tr( "You delivered a paralyzing blow" ) );
		}

		// Concussion Hit
		if( ( fightskill == SWORDSMANSHIP ) && ( IsAxe( pItem->id() ) ) && pDefender->isPlayer() )
			tempeffect( pAttacker, pDefender, 45, 0, 0, 0 );
		*/
	
		// We hit our target,
		// So let's play a soundeffect for our hit
		playSoundEffect( pAttacker, wSkill, pWeapon );

		// attacker poisons defender
		// and vice versa
//		pAttacker->applyPoison( pDefender );
//		pDefender->applyPoison( pAttacker );

		// Cancel an eventual paralyze spell
//		if( ( pDefender->effDex() > 0 ) )
//			pDefender->setPriv2( pDefender->priv2() & 0xFD );

		// when hitten and damage >1, defender fails if casting a spell!
		// Thats not really good, better make a check versus int+magic
		/*if( damage > 1 && pDefender->isPlayer() )//only if damage>1 and against a player
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
		}*/

		// Reactive Armor
		if( pDefender->hasReactiveArmor() )
		{
			// lets reflect (MAGERY/2)% of the damage
			SI32 reflectdamage = damage * pDefender->skillValue( MAGERY ) / 2000;
			damage -= reflectdamage;

			if( reflectdamage > 0 )
			{
				pAttacker->setHitpoints( QMAX( 0, pAttacker->hitpoints() - reflectdamage ) );
				pAttacker->updateHealth();

				if( pAttacker->hitpoints() <= 0 )
					pAttacker->kill();
			}

			pDefender->effect( 0x374A, 10, 15 ); // RA effect (update!)
		}

		// Finally deal the damage
		damage = QMAX( 0, damage );
		if( damage > 0 )
		{
			pDefender->setHitpoints( pDefender->hitpoints() - damage );
			
			if( pAttacker->objectType() == enPlayer )
			{
				P_PLAYER pp = dynamic_cast<P_PLAYER>(pAttacker);
				if( pp->socket() )
					pp->socket()->showSpeech( pDefender, QString::number( damage ), 0x1F, 6 );
			}

			// If we die anyway dont update our health
			if( pDefender->hitpoints() > 0 )
				pDefender->updateHealth();
		}

		// Create blood below the defender if severe
		// damage has been dealt
		/*if( RandomNum( 1, 10 ) == 1 ) // 10% chance
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
		}*/

		/*
		//===== SPLITTING NPCS
		UI08 splitnum = 0;
		if( ( pDefender->split() > 0 ) && ( pDefender->hitpoints() >= 1 ) )
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
*/
		// Resend the health bar of our defender to all surrounding characters
		// ONLY if the hp really changed!
		if( pDefender->stamina() != oldStm && pDefender->objectType() == enPlayer )
		{
			P_PLAYER pp = dynamic_cast<P_PLAYER>(pDefender);
			if( pp->socket() )
				pp->socket()->sendStatWindow();
		}

		// Get Hit sound + Animation
		playGetHitSoundEffect( pDefender );
		// pDefender->action( );
	}

	/*!
		Prepare hitting an opponent. Do several checks for the target and
		check if we can hit the target instantly.
	*/
	void combat( P_CHAR pAttacker )
	{
		// We are either not fighting or dont have a target
		if( !pAttacker || pAttacker->free || pAttacker->isDead() || !pAttacker->isAtWar() || pAttacker->combatTarget() == INVALID_SERIAL )
			return;

		P_CHAR pDefender = FindCharBySerial( pAttacker->combatTarget() );

		// Check our Target
		if( !pDefender || pDefender->free || pDefender->isHidden() || pDefender->isDead() )
		{
			pAttacker->setNextHitTime(0);
			if( pDefender )
				pDefender->setAttackerSerial(INVALID_SERIAL);
			pAttacker->setCombatTarget(INVALID_SERIAL);
			pAttacker->setAttackFirst( false );

			// Reset the target
			if( pAttacker->objectType() == enPlayer )
			{
				P_PLAYER pp = dynamic_cast<P_PLAYER>(pAttacker);
				if( pp->socket() )
				{
					cUOTxAttackResponse aResponse;
					aResponse.setSerial( INVALID_SERIAL );
					pp->socket()->send( &aResponse );
				}
			}

			return;
		}

		// Some special stuff for guards
		if( pAttacker->objectType() == enNPC && !pAttacker->inRange( pDefender, SrvParams->attack_distance() ) )
		{
			// Guards beam to their target if they are out of range
#pragma note("reimplement with new npc ai!")
/*			if( pAttacker->npcaitype() == 4 && pDefender->inGuardedArea() )
			{
					pAttacker->removeFromView( false );
					pAttacker->moveTo( pDefender->pos() );
					pAttacker->resend( false );
					
					pAttacker->soundEffect( 0x1FE );
					pAttacker->effect( 0x372A, 0x09, 0x06 );

					if( RandomNum( 1, 4 ) == 1 )
						pAttacker->talk( tr("Halt, scoundrel!"), -1, 0, true );
			}
			// If we are out of close range, get out of warmode and stop fighting
			else if( pAttacker->npcaitype() != 4 )
			{
				pAttacker->setCombatTarget( INVALID_SERIAL );
				pAttacker->setSwingTarget( INVALID_SERIAL );
				pAttacker->setNextHitTime(0);
				
				P_CHAR pc = FindCharBySerial( pAttacker->attacker() );

				if( pc )
				{
					pc->setAttackFirst( false );
					pc->setAttackerSerial(INVALID_SERIAL);
				}

				pAttacker->setAttackerSerial(INVALID_SERIAL);
				pAttacker->setAttackFirst( false );				
				
				// We have been at war for sure, so we can
				// update without harm
				pAttacker->setWar( false );
				pAttacker->update();

				return;
			}
			*/
		}

		// We have two delay-timers for attacking
		// Timer for melee weapons and for 
		// ranged weapons. If we cannot attack again yet, 
		// we return from this function.
		if( !isTimerOk( pAttacker ) )
			return;
		
		P_ITEM pWeapon = pAttacker->getWeapon();
		bool mayAttack = false;
		
		// We only need to do LOS checks for archery. Otherwise we'll "compute" a new 
		// swing value. We only can hit them when we're standing right beside them either.
		if( pWeapon && pWeapon->getWeaponSkill() == ARCHERY )		
		{
			// Only shot if our "head" can see the opponent
			if( !lineOfSight( pAttacker->pos() + Coord_cl( 0, 0, 13 ), pDefender->pos(), WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING ) )
				mayAttack = false;
		}
		// For other Combat Skills it's enough to stand near the opponent
		else if( pAttacker->inRange( pDefender, 1 ) )
			mayAttack = true;

		// If we cannot attack let us quit this function
		if( !mayAttack )
			return;

		// Let the defender fight back now that we are swinging at him.
		if( pDefender->objectType() == enPlayer && pDefender->combatTarget() == INVALID_SERIAL )
		{
			pDefender->fight( pAttacker );
		}

		// Attacking costs a certain amount of stamina
		if( pAttacker->objectType() == enPlayer )
		{
			P_PLAYER pp = dynamic_cast<P_PLAYER>(pAttacker);

			if( abs( SrvParams->attackstamina() ) > 0 && !pp->isGM() )
			{
				// We don't have enough Stamina
				if( ( SrvParams->attackstamina() < 0 ) && ( pp->stamina() < abs( SrvParams->attackstamina() ) ) )
				{
					if( pp->socket() )
						pp->socket()->sysMessage( tr( "You are too tired to attack." ) );
					
					// Only re-check for missing stamina the next time 
					// we could try to hit them.
					setWeaponTimeout( pp, pp->leftHandItem() );
					setWeaponTimeout( pp, pp->rightHandItem() );
					return;
				}

				// Reduce the Stamina
				pp->setStamina( QMIN( pp->dexterity(), QMAX( 0, pp->stamina() + SrvParams->attackstamina() ) ) );
			}
		}
				
		// Show the Combat Animations
		doCombatAnimations( pAttacker, pDefender, pWeapon );

		// If we are no guard, our target should re-attack us.
		// (DarkStorm): But only if our Target is not fighting someone else
		/*if( pAttacker->npcaitype() != 4 && pDefender->combatTarget() == INVALID_SERIAL )
		{
			pDefender->fight( pAttacker );
			pDefender->setAttackFirstrength();
			pAttacker->fight( pDefender );
			pAttacker->setAttackFirst( false );
		}*/

		// A tempeffect is needed here eventually
		// An Arrow doesnt hit its target immedeately..
		if( pWeapon && pWeapon->getWeaponSkill() == ARCHERY )
			hit( pAttacker, pDefender, true ); // We are positive that we can hit our target
		else
			pAttacker->setSwingTarget( pDefender->serial() );

		// Set the time for the next attack
		setWeaponTimeout( pAttacker, pWeapon );

		// No Spellcasting monsters right now
		//if( !pDefender->isInvulnerable() )
		//{
		//	NpcSpellAttack( pAttacker,pDefender,currenttime,los );
		//}

		// Our target finally died.
		if( pDefender->hitpoints() < 1 ) // Highlight // Repsys
		{
#pragma note("reimplement with new npc ai sys")
/*			if( ( pAttacker->npcaitype() == 4 || pAttacker->npcaitype() == 9 ) && pDefender->isNpc() )
			{
				pDefender->action( 0x15 );					
				pDefender->playDeathSound();
				cCharStuff::DeleteChar( pDefender ); // Guards, don't give body
			}
			else
			{
				pDefender->kill();
				pAttacker->setSwingTarget( -1 );
			}
			*/
				
			// murder count \/				
			if( ( pAttacker->objectType() == enPlayer ) && ( pDefender->objectType() == enPlayer ) ) //Player vs Player
			{
				if( pDefender->isInnocent() )
				{
					pAttacker->setKills( pAttacker->kills() + 1 );

					P_PLAYER pp = dynamic_cast<P_PLAYER>(pAttacker);
					if( pp->socket() )
					{
						pp->socket()->sysMessage( tr( "You have killed %1 innocent people." ).arg( pAttacker->kills() ) );

						if( pp->kills() == SrvParams->maxkills() + 1 )
							pp->socket()->sysMessage( tr("You are now a murderer!") );
					}
				}
				
				if( SrvParams->pvpLog() )
				{
					sprintf((char*)temp,"%s was killed by %s!\n",pDefender->name().latin1(), pAttacker->name().latin1());
					savelog((char*)temp,"PvP.log");
				}
			}

			if( pAttacker->objectType() == enNPC && pAttacker->isAtWar() )
			{
				P_NPC pn = dynamic_cast<P_NPC>(pAttacker);
				pn->toggleCombat();
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
			x = (15000*MY_CLOCKS_PER_SEC) / ((pAttacker->stamina()+100) * pWeapon->speed() ); 
		}
		else 
		{
			if( pAttacker->skillValue(WRESTLING) > 200 ) j = 35;
			else if( pAttacker->skillValue(WRESTLING) > 400 ) j = 40;
			else if( pAttacker->skillValue(WRESTLING) > 600 ) j = 45;
			else if( pAttacker->skillValue(WRESTLING) > 800 ) j = 50;
			else j = 30;
			x = (15000*MY_CLOCKS_PER_SEC) / ((pAttacker->stamina()+100) * j);
		}
		pAttacker->setNextHitTime(uiCurrentTime + x);
	}

	void doCombatAnimations( P_CHAR pAttacker, P_CHAR pDefender, P_ITEM pWeapon )
	{
		// make sure attacker is facing the right direction
		pAttacker->turnTo( pDefender );

		UINT16 id = pAttacker->bodyID();

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
			playmonstersound( pAttacker, pAttacker->bodyID(), SND_ATTACK );
		}
		else if (pAttacker->atLayer(cBaseChar::Mount))
		{
			doHorseCombatAnimation( pAttacker ); // determines weapon in hand and runs animation
		}
		else
		{
			doFootCombatAnimation( pAttacker );	// determines weapon in hand and runs animation kolours
		}

		// Show flying arrows if archery was used
/*		if( fightskill == ARCHERY )
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
		}*/
	}

	bool isTimerOk( P_CHAR pc )
	{
		if( !pc )
			return false;

		if( pc->nextHitTime() < uiCurrentTime ) 
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

	// play the get hit animation
	void playGetHitAnimation( P_CHAR pChar )
	{
		// When we are not currently doing a swing against a target
		if( pChar->swingTarget() == INVALID_SERIAL )
			pChar->action( 0x14 );
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

	void spawnGuard( P_CHAR pOffender, P_CHAR pCaller, const Coord_cl &pos )
	{
		if( !pOffender || !pCaller )
			return;
		
		if( pOffender->isDead() || pCaller->isDead())
			return;

		cTerritory* pRegion = pCaller->region();

		if( pRegion == NULL )
			return;
		
		if( pRegion->isGuarded() && SrvParams->guardsActive() && !pOffender->isInvulnerable() )
		{
			P_NPC pGuard = cCharStuff::createScriptNpc( pRegion->getGuardSect(), pos );
			
			if ( !pGuard ) 
				return;
			
//			pGuard->setNpcAIType( 4 );
			pGuard->setAttackFirst( true );
			pGuard->setAttackerSerial(pOffender->serial());
			pGuard->setCombatTarget(pOffender->serial());
			pGuard->setWanderType( enCombatTarget );
			pGuard->toggleCombat();
			pGuard->setNextMoveTime();
			pGuard->setSummonTime( ( uiCurrentTime + (MY_CLOCKS_PER_SEC*25) ) );    
			
			pGuard->soundEffect( 0x1FE );
			pGuard->effect( 0x372A, 0x09, 0x06 );

			// Send guard to surrounding Players
			pGuard->resend( false );

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
	UINT16 tempmage = pAttacker->skillValue( MAGERY ); //Easier than writing new functions for all these spells

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

		if (!pc_defender->isDead() && chardist(pc_attacker, pc_defender) < SrvParams->attack_distance() && spattacks > 0 )
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
	vector<SERIAL> vecContainer = contsp.getData(Attacker->serial());
	for ( ci = 0; ci < vecContainer.size(); ci++)
	{
		pi = FindItemBySerial(vecContainer[ci]);
		if ( ( ( pi->layer() == 1 && pi->type() != 9 ) || (pi->layer() == 2 ) ) )
		{
			if( pi->offspell() && ( pi->att || pi->maxDamage() ) && pi->type() == 15 )
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
