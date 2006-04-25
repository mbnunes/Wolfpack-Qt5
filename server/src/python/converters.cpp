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

#include <boost/python.hpp>

#include <QByteArray>
#include <QString>

using namespace boost::python;

struct QByteArray_to_python_str
{
	static PyObject *convert( QByteArray const& str )
	{
		if ( str.isEmpty() )
			return PyString_FromString("");
		return PyString_FromStringAndSize( str.data(), str.size() );
	}
};

struct QByteArray_from_python_str
{
	QByteArray_from_python_str()
	{
		converter::registry::push_back(&convertible, &construct, type_id<QByteArray>());
	}

	static void *convertible(PyObject *obj)
	{
		if ( PyString_Check( obj ) )
			return obj;
		return 0;
	}

	static void construct(PyObject *obj, converter::rvalue_from_python_stage1_data *data)
	{
		if ( PyString_Check( obj ) )
		{
			const char *value = PyString_AS_STRING(obj);
			if( !value ) 
				throw_error_already_set();
			void *storage=((converter::rvalue_from_python_storage<QString> *) data)->storage.bytes;
			new(storage) QByteArray(value, PyString_GET_SIZE(obj));
			data->convertible = storage;
		}
	}


};

struct QString_to_python_str
{
	static PyObject *convert( QString const& str )
	{
#if defined( Py_UNICODE_WIDE )
		PyObject *uobj;

		if ( ( uobj = PyUnicode_FromUnicode( 0, str.length() ) ) == 0 )
			throw_error_already_set();

		Py_UNICODE *pyu = PyUnicode_AS_UNICODE( uobj );

		for (int i = 0; i < str.length(); ++i)
			*pyu++ = (str.at(i)).unicode();

		return uobj;
#else
		return PyUnicode_FromWideChar( (const wchar_t*) str.unicode(), str.length() );
#endif
	}
};

struct QString_from_python_str
{
	QString_from_python_str()
	{
		converter::registry::push_back(&convertible, &construct, type_id<QString>());
	}

	static void *convertible(PyObject *obj)
	{
		if ( PyUnicode_Check( obj ) )
			return obj;
		if ( PyString_Check( obj ) )
			return obj;
		return 0;
	}

	static void construct(PyObject *obj, converter::rvalue_from_python_stage1_data *data)
	{
		if ( PyUnicode_Check( obj ) )
		{
			void *storage=((converter::rvalue_from_python_storage<QString> *) data)->storage.bytes;
			//QString* result = new(storage) QString;
			
#if defined(Py_UNICODE_WIDE)
			PY_UNICODE_TYPE *ucode = PyUnicode_AS_UNICODE( obj );
			int len = PyUnicode_GET_SIZE( obj );
			QString* out = new(storage) QString;
			for ( int i = 0; i < len; ++i )
				out->append( (uint)ucode[i] );
#else
			new(storage) QString( (const QChar *)PyUnicode_AS_UNICODE( obj ), PyUnicode_GET_SIZE(obj) );
#endif
			data->convertible = storage;
		}
		else if ( PyString_Check( obj ) )
		{
			const char *value=PyString_AsString(obj);
			if( !value ) 
				throw_error_already_set();
			void *storage=((converter::rvalue_from_python_storage<QString> *) data)->storage.bytes;
			new(storage) QString(QByteArray(value, PyString_Size(obj)));
			data->convertible = storage;
		}
	}
};


void registerConverters()
{
	to_python_converter<QByteArray, QByteArray_to_python_str>();
	QByteArray_from_python_str();
	to_python_converter<QString, QString_to_python_str>();
	QString_from_python_str();
}