/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2006 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#include "python/tempeffect.h"

#include "platform.h"

// Wolfpack Includes
#include "timers.h"
#include "items.h"

#include "serverconfig.h"
#include "network/network.h"
#include "definitions.h"
#include "network/uosocket.h"
#include "dbdriver.h"
#include "persistentbroker.h"
#include "skills.h"
#include "mapobjects.h"
#include "basechar.h"
#include "player.h"
#include "npc.h"
#include "ai/ai.h"
#include "basics.h"
#include "world.h"
#include "inlines.h"

#include <algorithm>
#include <typeinfo>
#include <math.h>

int cTimer::getDest()
{
	return destSer;
}

void cTimer::setDest( int ser )
{
	destSer = ser;
}

int cTimer::getSour()
{
	return sourSer;
}

void cTimer::setSour( int ser )
{
	sourSer = ser;
}

void cTimer::setExpiretime_s( int seconds )
{
	expiretime = Server::instance()->time() + ( seconds * MY_CLOCKS_PER_SEC );
}

void cTimer::setExpiretime_ms( float milliseconds )
{
	expiretime = Server::instance()->time() + ( int ) floor( ( milliseconds / 1000 ) * MY_CLOCKS_PER_SEC );
}

/*
	Save a float to the effects_properties table.
 */
void cTimer::saveFloat( unsigned int id, const QString& key, double value )
{
	QSqlQuery q;
	q.prepare( "REPLACE INTO effects_properties VALUES(?,?,?,?)" );
	q.addBindValue( id );
	q.addBindValue( key );
	q.addBindValue( "float" );
	q.addBindValue( value );
	q.exec();
}

/*
	Save an integer to the effects_properties table.
 */
void cTimer::saveInt( unsigned int id, const QString& key, int value )
{
	QSqlQuery q;
	q.prepare( "REPLACE INTO effects_properties VALUES(?,?,?,?)" );
	q.addBindValue( id );
	q.addBindValue( key );
	q.addBindValue( "int" );
	q.addBindValue( value );
	q.exec();
}

/*
	Save a string to the effects_properties table.
 */
void cTimer::saveString( unsigned int id, const QString& key, const QString& value )
{
	QSqlQuery q;
	q.prepare( "REPLACE INTO effects_properties VALUES(?,?,?,?)" );
	q.addBindValue( id );
	q.addBindValue( key );
	q.addBindValue( "string" );
	q.addBindValue( value );
	q.exec();
}

void cTimer::saveChar( unsigned int id, const QString& key, P_CHAR character )
{
	unsigned int value = character->serial();
	QSqlQuery q;
	q.prepare( "REPLACE INTO effects_properties VALUES(?,?,?,?)" );
	q.addBindValue( id );
	q.addBindValue( key );
	q.addBindValue( "char" );
	q.addBindValue( value );
	q.exec();
}

void cTimer::saveItem( unsigned int id, const QString& key, P_ITEM item )
{
	unsigned int value = item->serial();
	QSqlQuery q;
	q.prepare( "REPLACE INTO effects_properties VALUES(?,?,?,?)" );
	q.addBindValue( id );
	q.addBindValue( key );
	q.addBindValue( "item" );
	q.addBindValue( value );
	q.exec();
}

bool cTimer::loadChar( unsigned int id, const QString& key, P_CHAR& character )
{
	QSqlQuery result;
	result.prepare( "SELECT value FROM effects_properties WHERE id = ? AND keyname = ? AND type = 'char'" );
	result.addBindValue( id );
	result.addBindValue( key );
	result.exec();

	if ( !result.next() )
	{
		return false;
	}

	character = 0;
	character = World::instance()->findChar( result.value( 0 ).toInt() );

	return true;
}

bool cTimer::loadItem( unsigned int id, const QString& key, P_ITEM& item )
{
	QSqlQuery result;
	result.prepare( "SELECT value FROM effects_properties WHERE id = ? AND keyname = ? AND type = 'item'" );
	result.addBindValue( id );
	result.addBindValue( key );
	result.exec();

	if ( !result.next() )
	{
		return false;
	}

	item = 0;
	item = World::instance()->findItem( result.value( 0 ).toInt() );

	return true;
}

/*
	Load a float from the effects_properties table.
 */
bool cTimer::loadFloat( unsigned int id, const QString& key, double& value )
{
	QSqlQuery result = PersistentBroker::instance()->query( QString( "SELECT value FROM effects_properties WHERE id = '%1' AND keyname = '%2' AND type = 'float'" ).arg( id ).arg( PersistentBroker::instance()->quoteString( key ) ) );

	if ( !result.next() )
	{
		return false;
	}

	value = result.value( 0 ).toDouble();

	return true;
}

/*
	Load an integer from the effects_properties table.
 */
bool cTimer::loadInt( unsigned int id, const QString& key, int& value )
{
	QSqlQuery result;
	result.prepare( "SELECT value FROM effects_properties WHERE id = ? AND keyname = ? AND type = 'int'" );
	result.addBindValue( id );
	result.addBindValue( key );
	result.exec();

	if ( !result.next() )
	{
		return false;
	}

	value = result.value( 0 ).toInt();

	return true;
}

