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

#include "worlditerator.h"
#include "../world.h"

typedef struct {
    PyObject_HEAD;
	cItemIterator *iter;
} wpItemIterator;

static void wpItemIteratorDealloc( PyObject* self )
{
	delete ((wpItemIterator*)self)->iter;
    PyObject_Del( self );
}

static PyObject *wpItemIterator_getAttr( wpItemIterator *self, char *name )
{	
	if( !strcmp( name, "first" ) )
		return PyGetItemObject( self->iter->first() );
	else if( !strcmp( name, "next" ) )
		return PyGetItemObject( self->iter->next() );

	return PyFalse;
}

static PyTypeObject wpItemIteratorType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "wpItemIterator",
    sizeof(wpItemIteratorType),
    0,
	wpItemIteratorDealloc,
    0,
    (getattrfunc)wpItemIterator_getAttr,
    0,
    0,
    0,
    0,
    0,
};

PyObject* PyGetItemIterator()
{
	wpItemIterator *returnVal = PyObject_New( wpItemIterator, &wpItemIteratorType );
	returnVal->iter = new cItemIterator;

	return (PyObject*)returnVal;
}

/*
 *	Character Region Iterator
 */

typedef struct {
    PyObject_HEAD;
	cCharIterator *iter;
} wpCharIterator;

static void wpCharIteratorDealloc( PyObject* self )
{
	delete ((wpCharIterator*)self)->iter;
    PyObject_Del( self );
}

static PyObject *wpCharIterator_getAttr( wpCharIterator *self, char *name )
{	
	if( !strcmp( name, "first" ) )
		return PyGetCharObject( self->iter->first() );
	else if( !strcmp( name, "next" ) )
		return PyGetCharObject( self->iter->next() );

	return PyFalse;
}

static PyTypeObject wpCharIteratorType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "wpIteratorChars",
    sizeof(wpCharIteratorType),
    0,
	wpCharIteratorDealloc,
    0,
    (getattrfunc)wpCharIterator_getAttr,
    0,
    0,
    0,
    0,
    0,
};

PyObject* PyGetCharIterator()
{
	wpCharIterator *returnVal = PyObject_New( wpCharIterator, &wpCharIteratorType );
	returnVal->iter = new cCharIterator;

	return (PyObject*)returnVal;
}

