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
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#include "uopacket.h"
#include "../platform.h"

// Library Includes
#include <qstring.h>

#include <ctype.h>

/*****************************************************************************
  cUOPacket member functions
 *****************************************************************************/

/*!
  \class cUOPacket uopacket.h

  \brief The cUOPacket class provides an abstraction of Ultima Online network
  packets.

  \ingroup network
  \ingroup mainclass

  cUOPacket is the base class for all incoming and outgoing application level
  packets from Ultima Online. It provides methods for accessing basic type fields
  inside the package given an start offset.
  This class will also handle BigEndian/LittleEndian differences and do the proper
  conversions if necessary.

*/

/*!
  Constructs a packet that is a deep copy of \a d interpreted as
  raw data.
*/
cUOPacket::cUOPacket( const QByteArray& d ) : haveCompressed( false )
{
	init();
	rawPacket = d.copy();
}

cUOPacket::~cUOPacket()
{
}

/*!
  Constructs a packet that is a deep copy of \a p.
*/
cUOPacket::cUOPacket( cUOPacket& p ) : haveCompressed( false )
{
	init();
	assign( p );
}

/*!
  Constructs a packet of size \a size and filled with 0's.
*/
cUOPacket::cUOPacket( Q_UINT32 size ) : rawPacket( size ), haveCompressed( false )
{
	init();
	rawPacket.fill( ( char ) 0 );
}

/*!
  Constructs a packet of type \a packetId, of size \a size and filled
  with 0's in all positions except for the first byte which contains
  the packet type.
*/
cUOPacket::cUOPacket( Q_UINT8 packetId, Q_UINT32 size ) : rawPacket( size ), haveCompressed( false )
{
	init();
	rawPacket.fill( ( char ) 0 );
	rawPacket[0] = packetId;
}

/*!
	\internal
	Internal initialize function. Should be called from all constructors.
*/
void cUOPacket::init()
{
	haveCompressed = false;
}

/*!
  \fn cUOPacket::assign( cUOPacket& p )
  \internal
  Performs a deep copy of the internal packet buffers:
  \list
  \i rawPacket - Uncompressed raw data
  \i compressedBuffer - Compressed data
  \endlist
*/
void cUOPacket::assign( cUOPacket& p )
{
	rawPacket = p.rawPacket.copy();
	haveCompressed = p.haveCompressed;
	if ( p.haveCompressed )
		compressedBuffer = p.compressedBuffer.copy();
}

/*!
  \internal
  Performs an internal buffer resize.
*/
void cUOPacket::resize( uint newSize )
{
	rawPacket.resize( newSize );
}

/*!
  Returns the packet size. This method doesn't reflect the size of this packet
  type, but rather how many bytes this instance in particular uses. No
  consistency checks are made or enforced here.
*/
uint cUOPacket::size() const
{
	return rawPacket.size();
}

/*!
  Returns the same as \sa size().
*/
uint cUOPacket::count() const
{
	return rawPacket.count();
}

/*!
  Inserts into \a pos the raw data suplied by \a data parameter
  If the packet's size is not enought to hold the position + data.size(),
  more memory will be allocated automatically.
*/
void cUOPacket::setRawData( uint pos, const char* data, uint dataSize )
{
	haveCompressed = false;
	if ( size() < pos + dataSize )
		resize( pos + dataSize );
	memcpy( &rawPacket.data()[pos], data, dataSize );
}

