//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2004 by holders identified in authors.txt
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

#include "itemid.h"
#include "sectors.h"
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
#include "log.h"
#include "player.h"
#include "npc.h"
#include "chars.h"
#include "basedef.h"
#include "items.h"
#include "inlines.h"
#include "world.h"
#include "basics.h"

// System Includes
#include <math.h>

#define SWINGAT (unsigned int)1.75 * MY_CLOCKS_PER_SEC // changed from signed to unsigned, LB

/*!
	Play the sound effect for a miss.
*/
void cCombat::playMissedSoundEffect( P_CHAR pChar, UINT16 skill )
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
			cCharBaseDef *def = BaseDefManager::instance()->getCharBaseDef( pChar->bodyID() );

			if( def != 0 )
				id = def->basesound() + RandomNum( 0, 1 );
			break;
		}

	default:
		id = RandomNum( 0x238, 0x23a );
		break;
	}

	if( id != 0 )
		pChar->soundEffect( id );
}

/*!
	Play the sound effect for a hit.
*/
void cCombat::playSoundEffect( P_CHAR pChar, UINT16 skill, P_ITEM pWeapon )
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
void cCombat::playGetHitSoundEffect( P_CHAR pChar )
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
		pChar->bark( cBaseChar::Bark_GetHit );
}

/*!

	Check for the weaponskill required by this weapon by checking it's type.

*/
UI16 cCombat::weaponSkill( P_ITEM pi )
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
cCombat::enBowTypes cCombat::bowType( P_ITEM pi )
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
void cCombat::checkandhit( P_CHAR pAttacker )
{
/*	P_CHAR pDefender = FindCharBySerial( pAttacker->swingTarget() );
	
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

	if (pDefender->isDead()) {
		pAttacker->fight(0);
		return;
	}

	if (pDefender->isHidden()) {
		return;
	}

	// Can we see our target. 
	bool los = pAttacker->pos().lineOfSight( pDefender->pos(), true );
	hit( pAttacker, pDefender, los );*/
}

/*!
	This function is responsible for calculating
	the to-hit chance and dealing the right 
	amount of damage. This is called *after* 
	we completed a swing.
*/
void cCombat::hit( P_CHAR pAttacker, P_CHAR pDefender, bool los )
{
	/*UINT16 oldStm = pDefender->stamina();

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
		// If we missed using a Bow or Crossbow we 
		// could leave the ammunition at the feets of 
		// our target. 
		if( wSkill == ARCHERY )
		{
			if( pAmmo && RandomNum( 1, 3 ) == 1 ) // 1/3 chance
			{
				pAmmo->moveTo( pDefender->pos() );
				pAmmo->update();
			}
		}

		// Play the Miss Soundeffect
		playMissedSoundEffect( pAttacker, wSkill );

		return;
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

	// Macefighting Weapons (2handed only) 
	// Deal Stamina loss
	if( pWeapon && ( pWeapon->type() == 1003 || pWeapon->type() == 1004 ) && pWeapon->twohanded() )
		pDefender->setStamina( QMAX( 0, pDefender->stamina() - RandomNum( 3, 6 ) ) );

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


	// Finally deal the damage
	damage = QMAX( 0, damage );

	if (damage > 0) {
		damage = pDefender->damage( DAMAGE_PHYSICAL, damage, pAttacker );

		if( !damage || pDefender->isDead() )
			return;
	}

	// We lost Stamina
	if( pDefender->stamina() != oldStm && pDefender->objectType() == enPlayer )
	{
		P_PLAYER pp = dynamic_cast<P_PLAYER>(pDefender);
		if( pp->socket() )
			pp->socket()->sendStatWindow();
	}*/
}

/*!
	Prepare hitting an opponent. Do several checks for the target and
	check if we can hit the target instantly.
*/
void cCombat::combat( P_CHAR pAttacker )
{
	// We are either not fighting or dont have a target
/*	if( !pAttacker || pAttacker->free || pAttacker->isDead() || !pAttacker->isAtWar() || pAttacker->combatTarget() == INVALID_SERIAL )
		return;

	P_CHAR pDefender = FindCharBySerial( pAttacker->combatTarget() );

	// Check our Target
	if (!pDefender || pDefender->free || pDefender->isHidden() || pDefender->isDead()) {
		pAttacker->fight(0);
		return;
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
		if( pAttacker->pos().lineOfSight( pDefender->pos() ) )
		{
			mayAttack = true;
		}
	}
	// For other Combat Skills it's enough to stand near the opponent
	else if( pAttacker->inRange( pDefender, 1 ) )
		mayAttack = true;

	// If we cannot attack let us quit this function
	if( !mayAttack )
		return;

	// Let the defender fight back now that we are swinging at him.
	if (pDefender->combatTarget() == INVALID_SERIAL) {
		pDefender->fight(pAttacker);
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

	// A tempeffect is needed here eventually
	// An Arrow doesnt hit its target immedeately..
	if( pWeapon && pWeapon->getWeaponSkill() == ARCHERY )
		hit( pAttacker, pDefender, true ); // We are positive that we can hit our target
	else
		pAttacker->setSwingTarget(pDefender->serial());

	// Set the time for the next attack
	setWeaponTimeout( pAttacker, pWeapon );

	// No Spellcasting monsters right now
	//if( !pDefender->isInvulnerable() )
	//{
	//	NpcSpellAttack( pAttacker,pDefender,currenttime,los );
	//}

	// Our target finally died.
	if( pDefender->isDead() ) // Highlight // Repsys
	{
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
		}

		Log::instance()->print( LOG_NOTICE, QString( "%1 was killed by %2\n" ).arg( pDefender->name() ).arg( pAttacker->name() ) );
		pAttacker->fight(0);
	}*/
}

