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
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#ifndef __TARGET_H__
#define __TARGET_H__

#include "../defines.h"
#include "utilities.h"
#include "../targetrequests.h"

/*
	\object target
	\description This object type represents the clients response to a target request.
*/
typedef struct
{
	PyObject_HEAD;
	Coord pos;
	Q_UINT16 model;
	SERIAL object;

	PyObject *	py_pos;
	PyObject *	py_obj;	// char or item
} wpTarget;

extern PyTypeObject wpTargetType;

class cPythonTarget : public cTargetRequest
{
private:
	QString responsefunc,
	timeoutfunc,
	cancelfunc;
	PyObject* args;
public:
	cPythonTarget( QString _responsefunc, QString _timeoutfunc, QString _cancelfunc, PyObject* _args ) : responsefunc( _responsefunc ), timeoutfunc( _timeoutfunc ), cancelfunc( _cancelfunc ), args( _args )
	{
		Py_INCREF( args );
	}

	virtual ~cPythonTarget()
	{
		Py_XDECREF( args );
	}

	bool responsed( cUOSocket* socket, cUORxTarget* target );
	void timedout( cUOSocket* socket );
	void canceled( cUOSocket* socket );
};


#endif
