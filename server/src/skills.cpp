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

/* 
*  WOLFPACK Skills
*/

#include "wolfpack.h"
#include "wpdefmanager.h"
#include "basics.h"
#include "itemid.h"
#include "im.h"
#include "SndPkg.h"
#include "guildstones.h"
#include "tracking.h"
#include "tilecache.h"
#include "combat.h"
#include "targetrequests.h"
#include "mapobjects.h"
#include "srvparams.h"
#include "wpscriptmanager.h"
#include "skills.h"
#include "network.h"
#include "mapstuff.h"
#include "network/uosocket.h"
#include "classes.h"

#undef DBGFILE
#define DBGFILE "skills.cpp"
#include "debug.h"

/*
List of implemented skills:
Anatomy
Animal Lore
Arms Lore

*/

// This is the target-request for Anatomy
class cSkAnatomy: public cTargetRequest
{
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		P_CHAR pChar = socket->player();

		if( !pChar )
			return true;

		P_CHAR aChar = FindCharBySerial( target->serial() );

		if( !aChar )
		{
			socket->sysMessage( tr( "You need to examine that" ) );
			return true;
		}

		// Examination on yourself 
		if( pChar == aChar )
		{
			pChar->message( tr( "You know yourself well enough." ) );
			return true;
		}

		// Are we in range?
		if( !pChar->inRange( aChar, 9 ) )
		{
			pChar->message( tr( "You are too far away to examine that." ) );
			return true;
		}

		// Check the Skill
		if( !pChar->checkSkill( ANATOMY, 0, 1000 ) ) 
		{
			pChar->message( tr( "You are not certain." ) );
			return true;
		}

		register UINT16 dex = aChar->effDex();
		register UINT16 str = aChar->st();

		// No Str & no Dex = Not living
		if( !str && !dex )
		{
			socket->showSpeech( aChar, tr( "It does not appear to be a living being." ) );
			return true;
		}

		QString part1, part2;
		if		(str <= 10)	part1 = tr( "like they would have trouble lifting small objects" );
		else if (str <= 20)	part1 = tr( "rather feeble" );
		else if (str <= 30)	part1 = tr( "somewhat weak" );
		else if (str <= 40)	part1 = tr( "to be of normal strength" );
		else if (str <= 50)	part1 = tr( "somewhat strong" );
		else if (str <= 60)	part1 = tr( "very strong" );
		else if (str <= 70)	part1 = tr( "extremely strong" );
		else if (str <= 80)	part1 = tr( "extraordinarily strong" );
		else if (str <= 90)	part1 = tr( "strong as an ox" );
		else if (str <= 99)	part1 = tr( "one of the strongest people you have ever seen" );
		else if (str >=100) part1 = tr( "superhumanly strong" );

		if		(dex <= 10)	part2 = tr( "like they barely manage to stay standing" );
		else if (dex <= 20)	part2 = tr( "very clumsy" );
		else if (dex <= 30)	part2 = tr( "somewhat uncoordinated" );
		else if (dex <= 40)	part2 = tr( "moderately dextrous" );
		else if (dex <= 50)	part2 = tr( "somewhat agile" );
		else if (dex <= 60)	part2 = tr( "very agile" );
		else if (dex <= 70)	part2 = tr( "extremely agile" );
		else if (dex <= 80)	part2 = tr( "extraordinarily agile" );
		else if (dex <= 90)	part2 = tr( "moves like quicksilver" );
		else if (dex <= 99) part2 = tr( "one of the fastest people you have ever seen" );
		else if (dex >=100) part2 = tr( "superhumanly agile" );
		
		socket->showSpeech( aChar, tr( "This person looks %1 and %2." ).arg( part1 ).arg( part2 ) );
		return true;
	}
};

// This is the target-request for ArmsLore
class cSkArmsLore: public cTargetRequest
{
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		P_ITEM pItem = FindItemBySerial( target->serial() );
		P_CHAR pChar = socket->player();

		if( !pChar )
			return true;

		if( !pItem || !target->serial() )
		{
			socket->sysMessage( tr( "You need to target an item" ) );
			return true;
		}

		if( ( ( pItem->lodamage() == 0 ) && ( pItem->hidamage() == 0 ) ) && !pItem->def )
		{
			socket->sysMessage( tr( "This does not appear to be a weapon" ) );
			return true;
		}

		if( pChar->isGM() )
		{
			socket->sysMessage( tr("Attack [%1] Defense [%2] Lodamage [%3] Hidamage [%4]").arg( pItem->att ).arg( pItem->def ).arg( pItem->lodamage() ).arg( pItem->hidamage() ) );
			return true;
		}

		if( !pChar->checkSkill( ARMSLORE, 0, 250 ) )
		{
			socket->sysMessage( tr( "You are not certain..." ) );
			return true;
		}

		QStringList mParts;

		// You get the HP Status between 0 and 25%
		if( pItem->maxhp() )
		{
			float totalHp = (float)( pItem->hp() / pItem->maxhp() );
			QString status;
			
			if      (totalHp>0.9) status = tr( "is brand new" ); 
			else if (totalHp>0.8) status = tr( "is almost new" );
			else if (totalHp>0.7) status = tr( "is barely used, with a few nicks and scrapes" );
			else if (totalHp>0.6) status = tr( "is in fairly good condition" );
			else if (totalHp>0.5) status = tr( "suffered some wear and tear" );
			else if (totalHp>0.4) status = tr( "is well used" );
			else if (totalHp>0.3) status = tr( "is rather battered." );
			else if (totalHp>0.2) status = tr( "is somewhat badly damaged." );
			else if (totalHp>0.1) status = tr( "is flimsy and not trustworthy." );
			else                  status = tr( "is falling apart." );

			mParts.push_back( tr( "%1 [%2%%]" ).arg( status ).arg( totalHp*100, 0, 'f', 0 ) );
		}

		// You get the Damage status between 25% and 51%
		if( pChar->checkSkill( ARMSLORE, 250, 510, false ) )
		{
			if( pItem->hidamage() && pItem->lodamage() )
			{
				UINT32 mid = (UINT32)( ( pItem->hidamage() + pItem->lodamage() ) / 2 );
				QString status;
				if      ( mid > 26 ) status = tr( "would be extraordinarily deadly" );
				else if ( mid > 21 ) status = tr( "would be a superior weapon" );
				else if ( mid > 16 ) status = tr( "would inflict quite a lot of damage and pain" ); 
				else if ( mid > 11 ) status = tr( "would probably hurt your opponent a fair amount" );
				else if ( mid > 6 )  status = tr( "would do some damage" );
				else if ( mid > 3 )  status = tr( "would do minimal damage" );
				else                 status = tr( "might scratch your opponent slightly" );

				mParts.push_back( status );

				// The weapon speed is displayed between 50% and 100%
				if( pChar->checkSkill( ARMSLORE, 500, 1000, false ) )
				{
					if( pItem->speed() > 32 )		status = tr( "is very fast" );
					else if( pItem->speed() > 25 )	status = tr( "is fast" );
					else if( pItem->speed() > 15 )	status = tr( "is slow" );
					else							status = tr( "is very slow ");
					mParts.push_back( status );
				}
			}
			// Armor ratings
			else if( pItem->def )
			{
				QString status;

				if      ( pItem->def > 12) status = tr( "is superbly crafted to provide maximum protection" );
				else if ( pItem->def > 10) status = tr( "offers excellent protection" );
				else if ( pItem->def > 8 ) status = tr( "is a superior defense against attack" );
				else if ( pItem->def > 6 ) status = tr( "serves as a sturdy protection" );
				else if ( pItem->def > 4 ) status = tr( "offers some protection against blows" );
				else if ( pItem->def > 2 ) status = tr( "provides very little protection" );
				else if ( pItem->def > 0 ) status = tr( "provides almost no protection" );
				else					   status = tr( "offers no defense against attackers" );

				mParts.push_back( status );
			}
		}
	
		if( mParts.count() == 0 )
		{
			socket->sysMessage( tr( "You are not sure..." ) );
			return true;
		}

		// Build a human readable sentence
		if( mParts.count() < 3 )
			socket->sysMessage( tr( "The item %1." ).arg( mParts.join( tr( " and " ) ) ) );
		else
		{
			QString lastPart = mParts.last();
			mParts.remove( mParts.back() );
			socket->sysMessage( tr( "The item %1 and %2" ).arg( mParts.join( ", " ) ).arg( lastPart ) );
		}

		// Display the rank if there is one (Between 25% and 50%)
		if( ( pItem->rank > 0 ) && ( pItem->rank < 11 ) && SrvParams->rank_system() && pChar->checkSkill( ARMSLORE, 250, 500, false ) )
		{
			switch( pItem->rank )
			{
				case 1: socket->sysMessage( tr("It seems an item with no quality.") );				break;
				case 2: socket->sysMessage( tr("It seems an item very below standard quality.") );	break;
				case 3: socket->sysMessage( tr("It seems an item below standard quality.") );		break;
				case 4: socket->sysMessage( tr("It seems a weak quality item.") );					break;
				case 5: socket->sysMessage( tr("It seems a standard quality item.") );				break;
				case 6: socket->sysMessage( tr("It seems a nice quality item.") );					break;
				case 7: socket->sysMessage( tr("It seems a good quality item.") );					break;
				case 8: socket->sysMessage( tr("It seems a great quality item.") );					break;
				case 9: socket->sysMessage( tr("It seems a beautiful quality item.") );				break;
				case 10:socket->sysMessage( tr("It seems a perfect quality item.") );				break;
			}
		}
		return true;
	}
};

// Detecting hidden
class cSkDetectHidden: public cTargetRequest
{
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		P_CHAR pChar = socket->player();

		if( !pChar )
			return true;

		Coord_cl dPos = pChar->pos;
		dPos.x = target->x();
		dPos.y = target->y();
		dPos.z = target->z();

		// If its out of the characters visrange cancel (How could he've clicked there??)
		if( dPos.distance( pChar->pos ) > pChar->VisRange )
			return true;
        
		UINT16 dSkill = pChar->skill( DETECTINGHIDDEN );		
		double range = ( dSkill * dSkill / 1.0E6 ) * VISRANGE; // this seems like an ok formula
		
		RegionIterator4Chars ri( dPos );
        bool found = false;

		for( ri.Begin(); !ri.atEnd(); ri++ )
		{
			P_CHAR hChar = ri.GetData();
			if( hChar )
			{
				if( hChar->hidden() && !hChar->isHiddenPermanently() ) // do not detect invis people only hidden ones
				{
					UINT16 dx = abs( hChar->pos.x - dPos.x );
					UINT16 dy = abs( hChar->pos.y - dPos.y );
					double c = hypot( dx, dy );

					INT16 low = (UINT16)( hChar->skill(HIDING) * hChar->skill(HIDING) / 1E3 - ( range * 50 / VISRANGE ) * ( range - c ) / range );
					if( low < 0 ) 
						low = 0;
					else if( low > 1000 )
						low = 1000;
					
					if( ( pChar->checkSkill( DETECTINGHIDDEN, low, 1000 ) ) && ( c <= range ) )
					{
						hChar->unhide();
						hChar->message( tr( "You have been revealed!" ) );
						found = true;
					}
				}
			}
		}

		if( !found )
			socket->sysMessage( tr( "You didn't find anything hidden" ) );
		return true;
	}
};

// Item Identification
class cSkItemID: public cTargetRequest
{
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		P_CHAR pc_currchar = socket->player();
		const P_ITEM pi = FindItemBySerial( target->serial() );
		if ( !pi )
		{
			socket->sysMessage( tr("Unable to identify that.") );
			return true;
		}

		if ( !pi->isLockedDown() ) // Ripper
		{
			if (!pc_currchar->checkSkill( ITEMID, 0, 250))
			{
				socket->sysMessage( tr("You can't quite tell what this item is...") );
			}
			else
			{
				if( pi->corpse() )
				{
					socket->sysMessage( tr("You have to use your forensics evalutation skill to know more on this corpse.") );
					return true;
				}
				
				// Identify Item by Antichrist // Changed by MagiusCHE)
				if ( pc_currchar->checkSkill( ITEMID, 250, 500) )
				{
					if (pi->name2() == "#") 
						pi->setName( pi->name2() );
					
					socket->sysMessage( tr("You found that this item appears to be called: %1").arg(pi->name()) );
					
					// Show Creator by Magius(CHE)
					if (pc_currchar->checkSkill( ITEMID, 250, 500, false ))
					{
						if (pi->creator.size()>0)
						{
							if (pi->madewith>0) 
								socket->sysMessage( tr("It is %1 by %2").arg(skill[pi->madewith-1].madeword).arg(pi->creator.c_str()) ); // Magius(CHE)
							else if (pi->madewith<0) 
								socket->sysMessage( tr("It is %1 by %2").arg(skill[0-pi->madewith-1].madeword).arg(pi->creator.c_str()) ); // Magius(CHE)
							else 
								socket->sysMessage( tr("It is made by %1").arg(pi->creator.c_str()) ); // Magius(CHE)
						} else 
							socket->sysMessage( tr("You don't know its creator!") );
					} else 
						socket->sysMessage( tr("You can't know its creator!") );
					// End Show creator
					
					if (!pc_currchar->checkSkill( ITEMID, 250, 500, false ))
					{
						socket->sysMessage( tr("You can't tell if it is magical or not.") );
					}
					else
					{
						if(pi->type()!=15)
						{
							socket->sysMessage( tr("This item has no hidden magical properties.") );
						}
						else
						{
							if (!pc_currchar->checkSkill( ITEMID, 500, 1000, false ))
							{
								socket->sysMessage( tr("This item is enchanted with a spell, but you cannot determine which") );
							}
							else
							{
								if (!pc_currchar->checkSkill( ITEMID, 750, 1100, false ))
								{
									socket->sysMessage( tr("It is enchanted with the spell %1, but you cannot determine how many charges remain.").arg(spellname[(8*(pi->morex-1))+pi->morey-1]) );
								}
								else
								{
									socket->sysMessage( tr("It is enchanted with the spell %1, and has %2 charges remaining.").arg(spellname[(8*(pi->morex-1))+pi->morey-1]).arg(pi->morez) );
								}
							}
						}
					}
				}
			}
		}
		return true;
	}
};


