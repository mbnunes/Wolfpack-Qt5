//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2004 by holders identified in authors.txt
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

#if !defined(__ASYNCNETIO_H__)
#define __ASYNCNETIO_H__

#include <qthread.h>
#include <qmutex.h>

#include <qmap.h>

class QSocketDevice;
class cAsyncNetIOPrivate;
class cUOPacket;

class cAsyncNetIO : public QThread
{
	QMap<QSocketDevice*, cAsyncNetIOPrivate*> buffers;

	typedef QMap<QSocketDevice*, cAsyncNetIOPrivate*>::iterator			iterator;
	typedef QMap<QSocketDevice*, cAsyncNetIOPrivate*>::const_iterator	const_iterator;

	QMutex mapsMutex;
	QWaitCondition waitCondition;

	bool volatile canceled_;

public:
	cAsyncNetIO() : canceled_(false) {}
	~cAsyncNetIO() throw();

	bool registerSocket(QSocketDevice*, bool loginSocket);
	bool unregisterSocket(QSocketDevice*);
	Q_ULONG	bytesAvailable(QSocketDevice*) const;

	cUOPacket* recvPacket( QSocketDevice* );
	void sendPacket(QSocketDevice*, cUOPacket*, bool);

	void flush( QSocketDevice* );
	bool canceled() const { return canceled_;	}
	void cancel() {	canceled_ = true; waitCondition.wakeAll(); }

protected:
	virtual void run() throw();

private:
	void buildUOPackets( cAsyncNetIOPrivate* );
	void flushWriteBuffer( cAsyncNetIOPrivate* );
};

#endif //__ASYNCNETIO_H__