/*
	Load a string from the effects_properties table.
 */
bool cTimer::loadString( unsigned int id, const QString& key, QString& value )
{
	QSqlQuery result;
	result.prepare( "SELECT value FROM effects_properties WHERE id = ? AND keyname = ? AND type = 'string'" );
	result.addBindValue( id );
	result.addBindValue( key );
	result.exec();

	if ( !result.next() )
	{
		return false;
	}

	value = result.value( 0 ).toString();

	return true;
}

void cTimer::save( unsigned int id )
{
	QSqlQuery query;
	query.prepare( "INSERT INTO effects VALUES( ?, ?, ?, ?, ?, ?)" );
	query.addBindValue( id );
	query.addBindValue( objectID() );
	query.addBindValue( expiretime - Server::instance()->time() );
	query.addBindValue( dispellable ? 1 : 0 );
	query.addBindValue( sourSer );
	query.addBindValue( destSer );
	query.exec();
}

void cTimer::load( unsigned int /*id*/, QSqlQuery& result )
{
	unsigned int offset = 2;

	expiretime = result.value( offset++ ).toInt() + Server::instance()->time();
	dispellable = result.value( offset++ ).toInt() == 0 ? false : true;
	sourSer = result.value( offset++ ).toInt();
	destSer = result.value( offset++ ).toInt();

	serializable = true;
}

cTimers::cTimers()
{
	std::make_heap( teffects.begin(), teffects.end(), cTimers::ComparePredicate() ); // No temp effects to start with
}

void cTimers::check()
{
	cTimer* tEffect = 0;

	while ( !teffects.empty() )
	{
		tEffect = *teffects.begin();
		if ( !tEffect || tEffect->expiretime > Server::instance()->time() )
			break;

		if ( isCharSerial( tEffect->getDest() ) )
		{
			P_CHAR pChar = dynamic_cast<P_CHAR>( FindCharBySerial( tEffect->getDest() ) );
			if ( pChar )
			{
				pChar->removeTimer( tEffect );
			}
		}

		tEffect->Expire();

		erase( tEffect );

		delete tEffect;
	}
}

/*!
	Dispels all Timers lasting on pc_dest of a given type.
	If only Dispellable is false then all effects on this character
	of the specified type are reverted.
*/
void cTimers::dispel( P_CHAR pc_dest, P_CHAR pSource, const QString& type, bool silent, bool onlyDispellable )
{
	/*if (cPythonScript::canChainHandleEvent(EVENT_DISPEL, pc_dest->getScripts())) {
		PyObject *source;
		if (pSource) {
			source = pSource->getPyObject();
		} else {
			Py_INCREF(Py_None);
			source = Py_None;
		}
		const char *ptype = "";
		if (!type.isEmpty()) {
			ptype = type.toLatin1().constData();
		}
		PyObject *args = Py_BuildValue("(NNBBsN", pc_dest->getPyObject(), source,
			silent ? 1 : 0, onlyDispellable ? 0 : 1, ptype, PyTuple_New(0));
		bool result = cPythonScript::callChainedEventHandler(EVENT_DISPEL, pc_dest->getScripts(), args);
		Py_DECREF(args);
		if (result) {
			return;
		}
	}*/

	std::vector<cTimer*>::iterator it = teffects.begin();
	QList<cTimer*> eraselist;

	/*
		Note: Erasing iterators would invalidate our iterator and crash.
	*/
	for ( ; it != teffects.end(); ++it )
	{
		cTimer* effect = *it;

		if ( ( !onlyDispellable || effect->dispellable ) && ( uint ) effect->getDest() == ( uint ) pc_dest->serial() && effect->objectID() == type )
		{
			pc_dest->removeTimer( effect );
			effect->Dispel( pSource, silent );
			eraselist.append( effect );
		}
	}

	foreach ( cTimer* effect, eraselist )
	{
		erase( effect );
	}
}

void cTimers::dispel( P_CHAR pc_dest, P_CHAR pSource, bool silent )
{
	if ( pc_dest->canHandleEvent( EVENT_DISPEL ) )
	{
		PyObject* source;
		if ( pSource )
		{
			source = pSource->getPyObject();
		}
		else
		{
			Py_INCREF( Py_None );
			source = Py_None;
		}

		PyObject* args = Py_BuildValue( "(NNBBsN", pc_dest->getPyObject(), source, silent ? 1 : 0, 0, "", PyTuple_New( 0 ) );
		bool result = pc_dest->callEventHandler( EVENT_DISPEL, args );
		Py_DECREF( args );

		if ( result )
		{
			return;
		}
	}

	QList<cTimer*> eraselist;
	std::vector<cTimer*>::iterator i = teffects.begin();
	for ( i = teffects.begin(); i != teffects.end(); i++ )
		if ( ( *i ) != NULL && ( *i )->dispellable && ( uint ) ( *i )->getDest() == ( uint ) pc_dest->serial() )
		{
			if ( isCharSerial( ( *i )->getDest() ) )
			{
				P_CHAR pChar = FindCharBySerial( ( *i )->getDest() );
				if ( pChar )
					pChar->removeTimer( ( *i ) );
			}

			( *i )->Dispel( pSource, silent );
			eraselist.append( *i );
		}

	foreach ( cTimer* effect, eraselist )
	{
		erase( effect );
	}
}

