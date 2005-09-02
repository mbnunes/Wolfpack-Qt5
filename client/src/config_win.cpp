
#include "config.h"
#include <windows.h>

/*
	Tries to figure where UO client was installed, thru the registry keys
*/
static QString getUOPath()
{
	// Search for T3D preferably
	const char* Registry3d = "Software\\Origin Worlds Online\\Ultima Online Third Dawn\\1.0";
	const char* Registry2d = "Software\\Origin Worlds Online\\Ultima Online\\1.0";
	unsigned char exePath[MAX_PATH] = {0,};
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
			path = path.left( path.lastIndexOf( "\\" ) + 1 );
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
			path = path.left( path.lastIndexOf( "\\" ) + 1 );
			return path;
		}
		RegCloseKey( tempKey );
	}

	return QString::null;
}

const QString &cConfig::uoPath() {
	// Try to find it automatically
	if (uoPath_.isEmpty()) {
		setUoPath(getUOPath());
	}

	return uoPath_;
}
