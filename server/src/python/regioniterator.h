//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2004 by holders identified in authors.txt
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

#ifndef __REGIONITERATOR_H__
#define __REGIONITERATOR_H__

#include "../defines.h"
#include "utilities.h"
#include "../sectors.h"

typedef struct {
    PyObject_HEAD;
	cItemSectorIterator *iter;
} wpRegionIteratorItems;

static void wpItemIteratorDealloc( PyObject* self )
{
	delete ((wpRegionIteratorItems*)self)->iter;
    PyObject_Del( self );
}

static PyObject *wpRegionIteratorItems_getAttr( wpRegionIteratorItems *self, char *name )
{	
	if( !strcmp( name, "first" ) )
		return PyGetItemObject( self->iter->first() );
	else if( !strcmp( name, "next" ) )
		return PyGetItemObject( self->iter->next() );

	return PyFalse();
}

static PyTypeObject wpRegionIteratorItemsType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "wpRegionIteratorItems",
    sizeof(wpRegionIteratorItemsType),
    0,
	wpItemIteratorDealloc,
    0,
    (getattrfunc)wpRegionIteratorItems_getAttr,
    0,
    0,
    0,
    0,
    0,
};

static PyObject *PyGetItemRegionIterator( unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned char map )
{
	wpRegionIteratorItems *returnVal = PyObject_New( wpRegionIteratorItems, &wpRegionIteratorItemsType );
	returnVal->iter = SectorMaps::instance()->findItems( map, x1, y1, x2, y2 );

	return (PyObject*)returnVal;
}

static PyObject *PyGetItemRegionIterator( unsigned short xBlock, unsigned short yBlock, unsigned char map )
{
	wpRegionIteratorItems *returnVal = PyObject_New( wpRegionIteratorItems, &wpRegionIteratorItemsType );
	returnVal->iter = SectorMaps::instance()->findItems( map, xBlock * 8, yBlock * 8 );
	return (PyObject*)returnVal;
}

/*
 *	Character Region Iterator
 */

typedef struct {
    PyObject_HEAD;
	cCharSectorIterator *iter;
} wpRegionIteratorChars;

static void wpCharIteratorDealloc( PyObject* self )
{
	delete ((wpRegionIteratorChars*)self)->iter;
    PyObject_Del( self );
}

static PyObject *wpRegionIteratorChars_getAttr( wpRegionIteratorChars *self, char *name )
{	
	if( !strcmp( name, "first" ) )
		return PyGetCharObject( self->iter->first() );
	else if( !strcmp( name, "next" ) )
		return PyGetCharObject( self->iter->next() );

	return PyFalse();
}

static PyTypeObject wpRegionIteratorCharsType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "wpRegionIteratorChars",
    sizeof(wpRegionIteratorCharsType),
    0,
	wpCharIteratorDealloc,
    0,
    (getattrfunc)wpRegionIteratorChars_getAttr,
    0,
    0,
    0,
    0,
    0,
};

static PyObject *PyGetCharRegionIterator( unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned char map )
{
	wpRegionIteratorChars *returnVal = PyObject_New( wpRegionIteratorChars, &wpRegionIteratorCharsType );
	returnVal->iter = SectorMaps::instance()->findChars( map, x1, y1, x2, y2 );

	return (PyObject*)returnVal;
}

static PyObject *PyGetCharRegionIterator( unsigned short xBlock, unsigned short yBlock, unsigned char map )
{
	wpRegionIteratorChars *returnVal = PyObject_New( wpRegionIteratorChars, &wpRegionIteratorCharsType );
	returnVal->iter = SectorMaps::instance()->findChars( map, xBlock * 8, yBlock * 8 );
	return (PyObject*)returnVal;
}

#endif
