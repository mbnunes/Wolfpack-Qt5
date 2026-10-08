/*
*     Wolfpack Emu (WP)
* UO Server Emulation Program
*
* Copyright 2001-2017 by holders identified in AUTHORS.txt
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
* Wolfpack Homepage: https://github.com/Mutilador/Wolfpack
*/

#include <boost/python.hpp>

#include <QByteArray>
#include <QString>

using namespace boost::python;

struct QByteArray_to_python_bytes
{
	static PyObject *convert( QByteArray const& str )
	{
		return PyBytes_FromStringAndSize( str.constData(), str.size() );
	}
};

struct QByteArray_from_python
{
	QByteArray_from_python()
	{
		converter::registry::push_back( &convertible, &construct, type_id<QByteArray>() );
	}

	static void *convertible( PyObject *obj )
	{
		if ( PyBytes_Check( obj ) || PyUnicode_Check( obj ) )
			return obj;
		return 0;
	}

	static void construct( PyObject *obj, converter::rvalue_from_python_stage1_data *data )
	{
		void *storage = ( ( converter::rvalue_from_python_storage<QByteArray> * ) data )->storage.bytes;
		if ( PyBytes_Check( obj ) )
		{
			const char *value = PyBytes_AS_STRING( obj );
			Py_ssize_t size = PyBytes_GET_SIZE( obj );
			new( storage ) QByteArray( value, size );
			data->convertible = storage;
		}
		else if ( PyUnicode_Check( obj ) )
		{
			Py_ssize_t size = 0;
			const char *utf8 = PyUnicode_AsUTF8AndSize( obj, &size );
			if ( !utf8 )
				throw_error_already_set();
			new( storage ) QByteArray( utf8, size );
			data->convertible = storage;
		}
	}
};

struct QString_to_python_str
{
	static PyObject *convert( QString const& str )
	{
		QByteArray utf8 = str.toUtf8();
		return PyUnicode_FromStringAndSize( utf8.constData(), utf8.size() );
	}
};

struct QString_from_python_str
{
	QString_from_python_str()
	{
		converter::registry::push_back( &convertible, &construct, type_id<QString>() );
	}

	static void *convertible( PyObject *obj )
	{
		if ( PyUnicode_Check( obj ) || PyBytes_Check( obj ) )
			return obj;
		return 0;
	}

	static void construct( PyObject *obj, converter::rvalue_from_python_stage1_data *data )
	{
		void *storage = ( ( converter::rvalue_from_python_storage<QString> * ) data )->storage.bytes;
		if ( PyUnicode_Check( obj ) )
		{
			Py_ssize_t size = 0;
			const char *utf8 = PyUnicode_AsUTF8AndSize( obj, &size );
			if ( !utf8 )
				throw_error_already_set();
			new( storage ) QString( QString::fromUtf8( utf8, size ) );
			data->convertible = storage;
		}
		else if ( PyBytes_Check( obj ) )
		{
			const char *value = PyBytes_AS_STRING( obj );
			Py_ssize_t size = PyBytes_GET_SIZE( obj );
			new( storage ) QString( QString::fromUtf8( value, size ) );
			data->convertible = storage;
		}
	}
};

void registerConverters()
{
	to_python_converter<QByteArray, QByteArray_to_python_bytes>();
	QByteArray_from_python();
	to_python_converter<QString, QString_to_python_str>();
	QString_from_python_str();
}
