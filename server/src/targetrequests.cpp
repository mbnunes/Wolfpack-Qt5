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

// Wolfpack Includes
#include "targetrequests.h"
#include "maps.h"
#include "mapobjects.h"
#include "wpdefmanager.h"
#include "territories.h"
#include "items.h"
//#include "tilecache.h"
#include "srvparams.h"
#include "skills.h"
#include "guildstones.h"
#include "combat.h"
#include "scriptmanager.h"
#include "pythonscript.h"
#include "books.h"
#include "house.h"
#include "boats.h"
#include "accounts.h"
#include "makemenus.h"
#include "itemid.h"
#include "npc.h"
#include "chars.h"


// System Includes
#include <math.h>

bool cAddItemTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	if( target->x() == 0xFFFF || target->y() == 0xFFFF || target->z() == 0xFF )
		return true;

	//QStringList arguments = QStringList::split( " ", npc_ );
	const cElement *node = DefManager->getDefinition( WPDT_ITEM, item_ );

	// Check first if we even are able to create a char
	if( !node )
	{
		bool ok = false;
		hex2dec( item_ ).toULong( &ok );
		if( !ok )
		{
			socket->sysMessage( tr( "Item Definition '%1' not found" ).arg( item_ ) );
			return true;
		}
	}

	// Otherwise create our item here
	P_ITEM pItem = NULL;
	if( node )
		pItem = Items->createScriptItem( item_ );
	else
	{
		pItem = new cItem;
		pItem->Init();

		pItem->setName( "an item" );
		pItem->setId( hex2dec( item_ ).toULong() );
	}

	if( !pItem )
		return true;

	Coord_cl newPos = socket->player()->pos();
	newPos.x = target->x();
	newPos.y = target->y();
	newPos.z = target->z() + TileCache::instance()->tileHeight( target->model() ); // Model Could be an NPC as well i dont like the idea...
	pItem->moveTo( newPos );

	if( nodecay )
	{
		pItem->setDecayTime( 0 );
		pItem->setWipe( false );
	}

	// Send the item to its surroundings
	pItem->update();
	return true;
}

bool cAddNpcTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	if( target->x() == 0xFFFF || target->y() == 0xFFFF || target->z() == 0xFF )
		return true;

	//QStringList arguments = QStringList::split( " ", npc_ );
	const cElement *node = DefManager->getDefinition( WPDT_NPC, npc_ );

	// Check first if we even are able to create a char
	if( !node )
	{
		bool ok = false;
		hex2dec( npc_ ).toULong( &ok );
		if( !ok )
		{
			socket->sysMessage( tr( "NPC Definition '%1' not found" ).arg( npc_ ) );
			return true;
		}
	}

	// Otherwise create our character here
	P_NPC pChar = new cNPC;
	pChar->Init();

	Coord_cl newPos = socket->player()->pos();
	newPos.x = target->x();
	newPos.y = target->y();
	newPos.z = target->z() + TileCache::instance()->tileHeight( target->model() ); // Model Could be a NPC as well i dont like the idea...
	pChar->moveTo( newPos );

	pChar->setRegion( AllTerritories::instance()->region( pChar->pos().x, pChar->pos().y, pChar->pos().map ) );
	if( node )
	{
		pChar->applyDefinition( node );
	}
	else
	{
		pChar->setName("Character");
		pChar->setBodyID( hex2dec( npc_ ).toULong() );
		pChar->setOrgBodyID( pChar->bodyID() );
	}

	// Send the char to it's surroundings
	setcharflag( pChar );
	pChar->resend( false ); // It's new so no need to remove it first

	return true;
};

