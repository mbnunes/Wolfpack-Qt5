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

#ifndef __PYTHONSCRIPT_H__
#define __PYTHONSCRIPT_H__

// Wolfpack Includes
#include "python/engine.h"
#include "python/utilities.h"
#include "typedefs.h"
#include "customtags.h"

// Library Includes
#include <qstring.h>

// Script Based Events
enum ePythonEvent
{
	EVENT_USE						= 0,
	EVENT_SINGLECLICK,
	EVENT_COLLIDE,
	EVENT_WALK,
	EVENT_CREATE,
	EVENT_TALK,
	EVENT_WARMODETOGGLE,
	EVENT_LOGIN,
	EVENT_LOGOUT,
	EVENT_HELP,
	EVENT_CHAT,
	EVENT_SKILLUSE,
	EVENT_SKILLGAIN,
	EVENT_SHOWPAPERDOLL,
	EVENT_SHOWSKILLGUMP,
	EVENT_DEATH,
	EVENT_SHOWPAPERDOLLNAME,
	EVENT_CONTEXTENTRY,
	EVENT_SHOWTOOLTIP,
	EVENT_CHLEVELCHANGE,
	EVENT_SPEECH,
	EVENT_WEARITEM,
	EVENT_EQUIP,
	EVENT_UNEQUIP,
	EVENT_DROPONCHAR,
	EVENT_DROPONITEM,
	EVENT_DROPONGROUND,
	EVENT_PICKUP,
	EVENT_DAMAGE,
	EVENT_CASTSPELL,
	EVENT_TRADE,
	EVENT_TRADESTART,
	EVENT_DELETE,
	EVENT_SWING,
	EVENT_SHOWSTATUS,
	EVENT_CHANGEREGION,
	EVENT_ATTACH,
	EVENT_DETACH,
	EVENT_TIMECHANGE,
	EVENT_DISPEL,
	EVENT_TELEKINESIS,
	EVENT_CONTEXTCHECKVISIBLE,
	EVENT_CONTEXTCHECKENABLED,
	EVENT_SPAWN,
	EVENT_COUNT,
};

class cElement;
class cTerritory;
class cAccount;

class cPythonScript
{
protected:
	QCString name_; // Important!
	bool loaded;
	PyObject* codeModule; // This object stores the compiled Python Module
	PyObject* events[EVENT_COUNT];

public:
	cPythonScript();
	~cPythonScript();

	// We need an identification value for the scripts
	void setName( const QCString& value )
	{
		name_ = value;
	}
	const QCString &name() const
	{
		return name_;
	}

	bool load( const QCString& name );
	void unload( void );
	bool isLoaded() const;

	static bool canChainHandleEvent( ePythonEvent event, cPythonScript** chain );
	static bool callChainedEventHandler( ePythonEvent, cPythonScript** chain, PyObject* args = 0 );
	static PyObject* callChainedEvent( ePythonEvent, cPythonScript** chain, PyObject* args = 0 );

	PyObject* callEvent( ePythonEvent, PyObject* args = 0, bool ignoreErrors = false );
	PyObject* callEvent( const QString& event, PyObject* args = 0, bool ignoreErrors = false );
	bool callEventHandler( ePythonEvent, PyObject* args = 0, bool ignoreErrors = false );
	bool callEventHandler( const QString& event, PyObject* args = 0, bool ignoreErrors = false );

	bool canHandleEvent( const QString& event );
	bool canHandleEvent( ePythonEvent event )
	{
		if ( event >= EVENT_COUNT )
			return false;

		return events[event] != 0;
	}
};

struct stError
{
	signed char code;
	QString text;
};

#define PROPERTY_ERROR(errno, data) { stError *errRet = new stError; errRet->code = errno; errRet->text = data; return errRet; }

#define SET_STR_PROPERTY( id, setter ) if( name == id ) {\
	QString text = value.toString(); \
	if( text == QString::null )	\
		PROPERTY_ERROR( -2, "String expected" ) \
	setter = text; \
	return 0; \
	}

#define SET_INT_PROPERTY( id, setter ) if( name == id ) {\
	bool ok; \
	int data = value.toInt( &ok ); \
	if( !ok ) \
		PROPERTY_ERROR( -2, "Integer expected" ) \
	setter = data; \
	return 0; \
	}

