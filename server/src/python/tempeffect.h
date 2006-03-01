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

#if !defined(__TEMPEFFECT_H__)
#define __TEMPEFFECT_H__

#include "engine.h"
#include "utilities.h"

#include "../coord.h"
#include "../basics.h"
#include "../timers.h"
#include "../persistentbroker.h"
#include "../customtags.h"

#include "../dbdriver.h"
#include "../world.h"
#include "../console.h"

#include <QSqlQuery>
#include <QVariant>

class cPythonEffect : public cTimer
{
protected:
	PythonFunction* func;
	PythonFunction* dispelFunc_;
	QString dispelId_;
	PyObject* args;
public:
	cPythonEffect() : func( 0 ), dispelFunc_( 0 )
	{
		objectid = "cPythonEffect";
		args = 0;
	}

	cPythonEffect( PythonFunction* _functionObj, PyObject* _args ) : func( _functionObj ), dispelFunc_(0), args( _args )
	{
		objectid = "cPythonEffect";
		Py_INCREF( args );
	}

	virtual ~cPythonEffect()
	{
		Py_XDECREF( args );
		delete func;
		delete dispelFunc_;
	}

	void setDispelId( const QString& data )
	{
		dispelId_ = data;
	}

	QString dispelId()
	{
		return dispelId_;
	}

	void setDispelFunc( PythonFunction* data )
	{
		dispelFunc_ = data;
	}

	PythonFunction* dispelFunc()
	{
		return dispelFunc_;
	}

	void Dispel( P_CHAR pSource, bool /*silent*/ )
	{
		// We will ignore silent here.
		PyObject *list = PyList_New( 0 );
		Dispel( pSource, list );
		Py_DECREF( list );
	}

	// Dispel args: char, [args], source, [args] (while the last one is optional)
	void Dispel( P_CHAR pSource, PyObject* disp_args )
	{
		if ( !dispelFunc_ || !dispelFunc_->isValid() )
			return;

		// Create our Argument list
		PyObject* p_args = PyTuple_New( 4 );

		if ( isItemSerial( destSer ) )
			PyTuple_SetItem( p_args, 0, PyGetItemObject( FindItemBySerial( destSer ) ) );
		else if ( isCharSerial( destSer ) )
			PyTuple_SetItem( p_args, 0, PyGetCharObject( FindCharBySerial( destSer ) ) );
		else
		{
			Py_INCREF( Py_None );
			PyTuple_SetItem( p_args, 0, Py_None );
		}

		Py_INCREF( args ); // PyTuple_SetItem steals a reference
		PyTuple_SetItem( p_args, 1, args );

		PyTuple_SetItem( p_args, 2, PyGetCharObject( pSource ) );

		Py_INCREF( disp_args );
		PyTuple_SetItem( p_args, 3, disp_args );

		PyObject* result = (*dispelFunc_)( p_args );
		Py_XDECREF( result );
		Py_DECREF( p_args );
	}

	void Expire()
	{
		// Create our Argument list
		PyObject* p_args = PyTuple_New( 2 );
		if ( isItemSerial( destSer ) )
			PyTuple_SetItem( p_args, 0, PyGetItemObject( FindItemBySerial( destSer ) ) );
		else if ( isCharSerial( destSer ) )
			PyTuple_SetItem( p_args, 0, PyGetCharObject( FindCharBySerial( destSer ) ) );
		else
		{
			Py_INCREF( Py_None );
			PyTuple_SetItem( p_args, 0, Py_None );
		}

		Py_INCREF( args );
		PyTuple_SetItem( p_args, 1, args );

		PyObject* result = (*func)( p_args );
		Py_XDECREF( result );
		Py_DECREF( p_args );
	}

	void save( cBufferedWriter& writer, unsigned int version )
	{
		cTimer::save( writer, version );

		writer.writeUtf8( func ? func->functionPath() : "" );
		writer.writeUtf8( dispelFunc_ ? dispelFunc_->functionPath() : "" );
		writer.writeUtf8( dispelId_ );

		int count = PyTuple_Size( args );
		writer.writeInt( count );

		for ( int i = 0; i < count; ++i )
		{
			PyObject *object = PyTuple_GetItem( args, i );

			if ( PyInt_Check( object ) )
			{
				int value = PyInt_AsLong( object );
				cVariant( value ).serialize( writer, version );
			}
			else if ( PyString_Check( object ) || PyUnicode_Check( object ) )
			{
				QString value = Python2QString( object );
				cVariant( value ).serialize( writer, version );
			}
			else if ( PyFloat_Check( object ) )
			{
				double value = PyFloat_AsDouble( object );
				cVariant( value ).serialize( writer, version );
			}
			else if ( checkWpChar( object ) )
			{
				P_CHAR pChar = getWpChar( object );
				cVariant( pChar ).serialize( writer, version );
			}
			else if ( checkWpItem( object ) )
			{
				P_ITEM pItem = getWpItem( object );
				cVariant( pItem ).serialize( writer, version );
			}
			else if ( checkWpCoord( object ) )
			{
				Coord coord = getWpCoord( object );
				cVariant( coord ).serialize( writer, version );
			}
		}
	}

