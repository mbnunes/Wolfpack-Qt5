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


#include "zthread/FastMutex.h"
#include "zthread/Thread.h"
#include "zthread/LockedQueue.h"

#include "qmap.h"

class QSocketDevice;
class cAsyncNetIOPrivate;
class cUOPacket;

class cAsyncNetIO : public ZThread::Thread
{
	QMap<QSocketDevice*, cAsyncNetIOPrivate*> buffers;
	typedef QMap<QSocketDevice*, cAsyncNetIOPrivate*>::iterator iterator;
	typedef QMap<QSocketDevice*, cAsyncNetIOPrivate*>::const_iterator const_iterator;

	ZThread::FastMutex mapsMutex;

public:

	bool registerSocket(QSocketDevice*);
	bool unregisterSocket(QSocketDevice*);
    Q_ULONG	bytesAvailable(QSocketDevice*) const;

	virtual void run() throw();

	cUOPacket* readPacket( QSocketDevice* );
	void sendPacket(QSocketDevice*, cUOPacket*);

private:
	void buildUOPackets( cAsyncNetIOPrivate* );
};

