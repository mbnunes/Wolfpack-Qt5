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
#include "qstringlist.h"

// Wolfpack Includes
#include "../typedefs.h"

cUOPacket *getUOPacket( const QByteArray &data );

// 0x12: Action
class cUORxAction: public cUOPacket
{
public:
	cUORxAction( const QByteArray &data ): cUOPacket( data ) {}
	UINT8 type() const		{ return (*this)[3]; }
	QString action() const	{ return this->getAsciiString(4, getShort(1) - 4 ); }
};

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

	UINT32 pattern( void ) const	{ return getInt( 1 ); }
	eType type( void ) const		{ return ( (*this)[5] == 0x05 ) ? Skills : Stats; }
	UINT32 serial( void ) const		{ return getInt( 6 ); }
};

// 0x00: Create Char
class cUORxCreateChar: public cUOPacket
{
public:
	cUORxCreateChar( const QByteArray &data ): cUOPacket( data ) {}
	UINT32 pattern1( void ) const		{ return getInt( 1 ); }
	UINT32 pattern2( void ) const		{ return getInt( 5 ); }
	UINT8 pattern3( void ) const		{ return (*this)[ 9 ]; }
	QString name( void ) const			{ return this->getAsciiString(10, 30); }
	QString password( void ) const		{ return this->getAsciiString(40, 30); }
	UINT8 gender( void ) const			{ return (*this)[70]; } // 0 = male, 1 = female
	UINT8 strength( void ) const		{ return (*this)[71]; }
	UINT8 dexterity( void )	const		{ return (*this)[72]; }
	UINT8 intelligence( void ) const	{ return (*this)[73]; }
	UINT8 skillId1( void ) const		{ return (*this)[74]; }
	UINT8 skillValue1( void ) const		{ return (*this)[75]; }
	UINT8 skillId2( void ) const		{ return (*this)[76]; }
	UINT8 skillValue2( void ) const		{ return (*this)[77]; }
	UINT8 skillId3( void ) const		{ return (*this)[78]; }
	UINT8 skillValue3( void ) const		{ return (*this)[79]; }
	INT16 skinColor( void ) const		{ return getShort( 80 ); }
	INT16 hairStyle( void ) const		{ return getShort( 82 ); }
	INT16 hairColor( void ) const		{ return getShort( 84 ); }
	INT16 beardStyle( void ) const		{ return getShort( 86 ); }
	INT16 beardColor( void ) const		{ return getShort( 88 ); }
	// Here is an unkown byte (!)
	UINT8 startTown( void ) const		{ return (*this)[91]; }
	UINT16 unknown1( void ) const		{ return getShort( 92 ); }
	UINT16 slot( void ) const			{ return getShort( 94 ); }
	UINT32 ip( void ) const				{ return getInt( 96 ); }
	INT16 shirtColor( void ) const		{ return getShort( 100 ); }
	INT16 pantsColor( void ) const		{ return getShort( 102 ); }
};

// 0xC8: UpdateRange
class cUORxUpdateRange: public cUOPacket
{
public:
	cUORxUpdateRange( const QByteArray &data ): cUOPacket( data ) {}
	UINT8 range( void ) const { return (*this)[1]; }
};

// 0x01: NotifyDisconnect
class cUORxNotifyDisconnect: public cUOPacket
{
public:
	cUORxNotifyDisconnect( const QByteArray &data ): cUOPacket( data ) {}
	UINT32 pattern( void ) const { return getInt( 1 ); }
};

// 0x80: Login Request
class cUORxLoginRequest: public cUOPacket
{
public:
	cUORxLoginRequest( const QByteArray &data ): cUOPacket( data ) {}
	QString username( void ) const { return this->getAsciiString(1, 30); }
	QString password( void ) const { return this->getAsciiString(31,30); }
};

