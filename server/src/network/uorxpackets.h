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

#if !defined(__UO_RXPACKETS__)
#define __UO_RXPACKETS__

#include "uopacket.h"
#include "qcstring.h"
#include "qstring.h"

// Wolfpack Includes
#include "../typedefs.h"

cUOPacket *getUOPacket( const QByteArray &data );

// 0x34: Query
class cUORxQuery: public cUOPacket
{
public:
	enum eType
	{
		Stats = 0x04,
		Skills
	};

	cUORxQuery( const QByteArray &data ): cUOPacket( data ) {}

	UINT32 pattern( void ) { return getInt( 1 ); }
	eType type( void ) { return ( rawPacket[5] == 0x05 ) ? Skills : Stats; }
	UINT32 serial( void ) { return getInt( 6 ); }
};

// 0x00: Create Char
class cUORxCreateChar: public cUOPacket
{
public:
	cUORxCreateChar( const QByteArray &data ): cUOPacket( data ) {}
	UINT32 pattern1( void )		{ return getInt( 1 ); }
	UINT32 pattern2( void )		{ return getInt( 5 ); }
	UINT8 pattern3( void )		{ return rawPacket[ 9 ]; }
	QString name( void )			{ return &rawPacket.data()[10]; }
	QString password( void )		{ return &rawPacket.data()[40]; }
	UINT8 gender( void )			{ return rawPacket[70]; } // 0 = male, 1 = female
	UINT8 strength( void )		{ return rawPacket[71]; }
	UINT8 dexterity( void )		{ return rawPacket[72]; }
	UINT8 intelligence( void )	{ return rawPacket[73]; }
	UINT8 skillId1( void )		{ return rawPacket[74]; }
	UINT8 skillValue1( void )		{ return rawPacket[75]; }
	UINT8 skillId2( void )		{ return rawPacket[76]; }
	UINT8 skillValue2( void )		{ return rawPacket[77]; }
	UINT8 skillId3( void )		{ return rawPacket[78]; }
	UINT8 skillValue3( void )		{ return rawPacket[79]; }
	INT16 skinColor( void )		{ return getShort( 80 ); }
	INT16 hairStyle( void )		{ return getShort( 82 ); }
	INT16 hairColor( void )		{ return getShort( 84 ); }
	INT16 beardStyle( void )		{ return getShort( 86 ); }
	INT16 beardColor( void )		{ return getShort( 88 ); }
	// Here is an unkown byte (!)
	UINT8 startTown( void )			{ return rawPacket[91]; }
	UINT16 unknown1( void )		{ return getShort( 92 ); }
	UINT16 slot( void )			{ return getShort( 94 ); }
	UINT32 ip( void )				{ return getInt( 96 ); }
	INT16 shirtColor( void )		{ return getShort( 100 ); }
	INT16 pantsColor( void )		{ return getShort( 102 ); }
};

// 0xC8: UpdateRange
class cUORxUpdateRange: public cUOPacket
{
public:
	cUORxUpdateRange( const QByteArray &data ): cUOPacket( data ) {}
	UINT8 range( void ) { return rawPacket[1]; }
};

// 0x01: NotifyDisconnect
class cUORxNotifyDisconnect: public cUOPacket
{
public:
	cUORxNotifyDisconnect( const QByteArray &data ): cUOPacket( data ) {}
	UINT32 pattern( void ) { return getInt( 1 ); }
};

// 0x80: Login Request
class cUORxLoginRequest: public cUOPacket
{
public:
	cUORxLoginRequest( const QByteArray &data ): cUOPacket( data ) {}
	QString username( void ) { return &rawPacket.data()[1]; }
	QString password( void ) { return &rawPacket.data()[31]; }
};

// 0xA4: Hardware Info
class cUORxHardwareInfo: public cUOPacket
{
public:
	cUORxHardwareInfo( const QByteArray &data ): cUOPacket( data ) {}
	UINT8 processorType( void ) { return rawPacket[ 0x01 ]; }
	UINT16 processorSpeed( void ) { return getShort( 0x02 ); }
	UINT8 processorCount( void ) { return rawPacket[ 0x04 ]; }
	// QString directory -- unicode ? -- > 20 bytes useless trash
	// Video Card descritor: -- unicode ? -- > 20 bytes useless trash
	// ??? > another 20 byte trash field
	// ??? > ANOTHER 20 byte trash field
	UINT16 memoryInMb( void ) { return getShort( 0x85 ); }
	UINT16 largestPartitionInMb( void ) { return getShort( 0x87 ); }
	INT32 timezoneBias( void ) { return getInt( 0x8C ); }
};

