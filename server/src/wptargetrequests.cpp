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

#include "wptargetrequests.h"
#include "network.h"

map< UOXSOCKET, cTargetRequest* > targetRequests; // our global target-container

// This is called by the mainloop
// to garbage collect timed out target-requests
void checkTimedOutTargets( void )
{
	map< UOXSOCKET, cTargetRequest* >::iterator trIterator;

	for( trIterator = targetRequests.begin(); trIterator != targetRequests.end(); ++trIterator )
	{
		cTargetRequest *request = trIterator->second;

		if( request->timeout() == 0 )
			continue;

		if( request->timeout() <= uiCurrentTime )
		{
			trIterator->second->timedout( trIterator->first );
			delete trIterator->second;
			targetRequests.erase( trIterator );
			trIterator = targetRequests.begin();
		}
	}
}

// Attach a targetRequest to a specific socket
void attachTargetRequest( UOXSOCKET socket, cTargetRequest *targetRequest )
{
	// Issue the timeout for the old socket
	if( targetRequests.find( socket ) != targetRequests.end() )
	{
		targetRequests[ socket ]->timedout( socket );

		delete targetRequests[ socket ];
		targetRequests.erase( targetRequests.find( socket ) );
	}

	targetRequests[ socket ] = targetRequest;

	// Send the target-packet to the socket
	//target( socket, 0, 0, 0, 1, "" );

	UI08 *byteArray = new UI08[ 19 ];
	memset( byteArray, 0, 19 ); // Fill it with zeros
	
	byteArray[ 0 ] = 0x6C; // Packet ID
	byteArray[ 1 ] = 1;	// We want to allow map-only targets as well
	byteArray[ 5 ] = 1; // Our target-id

	Xsend( socket, byteArray, 19 );	

	// Allow a target for the socket (useles...)
	targetok[ socket ] = 1;
}
