//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
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
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

#ifndef __CONTENT_H__
#define __CONTENT_H__


#include "../defines.h"
#include "../globals.h"
#include "../junk.h"
#include "utilities.h"

typedef struct {
    PyObject_HEAD;
	SERIAL contserial;
} wpContent;

static int wpContent_length( wpContent *self )
{
	P_ITEM pi = FindItemBySerial(self->contserial);
	if ( pi )
		return pi->content().size();
	else
	{
		P_CHAR pc = FindCharBySerial(self->contserial);
		if ( pc )
			return pc->content().size();
	}
	return 0;
}

static PyObject *wpContent_get( wpContent *self, int id )
{
	if ( isCharSerial(self->contserial) )
	{
		P_CHAR pc = FindCharBySerial( self->contserial );
		if (!pc)
			return Py_None;
		cChar::ContainerContent container = pc->content();
		if ( id >= container.size() || id < 0 )
			return Py_None;
		cChar::ContainerContent::const_iterator it(container.begin());
		for ( uint i = 0; i < id && it != container.end(); ++i )	// Ask Correa before trying 
			++it;													// to 'optimize' this, there 
																	// isn't much standard complient options here
		return it != container.end() ? PyGetItemObject( *it ) : Py_None;

	}
	else
	{
		P_ITEM pi = FindItemBySerial( self->contserial );
		if (!pi)
			return Py_None;
		cItem::ContainerContent container = pi->content();
		if ( id >= container.size() || id < 0 )
			return Py_None;
		cItem::ContainerContent::const_iterator it(container.begin());
		for ( uint i = 0; i < id && it != container.end(); ++i )	// Ask Correa before trying 
			++it;													// to 'optimize' this, there 
																	// isn't much standard complient options here
		return it != container.end() ? PyGetItemObject( *it ) : Py_None;
	}
	return Py_None;
}

static PySequenceMethods wpContentSequence = {
	(inquiry)wpContent_length,
	0,		
	0,		
	(intargfunc)wpContent_get,
	0,
	0,
};

static PyTypeObject wpContentType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "wpContent",
    sizeof(wpContentType),
    0,
	wpDealloc,
    0,
    0,
    0,
    0,
    0,
    0,
    &wpContentSequence,
};

#endif
