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

/* 
	Online status class
*/

#include "onlinestatus.h"

void cOnlineStatus::reload()
{
	uptime_.restart();
}

cOnlineStatus::cOnlineStatus()
{ 

	uptime_.start(); 

#if defined(__unix__)
	cStatFile	stat_self;
	prv_uptime_ = uptime_.elapsed();
	prv_cpu_ = stat_self.getCPUTime() * 10;
#endif

}

QString cOnlineStatus::getUptime()
{
	int msecs, seconds, minutes, hours, days;
	int elps = uptime_.elapsed();
	
	days = elps / 86400000;
	hours = (elps % 86400000) / 3600000;
	minutes = (( elps % 86400000 ) % 3600000 ) / 60000;
	seconds = ((( elps % 86400000 ) % 3600000 ) % 60000 ) / 1000;
	msecs = ((( elps % 86400000 ) % 3600000 ) % 60000 ) % 1000;
	
	return QString( "%1:%2:%3:%4.%5" ).arg( days ).arg( hours ).arg( minutes ).arg( seconds ).arg( msecs );
	
}

QString	cOnlineStatus::getMem()
{
#if defined(__unix__)
	cStatFile	stat_self;
	return QString( "%1 K" ).arg( stat_self.getRSS() / 1024 );
#endif
	return "";
}
QString cOnlineStatus::getCpu()
{
#if defined(__unix__)
	cStatFile	stat_self;
	Q_UINT32	cur_cpu = stat_self.getCPUTime() * 10;
	Q_UINT32	cur_uptime = uptime_.elapsed();
	QString		percents;
	
	percents = QString( "%1.%2%" ).arg( ( 100 * ( cur_cpu - prv_cpu_ ) ) / ( cur_uptime - prv_uptime_ ) ).arg( (( 100 * ( cur_cpu - prv_cpu_ ) ) % ( cur_uptime - prv_uptime_ ) );
	
	prv_cpu_ = cur_cpu;
	prv_uptime_ = cur_uptime;

	return percents;
#endif

	return "";
}


#if defined(__unix__) //linux classes

bool cStatFile::refresh() {
    if ( stat_file_.open( IO_ReadOnly ) ) {
	QString tmp(stat_file_.readAll());
	stat_fields_ = tmp;    
	stat_file_.close();
	return true;
    }	 
    else return false;

}
Q_UINT32 cStatFile::getCPUTime() {
	refresh();
        return stat_fields_.section( ' ', 13, 13 ).toULong();   	
}

Q_UINT32 cStatFile::getVM() {
	refresh();
        return stat_fields_.section( ' ', 22, 22 ).toULong();   	
}

Q_UINT32 cStatFile::getRSS() {
	refresh();
        return stat_fields_.section( ' ', 23, 23 ).toULong() * 4096;   	
}

cStatFile::cStatFile( QString filename) {
    stat_file_.setName( filename );
}


#endif //linux classes