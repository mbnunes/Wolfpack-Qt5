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
#ifndef __WPTARGETREQUEST_H__
#define __WPTARGETREQUEST_H__

#include "platform.h"
#include "typedefs.h"
#include "targeting.h"

// System Include
#include <map>
//#include "wolfpack.h"

// Abstract base-class for target requests
class cTargetRequest
{
protected:
	UI32 timeout_; // Timeout in MS
	UI32 targetId_; // Target id so no overlapping targets are processed

public:
	cTargetRequest( void ) { timeout_ = 0; }; // Never times out
	virtual ~cTargetRequest( void ) { };

	virtual void responsed( UOXSOCKET socket, PKGx6C targetInfo ) {}; // Request has been answered
	virtual void timedout( UOXSOCKET socket ) {}; // Request is overwritten

	UI32 targetId( void ) { return targetId_; }
	void setTargetId( UI32 data ) { targetId_ = data; }

	UI32 timeout( void ) { return timeout_; }; // Get the timeout-value
	void setTimeout( UI32 data ) { timeout_ = data; }; // Set the timeout-value
};

// Several public functions
void checkTimedOutTargets( void );
void attachTargetRequest( UOXSOCKET socket, cTargetRequest *targetRequest, bool allowMapTarget = true );
void attachPlaceRequest( UOXSOCKET socket, cTargetRequest *targetRequest, UI16 houseId );

// We have that as only one targetting request is possible per socket
extern std::map< UOXSOCKET, cTargetRequest* > targetRequests;

#endif