bool cBuildMultiTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	if( target->x() & 0xFFFF || target->y() & 0xFFFF || target->z() & 0xFF )
		return true;
	
	const cElement* DefSection = DefManager->getDefinition( WPDT_MULTI, multisection_ );
	
	if( !DefSection )
		return true;

	if( DefSection->getAttribute( "type" ) == "house" )
	{
		cHouse* pHouse = new cHouse();
		
		pHouse->build( DefSection, target->x(), target->y(), target->z(), senderserial_, deedserial_ );
	}
	else if( DefSection->getAttribute( "type" ) == "boat" )
	{
		cBoat* pBoat = new cBoat();

		pBoat->build( DefSection, target->x(), target->y(), target->z(), senderserial_, deedserial_ );
	}
	return true;
};

bool cSkDetectHidden::responsed( cUOSocket *socket, cUORxTarget *target )
{
	P_PLAYER pChar = socket->player();
	
	if( !pChar )
		return true;
	
	Coord_cl dPos = pChar->pos();
	dPos.x = target->x();
	dPos.y = target->y();
	dPos.z = target->z();
	
	// If its out of the characters visrange cancel (How could he've clicked there??)
	if( dPos.distance( pChar->pos() ) > pChar->visualRange() )
		return true;
	
	UINT16 dSkill = pChar->skillValue( DETECTINGHIDDEN );		
	double range = ( dSkill * dSkill / 1.0E6 ) * VISRANGE; // this seems like an ok formula
	
	RegionIterator4Chars ri( dPos );
	bool found = false;
	
	for( ri.Begin(); !ri.atEnd(); ri++ )
	{
		P_CHAR hChar = ri.GetData();
		if( hChar )
		{
			if( hChar->isHidden() && !hChar->isInvisible() ) // do not detect invis people only hidden ones
			{
				UINT16 dx = abs( hChar->pos().x - dPos.x );
				UINT16 dy = abs( hChar->pos().y - dPos.y );
				double c = hypot( dx, dy );
				
				INT16 low = (UINT16)( hChar->skillValue(HIDING) * hChar->skillValue(HIDING) / 1E3 - ( range * 50 / VISRANGE ) * ( range - c ) / range );
				if( low < 0 ) 
					low = 0;
				else if( low > 1000 )
					low = 1000;
				
				if( ( pChar->checkSkill( DETECTINGHIDDEN, low, 1000 ) ) && ( c <= range ) )
				{
					hChar->unhide();
					if( hChar->objectType() == enPlayer )
						dynamic_cast<P_PLAYER>(hChar)->message( tr( "You have been revealed!" ) );

					found = true;
				}
			}
		}
	}
	
	if( !found )
		socket->sysMessage( tr( "You didn't find anything hidden" ) );
	return true;
}

bool cSkItemID::responsed( cUOSocket *socket, cUORxTarget *target )
{
	P_CHAR pc_currchar = socket->player();
	const P_ITEM pi = FindItemBySerial( target->serial() );
	if( !pi )
	{
		socket->sysMessage( tr("Unable to identify that.") );
		return true;
	}
	
	if ( !pi->isLockedDown() ) // Ripper
	{
		if( !pc_currchar->checkSkill( ITEMID, 0, 250 ) )
		{
			socket->clilocMessage( 0x7A281 ); // You are not certain...
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
					if (pi->creator().length()>0)
					{
						if (pi->madewith()>0) 
							socket->sysMessage( tr("It is crafted by %2").arg(pi->creator()) ); // Magius(CHE)
						else if (pi->madewith()<0) 
							socket->sysMessage( tr("It is crafted by %2").arg(pi->creator()) ); // Magius(CHE)
						else 
							socket->sysMessage( tr("It is made by %1").arg(pi->creator()) ); // Magius(CHE)
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
							//								if (!pc_currchar->checkSkill( ITEMID, 750, 1100, false ))
							//								{
							//									socket->sysMessage( tr("It is enchanted with the spell %1, but you cannot determine how many charges remain.").arg(spellname[(8*(pi->morex()-1))+pi->morey()-1]) );
							//								}
							//								else
							//								{
							//									socket->sysMessage( tr("It is enchanted with the spell %1, and has %2 charges remaining.").arg(spellname[(8*(pi->morex()-1))+pi->morey()-1]).arg(pi->morez()) );
							//								}
						}
					}
				}
			}
		}
	}
	return true;
}

