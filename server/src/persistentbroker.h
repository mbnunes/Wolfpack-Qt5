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
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

#if !defined(__PERSISTENTBROKER_H__)
#define __PERSISTENTBROKER_H__

#include <qsqldatabase.h>
#include <typeinfo>

class PersistentObject;

class PersistentBroker
{
	QSqlDatabase* connection;

public:
	PersistentBroker();
	~PersistentBroker();
	bool openDriver( const QString& driver );
	bool connect( const QString& host, const QString& db, const QString& username, const QString& password );
	void executeQuery( const QString& query );

	bool saveObject( PersistentObject* );
	bool deleteObject( PersistentObject* );
	QSqlDatabase* driver() const;
};

// Pseudo helper functions.
// We should consider inline templates in future.

inline QString __escapeReservedCharacters( const QString& d )
{
	return QString(d).replace( QRegExp("'"), "\\'" );
}

#define savePersistentIntValue(field, value) \
	SqlStatement += QString("%1='%2',").arg(field).arg(value)

#define savePersistentStrValue(field, value) \
	if ( !value.isNull() ) \
		SqlStatement += QString("%1='%2',").arg(field).arg(__escapeReservedCharacters(value))

#define startSaveSqlStatement(table)	\
	QString SqlStatement; \
	if ( this->isPersistent ) \
		SqlStatement = QString("UPDATE %1 SET ").arg(table); \
	else \
		SqlStatement = QString("INSERT INTO %1 SET ").arg(table); 

#define endSaveSqlStatement(condition)	\
	SqlStatement = SqlStatement.left( SqlStatement.length() - 1 ); \
	if ( this->isPersistent ) \
		persistentBroker->executeQuery( SqlStatement + QString("WHERE %1;").arg(condition)); \
	else \
		persistentBroker->executeQuery( SqlStatement + ";" ); 

#define startLoadSqlStatement(table, keyfield, keyvalue) \
	static QSqlCursor cursor(table); \
	cursor.setMode( QSqlCursor::ReadOnly ); \
	cursor.select( QString("%1='%2'").arg(keyfield).arg(keyvalue) ); \
	if ( cursor.first() ) 

#define endLoadSqlStatement(keyvalue) \
	else \
		qWarning(QString("Error trying to load persistent object %1, key %2").arg(typeid(this).name()).arg(keyvalue));

#define loadPersistentStrValue(fieldName, property) \
	property = cursor.field(fieldName)->value().toString();

#if defined(_DEBUG)
	#define loadPersistentIntValue(fieldName, property) \
		{\
			bool ok = false; \
			property = cursor.field(fieldName)->value().toInt(&ok); \
			if ( !ok ) \
				qWarning(QString("Error trying to read Integer value from field %1").arg(fieldName));\
		}
#else
	#define loadPersistentIntValue(fieldName, property) \
		property = cursor.field(fieldName)->value().toInt()
#endif

#if defined(_DEBUG)
	#define loadPersistentUIntValue(fieldName, property) \
		{ \
			bool ok = false; \
			property = cursor.field(fieldName)->value().toUInt(&ok); \
			if ( !ok ) \
				qWarning(QString("Error trying to read UInt value from field %1").arg(fieldName));\
		}
#else
	#define loadPersistentUIntValue(fieldName, property) \
		property = cursor.field(fieldName)->value().toUInt()
#endif


#endif // __PERSISTENTBROKER_H__
