/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In addition to that license, if you are running this program or modified
 * versions of it on a public system you HAVE TO make the complete source of
 * the version used by you available or provide people with a location to
 * download it.
 *
 * Wolfpack Homepage: http://wpdev.sf.net/
 */

#if !defined(__WPTARGETREQUEST_H__)
#define __WPTARGETREQUEST_H__

#include "platform.h"
#include "typedefs.h"

// Library includes
#include <qobject.h>

// Forward Definitions
class cUOSocket;
class cUORxTarget;

// Abstract base-class for target requests
class cTargetRequest
{
protected:
	UI32 timeout_; // Timeout in MS
	UI32 targetId_; // Target id so no overlapping targets are processed

public:
	cTargetRequest( void ) : timeout_(0) {} // Never times out
	virtual ~cTargetRequest( void ) {}

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target ) = 0; // Request has been answered
	virtual void timedout( cUOSocket *socket ) { Q_UNUSED(socket); }; // The user did not respond
	virtual void canceled( cUOSocket *socket ) { Q_UNUSED(socket); }; // Request has been canceled

	UI32 targetId( void ) const;
	void setTargetId( UI32 data );

	UI32 timeout( void ) const;
	void setTimeout( UI32 data );
};

// Inline members
inline UI32 cTargetRequest::targetId( void ) const
{
	return targetId_;
}

inline void cTargetRequest::setTargetId( UI32 data )
{
	targetId_ = data;
}

inline UI32 cTargetRequest::timeout( void ) const
{
	return timeout_;
}

inline void cTargetRequest::setTimeout( UI32 data )
{
	timeout_ = data;
}

#endif
