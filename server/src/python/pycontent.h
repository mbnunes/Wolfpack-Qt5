/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2017 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: https://github.com/Mutilador/Wolfpack
 */

#ifndef __CONTENT_H__
#define __CONTENT_H__

#include "../defines.h"

#include "../content.h"
#include "../items.h"
#include "../world.h"
#include "../basechar.h"
#include "utilities.h"

typedef struct
{
	PyObject_HEAD;
	SERIAL contserial;
} wpContent;

static Py_ssize_t wpContent_length( PyObject* self_obj )
{
	wpContent* self = ( wpContent* ) self_obj;
	P_ITEM pi = FindItemBySerial( self->contserial );
	if ( pi )
		return pi->content().count();
	else
	{
		P_CHAR pc = FindCharBySerial( self->contserial );
		if ( pc )
			return pc->content().size();
	}
	return 0;
}

static PyObject* wpContent_get( wpContent* self, Py_ssize_t id )
{
	if ( isCharSerial( self->contserial ) )
	{
		P_CHAR pc = FindCharBySerial( self->contserial );
		if ( !pc )
			goto error;
		cBaseChar::ItemContainer container = pc->content();
		if ( id < 0 || static_cast<size_t>( id ) >= container.size() )
			goto error;
		cBaseChar::ItemContainer::const_iterator it( container.begin() );
		/*
				 * Ask Correa before trying to `optimize` this,
				 * there isn't much standard complient options here.
				 */
		for ( Py_ssize_t i = 0; i < id && it != container.end(); ++i )
			++it;

		if ( it != container.end() )
			return PyGetItemObject( *it );
		else
			goto error;
	}
	else
	{
		P_ITEM pi = FindItemBySerial( self->contserial );
		if ( !pi )
			goto error;
		const ContainerContent &container = pi->content();
		if ( id < 0 || static_cast<size_t>( id ) >= static_cast<size_t>( container.count() ) )
			goto error;

		ContainerIterator it( container );
		it.seek( id );

		if ( !it.atEnd() )
			return ( *it )->getPyObject();
		else
			goto error;
	}
	error:
	Py_RETURN_NONE;
}

static PySequenceMethods wpContentSequence = {
	.sq_length = wpContent_length,
	.sq_item = ( ssizeargfunc ) wpContent_get,
};

static PyTypeObject wpContentType = {
	PyVarObject_HEAD_INIT( NULL, 0 )
	.tp_name = "wolfpack.content",
	.tp_basicsize = sizeof( wpContent ),
	.tp_dealloc = wpDealloc,
	.tp_as_sequence = &wpContentSequence,
};

#endif
