//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
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
#include <qglobal.h>

#if defined(Q_OS_WIN32) // Required by mysql.h
# include <winsock.h>
#endif

#include <qstring.h>
#include <qptrlist.h>
#include <mysql.h>
#include <errmsg.h>
#include <stdlib.h>

/*****************************************************************************
  cDBDriver member functions
 *****************************************************************************/

/*!
	\class cDBDriver dbdriver.h

	\brief The cDBDriver class provides an abstraction for the different DBMS
	supported by Wolfpack.

	\ingroup database
	\ingroup mainclass
*/

/*!
	Destructs the Database Driver instance
*/
cDBDriver::~cDBDriver()
{
	close();
}

/*!
	Opens the connection between wolfpack and the database
*/
bool cDBDriver::open( int id ) 
{
	if ( connection )
		return true;

	connection = mysql_init( 0 );
	if ( !connection )
		throw QString("mysql_init(): insufficient memory to allocate a new object");
	connection->reconnect = 1;
	
	if ( !mysql_real_connect(connection, _host.latin1(), _username.latin1(), _password.latin1(), _dbname.latin1(), 0, 0, CLIENT_COMPRESS ) )
	{ // Named pipes are acctually slower :(
		throw QString( "Connection to DB failed: %1" ).arg( mysql_error( connection ) );
	}
	connections[ id ] = connection;

	return true;
}

/*!
	Closes database handler and frees used memory
*/
void cDBDriver::close()
{
	mysql_close( connection );
}

/*!
	Executes a \a query and returns it's result
	\sa cDBResult
*/
cDBResult cDBDriver::query( const QString &query )
{
	MYSQL *mysql = connection;

	if( !mysql )
		throw QString( "Not connected to mysql server. Unable to execute query." );

	if( mysql_query( mysql, query.latin1() ) )
	{
		return cDBResult(); // Return invalid result
	}
	
	MYSQL_RES *result = mysql_use_result( mysql );
	return cDBResult( result, mysql );
}

/*!
	Executes a SQL command string without returning from the database.
	Returns true if executed successfully.
*/
bool cDBDriver::exec( const QString &query ) const
{
	if( !connection )
		throw QString( "Not connected to mysql server. Unable to execute query." );

	bool ok = !mysql_query( connection, query.latin1() );
	return ok;
}

void cDBDriver::lockTable( const QString& table ) const
{
	exec( QString("LOCK TABLES %1 WRITE;").arg(table) );
}

void cDBDriver::unlockTable( const QString& table ) const
{
	exec( QString("UNLOCK TABLES") );
}

// Returns an error (if there is one)
QString cDBDriver::error()
{
	const char *error = mysql_error( connection );

	if( error != 0 )
	{
		return QString(error);
	}
	else
	{
		return QString::null;
	}
}

void cDBDriver::setActiveConnection( int id )
{
	if( connections.find( id ) == connections.end() )
	{
		connection = NULL;
		open( id );		
	}
	else
	{
		connection = connections[ id ];
	}
}

/*****************************************************************************
  cDBResult member functions
 *****************************************************************************/

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
	_connection = 0;
}

// Get the data for the current row
char** cDBResult::data() const
{
	return _row;
}

// Get an integer with a specific offset
INT32 cDBResult::getInt( UINT32 offset ) const
{
	if( !_row )
		throw QString( "Trying to access a non valid result!" );

	return atoi( _row[offset] );
}

// Get a string with a specific offset
QString cDBResult::getString( UINT32 offset ) const
{
	if( !_row )
		throw QString( "Trying to access a non valid result!" );

	return _row[offset];
}
