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
#include "targeting.h"
#include "guildstones.h"
#include "mapobjects.h"
#include "srvparams.h"
#include "wpdefmanager.h"
#include "tilecache.h"
#include "wpscriptmanager.h"
#include "targetrequests.h"
#include "maps.h"
#include "classes.h"
#include "network/uosocket.h"
#include "gumps.h"
#include "network.h"
#include "skills.h"
#include "multis.h"
#include "weight.h"
#include "house.h"

#undef DBGFILE
#define DBGFILE "targeting.cpp"

static void KeyTarget(int s, P_ITEM pi) // new keytarget by Morollan
{
	/*if (pi)
	{
		if ((pi->more1==0)&&(pi->more2==0)&&
			(pi->more3==0)&&(pi->more4==0))
		{
			if ( pi->type() == 7 && (iteminrange(s,pi,2) || (!pi->isInWorld()) ) )
			{
				if (!Skills->CheckSkill(currchar[s], TINKERING, 400, 1000))
				{
					sysmessage(s,"You fail and destroy the key blank.");
					Items->DeleItem(pi);
				}
				else
				{
					pi->more1=addid1[s];
					pi->more2=addid2[s];
					pi->more3=addid3[s];
					pi->more4=addid4[s];
					sysmessage(s, "You copy the key."); //Morrolan can copy keys
				}
			}
			return;
		}//if
		else if (((pi->more1==addid1[s])&&(pi->more2==addid2[s])&&
			(pi->more3==addid3[s])&&(pi->more4==addid4[s]))||
			(addid1[s]==(unsigned char)'\xFF'))
		{
			if (((pi->type()==1)||(pi->type()==63))&&(iteminrange(s,pi,2)))
			{
				if(pi->type()==1) pi->setType( 8 );
				if(pi->type()==63) pi->setType( 64 );
				sysmessage(s, "You lock the container.");
				return;
			}
			else if ((pi->type()==7)&&(iteminrange(s,pi,2)))
			{
				currchar[s]->inputitem = pi->serial;
				currchar[s]->inputmode = cChar::enDescription;
				sysmessage(s,"Enter new name for key.");//morrolan rename keys
				return;
			}
			else if ((pi->type()==8)||(pi->type()==64)&&(iteminrange(s,pi,2)))
			{
				if(pi->type()==8) pi->setType( 1 );
				if(pi->type()==64) pi->setType( 63 );
				sysmessage(s, "You unlock the container.");
				return;
			}
			else if ((pi->type()==12)&&(iteminrange(s,pi,2)))
			{
				pi->setType( 13 );
				sysmessage(s, "You lock the door.");
				return;
			}
			else if ((pi->type()==13)&&(iteminrange(s,pi,2)))
			{
				pi->setType( 12 );
				sysmessage(s, "You unlock the door.");
				return;
			}
			else if (pi->id()==0x0BD2)
			{
				sysmessage(s, "What do you wish the sign to say?");
				currchar[s]->inputitem = pi->serial; //Morrolan sign kludge
				currchar[s]->inputmode=cChar::enHouseSign;
				return;
			}

			//Boats ->
			else if( pi->type() == 117 && ( pi->type2() == 3 || pi->type2() == 2 ) )
			{
				cBoat* pBoat = dynamic_cast< cBoat* >(FindItemBySerial(pi->tags.get("boatserial").toUInt()));
				if( pBoat != NULL )
				{
					pBoat->switchPlankState( pi );
					pi->update();
				}
			}
			//End Boats --^
		}//else if
		else
		{
			if (pi->type()==7) sysmessage (s, "That key is not blank!");
			else if (pi->more1=='\x00') sysmessage(s, "That does not have a lock.");
			else sysmessage(s, "The key does not fit into that lock.");
			return;
		}//else
		return;
	}//if*/
}

////////////////
// name:		NpcResurrectTarget
// history:		by UnKnown (Touched tabstops by Tauriel Dec 28, 1998)
// Purpose:		Resurrects a character
//
bool cTargets::NpcResurrectTarget(P_CHAR pc)
{
	pc->resurrect();
	return true;
}

