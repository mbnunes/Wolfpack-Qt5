//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2004 by holders identified in authors.txt
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
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================

#include "../srvparams.h"
#include "../globals.h"
#include "../console.h"
#include "../log.h"

// Qt Includes
#include <qdir.h>
#include <qstring.h>
#include <qhostaddress.h>

// System Includes
#include <windows.h>

Q_INT32 resolveName( const QString& data ); // declared in srvparams.cpp

/*!
	\internal
	Tries to figure where UO client was instaled, thru the registry keys
*/
static QString getUOPath()
{
	// Search for T3D preferably
	const char* Registry3d = "Software\\Origin Worlds Online\\Ultima Online Third Dawn\\1.0";
	const char* Registry2d = "Software\\Origin Worlds Online\\Ultima Online\\1.0";
	unsigned char exePath[MAX_PATH] = {0,};
	unsigned long pathLen;

	HKEY tempKey;

	// Look for 3D Client Path
	if( RegOpenKeyExA( HKEY_LOCAL_MACHINE, Registry3d, 0, KEY_READ, &tempKey ) == ERROR_SUCCESS )
	{
		if( RegQueryValueExA( tempKey, "ExePath", 0, 0, &exePath[0], &pathLen ) == ERROR_SUCCESS )
		{
			// We found a valid registry key
			RegCloseKey( tempKey );

			QString path((char*)&exePath);
			path = path.left( path.findRev( "\\" ) + 1 );
			return path;
		}
		RegCloseKey( tempKey );
	}

	// Look for 2D Client Path
	if( RegOpenKeyExA( HKEY_LOCAL_MACHINE, Registry2d, 0, KEY_READ, &tempKey ) == ERROR_SUCCESS )
	{
		if( RegQueryValueExA( tempKey, "ExePath", 0, 0, &exePath[0], &pathLen ) == ERROR_SUCCESS )
		{
			// We found a valid registry key
			RegCloseKey( tempKey );
			
			QString path((char*)&exePath);
			path = path.left( path.findRev( "\\" ) + 1 );
			return path;
		}
		RegCloseKey( tempKey );
	}

	return QString::null;
}

/*!
	Returns the *.mul files path, extracted from wolfpack.xml
	If the path is not especified or if we can't find the mul files in there,
	this method will query the registry to try to figure where the UO client
	was installed
*/
QString cSrvParams::mulPath() const
{
	QDir thePath( mulPath_ );
	if( !thePath.exists() || thePath.entryList("*.mul").isEmpty() )
	{
		Console::instance()->log( LOG_WARNING, QString("UO Mul files not found at '%1', trying to locate...\n").arg(mulPath_));
		QString uoPath(getUOPath());
		if( !uoPath.isEmpty() )
		{
			//mulPath_ = uoPath;
			cSrvParams* that = const_cast<cSrvParams*>(this); // perhaps not so const ;)
			that->setMulPath( uoPath );
		}
		else
			Console::instance()->log( LOG_ERROR, "Unable to find *.mul files path. Please check wolfpack.xml, section \"General\", key \"MulPath\"");
	}
	return mulPath_;
}

std::vector<ServerList_st>& cSrvParams::serverList()
{
	static unsigned int lastIpCheck = 0;
	static bool dynamicIP = false;
	if ( serverList_.empty() || ( dynamicIP && lastIpCheck <= uiCurrentTime ) ) // Empty? Try to load
	{
		bool bKeepLooping = true;
		unsigned int i = 1;
		do
		{
			QString tmp = getString("LoginServer", QString("Shard %1").arg(i++), QString::null, false).simplifyWhiteSpace();
			bKeepLooping = !tmp.isEmpty();
			if ( bKeepLooping ) // valid data.
			{
				QStringList strList = QStringList::split("=", tmp);
				if ( strList.size() == 2 )
				{
					ServerList_st server;
					server.sServer = strList[0];
					QStringList strList2 = QStringList::split(",", strList[1].stripWhiteSpace());
					QHostAddress host;
					host.setAddress( strList2[0] );
					server.sIP = strList2[0];
					server.ip = resolveName( server.sIP );

					bool ok = false;
					server.uiPort = strList2[1].toUShort(&ok);
					if ( !ok )
						server.uiPort = 2593; // Unspecified defaults to 2593

					// This code will retrieve the first
					// valid Internet IP it finds
					// and replace a 0.0.0.0 with it
					if( ( ( server.ip == 0 ) && ( lastIpCheck <= uiCurrentTime ) ) )
					{
						dynamicIP = true;
						hostent *hostinfo;
						char name[256];

						// We check for a new IP max. every 30 minutes
						// So we have a max. of 30 minutes downtime
						lastIpCheck = uiCurrentTime + (MY_CLOCKS_PER_SEC*30*60);

						// WSA Is needed for this :/
						if( !gethostname( name, sizeof( name ) ) )
						{
							hostinfo = gethostbyname( name );

							if( hostinfo )
							{
								Q_UINT32 i = 0;

								while( hostinfo->h_addr_list[i] )
								{
									// Check if it's an INTERNET ADDRESS
									char *hIp = inet_ntoa( *(struct in_addr *)hostinfo->h_addr_list[i++] );
									host.setAddress( hIp );
									Q_UINT32 ip = host.ip4Addr();
									Q_UINT8 part1 = ( ip & 0xFF000000 ) >> 24;
									Q_UINT8 part2 = ( ip & 0x00FF0000 ) >> 16;

									if	( 
										( part1 == 127 ) || //this one is class A too.
										( part1 == 10 ) || 
										( ( part1 == 192 ) && ( part2 == 168 ) ) || 
										( ( part1 == 172 ) && ( part2 >= 16 ) && ( part2 <= 31 ) )  ||
										( ( part1 == 169 ) && ( part2 == 254 ) ) // DHCP Space Stuff
										)
									{
										continue;
									}

									// We are now certain that it's a valid INET ip
									server.ip = ip;
									break;
								}
							}
						}

						// Fall back to localhost
						if( !server.sIP )
							server.sIP = 0x7F000001;
					}
					serverList_.push_back(server);
				}
			}
		} while ( bKeepLooping );
	}
	return serverList_;
}