/*!
  static Huffman codes table used for packet compression
*/
struct
{
	unsigned int size;
	unsigned int code;
}
static bitTable[257] =
{
	{0x02, 0x00}, {0x05, 0x1F}, {0x06, 0x22}, {0x07, 0x34}, {0x07, 0x75}, {0x06, 0x28}, {0x06, 0x3B}, {0x07, 0x32}, {0x08, 0xE0}, {0x08, 0x62}, {0x07, 0x56}, {0x08, 0x79}, {0x09, 0x19D}, {0x08, 0x97}, {0x06, 0x2A}, {0x07, 0x57}, {0x08, 0x71}, {0x08, 0x5B}, {0x09, 0x1CC}, {0x08, 0xA7}, {0x07, 0x25}, {0x07, 0x4F}, {0x08, 0x66}, {0x08, 0x7D}, {0x09, 0x191}, {0x09, 0x1CE}, {0x07, 0x3F}, {0x09, 0x90}, {0x08, 0x59}, {0x08, 0x7B}, {0x08, 0x91}, {0x08, 0xC6}, {0x06, 0x2D}, {0x09, 0x186}, {0x08, 0x6F}, {0x09, 0x93}, {0x0A, 0x1CC}, {0x08, 0x5A}, {0x0A, 0x1AE}, {0x0A, 0x1C0}, {0x09, 0x148}, {0x09, 0x14A}, {0x09, 0x82}, {0x0A, 0x19F}, {0x09, 0x171}, {0x09, 0x120}, {0x09, 0xE7}, {0x0A, 0x1F3}, {0x09, 0x14B}, {0x09, 0x100}, {0x09, 0x190}, {0x06, 0x13}, {0x09, 0x161}, {0x09, 0x125}, {0x09, 0x133}, {0x09, 0x195}, {0x09, 0x173}, {0x09, 0x1CA}, {0x09, 0x86}, {0x09, 0x1E9}, {0x09, 0xDB}, {0x09, 0x1EC}, {0x09, 0x8B}, {0x09, 0x85}, {0x05, 0x0A}, {0x08, 0x96}, {0x08, 0x9C}, {0x09, 0x1C3}, {0x09, 0x19C}, {0x09, 0x8F}, {0x09, 0x18F}, {0x09, 0x91}, {0x09, 0x87}, {0x09, 0xC6}, {0x09, 0x177}, {0x09, 0x89}, {0x09, 0xD6}, {0x09, 0x8C}, {0x09, 0x1EE}, {0x09, 0x1EB}, {0x09, 0x84}, {0x09, 0x164}, {0x09, 0x175}, {0x09, 0x1CD}, {0x08, 0x5E}, {0x09, 0x88}, {0x09, 0x12B}, {0x09, 0x172}, {0x09, 0x10A}, {0x09, 0x8D}, {0x09, 0x13A}, {0x09, 0x11C}, {0x0A, 0x1E1}, {0x0A, 0x1E0}, {0x09, 0x187}, {0x0A, 0x1DC}, {0x0A, 0x1DF}, {0x07, 0x74}, {0x09, 0x19F}, {0x08, 0x8D}, {0x08, 0xE4}, {0x07, 0x79}, {0x09, 0xEA}, {0x09, 0xE1}, {0x08, 0x40}, {0x07, 0x41}, {0x09, 0x10B}, {0x09, 0xB0}, {0x08, 0x6A}, {0x08, 0xC1}, {0x07, 0x71}, {0x07, 0x78}, {0x08, 0xB1}, {0x09, 0x14C}, {0x07, 0x43}, {0x08, 0x76}, {0x07, 0x66}, {0x07, 0x4D}, {0x09, 0x8A}, {0x06, 0x2F}, {0x08, 0xC9}, {0x09, 0xCE}, {0x09, 0x149}, {0x09, 0x160}, {0x0A, 0x1BA}, {0x0A, 0x19E}, {0x0A, 0x39F}, {0x09, 0xE5}, {0x09, 0x194}, {0x09, 0x184}, {0x09, 0x126}, {0x07, 0x30}, {0x08, 0x6C}, {0x09, 0x121}, {0x09, 0x1E8}, {0x0A, 0x1C1}, {0x0A, 0x11D}, {0x0A, 0x163}, {0x0A, 0x385}, {0x0A, 0x3DB}, {0x0A, 0x17D}, {0x0A, 0x106}, {0x0A, 0x397}, {0x0A, 0x24E}, {0x07, 0x2E}, {0x08, 0x98}, {0x0A, 0x33C}, {0x0A, 0x32E}, {0x0A, 0x1E9}, {0x09, 0xBF}, {0x0A, 0x3DF}, {0x0A, 0x1DD}, {0x0A, 0x32D}, {0x0A, 0x2ED}, {0x0A, 0x30B}, {0x0A, 0x107}, {0x0A, 0x2E8}, {0x0A, 0x3DE}, {0x0A, 0x125}, {0x0A, 0x1E8}, {0x09, 0xE9}, {0x0A, 0x1CD}, {0x0A, 0x1B5}, {0x09, 0x165}, {0x0A, 0x232}, {0x0A, 0x2E1}, {0x0B, 0x3AE}, {0x0B, 0x3C6}, {0x0B, 0x3E2}, {0x0A, 0x205}, {0x0A, 0x29A}, {0x0A, 0x248}, {0x0A, 0x2CD}, {0x0A, 0x23B}, {0x0B, 0x3C5}, {0x0A, 0x251}, {0x0A, 0x2E9}, {0x0A, 0x252}, {0x09, 0x1EA}, {0x0B, 0x3A0}, {0x0B, 0x391}, {0x0A, 0x23C}, {0x0B, 0x392}, {0x0B, 0x3D5}, {0x0A, 0x233}, {0x0A, 0x2CC}, {0x0B, 0x390}, {0x0A, 0x1BB}, {0x0B, 0x3A1}, {0x0B, 0x3C4}, {0x0A, 0x211}, {0x0A, 0x203}, {0x09, 0x12A}, {0x0A, 0x231}, {0x0B, 0x3E0}, {0x0A, 0x29B}, {0x0B, 0x3D7}, {0x0A, 0x202}, {0x0B, 0x3AD}, {0x0A, 0x213}, {0x0A, 0x253}, {0x0A, 0x32C}, {0x0A, 0x23D}, {0x0A, 0x23F}, {0x0A, 0x32F}, {0x0A, 0x11C}, {0x0A, 0x384}, {0x0A, 0x31C}, {0x0A, 0x17C}, {0x0A, 0x30A}, {0x0A, 0x2E0}, {0x0A, 0x276}, {0x0A, 0x250}, {0x0B, 0x3E3}, {0x0A, 0x396}, {0x0A, 0x18F}, {0x0A, 0x204}, {0x0A, 0x206}, {0x0A, 0x230}, {0x0A, 0x265}, {0x0A, 0x212}, {0x0A, 0x23E}, {0x0B, 0x3AC}, {0x0B, 0x393}, {0x0B, 0x3E1}, {0x0A, 0x1DE}, {0x0B, 0x3D6}, {0x0A, 0x31D}, {0x0B, 0x3E5}, {0x0B, 0x3E4}, {0x0A, 0x207}, {0x0B, 0x3C7}, {0x0A, 0x277}, {0x0B, 0x3D4}, {0x08, 0xC0}, {0x0A, 0x162}, {0x0A, 0x3DA}, {0x0A, 0x124}, {0x0A, 0x1B4}, {0x0A, 0x264}, {0x0A, 0x33D}, {0x0A, 0x1D1}, {0x0A, 0x1AF}, {0x0A, 0x39E}, {0x0A, 0x24F}, {0x0B, 0x373}, {0x0A, 0x249}, {0x0B, 0x372}, {0x09, 0x167}, {0x0A, 0x210}, {0x0A, 0x23A}, {0x0A, 0x1B8}, {0x0B, 0x3AF}, {0x0A, 0x18E}, {0x0A, 0x2EC}, {0x07, 0x62}, {0x04, 0x0D}
};

