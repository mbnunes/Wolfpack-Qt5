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

#include "Timing.h"

#include "walking.h"
#include "TmpEff.h"
#include "combat.h"
#include "mapobjects.h"
#include "srvparams.h"
#include "network.h"
#include "spawnregions.h"
#include "territories.h"
#include "skills.h"
#include "boats.h"
#include "house.h"
#include "typedefs.h"
#include "magic.h"
#include "itemid.h"
#include "basechar.h"
#include "npc.h"
#include "player.h"
#include "chars.h"
#include "ai.h"

// Library Includes
#include <qdatetime.h>
#include <math.h>

using namespace std;

#undef  DBGFILE
#define DBGFILE "Timing.cpp"

void checktimers() // Check shutdown timers
{
	register unsigned int tclock = uiCurrentTime;
	if (endtime)
	{
		if (endtime <= tclock) keeprun = 0;
	}
	lclock = tclock;
}

void restockNPC( UINT32 currenttime, P_NPC pc_i )
{
	if( SrvParams->shopRestock() && ( shoprestocktime <= currenttime ) )
	{
//		pc_i->restock();
	}
}

void checkRegeneration( P_CHAR pc, unsigned int currenttime )
{
	if( !pc )
		return;

	INT32 oldHealth = pc->hitpoints();
	INT32 oldStamina = pc->stamina();
	INT32 oldMana = pc->mana();

	// Regeneration stuff
	if( !pc->isDead() )
	{
		// Health regeneration
		if( pc->regenHitpointsTime() <= currenttime )
		{
			UINT32 interval = SrvParams->hitpointrate() * MY_CLOCKS_PER_SEC;

			// If it's not disabled hunger affects our health regeneration
			if( pc->hitpoints() < pc->maxHitpoints() && pc->hunger() > 3 || SrvParams->hungerRate() == 0 )
			{
				for( UINT16 c = 0; c < pc->maxHitpoints() + 1; ++c )
				{
					if( pc->regenHitpointsTime() + ( c * interval ) <= currenttime && pc->hitpoints() <= pc->maxHitpoints() )
					{
						if( pc->skillValue( HEALING ) < 500 )
							pc->setHitpoints( pc->hitpoints() + 1 );
					
						else if (pc->skillValue( HEALING ) < 800)
							pc->setHitpoints( pc->hitpoints() + 2 );

						else 
							pc->setHitpoints( pc->hitpoints() + 3 );

						if( pc->hitpoints() > pc->maxHitpoints() )
						{
							pc->setHitpoints( pc->maxHitpoints() );
							break;
						}
					}
				}
			}

			pc->setRegenHitpointsTime( currenttime + interval );
		}

		// Stamina regeneration
		if( pc->regenStaminaTime() <= currenttime )
		{
			UINT32 interval = SrvParams->staminarate()*MY_CLOCKS_PER_SEC;
			for( UINT16 c = 0; c < pc->maxStamina() + 1; ++c )
			{
				if (pc->regenStaminaTime() + (c*interval) <= currenttime && pc->stamina() <= pc->maxStamina())
				{
					if( pc->stamina() == pc->maxStamina() )
						break;
					else if( pc->stamina() > pc->maxStamina() )
					{
						pc->setStamina( pc->maxStamina() );
						break;
					}
					else
						pc->setStamina( pc->stamina() + 1 );
				}
			}
			pc->setRegenStaminaTime( currenttime + interval );			
		}

		// OSI Style Mana regeneration by blackwind
		// if (pc->in>pc->mn)  this leads to the 'mana not subtracted' bug (Duke)
		if( pc->regenManaTime() <= currenttime )
		{
			unsigned int interval = SrvParams->manarate()*MY_CLOCKS_PER_SEC;
			for( UINT16 c = 0; c < pc->maxMana() + 1 ; ++c )
			{
				if (pc->regenManaTime() + (c*interval) <= currenttime && pc->mana() <= pc->maxMana())
				{
					if (pc->mana()+1>pc->maxMana())
					{
						if ( pc->isMeditating() )
						{
							if( pc->objectType() == enPlayer )
							{
								P_PLAYER pp = dynamic_cast<P_PLAYER>(pc);
								if( pp->socket() )
									pp->socket()->sysMessage( tr("You are at peace." ) );
							}
							pc->setMeditating( false );
						}
						pc->setMana( pc->maxMana() );
						break;
					}
					else
						pc->setMana( pc->mana() + 1 );
				}
			}
			if( SrvParams->armoraffectmana() )
			{
				int ratio = ( ( 100 + 50 ) / SrvParams->manarate() );
				// 100 = Maximum skill (GM)
				// 50 = int affects mana regen (%50)
				int armorhandicap = ((Skills->GetAntiMagicalArmorDefence(pc) + 1) / SrvParams->manarate());
				int charsmeditsecs = (1 + SrvParams->manarate() - ((((pc->skillValue(MEDITATION) + 1)/10) + ((pc->intelligence() + 1) / 2)) / ratio));
				if (pc->isMeditating())
				{
					pc->setRegenManaTime( currenttime + ((armorhandicap + charsmeditsecs/2)* MY_CLOCKS_PER_SEC) );
				}
				else
					pc->setRegenManaTime( currenttime + ((armorhandicap + charsmeditsecs)* MY_CLOCKS_PER_SEC) );
			}
			else 
				pc->setRegenManaTime( currenttime + interval );
			}
			// end Mana regeneration
			
/* should be a temporal effect!
		// only if not permanently hidden
			if( ( pc->hidden() == 2 ) && ( pc->invisnextHitTime() <= currenttime ) && !pc->isHiddenPermanently() )
			{
				pc->setHidden( 0 );
				pc->setStealth(-1);
				pc->resend( false );
			}
*/
	}

	// Check if the character died
	if( pc->hitpoints() <= 0 && !pc->isDead() )
		pc->kill();

	if( pc->hitpoints() > pc->maxHitpoints() )
		pc->setHitpoints( pc->maxHitpoints() );

	if( pc->stamina() > pc->maxStamina() )
		pc->setStamina( pc->maxStamina() );

	if( pc->mana() > pc->maxMana() )
		pc->setMana( pc->maxMana() );

	// Now check if our Health, Stamina or Mana has changed
	if( oldHealth != pc->hitpoints() )
		pc->updateHealth();

	if( pc->objectType() == enPlayer )
	{
		P_PLAYER pp = dynamic_cast<P_PLAYER>(pc);
		if( pp->socket() )
		{
			if( oldHealth != pp->hitpoints() )
				pp->socket()->updateHealth(); // send it to the socket itself

			if( oldStamina != pp->stamina() )
				pp->socket()->updateStamina();

			if( oldMana != pp->mana() )
				pp->socket()->updateMana();
		}
	}
}