bool cSkIntEval::responsed( cUOSocket *socket, cUORxTarget *target )
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
	if( pc->dist(pc_currchar) >= 10 ) 
	{ 
		socket->sysMessage( tr("You need to be closer to find out") ); 
		return true; 
	} 
	
	if (!pc_currchar->checkSkill(EVALUATINGINTEL, 0, 1000)) 
	{
		socket->sysMessage( tr("You are not certain..") );
		return true;
	}
	if( !pc->intelligence() )
		socket->sysMessage( tr("It looks smarter than a rock, but dumber than a piece of wood.") );
	else
	{
		if		(pc->intelligence() <= 10)	socket->sysMessage( tr("That person looks slightly less intelligent than a rock.") );
		else if (pc->intelligence() <= 20)	socket->sysMessage( tr("That person looks fairly stupid.") );
		else if (pc->intelligence() <= 30)	socket->sysMessage( tr("That person looks not the brightest.") );
		else if (pc->intelligence() <= 40)	socket->sysMessage( tr("That person looks about average.") );
		else if (pc->intelligence() <= 50)	socket->sysMessage( tr("That person looks moderately intelligent.") );
		else if (pc->intelligence() <= 60)	socket->sysMessage( tr("That person looks very intelligent.") );
		else if (pc->intelligence() <= 70)	socket->sysMessage( tr("That person looks extremely intelligent.") );
		else if (pc->intelligence() <= 80)	socket->sysMessage( tr("That person looks extraordinarily intelligent.") );
		else if (pc->intelligence() <= 90)	socket->sysMessage( tr("That person looks like a formidable intellect, well beyond the ordinary.") );
		else if (pc->intelligence() <= 99)	socket->sysMessage( tr("That person looks like a definite genius.") );
		else if (pc->intelligence() >=100)	socket->sysMessage( tr("That person looks superhumanly intelligent in a manner you cannot comprehend.") );
	}
	return true;
}

bool cSkTame::responsed( cUOSocket *socket, cUORxTarget *target )
{
	
	P_CHAR pc = FindCharBySerial(target->serial());
	if ( pc == NULL ) 
		return true; 
	P_PLAYER pc_currchar = socket->player();
	
	if( !lineOfSight( pc_currchar->pos(), pc->pos(), WALLS_CHIMNEYS+DOORS+FLOORS_FLAT_ROOFING ) )
		return true;
	
	bool tamed = false;
	if ((pc->objectType() == enNPC) && ( pc_currchar->dist( pc ) <= 3))
	{
		P_NPC pn = dynamic_cast<P_NPC>(pc);
		if (pn->tamingMinSkill()>1200||pn->tamingMinSkill() == 0 )//Morrolan default is now no tame
		{
			socket->sysMessage( tr("You can't tame that creature.") );
			return true;
		}
		
		// Below... can't tame if you already have!
		if( pn->owner() == pc_currchar )
		{
			socket->sysMessage( tr("You already control that creature!" ) );
			return true;
		}
		
		// This creature is owned by someone else
		if( pn->owner() )
		{
			socket->sysMessage( tr("That creature looks tame already." ) );
			return true;
		}
		
		sprintf((char*)temp, "*%s starts to tame %s*",pc_currchar->name().latin1(),pn->name().latin1());
		for(int a=0;a<3;a++)
		{
			switch( RandomNum( 0, 3 ) )
			{
			case 0: pc_currchar->talk( tr("I've always wanted a pet like you."), -1 ,0);		break;
			case 1: pc_currchar->talk( tr("Will you be my friend?"), -1 ,0);					break;
			case 2: pc_currchar->talk( tr("Here %1.").arg(pn->name()), -1 ,0);			break;
			case 3: pc_currchar->talk( tr("Good %1.").arg(pn->name()), -1 ,0);			break;
			}
		}
		if ((!pc_currchar->checkSkill(TAMING, 0, 1000))||
			(pc_currchar->skillValue(TAMING)<pn->tamingMinSkill())) 
		{
			socket->sysMessage( tr("You were unable to tame it.") );
			
			return true;
		}
		socket->showSpeech( pc, tr("It seems to accept you as it's master!"));
		tamed = true;
		pn->setOwner( pc_currchar );
		pn->setTamed(true);
		pn->setWanderType(enHalt);
		if( pn->bodyID() == 0x000C || pn->bodyID() == 0x003B )
		{
			if(pn->skin() != 0x0481)
			{
//				pn->setNpcAIType( 10 );
				pn->resend();
			}
			else
			{
//				pn->setNpcAIType( 0 );
				pn->resend();
			}
		}
	}
	
	if( !tamed ) 
		socket->sysMessage( tr("You can't tame that!") );
	
	return true;
}