// Evaluate Inteligence
class cSkIntEval: public cTargetRequest
{
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		P_CHAR pc = FindCharBySerial( target->serial() );
		P_CHAR pc_currchar = socket->player();
		if(pc == NULL || pc_currchar == NULL) 
			return true;
		
		if (pc == pc_currchar)
		{ 
			socket->sysMessage( tr("You cannot analyze yourself!") ); 
			return true; 
		}
		
		
		// blackwind distance fix 
		if( pc->pos.distance(pc_currchar->pos) >= 10 ) 
		{ 
			socket->sysMessage( tr("You need to be closer to find out") ); 
			return true; 
		} 
		
		if (!pc_currchar->checkSkill(EVALUATINGINTEL, 0, 1000)) 
		{
			socket->sysMessage( tr("You are not certain..") );
			return true;
		}
		if( !pc->in() )
			socket->sysMessage( tr("It looks smarter than a rock, but dumber than a piece of wood.") );
		else
		{
			if		(pc->in() <= 10)	socket->sysMessage( tr("That person looks slightly less intelligent than a rock.") );
			else if (pc->in() <= 20)	socket->sysMessage( tr("That person looks fairly stupid.") );
			else if (pc->in() <= 30)	socket->sysMessage( tr("That person looks not the brightest.") );
			else if (pc->in() <= 40)	socket->sysMessage( tr("That person looks about average.") );
			else if (pc->in() <= 50)	socket->sysMessage( tr("That person looks moderately intelligent.") );
			else if (pc->in() <= 60)	socket->sysMessage( tr("That person looks very intelligent.") );
			else if (pc->in() <= 70)	socket->sysMessage( tr("That person looks extremely intelligent.") );
			else if (pc->in() <= 80)	socket->sysMessage( tr("That person looks extraordinarily intelligent.") );
			else if (pc->in() <= 90)	socket->sysMessage( tr("That person looks like a formidable intellect, well beyond the ordinary.") );
			else if (pc->in() <= 99)	socket->sysMessage( tr("That person looks like a definite genius.") );
			else if (pc->in() >=100)  socket->sysMessage( tr("That person looks superhumanly intelligent in a manner you cannot comprehend.") );
		}
		return true;
	}
};


// Tame
class cSkTame: public cTargetRequest
{
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		
		P_CHAR pc = FindCharBySerial(target->serial());
		if ( pc == NULL ) 
			return true; 
		P_CHAR pc_currchar = socket->player();
		
		if( !lineOfSight( pc_currchar->pos, pc->pos, WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING ) )
			return true;
		
		bool tamed = false;
		if ((pc->isNpc() && (chardist(pc_currchar, pc) <= 3))) //Ripper
		{
			if (pc->taming>1200||pc->taming==0)//Morrolan default is now no tame
			{
				socket->sysMessage( tr("You can't tame that creature.") );
				return true;
			}
			// Below... can't tame if you already have!
			if( (pc->tamed()) && pc_currchar->Owns(pc) )
			{
				socket->sysMessage( tr("You already control that creature!" ) );
				return true;
			}
			if( pc->tamed() )
			{
				socket->sysMessage( tr("That creature looks tame already." ) );
				return true;
			}
			sprintf((char*)temp, "*%s starts to tame %s*",pc_currchar->name.c_str(),pc->name.c_str());
			for(int a=0;a<3;a++)
			{
				switch(rand()%4)
				{
				case 0: pc_currchar->talk( tr("I've always wanted a pet like you."), -1 ,0);		break;
				case 1: pc_currchar->talk( tr("Will you be my friend?"), -1 ,0);					break;
				case 2: pc_currchar->talk( tr("Here %1.").arg(pc->name.c_str()), -1 ,0);			break;
				case 3: pc_currchar->talk( tr("Good %1.").arg(pc->name.c_str()), -1 ,0);			break;
				default: 
					LogError("switch reached default");
				}
			}
			if ((!pc_currchar->checkSkill(TAMING, 0, 1000))||
				(pc_currchar->skill(TAMING)<pc->taming)) 
			{
				socket->sysMessage( tr("You were unable to tame it.") );
				return true;
			}
			pc_currchar->talk(tr("It seems to accept you as it's master!"));
			tamed = true;
			pc->SetOwnSerial(pc_currchar->serial);
			pc->npcWander = 0;
			if( pc->id() == 0x000C || pc->id() == 0x003B )
			{
				if(pc->skin() != 0x0481)
				{
					pc->setNpcAIType( 10 );
					pc->setTamed(true);
					updatechar(pc);
				}
				else
				{
					pc->setNpcAIType( 0 );
					pc->setTamed(true);
					updatechar(pc);
				}
			}
		}
		if (!tamed) 
			socket->sysMessage( tr("You can't tame that!") );
		return true;
	}
};


// Begging
class cSkBegging: public cTargetRequest
{
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		int gold,x,y,realgold;
		char abort;
		P_CHAR pc_currchar = socket->player();
		
		P_CHAR pc = FindCharBySerial( target->serial() );
		if (!pc)
			return true;
		
		if(online(pc))
		{
			socket->sysMessage( tr("Maybe you should just ask.") );
			return true;
		}
		
		if(chardist(pc, pc_currchar)>=5)
		{
			socket->sysMessage( tr("You are not close enough to beg.") );
			return true;
		}
		
		if(pc->isHuman() && (pc->in() != 0)) //Used on human
		{
			if (pc->begging_timer()>=uiCurrentTime)
			{
				pc->talk(tr("Annoy someone else !"));
				return true;
			}
			
			switch ( RandomNum(0, 2))
			{
			case 0:		pc_currchar->talk( tr("Could thou spare a few coins?"), -1 ,0);						break;
			case 1:		pc_currchar->talk( tr("Hey buddy can you spare some gold?"), -1 ,0);				break;
			case 2:		pc_currchar->talk( tr("I have a family to feed, think of the children."), -1 ,0);	break;
			}
			
			if (!pc_currchar->checkSkill( BEGGING, 0, 1000))
			{
				socket->sysMessage( tr("They seem to ignore your begging plees.") );
			}
			else
			{
				pc->setBegging_timer( SrvParams->beggingTime() * MY_CLOCKS_PER_SEC + uiCurrentTime); 
				x=pc->skill(BEGGING)/50;
				
				if (x<1) x=1; 
				y=rand()%x;
				y+=RandomNum(1,4); 
				if (y>25) y=25;
				// pre-calculate the random amout of gold that is "targeted"
				
				P_ITEM pi_p = Packitem(pc);
				gold=0;
				realgold=0;
				abort=0;
				
				// check for gold in target-npc pack
				
				if (pi_p != NULL)				
				{
					unsigned int ci;
					vector<SERIAL> vecContainer = contsp.getData(pi_p->serial);
					for (ci = 0; ci < vecContainer.size(); ci++)
					{
						P_ITEM pi_j =  FindItemBySerial(vecContainer[ci]);
						if (pi_j != NULL)
						{
							if (pi_j->id()==0x0EED )
							{
								gold += pi_j->amount(); // calc total gold in pack
								
								int k = pi_j->amount();
								if(k>=y) // enough money in that pile in pack to satisfy pre-aclculated amount
								{
									pi_j->ReduceAmount( y );
									realgold += y; // calc gold actually given to player
									
									// This does not end in a crash !??
									//if( pi_j != NULL ) // Only if we still have an item
									pi_j->update(); // resend new amount
									
									abort = 1;
								}
								else // not enough money in this pile -> only delete it
								{
									Items->DeleItem( pi_j );
									pi_j->update(); // Refresh a deleted item?
									realgold += pi_j->amount();
								}
							}
						} // end of if j!=-1
						if (abort) break;
					} 
				}
				
				if (gold<=0)
				{				
					pc->talk( tr("Thou dost not look trustworthy... no gold for thee today! ") );
					return true;
				}
				pc->talk( tr("I feel sorry for thee... here have a gold coin ."), -1, 0 );
				addgold(toOldSocket(socket), realgold);
				socket->sysMessage( tr("Some gold is placed in your pack.") );
			}
		}
		else
			socket->sysMessage( tr("That would be foolish.") );
		return true;
	}
};


// Animal Lore
class cSkAnimalLore: public cTargetRequest
{
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		P_CHAR pc = FindCharBySerial( target->serial() );
		if( !pc ) return true;
		P_CHAR pc_currchar = socket->player();
		
		// blackwind distance fix 
		if( chardist( pc, pc_currchar ) >= 10 ) 
		{ 
			socket->sysMessage( tr("You need to be closer to find out more about them" ) ); 
			return true; 
		} 
		
		if (pc->isGMorCounselor())
		{
			socket->sysMessage( tr("Little is known of these robed gods.") );
			return true;
		}
		if (pc->isHuman()) // Used on human
		{
			socket->sysMessage( tr("The human race should use conversation!") );
			return true;
		}
		else // Lore used on a non-human
		{
			UI16 skill = pc_currchar->skill( ANIMALLORE );
			if( pc->tamed() || ( skill >= 1000 && skill <= 1100 && (pc->taming<=1200||pc->taming>0) ) ||
				skill >= 1100 )
			{
				if( pc_currchar->checkSkill( ANIMALLORE, 0, 1000 ) )
				{
					QString message = tr("Attack[%1-%2] Defense [%3] Hit Points [%5]. ").arg(pc->lodamage()).arg(pc->hidamage()).arg(pc->def()).arg(pc->hp());
					message += tr("It accepts");
					bool anyfood = false;
					register int bit;
					for( bit = 0; bit < enNumberOfFood; ++bit )
						if( pc->food() & ( 1 << bit ) )
						{
							anyfood = true;
							message += QString( " %1," ).arg( foodname[ bit+1 ] );
						}
					
					if( anyfood )
						message = message.left( message.length()-1 ); // cut the last ","
					else
						message += tr(" nothing");
					message += tr(" for food." );
					pc->talk( message );

					if( pc->ownserial() == pc_currchar->serial )
						pc->talk( tr("It is loyal to you!") );
					else
					{
						P_CHAR pOwner = FindCharBySerial( pc->ownserial() );
						QString ownername = "nobody";
						if( pOwner )
							ownername = QString( pOwner->name.c_str() );
						pc->talk( tr("It is loyal to %1!").arg( ownername ) );
					}
				}
				return true;
			}
			socket->sysMessage( tr("You can not think of anything relevant at this time.") );
		}
		return true;
	}
};

// Stealing
class cSkStealing: public cTargetRequest
{
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		int i, skill;
		char temp2[512];
		tile_st tile;
		P_CHAR pc_currchar = socket->player();
		int cansteal = QMAX( 1, pc_currchar->baseSkill( STEALING ) / 10 );
		cansteal = cansteal * 10;
		
		if (isCharSerial(target->serial()))
		{
			Skills->RandomSteal(socket, target->serial());
			return true;
		}
		
		const P_ITEM pi = FindItemBySerial(target->serial());
		if (!pi)
		{
			socket->sysMessage( tr("You cannot steal that.") );
			return true;
		}

		if ( pi->layer() != 0		// no stealing for items on layers other than 0 (equipped!) ,
			|| pi->newbie()			// newbie items,
			|| pi->isInWorld() )	// and items not being in containers allowed !
		{
			socket->sysMessage( tr("You cannot steal that.") );
			return true;
		}
		if( (pi->totalweight()/10) > cansteal ) // LB, bugfix, (no weight check)
		{
			socket->sysMessage( tr("That is too heavy.") );
			return true;
		}
		
		P_CHAR pc_npc = GetPackOwner(pi);
		
		if (pc_npc->npcaitype() == 17)
		{
			socket->sysMessage( tr("You cannot steal that.") );
			return true;
		}
		
		if (pc_npc == pc_currchar)
		{
			socket->sysMessage( tr("You catch yourself red handed.") );
			return true;
		}
		
