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

// Wolfpack Includes
#include "dbdriver.h"

// Library Includes
#ifdef WIN32
#include <winsock.h>
#endif

#include <qstring.h>
#include <mysql.h>

static MYSQL *mysql = 0;

// Executes a query
bool cDBDriver::query( const QString &query )
{
	if( mysql_query( mysql, query.latin1() ) )
		return false;

	result = mysql_use_result( mysql );
	return true;
}

// Just execute some SQL code, no result!
bool cDBDriver::execute( const QString &query )
{
	qWarning( "SQL: " + query );

	return !mysql_query( mysql, query.latin1() );
}

// Returns an error (if there is one)
QString cDBDriver::error()
{
	char *error = mysql_error( mysql );

	if( error != 0 )
	{
		return QString(error);
	}
	else
	{
		return QString::null;
	}
}

// Fetchs a new row, returns false if there is no new row
bool cDBDriver::fetchrow()
{
	row = mysql_fetch_row( result );
	return ( row != 0 );
}

// Call this to free the query
void cDBDriver::free()
{
	mysql_free_result( result );
}

// Get the data for the current row
char** cDBDriver::data()
{
	return row;
}

// Get an integer with a specific offset
INT32 cDBDriver::getInt( UINT32 offset )
{
	if( !row )
		throw QString( "Trying to access non-selected row!" );

	return atoi( row[offset] );
}

// Get a string with a specific offset
QString cDBDriver::getString( UINT32 offset )
{
	return row[offset];
}

bool cDBDriver::connect( const QString &host, const QString &database, const QString &username, const QString &password )
{
	// If we are already connected, try to disconnect
	if( mysql != 0 )
		disconnect();

	mysql = mysql_init( 0 );

	// This should be enabled/disabled via some SrvParam option ?!
	//mysql_options(mysql,MYSQL_OPT_COMPRESS,0);
	
	return mysql_real_connect( mysql, host.latin1(), username.latin1(), password.latin1(), database.latin1(), 0, NULL, 0 );
}

void cDBDriver::disconnect()
{
	if( mysql )
	{
		mysql_close( mysql );
		mysql = 0;
	}	
}
