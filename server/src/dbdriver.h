/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In addition to that license, if you are running this program or modified
 * versions of it on a public system you HAVE TO make the complete source of
 * the version used by you available or provide people with a location to
 * download it.
 *
 * Wolfpack Homepage: http://wpdev.sf.net/
 */

#if !defined(__DBDRIVER_H__)
#define __DBDRIVER_H__

struct st_mysql;
struct st_mysql_res;
struct sqlite;
struct sqlite_vm;

#include <qstring.h>
#include <map>
#include "defines.h"
#include "pythonscript.h"

class cDBResult;
class cSQLiteResult;
class cMySQLResult;

class cDBDriver
{
	friend class cDBResult;
protected:
	void* connection;

	QString _host, _dbname, _username, _password;
	QString lasterror_;

	void setLastError( const QString& d )
	{
		lasterror_ = d;
	}

public:
	cDBDriver() : connection( 0 )
	{
	}
	virtual ~cDBDriver()
	{
	};

	virtual const char* name() const
	{
		return NULL;
	}

	virtual bool open( int id = CONN_MAIN ) = 0;
	virtual void close() = 0;
	virtual bool exec( const QString& query ); // Just execute some SQL code, no return!
	virtual cDBResult query( const QString& query ) = 0; // Executes a query
	virtual void lockTable( const QString& table );
	virtual void unlockTable( const QString& table );
	virtual QString error(); // Returns an error (if there is one), uses the current connection
	virtual bool tableExists( const QString& table ) = 0;

	// Setters + Getters
	virtual void setActiveConnection( int id = CONN_MAIN );
	void setUserName( const QString& data )
	{
		_username = data;
	}
	void setPassword( const QString& data )
	{
		_password = data;
	}
	void setHostName( const QString& data )
	{
		_host = data;
	}
	void setDatabaseName( const QString& data )
	{
		_dbname = data;
	}
	QString host() const
	{
		return _host;
	}
	QString databaseName() const
	{
		return _dbname;
	}
	QString userName() const
	{
		return _username;
	}
	QString password() const
	{
		return _password;
	}
};

class cSQLiteDriver : public cDBDriver
{
public:
	const char* name() const
	{
		return "sqlite";
	}

	bool open( int id = CONN_MAIN );
	void close();

	bool tableExists( const QString& table );
	QString error()
	{
		return QString::null;
	}

	bool exec( const QString& query );
	cDBResult query( const QString& query );
	int lastInsertId();
};

class cMySQLDriver : public cDBDriver
{
	std::map<int, st_mysql*> connections;
public:
	const char* name() const
	{
		return "mysql";
	}

	bool open( int id = CONN_MAIN );
	void close();

	void lockTable( const QString& table );
	void unlockTable( const QString& table );
	bool tableExists( const QString& table );
	QString error();
	void setActiveConnection( int id );
	bool exec( const QString& query );
	cDBResult query( const QString& query );
};

class cDBResult : cPythonScriptable
{
public:
	char** _row;
	void* _result;
	void* _connection; // Connection occupied by this query
	bool mysql_type;
public:
	cDBResult() : _row( 0 ), _result( 0 ), _connection( 0 ), mysql_type( true )
	{
	} // Standard Constructor
	cDBResult( void* result, void* connection, bool mysql_type = true ) : _row( 0 ), _result( result ), _connection( connection )
	{
		this->mysql_type = mysql_type;
	}; // MySQL Constructor
	virtual ~cDBResult()
	{
	}

	void free(); // Call this to free the query
	char** data() const; // Get the data for the current row
	bool fetchrow(); // Fetchs a new row, returns false if there is no new row
	Q_INT32 getInt( Q_UINT32 offset ) const; // Get an integer with a specific offset
	QString getString( Q_UINT32 offset ) const; // Get a string with a specific offset

	bool isValid() const
	{
		return ( _result != 0 );
	}

	PyObject* getPyObject();
	bool implements( const QString& name );
	const char* className() const;
};

#undef MYSQL_RES

#endif