static int check_house_decay()
{
	int houses=0;   
	int decayed_houses=0;
	unsigned long int timediff;
	unsigned long int ct=getNormalizedTime();
	
	cItemIterator iter_items;
	P_ITEM pi;
	for( pi = iter_items.first(); pi; pi = iter_items.next() )
	{   
		if (!pi->free && IsHouse(pi->id()))
		{
			if (pi->time_unused>SrvParams->housedecay_secs()) // not used longer than max_unused time ? delete the house
			{          
				decayed_houses++;
				sprintf((char*)temp,"%s decayed! not refreshed for > %i seconds!\n",pi->name().ascii(), SrvParams->housedecay_secs());
				LogMessage((char*)temp);
				(dynamic_cast< cHouse* >(pi))->remove();
			}
			else // house ok -> update unused-time-attribute
			{
				timediff=(ct-pi->timeused_last)/MY_CLOCKS_PER_SEC;
				pi->time_unused+=timediff; // might be over limit now, but it will be cought next check anyway
				
				pi->timeused_last=ct;	// if we don't do that and housedecay is checked every 11 minutes,
				// it would add 11,22,33,... minutes. So now timeused_last should in fact
				// be called timeCHECKED_last. but as there is a new timer system coming up
				// that will make things like this much easier, I'm too lazy now to rename
				// it (Duke, 16.2.2001)
			}
			
			houses++;			
		}		
	}
	
	return decayed_houses;
}


