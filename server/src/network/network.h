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

#if !defined(__NETWORK_H__)
#define __NETWORK_H__

// Library Includes
#include <qmutex.h>
#include <qptrlist.h>

// Wolfpack Includes
#include "../platform.h"
#include "../singleton.h"
#include "../typedefs.h"
#include "uosocket.h"

//Forward class Declaration
class cNetworkStuff;
class cAsyncNetIO;
class cListener;
class QHostAddress;

class cNetwork : public cComponent
{
	QPtrList<cUOSocket> uoSockets;
	QPtrList<cUOSocket> loginSockets;
	cAsyncNetIO* netIo_;
	cListener* loginServer_;
	cListener* gameServer_;
	QMutex mutex;

public:
	cNetwork();
	~cNetwork();

	void startup();
	void shutdown();

	void load();
	void unload();
	void reload();

	bool CheckForBlockedIP( const QHostAddress& ip_address );

	void poll( void ); // called by the main loop

	void lock()
	{
		mutex.lock();
	}
	void unlock()
	{
		mutex.unlock();
	}
	cAsyncNetIO* netIo()
	{
		return netIo_;
	}
	cUOSocket* first()
	{
		return uoSockets.first();
	}
	cUOSocket* next()
	{
		return uoSockets.next();
	}
	UINT32 count()
	{
		return uoSockets.count();
	}
	QPtrListIterator<cUOSocket> getIterator()
	{
		return QPtrListIterator<cUOSocket>( uoSockets );
	}

	void broadcast( const QString& message, UINT16 color = 0x84d, UINT16 font = 0 );
};

typedef SingletonHolder<cNetwork> Network;

#endif