		skill = pc_currchar->checkSkill(STEALING, 0, 999);
		if( pc_currchar->inRange( pc_npc, 1 ) )
		{
			if (skill)
			{
				P_ITEM pi_pack = Packitem(pc_currchar);
				if (pi_pack == NULL) 
					return true;
				pi->setContSerial(pi_pack->serial);
				socket->sysMessage( tr("You successfully steal that item.") );
				pi->update();
			} 
			else 
				socket->sysMessage( tr("You failed to steal that item.") );
			
			if (((!(skill))&&(rand()%16==7)) || (pc_currchar->skill(STEALING)<rand()%1001))
			{
				socket->sysMessage( tr("You have been cought!") );
				
				if (pc_npc != NULL) //lb
				{
					if (pc_npc->isNpc()) 
						pc_npc->talk( tr("Guards!! A thief is amoung us!"), -1, 0x09 );
					
					criminal( pc_currchar );
					
					if (pc_npc->isInnocent() && pc_currchar->attacker != pc_npc->serial && GuildCompare(pc_currchar, pc_npc)==0)//AntiChrist
						criminal(pc_currchar);//Blue and not attacker and not guild
					
					if (pi->name() != "#")
					{
						sprintf((char*)temp, tr("You notice %1 trying to steal %2 from you!").arg(pc_currchar->name.c_str()).arg(pi->name()) );
						sprintf((char*)temp2, tr("You notice %1 trying to steal %2 from %3!").arg(pc_currchar->name.c_str()).arg(pi->name()).arg(pc_npc->name.c_str()) );
					} 
					else
					{
						tile = cTileCache::instance()->getTile( pi->id() );
						sprintf((char*)temp, tr("You notice %1 trying to steal %2 from you!").arg(pc_currchar->name.c_str()).arg((char*)tile.name) );
						sprintf((char*)temp2,tr("You notice %1 trying to steal %2 from %3!").arg(pc_currchar->name.c_str()).arg((char*)tile.name).arg(pc_npc->name.c_str()) );
					}
					socket->sysMessage((char*)temp); //lb
				}
				for ( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next())
				{
					if( mSock != socket && inrange1p( pc_currchar, mSock->player() ) && (rand()%10+10==17|| (rand()%2==1 && mSock->player()->in() >= pc_currchar->in()))) 
						mSock->sysMessage(temp2);
				}
			}
		} 
		else 
			socket->sysMessage( tr("You are too far away to steal that item.") );
		return true;
	}
};


// Forensics Evaluation
class cSkForensics: public cTargetRequest
{
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		int curtim=uiCurrentTime;
		const PC_ITEM pi = FindItemBySerial(target->serial());
		P_CHAR pc_currchar = socket->player();
		
		if( !pi || !pi->corpse() )
		{
			socket->sysMessage( tr( "That does not appear to be a corpse." ) );
			return true;
		}
		
		if(pc_currchar->isGM())
		{
			socket->sysMessage( tr("The %1 is %2 seconds old and the killer was %3.").arg(pi->name()).arg((curtim-pi->murdertime)/MY_CLOCKS_PER_SEC).arg( pi->murderer() ) );
		}
		else
		{
			if (!pc_currchar->checkSkill( FORENSICS, 0, 500)) 
				socket->sysMessage( tr("You are not certain about the corpse.")); 
			else
			{
				if(((curtim-pi->murdertime)/MY_CLOCKS_PER_SEC)<=60) strcpy((char*)temp2, tr("few") );
				if(((curtim-pi->murdertime)/MY_CLOCKS_PER_SEC)>60) strcpy((char*)temp2, tr("many") );
				if(((curtim-pi->murdertime)/MY_CLOCKS_PER_SEC)>180) strcpy((char*)temp2, tr("many many"));
				socket->sysMessage( tr("The %1 is %2 seconds old.").arg(pi->name()).arg(temp2) );
				
				if ( !pc_currchar->checkSkill( FORENSICS, 500, 1000, false ) || pi->murderer() == "" ) 
					socket->sysMessage( tr("You can't say who was the killer.") ); 
				else
				{
					socket->sysMessage( tr("The killer was %1.").arg( pi->murderer() ) );
				}
			}
		}
		return true;
	}
};


// Poisoning
class cSkPoisoning: public cTargetRequest
{
	bool poisonSelected;
	P_ITEM pPoison;
public:
	cSkPoisoning() : poisonSelected(false), pPoison(0) {}

	bool selectPoison( cUOSocket* socket, cUORxTarget* target )
	{
		pPoison = FindItemBySerial( target->serial() );
		if ( !pPoison || pPoison->type() != 19 || pPoison->type() != 6 )
		{
			socket->sysMessage( tr("That is not a valid poison") );
			return true;
		}
		poisonSelected = true;
		return false; // Resend the target request
	}

	bool poisonItem( cUOSocket* socket, cUORxTarget* target )
	{
		
		P_ITEM pi = FindItemBySerial( target->serial() );
		if ( !pi )
			return true;
		
		P_CHAR pc = socket->player();
		int success=0;
		switch(pPoison->morez)
		{
		case 1:	success=pc->checkSkill( POISONING, 0, 500);		break;//lesser poison
		case 2:	success=pc->checkSkill( POISONING, 151, 651);		break;//poison
		case 3:	success=pc->checkSkill( POISONING, 551, 1051);		break;//greater poison
		case 4:	success=pc->checkSkill( POISONING, 901, 1401);		break;//deadly poison
		default:
			LogError("cSkPoisoning::poisonItem(..): switch reached default\n");
			return true;
		}

		pc->soundEffect( 0x0247 ); //poisoning effect

		if(success)
		{
			
			if(pi->poisoned<pPoison->morez) pi->poisoned = pPoison->morez;
			socket->sysMessage( tr("You successfully poison that item.") );
		} 
		else
			socket->sysMessage( tr("You fail to apply the poison.") );
		
		//empty bottle after poisoning
		P_ITEM pi_poison = pPoison;
		if (!pPoison->isInWorld()) 
			contsp.remove(pPoison->contserial, pi_poison->serial);
		SERIAL kser = pPoison->serial;
		
		pPoison->Init(0);
		pPoison->SetSerial(kser);
		pPoison->setId(0x0F0E);
		pPoison->moveTo(pc->pos);
		pPoison->priv|=0x01;
		pPoison->update();
		return true;
	}

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		if ( poisonSelected )
			return poisonItem( socket, target );
		else
			return selectPoison( socket, target );
	}
};


// Taste Identification
class cSkTasteID: public cTargetRequest
{
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		const P_ITEM pi = FindItemBySerial( target->serial() );
		P_CHAR pc_currchar = socket->player();
		if ( pi && !pi->isLockedDown() ) // Ripper
		{
			if( !( pi->type() == 19 || pi->type() == 14) )
			{
				socket->sysMessage( tr("You cant taste that!") );
				return true;
			}
			if (!pc_currchar->checkSkill( TASTEID, 0, 250))
			{
				socket->sysMessage( tr("You can't quite tell what this item is...") );
			}
			else
			{
				if( pi->corpse() )
				{
					socket->sysMessage( tr("You have to use your forensics evalutation skill to know more on this corpse.") );
					return true;
				}
				
				// Identify Item by Antichrist // Changed by MagiusCHE)
				if (pc_currchar->checkSkill( TASTEID, 250, 500, false ))
					if (pi->name2() != "#")
						pi->setName( pi->name2() ); // Item identified! -- by Magius(CHE)
					
				socket->sysMessage( tr("You found that this item appears to be called: %1").arg(pi->name())  );
					
				if (pc_currchar->checkSkill( TASTEID, 250, 500, false ))
				{
					if((pi->poisoned>0) || (pi->morex==4 && pi->morey==6 && pi->morez==1))
						socket->sysMessage( tr("This item is poisoned!") );
					else
						socket->sysMessage( tr("This item shows no poison.") );
						
					// Show Creator by Magius(CHE)
					if (pc_currchar->checkSkill( TASTEID, 250, 500, false ))
					{
						if (pi->creator.size()>0)
						{
							if (pi->madewith>0) 
								socket->sysMessage( tr("It is %1 by %2").arg(skill[pi->madewith-1].madeword).arg(pi->creator.c_str()) ); // Magius(CHE)
							else if (pi->madewith<0) 
								socket->sysMessage( tr("It is %1 by %2").arg(skill[0-pi->madewith-1].madeword).arg(pi->creator.c_str()) ); // Magius(CHE)
							else 
								socket->sysMessage( tr("It is made by %1").arg(pi->creator.c_str()) ); // Magius(CHE)
						} else 
							socket->sysMessage( tr("You don't know its creator!") );
					} else 
						socket->sysMessage( tr("You can't know its creator!") );
				}
			}
		}
		return true;
	}
};


// Provocation
class cSkProvocation: public cTargetRequest
{
	bool firstTarget;
	P_CHAR attacker;
public:
	
	cSkProvocation() : firstTarget(false) {}
	
	bool acquireFirst( cUOSocket* socket, cUORxTarget* target )
	{
		P_CHAR pc = FindCharBySerial( target->serial() );
		P_CHAR pc_currchar = socket->player();
		
		if( pc == NULL ) 
			return true; // cancel the skill
		
		P_ITEM inst = Skills->GetInstrument(socket);
		if (inst == NULL) 
		{
			socket->sysMessage( tr("You do not have an instrument to play on!") );
			return true; // cancel the skill
		}
		if ( pc->isInvul() || pc->shop() || // invul or shopkeeper
			pc->npcaitype()==0x01 || // healer
			pc->npcaitype()==0x04 || // tele guard
			pc->npcaitype()==0x06 || // chaos guard
			pc->npcaitype()==0x07 || // order guard
			pc->npcaitype()==0x09)   // city guard
		{
			socket->sysMessage( tr(" You cant entice that npc!") );
			return true;
		}
		if (pc->inGuardedArea())
		{
			socket->sysMessage( tr(" You cant do that in town!") );
			return true;
		}

		if (pc->isPlayer())
			socket->sysMessage( tr("You cannot provoke other players.") );
		else
		{
			socket->sysMessage( tr("You play your music, inciting anger, and your target begins to look furious. Whom do you wish it to attack?") );
			Skills->PlayInstrumentWell(socket, inst);
			firstTarget = true;
			attacker = pc;
			return false; 
		}
		return true;
	}

	bool selectVictim( cUOSocket* socket, cUORxTarget* target )
	{
		cChar* Victim = FindCharBySerial( target->serial() );
		if (!Victim)
			return true;
		
		P_CHAR Player = socket->player();
	
		if (Victim->inGuardedArea())
		{
			socket->sysMessage( tr("You cant do that in town!") );
			return true;
		}
		if (Victim->isSameAs(attacker))
		{
			socket->sysMessage( tr("Silly bard! You can't get something to attack itself.") );
			return true;
		}
		
		P_ITEM inst = Skills->GetInstrument(socket);
		if (inst == NULL) 
		{
			socket->sysMessage( tr("You do not have an instrument to play on!") );
			return true;
		}
		if (Player->checkSkill( MUSICIANSHIP, 0, 1000))
		{
			Skills->PlayInstrumentWell(socket, inst);
			if (Player->checkSkill( PROVOCATION, 0, 1000))
			{
				if( Player->inGuardedArea() )
				{
					Coord_cl cPos = Player->pos;
					cPos.x++;
					Combat::spawnGuard( Player, Player, cPos );
				}
				
				socket->sysMessage( tr("Your music succeeds as you start a fight.") );
			}
			else 
			{
				socket->sysMessage( tr("Your music fails to incite enough anger.") );
				Victim = Player;		// make the targeted one attack the Player
			}
			
			attacker->fight(Victim);
			attacker->setAttackFirst();
			
			Victim->fight(attacker);
			Victim->resetAttackFirst();
			
			strcpy(temp, tr("* You see %1 attacking %2 *").arg(attacker->name.c_str()).arg(Victim->name.c_str()) );
			int i;
			for ( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next())
			{
				if( mSock != socket && inrange1p( attacker, mSock->player() ) ) 
					itemmessage( toOldSocket(mSock), temp, Victim->serial );
			}
		}
		else
		{
			Skills->PlayInstrumentPoor(socket, inst);
			socket->sysMessage( tr("You play rather poorly and to no effect.") );
		}
		return true;
	}

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		if ( !firstTarget )
			return acquireFirst( socket, target );
		else
			return selectVictim( socket, target);
	}
	
};

// Repair Item
bool cSkRepairItem::responsed( cUOSocket *socket, cUORxTarget *target )
{
	if( !socket )
		return true;

	P_CHAR pc = socket->player();
	if( !pc )
		return true;

	P_ITEM pi = FindItemBySerial( target->serial() );
	if( !pi || pi->isLockedDown() )
	{
		socket->sysMessage( tr("You can't repair that!") );
		return false;
	}
	else if( pc->packitem() != INVALID_SERIAL && pi->contserial != pc->packitem() )
	{
		socket->sysMessage( tr("The item must be in your backpack to get repaired!") );
		return true;
	}
	else if( pi->hp() == 0 )
	{
		socket->sysMessage( tr("That item can't be repaired.") );
		return true;
	}
	else if( pi->hp() >= pi->maxhp() )
	{
		socket->sysMessage( tr("That item is of top quality.") );
		return true;
	}
	else
	{
		bool anvilinrange = false;
		RegionIterator4Items ri( pc->pos, 2 );
		for( ri.Begin(); !ri.atEnd(); ri++ )
		{
			P_ITEM pri = ri.GetData();
			if( pri && IsAnvil( pri->id() ) )
			{
				anvilinrange = true;
				break;
			}
		}
		if( !anvilinrange )
		{
			socket->sysMessage( tr("You must stand in range of an anvil!" ) );
			return true;
		}
	}

	bool hasSuccess = true;
	short dmg=4;	// damage to maxhp

	if( makesection_ && (dmg = 10 - makesection_->calcRank( pc )) <= 0 )
		hasSuccess = false;

	if( !makesection_ )
	{
		short smithing = pc->baseSkill( BLACKSMITHING );
		if		((smithing>=900)) dmg=1;
		else if ((smithing>=700)) dmg=2;
		else if ((smithing>=500)) dmg=3;
		hasSuccess = pc->checkSkill(BLACKSMITHING, 0, 1000);
	}

	if( hasSuccess )
	{
		pi->setMaxhp( pi->maxhp() - dmg );
		pi->setHp( pi->maxhp() );
		socket->sysMessage( tr("* the item has been repaired.*") );
	}
	else
	{
		pi->setHp( pi->hp() - 2 );
		pi->setMaxhp( pi->maxhp() - 1 );
		socket->sysMessage( tr("* You fail to repair the item. *") );
		socket->sysMessage( tr("* You weaken the item.*") );
	}

	if( makesection_ && makesection_->baseAction() )
		pc->soundEffect( makesection_->baseAction()->sound() );

	return true;
}