void checkPC( P_PLAYER pc, unsigned int currenttime )
{
	cUOSocket *socket = pc->socket();

	// We are not logged in so dont check anything at all.
	if( !socket )
		return;
	unsigned int tempuint;
	signed short tempshort;
	int timer;//, valid=0;

	if ( pc == NULL ) return;

	// Check if the character is in a field which affects him
	Magic->CheckFieldEffects2( currenttime, pc, 1 ); 
	
	// We are not swinging 
	// So set up a swing target and start swinging
	if( !pc->isDead() && pc->swingTarget() == INVALID_SERIAL )
		Combat::combat( pc );

	// We are swinging and completed swinging
	else if( !pc->isDead() && ( pc->swingTarget() >= 0 && pc->nextHitTime() <= currenttime ) )
		Combat::checkandhit( pc );

	// Unmute players who have been muted for a certain amount of time
	if( pc->isMuted() )
	{
		if( pc->muteTime() && ( pc->muteTime() <= currenttime ) )
		{
			pc->setMuted( false );
			pc->setMuteTime( 0 );
			socket->sysMessage( tr( "You are no longer muted." ) );
		}
	}

	// Reputation System
	if( pc->criminalTime() > 0 && pc->criminalTime() <= currenttime  )
	{
		socket->sysMessage( tr( "You are no longer criminal" ) );
		pc->setCriminalTime(0);
	}

	if( pc->murdererTime() > 0 && pc->murdererTime() < currenttime )
	{
		if( pc->kills() > 0 )
			pc->setKills( pc->kills() - 1 );

		if ((pc->kills()<=SrvParams->maxkills())&&(SrvParams->maxkills()>0))
			socket->sysMessage( tr( "You are no longer a murderer." ) );
		else
			pc->setMurdererTime( ( SrvParams->murderdecay() * MY_CLOCKS_PER_SEC ) + currenttime );
	}

	setcharflag( pc );

/* still needed !?
	if( pc->isPlayer() && pc->casting() )//PC casting a spell
	{
		pc->setNextact( pc->nextact() - 1 );
		if( pc->spelltime() <= currenttime  )//Spell is complete target it
		{
			// TODO: Reactivate when spellcasting is redone
			//Magic->AfterSpellDelay( s, pc );
		}
		else if( pc->nextact() <= 0 )//redo the spell action
		{
			pc->setNextact( 75 );
			pc->action( pc->spellaction() );
		}
	}
*/

	if( SrvParams->bgSound() >= 1 )
	{
		timer = SrvParams->bgSound() * 100;
		if( timer == 0 )
			timer = 1;

		if( socket && !pc->isDead() && ( (rand()%(timer) ) == (timer/2))) 
			bgsound( pc );
	}

/*
	// Reset spirit-speak
	if( pc->spiritspeaktimer() > 0 && pc->spiritspeaktimer() <= uiCurrentTime )
		pc->setSpiritSpeakTimer(0);
*/
	
#pragma note("TODO: jail reimplementation with xml")
/*
	// Jail stuff
	if( pc->cell() > 0 )
	{
		if( ( pc->jailtimer() > 0 ) && ( pc->jailtimer() <= uiCurrentTime ) )
		{
			socket->sysMessage( tr( "Your jail time is over!" ) );
			
			if( pc )
			{
				jails[ pc->cell() ].occupied = false;

				pc->removeFromView( false );
				pc->moveTo( jails[ pc->cell() ].oldpos );
				pc->resend( false );
					
				pc->setCell(0);
				pc->setJailSecs(0);
				pc->setJailTimer(0);
				pc->setPriv2(0);					
					
				savelog( tr( "%1 [0x%2] is automatically released from jail." ).arg( pc->name() ).arg( pc->serial(), 8, 16 ), "server.log" );
				socket->sysMessage( tr( "You have been released." ) );
			}		
		}
	}
*/

	if( SrvParams->hungerRate() > 1 && ( pc->hungerTime() <= currenttime  ) )
	{
		if( !pc->isGMorCounselor() && pc->hunger() ) 
			pc->setHunger( pc->hunger() - 1 ); // GMs and Counselors don't get hungry

		switch( pc->hunger() )
		{
		case 6: break; //Morrolan
		case 5: socket->sysMessage( tr("You are still stuffed from your last meal") );	break;
		case 4:	socket->sysMessage( tr("You are not very hungry but could eat more") );	break;
		case 3:	socket->sysMessage( tr("You are feeling fairly hungry") );				break;
		case 2:	socket->sysMessage( tr("You are extremely hungry") );						break;
		case 1:	socket->sysMessage( tr("You are very weak from starvation") );			break;
		case 0:
			if (!pc->isGMorCounselor())
				socket->sysMessage( tr( "You must eat very soon or you will die!" ) );
			break;
		}
		pc->setHungerTime( currenttime + ( SrvParams->hungerRate() * MY_CLOCKS_PER_SEC ) );
	}

	 // Damage them if they are very hungry
	if( ( hungerdamagetimer <= currenttime ) && SrvParams->hungerDamage() )
	{
		hungerdamagetimer=currenttime+(SrvParams->hungerDamageRate()*MY_CLOCKS_PER_SEC); /** set new hungertime **/
		if( pc->hitpoints() > 0 && pc->hunger()<2 && !pc->isGMorCounselor() && !pc->isDead() )
		{
			socket->sysMessage( tr( "You are starving." ) );
//			pc->hp -= SrvParams->hungerDamage();
			pc->setHitpoints( pc->hitpoints() - SrvParams->hungerDamage() );
			
			if( pc->hitpoints() <=0 )
			{
				socket->sysMessage( tr( "You have died of starvation." ) );
				pc->kill();
			}
		}
	}

/*	// new math + poison wear off timer added by lord binary !
	if( pc->poisoned() && !pc->isInvulnerable() )
	{
		if( pc->poisonTime() <= currenttime )
		{
			if( pc->poisonWearOffTime() > currenttime ) // lb, makes poison wear off pc's
			{
				// We only support 4 levels of poison here.
				switch( pc->poisoned() )
				{
				case 1:
					pc->setPoisonTime(currenttime+(5*MY_CLOCKS_PER_SEC));
					if( pc->isontxt() <= currenttime )
					{
						pc->setPoisontxt(currenttime+(10*MY_CLOCKS_PER_SEC));
						pc->emote( tr( "*%1 looks a bit nauseous!*" ).arg( pc->name() ), 0x26 );
					}
				 
//					pc->hp -= QMAX(((pc->hp)*RandomNum(5,15))/100, RandomNum(0,1) ); // between 0% and 10% of player's hp 
					tempshort = pc->hitpoints();
					tempshort -= QMAX(((tempshort)*RandomNum(5,15))/100, RandomNum(0,1) );
					pc->setHitpoints( tempshort );
					pc->updateHealth();
					break;
				case 2:
					pc->setPoisonTime(currenttime+(4*MY_CLOCKS_PER_SEC));
					if( pc->poisontxt() <= currenttime )
					{
						pc->setPoisontxt(currenttime+(10*MY_CLOCKS_PER_SEC));
						pc->emote( tr( "*%1 looks disoriented and nauseous!*" ).arg( pc->name() ), 0x26 );
					}
					
//					pc->hp -= QMAX(((pc->hp)*RandomNum(10,20))/100, RandomNum(0,1)); //between 10% and 20% of player's hp
					tempshort = pc->hitpoints();
					tempshort -= QMAX(((tempshort)*RandomNum(10,20))/100, RandomNum(0,1));
					pc->setHitpoints( tempshort );
					pc->updateHealth();
					break;
				case 3:
					pc->setPoisonTime(currenttime+(3*MY_CLOCKS_PER_SEC));
					
					if( pc->poisontxt() <= currenttime )
					{
						pc->setPoisontxt(currenttime+(10*MY_CLOCKS_PER_SEC));
						pc->emote( tr( "*%1 is in severe pain!*" ).arg( pc->name() ), 0x26 );
					}
					
//					pc->hp -= QMAX( ( pc->hp * RandomNum( 20, 30 ) ) / 100, RandomNum( 0, 1 ) ); // between 20% and 30% of player's hp 
					tempshort = pc->hitpoints();
					tempshort -= QMAX( ( tempshort * RandomNum( 20, 30 ) ) / 100, RandomNum( 0, 1 ) );
					pc->setHitpoints( tempshort );
					pc->updateHealth();
					break;
				case 4:
					pc->setPoisonTime( currenttime+(3*MY_CLOCKS_PER_SEC) );

					if( pc->poisontxt() <= currenttime )
					{
						pc->setPoisontxt(currenttime+(10*MY_CLOCKS_PER_SEC));
						pc->emote( tr( "*%1 looks extremely sick*" ).arg( pc->name() ), 0x26 );
					}
				
//					pc->hp -= QMAX( ( pc->hp * RandomNum( 30, 40 ) ) / 100, 1 ); //between 30% and 40% of player's hp
					tempshort = pc->hitpoints();
					tempshort -= QMAX( ( tempshort * RandomNum( 30, 40 ) ) / 100, 1 );
					pc->setHitpoints( tempshort );
					pc->updateHealth();
					break;

				default:
					clConsole.send( tr( "Unknown poison type for Character %1 [0x%2]\n" ).arg( pc->name() ).arg( pc->serial(), 8, 16 ) );
					pc->setPoisoned( 0 );
					return;
				}

				if( pc->hitpoints() < 1 )
				{
					socket->sysMessage( tr( "The poison killed you.") );
					pc->kill();
				}
			} // end switch
		} // end if poison-wear off-timer
	} // end if poison-damage timer

	if( pc->poisoned() && pc->poisonWearOffTime() <= currenttime )
	{
		pc->setPoisoned( 0 );
		pc->update(); // a simple status-update should be enough here
		socket->sysMessage( tr( "The poison has worn off." ) );
	}*/
}

