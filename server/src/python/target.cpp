
#include "target.h"

static PyObject* wpTarget_getAttr( wpTarget* self, char* name )
{
	/*
		\rproperty target.pos The value of this property is a <object id="coord">coord</object> object representing the location of the targetted object or ground tile.
		Remember that this value should not be trusted.
	*/
	if ( !strcmp( name, "pos" ) )
	{
		if ( !self->py_pos )
			self->py_pos = PyGetCoordObject( self->pos );

		Py_INCREF( self->py_pos );
		return self->py_pos;
	}
	/*
		\rproperty target.model If a static tile has been targetted by the client, this property contains the art id of the targetted static tile. If a character has been targetted,
		this property contains the body id of the targetted character. Please note that this value is sent by the client and not determined by the server.
	*/
	else if ( !strcmp( name, "model" ) )
	{
		return PyInt_FromLong( self->model );
	}
	/*
		\rproperty target.item If a valid item has been targetted, this property contains an <object id="item">item</object> object for the targetted item.
	*/
	else if ( !strcmp( name, "item" ) )
	{
		if ( isItemSerial( self->object ) )
		{
			if ( !self->py_obj )
				self->py_obj = PyGetItemObject( FindItemBySerial( self->object ) );

			Py_INCREF( self->py_obj );
			return self->py_obj;
		}
	}
	/*
		\rproperty target.char If a valid character has been targetted, this property contains a <object id="char">char</object> object for the targetted character.
	*/
	else if ( !strcmp( name, "char" ) )
	{
		if ( isCharSerial( self->object ) )
		{
			if ( !self->py_obj )
				self->py_obj = PyGetCharObject( FindCharBySerial( self->object ) );

			Py_INCREF( self->py_obj );
			return self->py_obj;
		}
	}
	Py_RETURN_FALSE;
}


static void wpTarget_Dealloc( wpTarget * self )
{
	Py_XDECREF( self->py_pos	);
	Py_XDECREF( self->py_obj	);

	wpDealloc( (PyObject*) self );
}


PyTypeObject wpTargetType =
{
PyObject_HEAD_INIT( NULL )
0,
"wpTarget",
sizeof( wpTargetType ),
0,
( destructor )  wpTarget_Dealloc,
0,
( getattrfunc ) wpTarget_getAttr,
0,
0,
0,
0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

};


static PyObject* PyGetTarget( cUORxTarget* target, Q_UINT8 map )
{
	if ( !target )
	{
		Py_RETURN_NONE;
	}

	wpTarget* returnVal = PyObject_New( wpTarget, &wpTargetType );

	Coord_cl pos;
	pos.x = target->x();
	pos.y = target->y();
	pos.z = target->z();
	pos.map = map;

	returnVal->pos			= pos;
	returnVal->object		= target->serial();
	returnVal->model		= target->model();

	returnVal->py_pos		= NULL;
	returnVal->py_obj		= NULL;

	return ( PyObject * ) returnVal;
}

bool cPythonTarget::responsed( cUOSocket* socket, cUORxTarget* target )
{
	// Try to call the python function
	// Get everything before the last dot
	if ( responsefunc.contains( "." ) )
	{
		// Find the last dot
		int position = responsefunc.findRev( "." );
		QString sModule = responsefunc.left( position );
		QString sFunction = responsefunc.right( responsefunc.length() - ( position + 1 ) );

		PyObject* pModule = PyImport_ImportModule( const_cast<char*>( sModule.latin1() ) );

		if ( pModule )
		{
			PyObject* pFunc = PyObject_GetAttrString( pModule, const_cast<char*>( sFunction.latin1() ) );

			if ( pFunc && PyCallable_Check( pFunc ) )
			{
				// Create our Argument list
				PyObject* p_args = PyTuple_New( 3 );
				PyTuple_SetItem( p_args, 0, PyGetCharObject( socket->player() ) );
				Py_INCREF(args);
				PyTuple_SetItem( p_args, 1, args );
				PyTuple_SetItem( p_args, 2, PyGetTarget( target, socket->player()->pos().map ) );

				PyObject* result = PyEval_CallObject( pFunc, p_args );
				Py_XDECREF( result );
				reportPythonError( sModule );
			}

			Py_XDECREF( pFunc );
		}

		Py_XDECREF( pModule );
	}
	return true;
}

void cPythonTarget::timedout( cUOSocket* socket )
{
	if ( !timeoutfunc.isNull() && !timeoutfunc.isEmpty() && timeoutfunc.contains( "." ) )
	{
		// Find the last dot
		int position = timeoutfunc.findRev( "." );
		QString sModule = timeoutfunc.left( position );
		QString sFunction = timeoutfunc.right( timeoutfunc.length() - ( position + 1 ) );

		PyObject* pModule = PyImport_ImportModule( const_cast<char*>( sModule.latin1() ) );

		if ( pModule )
		{
			PyObject* pFunc = PyObject_GetAttrString( pModule, const_cast<char*>( sFunction.latin1() ) );

			if ( pFunc && PyCallable_Check( pFunc ) )
			{
				// Create our Argument list
				PyObject* args = PyTuple_New( 1 );
				PyTuple_SetItem( args, 0, PyGetCharObject( socket->player() ) );

				PyObject* result = PyEval_CallObject( pFunc, args );
				Py_XDECREF( result );
				reportPythonError( sModule );
			}

			Py_XDECREF( pFunc );
		}

		Py_XDECREF( pModule );
	}
}

void cPythonTarget::canceled( cUOSocket* socket )
{
	if ( !cancelfunc.isNull() && !cancelfunc.isEmpty() && cancelfunc.contains( "." ) )
	{
		// Find the last dot
		int position = cancelfunc.findRev( "." );
		QString sModule = cancelfunc.left( position );
		QString sFunction = cancelfunc.right( cancelfunc.length() - ( position + 1 ) );

		PyObject* pModule = PyImport_ImportModule( const_cast<char*>( sModule.latin1() ) );

		if ( pModule )
		{
			PyObject* pFunc = PyObject_GetAttrString( pModule, const_cast<char*>( sFunction.latin1() ) );

			if ( pFunc && PyCallable_Check( pFunc ) )
			{
				// Create our Argument list
				PyObject* args = PyTuple_New( 1 );
				PyTuple_SetItem( args, 0, PyGetCharObject( socket->player() ) );

				PyObject* result = PyEval_CallObject( pFunc, args );
				Py_XDECREF( result );

				reportPythonError( sModule );
			}

			Py_XDECREF( pFunc );
		}

		Py_XDECREF( pModule );
	}
}
