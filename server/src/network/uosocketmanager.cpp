/*
=================================================================
|   )      (\_     | WOLFPACK ULTIMA ONLINE EMULATOR            |
|  ((    _/{  "-;  | Created by: DarkStorm                      |
|   )).-' {{ ;'`   | Revised by:                                |
|  ( (  ;._ \\ ctr | Last Modification: Created                 |
=================================================================
*/

#include "uosocketmanager.h"

void cUOSocketManager::insert( cUOSocket* socket )
{
	sockets_.push_back( socket );
}

void cUOSocketManager::remove( cUOSocket* socket )
{
	vector< cUOSocket* >::iterator myIter;

	for( myIter = sockets_.begin(); myIter != sockets_.end(); ++myIter )
	{
		if( (*myIter) == socket )
		{
			sockets_.erase( myIter );
			return;
		}
	}
}