void checkNPC( P_NPC pc, unsigned int currenttime )
{
	if (pc == NULL)
		return;
	if (pc->stablemasterSerial() != INVALID_SERIAL) return;

	int pcalc;
	char t[120];

	if( currenttime >= pc->aiCheckTime() )
	{
		pc->setAICheckTime( uiCurrentTime + SrvParams->checkAITime() * MY_CLOCKS_PER_SEC );
		if( pc->ai() )
			pc->ai()->eventHandler();
	}
//	cCharStuff::CheckAI( currenttime, pc );
//	Movement::instance()->NpcMovement( currenttime, pc );
    setcharflag( pc );

	// We are at war and want to prepare a new swing
	if( !pc->isDead() && pc->swingTarget() == -1 && pc->isAtWar() )
		Combat::combat( pc );

	// We are swinging and completed our move
	else if( !pc->isDead() && ( pc->swingTarget() >= 0 && pc->nextHitTime() <= currenttime ) )
		Combat::checkandhit( pc );

	Magic->CheckFieldEffects2( currenttime, pc, 0 );

/*	if( pc->shop() )
		restockNPC( currenttime, pc );
*/
	if( !pc->free )
	{
		if( pc->summonTime() && ( pc->summonTime() <= currenttime ) )
		{
			pc->soundEffect( 0x01FE );
			pc->setDead( true );
			cCharStuff::DeleteChar(pc);
			return;
		}
	}

/*	// Character is not fleeing but reached the border
	if( pc->wanderType() != enFlee && pc->hitpoints() < pc->st()*pc->fleeat() / 100 )
	{
		pc->setOldNpcWander( pc->npcWander() );
		pc->setNpcWander(5);
		pc->setNextMoveTime();
	}

    // Character is fleeing and has regenerated
	if( pc->npcWander() == 5 && pc->hitpoints() > pc->st()*pc->reattackat() / 100 )
	{
		pc->setNpcWander( pc->oldnpcWander() );
		pc->setNextMoveTime();
		pc->setOldNpcWander( 0 );
	}*/

/*	// new poisoning code
	if (pc->poisoned() && !(pc->isInvulnerable()) )
	{
		if( pc->poisonTime() <= currenttime )
		{
			if (pc->poisonWearOffTime()>currenttime) // lb, makes poison wear off pc's
			{
				switch (pc->poisoned())
				{
				case 1:
					pc->setPoisonTime(currenttime+(5*MY_CLOCKS_PER_SEC));
					if( pc->poisontxt() <= currenttime )
					{
						pc->setPoisontxt(currenttime+(10*MY_CLOCKS_PER_SEC));
						pc->emote( tr("* %1 looks a bit nauseous *").arg(pc->name()), 0x0026);
					}
//					pc->hp -= RandomNum(1,2);
					pc->setHitpoints( pc->hitpoints() - RandomNum(1,2) );
					pc->updateHealth();
					break;
				case 2:
					pc->setPoisonTime(currenttime+(4*MY_CLOCKS_PER_SEC));
					if( pc->poisontxt() <= currenttime )
					{
						pc->setPoisontxt(currenttime+(10*MY_CLOCKS_PER_SEC));
						pc->emote( tr("* %1 looks disoriented and nauseous! *").arg(pc->name()), 0x0026);
					}

					pcalc = ( ( pc->hitpoints() * RandomNum(2,5) ) / 100) + RandomNum(0,2); // damage: 1..2..5% of hp's+ 1..2 constant
					
//					pc->hp -= pcalc;
					pc->setHitpoints( pc->hitpoints() - pcalc);
					pc->updateHealth();
					break;
				case 3:
					pc->setPoisonTime(currenttime+(3*MY_CLOCKS_PER_SEC));
					if( pc->poisontxt() <= currenttime )
					{
						pc->setPoisontxt(currenttime+(10*MY_CLOCKS_PER_SEC));
						pc->emote( tr("* %1 is in severe pain! *").arg(pc->name()), 0x0026 );
					}
					pcalc=( ( pc->hitpoints() * RandomNum(5,10) ) / 100 ) + RandomNum(1,3); // damage: 5..10% of hp's+ 1..2 constant
//					pc->hp -= pcalc;
					pc->setHitpoints( pc->hitpoints() - pcalc);
					pc->updateHealth();
					break; // lb !!!
				case 4:
					pc->setPoisonTime(currenttime+(3*MY_CLOCKS_PER_SEC));
					if( pc->poisontxt() <= currenttime )
					{
						pc->setPoisontxt(currenttime+(10*MY_CLOCKS_PER_SEC));
						pc->emote( tr("* %s looks extremely weak and is wrecked in pain! *").arg(pc->name()), 0x0026 );
					}

					pcalc=( (pc->hitpoints() * RandomNum(10,15) ) / 100 ) + RandomNum(3,6); // damage:10 to 15% of hp's+ 3..6 constant, quite deadly <g>
//					pc->hp -= pcalc;
					pc->setHitpoints(pc->hitpoints() - pcalc);
					pc->updateHealth();
					break;
				default:
					clConsole.send("ERROR: Fallout of switch statement without default. wolfpack.cpp, checkNPC()\n"); //Morrolan
					pc->setPoisoned(0);
					return;
				}
				if( pc->hitpoints() < 1 )
				{
					pc->kill();
				}
			} // end switch
		} // end if poison-wear off-timer
	} // end if poison-damage timer

	if ((pc->poisonWearOffTime()<=currenttime))
	{
		if ((pc->poisoned()))
		{
			pc->setPoisoned(0);
			pc->update();
		}
	}
*/
	//hunger code for npcs
	if( SrvParams->hungerRate() && (pc->hungerTime() <= currenttime ) )
	{
		t[0] = '\0';

		if (pc->hunger()) 
			pc->setHunger( pc->hunger()-1 ); //Morrolan GMs and Counselors don't get hungry

		if(pc->isTamed()) // && pc->npcaitype() != 17)
		{//if tamed let's display his hungry status
			switch(pc->hunger())
			{
			case 6:
			case 5:	break;
			case 4:	pc->emote( tr("* %1 looks a little hungry *").arg(pc->name()), 0x0026);		break;
			case 3:	pc->emote( tr("* %1 looks fairly hungry *").arg(pc->name()), 0x0026);			break;
			case 2:	pc->emote( tr("* %1 looks extremely hungry *").arg(pc->name()), 0x0026);		break;
			case 1:	pc->emote( tr("* %1 looks weak from starvation *",pc->name()), 0x0026);		break;
			case 0:
				//maximum hunger - untame code
				//pet release code here
				if(pc->isTamed())
				{
//					pc->setWanderFollowTarget( INVALID_SERIAL );
					pc->setWanderType( enFreely );
					pc->setTamed( false );

					if( pc->owner() )
						pc->setOwner( NULL );

					pc->emote( tr("%1 appears to have decided that it is better off without a master").arg(pc->name()), 0x0026 );

					pc->soundEffect( 0x01FE );
					if( SrvParams->tamedDisappear() == 1 )
						cCharStuff::DeleteChar(pc);
				}
				break;
			}
		}//if tamed
		pc->setHungerTime( currenttime+(SrvParams->hungerRate()*MY_CLOCKS_PER_SEC) );
	}//if hungerrate>1
}