	void load( cBufferedReader& reader, unsigned int version )
	{
		cTimer::load( reader, version );

		func = new PythonFunction( reader.readUtf8() );
		dispelFunc_ = new PythonFunction( reader.readUtf8() );
		dispelId_ = reader.readUtf8();
		int count = reader.readInt();

		args = PyTuple_New( count );
		for ( int i = 0; i < count; ++i )
		{
			cVariant variant;
			PyObject *object = 0;

			variant.serialize( reader, version );
			switch ( variant.type() )
			{
			case cVariant::LongType:
			case cVariant::IntType:
				object = PyInt_FromLong( variant.asInt() );
				break;

			case cVariant::StringType:
				object = QString2Python( variant.asString() );
				break;

			case cVariant::DoubleType:
				object = PyFloat_FromDouble( variant.asDouble() );
				break;

			case cVariant::BaseCharType:
				object = PyGetCharObject( variant.toChar() );
				break;

			case cVariant::ItemType:
				object = PyGetItemObject( variant.toItem() );
				break;

			case cVariant::CoordType:
				object = PyGetCoordObject( variant.toCoord() );
				break;
			case cVariant::InvalidType:
				break;
			}

			if ( !object )
			{
				Py_INCREF( Py_None );
				object = Py_None;
			}

			PyTuple_SetItem( args, i, object );
		}
	}

	void save( unsigned int id )
	{
		saveString( id, "functionname",  func ? func->functionPath() : "" );
		saveString( id, "dispelfunc", dispelFunc_ ? dispelFunc_->functionPath() : "" );
		saveString( id, "dispelid", dispelId_ );
		saveInt( id, "pycount", PyTuple_Size( args ) );

		// Serialize the py object
		for ( int i = 0; i < PyTuple_Size( args ); ++i )
		{
			PyObject* object = PyTuple_GetItem( args, i );
			QString name = "pyarg_" + QString::number( i );
			if ( PyInt_Check( object ) )
			{
				saveInt( id, name, ( int ) PyInt_AsLong( object ) );
			}
			else if ( PyString_Check( object ) )
			{
				saveString( id, name, PyString_AsString( object ) );
			}
			else if ( PyFloat_Check( object ) )
			{
				saveFloat( id, name, PyFloat_AsDouble( object ) );
			}
			else if ( checkWpChar( object ) )
			{
				saveChar( id, name, getWpChar( object ) );
			}
			else if ( checkWpItem( object ) )
			{
				saveItem( id, name, getWpItem( object ) );
			}
		}

		cTimer::save( id );
	}

	void load( unsigned int id, QSqlQuery& result )
	{
		// Load the Base Properties and then Select
		QString value;
		loadString( id, "functionname", value );
		func = new PythonFunction( value );
		loadString( id, "dispelfunc", value );
		dispelFunc_ = new PythonFunction( value );
		loadString( id, "dispelid", dispelId_ );

		int count;
		loadInt( id, "pycount", count );

		args = PyTuple_New( count );

		QSqlQuery res( QString( "SELECT keyname,type,value FROM effects_properties WHERE id = %1 AND keyname LIKE 'pyarg_%'" ).arg( id ) );

		while ( res.next() )
		{
			QString key = res.value( 0 ).toString();
			QString type = res.value( 1 ).toString();
			QString value = res.value( 2 ).toString();

			int id = key.right( key.length() - 6 ).toInt();

			if ( id >= count )
				continue;

			if ( type == "string" )
			{
				PyTuple_SetItem( args, id, PyString_FromString( value.toLatin1() ) );
			}
			else if ( type == "int" )
			{
				PyTuple_SetItem( args, id, PyInt_FromLong( value.toInt() ) );
			}
			else if ( type == "float" )
			{
				PyTuple_SetItem( args, id, PyFloat_FromDouble( value.toFloat() ) );
			}
			else if ( type == "char" )
			{
				PyTuple_SetItem( args, id, PyGetCharObject( World::instance()->findChar( value.toInt() ) ) );
			}
			else if ( type == "item" )
			{
				PyTuple_SetItem( args, id, PyGetItemObject( World::instance()->findItem( value.toInt() ) ) );
			}
		}

		cTimer::load( id, result );
	}
};

#endif