bool cSkArmsLore::responsed( cUOSocket *socket, cUORxTarget *target )
{
	P_ITEM pItem = FindItemBySerial( target->serial() );
	P_PLAYER pChar = socket->player();
	
	if( !pChar )
		return true;
	
	if( !pItem || !target->serial() )
	{
		socket->sysMessage( tr( "You need to target an item" ) );
		return true;
	}
	
	if( ( ( pItem->lodamage() == 0 ) && ( pItem->hidamage() == 0 ) ) && !pItem->def() )
	{
		socket->sysMessage( tr( "This does not appear to be a weapon" ) );
		return true;
	}
	
	if( pChar->isGM() )
	{
		socket->sysMessage( tr("Attack [%1] Defense [%2] Lodamage [%3] Hidamage [%4]").arg( pItem->att() ).arg( pItem->def() ).arg( pItem->lodamage() ).arg( pItem->hidamage() ) );
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
		else if( pItem->def() )
		{
			QString status;
			
			if      ( pItem->def() > 12) status = tr( "is superbly crafted to provide maximum protection" );
			else if ( pItem->def() > 10) status = tr( "offers excellent protection" );
			else if ( pItem->def() > 8 ) status = tr( "is a superior defense against attack" );
			else if ( pItem->def() > 6 ) status = tr( "serves as a sturdy protection" );
			else if ( pItem->def() > 4 ) status = tr( "offers some protection against blows" );
			else if ( pItem->def() > 2 ) status = tr( "provides very little protection" );
			else if ( pItem->def() > 0 ) status = tr( "provides almost no protection" );
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
	if( ( pItem->rank() > 0 ) && ( pItem->rank() < 11 ) && SrvParams->rank_system() && pChar->checkSkill( ARMSLORE, 250, 500, false ) )
	{
		switch( pItem->rank() )
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
	
bool cSkBegging::responsed( cUOSocket *socket, cUORxTarget *target )
{
	int gold,x,y,realgold;
	char abort;
	P_PLAYER pc_currchar = socket->player();
	
	P_CHAR pChar = FindCharBySerial( target->serial() );
	if (!pChar)
		return true;
	
	if(pChar->objectType() == enPlayer)
	{
		socket->sysMessage( tr("Maybe you should just ask.") );
		return true;
	}

	P_NPC pc = dynamic_cast<P_NPC>(pChar);
	
	if(pc->dist(pc)>=5)
	{
		socket->sysMessage( tr("You are not close enough to beg.") );
		return true;
	}
	
	if(pc->isHuman() && (pc->intelligence() != 0)) //Used on human
	{
		if (pc->nextBeggingTime()>=uiCurrentTime)
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
			pc->setNextBeggingTime( SrvParams->beggingTime() * MY_CLOCKS_PER_SEC + uiCurrentTime); 
			x=pc->skillValue(BEGGING)/50;
			
			if (x<1) x=1; 
			y=rand()%x;
			y+=RandomNum(1,4); 
			if (y>25) y=25;
			// pre-calculate the random amout of gold that is "targeted"
			
			P_ITEM pi_p = pc->getBackpack();
			gold=0;
			realgold=0;
			abort=0;
			
			// check for gold in target-npc pack
			
			if (pi_p != NULL)				
			{
				cItem::ContainerContent container(pi_p->content());
				cItem::ContainerContent::const_iterator it (container.begin());
				cItem::ContainerContent::const_iterator end(container.end());
				for (; it != end; ++it )
				{
					P_ITEM pi_j =  *it;
					if (pi_j != NULL)
					{
						if (pi_j->id() == 0x0EED )
						{
							gold += pi_j->amount(); // calc total gold in pack
							
							int k = pi_j->amount();
							if(k >= y) // enough money in that pile in pack to satisfy pre-aclculated amount
							{
								pi_j->reduceAmount( y );
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
					if (abort)
						break;
				} 
			}
			
			if (gold<=0)
			{				
				pc->talk( tr("Thou dost not look trustworthy... no gold for thee today! ") );
				return true;
			}
			pc->talk( tr("I feel sorry for thee... here have a gold coin ."), -1, 0 );
			addgold(socket, realgold);
			socket->sysMessage( tr("Some gold is placed in your pack.") );
		}
	}
	else
		socket->sysMessage( tr("That would be foolish.") );
	return true;
}

bool cSkStealing::responsed( cUOSocket *socket, cUORxTarget *target )
{
	int i, skill;
	char temp2[512];
	tile_st tile;
	P_PLAYER pc_currchar = socket->player();
	int cansteal = QMAX( 1, pc_currchar->skillValue( STEALING ) / 10 );
	cansteal = cansteal * 10;
	
	if( isCharSerial( target->serial() ) )
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
	
	P_CHAR pc_npc = pi->getOutmostChar();
	
/*	if (pc_npc->npcaitype() == 17)
	{
		socket->sysMessage( tr("You cannot steal that.") );
		return true;
	}*/
	
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
			P_ITEM pi_pack = pc_currchar->getBackpack();
			if (pi_pack == NULL) 
				return true;
			pi_pack->addItem(pi);
			socket->sysMessage( tr("You successfully steal that item.") );
			pi->update();
		} 
		else 
			socket->sysMessage( tr("You failed to steal that item.") );
		
		if (((!(skill))&&(rand()%16==7)) || (pc_currchar->skillValue(STEALING)<rand()%1001))
		{
			socket->sysMessage( tr("You have been cought!") );
			
			if (pc_npc != NULL) //lb
			{
				if (pc_npc->objectType() == enNPC) 
					pc_npc->talk( tr("Guards!! A thief is amoung us!"), -1, 0x09 );
				
				pc_currchar->isCriminal();
				
				if (pc_npc->isInnocent() && pc_currchar->attackerSerial() != pc_npc->serial() && GuildCompare(pc_currchar, pc_npc)==0)//AntiChrist
					pc_currchar->isCriminal();//Blue and not attacker and not guild
				
				if (pi->name() != "#")
				{
					sprintf((char*)temp, tr("You notice %1 trying to steal %2 from you!").arg(pc_currchar->name()).arg(pi->name()) );
					sprintf((char*)temp2, tr("You notice %1 trying to steal %2 from %3!").arg(pc_currchar->name()).arg(pi->name()).arg(pc_npc->name()) );
				} 
				else
				{
					tile = TileCache::instance()->getTile( pi->id() );
					sprintf((char*)temp, tr("You notice %1 trying to steal %2 from you!").arg(pc_currchar->name()).arg((char*)tile.name) );
					sprintf((char*)temp2,tr("You notice %1 trying to steal %2 from %3!").arg(pc_currchar->name()).arg((char*)tile.name).arg(pc_npc->name()) );
				}
				socket->sysMessage((char*)temp); //lb
			}
			for ( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next())
			{
				if( mSock != socket && inrange1p( pc_currchar, mSock->player() ) && (rand()%10+10==17|| (rand()%2==1 && mSock->player()->intelligence() >= pc_currchar->intelligence()))) 
					mSock->sysMessage(temp2);
			}
		}
	} 
	else 
		socket->sysMessage( tr("You are too far away to steal that item.") );
	return true;
}

bool cSkProvocation::acquireFirst( cUOSocket* socket, cUORxTarget* target )
{
	P_CHAR pc = FindCharBySerial( target->serial() );
	
	if( pc == NULL ) 
		return true; // cancel the skill
	
	P_ITEM inst = Skills->GetInstrument(socket);
	if (inst == NULL) 
	{
		socket->sysMessage( tr("You do not have an instrument to play on!") );
		return true; // cancel the skill
	}
	if ( pc->isInvulnerable() ) // invul
/*		pc->npcaitype()==0x01 || // healer
		pc->npcaitype()==0x04 || // tele guard
		pc->npcaitype()==0x06 || // chaos guard
		pc->npcaitype()==0x07 || // order guard
		pc->npcaitype()==0x09)   // city guard*/
	{
		socket->sysMessage( tr(" You cant entice that npc!") );
		return true;
	}
	if (pc->inGuardedArea())
	{
		socket->sysMessage( tr(" You cant do that in town!") );
		return true;
	}
	
	if (pc->objectType() == enPlayer)
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

bool cSkProvocation::selectVictim( cUOSocket* socket, cUORxTarget* target )
{
	P_CHAR Victim = FindCharBySerial( target->serial() );
	if (!Victim)
		return true;
	
	P_PLAYER Player = socket->player();
	
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
				Coord_cl cPos = Player->pos();
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
		attacker->setAttackFirst( true );
		
		Victim->fight(attacker);
		Victim->setAttackFirst( false );
		
		QString temp(tr("* You see %1 attacking %2 *").arg(attacker->name()).arg(Victim->name()) );
		for ( cUOSocket *mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next())
		{
			if( mSock != socket && inrange1p( attacker, mSock->player() ) ) 
				mSock->showSpeech(Victim, temp);
		}
	}
	else
	{
		Skills->PlayInstrumentPoor(socket, inst);
		socket->sysMessage( tr("You play rather poorly and to no effect.") );
	}
	return true;
}

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
	else if( pc->getBackpack() && !pc->getBackpack()->contains(pi) )
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
		RegionIterator4Items ri( pc->pos(), 2 );
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

	short smithing = pc->skillValue( BLACKSMITHING );
	if		((smithing>=900)) dmg=1;
	else if ((smithing>=700)) dmg=2;
	else if ((smithing>=500)) dmg=3;
	hasSuccess = pc->checkSkill(BLACKSMITHING, 0, 1000);

	if( hasSuccess )
	{
		pi->setMaxhp( pi->maxhp() - dmg );
		pi->setHp( pi->maxhp() );
		socket->sysMessage( tr("* the item has been repaired.*") );
		if( makesection_ && makesection_->baseAction() )
			pc->soundEffect( makesection_->baseAction()->succSound() );
	}
	else
	{
		pi->setHp( pi->hp() - 2 );
		pi->setMaxhp( pi->maxhp() - 1 );
		socket->sysMessage( tr("* You fail to repair the item. *") );
		socket->sysMessage( tr("* You weaken the item.*") );
		if( makesection_ && makesection_->baseAction() )
			pc->soundEffect( makesection_->baseAction()->failSound() );
	}

	return true;
}

bool cSetTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	P_CHAR pChar = FindCharBySerial( target->serial() );
	P_ITEM pItem = FindItemBySerial( target->serial() );
	
	cUObject *pObject = NULL;
	
	if( pItem )
		pObject = pItem;
	else if( pChar )
		pObject = pChar;
	
	// Only characters and items
	if( !pObject )
	{
		socket->sysMessage( tr( "Please select a valid character or item" ) );
		return true;
	}

	cVariant value( this->value );
	stError *error = pObject->setProperty( key, value );

 	if( error )
	{
		socket->sysMessage( error->text );
		delete error;
	}
		
	if( pChar )
		pChar->resend();
	else if( pItem )
		pItem->update();

	return true;
}

bool cRemoveTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	P_CHAR pChar = FindCharBySerial( target->serial() );
	P_ITEM pItem = FindItemBySerial( target->serial() );
	
	if( pChar )
	{
		if( pChar->objectType() == enPlayer )
		{
			P_PLAYER pp = dynamic_cast<P_PLAYER>(pChar);
			if( pp->socket() )
			{
				pp->socket()->sysMessage( tr("You cannot delete logged in characters") );
				return true;
			}

			if( pp->account() )
				pp->account()->removeCharacter( pp );

			cCharStuff::DeleteChar( pChar );
		}
		else
			cCharStuff::DeleteChar( pChar );
	}
	else if( pItem )
	{
		Items->DeleItem( pItem );
	}
	else
		socket->sysMessage( "You need to select either an item or a character" );
	return true;
}

bool cAddEventTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	cUObject *pObject = 0;
	
	if( isCharSerial( target->serial() ) )
		pObject = FindCharBySerial( target->serial() );
	else if( isItemSerial( target->serial() ) )
		pObject = FindItemBySerial( target->serial() );
	
	// We have to have a valid target
	if( !pObject )
	{
		socket->sysMessage( tr( "You have to target a character or an item." ) );
		return true;
	}
	
	// Check if we already have the event
	if( pObject->hasEvent( _event ) )
	{
		socket->sysMessage( tr( "This object already has the event '%1'" ).arg( _event ) );
		return true;
	}
	
	cPythonScript *script = ScriptManager->find( _event );
	
	if( !script )
	{
		socket->sysMessage( tr( "Invalid event: '%1'" ).arg( _event ) );
		return true;
	}
	
	pObject->addEvent( script );
	return true;
}

