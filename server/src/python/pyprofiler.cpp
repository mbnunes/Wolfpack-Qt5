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


#include "pyprofiler.h"
#include <QByteArray>

/* Python 2.3 Compatibility */
#if !defined( PyTrace_C_CALL )
#define PyTrace_C_CALL 4
#endif
#if !defined( PyTrace_C_EXCEPTION )
#define PyTrace_C_EXCEPTION 5
#endif
#if !defined( PyTrace_C_RETURN )
#define PyTrace_C_RETURN 6
#endif

int PyProfiler::tracefunc( PyObject */*obj*/, PyFrameObject *frame, int what, PyObject */*arg*/ )
{
	PyProfiler* that = PyProfiler::instance();
	ProfileData* pdata = 0;
	switch ( what )
	{
	case PyTrace_C_CALL:
        pdata = &that->data[tr("<core>").toLocal8Bit()][PyBytes_AS_STRING(frame->f_code->co_name)];
		if ( frame != frame->f_back )
		{
			pdata->calls++;
			if ( frame->f_back )
                pdata->callees.insert( PyBytes_AS_STRING( frame->f_back->f_code->co_name ) );
			else
				pdata->callees.insert( tr("<core>").toLocal8Bit() );
		}
		break;
	case PyTrace_CALL:
		//that->data.find()
		// Don't count recursions
        pdata = &that->data[PyBytes_AS_STRING(frame->f_code->co_filename)][PyBytes_AS_STRING(frame->f_code->co_name)];
		pdata->firstline = frame->f_code->co_firstlineno;
		if ( frame != frame->f_back )
		{
			pdata->calls++;
			if ( frame->f_back )
                pdata->callees.insert( PyBytes_AS_STRING( frame->f_back->f_code->co_name ) );
			else
				pdata->callees.insert( tr("<core>").toLocal8Bit() );
		}
        //that->data[PyBytes_AS_STRING(frame->f_code->co_filename)][PyBytes_AS_STRING(frame->f_code->co_name)] = pdata;
		break;
	case PyTrace_RETURN:
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
