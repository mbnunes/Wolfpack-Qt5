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

#include "../config.h"
#include "../globals.h"
#include "../console.h"
#include "../log.h"

// Qt Includes
#include <qhostaddress.h>
#include <qdir.h>
#include <qstringlist.h>

Q_INT32 resolveName( const QString& data ); // declared in srvparams.cpp

QString cConfig::mulPath() const
{
	QDir thePath( mulPath_ );
	if( !thePath.exists() || thePath.entryList("*.mul").isEmpty() )
	{
		// Can't detect under *nix, so just warn the user :(
		qWarning( "Unable to find *.mul files path. Please check wolfpack.xml, section \"General\", key \"MulPath\"" );
	}
	return mulPath_;
}

std::vector<ServerList_st>& cConfig::serverList()
{
	if ( serverList_.empty() ) // Empty? Try to load
	{
		bool bKeepLooping = true;
		unsigned int i = 1;
		do
		{
			QString tmp = getString("LoginServer", QString("Shard %1").arg(i++), "", false).simplifyWhiteSpace();
			bKeepLooping = ( tmp != "" );
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

					serverList_.push_back(server);
				}
			}
		} while ( bKeepLooping );
	}
	return serverList_;
}