bool cTileTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	if( !socket->player() )
		return true;
	
	// Set the first corner and readd ourself
	if( x1 == -1 || y1 == -1 )
	{
		x1 = target->x();
		y1 = target->y();
		socket->sysMessage( tr( "Please select the second corner." ) );
		return false;
	}
	else
	{
		INT16 x2;
		if( target->x() < x1 )
		{
			x2 = x1;
			x1 = target->x();
		}
		else
			x2 = target->x();
		
		INT16 y2;
		if( target->y() < y1 )
		{
			y2 = y1;
			y1 = target->y();
		}
		else
			y2 = target->y();
		
		socket->sysMessage( tr( "Tiling from %1,%2 to %3,%4" ).arg( x1 ).arg( y1 ).arg( x2 ).arg( y2 ) );
		UINT16 dCount = 0;
		
		for( UINT16 x = x1; x <= x2; ++x )
			for( UINT16 y = y1; y <= y2; ++y )
			{
				// Select a Random Tile from the list
				QString id = ids[ RandomNum( 0, ids.count()-1 ) ];
				P_ITEM pItem = Items->createScriptItem( id );
				
				if( pItem )
				{
					Coord_cl position( x, y, z, socket->player()->pos().map );
					pItem->setPos( position );
					MapObjects::instance()->add( pItem );
					pItem->update();
					++dCount;
				}
			}
			
			socket->sysMessage( tr( "Created %1 items." ).arg( dCount ) );
			return true;
	}		
}

