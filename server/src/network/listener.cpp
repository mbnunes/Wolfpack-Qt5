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

/*****************************************************************************
  QCString member functions
 *****************************************************************************/

/*!
  \class cListener listener.h
  \ingroup network
  \ingroup tools
  \ingroup threaded

  \brief The cListener class provides an abstraction of the Acceptor
  Pattern.


  cListener inherits ZThread::Thread, which is defined as in Zthread Library.

  You might use cListener for waitting new connections without pooling.

*/

/*!
  \fn cListener::cListener( Q_UINT16 port )
  Constructs a listener binded to \a port.
*/

cListener::cListener( Q_UINT16 port )
{
	listenningSocket.setBlocking( false ); // or else it would take a while to join()
	listenningSocket.bind( static_cast<Q_UINT32>(0), port );
	listenningSocket.listen( 20 );
}

cListener::~cListener()
{
	listenningSocket.close();
}

void cListener::run() throw()
{
	while ( !canceled() )
	{
		int fd = listenningSocket.accept();
		if ( fd != -1 )
		{
			QSocketDevice* socket = new QSocketDevice(fd, QSocketDevice::Stream);
			socket->setBlocking( false );
			readyConnections.add( socket );
		}
		else 
			sleep(2000); // if nothing interesting happen take a nap
	}
}

/*!
  Retrieves stabilished connections from queue.
*/
QSocketDevice* cListener::getNewConnection()
{
	return readyConnections.next();
}

/*!
  Queries if there are any stabilished connections for retrieve
  \sa getNewConnection().
*/
bool cListener::haveNewConnection()
{
	return !readyConnections.empty();
}

