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

// Client.h: interface for the cClient class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "typedefs.h"
#include "globals.h"

class cClient  
{
protected:
	UOXSOCKET socket_;
public:
	explicit cClient( UOXSOCKET nSocket );
	~cClient() {}

	UOXSOCKET socket() { return socket_; }
	P_CHAR player() { return currchar[ socket_ ]; }

	P_ITEM dragging( void );

	// Functions for the Client
	void sysMessage( UI16 color, const QString &text );
	void sysMessage( const QString &text );
};

typedef cClient* P_CLIENT;

#endif