// Formulas take from OSI's combat formulas
// attack speed should be determined here.
// attack speed = 15000 / ((DEX+100) * weapon speed)
void cCombat::setWeaponTimeout( P_CHAR pAttacker, P_ITEM pWeapon )
{
/*	UI32 x,j;
	
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
	pAttacker->setNextHitTime(uiCurrentTime + x);*/
}

void cCombat::doCombatAnimations( P_CHAR pAttacker, P_CHAR pDefender, P_ITEM pWeapon )
{
	/*// make sure attacker is facing the right direction
	pAttacker->turnTo( pDefender );

	UINT16 id = pAttacker->bodyID();

	// Monsters receive special treatment
	if( id < 0x0190 )
	{
		UINT8 action = 4 + RandomNum( 0, 2 ); // some creatures dont have animation #4
		cCharBaseDef *def = BaseDefManager::instance()->getCharBaseDef( id );

		if( def && def->flags() & 0x2 ) // anti blink bit set ?
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
		pAttacker->bark( cBaseChar::Bark_Hit );
	}
	else if (pAttacker->atLayer(cBaseChar::Mount))
	{
		doHorseCombatAnimation( pAttacker ); // determines weapon in hand and runs animation
	}
	else
	{
		doFootCombatAnimation( pAttacker );	// determines weapon in hand and runs animation kolours
	}
*/
}

bool cCombat::isTimerOk( P_CHAR pc )
{
	/*if( !pc )
		return false;

	if( pc->nextHitTime() < uiCurrentTime ) 
		return true;*/

	return false;
}

// play animation for weapon in hand during combat on a horse
void cCombat::doHorseCombatAnimation( P_CHAR pc )
{
/*	if( !pc ) 
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
	}*/
}

// play animation for weapon in hand for combat on foot
void cCombat::doFootCombatAnimation( P_CHAR pc )
{
/*	if( !pc )
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
	}*/
}

// play the get hit animation
void cCombat::playGetHitAnimation( P_CHAR pChar )
{
	// When we are not currently doing a swing against a target
	/*if( pChar->swingTarget() == INVALID_SERIAL )
		pChar->action( 0x14 );*/
}

// play the "MISSED" sound effect
void cCombat::playMissedSoundEffect( P_CHAR pChar )
{
/*	UI16 soundId = hex2dec( DefManager->getRandomListEntry( "SOUNDS_COMBAT_MISSED" ) ).toUShort();
	if( soundId == 0 )
	{
		switch( RandomNum( 1, 3 ) )
		{
		case 1: soundId = 0x238; break;
		case 2: soundId = 0x239; break;
		case 3: soundId = 0x23A; break;
		};
	}

	pChar->soundEffect( soundId );*/
}

void cCombat::spawnGuard( P_CHAR pOffender, P_CHAR pCaller, const Coord_cl &pos )
{
	if( !pOffender || !pCaller )
		return;
	
	if( pOffender->isDead() || pCaller->isDead())
		return;

	cTerritory* pRegion = pCaller->region();

	if( pRegion == NULL )
		return;
	
	if( pRegion->isGuarded() && SrvParams->guardsActive() )
	{
		QString guardsect = pRegion->getGuardSect();

		P_NPC pGuard = ( guardsect.isNull() ? NULL : cCharStuff::createScriptNpc( guardsect, pos ) );
		
		if ( !pGuard ) 
			return;
		
		// Send guard to surrounding Players
		pGuard->resend(false);			
		pGuard->soundEffect(0x1FE);
		pGuard->effect(0x372A, 0x09, 0x06);
		pGuard->fight(pOffender);
	}
}

cFightInfo::cFightInfo(P_CHAR attacker, P_CHAR victim, bool legitimate) {
	attacker_ = attacker;
	victim_ = victim;
	legitimate_ = legitimate;
	victimDamage_ = 0;
	attackerDamage_ = 0;
	lastaction_ = getNormalizedTime();

	// Register the fight info object with the combat system.
	attacker->fights().append(this);
	victim->fights().append(this);
	Combat::instance()->fights().append(this);
}

cFightInfo::~cFightInfo() {
	// Unregister the fight info object with the combat system.
	attacker_->fights().remove(this);
	victim_->fights().remove(this);
	Combat::instance()->fights().remove(this);
}

void cFightInfo::refresh() {
	lastaction_ = uiCurrentTime;
}
