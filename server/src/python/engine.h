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

/*!
	The content of this file is exposed to wolfpack.cpp
	which uses it for startup of the python engine
*/

#if !defined(__PYTHON_ENGINE_H__)
#define __PYTHON_ENGINE_H__

#include <qglobal.h>
#include <qstring.h>

//#define DEBUG_PYTHON
#undef slots
#if defined(_DEBUG) && defined(Q_CC_MSVC) && !defined(DEBUG_PYTHON)
#	undef _DEBUG
#	include <Python.h>
#	define _DEBUG
#else
#	include <Python.h>
#endif
#define slots

#include "../server.h"
typedef void (*fnCleanupHandler)();

class CleanupAutoRegister{
public:
	CleanupAutoRegister(fnCleanupHandler);
};

class cPythonEngine : public cComponent {
public:
	cPythonEngine();
	virtual ~cPythonEngine();

	void load();
	void unload();
};

typedef SingletonHolder<cPythonEngine> PythonEngine;

void registerCleanupHandler(fnCleanupHandler);
void reloadPython();
void stopPython();
void startPython(int argc, char* argv[]);
void reportPythonError(QString moduleName = QString::null);

#endif // __PYTHON_ENGINE_H__