void cSkills::Hide( cUOSocket *socket ) 
{ 
	P_CHAR pChar = socket->player();

	if( !pChar )
		return;

	P_CHAR aChar = FindCharBySerial( pChar->attacker );
	if( aChar && aChar->inRange( pChar, pChar->VisRange ) )
	{
		pChar->message( tr( "You cannot hide while fighting." ) );
		return; 
	}
	
	if( pChar->hidden() ) 
	{ 
		pChar->message( tr( "You are already hidden." ) );
		return; 
	} 
	
	if( !pChar->checkSkill( HIDING, 0, 1000 ) ) 
	{ 
		pChar->message( "You are unable to hide here." );
		return; 
	} 
	
	pChar->message( tr( "You have hidden yourself well." ) );
	pChar->setHidden( 1 );
	pChar->resend(); // Remove + Resend (GMs etc.)
	
	// Resend us to ourself
	if( pChar->socket() )
		pChar->socket()->updatePlayer();
}

void cSkills::Stealth( cUOSocket *socket )
{
	P_CHAR pChar = socket->player();

	if( !pChar )
		return;

	if( !pChar->hidden() )
	{
		pChar->message( tr( "You must hide first." ) );
		return;
	}

	if( pChar->skill( HIDING ) < 700 )
	{
		socket->sysMessage( tr( "You are not hidden well enough. Become better at hiding." ) );
		return;
	}

	if( !pChar->checkSkill( STEALTH, 0, 1000 ) ) 
	{
		socket->sysMessage( tr( "You fail to stealth with your environment." ) );
		pChar->unhide();
		return;
	}

	socket->sysMessage( tr( "You can move %1 steps unseen" ).arg( SrvParams->maxStealthSteps() ) );
	pChar->setStealth( 0 );
}

void cSkills::PeaceMaking(cUOSocket* socket)
{
	int res1, res2, j;
	P_ITEM p_inst = Skills->GetInstrument(socket);
	if (p_inst == NULL) 
	{
		socket->sysMessage( tr( "You do not have an instrument to play on!" ) );
		return;
	}
	P_CHAR pc_currchar = socket->player();
	res1=pc_currchar->checkSkill( PEACEMAKING, 0, 1000);
	res2=pc_currchar->checkSkill( MUSICIANSHIP, 0, 1000);
	if (res1 && res2)
	{
		Skills->PlayInstrumentWell(socket, p_inst);
		socket->sysMessage( tr( "You play your hypnotic music, stopping the battle.") );
		
		RegionIterator4Chars ri(pc_currchar->pos, VISRANGE);
		for (ri.Begin(); !ri.atEnd(); ri++)
		{
			P_CHAR mapchar = ri.GetData();
			if( mapchar && mapchar->war() )
			{
				j = calcSocketFromChar(mapchar);
				if( mapchar->socket() )
					mapchar->socket()->sysMessage( tr("You hear some lovely music, and forget about fighting.") );

				if( mapchar->isNpc() ) 
					mapchar->toggleCombat();

				mapchar->targ = INVALID_SERIAL;
				mapchar->attacker = INVALID_SERIAL;
				mapchar->resetAttackFirst();
			}
		}
	} 
	else 
	{
		Skills->PlayInstrumentPoor(socket, p_inst);
		socket->sysMessage( tr("You attempt to calm everyone, but fail.") );
	}
}

void cSkills::PlayInstrumentWell(cUOSocket* socket, P_ITEM pi)
{
	P_CHAR pc_currchar = socket->player();
	switch(pi->id())
	{
	case 0x0E9C:	pc_currchar->soundEffect( 0x0038 );	break;
	case 0x0E9D:
	case 0x0E9E:	pc_currchar->soundEffect( 0x0052 );	break;
	case 0x0EB1:
	case 0x0EB2:	pc_currchar->soundEffect( 0x0045 );	break;
	case 0x0EB3:
	case 0x0EB4:	pc_currchar->soundEffect( 0x004C );	break;
	}
}

void cSkills::PlayInstrumentPoor(cUOSocket* socket, P_ITEM pi)
{
	P_CHAR pc_currchar = socket->player();
	switch(pi->id())
	{
	case 0x0E9C:	pc_currchar->soundEffect( 0x0039);	break;
	case 0x0E9D:
	case 0x0E9E:	pc_currchar->soundEffect( 0x0053);	break;
	case 0x0EB1:
	case 0x0EB2:	pc_currchar->soundEffect( 0x0046);	break;
	case 0x0EB3:
	case 0x0EB4:	pc_currchar->soundEffect( 0x004D);	break;
	}
}

P_ITEM cSkills::GetInstrument(cUOSocket* socket)
{
	P_CHAR pc_currchar = socket->player();

	unsigned int ci = 0;
	vector<SERIAL> vecContainer = contsp.getData(pc_currchar->packitem());
	for (; ci < vecContainer.size(); ++ci)
	{
		P_ITEM pi = FindItemBySerial(vecContainer[ci]);
		if ( IsInstrument(pi->id()) )
		{
			return pi;
		}
	}
	return NULL;
}

//////////////////////////////
// name:	DoOnePotion
// history: Duke,20.04.2000
// Purpose:	helper function for DoPotion
//			checks if player has enough regs for selected potion and deletes them
//
static bool DoOnePotion(int s,short regid, int regamount, char* regname)
{
	bool success=false;

	if (getamount(currchar[s], regid) >= regamount)
	{
		success=true;
		currchar[s]->emote( tr("*%s starts grinding some %s in the mortar.*").arg(currchar[s]->name.c_str()).arg(regname) ); // LB, the 1 stops stupid alchemy spam
		delequan(currchar[s],regid,regamount);
	}
	else
		sysmessage(s, tr("You do not have enough reagents for that potion."));
	
	return success;
}

///////////////////////////
// name:	DoPotion
// history: unknown, revamped by Duke,21.04.2000
// Purpose:	determines regs and quantity, creates working sound and
//			indirectly calls CreatePotion on success
//
void cSkills::DoPotion(int s, int type, int sub, P_ITEM mortar)
{
	if (sub == 0)	// user cancelled second alchemy menu 
		return;
	bool success=false;
	
	switch((type*10)+sub)
	{
	case 11: success=DoOnePotion(s,0x0F7B, 1,"blood moss");		break;//agility
	case 12: success=DoOnePotion(s,0x0F7B, 3,"blood moss");		break;//greater agility
	case 21: success=DoOnePotion(s,0x0F84, 1,"garlic");			break;//lesser cure
	case 22: success=DoOnePotion(s,0x0F84, 3,"garlic");			break;//cure
	case 23: success=DoOnePotion(s,0x0F84, 6,"garlic");			break;//greater cure
	case 31: success=DoOnePotion(s,0x0F8C, 3,"sulfurous ash");	break;//lesser explosion
	case 32: success=DoOnePotion(s,0x0F8C, 5,"sulfurous ash");	break;//explosion
	case 33: success=DoOnePotion(s,0x0F8C,10,"sulfurous ash");	break;//greater explosion
	case 41: success=DoOnePotion(s,0x0F85, 1,"ginseng");		break;//lesser heal
	case 42: success=DoOnePotion(s,0x0F85, 3,"ginseng");		break;//heal
	case 43: success=DoOnePotion(s,0x0F85, 7,"ginseng");		break;//greater heal
	case 51: success=DoOnePotion(s,0x0F8D, 1,"spider's silk");	break;//night sight
	case 61: success=DoOnePotion(s,0x0F88, 1,"nightshade");		break;//lesser poison
	case 62: success=DoOnePotion(s,0x0F88, 2,"nightshade");		break;//poison
	case 63: success=DoOnePotion(s,0x0F88, 4,"nightshade");		break;//greater poison
	case 64: success=DoOnePotion(s,0x0F88, 8,"nightshade");		break;//deadly poison
	case 71: success=DoOnePotion(s,0x0F7A, 1,"black pearl");	break;//refresh
	case 72: success=DoOnePotion(s,0x0F7A, 5,"black pearl");	break;//total refreshment
	case 81: success=DoOnePotion(s,0x0F86, 2,"mandrake");		break;//strength
	case 82: success=DoOnePotion(s,0x0F86, 5,"mandrake");		break;//greater strength
	default:
		LogErrorVar("switch reached default for <%i>",(type*10)+sub);
		return;
	}
	if (success)
	{
		P_CHAR pc_currchar = currchar[s];
		tempeffect(pc_currchar, pc_currchar, 9, 0, 0, 0);	// make grinding sound for a while
		tempeffect(pc_currchar, pc_currchar, 9, 0, 3, 0);
		tempeffect(pc_currchar, pc_currchar, 9, 0, 6, 0);
		tempeffect(pc_currchar, pc_currchar, 9, 0, 9, 0);
		tempeffect2(pc_currchar, mortar, 10, type, sub, 0);	// this will indirectly call CreatePotion()
	}
}

///////////////////////////
// name:	CreatePotion
// history: unknown, revamped by Duke,21.04.2000
// Purpose:	does the appropriate skillcheck for the potion, creates it
//			in the mortar on success and tries to put it into a bottle
//
void cSkills::CreatePotion(P_CHAR pc, char type, char sub, P_ITEM pi_mortar)
{
	int success=0;

	if ( pc == NULL ) return;

	switch((10*type)+sub)
	{
	case 11:success=pc->checkSkill( ALCHEMY,151, 651);break;//agility
	case 12:success=pc->checkSkill( ALCHEMY,351, 851);break;//greater agility
	case 21:success=pc->checkSkill( ALCHEMY,  0, 500);break;//lesser cure
	case 22:success=pc->checkSkill( ALCHEMY,251, 751);break;//cure
	case 23:success=pc->checkSkill( ALCHEMY,651,1151);break;//greater cure
	case 31:success=pc->checkSkill( ALCHEMY, 51, 551);break;//lesser explosion
	case 32:success=pc->checkSkill( ALCHEMY,351, 851);break;//explosion
	case 33:success=pc->checkSkill( ALCHEMY,651,1151);break;//greater explosion
	case 41:success=pc->checkSkill( ALCHEMY,  0, 500);break;//lesser heal
	case 42:success=pc->checkSkill( ALCHEMY,151, 651);break;//heal
	case 43:success=pc->checkSkill( ALCHEMY,551,1051);break;//greater heal
	case 51:success=pc->checkSkill( ALCHEMY,  0, 500);break;//night sight
	case 61:success=pc->checkSkill( ALCHEMY,  0, 500);break;//lesser poison
	case 62:success=pc->checkSkill( ALCHEMY,151, 651);break;//poison
	case 63:success=pc->checkSkill( ALCHEMY,551,1051);break;//greater poison
	case 64:success=pc->checkSkill( ALCHEMY,901,1401);break;//deadly poison
	case 71:success=pc->checkSkill( ALCHEMY,  0, 500);break;//refresh
	case 72:success=pc->checkSkill( ALCHEMY,251, 751);break;//total refreshment
	case 81:success=pc->checkSkill( ALCHEMY,251, 751);break;//strength
	case 82:success=pc->checkSkill( ALCHEMY,451, 951);break;//greater strength
	default:
		LogError("switch reached default");
		return;
	}

	if (success==0 && !pc->isGM()) // AC bugfix
	{
		pc->emote( tr("*%1 tosses the failed mixture from the mortar, unable to create a potion from it.*").arg(pc->name.c_str()) );
		return;
	}
	pi_mortar->setType( 17 );
	pi_mortar->more1=type;
	pi_mortar->more2=sub;
	pi_mortar->morex=pc->skill(ALCHEMY);
	
	if (!(getamount(pc, 0x0F0E)>=1))
	{
		target(calcSocketFromChar(pc), 0, 1, 0, 109, "Where is an empty bottle for your potion?");
	}
	else
	{
		pc->soundEffect( 0x0240 );
		pc->emote( tr("*%1 pours the completed potion into a bottle.*").arg(pc->name.c_str()));
		delequan(pc, 0x0F0E, 1);
		Skills->PotionToBottle(pc, pi_mortar);
	} 
}

/////////////////////////
// name:	BottleTarget
// history: unknown, revamped by Duke,23.04.2000
// Purpose: Uses the targeted potion bottle *outside* the backpack to
//			pour in the potion from the mortar
//
void cSkills::BottleTarget(int s)
{
	P_ITEM pi=FindItemBySerPtr(buffer[s]+7);
	P_CHAR pc_currchar = currchar[s];
	if (!pi || pi->isLockedDown()) return;	// Ripper

	if (pi->id()==0x0F0E)	// an empty potion bottle ?
	{
		pi->ReduceAmount(1);

		P_ITEM mortar = FindItemBySerial(calcserial(addid1[s], addid2[s], addid3[s], addid4[s]));
		if(mortar == NULL) return;
		if (mortar->type() == 17) 
		{
			pc_currchar->emote( tr("*%1 pours the completed potion into a bottle.*").arg(pc_currchar->name.c_str()));
			Skills->PotionToBottle(pc_currchar, mortar);
		}
	}
	else
		sysmessage(s,"This is not an appropriate container for a potion.");
}

