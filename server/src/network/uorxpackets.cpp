//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2003 by holders identified in authors.txt
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

#include "uorxpackets.h"
#include "uopacket.h"
#include <math.h>

using namespace std;

cUOPacket *getUOPacket( const QByteArray &data )
{
	if( data.isEmpty() )
	{
		return 0;
	}

	Q_UINT8 packetId = data[0];

	// Please keep this in order.
	switch( packetId )
	{
	case 0x00:		return new cUORxCreateChar( data );
	case 0x01:		return new cUORxNotifyDisconnect( data );
	case 0x02:		return new cUORxWalkRequest( data );
	case 0x05:		return new cUORxRequestAttack( data );
	case 0x06:		return new cUORxDoubleClick( data );
	case 0x07:		return new cUORxDragItem( data );
	case 0x08:		return new cUORxDropItem( data );
	case 0x09:		return new cUORxRequestLook( data );
	case 0x12:		return new cUORxAction( data );
	case 0x13:		return new cUORxWearItem( data );
	case 0x22:		return new cUORxResyncWalk( data );
	case 0x2C:		return new cUORxResurrectionMenu( data );
	case 0x34:		return new cUORxQuery( data );
	case 0x3A:		return new cUORxSkillLock( data );
	case 0x3B:		return new cUORxBuy( data );
	case 0x5D:		return new cUORxPlayCharacter( data );
	case 0x66:		return new cUORxBookPage( data );
	case 0x6C:		return new cUORxTarget( data );
	case 0x72:		return new cUORxChangeWarmode( data );
	case 0x73:		return new cUORxPing( data );
	case 0x75:		return new cUORxRename( data );
	case 0x80:		return new cUORxLoginRequest( data );
	case 0x83:		return new cUORxDeleteCharacter( data );
	case 0x91:		return new cUORxServerAttach( data );
	case 0x93:		return new cUORxUpdateBook( data );
	case 0x95:		return new cUORxDye( data );
	case 0x9B:		return new cUORxHelpRequest( data );
	case 0x9F:		return new cUORxSell( data );
	case 0xA0:		return new cUORxSelectShard( data );
	case 0xA4:		return new cUORxHardwareInfo( data );
	case 0xA7:		return new cUORxGetTip( data );
	case 0xAD:		return new cUORxSpeechRequest( data );
	case 0xB1:		return new cUORxGumpResponse( data );
	case 0xBF:		return cUORxMultiPurpose::packet( data );
	case 0xBD:		return new cUORxSetVersion( data );
	case 0xC8:		return new cUORxUpdateRange( data );
	case 0xB8:		return new cUORxProfile( data );
	case 0xD7:		return cUORxAosMultiPurpose::packet( data );
	default:		return new cUOPacket( data );
	};	
}

cUOPacket *cUORxMultiPurpose::packet( const QByteArray& data ) 
{ 
	cUOPacket temp(data);
	// Switch the Subcommand 
	switch( temp.getShort( 3 ) ) 
	{ 
	case setLanguage: 
		return new cUORxSetLanguage( data ); break; 
	case contextMenuRequest: 
		return new cUORxContextMenuRequest( data ); break; 
	case contextMenuSelection: 
		return new cUORxContextMenuSelection( data ); break; 
	case castSpell:
		return new cUORxCastSpell( data ); break;
	case toolTip:
		return new cUORxRequestToolTip( data ); break;
	case customHouseRequest:
		return new cUORxCustomHouseRequest( data ); break;
	default:
		{
			//qWarning("Unknown cUORxMultiPurpose subcommand");
			//qWarning( cUOPacket::dump( data ) );
			return new cUOPacket( data ); 
		}
	}; 
} 
cUOPacket *cUORxAosMultiPurpose::packet( const QByteArray& data )
{
	cUOPacket temp( data );
	
	switch( temp.getShort( 7 ) )
	{
	default:
		{
			//qWarning("Unknown cUORxMultiPurpose subcommand");
			//qWarning( cUOPacket::dump( data ) );
			return new cUOPacket( data );
		}
	}
}
QString cUORxSpeechRequest::message()
{
	// 0x0c -> tokenized ascii speech
	if( type() & 0xc0 )
	{
		// Skip the keywords
		UINT16 skipCount = ( keywordCount() + 1 ) * 12; // We have 12 Bits for the count as well
		UINT16 skipBytes = static_cast<UINT16>( skipCount / 8 );
		if( skipCount % 8 > 0 ) // Round up
			skipBytes++;

		QString speech = getAsciiString(12 + skipBytes, getShort(1) - (12 + skipBytes) );
		// Sadly we are not finished yet
		// The UO client encodes the UNICODE speech in a rather strange... format.
		// So we need to iterate trough the speech

		return QString().fromUtf8( speech.data() );
	}
	else
		return getUnicodeString( 12, getShort( 1 ) - 12 );
}

QStringList cUORxBookPage::lines()
{
	if( this->numOfLines() == (UINT16)-1 )
		return QStringList();
	
	UINT16 i = 13;

	if( i >= size() )
		return QStringList();

	QStringList lines_ = QStringList();
	UINT16 currLine = 0;
	while( currLine < numOfLines() )
	{
		QString line = QString( getAsciiString(i) );
		i += (line.length()+1);
		lines_.push_back( line );
		currLine++; // next line!
	}
	return lines_;
}

gumpChoice_st cUORxGumpResponse::choice()
{
	gumpChoice_st choice;
	choice.button = getInt( 11 );
	UINT32 numSwitches = getInt( 15 );
	UINT32 i;
	for( i = 0; i < numSwitches; i++ )
	{
		choice.switches.push_back( getInt( 19 + 4 * i ) );
	}
	UINT32 numTextEntries = getInt( 19 + 4 * numSwitches );
	UINT32 offset = 0;
	for( i = 0; i < numTextEntries; i++ )
	{
		UINT16 textLength = getShort( 25 + 4 * numSwitches + offset );
		choice.textentries.insert( 
			make_pair< UINT16, QString >( getShort( 23 + 4 * numSwitches + offset ), getUnicodeString( 27 + 4 * numSwitches + offset, textLength * 2 ) ) );

		offset += 4 + textLength * 2;
	}

	return choice;
}

QValueVector< UINT16 > cUORxSpeechRequest::keywords()
{
	QValueVector< UINT16 > keywords;

	UINT16 count = keywordCount();
	UINT16 offset = 13; // Skip the count

	for( UINT16 i = 0; i < count; ++i )
	{
		// Invalid Packet size
		if( offset+2U > size() )
			return keywords;

		UINT16 value;

		// The second, fourth, etc. keyword always 
		// has the *first* 12 bits of a short
		if( i % 2 != 0 )
		{
			value = getShort( offset ) >> 4;
			offset++; // In the lower 4 bits still is information
		}

		// The first, third, etc. keyword always
		// has the *last* 12 bits of a short
		// Bitmask:
		// 00001111 11111111 = 0x0FFF
		else
		{
			value = getShort( offset ) & 0x0FFF;
			offset += 2; // Our short doesn't have any information left
		}

		keywords.push_back( value );
	}

	return keywords;
}
