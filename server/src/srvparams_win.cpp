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

#include "srvparams.h"

// Qt Includes
#include <qdir.h>
#include <qstring.h>

// System Includes
#include <windows.h>

static QString getUOPath()
{
	// Search for T3D preferably
	const QString Registry3d = "Software\\Origin Worlds Online\\Ultima Online Third Dawn\\1.0";
	const QString Registry2d = "Software\\Origin Worlds Online\\Ultima Online\\1.0";
	unsigned char exePath[MAX_PATH];
	unsigned long pathLen;

	HKEY tempKey;

	// Look for 3D Client Path
	if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, Registry3d.latin1(), 0, KEY_READ, &tempKey ) == ERROR_SUCCESS )
	{
		if( RegQueryValueEx( tempKey, "ExePath", 0, 0, &exePath[0], &pathLen ) == ERROR_SUCCESS )
		{
			// We found a valid registry key
			RegCloseKey( tempKey );

			QString path = (char*)&exePath;
			path = path.left( path.findRev( "\\" ) + 1 );
			return path;
		}
		RegCloseKey( tempKey );
	}

	// Look for 2D Client Path
	if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, Registry2d.latin1(), 0, KEY_READ, &tempKey ) == ERROR_SUCCESS )
	{
		if( RegQueryValueEx( tempKey, "ExePath", 0, 0, &exePath[0], &pathLen ) == ERROR_SUCCESS )
		{
			// We found a valid registry key
			RegCloseKey( tempKey );
			
			QString path = (char*)&exePath;
			path = path.left( path.findRev( "\\" ) + 1 );
			return path;
		}
		RegCloseKey( tempKey );
	}

	return QString::null;
}


QString cSrvParams::mulPath() const
{
	QDir thePath( mulPath_ );
	if( !thePath.exists() )
	{
		QString uoPath(getUOPath());
		if( uoPath != QString::null )
		{
			//mulPath_ = uoPath;
			cSrvParams* that = const_cast<cSrvParams*>(this); // perhaps not so const ;)
			that->setMulPath( uoPath );
		}
	}
	return mulPath_;
}