/////////////////////////
// name:	PotionToBottle
// history: unknown, revamped by Duke,23.04.2000
// Purpose: this really creates the potion
//
void cSkills::PotionToBottle(P_CHAR pc, P_ITEM pi_mortar)
{
	unsigned char id1,id2;
	char pn[50];

	if ( pc == NULL ) return;

	switch((10*pi_mortar->more1)+pi_mortar->more2)
	{
	case 11: id1=0x0F;id2=0x08;strcpy(pn, "an agility");				break;
	case 12: id1=0x0F;id2=0x08;strcpy(pn, "a greater agility");			break;
	case 21: id1=0x0F;id2=0x07;strcpy(pn, "a lesser cure");				break;
	case 22: id1=0x0F;id2=0x07;strcpy(pn, "a cure");					break;
	case 23: id1=0x0F;id2=0x07;strcpy(pn, "a greater cure");			break;
	case 31: id1=0x0F;id2=0x0D;strcpy(pn, "a lesser explosion");		break;
	case 32: id1=0x0F;id2=0x0D;strcpy(pn, "an explosion");				break;
	case 33: id1=0x0F;id2=0x0D;strcpy(pn, "a greater explosion");		break;
	case 41: id1=0x0F;id2=0x0C;strcpy(pn, "a lesser heal");				break;
	case 42: id1=0x0F;id2=0x0C;strcpy(pn, "a heal");					break;
	case 43: id1=0x0F;id2=0x0C;strcpy(pn, "a greater heal");			break;
	case 51: id1=0x0F;id2=0x06;strcpy(pn, "a night sight");				break;
	case 61: id1=0x0F;id2=0x0A;strcpy(pn, "a lesser poison");			break;
	case 62: id1=0x0F;id2=0x0A;strcpy(pn, "a poison");					break;
	case 63: id1=0x0F;id2=0x0A;strcpy(pn, "a greater poison");			break;
	case 64: id1=0x0F;id2=0x0A;strcpy(pn, "a deadly poison");			break;
	case 71: id1=0x0F;id2=0x0B;strcpy(pn, "a refresh");					break;
	case 72: id1=0x0F;id2=0x0B;strcpy(pn, "a total refreshment");		break;
	case 81: id1=0x0F;id2=0x09;strcpy(pn, "a strength");				break;
	case 82: id1=0x0F;id2=0x09;strcpy(pn, "a greater strength");		break;
	default:
		LogError("switch reached default");
		return;
	}
	
	P_ITEM pi_potion = Items->SpawnItem(calcSocketFromChar(pc), pc, 1,"#",0, id1, id2,0,1,0);
	if (pi_potion == NULL) 
		return;
	
	pi_potion->setName( QString( "%1 potion" ).arg( pn ) );
	pi_potion->setType( 19 );
	pi_potion->morex = pi_mortar->morex;
	pi_potion->morey = pi_mortar->more1;
	pi_potion->morez = pi_mortar->more2;
	
	// the remainder of this function NOT (yet) revamped by Duke !
	
	// Addon for Storing creator NAME and SKILLUSED by Magius(CHE) §
	if(!pc->isGM())
	{
		pi_potion->creator = pc->name; // Magius(CHE) - Memorize Name of the creator
		if (pc->skill(ALCHEMY)>950) pi_potion->madewith=ALCHEMY+1; // Memorize Skill used - Magius(CHE)
		else pi_potion->madewith=0-ALCHEMY-1; // Memorize Skill used - Magius(CHE)
	} else {
		pi_potion->creator[0]='\0';
		pi_potion->madewith=0;
	}
	
	pi_potion->update();
	pi_mortar->setType( 0 );
	// items[i].weight=100; // Ripper 11-25-99
	// AntiChrist NOTE: please! use the HARDITEMS.SCP...
	// the settings used in that script are used EVERY TIME we have an item created via
	// code ( and not via script )...so we simply can add the string:
	// WEIGHT 100
	// in the "potion section" of that file, and EVERY TIME the potions are created via scripts
	// they have those settings! :) that's easy isn't it? =P
	
	return;
}

char cSkills::AdvanceSkill(P_CHAR pc, int sk, char skillused)
{
	int i=0, retval, incval,a,d=0;
	unsigned int ges = 0;
	unsigned char lockstate;

	int atrophy_candidates[ALLSKILLS+1];

	if (pc == NULL)
		return 0;


	lockstate=pc->lockSkill[sk];
	if (pc->isGM()) lockstate=0;
	// for gms no skill cap exists, also ALL skill will be interperted as up, no matter how they are set

	if (lockstate==2 || lockstate==1) return 0;// skill is locked -> forget it

	// also NOthing happens if you train a skill marked for atrophy !!!
	// skills only fall if others raise, ( osi quote ), so my interpretion
	// is that those marked as falling cant fall if you use them directly
	// exception: if you are gm its ignored! 
	
	int c=0;

	for (int b=0;b<(ALLSKILLS+1);b++) 
	{
		if (pc->lockSkill[b]==1 && pc->baseSkill(b)!=0) // only count atrophy candidtes if they are above 0 !!!
		{
			atrophy_candidates[c]=b;
			c++;
		}
	}

	if (!pc->isGM())
	{
		for (a=0;a<ALLSKILLS;a++)
		{
			ges+=pc->baseSkill(a);
		}
		ges=ges/10;

		if (ges > SrvParams->skillcap() && c==0) // skill capped and no skill is marked as fall down.
		{
			sprintf((char*)temp,"You have reached the skill-cap of %i and no skill can fall!", SrvParams->skillcap());
			sysmessage(calcSocketFromChar(pc), (char*)temp);
			return 0;
		}

	} 
	else 
		ges=0;
	
	unsigned long loopexit=0;
	while ( (wpadvance[1+i+skill[sk].advance_index].skill==sk && 
		wpadvance[1+i+skill[sk].advance_index].base<=pc->baseSkill(sk)) && (++loopexit < MAXLOOPS) )
	{
		i++;
	}

	if(skillused)
		incval=(wpadvance[i+skill[sk].advance_index].success)*10;
	else
		incval=(wpadvance[i+skill[sk].advance_index].failure)*10;

	retval=0;
	if (incval>rand()%SrvParams->skillAdvanceModifier())
	{
		retval=1;
		pc->setBaseSkill(sk, pc->baseSkill(sk)+1);			
	}

	if (retval)
	{
		// no atrophy for gm's !! 
		if (ges > SrvParams->skillcap()) // atrophy only if cap is reached !!!
		// if we are above the skill cap -> we have to let the atrophy candidates fall
		// important: we have to let 2 skills fall, or we'll never go down to cap
		// (especially if we are far above the cap from previous verisons)
		{
			int dsk = 0;	// the skill to be decreased
			if (c==1) 
			{
				dsk = atrophy_candidates[0];
				if (pc->baseSkill(dsk)>=2) d=2; else d=1; // avoid value below 0 (=65535 cause unsigned)
				{ 
					if (d==1 && pc->baseSkill(dsk)==0) d=0; // should never happen ...
						pc->setBaseSkill(dsk, pc->baseSkill(dsk)-d);
					Skills->updateSkillLevel(pc, dsk); 		// we HAVE to correct the skill-value
					updateskill(calcSocketFromChar(pc), dsk); // and send changed skill values packet so that client can re-draw correctly			
				}
			// this is very important cauz this is ONLY done for the calling skill value automatically .
			} 
			else
			{
				if (c!=0) d=rand()%c; else d=0;
				dsk = atrophy_candidates[d];
				if (pc->baseSkill(dsk)>=1) 
				{
					pc->setBaseSkill(dsk, pc->baseSkill(dsk)-1);
					Skills->updateSkillLevel(pc, dsk); 	
					updateskill(calcSocketFromChar(pc), dsk); 				
				}

				if (c!=0) d=rand()%c; else d=0;
				dsk = atrophy_candidates[d];
				if (pc->baseSkill(dsk)>=1) 
				{
					pc->setBaseSkill(dsk, pc->baseSkill(dsk)-1);
					Skills->updateSkillLevel(pc, dsk); 	
					updateskill(calcSocketFromChar(pc), dsk); 			
				}
			}
		}
		Skills->AdvanceStats(pc, sk);
	}
	return retval;
}

//////////////////////////////
// name:	AdvanceOneStat
// history:	by Duke, 21 March 2000
// Purpose: little helper functions for cSkills::AdvanceStats
//			finds the appropriate line for the used skill in advance table
//			and uses the value of that skill (!) to increase the stat
//			and cuts it down to 100 if necessary
//
static int calcStatIncrement(int sk, int i, int stat)
{
	unsigned long loopexit=0;
	while ((wpadvance[i+1].skill==sk &&		// if NEXT line is for same skill
			wpadvance[i+1].base <= stat*10) && (++loopexit < MAXLOOPS) )	// and is not higher than our stat
	{
		i++;								// then proceed to it !
	}
	return wpadvance[i].success;			// gather small increases
}

static int AdvanceOneStat(int sk, int i, signed short *stat, signed short *stat2, bool *update, bool aGM)
{
	*stat2 += calcStatIncrement(sk,i,*stat);// gather small increases
	if (*stat2>1000)						// until they reach 1000
	{
		*stat2 -= 1000;						// then change it
		*stat += 1;							// into one stat point
		*update=true;
	}
	if(*stat>100 && !aGM)					// cutting, but not for GMs
	{
		*stat=100;
		*update=true;
	}
	return *update;
}

///////////////////////////////
// name:	AdvanceStats
// history: revamped by Duke, 21 March 2000
// Purpose: Advance STR, DEX and INT after use of a skill
//			checks if STR+DEX+INT are higher than statcap from server.scp
//			gives all three stats the chance (from skills.scp & server.scp) to rise
//			and reduces the two other stats if necessary
//
void cSkills::AdvanceStats( P_CHAR pChar, UINT16 skillId )
{
	bool update = false;
	bool atCap = false;
	signed short temp1,temp2;
	if ( !pChar ) 
		return;
	
	INT32 i = skill[skillId].advance_index;
	INT32 mod = SrvParams->statsAdvanceModifier();
	
	// Strength advancement
	
	if( skill[skillId].st > rand() % mod )
		if( AdvanceOneStat( skillId, i, &( temp1 = pChar->st()), &( temp2 = pChar->st2()), &update, pChar->isGM() ) && atCap && !pChar->isGM() )
			if( rand() % 2 ) 
				pChar->chgRealDex(-1); 
			else 
//				pChar->in-=1;
				pChar->setIn(pChar->in() - 1);
	
	if( skill[skillId].dx > rand() % mod )
		if( pChar->incDecDex( calcStatIncrement( skillId, i, pChar->realDex() ) ) )
		{
			update = true;
			if( atCap )
				if( rand() % 2 )
					pChar->setSt( ( pChar->st() ) - 1 );
				else 
//					pChar->in -= 1;
					pChar->setIn(pChar->in() - 1);
		}
	
	if( skill[skillId].in > rand() % mod )
		if( AdvanceOneStat( skillId, i, &( temp1 = pChar->in() ), &(temp2 = pChar->in2()), &update, pChar->isGM() ) && atCap && !pChar->isGM() )
			if( rand()%2 ) 
				pChar->chgRealDex(-1); 
			else 
				pChar->setSt( ( pChar->st() ) - 1 );
	
	cUOSocket *socket = pChar->socket();
	if( update && socket )
	{
		socket->sendStatWindow();
		for( i = 0; i < ALLSKILLS; ++i )
			updateSkillLevel(pChar, i); // When advancing stats we dont update the client (?)

		if( atCap && !pChar->isGM() )
			socket->sysMessage( tr( "You have reached the stat-cap of %1!" ).arg( SrvParams->statcap() ) );
	}
}

void cSkills::SpiritSpeak(int s) // spirit speak time, on a base of 30 seconds + skill[SPIRITSPEAK]/50 + INT
{
	//	Unsure if spirit speaking should they attempt again?
	//	Suggestion: If they attempt the skill and the timer is !0 do not have it raise the skill
	
	/*if(!Skills->CheckSkill(currchar[s], SPIRITSPEAK, 0, 1000))
	{
		sysmessage(s,"You fail your attempt at contacting the netherworld.");
		return;
	}
	
	impaction(s,0x11);			// I heard there is no action...but I decided to add one
	soundeffect(s,0x02,0x4A);	// only get the sound if you are successful
	sysmessage(s,"You establish a connection to the netherworld.");
	SetTimerSec(&currchar[s]->spiritspeaktimer,SrvParams->spiritspeaktimer()+currchar[s]->in);*/
}

