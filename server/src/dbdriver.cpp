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
#include "worldmain.h"
#include "junk.h"

// Library Includes
#ifdef WIN32
#include <winsock.h>
#endif

#include <qstring.h>
#include <mysql.h>

// Executes a query
bool cDBDriver::execute( const QString &query )
{
	MYSQL *mysql = cwmWorldState->mysql;

	return ( mysql_query( mysql, query.latin1() ) == 0 );
}

// Returns an error (if there is one)
QString cDBDriver::error()
{
	MYSQL *mysql = cwmWorldState->mysql;

	if( mysql_error( mysql ) )
	{
		return mysql_error( mysql );
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
	return atoi( row[offset] );
}

// Get a string with a specific offset
QString cDBDriver::getString( UINT32 offset )
{
	return row[offset];
}