// 0xA0: Select Shard
class cUORxSelectShard: public cUOPacket
{
public:
	cUORxSelectShard( const QByteArray &data ): cUOPacket( data ) {}
	UINT16 shardId( void ) { return getShort( 1 ); }
};

// 0x91: Charlist Request
class cUORxServerAttach: public cUOPacket
{
public:
	cUORxServerAttach( const QByteArray &data ): cUOPacket( data ) {}
	UINT32 authId( void ) { return getInt( 1 ); }
	QString username( void ) { return &rawPacket.data()[5]; }
	QString password( void ) { return &rawPacket.data()[35]; }
};

// 0x73 Ping
class cUORxPing: public cUOPacket
{
public:
	cUORxPing( const QByteArray &data ): cUOPacket( data ) {}
};

// 0x83 Delete Character
class cUORxDeleteCharacter: public cUOPacket
{
public:
	cUORxDeleteCharacter( const QByteArray &data ): cUOPacket( data ) {}

	QString password( void ) { return &rawPacket.data()[1]; }
	UINT32 index( void ) { return getInt( 31 ); }
	UINT32 ip( void ) { return getInt( 35 ); }
};

// 0x5D Play Character
class cUORxPlayCharacter: public cUOPacket
{
public:
	cUORxPlayCharacter( const QByteArray &data ): cUOPacket( data ) {}

	QString character( void ) { return &rawPacket.data()[5]; }
	QString password( void ) { return &rawPacket.data()[35]; }
	UINT32 slot( void ) { return getInt( 65 ); }
	UINT32 ip( void ) { return getInt( 69 ); }
};

// 0x05 Request Use
class cUORxRequestAttack: public cUOPacket
{
public:
	cUORxRequestAttack( const QByteArray &data ): cUOPacket( data ) {}

	UINT32 serial( void ) { return getInt( 1 ); }
};

// 0x09 Request Look
class cUORxRequestLook: public cUOPacket
{
public:
	cUORxRequestLook( const QByteArray &data ): cUOPacket( data ) {}

	UINT32 serial( void ) { return getInt( 1 ); }
};

// 0x2C Resurrection Menu
class cUORxResurrectionMenu: public cUOPacket
{
public:
	cUORxResurrectionMenu( const QByteArray &data ): cUOPacket( data ) {}

	enum eChoice
	{
		Resurrect = 1,
		Ghost
	};

	eChoice choice( void ) { return ( rawPacket[1] == 0x01 ) ? Resurrect : Ghost; }
};

// 0xBF Multi Purpose Packet -> Split up into other packets later
class cUORxMultiPurpose: public cUOPacket
{
public:
	enum eSubCommands
	{
		unknown = 0,
		initFastWalk,
	};

	cUORxMultiPurpose( const QByteArray &data ): cUOPacket( data ) {}
	cUOPacket *packet( void );
	eSubCommands subCommand( void ) { return (eSubCommands)getShort( 3 ); }
};

// 0xBF 0x13 PopUp Menu Request
class cUORxContextMenuRequest: public cUOPacket
{
public:
	cUORxContextMenuRequest( const QByteArray &data ): cUOPacket( data ) {}
	UINT32 serial( void ) { return getInt( 5 ); }
};

// 0xBF 0x0B Set Client Language
class cUORxSetLanguage: public cUOPacket
{
public:
	cUORxSetLanguage( const QByteArray &data ): cUOPacket( data ) {}
	QString language( void ) { return &rawPacket.data()[5]; }
};

// 0xBD Set Version
class cUORxSetVersion: public cUOPacket
{
public:
	cUORxSetVersion( const QByteArray &data ): cUOPacket( data ) {}
	QString version( void ) { return &rawPacket.data()[3]; }
};

// 0x02 Walk Request.
class cUORxWalkRequest : public cUOPacket
{
public:
	cUORxWalkRequest( const QByteArray& data ) : cUOPacket( data ) {}
	UINT8 key()			{ return (*this)[2]; }
	UINT8 direction()	{ return (*this)[1]; }
	UINT32 fastWalkKey(){ return getInt(3);	 }
};