void cSkills::SkillUse( cUOSocket *socket, UINT16 id) // Skill is clicked on the skill list
{
	P_CHAR pChar = socket->player();

	// No Char no Skill use
	if( !pChar )
		return;

	if( pChar->cell() > 0 )
	{
		socket->sysMessage( tr( "You are in jail and cant use skills here!" ) );
		return;
	}

	if( pChar->dead() )
	{
		socket->sysMessage( tr( "You cannot do that as a ghost." ) );
		return;
	}

	pChar->unhide(); // Unhide if we're stealthing or hiding too!
	pChar->disturbMed(); // Disturb meditation if we're using a skill

	if( pChar->casting() )
	{
		socket->sysMessage( tr( "You can't do that while you are casting." ) );
		return;
	}

	if( pChar->skilldelay > uiCurrentTime && !pChar->isGM() )
	{
		socket->sysMessage( tr( "You must wait a few moments before using another skill." ) );
		return;
	}

	cTargetRequest *targetRequest = NULL;
	QString message;
	int s = -1;

	switch( id )
	{
	case ARMSLORE:
		message = tr("What item do you wish to get information about?");
		targetRequest = new cSkArmsLore;
		break;
	case ANATOMY:
		message = tr("Whom shall I examine?");
		targetRequest = new cSkAnatomy;
		break;
	case ITEMID:
		message = tr("What do you wish to appraise and identify?");
		targetRequest = new cSkItemID;
		break;
	case EVALUATINGINTEL:
		message = tr("What would you like to evaluate?");
		targetRequest = new cSkIntEval;
		break;
	case TAMING:
		message = tr("Tame which animal?");
		targetRequest = new cSkTame;
		break;
	case HIDING:
		Skills->Hide( socket );
		break;
	case STEALTH:
		Skills->Stealth( socket );
		break;
	case DETECTINGHIDDEN:
		message = tr("Where do you wish to search for hidden characters?");
		targetRequest = new cSkDetectHidden;
		break;
	case PEACEMAKING:
		Skills->PeaceMaking(socket);
		break;
	case PROVOCATION:
		message = tr("Whom do you wish to incite?");
		targetRequest = new cSkProvocation;
		break;
	case ENTICEMENT:
		message = tr("Whom do you wish to entice?");
		//target(s, 0, 1, 0, 81, );
		break;
	case SPIRITSPEAK:
		Skills->SpiritSpeak(s);
		break;
	case STEALING:
		if( !SrvParams->stealingEnabled() )
		{
			socket->sysMessage( tr( "That skill has been disabled." ) );
			return;
		}
		
		message = tr("What do you wish to steal?");
		targetRequest = new cSkStealing;
		break;
	case INSCRIPTION:
		message = tr("What do you wish to place a spell on?");
		//target(s, 0, 1, 0, 160, );
		break;
	case TRACKING:
		trackingMenu( socket );
		break;
	case BEGGING:
		message = tr( "Whom do you wish to annoy?" );
		targetRequest = new cSkBegging;
		break;
	case ANIMALLORE:
		message = tr("What animal do you wish to get information about?");
		targetRequest = new cSkAnimalLore;
		break;
	case FORENSICS:
		message = tr("What corpse do you want to examine?");
		targetRequest = new cSkForensics;
		break;
	case POISONING:
		message = tr("What poison do you want to apply?");
		targetRequest = new cSkPoisoning;
		break;

	case TASTEID:
		message = tr("What do you want to taste?");
        targetRequest = new cSkTasteID;
        break;

	case MEDITATION:
		if( !SrvParams->armoraffectmana() )
		{
			socket->sysMessage( tr( "Meditation is disabled." ) );
			return;
		}
        
		Skills->Meditation( socket );
		break;
	case CARTOGRAPHY:
		Skills->Cartography( socket );
		break;
	case CARPENTRY:
		Skills->Carpentry( socket );
		break;
	case BLACKSMITHING:
		Skills->Blacksmithing( socket );
		break;
	case BOWCRAFT:
		Skills->Fletching( socket );
		break;
	case TAILORING:
		Skills->Tailoring( socket );
		break;
	case TINKERING:
		Skills->Tinkering( socket );
		break;
	default:
		socket->sysMessage( tr( "That skill has not been implemented yet." ) );
		return;
	}

	if( targetRequest )
		socket->attachTarget( targetRequest );
	
	if( message )
		pChar->message( message );

	pChar->setSkillDelay();
}

void cSkills::RandomSteal(cUOSocket* socket, SERIAL victim)
{
	int i, skill;
	char temp2[512];
	tile_st tile;
	P_CHAR pc_currchar = socket->player();	
	int cansteal = QMAX(1, pc_currchar->baseSkill(STEALING)/10);
	cansteal = cansteal * 10;
	
	P_CHAR pc_npc = FindCharBySerial(victim);
	if (pc_npc == NULL) 
		return;

	P_ITEM pBackpack = Packitem(pc_npc);
	if (pBackpack == NULL) 
	{
		socket->sysMessage( tr("bad luck, your victim doesnt have a backpack") ); 
		return; 
	}
	
	i=0;
	P_ITEM item = NULL;
	vector<SERIAL> vecContainer = contsp.getData(pBackpack->serial);
	if (vecContainer.size() != 0)
		item = FindItemBySerial(vecContainer[rand()%vecContainer.size()]);

	if (pc_npc == pc_currchar) {
		socket->sysMessage( tr("You catch yourself red handed.") );
		return;
	}
	
	if (pc_npc->npcaitype() == 17)
	{
		socket->sysMessage( tr("You cannot steal that.") );
		return;
	}

	// Lb, crashfix, happens if pack=empty i guess
	if (item == NULL) 
	{ 
		socket->sysMessage( tr("your victim doesnt have posessions") );
		return;
	}

	sprintf((char*)temp, "You reach into %s's pack and try to take something...%s",pc_npc->name.c_str(), item->name().ascii());
	socket->sysMessage( (char*)temp );
	if( pc_currchar->inRange( pc_npc, 1 ) )
	{
		if( ( (item->totalweight()/10) > cansteal ) && (item->type()!=1 && item->type()!=63 && item->type()!=65 && item->type()!=87))//Containers
		{
			socket->sysMessage( tr("That is too heavy.") );
			return;
		} else if((item->type() == 1 || item->type() == 63 || item->type() == 65 || item->type() == 87) && (Weight->RecursePacks(item) > cansteal))
		{
			socket->sysMessage( tr("That is too heavy.") );
			return;
		}
		if (pc_npc->isGMorCounselor())
		{
			socket->sysMessage( tr("You can't steal from gods.") );
			return;
		}
		if(item->priv & 0x02)//newbie
		{
			socket->sysMessage( tr("That item has no value to you.") );
			return;
		}
		
		skill = pc_currchar->checkSkill( STEALING, 0, 999);
		if (skill)
		{
			item->setContSerial(pc_currchar->packitem());
			socket->sysMessage( tr("You successfully steal that item.") );
			item->update();
		} else 
			socket->sysMessage( tr("You failed to steal that item.") );
		
		if ((!skill && rand()%5+15==17) || (pc_currchar->skill(STEALING)<rand()%1001))
		{//Did they get cought? (If they fail 1 in 5 chance, other wise their skill away from 1000 out of 1000 chance)
			socket->sysMessage( tr("You have been cought!") );
			
			if (pc_npc->isNpc()) 
				pc_npc->talk( tr("Guards!! A thief is amoung us!"), -1, 0x09 );
			
			if (pc_npc->isInnocent() && pc_currchar->attacker != pc_npc->serial && GuildCompare( pc_currchar, pc_npc )==0)//AntiChrist
				criminal( pc_currchar );//Blue and not attacker and not guild
			
			if (item->name() != "#")
			{
				sprintf((char*)temp,"You notice %s trying to steal %s from you!",pc_currchar->name.c_str(),item->name().ascii());
				sprintf(temp2,"You notice %s trying to steal %s from %s!",pc_currchar->name.c_str(), item->name().ascii(), pc_npc->name.c_str());
			} else {
				tile = cTileCache::instance()->getTile( item->id() );
				sprintf((char*)temp,"You notice %s trying to steal %s from you!",pc_currchar->name.c_str(), tile.name);
				sprintf(temp2,"You notice %s trying to steal %s from %s!",pc_currchar->name.c_str(),tile.name, pc_npc->name.c_str());
			}
			socket->sysMessage( (char*)temp); // bugfix, LB
			for ( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next())
			{
				if( mSock != socket && inrange1p( pc_currchar, mSock->player() ) && (rand()%10+10==17|| (rand()%2==1 && mSock->player()->in() >= pc_currchar->in() ))) 
					mSock->sysMessage(temp2);
			}
		}
	} else socket->sysMessage( tr("You are too far away to steal that item.") );
}

// Redone by LB on dec 28'th 1999
// Thx goes to Ripper for the design 
///////////////////////
// name:	CheckThreeSkills
// history:	by Duke, 8 April 2000
// Purpose:	little helper function for TellScroll()
//			checks the 3 skills required for engraving and cumulates
//			the returnvalues i.e. only a part==3 means success!
//
static int CheckThreeSkills(int s, int low, int high)
{
	int part=0;
	currentSpellType[s]=0;		// needed for MAGERY check
	part += currchar[s]->checkSkill( INSCRIPTION,  low, high);
	part += currchar[s]->checkSkill( MAGERY,		low, high);
	part += currchar[s]->checkSkill( TINKERING,	low, high);
	return part;
}

///////////////////////
// name:	TellScroll
// history:	unknown, modified by LB & AntiChrist, 
//			partially revamped by Duke, 8 April 2000
// Purpose:	callback function for inscription menue
//			This is the point where control is returned after the user
//			selected a spell to inscribe/engrave from the gump menu.
//
void TellScroll( char *menu_name, int s, long snum )
{
/*	P_CHAR pc_currchar = currchar[s];
	unsigned cir,spl;
	int part;

	if(snum<=0) return;				// bad spell selction

	P_ITEM pi = FindItemBySerial(pc_currchar->lastTarget);

	cir=(int)((snum-800)/10);		// snum holds the circle/spell as used in inscribe.gmp
	spl=(((snum-800)-(cir*10))+1);	// i.e. 800 + 1-based circle*10 + zero-based spell
									// snum is also equals the item # in items.scp of the scrool to be created !
	P_ITEM pBackpack = Packitem(pc_currchar);
	if (pBackpack == NULL) return;
	
	AllItemsIterator iterItems;
	for (iterItems.Begin(); !iterItems.atEnd(); iterItems++)		// find the spellbook
	{
		P_ITEM pb = iterItems.GetData();
		if (pb->type()==9 && (pb->contserial==pBackpack->serial ||
			(pb->layer()==1 && pc_currchar->Wears(pb))))
		{
			if (!Magic->CheckBook( cir, spl-1, pb))
			{
				sysmessage(s,"You don't have this spell in your spell book!");
				return;
			}
		}
	}
	
	int num = (8*(cir-1)) + spl;	// circle & spell combined to a zero-based index
	
	Magic->Action4Spell(s,num);
	Magic->SpeakMantra4Spell(pc_currchar,num);
	
	if(!Magic->CheckReagents(pc_currchar, num)
		|| !Magic->CheckMana(pc_currchar, num))
	{
		Magic->SpellFail(s);
		return;
	}
	Magic->SubtractMana4Spell(pc_currchar, num);
	
	if (pi->id()==0x0E34)  //is it a scroll?
	{
		itemmake[s].Mat1id=0x0E34; 
		itemmake[s].needs=1; 
		itemmake[s].has = getamount(pc_currchar, 0x0E34); 
		itemmake[s].minskill=(cir-1)*100;	//set range values based on scroll level
		itemmake[s].maxskill=(cir+2)*100;

		Magic->DelReagents(pc_currchar, num);
		
		Skills->MakeMenuTarget(s,snum,INSCRIPTION); //put it in your pack
	}
	else if ((pi->att>0)||(pi->def>0)||(pi->hidamage())) //or is it an item?
	{
		part=0;
		switch(cir)
		{
		case 1:	part = CheckThreeSkills(s,  11, 401);break;
		case 2:	part = CheckThreeSkills(s,  61, 501);break;
		case 3:	part = CheckThreeSkills(s, 161, 601);break;
		case 4:	part = CheckThreeSkills(s, 261, 701);break;
		case 5:	part = CheckThreeSkills(s, 361, 801);break;
		case 6:	part = CheckThreeSkills(s, 461, 901);break;
		case 7:	part = CheckThreeSkills(s, 661,1101);break;
		case 8:	part = CheckThreeSkills(s, 761,1201);break;
		default:
			LogError("switch reached default");
			return;
		}
		
		if( part < 3 )		// failure !
		{
			switch(part)
			{
			case 0:
				sysmessage(s,"Your hand jerks and you punch a hole in the item");
				pi->setHp( pi->hp() - 3 );
				break;
			case 1:
				sysmessage(s,"Your hand slips and you dent the item");
				pi->setHp( pi->hp() - 2 );
				break;
			case 2:
				sysmessage(s,"Your hand cramps and you scratch the item");
				pi->setHp( pi->hp() - 1 );
				break;
			}
			if( pi->hp() < 1 )
				pi->setHp( 1 );
		}
		else				// success !
		{
			if (!( pi->morez == 0 ||(pi->morex == cir && pi->morey == spl))  )
				sysmessage(s,"This item already has a spell!");

			else if ( pi->morez >= (9 - pi->morex)*2)
				sysmessage(s,"Item at max charges!");

			else
			{
				Skills->EngraveAction(s, pi, cir, spl);	// check mana & set name

				if (!(pi->morex == cir && pi->morey == spl))	// not THIS spell
				{
					pi->setType2( pi->type() ); //kept type of item for returning to this type when item remain no charge 
					pi->setType( 15 );  //make it magical
					pi->morex=cir;//spell circle
					pi->morey=spl;//spell number
					sysmessage(s,"Item successfully Engraved");
				}
				else
					sysmessage(s,"You added one more charge to the Item");

				pi->morez++;  // charges
				sysmessage(s,"Item successfully Engraved");
			}
		}
	}//else if*/
}

/////////////////////////////
// name:	Inscribe
// history:	unknown, modified by AntiChrist
//			partially revamped by Duke,8 April 2000
// Purpose:	bring up inscription menue
// Remarks:	snum is not needed here. Should change classes.h and targeting.cpp
//
int cSkills::Inscribe(int s,long snum) 
{
	if(buffer[s][11]==0xFF && buffer[s][12]==0xFF && buffer[s][13]==0xFF && buffer[s][14]==0xFF) 
	{
		LogError("Morrolan - Inscribing click on invalid object?");
		return 0;	// don't do this if the buffer got flushed - Morrolan prevents CRASH!
					// are you sure ??? that is click xLoc & click yLoc ! - Duke
	}

	P_ITEM pi = FindItemBySerPtr(buffer[s]+7);		// Find what they clicked on
	if (pi == NULL || !((pi->id()==0x0E34)||  //its a scroll
		(pi->att>0 || pi->def>0 || pi->hidamage()>0)))	// its something else
	{
		sysmessage(s,"You could not possibly engrave on that!");
		currchar[s]->lastTarget = INVALID_SERIAL;
	}
	else
	{
		currchar[s]->lastTarget = pi->serial;		//we gotta remember what they clicked on!

		/* select spell gump menu system here, must return control to WOLFPACK so we dont
		freeze the game. when returning to this routine, use snum to determine where to go
		and snum is also the value of what they selected. */
		im_sendmenu( "InscribeMenu", s );  //this is in im.cpp file using inscribe.gmp
	}
	return 0;
}