bool cSetMultiOwnerTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	cMulti* pMulti = dynamic_cast< cMulti* >( FindItemBySerial( multi_ ) );
	if( !pMulti || !socket->player() )
	{
		socket->sysMessage( tr( "An error occured! Send a bug report to the staff, please!" ) );
		return true;
	}
	
	if( target->x() == 0xFFFF && target->y() == 0xFFFF && target->z() == (INT8)0xFF )
		return true;
	
	P_PLAYER pc = dynamic_cast<P_PLAYER>(FindCharBySerial( target->serial() ));
	if( !pc )
	{
		socket->sysMessage( tr( "This is not a valid target!" ) );
		return false;
	}
	
	if( coowner_ )
	{
		pMulti->setCoOwner( pc );
		socket->sysMessage( tr("You have made %1 to the new co-owner of %2").arg( pc->name() ).arg( pMulti->name() ) );
		pc->socket()->sysMessage( tr("%1 has made you to the new co-owner of %2").arg( socket->player()->name() ).arg( pMulti->name() ) );
	}
	else
	{
		pMulti->setOwner( pc );
		socket->sysMessage( tr("You have made %1 to the new owner of %2").arg( pc->name() ).arg( pMulti->name() ) );
		pc->socket()->sysMessage( tr("%1 has made you to the new owner of %2").arg( socket->player()->name() ).arg( pMulti->name() ) );
	}
	return true;
}

