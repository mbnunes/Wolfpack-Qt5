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

#ifndef __REGIONITERATOR_H__
#define __REGIONITERATOR_H__

#include "../defines.h"
#include "utilities.h"
#include "../mapobjects.h"

typedef struct {
    PyObject_HEAD;
	std::vector< SERIAL > *serials;
	std::vector< SERIAL >::const_iterator *it;
	unsigned short x1,y2,x2,y1;
} wpRegionIteratorItems;

static void wpIteratorDealloc( PyObject* self )
{
	delete ((wpRegionIteratorItems*)self)->serials;
	delete ((wpRegionIteratorItems*)self)->it;
    PyObject_Del( self );
}

static PyObject *wpRegionIteratorItems_getAttr( wpRegionIteratorItems *self, char *name )
{	
	if( !strcmp( name, "first" ) )
	{
		*(self->it) = self->serials->begin();

        // Find the next item
		P_ITEM pItem = 0;

		while( !pItem && *(self->it) != self->serials->end() )
		{
			if( isItemSerial( *(*(self->it)) ) )
			{
				pItem = FindItemBySerial( *( *( self->it ) ) );
				
				if( pItem && ( pItem->pos().x < self->x1 || pItem->pos().x > self->x2 || pItem->pos().y < self->y1 || pItem->pos().y > self->y2 ) )
					pItem = NULL;
			}

			(*(self->it))++;
		}

		return PyGetItemObject( pItem );
	}
	else if( !strcmp( name, "next" ) )
	{
        // Find the next item
		P_ITEM pItem = 0;

		while( !pItem && *(self->it) != self->serials->end() )
		{
			if( isItemSerial( *(*(self->it)) ) )
			{
				pItem = FindItemBySerial( *( *( self->it ) ) );
				
				if( pItem && ( pItem->pos().x < self->x1 || pItem->pos().x > self->x2 || pItem->pos().y < self->y1 || pItem->pos().y > self->y2 ) )
					pItem = NULL;
			}

			(*(self->it))++;
		}

		return PyGetItemObject( pItem );
	}

	return PyFalse;
}

static PyTypeObject wpRegionIteratorItemsType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "wpRegionIteratorItems",
    sizeof(wpRegionIteratorItemsType),
    0,
	wpIteratorDealloc,
    0,
    (getattrfunc)wpRegionIteratorItems_getAttr,
    0,
    0,
    0,
    0,
    0,
};

static PyObject *PyGetRegionIterator( unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned char map )
{
	wpRegionIteratorItems *returnVal = PyObject_New( wpRegionIteratorItems, &wpRegionIteratorItemsType );
	returnVal->ob_type = &wpRegionIteratorItemsType;
	returnVal->serials = new std::vector< SERIAL >;
	returnVal->it = new std::vector< SERIAL >::const_iterator;

	MapObjects::instance()->search( x1, y1, x2, y2, map, *(returnVal->serials) );
	returnVal->x1 = x1;
	returnVal->x2 = x2;
	returnVal->y1 = y1;
	returnVal->y2 = y2;

	return (PyObject*)returnVal;
}

#endif
