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

#if !defined(__TARGETREQUEST_H__)
#define __TARGETREQUEST_H__

class cUORxTarget;
class cUOSocket;

/*!
	\brief Abstract baseclass for target requests sent to the client.
*/
class cTargetRequest
{
protected:
	unsigned int timeout_; // Timeout in MS
	unsigned int targetId_; // Target id so no overlapping targets are processed
public:
	cTargetRequest() : timeout_( 0 )
	{
	}

	virtual ~cTargetRequest()
	{
	}

	/*!
		The client selected a target.
	*/
	virtual bool responsed( cUOSocket* socket, cUORxTarget* target ) = 0;

	/*!
		The target request timed out after the given timeout.
	*/
	virtual void timedout( cUOSocket* )
	{
	}

	/*!
		The target request has been canceled by the client.
	*/
	virtual void canceled( cUOSocket* )
	{
	}

	/*!
		Return the unique id for this target.
	*/
	unsigned int targetId() const;

	/*!
		Set the unique id for this target.
	*/
	void setTargetId( unsigned int data );

	/*!
		Return the timeout value for this target.
	*/
	unsigned int timeout() const;

	/*!
		Set the timeout value for this target.
	*/
	void setTimeout( unsigned int data );
};

// Inline members
inline unsigned int cTargetRequest::targetId() const
{
	return targetId_;
}

inline void cTargetRequest::setTargetId( unsigned int data )
{
	targetId_ = data;
}

inline unsigned int cTargetRequest::timeout() const
{
	return timeout_;
}

inline void cTargetRequest::setTimeout( unsigned int data )
{
	timeout_ = data;
}

#endif