void cTargets::HouseOwnerTarget(int s) // crackerjack 8/10/99 - change house owner
{
/*	int os, i;
	int o_serial = LongFromCharPtr(buffer[s]+7);
	if(o_serial==-1) return;
	P_CHAR pc = FindCharBySerial(o_serial);
	if ( pc == NULL ) return;

	SERIAL serial = calcserial(addid1[s],addid2[s],addid3[s],addid4[s]);
	P_ITEM pSign = FindItemBySerial(serial);
	if ( pSign == NULL )
		return;

	serial = calcserial(pSign->more1(), pSign->more2(), pSign->more3(), pSign->more4());
	P_ITEM pHouse = FindItemBySerial(serial);
	if ( pHouse == NULL )
		return;
	
	if(pc->serial == currchar[s]->serial)
	{
		sysmessage(s, "you already own this house!");
		return;
	}
	
	pSign->SetOwnSerial(o_serial);
	
	pHouse->SetOwnSerial(o_serial);
	
	dynamic_cast< cHouse* >(pHouse)->removeKeys();
	
	os=-1;
	for(i=0;i<now && os==-1;i++)
	{
		if( currchar[i]->serial == pc->serial && perm[i]) os=i;
	}
	
	P_ITEM pi3=NULL;
	if(os!=-1)
	{
		pi3 = Items->SpawnItem(os, pc, 1, "a house key", 0, 0x10, 0x0F, 0, 1,1);//gold key for everything else
		if(pi3 == NULL) return;
	}
	else
	{
		pi3 = Items->SpawnItem(pc, 1, "a house key", 0, 0x100F,0,0);//gold key for everything else
		if(!pi3) return;
		pi3->moveTo(pc->pos);
		pi3->update();
	}
	pi3->setMore1( static_cast<unsigned char>((pHouse->serial&0xFF000000)>>24) );
	pi3->setMore2( static_cast<unsigned char>((pHouse->serial&0x00FF0000)>>16) );
	pi3->setMore3( static_cast<unsigned char>((pHouse->serial&0x0000FF00)>>8) );
	pi3->setMore4( static_cast<unsigned char>((pHouse->serial&0x000000FF)) );
	pi3->setType( 7 );
	
	sysmessage(s, "You have transferred your house to %s.", pc->name.latin1());
	sprintf((char*)temp, "%s has transferred a house to %s.", currchar[s]->name.latin1(), pc->name.latin1());

	int k;
	for(k=0;k<now;k++)
		if(k!=s && ( (perm[k] && inrange1p(currchar[k], currchar[s]) )||
			(currchar[k]->serial==o_serial)))
			sysmessage(k, (char*)temp);*/
}

void cTargets::HouseEjectTarget(int s) // crackerjack 8/11/99 - kick someone out of house
{
/*	P_CHAR pc = FindCharBySerPtr(buffer[s]+7);
	if (!pc)
		return;
	int serial=calcserial(addid1[s],addid2[s],addid3[s],addid4[s]);
	if(serial == INVALID_SERIAL) return;
	P_ITEM pHouse = FindItemBySerial(serial);
	if(pHouse != NULL)
	{
		int sx, sy, ex, ey;
		Map->MultiArea(pHouse, &sx,&sy,&ex,&ey);
		if(pc->serial == currchar[s]->serial)
		{
			sysmessage(s,"Do you really want to do that?!");
			return;
		}
		if(pc->pos.x>=sx&&pc->pos.y>=sy&&pc->pos.x<=ex&&pc->pos.y<=ey)
		{
			Coord_cl pos(pc->pos);
			pos.x = ex;
			pos.y = ey;
			pc->moveTo( pos );
			teleport(pc);
			sysmessage(s, "Player ejected.");
		}
		else
			sysmessage(s, "That is not inside the house.");
	}
*/
}

void cTargets::HouseBanTarget(int s) 
{
/*	Targ->HouseEjectTarget(s);	// first, eject the player

	P_CHAR pc = FindCharBySerPtr(buffer[s]+7);
	P_CHAR pc_home = currchar[s];
	if (!pc)
		return;
	cHouse* pHouse = dynamic_cast< cHouse* >( cMulti::findMulti( pc_home->pos ) );
	if( pHouse )
	{
		if (pc->serial == pc_home->serial) return;
		pHouse->addBan(pc);
		sysmessage(s, "%s has been banned from this house.", pc->name.latin1());
	}*/
}

void cTargets::HouseFriendTarget(int s) // crackerjack 8/12/99 - add somebody to friends list
{
/*	P_CHAR Friend = FindCharBySerPtr(buffer[s]+7);
	P_CHAR pc_home = currchar[s];

	cHouse* pHouse = dynamic_cast< cHouse* >( cMulti::findMulti( pc_home->pos ) );

	if(Friend && pHouse)
	{
		if(Friend->serial == pc_home->serial)
		{
			sysmessage(s,"You are already the owner!");
			return;
		}
		pHouse->addFriend(Friend);
		sysmessage(s, "%s has been made a Friend of the house.", Friend->name.latin1());
	}*/
}

void cTargets::HouseUnBanTarget(int s)
{
/*	P_CHAR pc_banned = FindCharBySerPtr(buffer[s]+7);
	P_CHAR pc_owner  = currchar[s];
	
	cHouse* pHouse = dynamic_cast< cHouse* >( cMulti::findMulti( pc_owner->pos ) );

	if(pc_banned && pHouse)
	{
		if(pc_banned->serial==pc_owner->serial)
		{
			sysmessage(s,"You are the owner of this home!");
			return;
		}
		pHouse->removeBan(pc_banned);
		sysmessage(s,"%s has been UnBanned!",pc_banned->name.latin1());
	}
	return;*/
}

