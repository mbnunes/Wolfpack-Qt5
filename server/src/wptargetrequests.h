#ifndef __WPTARGETREQUEST_H__
#define __WPTARGETREQUEST_H__

#include "wolfpack.h"

// Abstract base-class for target requests
class cTargetRequest
{
protected:
	UI32 timeout_; // Timeout in MS


public:
	cTargetRequest( void ) { timeout_ = 0; }; // Never times out
	virtual ~cTargetRequest( void ) { };

	virtual void responsed( UOXSOCKET socket, PKGx6C targetInfo ) {}; // Request has been answered
	virtual void timedout( UOXSOCKET socket ) {}; // Request is overwritten

	UI32 timeout( void ) { return timeout_; }; // Get the timeout-value
	void setTimeout( UI32 data ) { timeout_ = data; }; // Set the timeout-value
};

// Several public functions
void checkTimedOutTargets( void );
void attachTargetRequest( UOXSOCKET socket, cTargetRequest *targetRequest );

// We have that as only one targetting request is possible per socket
extern map< UOXSOCKET, cTargetRequest* > targetRequests;

#endif