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

#ifndef __CONTENT_H__
#define __CONTENT_H__

#include "../defines.h"
#include "../accounts.h"
#include "../items.h"
#include "../world.h"
#include "../basechar.h"
#include "utilities.h"

typedef struct
{
	PyObject_HEAD;
	cAccounts::const_iterator it;
} wpAccountsIter;

static PyObject *wpAccountsIter_First(wpAccountsIter *self) {
	self->it = Accounts::instance()->begin();
	return (PyObject*)self;
}

static PyObject *wpAccountsIter_Next(wpAccountsIter *self) {
	if (self->it == Accounts::instance()->end()) {		
		PyErr_SetNone(PyExc_StopIteration);
		return 0;
	} else {		
		return PyGetAccountObject(*(self->it++));
	}
}

static PyTypeObject wpAccountsIterType =
{
PyObject_HEAD_INIT( NULL )
0,
"wpAccountsIter",
sizeof( wpAccountsIterType ),
0,
wpDealloc,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
Py_TPFLAGS_HAVE_ITER,
0,
0,
0,
0,
0,
(getiterfunc)wpAccountsIter_First,
(iternextfunc)wpAccountsIter_Next,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
};

#endif
