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

#if !defined (__ONLINESTATUS_H__)
#define __ONLINESTATUS_H__

#include "typedefs.h"
#include "singleton.h"
#include "qdatetime.h"
#include "qfile.h"

/*	Online status class
	contains global performance data, timings etc
*/

class cOnlineStatus {

public:
				cOnlineStatus() { tUptime_.start(); }
		void	reload();
		QString	getUptime();

private:
	QString		pCpuload_;		// in percents
	QTime		tUptime_;		// uptime days:hours:minutes:seconds.msecs
	

};

typedef SingletonHolder<cOnlineStatus> OnlineStatus;

#if defined(__unix__) //linux classes

class cStatFile {

public:
		cStatFile( QString filename = "/proc/self/stat" );
    Q_UINT32 	getCPUTime();
    Q_UINT32	getVM();
    Q_UINT32	getRSS();
    bool	refresh();
    
private:
    QString 	stat_fields_;
    QFile 	stat_file_;
    bool	stat_opened;

};

#endif // linux classes

#endif // __ONLINESTATUS_H__