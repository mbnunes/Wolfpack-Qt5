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

#if !defined(__DBDRIVER_H__)
#define __DBDRIVER_H__

struct st_mysql;
struct st_mysql_res;

#include <qstring.h>
#include <map>
#include "defines.h"

class cDBResult;

class cDBDriver
{
	friend class cDBResult;
private:
	std::map< int, st_mysql* > connections;
	st_mysql *connection;
	QString _host, _dbname, _username, _password;
	QString lasterror_;

	void setLastError( const QString& d ) { lasterror_ = d; }

public:
	cDBDriver() : connection(0) {}
	virtual ~cDBDriver();

	bool open( int id = CONN_MAIN );
	void close();
	bool exec( const QString &query ) const; // Just execute some SQL code, no return!	
	cDBResult query( const QString &query ); // Executes a query
	void lockTable( const QString& table ) const;
	void unlockTable( const QString& table ) const;
	QString error(); // Returns an error (if there is one), uses the current connection
	
	// Setters + Getters
	void setActiveConnection( int id = CONN_MAIN );
	void setUserName( const QString &data ) { _username = data; }
	void setPassword( const QString &data ) { _password = data; }
	void setHostName( const QString &data ) { _host = data; }
	void setDatabaseName( const QString &data ) { _dbname = data; }
	QString host() const { return _host; }
	QString databaseName() const { return _dbname; }
	QString userName() const { return _username; }
	QString password() const { return _password; }
};

class cDBResult
{
private:
	char **_row;
	st_mysql_res *_result;
	st_mysql *_connection; // Connection occupied by this query
public:
	cDBResult(): _row( 0 ), _result( 0 ), _connection( 0 ) {} // Standard Constructor
	cDBResult( st_mysql_res *result, st_mysql *connection ): _row( 0 ), _result( result ), _connection( connection ) {}; // MySQL Constructor
	virtual ~cDBResult() {}

	void free(); // Call this to free the query
	char** data() const; // Get the data for the current row
	bool fetchrow(); // Fetchs a new row, returns false if there is no new row
	INT32 getInt( UINT32 offset ) const; // Get an integer with a specific offset
	QString getString( UINT32 offset ) const; // Get a string with a specific offset

	bool isValid() const { return ( _result != 0 ); }
};

#undef MYSQL_RES

#endif 
