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
#include <iostream.h>

#include <stdlib.h>

// Namespaces
using namespace std;
using namespace ZThread;

QSocketDevice* socket = 0;
cAsyncNetIO* netio = 0;

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
				socket = listener->getNewConnection(); 
				netio->registerSocket( socket );
				cout << "Socket connected" << endl;
			}
			if ( socket )
			{
				cUOPacket* packet = netio->recvPacket( socket );
				if ( packet )
				{
					packet->print( &cout );
					delete packet;
				}
				if ( !socket->isOpen() )
				{
					cout << "Socket disconnected" << endl;
					netio->unregisterSocket( socket );
					delete socket;
					socket = 0;
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