/*!
  \fn cUOPacket::compress(void)
  \internal
  Compresses rawPacket buffer and stores the result into compressedBuffer.
  The compression algorithm is a simple Huffman coding using fixed frequency
  table \sa bitTable.
*/
void cUOPacket::compress( void )
{
	QByteArray temp( rawPacket.size() * 2 ); // worst case scenario for memory size
	int bufferSize = 0; // 32 bits buffer size (bits)
	Q_INT32 buffer32 = 0; // 32 bits buffer to store the compressed data until it's larger than 1 byte
	int codeSize = 0; // Size (in bits) of the Huffman code
	int code = 0; // Huffman code that represents current byte being compressed
	uint packetPos = 0; // Current byte being compressed
	int actByte = 0; // Number of bytes in the compressed buffer (temp)

	const uint packetSize = rawPacket.size(); // small optimization
	while ( packetPos < packetSize )
	{
		Q_UINT8 packetByte = static_cast<Q_UINT8>( rawPacket.at( packetPos++ ) );
		codeSize = bitTable[packetByte].size;
		code = bitTable[packetByte].code;
		buffer32 <<= codeSize;
		buffer32 |= code;
		bufferSize += codeSize;
		while ( bufferSize >= 8 ) // do we have 1 byte ready?
		{
			bufferSize -= 8;
			temp[actByte++] = ( unsigned char ) ( buffer32 >> bufferSize ) & 0xFF;
		}
	}
	codeSize = bitTable[256].size;
	code = bitTable[256].code;
	buffer32 <<= codeSize;
	buffer32 |= code;
	bufferSize += codeSize;
	while ( bufferSize >= 8 )
	{
		bufferSize -= 8;
		temp[actByte++] = ( unsigned char ) ( buffer32 >> bufferSize ) & 0xFF;//31;
	}
	if ( bufferSize > 0 )
	{
		temp[actByte++] = ( unsigned char ) ( buffer32 << 8 - bufferSize ) & 0xFF;//& 31;
	}
	compressedBuffer.duplicate( temp.data(), actByte );
}

