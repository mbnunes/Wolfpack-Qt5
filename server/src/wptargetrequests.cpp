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
void attachTargetRequest( UOXSOCKET socket, cTargetRequest *targetRequest, bool allowMapTarget )
{
	// Issue the timeout for the old socket
	map< UOXSOCKET, cTargetRequest* >::iterator it = targetRequests.find( socket );
	if( it != targetRequests.end() )
	{
		(*it).second->timedout( socket );

		delete (*it).second;
		targetRequests.erase( it );
	}

	targetRequests.insert( make_pair( socket, targetRequest ) );

	// Send the target-packet to the socket
	QByteArray byteArray;
	byteArray.resize( 19 );
	byteArray.fill( (char)0 );
	
	byteArray[ 0 ] = 0x6C; // Packet ID
	byteArray[ 1 ] = allowMapTarget ? true : false;	// We want to allow map-only targets as well

	UI32 targetId = RandomNum( 0xFE000000, 0xFEFFFFFF );
	LongToCharPtr( targetId, (UI08*)&byteArray.data()[2] );
	targetRequest->setTargetId( targetId );

	Xsend( socket, byteArray, 19 );	

	// Allow a target for the socket (quite useless...)
	targetok[ socket ] = 1;
}

void attachPlaceRequest( UOXSOCKET socket, cTargetRequest *targetRequest, UI16 houseId )
{
	// Issue the timeout for the old socket
	map< UOXSOCKET, cTargetRequest* >::iterator it = targetRequests.find( socket );
	if( it != targetRequests.end() )
	{
		(*it).second->timedout( socket );

		delete (*it).second;
		targetRequests.erase( it );
	}

	targetRequests.insert( make_pair( socket, targetRequest ) );

	// Send the target-packet to the socket
	QByteArray byteArray;
	byteArray.resize( 26 );
	byteArray.fill( (char)0 );
	
	byteArray[ 0 ] = 0x99; // Packet ID
	byteArray[ 1 ] = 0x01; // From Server

	UI32 targetId = RandomNum( 0xFE000000, 0xFEFFFFFF );
	LongToCharPtr( targetId, (UI08*)&byteArray.data()[2] );
	targetRequest->setTargetId( targetId );

	// Byte 17: model Id - 0x4000
	ShortToCharPtr( houseId, (UI08*)&byteArray.data()[18] );

	Xsend( socket, byteArray, 26 );	

	// Allow a target for the socket (quite useless...)
	targetok[ socket ] = 1;
}
