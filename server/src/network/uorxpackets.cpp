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

#include "uorxpackets.h"
#include "uopacket.h"
#include <math.h>

cUOPacket *getUOPacket( const QByteArray &data )
{
	if( data.isEmpty() )
		return 0;

	Q_UINT8 packetId = data[0];

	// Please keep this in order.
	switch( packetId )
	{
	case 0x00:		return new cUORxCreateChar( data );
	case 0x01:		return new cUORxNotifyDisconnect( data );
	case 0x02:		return new cUORxWalkRequest( data );
	case 0x05:		return new cUORxRequestAttack( data );
	case 0x06:		return new cUORxDoubleClick( data );
	case 0x09:		return new cUORxRequestLook( data );
	case 0x2C:		return new cUORxResurrectionMenu( data );
	case 0x34:		return new cUORxQuery( data );
	case 0x5D:		return new cUORxPlayCharacter( data );
	case 0x73:		return new cUORxPing( data );
	case 0x80:		return new cUORxLoginRequest( data );
	case 0x83:		return new cUORxDeleteCharacter( data );
	case 0x91:		return new cUORxServerAttach( data );
	case 0xA0:		return new cUORxSelectShard( data );
	case 0xA4:		return new cUORxHardwareInfo( data );
	case 0xA7:		return new cUORxGetTip( data );
	case 0xAD:		return new cUORxSpeechRequest( data );
	case 0xBF:		return new cUORxMultiPurpose( data );
	case 0xBD:		return new cUORxSetVersion( data );
	case 0xC8:		return new cUORxUpdateRange( data );
	case 0x93:		return new cUORxUpdateBook( data );
	case 0x75:		return new cUORxRename( data );
	case 0x72:		return new cUORxChangeWarmode( data );
	case 0x6C:		return new cUORxTarget( data );
	default:		return new cUOPacket( data );
	};	
}

cUOPacket *cUORxMultiPurpose::packet( void )
{
	// Switch the Subcommand
	switch( getShort( 3 ) )
	{
	case 0x0b:
		return new cUORxSetLanguage( rawPacket ); break;
	case 0x13:
		return new cUORxContextMenuRequest( rawPacket ); break;
	default:
		return new cUOPacket( (*this) );
	};
}

QString cUORxSpeechRequest::message()
{
	// 0x0c -> tokenized ascii speech
	if( type() & 0xc0 )
	{
		// Skip the keywords
		UINT16 skipCount = ( keywordCount() + 1 ) * 12; // We have 12 Bits for the count as well
		UINT16 skipBytes = (UINT16)floor( skipCount / 8 );
		if( (float)skipBytes < ( skipCount / 8 ) )
			skipBytes++;

		return &rawPacket.data()[ 12 + skipBytes ];
	}
	else
		return getUnicodeString( 12, getShort( 1 ) - 12 );
}
