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

#include "targetactions.h"
#include "srvparams.h"
//#include "classes.h"
#include "skills.h"
#include "prototypes.h"
#include "TmpEff.h"
#include "items.h"
#include "chars.h"
#include "basics.h"
#include "inlines.h"
#include "network/uorxpackets.h"
#include "network/uosocket.h"

bool cSkHealing::responsed( cUOSocket *socket, cUORxTarget *target )
{
	P_ITEM pBandage = FindItemBySerial( bandageSerial ); // item index of bandage

	P_CHAR pTarget = FindCharBySerial( target->serial() );

	if( !pTarget )
	{
		socket->sysMessage( tr( "Please select a human being or an animal to heal." ) );
		return true;
	}
		
	P_CHAR pHealer = socket->player();

	// Check for an ongoing fight
	if( !SrvParams->bandageInCombat() && ( pTarget->war() || pHealer->war() ) )
	{
		P_CHAR pAttacker = FindCharBySerial( pHealer->attacker() );
		if( pAttacker && pAttacker->war() )
		{
			socket->sysMessage( tr("You can`t heal while in a fight!") );
			return true;
		}
	}

	// Out of range?
	if( !pHealer->inRange( pTarget, 5 ) )
	{
		socket->sysMessage( tr("You are not close enough to apply the bandages.") );
		return true;
	}

	// Healing Criminals flags you criminal as well
	if( pHealer->isInnocent() && !pTarget->isInnocent() )
		pHealer->criminal();

	// Resurrecting
	if( pTarget->dead() )
	{
		if( pHealer->skillValue( HEALING ) < 800 || pHealer->skillValue( ANATOMY ) < 800 )
		{
			socket->sysMessage( tr( "You are not skilled enough to resurrect." ) );
			return true;
		}

		int reschance = static_cast<int>( ( pHealer->skillValue( HEALING ) + pHealer->skillValue( ANATOMY ) ) * 0.17 );
		int rescheck = RandomNum( 1, 100 );

		if( pHealer->checkSkill( HEALING, 800, 1000 ) && pHealer->checkSkill( ANATOMY, 800, 1000 ) && reschance <= rescheck )
			socket->sysMessage( tr( "You failed to resurrect the ghost." ) );
		else
		{
			pTarget->resurrect();
			socket->sysMessage( tr( "Because of your skill, you were able to resurrect the ghost." ) );
		}

		pBandage->ReduceAmount();

		return true;
	}
	
	// Cure Poison
	if( pTarget->poisoned() )
	{
		UINT8 firstSkill = HEALING;
		UINT8 secondSkill = ANATOMY;

		if( !pTarget->isHuman() )
		{
			firstSkill = VETERINARY;
			secondSkill = ANIMALLORE;
		}

		if( pHealer->skillValue( firstSkill ) < 600 || pHealer->skillValue( secondSkill ) < 600 )
		{
			socket->sysMessage( tr("You are not skilled enough to cure poison.") );
			return true;			
		}

		unsigned int curechance = static_cast<int>( ( pHealer->skillValue( firstSkill ) + pHealer->skillValue( secondSkill ) ) *0.67 );
		unsigned int curecheck = RandomNum( 1, 100 );
		pHealer->checkSkill( firstSkill, 600, 1000 );
		pHealer->checkSkill( secondSkill, 600, 1000 );
			
		if( curechance <= curecheck )
		{
			pTarget->setPoisoned( 0 );
			socket->sysMessage( tr( "Because of your skill, you were able to cure the poison." ) );
		}
		else
		{
			socket->sysMessage( tr( "You fail to cure the poison." ) );
		}

		pBandage->ReduceAmount();
		return true;
	}

	// Normal Healing
	if( pTarget->hp() == pTarget->st() )
	{
		socket->sysMessage( tr( "That being is not damaged." ) );
		return true;
	}

	// Healing Humans
	UINT8 firstSkill = HEALING;
	UINT8 secondSkill = ANATOMY;

	if( !pTarget->isHuman() )
	{
		firstSkill = VETERINARY;
		secondSkill = ANIMALLORE;
	}
		
	if( !pHealer->checkSkill( firstSkill, 0, 1000 ) )
	{
		socket->sysMessage( tr( "You apply the bandages, but they barely help!" ) );
		pTarget->setHp( pTarget->hp() + 1 );
	}
	else
	{
		unsigned int healmin = ( ( ( pHealer->skillValue( firstSkill ) / 5 ) + ( pHealer->skillValue( secondSkill ) / 5 ) ) + 3 );
		unsigned int healmax = ( ( ( pHealer->skillValue( firstSkill ) / 5 ) + ( pHealer->skillValue( secondSkill ) / 2 ) ) + 10 );
		unsigned int amount = RandomNum( healmin, healmax );
		
		// We don't heal over the maximum amount.
		if( pTarget->hp() + amount > pTarget->st() )
			amount = pTarget->st() - pTarget->hp();

		// Show the HitUnarmed Animation and Make the Effect delayed
		pHealer->action( 0x09 );
		cTempEffect *tEff = new cDelayedHeal( pHealer, pTarget, amount );
		tEff->setExpiretime_s( SrvParams->bandageDelay() );
		TempEffects::instance()->insert( tEff );
	}

	pHealer->setObjectDelay( SetTimerSec( pHealer->objectdelay(), SrvParams->objectDelay() + SrvParams->bandageDelay() ) );	
	pBandage->ReduceAmount();

	return true;
}