bool cMultiAddToListTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	cMulti* pMulti = dynamic_cast< cMulti* >( FindItemBySerial( multi_ ) );
	if( !pMulti || !socket->player() )
	{
		socket->sysMessage( tr( "An error occured! Send a bug report to the staff, please!" ) );
		return true;
	}
	
	if( target->x() == 0xFFFF && target->y() == 0xFFFF && target->z() == (INT8)0xFF )
		return true;
	
	P_PLAYER pc = dynamic_cast<P_PLAYER>(FindCharBySerial( target->serial() ));
	if( !pc )
	{
		socket->sysMessage( tr( "This is not a valid target!" ) );
		return false;
	}
	
	if( banlist_ )
	{
		pMulti->addBan( pc );
	}
	else
	{
		pMulti->addFriend( pc );
	}
	socket->sysMessage( tr("Select another char to add to the %1!").arg( banlist_ ? tr("list of banned") : tr("list of friends") ) );
	return false;
}

bool cMultiChangeLockTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	cMulti* pMulti = dynamic_cast< cMulti* >( FindItemBySerial( multi_ ) );
	if( !pMulti || !socket->player() )
	{
		socket->sysMessage( tr( "An error occured! Send a bug report to the staff, please!" ) );
		return true;
	}
	
	if( target->x() == 0xFFFF && target->y() == 0xFFFF && target->z() == (INT8)0xFF )
		return true;
	
	P_ITEM pi = FindItemBySerial( target->serial() );
	if( !pi )
	{
		socket->sysMessage( tr( "This is not a valid target!" ) );
		return false;
	}
	
	if( pi->multis() == pMulti->serial() && pi->type() != 117 )
	{
		pi->setMagic((pi->magic() == 4 && pi->type() != 222) ? 0 : 4);
		pi->update();
	}
	socket->sysMessage( tr("Select another item to lock/unlock!") );
	return false;
}

