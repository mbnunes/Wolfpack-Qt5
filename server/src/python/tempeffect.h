//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
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

#if !defined(__TEMPEFFECT_H__)
#define __TEMPEFFECT_H__

#include "../TmpEff.h"
#include "../world.h"

class cPythonEffect : public cTempEffect
{
protected:
	QString functionName;
	QString dispelId_, dispelFunc_;
	PyObject *args;
public:
	cPythonEffect() { objectid = "cPythonEffect"; }
	
	cPythonEffect( const QString &_functionName, PyObject *_args ): 
		functionName( _functionName ), args( _args ) 
		{
			objectid = "cPythonEffect"; 
			Py_INCREF( args );
		}
	
	virtual ~cPythonEffect() {;}
	
	void setDispelId( const QString &data ) { dispelId_ = data; }
	QString dispelId() { return dispelId_; }

	void setDispelFunc( const QString &data ) { dispelFunc_ = data; }
	QString dispelFunc() { return dispelFunc_; }

	void Dispel( P_CHAR pSource, bool silent )
	{
		// We will ignore silent here.
		Dispel( pSource, PyList_New( 0 ) );
	}

	// Dispel args: char, [args], source, [args] (while the last one is optional)
	void Dispel( P_CHAR pSource, PyObject *disp_args )
	{
		if( dispelFunc_.isNull() )
			return;

		// Get everything before the last dot
		if( dispelFunc_.contains( "." ) )
		{
			// Find the last dot
			INT32 position = dispelFunc_.findRev( "." );
			QString sModule = dispelFunc_.left( position );
			QString sFunction = dispelFunc_.right( dispelFunc_.length() - (position+1) );

			PyObject *pModule = PyImport_ImportModule( const_cast< char* >( sModule.latin1() ) );

			if( pModule )
			{
				PyObject *pFunc = PyObject_GetAttrString( pModule, const_cast< char* >( sFunction.latin1() ) );
				if( pFunc && PyCallable_Check( pFunc ) )
				{
					// Create our Argument list
					PyObject *p_args = PyTuple_New( 4 );
					if( isItemSerial( destSer ) )
						PyTuple_SetItem( p_args, 0, PyGetItemObject( FindItemBySerial( destSer ) ) );
					else if( isCharSerial( destSer ) )
						PyTuple_SetItem( p_args, 0, PyGetCharObject( FindCharBySerial( destSer ) ) );
					else
						PyTuple_SetItem( p_args, 0, PyFalse );

					PyTuple_SetItem( p_args, 1, args );
					PyTuple_SetItem( p_args, 2, PyGetCharObject( pSource ) );
					PyTuple_SetItem( p_args, 3, disp_args );

					PyEval_CallObject( pFunc, p_args );
					
					if( PyErr_Occurred() )
						PyErr_Print();
				}
			}
		}

		Py_DECREF( args );
	}

	void Expire()
	{
		// Get everything before the last dot
		if( functionName.contains( "." ) )
		{
			// Find the last dot
			INT32 position = functionName.findRev( "." );
			QString sModule = functionName.left( position );
			QString sFunction = functionName.right( functionName.length() - (position+1) );

			PyObject *pModule = PyImport_ImportModule( const_cast< char* >( sModule.latin1() ) );

			if( pModule )
			{
				PyObject *pFunc = PyObject_GetAttrString( pModule, const_cast< char* >( sFunction.latin1() ) );
				if( pFunc && PyCallable_Check( pFunc ) )
				{
					// Create our Argument list
					PyObject *p_args = PyTuple_New( 2 );
					if( isItemSerial( destSer ) )
						PyTuple_SetItem( p_args, 0, PyGetItemObject( FindItemBySerial( destSer ) ) );
					else if( isCharSerial( destSer ) )
						PyTuple_SetItem( p_args, 0, PyGetCharObject( FindCharBySerial( destSer ) ) );
					else
						PyTuple_SetItem( p_args, 0, PyFalse );

					PyTuple_SetItem( p_args, 1, args );

					PyEval_CallObject( pFunc, p_args );
					
					if( PyErr_Occurred() )
						PyErr_Print();
				}
			}
		}

		Py_DECREF( args );
	}

	void Serialize( ISerialization &archive )
	{
		if( archive.isReading() )
		{
			archive.read( "functionname",	functionName );
			archive.read( "dispelfunc", dispelFunc_ );
			archive.read( "dispelid", dispelId_ );

			UINT32 pCount;
			QString type;
			archive.read( "pcount", pCount );
			args = PyTuple_New( pCount );

			for( UINT32 i = 0; i < pCount; ++i )
			{
				archive.read( QString( "pt%1" ).arg( i ), type );

				// Read an integer
				if( type == "i" )
				{
					INT32 data;
					archive.read( QString( "pv%1" ).arg( i ), data );
					PyObject *obj = PyInt_FromLong( data );
					PyTuple_SetItem( args, i, PyInt_FromLong( data ) );
				}
				// Read a string
				else if( type == "s" )
				{
					QString data;
					archive.read( QString( "pv%1" ).arg( i ), data );
					PyObject *obj = PyString_FromString( data.latin1() );
					PyTuple_SetItem( args, i, obj );
				}
			}		
		}
		else if( archive.isWritting() )
		{
			archive.write( "functionname",	functionName );
			archive.write( "dispelfunc", dispelFunc_ );
			archive.write( "dispelid",	dispelId_ );

			archive.write( "pcount", PyTuple_Size( args ) );

			// Serialize the py object
			for( UINT32 i = 0; i < PyTuple_Size( args ); ++i )
			{
				if( PyInt_Check( PyTuple_GetItem( args, i ) ) )
				{
					archive.write( QString( "pt%1" ).arg( i ), QString( "i" ) );
					archive.write( QString( "pv%1" ).arg( i ), (int)PyInt_AsLong( PyTuple_GetItem( args, i ) ) );
				}
				else if( PyString_Check( PyTuple_GetItem( args, i ) ) )
				{
					archive.write( QString( "pt%1" ).arg( i ), QString( "s" ) );
					archive.write( QString( "pv%1" ).arg( i ), PyString_AsString( PyTuple_GetItem( args, i ) ) );
				}
				// Something we can't save -> Py_None on load
				else
					archive.write( QString( "pt%1" ).arg( i ), QString( "n" ) );
			}
		}

		cTempEffect::Serialize( archive );
	}
};

#endif