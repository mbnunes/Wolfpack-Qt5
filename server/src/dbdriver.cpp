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

// Wolfpack Includes
#include "dbdriver.h"

// Library Includes
#ifdef WIN32
#include <winsock.h>
#endif

#include <qstring.h>
#include <qptrlist.h>
#include <mysql.h>
#include <errmsg.h>
#include <stdlib.h>

QPtrList< MYSQL > connections; // A connection pool

// Keep Alive Connections
void cDBDriver::ping()
{
	for( st_mysql* mysql = connections.first(); mysql; mysql = connections.next() )
	{
		if( !mysql_ping( mysql ) )
			connections.remove(); // Remove Current Connection
	}
}

// Closes unused connections
void cDBDriver::garbageCollect()
{
	while( connections.count() > 1 )
	{
		mysql_close( connections.last() );
		connections.remove();
	}
}

// Checks if there is a free connection available
// If yes, remove it from the pool
// Otherwise connect
st_mysql *cDBDriver::getConnection()
{
	// Connection available
	if( connections.count() >= 1 )
	{
		MYSQL *mysql = connections.first();
		connections.remove();
		return mysql;
	}

	MYSQL *mysql = mysql_init( 0 );
	if ( !mysql )
		throw QString("mysql_init(): insufficient memory to allocate a new object");
	mysql->reconnect = 1;
	
	if( !mysql_real_connect( mysql, _host.latin1(), _username.latin1(), _password.latin1(), _dbname.latin1(), 0, "wolfpack_db_pipe", CLIENT_COMPRESS ) )
	{
		switch ( mysql_errno(mysql) )
		{
		case CR_NAMEDPIPEOPEN_ERROR:
		case CR_NAMEDPIPEWAIT_ERROR:
		case CR_NAMEDPIPESETSTATE_ERROR:
			if ( mysql_real_connect(mysql, _host.latin1(), _username.latin1(), _password.latin1(), _dbname.latin1(), 0, 0, CLIENT_COMPRESS ) )
			{
				qWarning("Named Pipe connection to database failed, falling back to slower TCP/IP connection to database");
				break; 
			} // let it fall
		default:
			if ( !mysql_real_connect(mysql, _host.latin1(), _username.latin1(), _password.latin1(), _dbname.latin1(), 0, 0, CLIENT_COMPRESS ) )
			{ // for *nix users who's mysql does not have the named pipe option
				throw QString( "Connection to DB failed: %1" ).arg( mysql_error( mysql ) );
			}
		}
	}

	return mysql;
}

void cDBDriver::putConnection( st_mysql *data )
{
	// Put a connection back to the pool
	connections.append( data );
}

cDBDriver::~cDBDriver()
{
}

// Executes a query
cDBResult cDBDriver::query( const QString &query )
{
	MYSQL *mysql = getConnection();

	if( !mysql )
		throw QString( "Not connected to mysql server. Unable to execute query." );

	if( mysql_query( mysql, query.latin1() ) )
	{
		putConnection( mysql );
		return cDBResult(); // Return invalid result
	}
	
	MYSQL_RES *result = mysql_use_result( mysql );
	return cDBResult( result, mysql );
}

// Just execute some SQL code, no result!
bool cDBDriver::execute( const QString &query )
{
	MYSQL *mysql = getConnection();

	if( !mysql )
		throw QString( "Not connected to mysql server. Unable to execute query." );

	bool ok = !mysql_query( mysql, query.latin1() );
	putConnection( mysql );
	return ok;
}

// Returns an error (if there is one)
QString cDBDriver::error()
{
	MYSQL *mysql = getConnection();
	const char *error = mysql_error( mysql );

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
bool cDBResult::fetchrow()
{
	if( !_result )
		return false;

	_row = mysql_fetch_row( _result );
	return ( _row != 0 );
}

// Call this to free the query
void cDBResult::free()
{
	mysql_free_result( _result );
	_result = 0;
	_row = 0;

	if( _connection )
	{
		cDBDriver driver;
		driver.putConnection( _connection );
		_connection = 0;
	}
}

// Get the data for the current row
char** cDBResult::data()
{
	return _row;
}

// Get an integer with a specific offset
INT32 cDBResult::getInt( UINT32 offset )
{
	if( !_row )
		throw QString( "Trying to access a non valid result!" );

	return atoi( _row[offset] );
}

// Get a string with a specific offset
QString cDBResult::getString( UINT32 offset )
{
	if( !_row )
		throw QString( "Trying to access a non valid result!" );

	return _row[offset];
}

// Static Member Declarations...
QString cDBDriver::_host;
QString cDBDriver::_dbname;
QString cDBDriver::_username;
QString cDBDriver::_password;
