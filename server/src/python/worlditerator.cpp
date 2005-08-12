/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2005 by holders identified in AUTHORS.txt
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

#include "worlditerator.h"
#include "../world.h"
#include "../definitions.h"

/*
	\object itemiterator
	\description This object type allows you to iterate over all items
	registered in the world in a memory efficient way. A typical iteration could look
	like this:

	<code>iterator = wolfpack.itemiterator()
	item = iterator.first
	while item:
	&nbsp;&nbsp;# Access item properties here
	&nbsp;&nbsp;item = iterator.next</code>
*/
typedef struct
{
	PyObject_HEAD;
	cItemIterator* iter;
} wpItemIterator;

static void wpItemIteratorDealloc( PyObject* self )
{
	delete ( ( wpItemIterator * ) self )->iter;
	PyObject_Del( self );
}

static PyObject* wpItemIterator_getAttr( wpItemIterator* self, char* name )
{
	/*
		\rproperty itemiterator.first Accessing this property will reset the iterator to the first item and return it.
		If there are no items to iterate over, None is returned.
	*/
	if ( !strcmp( name, "first" ) )
		return PyGetItemObject( self->iter->first() );
	/*
		\rproperty itemiterator.next Accessing this property will advance the iterator to the next item and return it.
		At the end of the iteration, None is returned.
	*/
	else if ( !strcmp( name, "next" ) )
		return PyGetItemObject( self->iter->next() );

	Py_RETURN_FALSE;
}

static PyTypeObject wpItemIteratorType =
{
PyObject_HEAD_INIT( NULL )
0,
"wpItemIterator",
sizeof( wpItemIteratorType ),
0,
wpItemIteratorDealloc,
0,
( getattrfunc ) wpItemIterator_getAttr,
0,
0,
0,
0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

};

PyObject* PyGetItemIterator()
{
	wpItemIterator* returnVal = PyObject_New( wpItemIterator, &wpItemIteratorType );
	returnVal->iter = new cItemIterator;

	return ( PyObject * ) returnVal;
}

/*
 *	Character Region Iterator
 */

/*
	\object chariterator
	\description This object type allows you to iterate over all characters
	registered in the world in a memory efficient way. A typical iteration could look
	like this:

	<code>iterator = wolfpack.chariterator()
	char = iterator.first
	while char:
	&nbsp;&nbsp;# Access char properties here
	&nbsp;&nbsp;char = iterator.next</code>
*/
typedef struct
{
	PyObject_HEAD;
	cCharIterator* iter;
} wpCharIterator;

static void wpCharIteratorDealloc( PyObject* self )
{
	delete ( ( wpCharIterator * ) self )->iter;
	PyObject_Del( self );
}

static PyObject* wpCharIterator_getAttr( wpCharIterator* self, char* name )
{
	/*
		\rproperty chariterator.first Accessing this property will reset the iterator to the first character and return it.
		If there are no characters to iterate over, None is returned.
	*/
	if ( !strcmp( name, "first" ) )
		return PyGetCharObject( self->iter->first() );
	/*
		\rproperty chariterator.next Accessing this property will advance the iterator to the next character and return it.
		At the end of the iteration, None is returned.
	*/
	else if ( !strcmp( name, "next" ) )
		return PyGetCharObject( self->iter->next() );

	Py_RETURN_FALSE;
}

static PyTypeObject wpCharIteratorType =
{
PyObject_HEAD_INIT( NULL )
0,
"wpCharIterator",
sizeof( wpCharIteratorType ),
0,
wpCharIteratorDealloc,
0,
( getattrfunc ) wpCharIterator_getAttr,
0,
0,
0,
0,
0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

PyObject* PyGetCharIterator()
{
	wpCharIterator* returnVal = PyObject_New( wpCharIterator, &wpCharIteratorType );
	returnVal->iter = new cCharIterator;

	return ( PyObject * ) returnVal;
}

/*
 *	Definition Iterator
 */

/*
	\object definitioniterator
	\description This object type allows you to iterate over all definitions of a given type like this:

	<code>iterator = wolfpack.definitionsiterator(type)
	node = iterator.first
	while node:
	&nbsp;&nbsp;# Access node properties here
	&nbsp;&nbsp;node = iterator.next</code>
*/
typedef struct
{
	PyObject_HEAD;
	cDefinitions::Iterator *iter;
	eDefCategory type;
} wpDefinitionsIterator;

static void wpDefinitionsIteratorDealloc( PyObject* self )
{
	delete ( ( wpDefinitionsIterator* ) self )->iter;
	PyObject_Del( self );
}

static PyObject* wpDefinitionsIterator_getAttr( wpDefinitionsIterator* self, char* name )
{
	/*
		\rproperty definitioniterator.first Accessing this property will reset the iterator to the first definition and return it.
		If there are no definitions to iterate over, None is returned.
	*/
	if ( !strcmp( name, "first" ) ) {
		delete self->iter;
		self->iter = new cDefinitions::Iterator(Definitions::instance()->begin(self->type));

		if (*(self->iter) == Definitions::instance()->end(self->type)) {
			Py_RETURN_NONE;
		} else {
			PyObject *result = self->iter->data()->getPyObject();
			(*(self->iter))++;
			return result;
		}
	}
	/*
		\rproperty definitioniterator.next Accessing this property will advance the iterator to the next definition and return it.
		At the end of the iteration, None is returned.
	*/
	else if ( !strcmp( name, "next" ) ) {
		if (*(self->iter) == Definitions::instance()->end(self->type)) {
			Py_RETURN_NONE;
		} else {
			PyObject *result = self->iter->data()->getPyObject();
			(*(self->iter))++;
			return result;
		}
	}

	Py_RETURN_FALSE;
}

static PyTypeObject wpDefinitionsIteratorType =
{
PyObject_HEAD_INIT( NULL )
0,
"wpDefinitionsIterator",
sizeof( wpDefinitionsIteratorType ),
0,
wpDefinitionsIteratorDealloc,
0,
( getattrfunc ) wpDefinitionsIterator_getAttr,
0,
0,
0,
0,
0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

PyObject* PyGetDefinitionIterator(eDefCategory type)
{
	wpDefinitionsIterator* returnVal = PyObject_New( wpDefinitionsIterator, &wpDefinitionsIteratorType );
	returnVal->iter = 0;
	returnVal->type = type;
	return ( PyObject * ) returnVal;
}
