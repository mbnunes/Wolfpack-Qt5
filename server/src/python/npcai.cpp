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

#include "utilities.h"

#include "../ai.h"
#include "../player.h"

/*!
	Struct for WP Python AI
*/
typedef struct {
    PyObject_HEAD;
	cNPC_AI* pAI;
} wpAI;

PyObject *wpAI_getAttr( wpAI *self, char *name );
int wpAI_setAttr( wpAI *self, char *name, PyObject *value );

/*!
	The typedef for Wolfpack Python AI
*/
static PyTypeObject wpAIType = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,
    "wpai",
    sizeof(wpAIType),
    0,
    wpDealloc,
    0,
    (getattrfunc)wpAI_getAttr,
    (setattrfunc)wpAI_setAttr,
};

PyObject* PyGetAIObject( cNPC_AI* pAI )
{
	if( !pAI )
		return Py_None;

	wpAI *returnVal = PyObject_New( wpAI, &wpAIType );
	returnVal->pAI = pAI;
	return (PyObject*)returnVal;
}

PyObject* wpAI_attacked( wpAI* self, PyObject* args )
{
	if( !self->pAI || !self->pAI->currState() )
		return PyFalse;

	self->pAI->currState()->attacked();
	self->pAI->updateState();
	return PyTrue;
}

PyObject* wpAI_won( wpAI* self, PyObject* args )
{
	if( !self->pAI || !self->pAI->currState() )
		return PyFalse;

	self->pAI->currState()->won();
	self->pAI->updateState();
	return PyTrue;
}

PyObject* wpAI_combatcancelled( wpAI* self, PyObject* args )
{
	if( !self->pAI || !self->pAI->currState() )
		return PyFalse;

	self->pAI->currState()->combatCancelled();
	self->pAI->updateState();
	return PyTrue;
}

PyObject* wpAI_hitpointscritical( wpAI* self, PyObject* args )
{
	if( !self->pAI || !self->pAI->currState() )
		return PyFalse;

	self->pAI->currState()->hitpointsCritical();
	self->pAI->updateState();
	return PyTrue;
}

PyObject* wpAI_hitpointsrestored( wpAI* self, PyObject* args )
{
	if( !self->pAI || !self->pAI->currState() )
		return PyFalse;

	self->pAI->currState()->hitpointsRestored();
	self->pAI->updateState();
	return PyTrue;
}

PyObject* wpAI_speechinput( wpAI* self, PyObject* args )
{
	if( !self->pAI || !self->pAI->currState() )
		return PyFalse;

	if( PyTuple_Size( args ) != 2 || !checkArgChar( 0 ) || !checkArgStr( 1 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	P_PLAYER pp = dynamic_cast< P_PLAYER >( getArgChar( 0 ) );
	if( !pp )
	{
		PyErr_BadArgument();
		return NULL;
	}

	self->pAI->currState()->speechInput( pp, getArgStr( 1 ) );
	self->pAI->updateState();
	return PyTrue;
}

PyObject* wpAI_foundvictim( wpAI* self, PyObject* args )
{
	if( !self->pAI || !self->pAI->currState() )
		return PyFalse;

	if( PyTuple_Size( args ) != 1 || !checkArgChar( 0 ) )
	{
		PyErr_BadArgument();
		return NULL;
	}

	self->pAI->currState()->foundVictim( getArgChar( 0 ) );
	self->pAI->updateState();
	return PyTrue;
}

PyObject* wpAI_refresh( wpAI* self, PyObject* args )
{
	if( !self->pAI || !self->pAI->currState() )
		return PyFalse;

	self->pAI->currState()->refresh();
	self->pAI->updateState();
	return PyTrue;
}

PyObject* wpAI_tameattempt( wpAI* self, PyObject* args )
{
	if( !self->pAI || !self->pAI->currState() )
		return PyFalse;

	self->pAI->currState()->tameAttempt();
	self->pAI->updateState();
	return PyTrue;
}


static PyMethodDef wpAIMethods[] = 
{
	// events
	{ "attacked",			(getattrofunc)wpAI_attacked, METH_VARARGS, "" },
	{ "won",				(getattrofunc)wpAI_won, METH_VARARGS, "" },
	{ "combatcancelled",	(getattrofunc)wpAI_combatcancelled, METH_VARARGS, "" },
	{ "hitpointscritical",	(getattrofunc)wpAI_hitpointscritical, METH_VARARGS, "" },
	{ "hitpointsrestored",	(getattrofunc)wpAI_hitpointsrestored, METH_VARARGS, "" },
	{ "speechinput",		(getattrofunc)wpAI_speechinput, METH_VARARGS, "" },
	{ "foundvictim",		(getattrofunc)wpAI_foundvictim, METH_VARARGS, "" },
	{ "refresh",			(getattrofunc)wpAI_refresh, METH_VARARGS, "" },
	{ "tameattempt",		(getattrofunc)wpAI_tameattempt, METH_VARARGS, "" },
    { NULL, NULL, 0, NULL }
};

// Getters & Setters
PyObject *wpAI_getAttr( wpAI *self, char *name )
{
	// Python specific stuff
	if ( !strcmp( "name", name ) )
	{
		if( self->pAI )
			return PyString_FromString( self->pAI->AIType().latin1() );
	}
	else if( !strcmp( "state", name ) )
	{
		if( self->pAI && self->pAI->currState() )
			return PyString_FromString( self->pAI->currState()->stateType().latin1() );
	}

	// If no property is found search for a method
	return Py_FindMethod( wpAIMethods, (PyObject*)self, name );
}

int wpAI_setAttr( wpAI *self, char *name, PyObject *value )
{
	// Special Python things.
	if( !strcmp( "state", name ) && PyString_Check( value ) )
	{
		AbstractState* pState = StateFactory::instance()->createObject( PyString_AsString( value ) );
		if( pState && self->pAI )
			self->pAI->updateState( pState );
	}

	return 0;
}

cNPC_AI* getWpAI( PyObject *pObj )
{
	if( pObj->ob_type != &wpAIType )
		return 0;

	wpAI *item = (wpAI*)( pObj );
	return item->pAI;
}

bool checkWpAI( PyObject *pObj )
{
	if( pObj->ob_type != &wpAIType )
		return false;
	else
		return true;
}