bool cSkLockpicking::responsed( cUOSocket *socket, cUORxTarget *target )
{
 	const P_ITEM pi = FindItemBySerial( target->serial() );
	P_CHAR pc_currchar = socket->player();
	if (pi && !pi->isLockedDown()) // Ripper
	{
		P_ITEM piPick = FindItemBySerial(lockPick);
		if (piPick == NULL)
			return true;
		if(pi->type()==1 || pi->type()==12 || pi->type()==63) 
		{
			socket->sysMessage( tr("That is not locked.") );
			return true;
		}
		
		if(pi->type()==8 || pi->type()==13 || pi->type()==64)
		{
			if(pi->more1()==0 && pi->more2()==0 && pi->more3()==0 && pi->more4()==0)
			{ //Make sure it isn't an item that has a key (i.e. player house, chest..etc)
				if(pc_currchar->checkSkill( LOCKPICKING, 0, 1000))
				{
					switch(pi->type())
					{
					case 8:  pi->setType( 1 );  break;
					case 13: pi->setType( 12 ); break;
					case 64: pi->setType( 63 ); break;
					default:
						LogError("switch reached default");
						return true;
					}
					pi->soundEffect( 0x241 );
				} 
				else
				{
					if((rand()%100)>50) 
					{
						socket->sysMessage( tr("You broke your lockpick!") );
						piPick->ReduceAmount( 1 );
					} 
					else
						socket->sysMessage( tr("You fail to open the lock.") );
				}
			} else
			{
				socket->sysMessage( tr("That cannot be unlocked without a key.") );
			}
		}
	}
	return true;
}

bool cDyeTarget::responsed( cUOSocket* socket, cUORxTarget *target )
{
	P_ITEM pi = FindItemBySerial(target->serial());
	if ( pi && pi->dye() == 1 )
	{
		P_CHAR pc = pi->getOutmostChar();
		if(pc == socket->player() || pi->isInWorld())
		{//if on ground or currchar is owning the item - AntiChrist
			pi->setColor( static_cast<unsigned short>( color ) );
			pi->update();
			socket->soundEffect( 0x023e, pi );
		} 
		else
		{
			socket->sysMessage( tr("That is not yours!!") );
		}
	}
	else
	{
		socket->sysMessage( tr("You can only dye cloth with this.") );
	}
	return true;
}
