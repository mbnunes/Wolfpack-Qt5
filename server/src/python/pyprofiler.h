/*
*     Wolfpack Emu (WP)
* UO Server Emulation Program
*
* Copyright 2001-2016 by holders identified in AUTHORS.txt
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


#if !defined ( __PYPROFILER_H__ )
#define __PYPROFILER_H__

#include "../singleton.h"
#include "engine.h"
#include <compile.h>
#include <frameobject.h>

#include <QObject>
#include <QHash>
#include <QSet>
#include <QMap>

class ProfileData
{
public:
	ProfileData() : timming( 0 ), calls( 0 ), firstline ( 0 )
	{
	}

	uint timming;
	uint calls;
	uint firstline;
	QSet<QByteArray> callees;
};

typedef QHash< QByteArray, QMap< QByteArray, ProfileData > > ProfileSession;

class PyProfiler : public QObject, public Singleton<PyProfiler>
{
	Q_OBJECT

	ProfileSession data;
public:

	ProfileSession sessionData() const { return data; }

public slots:
	void start();
	void stop();

signals:
	void stopped();

private:
	static int tracefunc( PyObject *obj, PyFrameObject *frame, int what, PyObject *arg );
};



#endif // __PYPROFILER_H__
