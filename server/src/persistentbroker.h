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

#if !defined(__PERSISTENTBROKER_H__)
#define __PERSISTENTBROKER_H__

#include <qsqldatabase.h>
#include <typeinfo>
#include <qregexp.h>
#include <vector>

class PersistentObject;
class cDBDriver;
class cDBResult;

struct stDeleteItem
{
	QString tables;
	QString conditions;
};

class PersistentBroker
{
	cDBDriver* connection;
	bool sqlite;
	std::vector< stDeleteItem > deleteQueue;

public:
	PersistentBroker();
	~PersistentBroker();
	bool openDriver( const QString& driver );

	bool connect( const QString& host, const QString& db, const QString& username, const QString& password );
	void disconnect();

	bool executeQuery( const QString& query );
	cDBResult query( const QString& query );
	void flushDeleteQueue();
	void clearDeleteQueue();
	void addToDeleteQueue( const QString &tables, const QString &conditions );
	
	QString quoteString( QString d )
	{
		if( sqlite )
			return d.replace( QRegExp("'"), "''" );
		else
			return d.replace( QRegExp("'"), "\\'" );
	}

	void lockTable( const QString& table ) const;
	void unlockTable( const QString& table ) const;
	bool tableExists( const QString& table );

	bool saveObject( PersistentObject* );
	bool deleteObject( PersistentObject* );
	QString lastError() const;
	cDBDriver* driver() const;

	void startTransaction();
	void commitTransaction();
	void rollbackTransaction();
};

#define initSave QStringList conditions, fields, values; QString table;
#define clearFields conditions.clear(); fields.clear(); values.clear();
#define setTable( value ) table = value;

// Check here if we are updating or inserting
// for inserting we use the faster VALUES() method

#define addField( name, value ) \
	if( isPersistent ) \
		fields.push_back( QString( "%1='%2'" ).arg( name ).arg( QString::number( value ) ) ); \
	else \
	{ \
		/* fields.push_back( name ); */ \
		values.push_back( QString::number( value ) ); \
	}

#define addStrField( name, value ) \
	if( isPersistent ) \
		fields.push_back( QString( "%1='%2'" ).arg( name ).arg( persistentBroker->quoteString( value ) ) ); \
	else \
	{ \
		/* fields.push_back( name ); */ \
		values.push_back( "'" + ( value.isNull() ? QString( "" ) : persistentBroker->quoteString( value ) ) + "'" ); \
	}

#define addCondition( name, value ) conditions.push_back( QString( "%1 = '%2'" ).arg( name ).arg( value ) );

#define saveFields \
	if( isPersistent ) \
	{ \
		persistentBroker->executeQuery( QString( "UPDATE %1 SET %2 WHERE %3" ).arg( table ).arg( fields.join( "," ) ).arg( conditions.join( " AND " ) ) ); \
	} \
	else \
	{ \
		persistentBroker->executeQuery( QString( "REPLACE INTO %1 VALUES(%3)" ).arg( table )/*.arg( fields.join( "," ) )*/.arg( values.join( "," ) ) ); \
	}

#endif // __PERSISTENTBROKER_H__