bool cDyeTubDyeTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	if( !socket->player() )
		return true;
	
	P_ITEM pItem = FindItemBySerial( target->serial() );
	
	if( !pItem )
		return true;
	
	if( pItem->getOutmostChar() != socket->player() && !( pItem->isInWorld() && pItem->inRange( socket->player(), 4 ) ) )
	{
		socket->sysMessage( tr( "You can't reach this object to dye it." ) );
		return true;
	}
	
	if( pItem->type() != 406 )
	{
		socket->sysMessage( tr( "You can only dye dye tubs with this." ) );
		return true;
	}
	
	pItem->setColor( _color );
	pItem->update();
	
	return true;
}

bool cShowTarget::responsed( cUOSocket *socket, cUORxTarget *target )
{
	// Check for a valid target
	cUObject *pObject;
	P_ITEM pItem = FindItemBySerial( target->serial() );
	P_CHAR pChar = FindCharBySerial( target->serial() );
	if( !pChar && !pItem )
	{
		socket->sysMessage( tr( "You have to target a valid object." ) );
		return true;
	}

	if( pChar )
		pObject = pChar;
	else
		pObject = pItem;


	cVariant result;
	stError *error = pObject->getProperty( key, result );

	if( error )
	{
		socket->sysMessage( error->text );
		delete error;
		return true;
	}

	socket->sysMessage( tr( "'%1' is '%2'" ).arg( key ).arg( result.toString() ) );
	return true;
}
