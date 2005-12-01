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

#include "../serverconfig.h"
#include "../defines.h"
#include "../console.h"
#include "../log.h"

// Qt Includes
#include <QDir>
#include <QString>
#include <qhostaddress.h>

// System Includes
#include <windows.h>

/*!
	\internal
	Tries to figure where UO client was instaled, thru the registry keys
*/
static QString getUOPath()
{
	// Search for T3D preferably
	const char* Registry3d = "Software\\Origin Worlds Online\\Ultima Online Third Dawn\\1.0";
	const char* Registry2d = "Software\\Origin Worlds Online\\Ultima Online\\1.0";
	unsigned char exePath[MAX_PATH] =
	{
		0,
	};
	unsigned long pathLen = MAX_PATH;

	HKEY tempKey;

	// Look for 3D Client Path
	if ( RegOpenKeyExA( HKEY_LOCAL_MACHINE, Registry3d, 0, KEY_READ, &tempKey ) == ERROR_SUCCESS )
	{
		if ( RegQueryValueExA( tempKey, "ExePath", 0, 0, &exePath[0], &pathLen ) == ERROR_SUCCESS )
		{
			// We found a valid registry key
			RegCloseKey( tempKey );

			QString path( ( char* ) &exePath );
			path = path.left( path.findRev( "\\" ) + 1 );
			return path;
		}
		RegCloseKey( tempKey );
	}

	pathLen = MAX_PATH;

	// Look for 2D Client Path
	if ( RegOpenKeyExA( HKEY_LOCAL_MACHINE, Registry2d, 0, KEY_READ, &tempKey ) == ERROR_SUCCESS )
	{
		if ( RegQueryValueExA( tempKey, "ExePath", 0, 0, &exePath[0], &pathLen ) == ERROR_SUCCESS )
		{
			// We found a valid registry key
			RegCloseKey( tempKey );

			QString path( ( char* ) &exePath );
			path = path.left( path.findRev( "\\" ) + 1 );
			return path;
		}
		RegCloseKey( tempKey );
	}

	return QString::null;
}

/*!
	Returns the *.mul files path, extracted from wolfpack.xml
	If the path is not specified or if we can't find the mul files in there,
	this method will query the registry to try to figure where the UO client
	was installed
*/
QString cConfig::mulPath() const
{
	QDir thePath( mulPath_ );
	if ( !thePath.exists() || thePath.entryList( "*.mul" ).isEmpty() )
	{
		Console::instance()->log( LOG_WARNING, tr( "UO Mul files not found at '%1', trying to locate...\n" ).arg( mulPath_ ) );
		QString uoPath( getUOPath() );
		if ( !uoPath.isEmpty() )
		{
			//mulPath_ = uoPath;
			cConfig* that = const_cast<cConfig*>( this ); // perhaps not so const ;)
			that->setMulPath( uoPath );
		}
		else
			Console::instance()->log( LOG_ERROR, tr( "Unable to find *.mul files path. Please check wolfpack.xml, section \"General\", key \"MulPath\"" ) );
	}
	return mulPath_;
}
