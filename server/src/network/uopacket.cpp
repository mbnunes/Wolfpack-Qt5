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

#include "uopacket.h"

// Library Includes
#include "qstring.h"

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
*/

/*!
  Constructs a packet that is a deep copy of \a d interpreted as
  raw data.
*/
cUOPacket::cUOPacket( QByteArray d )
{
	rawPacket = d.copy();
}

/*!
  Constructs a packet that is a deep copy of \a p.
*/
cUOPacket::cUOPacket( cUOPacket& p ) // copy constructor
{
	assign(p);
}

/*!
  Constructs a packet of size \a size and filled with 0's.
*/
cUOPacket::cUOPacket( Q_UINT32 size ) : rawPacket( size )
{
	rawPacket.fill( (char)0 );
}

/*!
  Constructs a packet of type \a packetId, of size \a size and filled
  with 0's in all positions except for the first byte which contains
  the packet type.
*/
cUOPacket::cUOPacket( Q_UINT8 packetId, Q_UINT32 size ) : rawPacket( size )
{
	rawPacket.fill( (char)0 );
	rawPacket[0] = packetId;
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
void cUOPacket::assign( cUOPacket& p)
{
	rawPacket = p.rawPacket.copy();
	if ( p.compressedBuffer.size() )
		compressedBuffer = p.compressedBuffer.copy();
}

// Compresses the packet
// Author: Beosil
static unsigned int bitTable[257][2] =
{
	{0x02, 0x00}, 	{0x05, 0x1F}, 	{0x06, 0x22}, 	{0x07, 0x34}, 	{0x07, 0x75}, 	{0x06, 0x28}, 	{0x06, 0x3B}, 	{0x07, 0x32},
	{0x08, 0xE0}, 	{0x08, 0x62}, 	{0x07, 0x56}, 	{0x08, 0x79}, 	{0x09, 0x19D},	{0x08, 0x97}, 	{0x06, 0x2A}, 	{0x07, 0x57},
	{0x08, 0x71}, 	{0x08, 0x5B}, 	{0x09, 0x1CC},	{0x08, 0xA7}, 	{0x07, 0x25}, 	{0x07, 0x4F}, 	{0x08, 0x66}, 	{0x08, 0x7D},
	{0x09, 0x191},	{0x09, 0x1CE}, 	{0x07, 0x3F}, 	{0x09, 0x90}, 	{0x08, 0x59}, 	{0x08, 0x7B}, 	{0x08, 0x91}, 	{0x08, 0xC6},
	{0x06, 0x2D}, 	{0x09, 0x186}, 	{0x08, 0x6F}, 	{0x09, 0x93}, 	{0x0A, 0x1CC},	{0x08, 0x5A}, 	{0x0A, 0x1AE},	{0x0A, 0x1C0},
	{0x09, 0x148},	{0x09, 0x14A}, 	{0x09, 0x82}, 	{0x0A, 0x19F}, 	{0x09, 0x171},	{0x09, 0x120}, 	{0x09, 0xE7}, 	{0x0A, 0x1F3},
	{0x09, 0x14B},	{0x09, 0x100},	{0x09, 0x190},	{0x06, 0x13}, 	{0x09, 0x161},	{0x09, 0x125},	{0x09, 0x133},	{0x09, 0x195},
	{0x09, 0x173},	{0x09, 0x1CA},	{0x09, 0x86}, 	{0x09, 0x1E9}, 	{0x09, 0xDB}, 	{0x09, 0x1EC},	{0x09, 0x8B}, 	{0x09, 0x85},
	{0x05, 0x0A}, 	{0x08, 0x96}, 	{0x08, 0x9C}, 	{0x09, 0x1C3}, 	{0x09, 0x19C},	{0x09, 0x8F}, 	{0x09, 0x18F},	{0x09, 0x91},
	{0x09, 0x87}, 	{0x09, 0xC6}, 	{0x09, 0x177},	{0x09, 0x89}, 	{0x09, 0xD6}, 	{0x09, 0x8C}, 	{0x09, 0x1EE},	{0x09, 0x1EB},
	{0x09, 0x84}, 	{0x09, 0x164}, 	{0x09, 0x175},	{0x09, 0x1CD}, 	{0x08, 0x5E}, 	{0x09, 0x88}, 	{0x09, 0x12B},	{0x09, 0x172},
	{0x09, 0x10A},	{0x09, 0x8D}, 	{0x09, 0x13A},	{0x09, 0x11C}, 	{0x0A, 0x1E1},	{0x0A, 0x1E0}, 	{0x09, 0x187},	{0x0A, 0x1DC},
	{0x0A, 0x1DF},	{0x07, 0x74}, 	{0x09, 0x19F},	{0x08, 0x8D},	{0x08, 0xE4}, 	{0x07, 0x79}, 	{0x09, 0xEA}, 	{0x09, 0xE1},
	{0x08, 0x40}, 	{0x07, 0x41}, 	{0x09, 0x10B},	{0x09, 0xB0}, 	{0x08, 0x6A}, 	{0x08, 0xC1}, 	{0x07, 0x71}, 	{0x07, 0x78},
	{0x08, 0xB1}, 	{0x09, 0x14C}, 	{0x07, 0x43}, 	{0x08, 0x76}, 	{0x07, 0x66}, 	{0x07, 0x4D}, 	{0x09, 0x8A}, 	{0x06, 0x2F},
	{0x08, 0xC9},	{0x09, 0xCE}, 	{0x09, 0x149},	{0x09, 0x160}, 	{0x0A, 0x1BA}, 	{0x0A, 0x19E}, 	{0x0A, 0x39F}, 	{0x09, 0xE5},
	{0x09, 0x194}, 	{0x09, 0x184}, 	{0x09, 0x126}, 	{0x07, 0x30}, 	{0x08, 0x6C}, 	{0x09, 0x121}, 	{0x09, 0x1E8}, 	{0x0A, 0x1C1},
	{0x0A, 0x11D}, 	{0x0A, 0x163}, 	{0x0A, 0x385}, 	{0x0A, 0x3DB}, 	{0x0A, 0x17D}, 	{0x0A, 0x106}, 	{0x0A, 0x397}, 	{0x0A, 0x24E},
	{0x07, 0x2E}, 	{0x08, 0x98}, 	{0x0A, 0x33C}, 	{0x0A, 0x32E}, 	{0x0A, 0x1E9}, 	{0x09, 0xBF}, 	{0x0A, 0x3DF}, 	{0x0A, 0x1DD},
	{0x0A, 0x32D}, 	{0x0A, 0x2ED}, 	{0x0A, 0x30B}, 	{0x0A, 0x107}, 	{0x0A, 0x2E8}, 	{0x0A, 0x3DE}, 	{0x0A, 0x125}, 	{0x0A, 0x1E8},
	{0x09, 0xE9}, 	{0x0A, 0x1CD}, 	{0x0A, 0x1B5}, 	{0x09, 0x165}, 	{0x0A, 0x232}, 	{0x0A, 0x2E1}, 	{0x0B, 0x3AE}, 	{0x0B, 0x3C6},
	{0x0B, 0x3E2}, 	{0x0A, 0x205}, 	{0x0A, 0x29A}, 	{0x0A, 0x248}, 	{0x0A, 0x2CD}, 	{0x0A, 0x23B}, 	{0x0B, 0x3C5}, 	{0x0A, 0x251},
	{0x0A, 0x2E9}, 	{0x0A, 0x252}, 	{0x09, 0x1EA}, 	{0x0B, 0x3A0}, 	{0x0B, 0x391}, 	{0x0A, 0x23C}, 	{0x0B, 0x392}, 	{0x0B, 0x3D5},
	{0x0A, 0x233}, 	{0x0A, 0x2CC}, 	{0x0B, 0x390}, 	{0x0A, 0x1BB}, 	{0x0B, 0x3A1}, 	{0x0B, 0x3C4}, 	{0x0A, 0x211}, 	{0x0A, 0x203},
	{0x09, 0x12A}, 	{0x0A, 0x231}, 	{0x0B, 0x3E0}, 	{0x0A, 0x29B}, 	{0x0B, 0x3D7}, 	{0x0A, 0x202}, 	{0x0B, 0x3AD}, 	{0x0A, 0x213},
	{0x0A, 0x253}, 	{0x0A, 0x32C}, 	{0x0A, 0x23D}, 	{0x0A, 0x23F}, 	{0x0A, 0x32F}, 	{0x0A, 0x11C}, 	{0x0A, 0x384}, 	{0x0A, 0x31C},
	{0x0A, 0x17C}, 	{0x0A, 0x30A}, 	{0x0A, 0x2E0}, 	{0x0A, 0x276}, 	{0x0A, 0x250}, 	{0x0B, 0x3E3}, 	{0x0A, 0x396}, 	{0x0A, 0x18F},
	{0x0A, 0x204}, 	{0x0A, 0x206}, 	{0x0A, 0x230}, 	{0x0A, 0x265}, 	{0x0A, 0x212}, 	{0x0A, 0x23E}, 	{0x0B, 0x3AC}, 	{0x0B, 0x393},
	{0x0B, 0x3E1}, 	{0x0A, 0x1DE}, 	{0x0B, 0x3D6}, 	{0x0A, 0x31D}, 	{0x0B, 0x3E5}, 	{0x0B, 0x3E4}, 	{0x0A, 0x207}, 	{0x0B, 0x3C7},
	{0x0A, 0x277}, 	{0x0B, 0x3D4}, 	{0x08, 0xC0},	{0x0A, 0x162}, 	{0x0A, 0x3DA}, 	{0x0A, 0x124}, 	{0x0A, 0x1B4}, 	{0x0A, 0x264},
	{0x0A, 0x33D}, 	{0x0A, 0x1D1}, 	{0x0A, 0x1AF}, 	{0x0A, 0x39E}, 	{0x0A, 0x24F}, 	{0x0B, 0x373}, 	{0x0A, 0x249}, 	{0x0B, 0x372},
	{0x09, 0x167}, 	{0x0A, 0x210}, 	{0x0A, 0x23A}, 	{0x0A, 0x1B8}, 	{0x0B, 0x3AF}, 	{0x0A, 0x18E}, 	{0x0A, 0x2EC}, 	{0x07, 0x62},
	{0x04, 0x0D}
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
	QByteArray temp( rawPacket.size()*2 ); // worst case scenario for memory size
	unsigned char *pIn  = (unsigned char*)rawPacket.data();
	unsigned char *pOut = (unsigned char*)temp.data();

	int actByte = 0;
	int bitByte = 0;
	int nrBits;
	unsigned int value;
	unsigned int len = rawPacket.size();

	while(len--)
	{
		nrBits = bitTable[*pIn][0];
		value  = bitTable[*pIn++][1];

		while(nrBits--)
		{
			pOut[actByte] = (pOut[actByte] << 1) | (unsigned char)((value >> nrBits) & 0x1);
			bitByte = (bitByte + 1) & 0x07;
			if(!bitByte) 
				++actByte;
		}
	}

	nrBits = bitTable[256][0];
	value  = bitTable[256][1];

	while(nrBits--)
	{
		pOut[actByte] = (pOut[actByte] << 1) | (unsigned char)((value >> nrBits) & 0x1);

		bitByte = (bitByte + 1) & 0x07;
		if(!bitByte) 
			++actByte;
	}

	if(bitByte)
	{
		while(bitByte < 8)
		{
			pOut[actByte] <<= 1;
			++bitByte;
		}
		++actByte;
	}
	compressedBuffer.duplicate( temp.data(), actByte);
}

/*!
  Returns the compressed packet data.
*/
QByteArray cUOPacket::compressed()
{
	if( compressedBuffer.size() == 0 )
		compress();

	return compressedBuffer;
}

/*!
  Reads a 32 bits integer value from the raw data buffer starting at position \a pos
*/
int cUOPacket::getInt( uint pos )
{
	int value = rawPacket.at(pos+3) & 0x000000FF;
	value |= rawPacket.at(pos+2) << 8;
	value |= rawPacket.at(pos+1) << 16;
	value |= rawPacket.at(pos)   << 24;
	return value;
}

/*!
  Reads a 16 bits integer value from the raw data buffer starting at position \a pos
*/
short cUOPacket::getShort( uint pos )
{
	short value = (Q_INT16)(rawPacket.at(pos+1)) & 0x00FF;
	value |= ((Q_INT16)(rawPacket.at(pos)) << 8) & 0xFF00;
	return value;
}

/*!
  Reads an unicode string from the raw data buffer starting at position \a pos
  and with size no longer than \a fieldLength. If the actual string in buffer
  is longer than the supplied \a fieldLength, it will be truncated.
*/
QString cUOPacket::getUnicodeString( uint pos, uint fieldLength )
{
	QString result;
	if ( pos + fieldLength > rawPacket.size() )
		qWarning("cUOPacket::getUnicodeString() - field size is bigger than packet");
	for ( uint i = pos; i < pos + fieldLength; i += 2 )
	{
		QChar ch(getShort(pos + i) );
		result.append(ch);
		if ( ch.isNull() )
			break;
	}
	return result;
}

/*!
  Writes a 32 bits integer \a value to the raw data buffer starting at position \a pos
*/
void  cUOPacket::setInt( unsigned int pos, unsigned int value )
{
	rawPacket.at(pos++) = static_cast<char>((value >> 24) & 0x000000FF);
	rawPacket.at(pos++) = static_cast<char>((value >> 16) & 0x000000FF);
	rawPacket.at(pos++) = static_cast<char>((value >> 8 ) & 0x000000FF);
	rawPacket.at(pos)   = static_cast<char>((value)       & 0x000000FF);
}

/*!
  Writes a 16 bits integer \a value to the raw data buffer starting at position \a pos
*/
void  cUOPacket::setShort( unsigned int pos, unsigned short value )
{
	rawPacket.at(pos++) = static_cast<char>((value >> 8 ) & 0x000000FF);
	rawPacket.at(pos)   = static_cast<char>((value)       & 0x000000FF);
}

/*!
  Writes an unicode string \a data to the raw data buffer starting at position \a pos
  and with field size \a maxlen. If the actual string \a data is longer than \a maxlen
  it will be truncated.
*/
void cUOPacket::setUnicodeString( uint pos, QString& data, uint maxlen )
{
	const QChar* unicodeData = data.unicode();
	const uint length = data.length() * 2 > maxlen ? maxlen/2 : data.length();
	for ( uint i = 0; i < length; ++i )
	{
		setShort(pos + i * 2, (*(unicodeData + i)).unicode());
	}
}

char& cUOPacket::operator[] ( unsigned int index )
{
	return rawPacket.at( index );
}

cUOPacket& cUOPacket::operator=( cUOPacket& p )
{
	assign(p);
	return *this;
}

// Leave as last method, please
void cUOPacket::print( ostream* s )
{
	if ( s )
		(*s) << dump( rawPacket ).latin1() << endl;
}

QString cUOPacket::dump( const QByteArray &data )
{
	Q_INT32 length = data.count();
	QString dumped = QString( "\n[ packet: %1 ; length: %2 ]\n" ).arg( (Q_UINT8)data[0], 2, 16 ).arg( data.count() );

	for(int actLine = 0; actLine < (length / 16) + 1; actLine++)
	{
		QString line;
		line.sprintf( "%04x: ", actLine * 16 );
		int actRow = 0;
		for(; actRow < 16; actRow++)
		{
			if( actLine * 16 + actRow < length ) 
				line += QString().sprintf( "%02x ", (unsigned int)((unsigned char)data[actLine * 16 + actRow]) );
			else 
				line += "-- ";
		}

		line += ": ";

		for(actRow = 0; actRow < 16; actRow++)
		{
			if( actLine * 16 + actRow < length ) 
				line += QString().sprintf( "%c", ( isprint(static_cast< Q_UINT8 >( data[actLine * 16 + actRow] ) ) ) ? data[actLine * 16 + actRow] : '.' );
		}

		line += "\n";
		dumped += line;
	}
	return dumped;
}

