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

// WorldMain.h: interface for the CWorldMain class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__WORLDMAIN_H__)
#define __WORLDMAIN_H__ 

// Platform Specifics 
#include "platform.h"


// System includes

#include <iostream>

using namespace std;

// forward class declaration

class CWorldMain ;

//Wolfpack Includes
#include "typedefs.h"
#include "debug.h"

#include <zthread/Thread.h>
#include <zthread/FastMutex.h>
#include <zthread/LockedQueue.h>
#include <qstring.h>
#include <qmap.h>
#ifdef WIN32
#include <winsock.h>
#endif

class CWorldMain  
{
public:
	virtual int announce();
	virtual void announce(int choice);
	virtual void loadnewworld( QString module );
	virtual void savenewworld( QString module );
	CWorldMain();
	virtual ~CWorldMain();

	bool Saving( void );
	bool RemoveItemsFromCharBody(int charserial, int type1, int type2);
private:
	bool isSaving;
	int DisplayWorldSaves;
	FILE *iWsc, *cWsc;
	unsigned long Cur, Max;
	long PerLoop;
	void SaveChar( P_CHAR );
	void SaveItem( P_ITEM pi, P_ITEM pDefault );

	class cItemsSaver : public ZThread::Thread
	{
	private:
		ZThread::FastMutex waitMutex;
		QString module;
	public:
		cItemsSaver(QString mod) : module(mod) {}
		virtual ~cItemsSaver() throw() {}
		virtual void run() throw();
		void wait();
	};
};

#endif 
 