void checkauto() // Check automatic/timer controlled stuff (Like fighting and regeneration)
{
	//int k;
	unsigned int currenttime = uiCurrentTime;
	static unsigned int checkspawnregions = 0;
	static unsigned int checknpcs = 0;
	static unsigned int checktamednpcs = 0;
	static unsigned int checknpcfollow = 0;
	static unsigned int checkitemstime = 0;
	static unsigned int lighttime = 0;
	static unsigned int htmltime = 0;
	static unsigned int housedecaytimer = 0;

	if( shoprestocktime == 0 )
		shoprestocktime = currenttime + MY_CLOCKS_PER_SEC * 60 * 20;

	//static unsigned int repairworldtimer=0;

	if (housedecaytimer<=currenttime)
	{
		//////////////////////
		///// check_houses
		/////////////////////
		if(SrvParams->housedecay_secs() != -1)
			check_house_decay();

/*	TODO: rewrite STABLEMASTER with python
		////////////////////
		// check stabling
		///////////////////
		unsigned long int diff;
		cCharIterator iter_char;
		for( P_CHAR pc = iter_char.first(); pc; pc = iter_char.next() )
		{
			if( pc->npc_type() == 1 )
			{
				vector<SERIAL> pets( stablesp.getData(pc->serial()) );
				unsigned int ci;
				for (ci = 0; ci < pets.size();ci++)
				{
					P_CHAR pc_pet = FindCharBySerial(pets[ci]);
					if (pc_pet != NULL)
					{
						diff = (getNormalizedTime() - pc_pet->timeused_last()) / MY_CLOCKS_PER_SEC;
						pc_pet->setTime_unused( pc_pet->time_unused() + diff );
					}
				}
			}
		}
*/
		housedecaytimer=uiCurrentTime+MY_CLOCKS_PER_SEC*60*30; // check only each 30 minutes
	}


	if(checkspawnregions<=currenttime && SrvParams->spawnRegionCheckTime() != -1)//Regionspawns
	{
		SpawnRegions::instance()->check();
		checkspawnregions=uiCurrentTime+SrvParams->spawnRegionCheckTime()*MY_CLOCKS_PER_SEC;//Don't check them TOO often (Keep down the lag)
	}

	if( ( SrvParams->html() > 0 ) && ( htmltime <= currenttime ) )
	{
//			updatehtml();
			htmltime = currenttime + ( SrvParams->html() * MY_CLOCKS_PER_SEC );
	}

	if( SrvParams->saveInterval() != 0 )
	{
		if (autosaved == 0)
		{
			autosaved = 1;
			time( (time_t*) (&oldtime)) ;
		}
		time( (time_t*) ( &newtime)) ;

		if (dosavewarning==1)
		if (difftime(newtime,oldtime)==SrvParams->saveInterval()-10) 
		{
			cNetwork::instance()->broadcast( tr( "World will be saved in 10 seconds.." ) );
			dosavewarning = 0;
		}

		if (difftime(newtime, oldtime)>=SrvParams->saveInterval() )
		{
			autosaved = 0;
			World::instance()->save();
			dosavewarning = 1;
		}
	}

	//Time functions
	if( uotickcount <= currenttime )
	{
		uoTime.addSecs(1);
		uotickcount = currenttime + SrvParams->secondsPerUOMinute()*MY_CLOCKS_PER_SEC;
	}

	// Recalculate and Resend Lightlevel
	if( lighttime <= currenttime )
	{
		// Resend the lightlevel to all clients
		UINT16 level = 0xFF;

		if( uoTime.time().hour() <= 3 && uoTime.time().hour() >= 10 )
			level = SrvParams->worldDarkLevel();
		else
			level = SrvParams->worldBrightLevel();

		if( level == 0xFFFF )
		{
			level = ( ( ( 60 * ( uoTime.time().hour() - 4 ) ) + uoTime.time().minute()) * (SrvParams->worldDarkLevel()-SrvParams->worldBrightLevel())) / 360;

			if(uoTime.time().hour() < 12)
				level += SrvParams->worldBrightLevel();
			else
				level = SrvParams->worldDarkLevel() - level;
		}

		// Only update and resend when the lightlevel
		// Really changed
		if( level != SrvParams->worldCurrentLevel() )
		{
			SrvParams->worldCurrentLevel() = level;

			cUOSocket *mSock = cNetwork::instance()->first();

			while( mSock )
			{
				mSock->updateLightLevel( level );
				mSock = cNetwork::instance()->next();
			}
		}

		// Update lightlevel every 30 seconds
		lighttime = currenttime + 30 * MY_CLOCKS_PER_SEC;
	}

	// Check Spawners
	//static UINT32 respawntime = 0;

    //if( respawntime <= currenttime )
	//{
     //  respawntime = currenttime + 5 * MY_CLOCKS_PER_SEC;

	   // This is a rather "crappy" loop
	   // We iterate trough all items just because we want
	   // To "fix" spawners. We better leave that alone
	   // And ensure that the spawners are no-decay in the scripts
	   /*AllItemsIterator iterItems;
       for( iterItems.Begin(); !iterItems.atEnd(); ++iterItems )
	   {
			P_ITEM pi = iterItems.GetData();
			if(pi != NULL)
			{
				// lets make sure they are spawners.
				if( ( pi->type() >= 61 && pi->type() <= 65) || ( pi->type() == 69) || ( pi->type() == 125 ) )
				{
					// set to nodecay and refresh.
	                pi->priv=0;
		            pi->update();
				 }
			}
		}*/
	//}

	// Regenerate only players who are online
	for( cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next() )
	{
		if( !socket->player() )
			continue;

		checkRegeneration( socket->player(), currenttime );
		checkPC( socket->player(), currenttime );

		// Check all Characters first (Intersting. we seem to check characters more than once)
		RegionIterator4Chars iterator( socket->player()->pos() );
		for( iterator.Begin(); !iterator.atEnd(); iterator++ )
		{
			P_CHAR pChar = iterator.GetData();

			if( !pChar )
				continue;

			// we check tamed npcs more often than
			// untamed npcs so they can react faster
			if( ( !pChar->isTamed() && checknpcs <= currenttime ) ||
				( pChar->isTamed() && checktamednpcs <= currenttime ) )
			{
				if( pChar->objectType() == enNPC )
				{
					// Regenerate Mana+Stamina+Health
					checkRegeneration( pChar, currenttime );

					// We only process the AI for NPCs who are in a used area
					if( pChar->dist( socket->player() ) <= 24 )
						checkNPC( dynamic_cast<P_NPC>(pChar), currenttime );
				}
				// Timed for logout
				else if( pChar->objectType() == enPlayer )
				{
					P_PLAYER pp = dynamic_cast<P_PLAYER>(pChar);
					if( pp->logoutTime() && pp->logoutTime() >= currenttime )
					{
						pp->setLogoutTime( 0 );
						pp->removeFromView( false );
						pp->resend( false );
					}
				}
			}		
		}

		// Check the mapregions around this character. 
		// This *could* be expensive when many characters are in the same
		// region.
		if( checkitemstime <= currenttime  )
		{
			RegionIterator4Items itemIter( socket->player()->pos() );
			for( itemIter.Begin(); !itemIter.atEnd(); itemIter++ )
			{
				P_ITEM pItem = itemIter.GetData();
	
				if( !pItem )
					continue;
			
				Items->RespawnItem( currenttime, pItem ); // Checks if the item is a spawner
				Items->DecayItem( currenttime, pItem ); // Checks if the item should decay

				switch( pItem->type() )
				{
				// This is rather stupid, why is it not a normal decaytimer!
				case 51:
				case 52:
					if( pItem->gatetime() < currenttime )
						Items->DeleItem( pItem );
					break;

				// If it is a sound-item there is a 1%*item->morez chance that
				// It "emits" a sound to this character.
				// This is a rather stupid method...
				// Only one character is hearing the sound at once
				case 88:
					if( pItem->dist( socket->player() ) < pItem->morey() )
						if( RandomNum( 1, 100 ) <= pItem->morez() )
							socket->soundEffect( pItem->morex(), pItem );
					break;

				// Move Boats
				case 117:
					if( pItem->tags().get( "tiller" ).isValid() && 
						( pItem->gatetime() <= currenttime ) )
					{
						cBoat* pBoat = dynamic_cast< cBoat* >( FindItemBySerial( pItem->tags().get( "boatserial" ).toInt() ) );
						if( pBoat )
						{
							pBoat->move();
							pItem->setGateTime((UINT32)( currenttime + (double)( SrvParams->boatSpeed() * MY_CLOCKS_PER_SEC ) ));
						}
					}
					break;
				};				
			}
		}
	}

	// Check the TempEffects
	TempEffects::instance()->check();

	if( checknpcs <= currenttime ) checknpcs=(unsigned int)((double)(SrvParams->checkNPCTime()*MY_CLOCKS_PER_SEC+currenttime)); //lb
	if( checktamednpcs <= currenttime ) checktamednpcs=(unsigned int)((double) currenttime+(SrvParams->checkTammedTime()*MY_CLOCKS_PER_SEC)); //AntiChrist
	if( checknpcfollow <= currenttime ) checknpcfollow=(unsigned int)((double) currenttime+(SrvParams->checkFollowTime()*MY_CLOCKS_PER_SEC)); //Ripper
	if( checkitemstime <= currenttime ) checkitemstime=(unsigned int)((double)(SrvParams->checkItemTime()*MY_CLOCKS_PER_SEC+currenttime)); //lb
	if( shoprestocktime <= currenttime )
		shoprestocktime = currenttime + MY_CLOCKS_PER_SEC * 60 * 20;

	// Update the delay for the next field-effect (every 500ms)
	if( nextfieldeffecttime <= currenttime )
		nextfieldeffecttime = (unsigned int) ( currenttime + ( 0.5 * MY_CLOCKS_PER_SEC ) );

	// Update the next-decay time (decay-check is made every 15 seconds)
	if( nextdecaytime <= currenttime )
		nextdecaytime = currenttime + ( 15 * MY_CLOCKS_PER_SEC );
}
