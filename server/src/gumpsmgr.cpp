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

#include <algorithm>

using namespace std;

#include "debug.h"

#undef  DBGFILE
#define DBGFILE "gumpsmgr.cpp"

struct max_serialPred : binary_function<pair<SERIAL, cGump*>, pair<SERIAL, cGump*>, bool>
{
	bool operator()(pair<SERIAL,cGump*> a, pair<SERIAL,cGump*> b)
	{
		return a.first < b.first;
	}
};

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
	QByteArray packet( 21 + layout.length() + 2 );
	packet[ 0 ] = 0xB0;
	LongToCharPtr( gump->serial(), (Q_UINT8*)&packet.data()[3] );
	LongToCharPtr( gump->type(), (Q_UINT8*)&packet.data()[7] );
	LongToCharPtr( gump->x(), (Q_UINT8*)&packet.data()[11] );
	LongToCharPtr( gump->y(), (Q_UINT8*)&packet.data()[15] );
	ShortToCharPtr( layout.length(), (Q_UINT8*)&packet.data()[19] );

	memcpy( &packet.data()[21], layout.latin1(), layout.length() );

	// Send the unicode text-lines
	QStringList text_ = gump->text();
	ShortToCharPtr( text_.count(), (Q_UINT8*)&packet.data()[ 21 + layout.length() ] );

	for( Q_INT32 i = 0; i < text_.count(); ++i )
	{
		packet.resize( packet.count() + 2 + ( text_[ i ].length() * 2 ) );
	
		// Bytes are not swapped for network byteorder so lets shift them left by one byte and copy one byte less
		packet[ (int)(packet.count() - ( text_[ i ].length() * 2 ) ) ] = 0;
		memcpy( (Q_UINT8*)&packet.data()[ packet.count() - ( text_[ i ].length() * 2 ) + 1 ], text_[i].unicode(), (text_[i].length()*2) - 1 );
		ShortToCharPtr( text_[i].length(), (Q_UINT8*)&packet.data()[ packet.count() - ( text_[ i ].length() * 2 ) - 2 ]  );
	}
	
	// Calc the packet length
	ShortToCharPtr( packet.count(), (Q_UINT8*)&packet.data()[1] );
	cUOPacket uoPacket( packet );
	socket->send( &uoPacket );
}

void cGumpsManager::handleResponse( cUOSocket* socket, SERIAL serial, UINT32 type, UINT32 choice ) throw(wp_exceptions::wpbad_ptr)
{
	std::map<SERIAL, cGump*>::iterator iter = this->find( serial );
	if ( iter != this->end() )
	{
		cGump* gump = iter->second;
		switch( type )
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
		}

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
	list<cGump*>::iterator it;
	for (it = deletedGumps.begin(); it != deletedGumps.end(); ++it)
	{
		delete *it;
	}
	deletedGumps.clear();
}

