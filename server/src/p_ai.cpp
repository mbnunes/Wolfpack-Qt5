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

#include "wolfpack.h"
#include "debug.h"
#include "mapobjects.h"
#include "srvparams.h"
#include "classes.h"
#include "territories.h"
#include "msgboard.h"

using namespace std;

#undef  DBGFILE
#define DBGFILE "p_ai.cpp"

void cCharStuff::CheckAI( unsigned int currenttime, P_CHAR pc_i )
{
	int d;
	unsigned int chance;
	if ( pc_i == NULL )
		return;
	
	if ( nextnpcaitime > currenttime )
		return;

	if( pc_i->targ() == INVALID_SERIAL && pc_i->war() )
		pc_i->toggleCombat();

	switch( pc_i->npcaitype() )
	{
		case 0: // Shopkeepers greet players..Ripper
			if (SrvParams->vendorGreet() == 1 && pc_i->isNpc() && pc_i->shop() && pc_i->isHuman())
			{
				RegionIterator4Chars ri(pc_i->pos());
				for (ri.Begin(); !ri.atEnd(); ri++)
				{
					P_CHAR pc = ri.GetData();
					if (pc != NULL)
					{
						d = pc_i->dist( pc );
						if (d > 3)
							continue;
						if (pc->isNpc() || !online(pc))
							continue;
						if (pc->isInvul() || pc->dead() || !pc->isInnocent())
							continue;
						pc_i->talk( tr("Hello %1, Welcome to my shop, How may i help thee?.").arg( pc->name() ), -1, 0, true );
						pc_i->setAntispamtimer(uiCurrentTime + MY_CLOCKS_PER_SEC*30);
					}
				}
			}
			break;
		case 1: // good healers
			if( !pc_i->war() )
			{
				RegionIterator4Chars ri(pc_i->pos());
				for (ri.Begin(); !ri.atEnd(); ri++)
				{
					P_CHAR pc = ri.GetData();
					if( pc )
					{
						if( !pc->inRange( pc_i, 3 ) )
							continue;

						if( pc->isNpc() || !pc->socket() )
							continue;
						if( !pc->dead() )
							continue;
						if( pc->isMurderer() )
						{
							pc_i->talk( "I will nay give life to a scoundrel like thee!" );
							return;
						}
						else if (pc->isCriminal()) 
						{
							pc_i->talk( "I will nay give life to thee for thou art a criminal!" );
							return;
						}
						else if( pc->isInnocent() )
						{
							pc_i->action( 0x10 );
							pc->resurrect();
							pc->effect( 0x376A, 0x09, 0x06 );
							switch (RandomNum(0, 4)) 
							{
							case 0: 
								pc_i->talk( "Thou art dead, but 'tis within my power to resurrect thee.  Live!" );
								break;
							case 1: 
								pc_i->talk( "Allow me to resurrect thee ghost.  Thy time of true death has not yet come." );
								break;
							case 2: 
								pc_i->talk( "Perhaps thou shouldst be more careful.  Here, I shall resurrect thee." );
								break;
							case 3: 
								pc_i->talk( "Live again, ghost!  Thy time in this world is not yet done." );
								break;
							case 4: 
								pc_i->talk( "I shall attempt to resurrect thee." );
								break;
							}
						}
					}
				}
			}
			break;
		case 2 : // Monsters, PK's - (stupid NPCs)
			if( !pc_i->war() )
			{
				// Get the one with the least distance!
				P_CHAR Victim = NULL;
				UI32 minDist;

				RegionIterator4Chars ri( pc_i->pos(), SrvParams->attack_distance() );
				for (ri.Begin(); !ri.atEnd(); ri++)
				{
					P_CHAR pc = ri.GetData();
					
					if( !pc )
						continue;

					chance = RandomNum(1, 100);
					UI32 d = pc_i->dist( pc );
					
					if( ( !pc->isNpc() ) && ( !online( pc ) ) )
						continue;

					if ( pc->isInvul() || pc->isHidden() || pc->dead() )
						continue;

					if ( pc->isNpc() && ( pc->npcaitype() == 2 || pc->npcaitype() == 1 ) )
						continue;

					if ( SrvParams->monsters_vs_animals() == 0 && ( pc->title().isEmpty() && !pc->isHuman() ) )
						continue;

					if ( SrvParams->monsters_vs_animals() == 1 && chance > SrvParams->animals_attack_chance() )
						continue;

					// If the distance is below the minimal distance we found
					if( ( ( Victim == NULL ) || ( minDist > d ) ) )
					{
						Victim = pc;
						minDist = d;
					}

				}

				if (pc_i->baseSkill(MAGERY)>400)
				{
					if (pc_i->hp() <(pc_i->st()/2))
					{
						pc_i->talk( tr("In Vas Mani"), -1, 0 );
						Magic->NPCHeal(pc_i);
					}
					else if (pc_i->poisoned())
					{
						pc_i->talk( tr("An Nox"), -1, 0 );
						Magic->NPCCure(pc_i);
					}
					else if ( Victim && Victim->priv2()&0x20 )
					{
						pc_i->talk( tr("An Ort"), -1, 0 );
						Magic->NPCDispel(pc_i, Victim);
					}
				}

				// We found a victim
				if( Victim )
					pc_i->attackTarget( Victim );

				return;
			}
			break;
		case 3 : // Evil Healers
			if (!pc_i->war())
			{
				RegionIterator4Chars ri(pc_i->pos());
				for (ri.Begin(); !ri.atEnd(); ri++)
				{
					P_CHAR pc = ri.GetData();
					if (pc != NULL)
					{
						d = pc_i->dist( pc );
						if (d > 3)
							continue;
						if (pc->isNpc() || !online(pc))
							continue;
						if (!pc->dead())
							continue;
						if (pc->isInnocent())
						{
							pc_i->talk( tr("I dispise all things good. I shall not give thee another chance!"), -1, 0 );
							return;
						}
						else
						{
							pc_i->action(0x10);
							Targ->NpcResurrectTarget(pc);
							pc->effect(0x3709, 0x09, 0x19);
							switch (RandomNum(0, 4)) 
							{
							case 0: pc_i->talk( tr("Fellow minion of Mondain, Live!!"), -1, 0);												break;
							case 1: pc_i->talk( tr("Thou has evil flowing through your vains, so I will bring you back to life."), -1, 0);	break;
							case 2: pc_i->talk( tr("If I res thee, promise to raise more hell!."), -1, 0);									break;
							case 3: pc_i->talk( tr("From hell to Britannia, come alive!."), -1, 0);											break;
							case 4: pc_i->talk( tr("Since you are Evil, I will bring you back to consciouness."), -1, 0);					break;
							}
						}
					}
				}
			}
			break;
		case 4 : // Teleporting Guards
			if (!pc_i->war()	// guard isnt busy 
				&& pc_i->inGuardedArea())	// this region is guarded
			{	// this bracket just to keep compiler happy

				P_CHAR Victim = NULL;
				UI32 minDist;

				RegionIterator4Chars ri(pc_i->pos());
				for (ri.Begin(); !ri.atEnd(); ri++)
				{
					P_CHAR pc = ri.GetData();
					if (pc != NULL)
					{
						d = pc->dist( pc_i );

						if( ( !pc->isNpc() ) && ( !online( pc ) ) )
						    continue;
						if (pc_i == pc || d > SrvParams->attack_distance() || pc->isInvul() || pc->dead())
							continue;
						if (!pc->inGuardedArea())
							continue;
						// If the distance is below the minimal distance we found
					    if( ( Victim == NULL ) || ( minDist > d ) )
						{
						   Victim = pc;
						   minDist = d;
						}
						if (pc->isPlayer() && pc->crimflag() > 0 && d <= 3)
						{
							pc_i->talk( tr("You better watch your step %1, I am watching thee!!").arg( pc->name() ), -1, 0, true );
							pc_i->setAntispamtimer( uiCurrentTime + MY_CLOCKS_PER_SEC*30 );
						}
						else if (pc->isPlayer() && pc->isInnocent() && d <= 3)
						{
							pc_i->talk( tr("%1 is an upstanding citizen, I will protect thee in %2.").arg( pc->name() ).arg( pc->region()->name() ), -1, 0, true );
							pc_i->setAntispamtimer( uiCurrentTime + MY_CLOCKS_PER_SEC*30 );
						}
						else if (d <= SrvParams->attack_distance() &&(
							(pc->isNpc() &&(pc->npcaitype() == 2))	// evil npc
							||(pc->isPlayer() && pc->isMurderer() && !(pc->isInnocent()) || pc->isCriminal()))	// a player,is murderer & not grey or blue
							||(pc->attackfirst()))	// any agressor
						{
							pc_i->moveTo( pc->pos() );
							pc_i->soundEffect( 0x01FE );
							pc_i->effect( 0x372A, 0x09, 0x06 );
							// We found a victim
				            if( Victim )
							 pc_i->attackTarget( Victim );
							pc_i->talk( tr("Thou shalt regret thine actions, swine!"), -1, 0 );
							return;
						}
					}
				}
			}
			break;
		case 5: // npc beggars
			if (!pc_i->war())
			{
				RegionIterator4Chars ri(pc_i->pos());
				for (ri.Begin(); !ri.atEnd(); ri++)
				{
					P_CHAR pc = ri.GetData();
					if (pc != NULL)
					{
						d = pc->dist( pc_i );
						if (d > 3)
							continue;
						if (pc->isNpc() || !online(pc))
							continue;
						if (pc->isInvul() || pc->dead() || !pc->isInnocent())
							continue;
						switch (RandomNum(0, 2))
						{
						case 0: pc_i->talk( tr("Could thou spare a few coins?"), -1, 0, true );					break;
						case 1: pc_i->talk( tr("Hey buddy can you spare some gold?"), -1, 0, true );				break;
						case 2: pc_i->talk( tr("I have a family to feed, think of the children."), -1, 0, true );	break;
						}
						pc_i->setAntispamtimer(uiCurrentTime + MY_CLOCKS_PER_SEC*30);
					}
				}
			}
			break;
		case 6: 
			break; // Ripper- chaos guards.
		case 7: 
			break; // Ripper- order guards.
		case 8: 
			break; // morrolan - old banker
		case 9 : // in world guards, they dont teleport out...Ripper
			if (!pc_i->war()	// guard isnt busy 
				&& pc_i->inGuardedArea())	// this region is guarded
			{	// this bracket just to keep compiler happy

				P_CHAR Victim = NULL;
				UI32 minDist;

				RegionIterator4Chars ri(pc_i->pos());
				for (ri.Begin(); !ri.atEnd(); ri++)
				{
					P_CHAR pc = ri.GetData();
					if (pc != NULL)
					{
						d = pc->dist( pc_i );

						if( ( !pc->isNpc() ) && ( !online( pc ) ) )
						    continue;
						if (pc_i == pc || d > SrvParams->attack_distance() || pc->isInvul() || pc->dead())
							continue;
						// If the distance is below the minimal distance we found
					    if( ( Victim == NULL ) || ( minDist > d ) )
						{
						   Victim = pc;
						   minDist = d;
						}
						if (pc->isPlayer() && pc->crimflag() > 0 && d <= 3)
						{
							pc_i->talk( tr("You better watch your step %1, I am watching thee!!").arg(pc->name()), -1, 0, true );
							pc_i->setAntispamtimer(uiCurrentTime + MY_CLOCKS_PER_SEC*30);
						}
						else if (pc->isPlayer() && pc->isInnocent() && d <= 3)
						{
							pc_i->talk( tr("%1 is an upstanding citizen, I will protect thee in %2.").arg(pc->name()).arg(pc->region()->name()), -1, 0, true );
							pc_i->setAntispamtimer(uiCurrentTime + MY_CLOCKS_PER_SEC*30);
						}
						else if (d <= SrvParams->attack_distance() &&(
							(pc->isNpc() &&(pc->npcaitype() == 2))	// evil npc
							||(pc->isPlayer() && pc->isMurderer() && !(pc->isInnocent()) || pc->isCriminal()))	// a player,is murderer & not grey or blue
							||(pc->attackfirst()))	// any agressor
						{
							pc_i->moveTo( pc->pos() );
							pc_i->soundEffect( 0x01FE );
							pc_i->effect( 0x372A, 0x09, 0x06 );
							// We found a victim
				            if( Victim )
							pc_i->attackTarget( Victim );
							pc_i->talk( tr("Thou shalt regret thine actions, swine!"), -1, 0 );
							return;
						}
					}
				}
			}
			break;
		case 10: // Tamed Dragons ..not white wyrm..Ripper
			// so regular dragons attack reds on sight while tamed.
			if (pc_i->isNpc() && pc_i->tamed())
			{
				RegionIterator4Chars ri(pc_i->pos());
				for (ri.Begin(); !ri.atEnd(); ri++)
				{
					P_CHAR pc = ri.GetData();
					if (pc != NULL)
					{
						d = pc->dist( pc_i );
						if (d > SrvParams->attack_distance())
							continue;
						if (pc->isPlayer())
							continue;
						if (pc->isNpc() && pc->npcaitype() != 2)
							continue;
						pc_i->attackTarget( pc );
						return;
					}
				}
			}
			break;
		case 11 : // add NPCAI B in scripts to make them attack reds. (Ripper)
			if (!pc_i->war())
			{
				RegionIterator4Chars ri(pc_i->pos());
				for (ri.Begin(); !ri.atEnd(); ri++)
				{
					P_CHAR pc = ri.GetData();
					if (pc != NULL)
					{
						d = pc->dist( pc_i );
						if (d > SrvParams->attack_distance())
							continue;
						if( ( !pc->isNpc() ) && ( !online( pc ) ) )
						    continue;
						if (pc->isInvul() || pc->dead())
							continue;
						if (!(pc->npcaitype() == 2 || pc->isMurderer()))
							continue;
						pc_i->attackTarget( pc );
					}
				}
			}
			break;
		case 17: 
			break; // Zippy Player Vendors.
		case 18: // Ripper.. Escort speech.
//		if (!pc_i->war() && pc_i->questType() == ESCORTQUEST)
//		{
//			RegionIterator4Chars ri(pc_i->pos);
//			for (ri.Begin(); !ri.atEnd(); ri++)
//			{
//				P_CHAR pc = ri.GetData();
//				if (pc != NULL)
//				{
//				    d = pc->dist( pc_i );
//					if (d > 10)
//					    continue;
//					if( ( pc->isNpc() ) && ( !online( pc ) ) )
//						    continue;
//				    if (pc->dead())
//					    continue;
//
//					pc_i->talk( tr("I am waiting for my escort to %1, Will you take me?").arg( pc_i->questDestRegion() ), -1 , 0 );
//				    pc_i->setAntispamtimer(uiCurrentTime+MY_CLOCKS_PER_SEC*30);
//				    return;
//				}
//			}
//		}
		break;
		case 19:
			break; // real estate broker...Ripper
		case 32:
			// We don't have anything to do
			// So let's attack what has attacked our guarding target
			if( pc_i->isNpc() && pc_i->guarding() && !pc_i->war() && pc_i->targ() == INVALID_SERIAL )
			{
				P_CHAR pGuarded = pc_i->guarding();

				if( pGuarded->attacker() != INVALID_SERIAL )
				{
					P_CHAR pAttacker = FindCharBySerial( pGuarded->attacker() );
					if( pAttacker )
						pGuarded->fight( pAttacker );
				}
			}
			break;
		   case 30:// no idea?
		   case 50:// EV/BS
			if (!pc_i->war())
			{
				RegionIterator4Chars ri(pc_i->pos());
				for (ri.Begin(); !ri.atEnd(); ri++)
				{
					P_CHAR pc = ri.GetData();
					if (pc != NULL)
					{
						d = pc->dist(pc_i);
						if (d > SrvParams->attack_distance())
							continue;
						if( ( !pc->isNpc() ) && ( !online( pc ) ) )
						    continue;
						if (pc->isInvul() || pc->dead())
							continue;
						pc_i->attackTarget( pc );
						return;
					}
				}
			}
			break;
		case 96:
		case 60: // Skyfire - Dragon AI
			break;
		default:
			clConsole.send("ERROR: cCharStuff::CheckAI-> Error npc (%8x) has invalid AI type %i\n", pc_i->serial(), pc_i->npcaitype()); // Morrolan
			return;
	}// switch
}// void checknpcai

