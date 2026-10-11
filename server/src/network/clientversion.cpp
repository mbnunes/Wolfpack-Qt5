/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2026 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "clientversion.h"

const ClientVersion Version400a( 4, 0, 0, 1 );
const ClientVersion Version407a( 4, 0, 7, 1 );
const ClientVersion Version500a( 5, 0, 0, 1 );
const ClientVersion Version502b( 5, 0, 2, 2 );
const ClientVersion Version6000( 6, 0, 0, 0 );
const ClientVersion Version6017( 6, 0, 1, 7 );
const ClientVersion Version60142( 6, 0, 14, 2 );
const ClientVersion Version7000( 7, 0, 0, 0 );
const ClientVersion Version7090( 7, 0, 9, 0 );
const ClientVersion Version70130( 7, 0, 13, 0 );
const ClientVersion Version70160( 7, 0, 16, 0 );
const ClientVersion Version70300( 7, 0, 30, 0 );
const ClientVersion Version70331( 7, 0, 33, 1 );
const ClientVersion Version704565( 7, 0, 45, 65 );
const ClientVersion Version70500( 7, 0, 50, 0 );
const ClientVersion Version70610( 7, 0, 61, 0 );

ClientVersion::ClientVersion( quint32 maj, quint32 min, quint32 rev, quint32 pat, ClientType t )
	: major( maj ), minor( min ), revision( rev ), patch( pat ), type( t )
{
	if ( type != ClientSA && major >= 67 )
	{
		type = ClientSA;
	}
}

ClientVersion::ClientVersion( const QString& str )
	: major( 0 ), minor( 0 ), revision( 0 ), patch( 0 ), type( ClientRegular )
{
	fromString( str );
}

void ClientVersion::fromString( const QString& input )
{
	QString fmt = input.trimmed().toLower();
	major = 0;
	minor = 0;
	revision = 0;
	patch = 0;
	type = ClientRegular;

	if ( fmt.isEmpty() )
	{
		return;
	}

	try
	{
		int br1 = fmt.indexOf( '.' );
		if ( br1 < 0 )
		{
			major = fmt.toUInt();
			return;
		}

		int br2 = fmt.indexOf( '.', br1 + 1 );
		if ( br2 < 0 )
		{
			major = fmt.left( br1 ).toUInt();
			minor = fmt.mid( br1 + 1 ).toUInt();
			return;
		}

		int br3 = br2 + 1;
		while ( br3 < fmt.length() && fmt.at( br3 ).isDigit() )
		{
			br3++;
		}

		major = fmt.mid( 0, br1 ).toUInt();
		minor = fmt.mid( br1 + 1, br2 - br1 - 1 ).toUInt();
		revision = fmt.mid( br2 + 1, br3 - br2 - 1 ).toUInt();

		if ( br3 < fmt.length() )
		{
			if ( major <= 5 && minor <= 0 && revision <= 6 ) // Before 5.0.7
			{
				if ( !fmt.at( br3 ).isSpace() )
				{
					QChar c = fmt.at( br3 );
					if ( c >= 'a' && c <= 'z' )
					{
						patch = ( c.unicode() - 'a' ) + 1;
					}
				}
			}
			else
			{
				int nextDot = br3;
				if ( fmt.at( nextDot ) == '.' )
				{
					nextDot++;
				}
				int endDigit = nextDot;
				while ( endDigit < fmt.length() && fmt.at( endDigit ).isDigit() )
				{
					endDigit++;
				}
				if ( endDigit > nextDot )
				{
					patch = fmt.mid( nextDot, endDigit - nextDot ).toUInt();
				}
			}
		}

		if ( major >= 67 )
		{
			type = ClientSA;
		}
		else if ( fmt.contains( "god" ) || fmt.contains( "gq" ) )
		{
			type = ClientGod;
		}
		else if ( fmt.contains( "third dawn" ) || fmt.contains( "uo:td" ) || fmt.contains( "uotd" ) || fmt.contains( "uo3d" ) || fmt.contains( "uo:3d" ) )
		{
			type = ClientUOTD;
		}
		else
		{
			type = ClientRegular;
		}
	}
	catch ( ... )
	{
		major = 0;
		minor = 0;
		revision = 0;
		patch = 0;
		type = ClientRegular;
	}
}

QString ClientVersion::toString() const
{
	QString result = QString::asprintf( "%u.%u.%u", major, minor, revision );
	if ( major <= 5 && minor <= 0 && revision <= 6 )
	{
		if ( patch > 0 && patch <= 26 )
		{
			result += QChar( 'a' + patch - 1 );
		}
	}
	else
	{
		result += QString::asprintf( ".%u", patch );
	}

	if ( type == ClientSA )
	{
		result += " SA";
	}
	else if ( type == ClientGod )
	{
		result += " God";
	}
	else if ( type == ClientUOTD )
	{
		result += " UOTD";
	}

	return result;
}

bool ClientVersion::operator==( const ClientVersion& o ) const
{
	return major == o.major && minor == o.minor && revision == o.revision && patch == o.patch;
}

bool ClientVersion::operator!=( const ClientVersion& o ) const
{
	return !( *this == o );
}

bool ClientVersion::operator<( const ClientVersion& o ) const
{
	if ( major != o.major )
	{
		return major < o.major;
	}
	if ( minor != o.minor )
	{
		return minor < o.minor;
	}
	if ( revision != o.revision )
	{
		return revision < o.revision;
	}
	return patch < o.patch;
}

bool ClientVersion::operator<=( const ClientVersion& o ) const
{
	return !( o < *this );
}

bool ClientVersion::operator>( const ClientVersion& o ) const
{
	return o < *this;
}

bool ClientVersion::operator>=( const ClientVersion& o ) const
{
	return !( *this < o );
}

quint32 protocolChangesForVersion( const ClientVersion& v )
{
	if ( v >= Version70610 )
		return ProtocolVersion70610;
	if ( v >= Version70500 )
		return ProtocolVersion70500;
	if ( v >= Version704565 )
		return ProtocolVersion704565;
	if ( v >= Version70331 )
		return ProtocolVersion70331;
	if ( v >= Version70300 )
		return ProtocolVersion70300;
	if ( v >= Version70160 )
		return ProtocolVersion70160;
	if ( v >= Version70130 )
		return ProtocolVersion70130;
	if ( v >= Version7090 )
		return ProtocolVersion7090;
	if ( v >= Version7000 )
		return ProtocolVersion7000;
	if ( v >= Version60142 )
		return ProtocolVersion60142;
	if ( v >= Version6017 )
		return ProtocolVersion6017;
	if ( v >= Version6000 )
		return ProtocolVersion6000;
	if ( v >= Version502b )
		return ProtocolVersion502b;
	if ( v >= Version500a )
		return ProtocolVersion500a;
	if ( v >= Version407a )
		return ProtocolVersion407a;
	if ( v >= Version400a )
		return ProtocolVersion400a;
	return ProtocolNone;
}
