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

#include "admin.h"

#include "wolfpack.h"
#include "globals.h"
#include "srvparams.h"

#undef DBGFILE
#define DBGFILE "admin.cpp"
#include "debug.h"


// Library Includes
#include "qstring.h"

cAdmin::cAdmin()
{
}

cAdmin::~cAdmin()
{
}

void cAdmin::ReadIni()
{
	QString basePath = SrvParams->getString("General", "MulPath", "./", true);
	if ( basePath.left(1) != "/" )
		basePath += "/";
	strcpy(Map->mapname, basePath + "map0.mul");
	strcpy(Map->sidxname, basePath + "staidx0.mul");
	strcpy(Map->statname, basePath + "statics0.mul");
	strcpy(Map->vername, basePath + "verdata.mul");
	strcpy(Map->tilename, basePath + "tiledata.mul");
	strcpy(Map->multiname, basePath + "multi.mul");
	strcpy(Map->midxname, basePath + "multi.idx");
	saveinterval = SrvParams->getNumber("General", "Save Interval", 900, true);
	heartbeat = SrvParams->getBool("General", "HeartBeat", false, true);
	defaultpriv1 = SrvParams->getNumber("General", "DefaultPrivileage1", 0, true);
	defaultpriv2 = SrvParams->getNumber("General", "DefaultPrivileage2", 0, true);
}
