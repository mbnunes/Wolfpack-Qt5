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

#include "gumpsmgr.h"
#include "gumps.h"

#include "basics.h"
#include "network/uopacket.h"
#include "network/uosocket.h"
#include "srvparams.h"

#include <algorithm>

using namespace std;

#include "debug.h"

#undef  DBGFILE
#define DBGFILE "gumpsmgr.cpp"


cGumpsManager::~cGumpsManager()
{
	iterator it = begin();
	for (; it != end(); ++it )
		delete it->second;
}


/*!
 * Registers an Gump into the GumpsManager Instance and enable lookups by serial.
 *
 * @param pc : Pointer to Gump
 *
 * @return void  : none
 */
void cGumpsManager::registerGump(cGump* pi) throw(wp_exceptions::wpbad_ptr)
{
	if ( pi != NULL)
	{
		insert(make_pair(pi->serial(), pi));
		lastUsedSerial = QMAX(lastUsedSerial, pi->serial());
		pi->setTimeOut( uiCurrentTime + SrvParams->gumpTimeOut() * MY_CLOCKS_PER_SEC );
	}
	else
	{
		throw wp_exceptions::wpbad_ptr("Invalid argument PI at cGumpsManager::registerGump");
	}
}

/*!
 * Unregister from GumpsManager. Searches by serial will not find this Gump anymore.
 *
 * @param pc : Pointer to Gump. Gump itself is unchanged.
 *
 * @return void  : none
 */
void cGumpsManager::unregisterGump(cGump* pi) throw(wp_exceptions::wpbad_ptr)
{
	if ( pi != NULL)
		erase(pi->serial());
	else
		throw wp_exceptions::wpbad_ptr("Invalid argument PI at cGumpsManager::unregisterGump");
}

/*!
 * Returns an unused, valid Serial number for Gumps
 *
 * @param none
 *
 * @return SERIAL  : Valid serial number
 */
SERIAL cGumpsManager::getUnusedSerial() const
{
//	typedef maxKeyPred<SERIAL, cGump*> max_serialPred;
//	map<SERIAL, cGump*>::const_iterator temp = std::max_element(this->begin(), this->end(), max_serialPred());
	return QMAX(0x00000001, lastUsedSerial + 1);
}

void cGumpsManager::attachGump( cUOSocket *socket, cGump *gump )
{
	gump->setSerial( this->getUnusedSerial() );
	registerGump( gump );

	QString layout = gump->layout().join( "" );
	Q_UINT32 gumpsize = 21 + layout.length() + 2;
	QStringList text = gump->text();
	QStringList::const_iterator it = text.begin();
	while( it != text.end() )
	{
		gumpsize += (*it).length() * 2 + 2;
		it++;
	}
	cUOTxGumpDialog uoPacket( gumpsize );

	uoPacket.setSerial( gump->serial() );
	uoPacket.setType( gump->type() );
	uoPacket.setX( gump->x() );
	uoPacket.setY( gump->y() );
	uoPacket.setContent( layout, text );

	socket->send( &uoPacket );
	//uoPacket.print( &cout ); // for debugging
}

void cGumpsManager::handleResponse( cUOSocket* socket, SERIAL serial, UINT32 type, gumpChoice_st choice ) throw(wp_exceptions::wpbad_ptr)
{
	std::map<SERIAL, cGump*>::iterator iter = this->find( serial );
	if ( iter != this->end() )
	{
		cGump* gump = iter->second;
/*		switch( type )    <--- not needed yet...
		{
		case 1:
			// normal cGump
			break;
		case 2:
			gump = dynamic_cast< cPythonGump* >(gump);
			break;
		case 3:
			gump = dynamic_cast< cSpawnRegionInfoGump* >(gump);
			break;
		default:
			break;
		}*/

		gump->handleResponse( socket, choice );

		deletedGumps.push_back(gump);
		unregisterGump(gump);
	}
}

/*!
 * Free memory of Gumps queued for delete.
 *
 * @param none
 *
 * @return void  : none
 */
void cGumpsManager::purge()
{
	std::map<SERIAL, cGump*>::iterator iter = this->begin();
	while( iter != this->end() )
	{
		if( iter->second )
		{
			if( iter->second->timeout() <= uiCurrentTime )
				deletedGumps.push_back( iter->second );
			unregisterGump( iter->second );
		}
		iter++;
	}

	list<cGump*>::iterator it;
	for (it = deletedGumps.begin(); it != deletedGumps.end(); ++it)
	{
		delete *it;
	}
	deletedGumps.clear();
}

