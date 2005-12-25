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


#include "pyprofiler.h"
#include <QByteArray>

int PyProfiler::tracefunc( PyObject *obj, PyFrameObject *frame, int what, PyObject *arg )
{
	PyProfiler* that = PyProfiler::instance();
	ProfileData* pdata = 0; 
	switch ( what )
	{
	case PyTrace_CALL:
		//that->data.find()
		// Don't count recursions
		pdata = &that->data[PyString_AS_STRING(frame->f_code->co_filename)][PyString_AS_STRING(frame->f_code->co_name)];	
		pdata->firstline = frame->f_code->co_firstlineno;
		if ( frame != frame->f_back )
		{
			pdata->calls++;
			if ( frame->f_back )
				pdata->callees.insert( PyString_AS_STRING( frame->f_back->f_code->co_name ) );
			else
				pdata->callees.insert( tr("<core>").toLocal8Bit() );
		}
		//that->data[PyString_AS_STRING(frame->f_code->co_filename)][PyString_AS_STRING(frame->f_code->co_name)] = pdata;
		
	case PyTrace_RETURN:
	case PyTrace_C_CALL:
	case PyTrace_C_RETURN:
	default:
		break;
	}
	return 0;
}

void PyProfiler::start()
{
	data.clear();
	PyEval_SetProfile( (Py_tracefunc)PyProfiler::tracefunc, 0 );
}

void PyProfiler::stop()
{
	PyEval_SetProfile( 0, 0 );
	emit stopped();
}
