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

#include "uotxpackets.h"
#include "uopacket.h"

void cUOTxShardList::addServer( Q_UINT16 serverIndex, QString serverName, Q_UINT8 serverFull, Q_INT8 serverTimeZone, Q_UINT32 serverIp )
{
	// Increase the server-count
	// Offset: 4
	setShort( 4, getShort( 4 ) + 1 );

	Q_INT32 offset = rawPacket.count();
	rawPacket.resize( rawPacket.count() + 40 ); // 40 byte per server
	setShort( 1, rawPacket.count() );
	setShort( offset, serverIndex );

	if( serverName.length() > 31 ) 
		serverName = serverName.left( 31 );

	strcpy( &rawPacket.data()[ offset + 2 ], serverName.latin1() );

	rawPacket[ offset + 34 ] = serverFull;
	rawPacket[ offset + 35 ] = serverTimeZone;
	setInt( offset + 36, serverIp );
}

void cUOTxCharTownList::addCharacter( QString name )
{
	// Trunace the name if needed
	if( name.length() > 29 )
		name = name.left( 29 );

	characters.push_back( name );
}

void cUOTxCharTownList::addTown( Q_UINT8 index, const QString &name, const QString &area )
{
	stTown town;
	town.town = ( name.length() > 30 ) ? name.left( 30 ) : name;
	town.area = ( area.length() > 30 ) ? area.left( 30 ) : area;
	town.index = index;
	towns.push_back( town );
}

void cUOTxCharTownList::compile( void )
{
	rawPacket.resize( 309 + ( towns.size() * 63 ) );
	rawPacket[ 0 ] = (Q_UINT8)0xA9;

	rawPacket[ 3 ] = characters.size(); // Char Count

	for( Q_UINT8 c = 0; c < 5; ++c )
		if( c < characters.size() )
		{
			strcpy( &rawPacket.data()[ 4 + ( c * 60 ) ], characters[ c ].latin1() );
			rawPacket[ 4 + ( c * 60 ) + 30 ] = 0x00; // No Password (!)
		}
		else
			rawPacket[ 4 + ( c * 60 ) ] = 0x00; // "Pad-out" the char

	// Town Count
	Q_INT32 offset = 304;
	rawPacket[ offset++ ] = towns.size();

	for( Q_UINT8 t = 0; t < towns.size(); ++t )
	{
		rawPacket[ offset ] = towns[ t ].index;
		strcpy( &rawPacket.data()[ offset + 1 ], towns[ t ].town.latin1() );
		strcpy( &rawPacket.data()[ offset + 32 ], towns[ t ].area.latin1() );
		offset += 63;
	}

	if( charLimit >= 0 )
		setInt( offset, ( charLimit << 4 ) | flags | 0x4 );
	else
		setInt( offset, flags );

	// New Packet Size
	setShort( 1, rawPacket.count() );
}

void cUOTxUpdateCharList::setCharacter( Q_UINT8 index, QString name )
{
	Q_INT32 offset = 4 + ( index * 60 );
	rawPacket[ 3 ]++;

	if( name.length() > 29 )
		name = name.left( 29 );

	strcpy( &rawPacket.data()[offset], name.latin1() );
}

void cUOTxSendSkills::addSkill( Q_UINT16 skillId, Q_UINT16 skill, Q_UINT16 realSkill, eStatus status )
{
	// Overwrite the last 2 bytes (terminator) and readd them later
	Q_INT32 offset = rawPacket.count() - 2;
	rawPacket.resize( rawPacket.count() + 7 );
	setShort( 1, rawPacket.count() );

	setShort( offset, skillId );
	setShort( offset+2, skill );
	setShort( offset+4, realSkill );
	rawPacket[ offset+6 ] = status;
	setShort( offset+7, 0 ); // Terminator
}

void cUOTxDrawObject::addEquipment( Q_UINT32 serial, Q_UINT16 model, Q_UINT8 layer, Q_UINT16 color )
{
	// Overwrite the last 4 bytes (terminator) and readd them later
	Q_INT32 offset = rawPacket.count() - 4;
	rawPacket.resize( rawPacket.count() + 9 );

	setInt( offset, serial );
	setShort( offset+4, (model|0x8000) );
	rawPacket[offset+6] = layer;
	setShort( offset+7, color );
	setInt( offset+9, 0 ); // Terminator
}

void cUOTxUnicodeSpeech::setText( const QString &data )
{
	rawPacket.resize( 50 + (data.length()*2) );

	Q_INT32 offset = 48; // Pad right by one - remeber to copy one byte less
	rawPacket[ offset ] = 0x00;
	memcpy( &rawPacket.data()[ offset + 1 ], data.unicode(), (data.length()*2)-1 );

	// Add the new Terminator
	setShort( rawPacket.count() - 2, 0 );
}

// Sets all data automatically
void cUOTxConfirmLogin::fromChar( P_CHAR pChar )
{
	setSerial( pChar->serial );
	setBody( pChar->id() );
	setDirection( pChar->dir );
	setX( pChar->pos.x );
	setY( pChar->pos.y );
	setZ( pChar->pos.z );
}

void cUOTxSendSkills::fromChar( P_CHAR pChar )
{
	if( !pChar )
		return;

	for( Q_UINT8 i = 0; i < ALLSKILLS; ++i )
		addSkill( i, pChar->skill( i ), pChar->baseSkill( i ), cUOTxSendSkills::Up );
}