////////////////////////
// name:	TellScroll
// history:	unknown, modified by AntiChrist, totally revamped by Duke,8 April 2000
// Purpose:	check mana&regs and set the name of selected spell
//
int cSkills::EngraveAction(int s, P_ITEM pi, int cir, int spl)
{
	char *spn;					// spellname
	int num=(8*(cir-1))+spl;
	if (pi == NULL)
		return -1;
	Magic->DelReagents(currchar[s], num);
		
	switch(cir*10 + spl)
	{
	// first circle
	case 11: spn="Clumsy";
			pi->setOffspell( 1 );
			break;
	case 12: spn="Create Food";break;   
	case 13: spn="Feeblemind";
			pi->setOffspell( 2 );
			break;
	case 14: spn="Heal";break;
	case 15: spn="Magic Arrow";
			pi->setOffspell( 3 );
			break;
	case 16: spn="Night Sight";break;
	case 17: // Reactive Armor
			sysmessage(s, "Sorry this spell is not implemented!");
			return 0;
	case 18: spn="Weaken";
			pi->setOffspell( 4 );
			break;
	// 2nd circle
	case 21: spn="Agility";break;
	case 22: spn="Cunning";break;
	case 24: spn="Harm";
			pi->setOffspell( 5 );
			break;
	case 23:// Cure
	case 25:// Magic Trap
	case 26:// Magic Untrap
	case 27:// Protection
			sysmessage(s, "Sorry this spell is not implemented!");
			return 0;
	case 28: spn="Strength";break;
	// 3rd circle
	case 31: spn="Bless";break;
	case 32:// Fireball
			spn="Daemon's Breath";
			pi->setOffspell( 6 );
			break;
	case 34: spn="Poison";break;
	case 33: //Magic lock
	case 35: //Telekinesis
			sysmessage(s, "Sorry this spell is not implemented!");
			return 0;
	case 36: spn="Teleport";break;
	case 37: spn="Unlock";break;
	case 38: spn="Wall of Stone";break;
	// 4th circle
	case 41:// Arch Cure
	case 42:// Arch Protection
	case 47:// Mana Drain
			sysmessage(s, "Sorry this spell is not implemented!");
			return 0;
	case 43:// Curse
			spn="Evil's Eye";
			pi->setOffspell( 8 );
			break;
	case 44: spn="Fire Field";break;
	case 45: spn="Greater Heal";break;
	case 46: spn="Lightning";
			pi->setOffspell( 9 );
			break;
	case 48: spn="Recall";break;
	// 5th circle
	case 51: spn="Blade Spirit";break;
	case 52: spn="Dispel Field";break;
	case 54: spn="Magic Reflection";break;
	case 55: spn="Mind Blast";
			pi->setOffspell( 11 );
			break;
	case 56://Paralyze
			spn="Ghoul's Touch";
			pi->setOffspell( 12 );
			break;
	case 57: spn="Poison Field";break;
	case 53://Incognito
	case 58://Summon Creature
			sysmessage(s, "Sorry this spell is not implemented!");
			return 0;
	// 6th circle
	case 61: spn="Dispel";break;
	case 62: spn="Energy Bolt";
			pi->setOffspell( 13 );
			break;  
	case 63: spn="Explosion";
			pi->setOffspell( 14 );
			break;
	case 64: spn="Invisibility";break;
	case 65: spn="Mark";break;
	case 66://Mass Curse
			sysmessage(s, "Sorry this spell is not implemented!");
			return 0;
	case 67: spn="Paralyse Field";break;
	case 68: spn="Reveal";break;
	// 7th circle
	case 72: spn="Energy Field";break;
	case 73: spn="Flamestrike";
			pi->setOffspell( 15 );
			break;
	case 74: spn="Gate Travel";break;
	case 71:// Chain Lightning
	case 75:// Mana Vampire
	case 76:// Mass Dispel
	case 77:// Meteor Storm
	case 78:// Polymorph
	// 8th circle
	case 81:// Earthquake
			sysmessage(s, "Sorry this spell is not implemented!");
			return 0;
	case 82: spn="Energy Vortex";break;
	case 83: spn="Resurrection";break;
	case 84: spn="Summon Air Elemental";break;
	case 85: spn="Summon Daemon";break;
	case 86: spn="Summon earth Elemental";break;
	case 87: spn="Summon Fire Elemental";break;
	case 88: spn="Summon Water Elemental";break;
	default:
		LogError("switch reached default");
		return 0;
	}
	pi->setName2( QString( "of %s with" ).arg( spn ) );
	return 1;
}

// Calculate the skill of this character based on the characters baseskill and stats
void cSkills::updateSkillLevel(P_CHAR pc, int s)
{
	int temp = (((skill[s].st * pc->st()) / 100 +
		(skill[s].dx * pc->effDex()) / 100 +
		(skill[s].in * pc->in()) / 100)
		*(1000-pc->baseSkill(s)))/1000+pc->baseSkill(s);
	
		
	pc->setSkill( s, QMAX( static_cast<unsigned int>(pc->baseSkill(s)), static_cast<unsigned int>(temp) ) );
}

void CollectAmmo(int s, int a, int b)
{
	
	if (a)
	{
		P_ITEM pi = Items->SpawnItem(s, currchar[s],a,"#",1,0x0F,0x3F,0,1,1);
		if(pi == NULL) return;
		pi->att=0;
		sysmessage(s, "You collect the arrows.");
	}
	
	if (b)
	{
		P_ITEM pi = Items->SpawnItem(s, currchar[s], b,"#",1,'\x1B','\xFB',0,1,1);
		if(pi == NULL) return;
		pi->att=0;
		sysmessage(s, "You collect the bolts.");
	}
}

/*void cSkills::AButte(int s1, P_ITEM pButte)
{
	int v1,i;
	int arrowsquant=0;
	P_CHAR pc_currchar = currchar[s1];
	int type=Combat->GetBowType(pc_currchar);
	if(pButte->id()==0x100A)
	{ // East Facing Butte
		if ((pButte->pos.x > pc_currchar->pos.x)||(pButte->pos.y != pc_currchar->pos.y))
			v1=-1;
		else v1=pc_currchar->pos.x-pButte->pos.x;
	}
	else
	{ // South Facing Butte
		if ((pButte->pos.y>pc_currchar->pos.y)||(pButte->pos.x!=pc_currchar->pos.x))
			v1=-1;
		else v1=pc_currchar->pos.y-pButte->pos.y;
	}
	
	if(v1==1)
	{
		if(pButte->more1>0)
		{
			P_ITEM pi = Items->SpawnItem(s1, pc_currchar,pButte->more1/2,"#",1,0x0F,0x3F,0,1,0);
			if(pi == NULL) return;
			pi->update();
		}
		
		if(pButte->more2>0)
		{
			P_ITEM pi = Items->SpawnItem(s1,pc_currchar,pButte->more2/2,"#",1,0x1B,0xFB,0,1,0);
			if(pi == NULL) return;
			pi->update();
		}
		
		i=0;
		if(pButte->more1>0) i++;
		if(pButte->more2>0) i+=2;
		
		switch(i)
		{
		case 0:
			sprintf((char*)temp,"This target is empty");
			break;
		case 1:
			sprintf((char*)temp,"You pull %d arrows from the target",pButte->more1/2);
			break;
		case 2:
			sprintf((char*)temp,"You pull %d bolts from the target",pButte->more2/2);
			break;
		case 3:
			sprintf((char*)temp,"You pull %d arrows and %d bolts from the target",pButte->more1,pButte->more2/2);
			break;
		default:
			LogError("switch reached default");
			return;
		}
		sysmessage(s1,(char*)temp);
		pButte->more1=0;
		pButte->more2=0;
	}
	
	if((v1>=5)&&(v1<=8))
	{
		if (type == 0)
		{
			sysmessage(s1, "You need to equip a bow to use this.");
			return;
		} 
		if ((pButte->more1+pButte->more2)>99)
		{
			sysmessage(s1, "You should empty the butte first!");
			return;
		}
		if (type==1) 
			arrowsquant=getamount(pc_currchar, 0x0F3F);
		else 
			arrowsquant=getamount(pc_currchar, 0x1BFB);
		if (arrowsquant==0) 
		{
			sysmessage(s1, "You have nothing to fire!");
			return;
		}
		if (type==1) 
		{
			delequan(pc_currchar, 0x0F3F, 1);
			pButte->more1++;
			//add moving effect here to item, not character
		}
		else
		{
			delequan(pc_currchar, 0x1BFB, 1, NULL);
			pButte->more2++;
			//add moving effect here to item, not character
		} 
		if (pc_currchar->onHorse()) Combat->CombatOnHorse(pc_currchar);
		else Combat->CombatOnFoot(pc_currchar);
		
		if (pc_currchar->skill(ARCHERY) < 350)
			Skills->CheckSkill(pc_currchar,ARCHERY, 0, 1000);
		else
			sysmessage(s1, "You learn nothing from practicing here");

		switch((pc_currchar->skill(ARCHERY)+((rand()%200)-100))/100)
		{
		case -1:
		case 0:
		case 1:
			sysmessage(s1, "You miss the target");
			soundeffect(s1, 0x02, 0x38);
			break;
		case 2:
		case 3:
			sysmessage(s1, "You hit the outer ring!");
			soundeffect(s1, 0x02, 0x34);
			break;
		case 4:
		case 5:
		case 6:
			sysmessage(s1, "You hit the middle ring!");
			soundeffect(s1, 0x02, 0x34);
			break;
		case 7:
		case 8:
		case 9:
			sysmessage(s1, "You hit the inner ring!");
			soundeffect(s1, 0x02, 0x34);
			break;
		case 10:
		case 11:
			sysmessage(s1, "You hit the bullseye!!");
			soundeffect(s1, 0x02, 0x34);
			break;
		default:
			break;
		}
	}
	if ((v1>1)&&(v1<5)||(v1>8)) sysmessage(s1, "You cant use that from here.");
}*/

void cSkills::Meditation( cUOSocket *socket )
{
	P_CHAR pc_currchar = socket->player();

	if (pc_currchar->war())
	{
		socket->sysMessage( tr("Your mind is too busy with the war thoughts.") );
		return;
	}
	if (Skills->GetAntiMagicalArmorDefence(pc_currchar)>15) // blackwind armor affect fix
	{
		socket->sysMessage( tr("Regenerative forces cannot penetrate your armor."));
		pc_currchar->setMed(false);
		return;
	}
	else if (pc_currchar->hasWeapon() || pc_currchar->hasShield())
	{
		socket->sysMessage( tr("You cannot meditate with a weapon or shield equipped!"));
		pc_currchar->setMed( false );
		return;
	}
	else if ( pc_currchar->mn() == pc_currchar->in() )
	{
		socket->sysMessage( tr("You are at peace."));
		pc_currchar->setMed( false );
		return;
	}
	else if (!pc_currchar->checkSkill( MEDITATION, 0, 1000))
	{
		socket->sysMessage( tr("You cannot focus your concentration."));
		pc_currchar->setMed( false );
		return;
	}
	else
	{
		socket->sysMessage( tr("You enter a meditative trance."));
		pc_currchar->setMed( true );
		soundeffect(toOldSocket(socket), 0x00, 0xf9);
		return;
	}
}

// If you are a ghost and attack a player, you can PERSECUTE him
// and his mana decreases each time you try to persecute him
// decrease=3+(your int/10)
void cSkills::Persecute ( cUOSocket* socket )
{
	P_CHAR pc_currchar = socket->player();

	if( !pc_currchar )
		return;

	P_CHAR target = FindCharBySerial(pc_currchar->targ);

	if( target->isGM() )
		return;

	int decrease = ( pc_currchar->in() / 10 ) + 3;

	if((pc_currchar->skilldelay<=uiCurrentTime) || pc_currchar->isGM())
	{
		if(((rand()%20)+pc_currchar->in())>45) //not always
		{
			if( target->mn() <= decrease )
				target->setMn(0);
			else 
//				target->mn-=decrease;//decrease mana
				target->setMn(target->mn() - decrease);
			updatestats(target,1);//update
			socket->sysMessage(tr("Your spiritual forces disturb the enemy!"));
			if ( target->socket() )
			{
				target->socket()->sysMessage(tr("A damned soul is disturbing your mind!"));
			}
			pc_currchar->setSkillDelay();

			sprintf((char*)temp, "%s is persecuted by a ghost!!", target->name.c_str());
					
			// Dupois pointed out the for loop was changing i which would drive stuff nuts later
				
			for( cUOSocket *s = cNetwork::instance()->first(); s; s = cNetwork::instance()->next() )
			{
				if(socket->inRange(s) && s != socket) 
				{
					pc_currchar->setEmoteColor( 0x0026 );
					npcemote(toOldSocket(s), target, (char*)temp, 1);
				}
			}
		} 
		else
		{
			socket->sysMessage( tr( "Your mind is not strong enough to disturb the enemy." ) );
		}
	} 
	else
	{
		socket->sysMessage( tr( "You are unable to persecute him now...rest a little..." ) );
	}
}