/*!
  Returns the compressed packet data.
*/
QByteArray cUOPacket::compressed()
{
	if ( !haveCompressed )
	{
		compress();
		haveCompressed = true;
	}

	return compressedBuffer;
}

/*!
  Reads a 32 bits integer value from the raw data buffer starting at position \a pos
*/
int cUOPacket::getInt( uint pos ) const
{
#if defined(_DEBUG)
	if ( rawPacket.size() < pos + 3 )
	{
		qWarning( "Warning: cUOPacket::getInt() called with params out of bounds" );
	}
#endif
	int value;
	wpCopyIn(value, &rawPacket[(int)pos] );
	value = B_BENDIAN_TO_HOST_INT32(value);
	return value;
}

/*!
  Reads a 16 bits integer value from the raw data buffer starting at position \a pos
*/
short cUOPacket::getShort( uint pos ) const
{
#if defined(_DEBUG)
	if ( rawPacket.size() < pos + 1 )
	{
		qWarning( "Warning: cUOPacket::getShort() called with params out of bounds" );
		return 0;
	}
#endif
	short value;
	wpCopyIn(value, &rawPacket[(int)pos]);
	value = B_BENDIAN_TO_HOST_INT16(value);
	return value;
}

/*!
  Reads an unicode string from the raw data buffer starting at position \a pos
  and with size no longer than \a fieldLength. If the actual string in buffer
  is longer than the supplied \a fieldLength, it will be truncated.
*/
QString cUOPacket::getUnicodeString( uint pos, uint fieldLength ) const
{
	QString result;
#if defined(_DEBUG)
	if ( pos + fieldLength > rawPacket.size() )
	{
		qWarning( "cUOPacket::getUnicodeString() - field size is bigger than packet" );
		return result;
	}
#endif
	for ( uint i = pos; i < pos + fieldLength; i += 2 )
	{
		QChar ch( getShort( i ) );
		if ( ch.isNull() )
			break;
		result.append( ch );
	}
	return result;
}

/*!
  Reads an ASCII string from the raw data buffer starting at position \a pos
  and with size no longer than \a fieldLength. If the actual string in buffer
  is longer than the supplied \a fieldLength, it will be truncated.
  \a fieldLength can be 0, in which case, no size check is performed the string
  is read until a \0 is found.
*/
QCString cUOPacket::getAsciiString( uint pos, uint fieldLength ) const
{
#if defined(_DEBUG)
	if ( rawPacket.size() < fieldLength + pos )
	{
		qWarning( "Warning: cUOPacket::getAsciiString() called with params out of bounds" );
		return QCString(); //#better return empty ?
	}
#endif
	if ( fieldLength )
	{
		char* buffer = new char[fieldLength + 1];
		qstrncpy( buffer, rawPacket.data() + pos, fieldLength );
		buffer[fieldLength] = 0; // truncate if larger
		QCString result( buffer );
		delete[] buffer;
		return result;
	}
	else
	{
		return rawPacket.data() + pos; // go until an \0 is found.
	}
}

/*!
  Writes a 32 bits integer \a value to the raw data buffer starting at position \a pos
*/
void cUOPacket::setInt( unsigned int pos, unsigned int value )
{
	haveCompressed = false; // changed
	value = B_HOST_TO_BENDIAN_INT32(value);
	wpCopyOut( &rawPacket[(int)pos], value );
}

/*!
  Writes a 16 bits integer \a value to the raw data buffer starting at position \a pos
*/
void cUOPacket::setShort( unsigned int pos, unsigned short value )
{
	haveCompressed = false; // changed
	value = B_HOST_TO_BENDIAN_INT16(value);
	wpCopyOut( &rawPacket[(int)pos], value );
}