// 0xA4: Hardware Info
class cUORxHardwareInfo: public cUOPacket
{
public:
	cUORxHardwareInfo( const QByteArray &data ): cUOPacket( data ) {}
	UINT8 processorType( void ) const		{ return (*this)[ 0x01 ]; }
	UINT16 processorSpeed( void ) const		{ return getShort( 0x02 ); }
	UINT8 processorCount( void ) const		{ return (*this)[ 0x04 ]; }
	// QString directory -- unicode ? -- > 20 bytes useless trash
	// Video Card descritor: -- unicode ? -- > 20 bytes useless trash
	// ??? > another 20 byte trash field
	// ??? > ANOTHER 20 byte trash field
	UINT16 memoryInMb( void ) const			{ return getShort( 0x85 ); }
	UINT16 largestPartitionInMb( void ) const { return getShort( 0x87 ); }
	INT32 timezoneBias( void ) const		{ return getInt( 0x8C ); }
};

// 0xA0: Select Shard
class cUORxSelectShard: public cUOPacket
{
public:
	cUORxSelectShard( const QByteArray &data ): cUOPacket( data ) {}
	UINT16 shardId( void ) const { return getShort( 1 ); }
};

// 0x91: Charlist Request
class cUORxServerAttach: public cUOPacket
{
public:
	cUORxServerAttach( const QByteArray &data ): cUOPacket( data ) {}
	UINT32 authId( void ) const		{ return getInt( 1 ); }
	QString username( void ) const	{ return this->getAsciiString(5, 30); }
	QString password( void ) const	{ return this->getAsciiString(35,30); }
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

	QString password( void ) const	{ return this->getAsciiString(1, 30); }
	UINT32 index( void ) const		{ return getInt( 31 ); }
	UINT32 ip( void ) const			{ return getInt( 35 ); }
};

// 0x5D Play Character
class cUORxPlayCharacter: public cUOPacket
{
public:
	cUORxPlayCharacter( const QByteArray &data ): cUOPacket( data ) {}

	QString character( void ) const	{ return this->getAsciiString(5, 30); }
	QString password( void ) const	{ return this->getAsciiString(35, 30); }
	UINT32 slot( void ) const		{ return getInt( 65 ); }
	UINT32 ip( void ) const			{ return getInt( 69 ); }
};

// 0x05 Request Use
class cUORxRequestAttack: public cUOPacket
{
public:
	cUORxRequestAttack( const QByteArray &data ): cUOPacket( data ) {}

	UINT32 serial( void ) const		{ return getInt( 1 ); }
};

// 0x09 Request Look
class cUORxRequestLook: public cUOPacket
{
public:
	cUORxRequestLook( const QByteArray &data ): cUOPacket( data ) {}

	UINT32 serial( void ) const		{ return getInt( 1 ); }
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

	eChoice choice( void ) const	{ return ( (*this)[1] == 0x01 ) ? Resurrect : Ghost; }
};

// 0xBF Multi Purpose Packet -> Split up into other packets later
class cUORxMultiPurpose: public cUOPacket
{
public:
	enum eSubCommands
	{
		unknown = 0,
		initFastWalk,
		contextMenuRequest = 0x13,
		contextMenuSelection = 0x15,
		setLanguage = 0x0B,
		
	};

	cUORxMultiPurpose( const QByteArray &data ): cUOPacket( data ) {}
	eSubCommands subCommand( void ) const { return (eSubCommands)getShort( 3 ); }
	static cUOPacket *packet( const QByteArray& data );
};

// 0xBF 0x13 PopUp Menu Request
class cUORxContextMenuRequest: public cUORxMultiPurpose
{
public:
	cUORxContextMenuRequest( const QByteArray &data ): cUORxMultiPurpose( data ) {}
	UINT32 serial( void ) const { return getInt( 5 ); }
};

// 0xBF 0x15 PopUp Menu Selection
class cUORxContextMenuSelection: public cUORxMultiPurpose 
{ 
public: 
	cUORxContextMenuSelection( const QByteArray &data ): cUORxMultiPurpose( data ) {} 
	
