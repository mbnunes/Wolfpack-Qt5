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

#include "listener.h"

/*****************************************************************************
  cListener member functions
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
	_port = port;
}

cListener::~cListener() throw()
{
}

void cListener::run() throw()
{
	listenningSocket.bind( static_cast<Q_UINT32>(0), _port );
	listenningSocket.listen( 20 );
	listenningSocket.setBlocking( false ); // or else it would take a while to join()

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
		{ 
			try
			{
				sleep(2000); // if nothing interesting happen take a nap
			}
			catch( ZThread::Interrupted_Exception& e )
			{ // Looks like we are about to exit from this thread
			}
		}
	}

	listenningSocket.close();
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

