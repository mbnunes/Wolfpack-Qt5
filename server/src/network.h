//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
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

#if !defined(__NETWORK_H__)
#define __NETWORK_H__


//Platform specifics
#include "platform.h"

// System Includes

//Forward class Declaration
class cNetworkStuff;
class cAsyncNetIO;
class cUOSocket;
class cListener;
class QHostAddress;

//Wolfpack includes
#include "typedefs.h"

// Library Includes
#include <qptrlist.h>

struct ip_block_st
{
	unsigned long address;
	unsigned long mask;
};

class cNetwork
{
private:
	cNetwork();
	~cNetwork();

	//typedef gamesocketsIterator QPtrListIterator;
	//typedef loginsocketsIterator QPtrListIterator;

	static cNetwork *instance_;
	
//	std::vector<ip_block_st> hosts_deny;
	QPtrList< cUOSocket > uoSockets;
	QPtrList< cUOSocket > loginSockets;
	cAsyncNetIO *netIo_;
	cListener *listener_;

	// junk
	UOXSOCKET getuoSocketsIndex( const cUOSocket* );
	friend UOXSOCKET calcSocketFromChar(P_CHAR);

public:

	void xSend(UOXSOCKET s, const void *point, int length, int test);
	
	void load( void );
	void unload( void );
	void reload( void );
	
	bool CheckForBlockedIP( const QHostAddress& ip_address );

	static void startup( void ) { instance_ = new cNetwork; }
	static void shutdown( void ) { delete instance_; }
	static cNetwork *instance( void ) { return instance_; }

	void poll( void ); // called by the main loop

	cAsyncNetIO *netIo()	{ return netIo_; }
	cUOSocket *first()		{ return uoSockets.first(); }
	cUOSocket *next()		{ return uoSockets.next(); }
	UINT32 count()			{ return uoSockets.count(); }
	QPtrListIterator<cUOSocket> getIterator() { return QPtrListIterator<cUOSocket>(uoSockets); }

	void	broadcast( const QString &message, UINT16 color = 0x84d, UINT16 font = 0 );
};

// Helper Function
UOXSOCKET calcSocketFromChar(P_CHAR pc);
#endif
