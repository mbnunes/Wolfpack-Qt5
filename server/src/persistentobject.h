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

#if !defined(__PERSISTENTOBJECT_H__)
#define __PERSISTENTOBJECT_H__

#include <typeinfo>
#include <qobject.h>
#include <qstringlist.h>
#include "singleton.h"
#include "factory.h"

class cBufferedReader;
class cBufferedWriter;

#pragma pack(1)
class PersistentObject
{
protected:
	bool isPersistent : 1;
public:
	PersistentObject();
	virtual ~PersistentObject()
	{
	}

	static void buildSqlString( const char *objectid, QStringList& fields, QStringList& tables, QStringList& conditions );
	virtual bool del();

	virtual void load( char**, UINT16& );
	virtual void save();

	// Wrapper
	virtual void load( cBufferedReader& reader ) = 0;
	virtual void save( cBufferedWriter& reader ) = 0;

	// "Real" ones
	virtual void load( cBufferedReader& reader, unsigned int version ) = 0;
	virtual void save( cBufferedWriter& reader, unsigned int version ) = 0;
	virtual void postload( unsigned int version );

	virtual const char* objectID() const
	{
		return "PersistentObject";
	}
};
#pragma pack()

/*
	This class is used to assign 8-bit keys to all 
	classes that should be serialized by the binary
	serialization system.
*/
class cBinaryTypemap {
protected:
	typedef QMap<unsigned char, QCString> Container;
	typedef Container::iterator Iterator;

	/*
		The map storing the 8-bit to string mapping.
	*/
	Container typemap;

public:
	/*
		Register a new type with this object and return
		the 8-bit identifier assigned to this type.
	*/
	unsigned int registerType(QCString type) {
		// Find the largest typeid in use
		int lastid = -1;        
		for (Iterator it = typemap.begin(); it != typemap.end(); ++it) {
			if (it.key() > lastid) {
				lastid = it.key();
			}
		}

		// Increase the typeid and register the type
		lastid += 1;
		typemap.insert(lastid, type);

		// Return the assigned type id.
		return lastid;
	}

	/*
		Get a type identified by its 8-bit key.
	*/
	QString getType(unsigned char key) {
		if (!typemap.contains(key)) {
			return QString::null;
		}

		return typemap[key];
	}

	/*
		Check if a given 8-bit key is known.
	*/
	bool hasType(unsigned char key) {
		return typemap.contains(key);
	}

	/*
		Return the full typemap.
	*/
	const QMap<unsigned char, QCString> &getTypemap() {
		return typemap;
	}
};

typedef SingletonHolder<cBinaryTypemap> BinaryTypemap;

/*
	This factory creates objects from type ids used in the 
	binary and database serialization process.
*/
class cPersistentFactory : public Factory<PersistentObject, QString> {
public:
	/*
		Register a SQL query to retrieve objects of a given type.
	*/
	void registerSqlQuery( const QString& type, const QString& query ) {
		sql_queries.insert(type, query);
		sql_keys.push_back( type );
	}

	/*
		Retrieve a SQL query to retrieve objects of a given type.
	*/
	QString findSqlQuery(const QString& type) const {
		QMap<QString, QString>::const_iterator iter = sql_queries.find( type );

		if (iter == sql_queries.end())
			return QString::null;
		else
			return iter.data();
	}

	/*
		Retrieve the list of registered object types.
	*/
	const QStringList &objectTypes() const {
		return sql_keys;
	}

private:
	QMap<QString, QString> sql_queries;
	QStringList sql_keys;
};

typedef SingletonHolder<cPersistentFactory> PersistentFactory;

/*
	A static and local instance of this class allows an automatic 
	registration of a type in the factory at application startup.
	The registration order isn't predictable though.
*/
template <class C>
class FactoryRegistration {
public:
	static PersistentObject *productCreator() {
		return new C;
	}

	FactoryRegistration(const char *className) {
		// Register the type for creation
		PersistentFactory::instance()->registerType(className, productCreator);

		// Build the SQL query for selecting all objects of this type
		QStringList fields, tables, conditions;
		C::buildSqlString(className, fields, tables, conditions);
		QString sqlString = QString("SELECT %1 FROM %2").arg(fields.join(",")).arg(tables.join(","));
		if (conditions.count() > 0) {
			sqlString.append(" WHERE ");
			sqlString.append(conditions.join(" AND "));
		}

		// Register the SQL query for this type in the factory
		PersistentFactory::instance()->registerSqlQuery(className, sqlString);

		// Register the type for the 8-bit binary mapping
		C::setClassid(BinaryTypemap::instance()->registerType(className));		
	}
};

#endif // __PERSISTENTOBJECT_H__
