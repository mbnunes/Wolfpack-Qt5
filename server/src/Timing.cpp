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

#include "Timing.h"
#include "worldmain.h"
#include "walking2.h"
#include "TmpEff.h"
#include "combat.h"
#include "regions.h"
#include "srvparams.h"
#include "classes.h"
#include "network/uosocket.h"
#include "network.h"
#include "spawnregions.h"
#include "territories.h"

// Library Includes
#include "qdatetime.h"

#undef  DBGFILE
#define DBGFILE "Timing.cpp"

void checktimers() // Check shutdown timers
{
	register unsigned int tclock = uiCurrentTime;
	overflow = ( lclock > tclock );
	if (endtime)
	{
		if (endtime <= tclock) keeprun = 0;
	}
	lclock = tclock;
}

void restockNPC( UINT32 currenttime, P_CHAR pc_i )
{
	if( SrvParams->shopRestock() && ( shoprestocktime <= currenttime ) )
	{
		P_ITEM pStock = pc_i->GetItemOnLayer( 0x1A );

		if( pStock )
		{
			vector<SERIAL> content = contsp.getData( pStock->serial );
			for( UINT32 i = 0; i < content.size(); ++i )
			{
				P_ITEM pItem = FindItemBySerial( content[i] );
				if( pItem )
				{
					if( pItem->restock < pItem->amount() )
					{
						UINT16 restock = QMAX( ( pItem->amount() - pItem->restock ) / 2, 1 );
						pItem->restock += restock;
					}

					if( SrvParams->trade_system() )
					{
						cTerritory* Region = cAllTerritories::getInstance()->region( pc_i->pos.x, pc_i->pos.y );
						
						if( Region != NULL )
							StoreItemRandomValue( pItem, Region->name() );
					}
				}
			}
		}
	}
}

void genericCheck(P_CHAR pc, unsigned int currenttime)// Char mapRegions
{
	if( !pc )
		return;
	
	if( !pc->dead )
	{
		if( pc->hp > pc->st )
		{
			pc->hp = pc->st;
			pc->updateHealth();
		}

		if( pc->stm > pc->effDex() )
		{
			pc->stm = pc->effDex();
			pc->socket()->updateStamina();
		}
		if (pc->mn>pc->in)
		{
			pc->mn = pc->in;
			pc->socket()->updateMana();
		}

		// Health regeneration
		if( ( pc->regen <= currenttime ) || (overflow) )
		{
			UINT32 interval = SrvParams->hitpointrate() * MY_CLOCKS_PER_SEC;

			// If it's not disabled hunger affects our health regeneration
			if( pc->hp < pc->st && pc->hunger() > 3 || SrvParams->hungerRate() == 0 )
			{
				for( UINT16 c = 0; c < pc->st + 1; ++c )
				{
					if( pc->regen + ( c * interval ) <= currenttime && pc->hp <= pc->st )
					{
						if( pc->skill( HEALING ) < 500 )
							pc->hp++;
						else if (pc->skill( HEALING ) < 800)
							pc->hp += 2;
						else 
							pc->hp += 3;

						if( pc->hp > pc->st )
						{
							pc->hp = pc->st;
							break;
						}
					}
				}
				pc->updateHealth();
			}

			pc->regen = currenttime + interval;
		}

		// Stamina regeneration
		if( ( pc->regen2 <= currenttime ) || overflow )
		{
			UINT32 interval = SrvParams->staminarate()*MY_CLOCKS_PER_SEC;
			for( UINT16 c = 0; c < pc->effDex() + 1; ++c )
			{
				if (pc->regen2 + (c*interval) <= currenttime && pc->stm <= pc->effDex())
				{
					pc->stm++;
					if (pc->stm>pc->effDex())
					{
						pc->stm = pc->effDex();
						break;
					}
					pc->socket()->updateStamina();
				}
			}
			pc->regen2 = currenttime + interval;			
		}

		// OSI Style Mana regeneration by blackwind
		// if (pc->in>pc->mn)  this leads to the 'mana not subtracted' bug (Duke)
		if ((pc->regen3 <= currenttime) || (overflow))
		{
			unsigned int interval = SrvParams->manarate()*MY_CLOCKS_PER_SEC;
			for( UINT16 c = 0; c < pc->in + 1 ; ++c )
			{
				if (pc->regen3 + (c*interval) <= currenttime && pc->mn <= pc->in)
				{
				pc->mn++;
				if (pc->med() && pc->mn <= pc->mn2)
						pc->mn += 5;
					if (pc->mn>pc->in)
					{
						if (pc->med())
						{
							pc->socket()->sysMessage( tr("You are at peace." ) );
							pc->setMed( false );
						}
						pc->mn = pc->in;
						break;
					}
					pc->socket()->updateMana();
				}
			}
			if (SrvParams->armoraffectmana())
			{
				int ratio = ( ( 100 + 50 ) / SrvParams->manarate() );
				// 100 = Maximum skill (GM)
				// 50 = int affects mana regen (%50)
				int armorhandicap = ((Skills->GetAntiMagicalArmorDefence(pc) + 1) / SrvParams->manarate());
				int charsmeditsecs = (1 + SrvParams->manarate() - ((((pc->skill(MEDITATION) + 1)/10) + ((pc->in + 1) / 2)) / ratio));
				if (pc->med())
				{
					pc->regen3 = currenttime + ((armorhandicap + charsmeditsecs/2)* MY_CLOCKS_PER_SEC);
				}
				else
					pc->regen3 = currenttime + ((armorhandicap + charsmeditsecs)* MY_CLOCKS_PER_SEC);
			}
			else 
				pc->regen3 = currenttime + interval;
			}
			// end Mana regeneration
			
			if ((pc->hidden() == 2) && ((pc->invistimeout() <= currenttime) || (overflow)) && (!(pc->priv2&8)))
			{// only if not permanently hidden - AntiChrist
				pc->setHidden( 0 );
				pc->setStealth(-1);
				pc->resend( false );
			}
	}

	// Check if the character died
	if( pc->hp <= 0 && !pc->dead )
		pc->kill();
}