void cTargets::HouseUnFriendTarget(int s)
{
/*	P_CHAR pc_friend = FindCharBySerPtr(buffer[s]+7);
	P_CHAR pc_owner  = currchar[s];
	
	cHouse* pHouse = dynamic_cast< cHouse* >( cMulti::findMulti( pc_owner->pos ) );

	if(pc_friend && pHouse)
	{
		if(pc_friend->serial==pc_owner->serial)
		{
			sysmessage(s,"You are the owner of this home!");
			return;
		}
		pHouse->removeFriend(pc_friend);
		sysmessage(s,"%s is no longer a Friend of this home!", pc_friend->name.latin1());
	}
	return;*/
}

void cTargets::HouseLockdown( UOXSOCKET s ) // Abaddon
// PRE:		S is the socket of a valid owner/coowner and is in a valid house
// POST:	either locks down the item, or puts a message to the owner saying he's a moron
// CODER:	Abaddon
// DATE:	17th December, 1999
{
/*	int ser = LongFromCharPtr(buffer[s]+7);
	P_ITEM pi = FindItemBySerial(ser);

	if( pi != NULL )
	{
		short id = pi->id();

		// not needed anymore, cause called from house_sped that already checks that ...

		// time to lock it down!

		if (Items->isFieldSpellItem ( pi ))
		{
			sysmessage(s,"you cannot lock this down!");
			return;
		}
		if( pi->type() == 12 || pi->type() == 13 || pi->type() == 203 )
		{
			sysmessage(s, "You cant lockdown doors or signs!");
			return;
		}
		if( IsAnvil( id ) )
		{
			sysmessage(s, "You cant lockdown anvils!");
			return;
		}
		if( IsForge( id ) )
		{
			sysmessage(s, "You cant lockdown forges!");
			return;
		}

		cMulti* pi_multi = cMulti::findMulti( pi->pos );
		if( pi_multi )
		{
			if(pi->isLockedDown())
			{
				sysmessage(s,"That item is already locked down, release it first!");
				return;
			}
			pi->setLockedDown();	// LOCKED DOWN!
			pi->setOwnSerialOnly(currchar[s]->serial);
			pi->update();
			return;
		}
		else
		{
			// not in a multi!
			sysmessage( s, "That item is not in your house!" );
			return;
		}
	}
	else
	{
		sysmessage( s, "Invalid item!" );
		return;
	}*/
}

void cTargets::HouseSecureDown( UOXSOCKET s ) // Ripper
// For locked down and secure chests
{
/*	int ser = LongFromCharPtr(buffer[s]+7);
	P_ITEM pi = FindItemBySerial(ser);
	if( pi != NULL )
	{
		// time to lock it down!
		if (Items->isFieldSpellItem ( pi ) )
		{
			sysmessage(s,"you cannot lock this down!");
			return;
		}
		if( pi->type() == 12 || pi->type() == 13 || pi->type() == 203 )
		{
			sysmessage(s, "You cant lockdown doors or signs!");
			return;
		}
		if( pi->isLockedDown() )
		{
			sysmessage(s,"That item is already locked down, release it first!");
			return;
		}

		cMulti* pi_multi = cMulti::findMulti( pi->pos );
		if( pi_multi && pi->type() == 1 )
		{
		    pi->setLockedDown();	// LOCKED DOWN!
			pi->setSecured( true );
			pi->setOwnSerialOnly(currchar[s]->serial);
			pi->update();
			return;
		}
		if( pi->type() != 1 )
		{
			sysmessage(s,"You can only secure chests!");
			return;
		}
		else
		{
			// not in a multi!
			sysmessage( s, "That item is not in your house!" );
			return;
		}
	}
	else
	{
		sysmessage( s, "Invalid item!" );
		return;
	}*/
}

void cTargets::HouseRelease( UOXSOCKET s ) // Abaddon & Ripper
// PRE:		S is the socket of a valid owner/coowner and is in a valid house, the item is locked down
// POST:	either releases the item from lockdown, or puts a message to the owner saying he's a moron
// CODER:	Abaddon
// DATE:	17th December, 1999
// update: 5-8-00
{
/*	int ser = LongFromCharPtr(buffer[s]+7);
	P_CHAR pc_currchar = currchar[s];
	P_ITEM pi = FindItemBySerial(ser);
	if( pi != NULL )
	{
		if (pi->secured() && !pc_currchar->Owns(pi))
		{
			sysmessage(s,"You cannot do that!");
			return;
		}
		if (Items->isFieldSpellItem(pi))
		{
			sysmessage(s,"you cannot release this!");
			return;
		}
		if( pi->type() == 12 || pi->type() == 13 || pi->type() == 203 )
		{
			sysmessage(s, "You cant release doors or signs!");
			return;
		}

		// time to unlock it!
		cMulti* pi_multi = cMulti::findMulti( pi->pos );
		if( pi_multi && pi->isLockedDown() || pi->type() == 1 )
		{
			pi->setAllMovable();	// Default as stored by the client, perhaps we should keep a backup?
			pi->setSecured( false );
			pi->update();
			return;
		}
		else if( pi_multi == NULL )
		{
			// not in a multi!
			sysmessage( s, "That item is not in your house!" );
			return;
		}
	}
	else
	{
		sysmessage( s, "Invalid item!" );
		return;
	}*/
}
