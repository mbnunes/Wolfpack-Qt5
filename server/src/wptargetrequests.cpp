
#include "wptargetrequests.h"

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