void checkPC( P_CHAR pc, unsigned int currenttime ) //Char mapRegions
{
	cUOSocket *socket = pc->socket();

	// We are not logged in so dont check anything at all.
	if( !socket )
		return;

	int y,x, timer;//, valid=0;
	char t[120];

	if ( pc == NULL ) return;

	// Check if the character is in a field which affects him
	Magic->CheckFieldEffects2( currenttime, pc, 1 ); 
	
	if( !pc->dead && pc->swingtarg() == -1 )
		Combat->DoCombat( pc, currenttime );
	else if( !pc->dead && ( pc->swingtarg() >= 0 && pc->timeout <= currenttime ) )
		Combat->CombatHitCheckLoS( pc, currenttime );

	// Guess it was taken out because of traffic-concerns ?
/*	if (wtype==1 && raindroptime<=currenttime && !noweather[s]) // implment. of xuri's raindrop idea, LB
	{
		switch(rand()%3)
		{
		case 0:soundeffect2(i,0x00,0x24); // fall-throughs intentional !!
		case 1:soundeffect2(i,0x00,0x23);
		 case 2:soundeffect2(i,0x00,0x22);
		}
	} */

	if( pc->isPlayer() && pc->squelched() == 2 )
	{
		if( pc->mutetime() > 0 )
		{
			if (pc->mutetime()<=currenttime||overflow)
			{
				pc->setSquelched(0);
				pc->setMutetime(0);
				socket->sysMessage( tr( "You are no longer muted." ) );
			}
		}
	}

	if( pc->isPlayer() )
	{
		if ( pc->crimflag() > 0 && ( pc->crimflag() <= currenttime || overflow ) &&  pc->isCriminal() )//AntiChrist
		{
			socket->sysMessage( tr( "You are no longer criminal" ) );
			pc->setCrimflag(0);
			pc->setInnocent();
		}

		if( pc->murderrate() < currenttime )
		{
			if (pc->kills>0)
				pc->kills--;
			if ((pc->kills==SrvParams->maxkills())&&(SrvParams->maxkills()>0))
				socket->sysMessage( tr( "You are no longer a murderer." ) );
			pc->setMurderrate( ( SrvParams->murderdecay() * MY_CLOCKS_PER_SEC ) + currenttime );
		}

		setcharflag( pc );
	}

	if( pc->isPlayer() && pc->casting() )//PC casting a spell
	{
		pc->setNextact( pc->nextact() - 1 );
		if( pc->spelltime() <= currenttime || overflow )//Spell is complete target it
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

	if( SrvParams->bgSound() >= 1 )
	{
		timer = SrvParams->bgSound() * 100;
		if( timer == 0 )
			timer = 1;

		if( socket && !pc->dead && ( (rand()%(timer) ) == (timer/2))) 
			bgsound( pc );
	}

	// Reset spirit-speak
	if( pc->spiritspeaktimer > 0 && pc->spiritspeaktimer <= uiCurrentTime )
		pc->spiritspeaktimer = 0;
	
	// Jail stuff
	if( pc->cell > 0 )
	{
		if( ( pc->jailtimer > 0 ) && ( pc->jailtimer <= uiCurrentTime ) )
		{
			socket->sysMessage( tr( "Your jail time is over!" ) );
			
			if( pc )
			{
				if( pc->cell == 0 )
				{			
					socket->sysMessage( tr( "You're not in jail already ? Please report to a GM. This has been logged." ) );
					pc->jailtimer = 0;
					savelog( tr( "%1 [0x%2] caused bug in jail system." ).arg( pc->name.c_str() ).arg( pc->serial, 8, 16 ), "server.log" );
				}
				else
				{
					jails[ pc->cell ].occupied = false;

					pc->removeFromView( false );
					pc->moveTo( jails[ pc->cell ].oldpos );
					pc->resend( false );
					
					pc->cell = 0;
					pc->jailsecs = 0;
					pc->jailtimer = 0;
					pc->priv2 = 0;					
					
					savelog( tr( "%1 [0x%2] is automatically released from jail." ).arg( pc->name.c_str() ).arg( pc->serial, 8, 16 ), "server.log" );
					socket->sysMessage( tr( "You have been released." ) );
				}
			}		
		}
	}	

	// changed to seconds instead of crappy #of checks
	// This is totally broken
	/*if( pc->trackingtimer > currenttime )
	{
		// TODO: Disable quest-arrow here

		if( pc->trackingdisplaytimer() <= currenttime )
		{
			pc->setTrackingdisplaytimer( currenttime + SrvParams->redisplaytime() * MY_CLOCKS_PER_SEC );
			Skills->Track( pc );
		}
	}
	else
	{
		// dont send arrow-away packet all the time
		// NOTE: This should be send every 5 seconds (!)
		if( pc->trackingtimer > ( currenttime / 10 ) )
		{
			pc->trackingtimer = 0;
			unsigned char arrow[7] = {0xBA, 0,};
			P_CHAR pc_trackingTarget = FindCharBySerial( pc->trackingtarget );
		}
	}*/

	if( SrvParams->hungerRate() > 1 && ( pc->hungertime() <= currenttime || overflow ) )
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

	if (((hungerdamagetimer<=currenttime)||(overflow))&&(SrvParams->hungerDamage()>0)) // Damage them if they are very hungry
	{
		hungerdamagetimer=currenttime+(SrvParams->hungerDamageRate()*MY_CLOCKS_PER_SEC); /** set new hungertime **/
		if( pc->hp > 0 && pc->hunger()<2 && !pc->isGMorCounselor() && !pc->dead )
		{
			socket->sysMessage( tr( "You are starving." ) );
			pc->hp -= SrvParams->hungerDamage();
			
			if( pc->hp <=0 )
			{
				socket->sysMessage( tr( "You have died of starvation." ) );
				pc->kill();
			}
		}
	}

	// new math + poison wear off timer added by lord binary !
	if( pc->poisoned() && !pc->isInvul() )
	{
		if( pc->poisontime() <= currenttime || ( overflow ) )
		{
			if( pc->poisonwearofftime() > currenttime ) // lb, makes poison wear off pc's
			{
				// We only support 4 levels of poison here.
				switch( pc->poisoned() )
				{
				case 1:
					pc->setPoisontime(currenttime+(5*MY_CLOCKS_PER_SEC));
					if ( pc->poisontxt()<=currenttime || (overflow))
					{
						pc->setPoisontxt(currenttime+(10*MY_CLOCKS_PER_SEC));
						pc->emote( tr( "*%1 looks a bit nauseous!*" ).arg( pc->name.c_str() ), 0x26 );
					}
				 
					pc->hp -= QMAX(((pc->hp)*RandomNum(5,15))/100, RandomNum(0,1) ); // between 0% and 10% of player's hp 
 
					updatestats(pc, 0);
					break;
				case 2:
					pc->setPoisontime(currenttime+(4*MY_CLOCKS_PER_SEC));
					if ((pc->poisontxt()<=currenttime)||(overflow))
					{
						pc->setPoisontxt(currenttime+(10*MY_CLOCKS_PER_SEC));
						pc->emote( tr( "*%1 looks disoriented and nauseous!*" ).arg( pc->name.c_str() ), 0x26 );
					}
					
					pc->hp -= QMAX(((pc->hp)*RandomNum(10,20))/100, RandomNum(0,1)); //between 10% and 20% of player's hp
					pc->updateHealth();
					break;
				case 3:
					pc->setPoisontime(currenttime+(3*MY_CLOCKS_PER_SEC));
					
					if( pc->poisontxt() <= currenttime ||(overflow))
					{
						pc->setPoisontxt(currenttime+(10*MY_CLOCKS_PER_SEC));
						pc->emote( tr( "*%1 is in severe pain!*" ).arg( pc->name.c_str() ), 0x26 );
					}
					
					pc->hp -= QMAX( ( pc->hp * RandomNum( 20, 30 ) ) / 100, RandomNum( 0, 1 ) ); // between 20% and 30% of player's hp 
					pc->updateHealth();
					break;
				case 4:
					pc->setPoisontime( currenttime+(3*MY_CLOCKS_PER_SEC) );

					if( pc->poisontxt() <= currenttime || (overflow) )
					{
						pc->setPoisontxt(currenttime+(10*MY_CLOCKS_PER_SEC));
						pc->emote( tr( "*%1 looks extremely sick*" ).arg( pc->name.c_str() ), 0x26 );
					}
				
					pc->hp -= QMAX( ( pc->hp * RandomNum( 30, 40 ) ) / 100, 1 ); //between 30% and 40% of player's hp 
					updatestats(pc, 0);
					break;

				default:
					clConsole.send( tr( "Unknown poison type for Character %1 [0x%2]\n" ).arg( pc->name.c_str() ).arg( pc->serial, 8, 16 ) );
					pc->setPoisoned( 0 );
					return;
				}

				if( pc->hp < 1 )
				{
					socket->sysMessage( tr( "The poison killed you.") );
					pc->kill();
				}
			} // end switch
		} // end if poison-wear off-timer
	} // end if poison-damage timer

	if( pc->poisoned() && pc->poisonwearofftime() <= currenttime )
	{
		pc->setPoisoned( 0 );
		pc->update(); // a simple status-update should be enough here
		socket->sysMessage( tr( "The poison has worn off." ) );
	}

	// Horse checks ( Todo: move to DELEITEM + SETCONTSERIAL + SETLAYER )
	if( pc->onHorse() )
	{
		P_ITEM pHorse = pc->GetItemOnLayer( 0x19 );
		if( !pHorse )
		{
			pc->setOnHorse( false );	// turn it off, we aren't on one because there's no item!
			return;
		}
		else
		{
			if( pHorse->decaytime != 0 && ( pHorse->decaytime <= uiCurrentTime || overflow ) )
			{
				pc->setOnHorse( false );
				Items->DeleItem( pHorse );
			}
		}
	}
}

void checkNPC(P_CHAR pc, unsigned int currenttime)
{
	if (pc == NULL)
		return;
	if (pc->stablemaster_serial() != INVALID_SERIAL) return;

	int pcalc;
	char t[120];

	Npcs->CheckAI( currenttime, pc );
	Movement->NpcMovement( currenttime, pc );
    setcharflag( pc );

	if( !pc->dead && pc->swingtarg() == -1 )
		Combat->DoCombat( pc, currenttime );
	else if( !pc->dead && ( pc->swingtarg() >= 0 && pc->timeout <= currenttime ) )
		Combat->CombatHitCheckLoS(pc,currenttime);

	Magic->CheckFieldEffects2( currenttime, pc, 0 );

	if( pc->shop )
		restockNPC( currenttime, pc );

	if( !pc->free )
	{
		if ((pc->disabled()>0)&&((pc->disabled()<=currenttime)||(overflow)))
		{
			pc->setDisabled(0);
		}
		if (pc->summontimer<=currenttime||(overflow))
		{
			if(pc->summontimer>0)
			{
				// Dupois - Added Dec 20, 1999
				// QUEST expire check - after an Escort quest is created a timer is set
				// so that the NPC will be deleted and removed from the game if it hangs around
				// too long without every having its quest accepted by a player so we have to remove
				// its posting from the message board before icing the NPC
				// Only need to remove the post if the NPC does not have a follow target set
				if ( (pc->questType()==ESCORTQUEST) && (pc->ftarg == INVALID_SERIAL) )
				{
					MsgBoardQuestEscortRemovePost( pc );
					MsgBoardQuestEscortDelete( pc );
					return;
				}
				// Dupois - End

				soundeffect2(pc, 0x01FE);
				pc->dead=true;
				Npcs->DeleteChar(pc);
				return;
			}
		}
	}

	// Character is not fleeing but reached the border
	if( pc->npcWander != 5 && pc->hp < pc->st*pc->fleeat() / 100 )
	{
		pc->oldnpcWander = pc->npcWander;
		pc->npcWander=5;
		pc->setNextMoveTime();
	}

    // Character is fleeing and has regenerated
	if( pc->npcWander == 5 && pc->hp > pc->st*pc->reattackat() / 100 )
	{
		pc->npcWander = pc->oldnpcWander;
		pc->setNextMoveTime();
		pc->oldnpcWander = 0;
	}

	// new poisoning code
	if (pc->poisoned() && !(pc->isInvul()) )
	{
		if ((pc->poisontime()<=currenttime)||(overflow))
		{
			if (pc->poisonwearofftime()>currenttime) // lb, makes poison wear off pc's
			{
				switch (pc->poisoned())
				{
				case 1:
					pc->setPoisontime(currenttime+(5*MY_CLOCKS_PER_SEC));
					if ((pc->poisontxt()<=currenttime)||(overflow))
					{
						pc->setPoisontxt(currenttime+(10*MY_CLOCKS_PER_SEC));
						sprintf(t,"* %s looks a bit nauseous *",pc->name.c_str());
						pc->emotecolor = 0x0026;//buffer[s][4];
						npcemoteall(pc,t,1);
					}
					pc->hp -= RandomNum(1,2);
					updatestats(pc, 0);
					break;
				case 2:
					pc->setPoisontime(currenttime+(4*MY_CLOCKS_PER_SEC));
					if ((pc->poisontxt()<=currenttime)||(overflow))
					{
						pc->setPoisontxt(currenttime+(10*MY_CLOCKS_PER_SEC));
						sprintf(t,"* %s looks disoriented and nauseous! *",pc->name.c_str());
						pc->emotecolor = 0x0026; //buffer[s][4];
						npcemoteall(pc,t,1);
					}

					pcalc = ( ( pc->hp * RandomNum(2,5) ) / 100) + RandomNum(0,2); // damage: 1..2..5% of hp's+ 1..2 constant
					pc->hp -= pcalc;
					updatestats(pc, 0);
					break;
				case 3:
					pc->setPoisontime(currenttime+(3*MY_CLOCKS_PER_SEC));
					if ((pc->poisontxt()<=currenttime)||(overflow))
					{
						pc->setPoisontxt(currenttime+(10*MY_CLOCKS_PER_SEC));
						sprintf(t,"* %s is in severe pain! *",pc->name.c_str());
						pc->emotecolor = 0x0026;//buffer[s][4];
						npcemoteall(pc,t,1);
					}
					pcalc=( ( pc->hp * RandomNum(5,10) ) / 100 ) + RandomNum(1,3); // damage: 5..10% of hp's+ 1..2 constant
					pc->hp -= pcalc;
					updatestats(pc, 0);
					break; // lb !!!
				case 4:
					pc->setPoisontime(currenttime+(3*MY_CLOCKS_PER_SEC));
					if ((pc->poisontxt()<=currenttime)||(overflow))
					{
						pc->setPoisontxt(currenttime+(10*MY_CLOCKS_PER_SEC));
						sprintf(t,"* %s looks extremely weak and is wrecked in pain! *",pc->name.c_str());
						pc->emotecolor = 0x0026;//buffer[s][4];
						npcemoteall(pc,t,1);
					}

					pcalc=( (pc->hp * RandomNum(10,15) ) / 100 ) + RandomNum(3,6); // damage:10 to 15% of hp's+ 3..6 constant, quite deadly <g>
					pc->hp -= pcalc;
					updatestats(pc, 0);
					break;
				default:
					clConsole.send("ERROR: Fallout of switch statement without default. wolfpack.cpp, checkNPC()\n"); //Morrolan
					pc->setPoisoned(0);
					return;
				}
				if( pc->hp < 1 )
				{
					pc->kill();
				}
			} // end switch
		} // end if poison-wear off-timer
	} // end if poison-damage timer

	if ((pc->poisonwearofftime()<=currenttime))
	{
		if ((pc->poisoned()))
		{
			pc->setPoisoned(0);
			impowncreate(calcSocketFromChar(pc), pc, 1); // updating to blue stats-bar ...
		}
	}

	//hunger code for npcs
	if (SrvParams->hungerRate()>1 && (pc->hungertime()<=currenttime || overflow))
	{
		t[0] = '\0';

		if (pc->hunger()) 
			pc->setHunger( pc->hunger()-1 ); //Morrolan GMs and Counselors don't get hungry

		if(pc->tamed() && pc->npcaitype() != 17)
		{//if tamed let's display his hungry status
			switch(pc->hunger())
			{
			case 6:
			case 5:	break;
			case 4:	sprintf(t,"* %s looks a little hungry *",pc->name.c_str());			break;
			case 3:	sprintf(t,"* %s looks fairly hungry *",pc->name.c_str());			break;
			case 2:	sprintf(t,"* %s looks extremely hungry *",pc->name.c_str());		break;
			case 1:	sprintf(t,"* %s looks weak from starvation *",pc->name.c_str());	break;
			case 0:
				//maximum hunger - untame code - AntiChrist
				//pet release code here
				if(pc->tamed())
				{
					pc->ftarg = INVALID_SERIAL;
					pc->npcWander=2;
					pc->setTamed(false);
					if(pc->ownserial!=-1) 
						pc->SetOwnSerial(-1);
					sprintf((char*)temp, "* %s appears to have decided that it is better off without a master *", pc->name.c_str());
					npctalkall(pc, (char*)temp,0);
					{
						soundeffect2(pc, 0x01FE);
						if(SrvParams->tamedDisappear()==1)
						Npcs->DeleteChar(pc);
					}
				}
				//sprintf(t,"* %s must eat very soon or he will die! *",pc->name);
				break;
			}

			if(strlen(t))
			{//display message ( if there's one
				pc->emotecolor = 0x0026;//buffer[s][4];
				npcemoteall(pc,t,1);
			}
		}//if tamed
		pc->setHungerTime( currenttime+(SrvParams->hungerRate()*MY_CLOCKS_PER_SEC) );
	}//if hungerrate>1
}

void checkauto() // Check automatic/timer controlled stuff (Like fighting and regeneration)
{
	//int k;
	unsigned int i;
	register unsigned int currenttime = uiCurrentTime;
	static unsigned int checkspawnregions = 0;
	static unsigned int checknpcs = 0;
	static unsigned int checktamednpcs = 0;
	static unsigned int checknpcfollow = 0;
	static unsigned int checkitemstime = 0;
	static unsigned int lighttime = 0;
	static unsigned int htmltime = 0;
	static unsigned int housedecaytimer = 0;
	static unsigned int freeUnusedMemory = 0;

	//static unsigned int repairworldtimer=0;

	if (housedecaytimer<=currenttime)
	{
		//////////////////////
		///// check_houses
		/////////////////////
		if(SrvParams->housedecay_secs() != -1)
			int check_house_decay();

		////////////////////
		// check stabling
		///////////////////

		unsigned long int diff;

		// TODO: BAD !! We do that twice a loop
		AllCharsIterator iter_char;
		   
		for (iter_char.Begin(); !iter_char.atEnd(); ++iter_char)
		{
			P_CHAR pc = iter_char.GetData();
			if (pc->npc_type() == 1)
			{
				vector<SERIAL> pets( stablesp.getData(pc->serial) );
				unsigned int ci;
				for (ci = 0; ci < pets.size();ci++)
				{
					P_CHAR pc_pet = FindCharBySerial(pets[ci]);
					if (pc_pet != NULL)
					{
						diff = (getNormalizedTime() - pc_pet->timeused_last()) / MY_CLOCKS_PER_SEC;
						pc_pet->setTime_unused( pc_pet->time_unused() + diff );
						//clConsole.send("stabling-check debug-name: %s\n",chars[i].name);
					}
				}
			}
		}
		housedecaytimer=uiCurrentTime+MY_CLOCKS_PER_SEC*60*30; // check only each 30 minutes
	}


	if(checkspawnregions<=currenttime && SrvParams->spawnRegionCheckTime() != -1)//Regionspawns
	{
		cAllSpawnRegions::getInstance()->check();
		checkspawnregions=uiCurrentTime+SrvParams->spawnRegionCheckTime()*MY_CLOCKS_PER_SEC;//Don't check them TOO often (Keep down the lag)
	}

	if(SrvParams->html()>0 && (htmltime<=currenttime || overflow))
	{
			updatehtml();
			htmltime=currenttime+(SrvParams->html()*MY_CLOCKS_PER_SEC);
	}

	if (SrvParams->saveInterval() != 0)
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
		   sysbroadcast("World will be saved in 10 seconds..");
		   dosavewarning = 0;
		}

		if (difftime(newtime, oldtime)>=SrvParams->saveInterval() || cwmWorldState->Saving() )
		{
			autosaved = 0;
			cwmWorldState->savenewworld(0);
			dosavewarning = 1;
		}
	}

	//Time functions
	if (uotickcount<=currenttime||(overflow))
	{
		uoTime.addSecs(1);
		uotickcount = currenttime + SrvParams->secondsPerUOMinute()*MY_CLOCKS_PER_SEC;
	}

	if(lighttime<=currenttime || (overflow))
	{
		doworldlight(); //Changes lighting, if it is currently time to.
		int i;
		for (i = 0; i < now; i++) 
			if (online(currchar[i])) 
				dolight(i, SrvParams->worldCurrentLevel()); // bandwidth fix, LB
		lighttime=currenttime+30*MY_CLOCKS_PER_SEC;
	}
	static unsigned int itemlooptime = 0;
    if(itemlooptime <=currenttime || (overflow))
	{
       itemlooptime = currenttime+5*MY_CLOCKS_PER_SEC;
	   AllItemsIterator iterItems;
       for( iterItems.Begin(); !iterItems.atEnd(); ++iterItems ) // Ripper...so spawners get set to nodecay.
	   {
			P_ITEM pi = iterItems.GetData();
			if(pi != NULL)
			{
				// lets make sure they are spawners.
				if ((pi->type()>=61 && pi->type()<=65) || (pi->type()==69) || (pi->type()==125))
				{
					// set to nodecay and refresh.
	                pi->priv=0;
		            RefreshItem(pi);
				 }
			}
		}
	}

	for( cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next() )
	{
		if( !socket->player() )
			continue;

		genericCheck( socket->player(), currenttime );
		checkPC( socket->player(), currenttime );

		// Check all Characters first
		RegionIterator4Chars iterator( socket->player()->pos );
		for( iterator.Begin(); !iterator.atEnd(); iterator++ )
		{
			P_CHAR pChar = iterator.GetData();

			if( !pChar )
				continue;

			// we check tamed npcs more often than
			// untamed npcs so they can react faster
			if( ( !pChar->tamed() && checknpcs <= currenttime ) ||
				( pChar->tamed() && checktamednpcs <= currenttime ) ||
				( ( pChar->npcWander == 1 ) && checknpcfollow <= currenttime ) ||
				overflow )
			{
				if( pChar->isNpc() )
				{
					genericCheck( pChar, currenttime );

					// We only process the AI for NPCs who are in a used area
					if( pChar->pos.distance( socket->player()->pos ) <= 24 )
						checkNPC( pChar, currenttime );
				}
				// Timed for logout
				else if( ( 
						pChar->logout() && 
						( pChar->logout() >= currenttime ) 
					) || overflow )
				{
					pChar->setLogout( 0 );
					pChar->removeFromView( false );
					pChar->resend( false );
				}
			}		
		}

		// Now for the items
		if( ( checkitemstime <= currenttime ) || overflow )
		{
			RegionIterator4Items itemIter( socket->player()->pos );
			for( itemIter.Begin(); !itemIter.atEnd(); itemIter++ )
			{
				P_ITEM pItem = itemIter.GetData();
	
				if( !pItem )
					continue;
			
				Items->RespawnItem( currenttime, pItem ); // Checks if the item is a spawner
				Items->DecayItem( currenttime, pItem ); // Checks if the item should decay

				switch( pItem->type() )
				{
				case 51:
				case 52:
					if( pItem->gatetime < currenttime )
						Items->DeleItem( pItem );
					break;

				case 88:
					if( pItem->pos.distance( socket->player()->pos ) < pItem->morey )
						if( RandomNum( 1, 100 ) <= pItem->morez )
							socket->soundEffect( pItem->morex, pItem );
					break;
				case 117:
					if( ( pItem->tags.get( "tiller" ).isValid() && 
						( pItem->gatetime <= currenttime ) ) || overflow )
					{
						cBoat* pBoat = dynamic_cast< cBoat* >( FindItemBySerial( pItem->tags.get( "boatserial" ).toUInt() ) );
						if( pBoat )
						{
							pBoat->move();
							pItem->gatetime=(UINT32)( currenttime + (double)( SrvParams->boatSpeed() * MY_CLOCKS_PER_SEC ) );
						}
					}
					break;
				};				
			}
		}
	}

	// Check the TempEffects
	cTempEffects::getInstance()->check();

	if ( freeUnusedMemory <= currenttime )
	{
		cItemsManager::getInstance()->purge();
		cCharsManager::getInstance()->purge();
		cGumpsManager::getInstance()->purge();
		freeUnusedMemory = currenttime + MY_CLOCKS_PER_SEC*60*40; // check only each 40 minutes
	}

	if(checknpcs<=currenttime) checknpcs=(unsigned int)((double)(SrvParams->checkNPCTime()*MY_CLOCKS_PER_SEC+currenttime)); //lb
	if(checktamednpcs<=currenttime) checktamednpcs=(unsigned int)((double) currenttime+(SrvParams->checkTammedTime()*MY_CLOCKS_PER_SEC)); //AntiChrist
	if(checknpcfollow<=currenttime) checknpcfollow=(unsigned int)((double) currenttime+(SrvParams->checkFollowTime()*MY_CLOCKS_PER_SEC)); //Ripper
	if(checkitemstime<=currenttime) checkitemstime=(unsigned int)((double)(SrvParams->checkItemTime()*MY_CLOCKS_PER_SEC+currenttime)); //lb

	if (nextnpcaitime <= currenttime)
		nextnpcaitime = (unsigned int)((double) currenttime + (SrvParams->checkAITime()*MY_CLOCKS_PER_SEC)); // lb
	if (nextfieldeffecttime <= currenttime)
		nextfieldeffecttime = (unsigned int)((double) currenttime + (0.5*MY_CLOCKS_PER_SEC));
	if (nextdecaytime <= currenttime)
		nextdecaytime = currenttime + (15*MY_CLOCKS_PER_SEC); // lb ...
}
