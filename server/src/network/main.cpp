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



#include "listener.h"
#include "asyncnetio.h"
#include "uopacket.h"
#include "uosocket.h"

// Library Includes
#include "zthread/exceptions.h"

#include <conio.h>
#include <iostream>
#include <vector>
#include <stdlib.h>

// Namespaces
using namespace std;
using namespace ZThread;

cAsyncNetIO* netio = 0;
vector< cUOSocket* > uoSockets;

int main( int argc, char** argv )
{
	cListener* listener = new cListener(2593);
	netio = new cAsyncNetIO;

	listener->start();
	netio->start();

	try
	{
		while ( true )
		{
			if ( listener->haveNewConnection() )
			{
				QSocketDevice *socket = listener->getNewConnection(); 
				netio->registerSocket( socket );
				uoSockets.push_back( new cUOSocket(socket) );
				cout << QString( "Socket connected [%1]\n" ).arg( socket->peerAddress().toString() ).latin1();
			}

			// Process waiting packets
			vector< cUOSocket* >::iterator uoIterator;

			for( uoIterator = uoSockets.begin(); uoIterator != uoSockets.end(); ++uoIterator )
			{
				cUOSocket *uoSocket = (*uoIterator);

				if( uoSocket->socket()->status() != IO_Ok )
				{
					cout << QString( "Socket disconnected [%1]\n" ).arg( uoSocket->socket()->peerAddress().toString() ).latin1();
					netio->unregisterSocket( uoSocket->socket() );
					uoSockets.erase( uoIterator );
				}
				else
				{
					uoSocket->recieve();
				}
			}

			_sleep( 40 );
		}
	}
	catch( Synchronization_Exception& e )
	{
		cerr << e.what() << endl;
		netio->cancel();
		listener->cancel();
	}
	catch ( ... )
	{
		cerr << "Unknown exception" << endl;
		terminate();
	}

	return 0;
}
