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

#include "persistentbroker.h"
#include "persistentobject.h"

#include "dbdriver.h"
#include "wpconsole.h"
#include "globals.h"

// Qt Includes
#include <qstring.h>


PersistentBroker::PersistentBroker() : connection(0)
{
}

PersistentBroker::~PersistentBroker()
{
	delete connection;
}

bool PersistentBroker::openDriver( const QString& driver )
{
	if( connection != 0 )
	{
		connection->close();
		delete connection;
		connection = 0;
	}

	if( driver == "sqlite" )
	{
		connection = new cSQLiteDriver();
		sqlite = true;
	}
	else
	{
		connection = new cDBDriver;
		sqlite = false;
	}

	if ( !connection )
		return false;

	return true;
}

bool PersistentBroker::connect( const QString& host, const QString& db, const QString& username, const QString& password )
{
	if (!connection)
		return false;

	// This does nothing but a little test-connection
	connection->setDatabaseName( db );
	connection->setUserName( username );
	connection->setPassword( password );
	connection->setHostName( host );
	
	if( !connection->open() )
		return false;

	// Disable fsynch for sqlite
	if( sqlite )
	{
		connection->exec( "PRAGMA synchronous = OFF;" );
		connection->exec( "PRAGMA default_synchronous = OFF;" );
		connection->exec( "PRAGMA full_column_names = OFF;" );
		connection->exec( "PRAGMA show_datatypes = OFF;" );
	}

	return true;
}

void PersistentBroker::disconnect()
{
	if( connection )
		connection->close();
}

bool PersistentBroker::saveObject( PersistentObject* object )
{
	// Start Transaction
	/*if( sqlite )
		connection->exec( "BEGIN TRANSACTION;" );*/

	try
	{
		object->save();
	}
	catch( ... )
	{
		// Rollback
		/*if( sqlite )
			connection->exec( "ROLLBACK TRANSACTION;" );*/
		throw;
	}

	// Commit
	/*if( sqlite )
		connection->exec( "COMMIT TRANSACTION;" );*/

	return true;
}

bool PersistentBroker::deleteObject( PersistentObject* object )
{
//	static const bool hasTransaction = connection->driver()->hasFeature(QSqlDriver::Transactions);
//	if ( hasTransaction )
//		connection->transaction();
	if ( object->del() )
	{
//		if ( hasTransaction )
//			connection->commit();
		return true;
	}
	else
	{
//		if ( hasTransaction )
//			connection->rollback();
		return false;
	}
}

bool PersistentBroker::executeQuery( const QString& query )
{
	if( !connection )
		throw QString( "PersistentBroker not connected to database." );

	//qWarning( query );
	bool result = connection->exec(query);
	if( !result )
	{
		qWarning( query );
		clConsole.ChangeColor( WPC_RED );
		clConsole.send( "ERROR" );
		clConsole.ChangeColor( WPC_NORMAL );
		clConsole.send( ":" + connection->error() + "\n" );
	}
	return result;
}

cDBDriver* PersistentBroker::driver() const
{
	return connection;
}

cDBResult PersistentBroker::query( const QString& query )
{
	if( !connection )
		throw QString( "PersistentBroker not connected to database." );

	return connection->query( query );
}

void PersistentBroker::clearDeleteQueue()
{
	deleteQueue.clear();
}

void PersistentBroker::flushDeleteQueue()
{
	std::vector< stDeleteItem >::iterator iter;
	for( iter = deleteQueue.begin(); iter != deleteQueue.end(); ++iter )
	{
		QString tables = (*iter).tables;
		QString conditions = (*iter).conditions;
		QString sql = QString( "DELETE FROM %1 WHERE %2" ).arg( tables ).arg( conditions );
		executeQuery( sql );
	}

	deleteQueue.clear();
}

void PersistentBroker::addToDeleteQueue( const QString &tables, const QString &conditions )
{
	stDeleteItem dItem;
	dItem.tables = tables;
	dItem.conditions = conditions;
	deleteQueue.push_back( dItem );
}

QString PersistentBroker::lastError() const
{
	return connection->error();
}

void PersistentBroker::lockTable( const QString& table ) const
{
	connection->lockTable( table );
}

void PersistentBroker::unlockTable( const QString& table ) const
{
	connection->unlockTable( table );
}