void loadskills()
{
	QStringList skills = DefManager->getSections( WPDT_SKILL );
	UINT16 l = 0;

	for( UINT32 i = 0; i < skills.count(); ++i )
	{
		bool ok = false;
		UINT16 skillId = skills[i].toInt( &ok );

		if( !ok || skillId > SKILLS ) 
			continue;

        QDomElement *skillNode = DefManager->getSection( WPDT_SKILL, skills[i] );
		if( skillNode->isNull() )
			skillNode = DefManager->getSection( WPDT_SKILL, QString( skillname[i] ).lower() );
		if( skillNode->isNull() )
			continue;

		QDomNode childNode = skillNode->firstChild();
		
		skill[skillId].advance_index = l;
		
		while( !childNode.isNull() )
		{
			QDomElement node = childNode.toElement();
			if( !node.isNull() )
			{
				if( node.nodeName() == "str" )
					skill[skillId].st = node.text().toInt();
				
				else if( node.nodeName() == "dex" )
					skill[skillId].dx = node.text().toInt();

				else if( node.nodeName() == "int" )
					skill[skillId].in = node.text().toInt();

				else if( node.nodeName() == "makeword" )
					skill[skillId].madeword = node.text();

				else if( node.nodeName() == "advancement" )
				{
					wpadvance[l].base = node.attribute( "base", "0" ).toInt();
					wpadvance[l].failure = node.attribute( "failure", "0" ).toInt();
					wpadvance[l].success = node.attribute( "success", "0" ).toInt();
					wpadvance[l].skill = skillId;
					l++;
				}
			}

			childNode = childNode.nextSibling();
		}
	}
}

void SkillVars()
{
/*	strcpy(skill[ALCHEMY].madeword,"mixed");
	strcpy(skill[ANATOMY].madeword,"made");
	strcpy(skill[ANIMALLORE].madeword,"made");
	strcpy(skill[ITEMID].madeword,"made");
	strcpy(skill[ARMSLORE].madeword,"made");
	strcpy(skill[PARRYING].madeword,"made");
	strcpy(skill[BEGGING].madeword,"made");
	strcpy(skill[BLACKSMITHING].madeword,"forged");
	strcpy(skill[BOWCRAFT].madeword,"bowcrafted");
	strcpy(skill[PEACEMAKING].madeword,"made");
	strcpy(skill[CAMPING].madeword,"made");
	strcpy(skill[CARPENTRY].madeword,"made");
	strcpy(skill[CARTOGRAPHY].madeword,"wrote");
	strcpy(skill[COOKING].madeword,"cooked");
	strcpy(skill[DETECTINGHIDDEN].madeword,"made");
	strcpy(skill[ENTICEMENT].madeword,"made");
	strcpy(skill[EVALUATINGINTEL].madeword,"made");
	strcpy(skill[HEALING].madeword,"made");
	strcpy(skill[FISHING].madeword,"made");
	strcpy(skill[FORENSICS].madeword,"made");
	strcpy(skill[HERDING].madeword,"made");
	strcpy(skill[HIDING].madeword,"made");
	strcpy(skill[PROVOCATION].madeword,"made");
	strcpy(skill[INSCRIPTION].madeword,"wrote");
	strcpy(skill[LOCKPICKING].madeword,"made");
	strcpy(skill[MAGERY].madeword,"envoked");
	strcpy(skill[MAGICRESISTANCE].madeword,"made");
	strcpy(skill[TACTICS].madeword,"made");
	strcpy(skill[SNOOPING].madeword,"made");
	strcpy(skill[MUSICIANSHIP].madeword,"made");
	strcpy(skill[POISONING].madeword,"made");
	strcpy(skill[ARCHERY].madeword,"made");
	strcpy(skill[SPIRITSPEAK].madeword,"made");
	strcpy(skill[STEALING].madeword,"made");
	strcpy(skill[TAILORING].madeword,"sewn");
	strcpy(skill[TAMING].madeword,"made");
	strcpy(skill[TASTEID].madeword,"made");
	strcpy(skill[TINKERING].madeword,"made");
	strcpy(skill[TRACKING].madeword,"made");
	strcpy(skill[VETERINARY].madeword,"made");
	strcpy(skill[SWORDSMANSHIP].madeword,"made");
	strcpy(skill[MACEFIGHTING].madeword,"made");
	strcpy(skill[FENCING].madeword,"made");
	strcpy(skill[WRESTLING].madeword,"made");
	strcpy(skill[LUMBERJACKING].madeword,"made");
	strcpy(skill[MINING].madeword,"smelted");
	strcpy(skill[MEDITATION].madeword,"envoked");
	strcpy(skill[STEALTH].madeword,"made");
	strcpy(skill[REMOVETRAPS].madeword,"made");*/
}

int cSkills::GetAntiMagicalArmorDefence(P_CHAR pc)
{
	int ar = 0;

	if (ishuman(pc))
	{
		
		unsigned int ci = 0;
		P_ITEM pi;
		vector<SERIAL> vecContainer = contsp.getData(pc->serial);
		for ( ci = 0; ci < vecContainer.size(); ci++)
		{
			pi = FindItemBySerial(vecContainer[ci]);
			if (pi != NULL)
			if (pi->layer()>1 && pi->layer() < 25)
			{
				if (!(strstr(pi->name().ascii(), "leather") || strstr(pi->name().ascii(), "magic") ||
					strstr(pi->name().ascii(), "boot")|| strstr(pi->name().ascii(), "mask")))
					ar += pi->def;
			}
		}
	}
	return ar;
}

void cSkills::Snooping( P_CHAR player, P_ITEM container )
{
	cUOSocket *socket = player->socket();

	if( !socket )
		return;

	P_CHAR pc_owner = GetPackOwner( container );

	if( pc_owner->isGMorCounselor() )
	{
		pc_owner->message( tr( "%1 is trying to snoop in your pack" ).arg( player->name.c_str() ) );
		socket->sysMessage( tr( "You can't peek into that container or you'll be jailed." ) );
		return;
	}
	else if( player->checkSkill( SNOOPING, 0, 1000 ) )
	{
		socket->sendContainer( container );
		socket->sysMessage( tr( "You successfully peek into that container." ) );
	}
	else
	{
		socket->sysMessage( tr( "You failed to peek into that container." ) );

		if( pc_owner->isNpc() )
			pc_owner->talk( tr( "Art thou attempting to disturb my privacy?" ) );
		else
			pc_owner->message( tr( "You notice %1 trying to peek into your pack!" ).arg( player->name.c_str() ) );
	}

	SetTimerSec(&player->objectdelay, SrvParams->objectDelay()+SrvParams->snoopdelay());
}

void cSkills::Cartography( cUOSocket* socket )
{
	cAllMakeMenus::getInstance()->callMakeMenu( socket, "CRAFTMENU_CARTOGRAPHY" );
}

/*
	By Polygon:
	Attempt to decipher a tattered treasure map
	Called when double-clicked such a map
*/

void cSkills::Decipher(P_ITEM tmap, int s)
{
/*	P_CHAR pc_currchar = currchar[s];
	char sect[500];			// Needed for script search
	int regtouse;			// Stores the region-number of the TH-region
	int i;					// Loop variable
	int btlx, btly, blrx, blry; // Stores the borders of the tresure region (topleft x-y, lowright x-y)
	int tlx, tly, lrx, lry;		// Stores the map borders
	int x, y;					// Stores the final treasure location

	if(pc_currchar->skilldelay<=uiCurrentTime || pc_currchar->isGM())	// Char doin something?
	{
		if (CheckSkill(pc_currchar, CARTOGRAPHY, tmap->morey * 10, 1000))	// Is the char skilled enaugh to decipher the map
		{
			P_ITEM nmap = Items->SpawnItemBackpack2(s, "70025", 0);
			if (nmap == NULL)
			{
				LogWarning("bad script item # 70025(Item Not found).");
				return;	//invalid script item
			} 
			char temp[256];
			sprintf(temp, "a deciphered lvl.%d treasure map", tmap->morez);	// Give it the correct name
			nmap->setName( temp );
			nmap->morez = tmap->morez;				// Give it the correct level
			nmap->creator = pc_currchar->name;	// Store the creator
			Script *rscript=i_scripts[regions_script];	// Region script
			if (!rscript->Open())
			{
				LogWarning("Treasure hunting cSkills::Decipher : Cannot open regions-script");
				return;
			}
			sprintf(sect, "TREASURE%i", nmap->morez);
			if (!rscript->find(sect)) 
			{
				rscript->Close();
				LogWarningVar("Treasure hunting cSkills::Decipher : Unable to find 'SECTION TREASURE%X' in regions-script", nmap->morez);
				return;
			}
			rscript->NextLine();				// Get the number of areas
			regtouse = rand()%str2num(script1);	// Select a random one
			for (i = 0; i < regtouse; i++)		// Skip the ones before the correct one
			{
				rscript->NextLine();
				rscript->NextLine();
				rscript->NextLine();
				rscript->NextLine();
			}
			rscript->NextLine();				// Get the treasure region borders
			btlx = str2num(script1);
			rscript->NextLine();
			btly = str2num(script1);
			rscript->NextLine();
			blrx = str2num(script1);
			rscript->NextLine();
			blry = str2num(script1);
			rscript->Close();					// Close the script
			if ((btlx < 0) || (btly < 0) || (blrx > 0x13FF) || (blry > 0x0FFF))	// Valid region?
			{
				sprintf(sect, "Treasure Hunting cSkills::Decipher : Invalid region borders for lvl.%d , region %d", nmap->morez, regtouse+1);	// Give out detailed warning :D
				LogWarning(sect);
				return;
			}
			x = btlx + (rand()%(blrx-btlx));	// Generate treasure location
			y = btly + (rand()%(blry-btly));
			tlx = x - 250;		// Generate map borders
			tly = y - 250;
			lrx = x + 250;
			lry = y + 250;
			// Check if we are over the borders and correct errors
			if (tlx < 0)	// Too far left?
			{
				lrx -= tlx;	// Add the stuff too far left to the right border (tlx is neg. so - and - gets + ;)
				tlx = 0;	// Set tlx to correct value
			}
			else if (lrx > 0x13FF) // Too far right?
			{
				tlx -= lrx - 0x13FF;	// Subtract what is to much from the left border
				lrx = 0x13FF;	// Set lrx to correct value
			}
			if (tly < 0)	// Too far top?
			{
				lry -= tly;	// Add the stuff too far top to the bottom border (tly is neg. so - and - gets + ;)
				tly = 0;	// Set tly to correct value
			}
			else if (lry > 0x0FFF) // Too far bottom?
			{
				tly -= lry - 0x0FFF;	// Subtract what is to much from the top border
				lry = 0x0FFF;	// Set lry to correct value
			}
			nmap->more1 = tlx>>8;	// Store the map extends
			nmap->more2 = tlx%256;
			nmap->more3 = tly>>8;
			nmap->more4 = tly%256;
			nmap->setMoreb1( lrx>>8 );
			nmap->setMoreb2( lrx%256 );
			nmap->setMoreb3( lry>>8 );
			nmap->setMoreb4( lry%256 );
			nmap->morex = x;		// Store the treasure's location
			nmap->morey = y;
			Items->DeleItem(tmap);	// Delete the tattered map
		}
		else
			sysmessage(s, "You fail to decipher the map");		// Nope :P
		// Set the skill delay, no matter if it was a success or not
		SetTimerSec(&pc_currchar->skilldelay, SrvParams->skillDelay());
		soundeffect(s, 0x02, 0x49);	// Do some inscription sound regardless of success or failure
		sysmessage(s, "You put the deciphered tresure map in your pack");	// YAY
	}
	else
		sysmessage(s, "You must wait to perform another action");	// wait a bit
*/
}

////////////////////
// name:	Carpentry()
// history:	unknown, Duke, 25.05.2000, rewritten for 13.x sereg, 16.08.2002
// purpose:	sets up appropriate Makemenu when player dclick on carpentry tool
//			

void cSkills::Carpentry( cUOSocket* socket )
{
	cAllMakeMenus::getInstance()->callMakeMenu( socket, "CRAFTMENU_CARPENTRY" );
}


void cSkills::Fletching( cUOSocket* socket )
{
	cAllMakeMenus::getInstance()->callMakeMenu( socket, "CRAFTMENU_FLETCHING" );
}

void cSkills::Tailoring( cUOSocket* socket )
{
	cAllMakeMenus::getInstance()->callMakeMenu( socket, "CRAFTMENU_TAILORING" );
}

void cSkills::Blacksmithing( cUOSocket* socket )
{
	P_CHAR pc = socket->player();
	bool foundAnvil = false;
	AllItemsIterator iterItems;
	for (iterItems.Begin(); !iterItems.atEnd() && !foundAnvil;iterItems++)
	{
		P_ITEM pi = iterItems.GetData();
		foundAnvil = ( pi && IsAnvil( pi->id() ) && pc->inRange( pi, 3 ) );
	}
	if( !foundAnvil )
	{
		socket->sysMessage( tr("You must stand in range of an anvil!") );
		return;
	}

	cAllMakeMenus::getInstance()->callMakeMenu( socket, "CRAFTMENU_BLACKSMITHING" );
}

void cSkills::Tinkering( cUOSocket* socket )
{
	cAllMakeMenus::getInstance()->callMakeMenu( socket, "CRAFTMENU_TINKERING" );
}