/*!
  Writes an unicode string \a data to the raw data buffer starting at position \a pos
  and with field size \a maxlen. If the actual string \a data is longer than \a maxlen
  it will be truncated.
*/
void cUOPacket::setUnicodeString( uint pos, const QString& data, uint maxlen, bool swapbytes )
{
	haveCompressed = false; // changed
	const QChar* unicodeData = data.unicode();
	const uint length = data.length() * 2 > maxlen ? maxlen / 2 : data.length();
	for ( uint i = 0; i < length; ++i )
	{
		if ( !swapbytes )
		{
			setShort( pos + i * 2, ( *( unicodeData + i ) ).unicode() );
		}
		else
		{
			( *this )[pos + i * 2 + 1] = ( ( *( unicodeData + i ) ).unicode() >> 8 ) & 0xFF;
			( *this )[pos + i * 2] = ( ( *( unicodeData + i ) ).unicode() ) & 0xFF;
		}
	}
}

/*!
  Writes an ascii (Latin-1) string \a data to the raw data buffer starting at position \a pos
  and with field size \a maxlen. If the actual string \a data is longer than \a maxlen
  it will be truncated.
*/
void cUOPacket::setAsciiString( uint pos, const char* data, uint maxlen )
{
	haveCompressed = false; // changed
	qstrncpy( rawPacket.data() + pos, data, maxlen );
}

/*!
  This is a method added for convinience. It permits read/write access to a single byte
  inside the packet.
*/
char& cUOPacket::operator[]( unsigned int index )
{
	haveCompressed = false; // better safe than sorry
	return rawPacket.at( index );
}

/*!
  Performs a deep copy of \a p.
*/
cUOPacket& cUOPacket::operator=( cUOPacket& p )
{
	if ( &p != this )
		assign( p );
	return *this;
}

/*!
  This is a debug method. It will dump a numerical hex and ascii representation of the packet
  as well as a right table of offsets into \a s.
  For example:
  \code
  cUOPacket p;
  ...
  p.print(&cout);
  \endcode
  Will produce an output similar to this:
  \verbatim
	[ packet: 9b ; length: 258 ]
	0000: 9b 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 : ................
	0010: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 : ................
	0020: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 : ................
	0030: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 : ................
	0040: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 : ................
	0050: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 : ................
	0060: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 : ................
	0070: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 : ................
	0080: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 : ................
	0090: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 : ................
	00a0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 : ................
	00b0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 : ................
	00c0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 : ................
	00d0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 : ................
	00e0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 : ................
	00f0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 : ................
	0100: 00 00 -- -- -- -- -- -- -- -- -- -- -- -- -- -- : ..
  \endverbatim
*/
QCString cUOPacket::dump( const QByteArray& data )
{
	Q_INT32 length = data.count();
	QCString dumped = QString( "\n[ packet: %1; length: %2 ]\n" ).arg( ( Q_UINT8 ) data[0], 2, 16 ).arg( data.count() ).latin1();

	int lines = length / 16;
	if ( length % 16 ) // always round up.
		lines++;

	for ( int actLine = 0; actLine < lines; ++actLine )
	{
		QCString line; //= QString("%1: ").arg(actLine*16, 4, 16); // Faster, but doesn't look so good
		line.sprintf( "%04x: ", actLine * 16 );
		int actRow = 0;
		for ( ; actRow < 16; ++actRow )
		{
			if ( actLine * 16 + actRow < length )
			{
				QCString number = QString::number( static_cast<uint>( static_cast<Q_UINT8>( data[actLine*16 + actRow] ) ), 16 ).latin1() + QCString( " " );
				//line += QString().sprintf( "%02x ", (unsigned int)((unsigned char)data[actLine * 16 + actRow]) );
				if ( number.length() < 3 )
					number.prepend( "0" );
				line += number;
			}
			else
				line += "-- ";
		}

		line += ": ";

		for ( actRow = 0; actRow < 16; ++actRow )
		{
			if ( actLine * 16 + actRow < length )
				line += ( isprint( static_cast<Q_UINT8>( data[actLine * 16 + actRow] ) ) ) ? data[actLine * 16 + actRow] : '.' ;
		}

		line += "\n";
		dumped += line;
	}
	return dumped;
}