	Q_UINT32 CharID( void ) const { return getInt( 5 ); } 
	Q_UINT16 EntryTag( void ) const { return getShort( 9 ); } 
}; 

// 0xBF 0x0B Set Client Language
class cUORxSetLanguage: public cUORxMultiPurpose
{
public:
	cUORxSetLanguage( const QByteArray &data ): cUORxMultiPurpose( data ) {}
	QString language( void ) const { return this->getAsciiString(5, 4); }
};

// 0xBD Set Version
class cUORxSetVersion: public cUOPacket
{
public:
	cUORxSetVersion( const QByteArray &data ): cUOPacket( data ) {}
	QString version( void ) const { return this->getAsciiString(3, getShort(1) - 3); }
};

// 0x02 Walk Request.
class cUORxWalkRequest : public cUOPacket
{
public:
	cUORxWalkRequest( const QByteArray& data ) : cUOPacket( data ) {}
	UINT8 key() const			{ return (*this)[2]; }
	UINT8 direction() const		{ return (*this)[1]; }
	UINT32 fastWalkKey() const	{ return getInt(3);	 }
};

// 0xAD Speech Request
class cUORxSpeechRequest: public cUOPacket
{
public:
	cUORxSpeechRequest( const QByteArray& data ) : cUOPacket( data ) {}
	UINT8 type() const		{ return (*this)[3]; }
	UINT16 color() const	{ return getShort( 4 ); }
	UINT16 font() const		{ return getShort( 6 ); }
	UINT16 keywordCount() const	{ return getShort( 12 ) >> 4; }
	QString language() const	{ return this->getAsciiString(12, 4); }
	QString message();
};

//0x06 Double Click
class cUORxDoubleClick : public cUOPacket
{
public:
	cUORxDoubleClick( const QByteArray& data ) : cUOPacket( data )	{}
	SERIAL serial() const		{ return getInt(1)&0x7FFFFFFF; }
	bool   keyboard() const		{ return (*this)[1] & 0x80; }
};

//0xA7 Get Tip
class cUORxGetTip : public cUOPacket
{
public:
	cUORxGetTip( const QByteArray& data ) : cUOPacket( data ) {}
	UINT16 lastTip() const	{ return getShort(1);		}
	bool   isTip() const	{ return (*this)[3] == 0;	}
	bool   isNotice() const	{ return (*this)[3] == 1;	}
};

// 0x93 UpdateBook
class cUORxUpdateBook: public cUOPacket
{
public:
	cUORxUpdateBook( const QByteArray &data ): cUOPacket( data ) {}
	UINT32 serial() const	{ return getInt( 1 ); }
	UINT32 unknown() const	{ return getInt( 5 ); }
	QString title() const	{ return this->getAsciiString(9, 60); }
	QString author() const	{ return this->getAsciiString(69, 30); }
};

// 0x75 Rename Character
class cUORxRename: public cUOPacket
{
public:
	cUORxRename( const QByteArray &data ): cUOPacket( data ) {}
	UINT32 serial() const	{ return getInt( 1 ); }
	QString name() const	{ return this->getAsciiString(5, 30); }
};

// 0x72 Change Warmode
class cUORxChangeWarmode: public cUOPacket
{
public:
	cUORxChangeWarmode( const QByteArray &data ): cUOPacket( data ) {}
	bool warmode() const	{ return ( (*this)[1] == 0 ) ? false : true; }
};

// 0x6C Target
class cUORxTarget: public cUOPacket
{
public:
	cUORxTarget( const QByteArray &data ): cUOPacket( data ) {}
	UINT8 type() const			{ return (*this)[1]; }
	UINT32 targetSerial() const	{ return getInt( 2 ); }
	UINT8 cursorType() const	{ return (*this)[6]; }
	UINT32 serial()	const		{ return getInt( 7 ); }
	UINT16 x() const			{ return getShort( 11 ); }
	UINT16 y() const			{ return getShort( 13 ); }
	UINT8 unknown() const		{ return (*this)[15]; }
	INT8 z() const				{ return (*this)[16]; }
	UINT16 model() const		{ return getShort( 17 ); }
};