// 0xAD Speech Request
class cUORxSpeechRequest: public cUOPacket
{
public:
	cUORxSpeechRequest( const QByteArray& data ) : cUOPacket( data ) {}
	UINT8 type()		{ return (*this)[3]; }
	UINT16 color()		{ return getShort( 4 ); }
	UINT16 font()		{ return getShort( 6 ); }
	UINT16 keywordCount() { return getShort( 12 ) >> 4; }
	QString language()	{ return &rawPacket.data()[12]; }
	QString message();
};

//0x06 Double Click
class cUORxDoubleClick : public cUOPacket
{
public:
	cUORxDoubleClick( const QByteArray& data ) : cUOPacket( data )	{}
	SERIAL serial()		{ return getInt(1)&0x7FFFFFFF; }
	bool   keyboard()   { return (*this)[1] & 0x80; }
};

//0xA7 Get Tip
class cUORxGetTip : public cUOPacket
{
public:
	cUORxGetTip( const QByteArray& data ) : cUOPacket( data ) {}
	UINT16 lastTip()	{ return getShort(1);		}
	bool   isTip()		{ return (*this)[3] == 0;	}
	bool   isNotice()	{ return (*this)[3] == 1;	}
};

// 0x93 UpdateBook
class cUORxUpdateBook: public cUOPacket
{
public:
	cUORxUpdateBook( const QByteArray &data ): cUOPacket( data ) {}
	UINT32 serial() { return getInt( 1 ); }
	UINT32 unknown() { return getInt( 5 ); }
	QString title() { return &rawPacket.data()[9]; }
	QString author() { return &rawPacket.data()[69]; }
};

// 0x75 Rename Character
class cUORxRename: public cUOPacket
{
public:
	cUORxRename( const QByteArray &data ): cUOPacket( data ) {}
	UINT32 serial() { return getInt( 1 ); }
	QString name() { return &rawPacket.data()[5]; }
};

// 0x72 Change Warmode
class cUORxChangeWarmode: public cUOPacket
{
public:
	cUORxChangeWarmode( const QByteArray &data ): cUOPacket( data ) {}
	bool warmode() { return ( rawPacket[1] == 0 ) ? false : true; }
};

// 0x6C Target
class cUORxTarget: public cUOPacket
{
public:
	cUORxTarget( const QByteArray &data ): cUOPacket( data ) {}
	UINT8 type() { return rawPacket[1]; }
	UINT32 targetSerial() { return getInt( 2 ); }
	UINT8 cursorType() { return rawPacket[6]; }
	UINT32 serial() { return getInt( 7 ); }
	UINT16 x() { return getShort( 11 ); }
	UINT16 y() { return getShort( 13 ); }
	UINT8 unknown() { return rawPacket[15]; }
	INT8 z() { return rawPacket[16]; }
	UINT16 model() { return getShort( 17 ); }
};

// 0x22 ResyncWalk
class cUORxResyncWalk: public cUOPacket
{
public:
	cUORxResyncWalk( const QByteArray &data ): cUOPacket( data ) {}
	UINT8 sequence() { return rawPacket[1]; }
};

// 0x07 Drag Items
class cUORxDragItem: public cUOPacket
{
public:
	cUORxDragItem( const QByteArray &data ): cUOPacket( data ) {}
	UINT16 amount() { return getShort( 5 ); }
	UINT32 serial() { return getInt( 1 ); }
};

// 0x08 Drop Items
class cUORxDropItem: public cUOPacket
{
public:
	cUORxDropItem( const QByteArray &data ): cUOPacket( data ) {}
	UINT32 serial() { return getInt( 1 ); }
	UINT16 x() { return getShort( 5 ); }
	UINT16 y() { return getShort( 7 ); }
	INT8 z() { return rawPacket[9]; }
	UINT32 cont() { return getInt( 10 ); }
};

// 0x13 Wear Item
class cUORxWearItem: public cUOPacket
{
public:
	cUORxWearItem( const QByteArray &data ): cUOPacket( data ) {}
	UINT32 serial() { return getInt( 1 ); }
	UINT8 layer() { return rawPacket[5]; }
	UINT32 wearer() { return getInt( 6 ); }
};

#endif