#define SET_FLOAT_PROPERTY( id, setter ) if( name == id ) {\
	bool ok; \
	float data = static_cast<QString>( value.toString() ).toFloat( &ok ); \
	if( !ok ) \
		PROPERTY_ERROR( -2, "Float expected" ) \
	setter = data; \
	return 0; \
	}

#define SET_BOOL_PROPERTY( id, setter ) if( name == id ) {\
	bool ok; \
	Q_INT32 data = value.toInt( &ok ); \
	if( !ok ) \
		PROPERTY_ERROR( -2, "Boolean expected" ) \
	setter = data == 0 ? false : true; \
	return 0; \
	}

#define SET_CHAR_PROPERTY( id, setter ) if( name == id ) {\
	setter = value.toChar(); \
	return 0; \
	}

#define SET_ITEM_PROPERTY( id, setter ) if( name == id ) {\
	setter = value.toItem(); \
	return 0; \
	}

/*!
	\brief A base class for object scriptable trough the python interface.
	This class provides comfortable methods to create python objects out of
	objects.
*/
class cPythonScriptable
{
public:

	virtual ~cPythonScriptable()
	{
	}

	/*!
		\brief Used to identify a specific object type.
		\returns A pointer to the name of this class.
	*/
	virtual const char* className() const;

	/*!
		\brief Does this class implement a given interface?
		\param name The name of the interface.
		\returns True if this class implements the interface, otherwise false.
	*/
	virtual bool implements( const QString& name ) const;

	/*!
		\brief Create and return a python wrapper object for this object.
		\returns A python object representing this object.
	*/
	virtual PyObject* getPyObject() = 0;

	/*!
		\brief Sets a property for this object.
		This method allows the developer to set properties for this object
		using a variant \a value and the \a name of the property.
		\param name The name of the property.
		\param value The value that should be set.
		\returns If an error occured it returns a pointer to an error structure. The callee is responsible
			for freeing this structure, otherwise a null pointer is returned.
	*/
	virtual stError* setProperty( const QString& name, const cVariant& value );

	/*!
		\brief Gets a property of this object.
		This method gets a property of this object and stores it's value in \a value.
		\param name The name of the property.
		\returns A python object with the value or None.
	*/
	virtual PyObject* getProperty( const QString& name );
	virtual bool setPropety( const QString& name, PyObject* value );

	// Functions for creating python representations of objects
	inline PyObject* createPyObject( cPythonScriptable* object )
	{
		if ( object )
		{
			return object->getPyObject();
		}
		else
		{
			Py_RETURN_NONE;
		}
	}

	inline PyObject* createPyObject( cTerritory* object )
	{
		if ( object )
		{
			return PyGetRegionObject( object );
		}
		else
		{
			Py_RETURN_NONE;
		}
	}

	inline PyObject* createPyObject( cAccount* object )
	{
		if ( object )
		{
			return PyGetAccountObject( object );
		}
		else
		{
			Py_RETURN_NONE;
		}
	}

	inline PyObject* createPyObject( int value )
	{
		return PyInt_FromLong( value );
	}

	inline PyObject* createPyObject( bool value )
	{
		if ( value )
		{
			Py_RETURN_TRUE;
		}
		else
		{
			Py_RETURN_FALSE;
		}
	}

	inline PyObject* createPyObject( const Coord_cl& pos )
	{
		return PyGetCoordObject( pos );
	}

	inline PyObject* createPyObject( unsigned int value )
	{
		return PyInt_FromLong( value );
	}

	inline PyObject* createPyObject( const QCString& value )
	{
		return QString2Python( value.data() );
	}

	inline PyObject* createPyObject( const QString& value )
	{
		return QString2Python( value );
	}

	inline PyObject* createPyObject( double value )
	{
		return PyFloat_FromDouble( value );
	}

	// Method for converting python objects into normal objects
	// Functions for creating python representations of objects
	bool convertPyObject( PyObject* object, P_CHAR& pChar );
	bool convertPyObject( PyObject* object, P_ITEM& pItem );
	bool convertPyObject( PyObject* object, Coord_cl& pos );
	bool convertPyObject( PyObject* object, QString& string );
	bool convertPyObject( PyObject* object, QCString& string );
	bool convertPyObject( PyObject* object, unsigned int& data );
	bool convertPyObject( PyObject* object, int& data );

#define PY_PROPERTY(namestr, getter) if ((namestr == name)) { \
		return createPyObject((getter)); \
	}
};

#endif