// 0x22 ResyncWalk
class cUORxResyncWalk: public cUOPacket
{
public:
	cUORxResyncWalk( const QByteArray &data ): cUOPacket( data ) {}
	UINT8 sequence() const		{ return (*this)[1]; }
};

// 0x07 Drag Items
class cUORxDragItem: public cUOPacket
{
public:
	cUORxDragItem( const QByteArray &data ): cUOPacket( data ) {}
	UINT16 amount() const		{ return getShort( 5 ); }
	UINT32 serial() const		{ return getInt( 1 ); }
};

// 0x08 Drop Items
class cUORxDropItem: public cUOPacket
{
public:
	cUORxDropItem( const QByteArray &data ): cUOPacket( data ) {}
	UINT32 serial() const	{ return getInt( 1 ); }
	UINT16 x() const		{ return getShort( 5 ); }
	UINT16 y() const		{ return getShort( 7 ); }
	INT8 z() const			{ return (*this)[9]; }
	UINT32 cont() const		{ return getInt( 10 ); }
};

// 0x13 Wear Item
class cUORxWearItem: public cUOPacket
{
public:
	cUORxWearItem( const QByteArray &data ): cUOPacket( data ) {}
	UINT32 serial()	const	{ return getInt( 1 ); }
	UINT8 layer() const		{ return (*this)[5]; }
	UINT32 wearer() const	{ return getInt( 6 ); }
};

// 0x66 Book Page
class cUORxBookPage: public cUOPacket
{
public:
	cUORxBookPage( const QByteArray &data ): cUOPacket( data ) {}
	UINT16 size() const			{ return getShort( 1 ); }
	UINT32 serial() const		{ return getInt( 3 ); }
	UINT16 page() const			{ return getShort( 9 ); }
	UINT16 numOfLines() const	{ return getShort( 11 ); }

	QStringList lines();
};

// 0xB1 Gump Response
class cUORxGumpResponse : public cUOPacket
{
public:
	cUORxGumpResponse( const QByteArray &data ): cUOPacket( data ) {}
	UINT16 size() const		{ return getShort( 1 ); }
	UINT32 serial() const	{ return getInt( 3 ); }
	UINT32 type() const		{ return getInt( 7 ); }
	gumpChoice_st choice();
};

// 0x3B Buy
class cUORxBuy: public cUOPacket
{
public:
	cUORxBuy( const QByteArray &data ): cUOPacket( data ) {}
	UINT32 serial() const				{ return getInt( 3 ); }
	UINT16 itemCount() const			{ return ( ( size() - 8 ) / 7 ); }
	UINT8 iLayer( UINT16 item ) const	{ return (*this)[ 8 + ( item * 7 ) ]; }
	UINT32 iSerial( UINT16 item ) const	{ return getInt( 9 + ( item * 7 ) ); }
	UINT16 iAmount( UINT16 item ) const	{ return getShort( 13 + ( item * 7 ) ); }
};

// 0x95 Dye
class cUORxDye: public cUOPacket
{
public:
	cUORxDye( const QByteArray &data ): cUOPacket( data ) {}
	UINT32 serial() const	{ return getInt( 1 ); }
	UINT16 model() const	{ return getShort( 5 ); }
	UINT16 color() const	{ return getShort( 7 ); }
};

// 0x9B Help Request - nice one :D
class cUORxHelpRequest : public cUOPacket
{
public:
	cUORxHelpRequest( const QByteArray &data ) : cUOPacket( data ) {}
};

// 0x3A Set Skill Lock
class cUORxSkillLock : public cUOPacket
{
public:
	cUORxSkillLock( const QByteArray &data ): cUOPacket( data ) {}
	UINT16 skill() const		{ return getShort( 3 ); }
	UINT8 lock() const			{ return (*this)[ 5 ]; }
};

#endif


