//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
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
#include "skills.h"
#include "TmpEff.h"
#include "items.h"
#include "basics.h"
#include "inlines.h"
#include "network/uorxpackets.h"
#include "network/uosocket.h"
#include "world.h"
#include "basechar.h"
#include "player.h"

bool cSkLockpicking::responsed( cUOSocket *socket, cUORxTarget *target )
{
/* 	const P_ITEM pi = FindItemBySerial( target->serial() );
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
						piPick->reduceAmount( 1 );
					} 
					else
						socket->sysMessage( tr("You fail to open the lock.") );
				}
			} else
			{
				socket->sysMessage( tr("That cannot be unlocked without a key.") );
			}
		}
	}*/
	return true;
}
