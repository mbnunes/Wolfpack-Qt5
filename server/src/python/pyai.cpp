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

#include "engine.h"

#include "../ai/ai.h"
#include "../basechar.h"
#include "../player.h"
#include "../singleton.h"

#include "utilities.h"
#include "content.h"
#include "tempeffect.h"
#include "objectcache.h"

/*!
	The object for Wolfpack Python items
*/
struct wpAI
{
	PyObject_HEAD;
	AbstractAI* pAI;
};

// Forward Declarations
static PyObject* wpAI_getAttr( wpAI* self, char* name );
static int wpAI_setAttr( wpAI* self, char* name, PyObject* value );
int wpAI_compare( PyObject* a, PyObject* b );

/*!
	The typedef for Wolfpack Python items
*/
static PyTypeObject wpAIType =
{
	PyObject_HEAD_INIT( &PyType_Type )
	0, "wpai", sizeof( wpAIType ), 0,
	//FreeItemObject,
	wpDealloc, 0, ( getattrfunc ) wpAI_getAttr, ( setattrfunc ) wpAI_setAttr, wpAI_compare, 0, 0, 0, 0, 0
};

PyObject* PyGetAIObject( AbstractAI* ai )
{
	if ( !ai )
	{
		Py_INCREF( Py_None );
		return Py_None;
	}

	wpAI* returnVal = PyObject_New( wpAI, &wpAIType );
	returnVal->pAI = ai;
	return ( PyObject * ) returnVal;
}

// Method declarations

static PyObject* wpAI_onSpeechInput( wpAI* self, PyObject* args )
{
	if ( !checkArgChar( 0 ) )
		return 0;
	if ( !checkArgStr( 1 ) )
		return 0;
	P_CHAR pc = getArgChar( 0 );

	P_PLAYER player = dynamic_cast<P_PLAYER>( pc );
	if ( !player )
		return PyFalse();

	QString str = getArgStr( 1 );
	self->pAI->onSpeechInput( player, str.upper() );

	return PyTrue();
}

static PyMethodDef wpAIMethods[] =
{
	{ "onSpeechInput",			( getattrofunc ) wpAI_onSpeechInput, METH_VARARGS, "Executes the onSpeechInput event" }, { NULL, NULL, 0, NULL }
};

// Getters + Setters

static PyObject* wpAI_getAttr( wpAI* self, char* name )
{
	// Special Python things

	return Py_FindMethod( wpAIMethods, ( PyObject * ) self, name );
}

static int wpAI_setAttr( wpAI* self, char* name, PyObject* value )
{
	// Special Python things.

	return 0;
}

AbstractAI* getWpAI( PyObject* pObj )
{
	if ( pObj->ob_type != &wpAIType )
		return 0;

	wpAI* item = ( wpAI* ) ( pObj );
	return item->pAI;
}

bool checkWpAI( PyObject* pObj )
{
	if ( pObj->ob_type != &wpAIType )
		return false;
	else
		return true;
}

int wpAI_compare( PyObject* a, PyObject* b )
{
	// Both have to be characters
	if ( a->ob_type != &wpAIType || b->ob_type != &wpAIType )
		return -1;

	AbstractAI* pA = getWpAI( a );
	AbstractAI* pB = getWpAI( b );

	return !( pA == pB );
}