void cTimers::load()
{
	// Query the Database

	QSqlQuery result( "SELECT id,objectid,expiretime,dispellable,source,destination FROM effects ORDER BY expiretime ASC;" );

	while ( result.next() )
	{
		unsigned int id = result.value( 0 ).toInt();
		QString objectId = result.value( 1 ).toString();

		cTimer* effect = 0;

		if ( objectId == "cPythonEffect" )
		{
			effect = new cPythonEffect;
		}
		else
		{
			throw wpException( QString( "Unknown TempEffect Type: %1" ).arg( objectId ) );
		}

		effect->load( id, result );

		insert( effect );
	}
}

void cTimers::save()
{
	QSqlQuery query;
	if (!query.exec( "TRUNCATE effects;" )) {
		query.exec( "DELETE FROM effects;" );
	}
	if (!query.exec( "TRUNCATE effects_properties;" )) {
		query.exec( "DELETE FROM effects_properties;" );
	}

	std::vector<cTimer*>::iterator it = teffects.begin();
	unsigned int id = 0;

	while ( it != teffects.end() )
	{
		if ( ( *it )->isSerializable() )
			( *it )->save( id++ );

		++it;
	}
}

int cTimers::countSerializables()
{
	int count = 0;

	std::vector<cTimer*>::iterator it = teffects.begin();
	std::vector<cTimer*>::iterator end = teffects.end();
	for ( ; it != end; ++it )
	{
		if ( ( *it )->isSerializable() )
			++count;
	}
	return count;
}

cDelayedOnCreateCall::cDelayedOnCreateCall( cUObject* obj, const QString& definition ) : objSer_( obj->serial() ), def_( definition )
{
	setSerializable( false );
	expiretime = 0; // right on the next loop.
}

void cDelayedOnCreateCall::Expire()
{
	cUObject *object = World::instance()->findObject( objSer_ );

	if ( object )
	{
		object->onCreate( def_ );
	}
}

void cTimers::insert( cTimer* pT )
{
	// If the tempeffect has a char it affects,
	// then don't forge to add it to his effects
	if ( isCharSerial( pT->getDest() ) )
	{
		P_CHAR pChar = FindCharBySerial( pT->getDest() );
		if ( pChar )
			pChar->addTimer( pT );
	}

	this->teffects.push_back( pT );
	std::push_heap( teffects.begin(), teffects.end(), cTimers::ComparePredicate() );
}

void cTimers::erase( cTimer* pT )
{
	if ( pT == ( *teffects.begin() ) )
	{
		std::pop_heap( teffects.begin(), teffects.end(), cTimers::ComparePredicate() );
		teffects.pop_back();
	}
	else
	{
		std::vector<cTimer*>::iterator it = std::find( teffects.begin(), teffects.end(), pT );
		if ( it != teffects.end() )
		{
			teffects.erase( it );
			std::make_heap( teffects.begin(), teffects.end(), cTimers::ComparePredicate() );
		}
	}
}

void cTimer::load( cBufferedReader& reader, unsigned int /*version*/ )
{
	serializable = true;
	expiretime = Server::instance()->time() + reader.readInt();
	dispellable = reader.readBool();
	sourSer = reader.readInt();
	destSer = reader.readInt();
}

void cTimer::save( cBufferedWriter& writer, unsigned int /*version*/ )
{
	writer.writeInt( expiretime - Server::instance()->time() );
	writer.writeBool( dispellable );
	writer.writeInt( sourSer );
	writer.writeInt( destSer );
}

void cTimers::save( cBufferedWriter& writer )
{
	std::vector<cTimer*>::iterator it;
	for ( it = teffects.begin(); it != teffects.end(); ++it )
	{
		if ( ( *it )->isSerializable() )
		{
			writer.writeByte( 0xFC );
			writer.writeAscii( ( *it )->objectID().toLatin1() );
			( *it )->save( writer, writer.version() );
		}
	}
}

void cTimers::load( cBufferedReader& reader )
{
	QString objectId = reader.readAscii();

	cTimer *timer = 0;

	if ( objectId == "cPythonEffect" )
	{
		timer = new cPythonEffect;
	}
	else
	{
		throw wpException( QString( "Unknown TempEffect Type: %1" ).arg( objectId ) );
	}

	timer->load( reader, reader.version() );
	insert( timer );
}

cTimers::~cTimers()
{
	// Clear all teffects
	std::vector<cTimer*>::iterator it;
	for ( it = teffects.begin(); it != teffects.end(); ++it )
	{
		delete * it;
	}
	teffects.clear();
}
