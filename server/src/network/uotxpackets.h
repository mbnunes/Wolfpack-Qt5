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

#ifndef __UO_TXPACKETS__
#define __UO_TXPACKETS__

// Library includes
#include "qcstring.h"
#include "qstring.h"
#include "qstringlist.h"
#include <vector>

#include "uopacket.h"

enum eDenyLogin
{
	DL_NOACCOUNT = 0x00,
	DL_INUSE,
	DL_BLOCKED,
	DL_BADPASSWORD,
	DL_BADCOMMUNICATION
};

// 0x82: DenyLogin
class cUOTxDenyLogin: public cUOPacket
{
public:
	cUOTxDenyLogin( eDenyLogin reason ): cUOPacket( 2 )
	{
		rawPacket[0] = (Q_UINT8)0x82;
		rawPacket[1] = reason;
	}
};

// 0x81: Accept Login ( Sends Serverlist )
// BYTE, WORD, BYTE (count), BYTE(UNK), CHARLIST
class cUOTxAcceptLogin: public cUOPacket
{
public:
	cUOTxAcceptLogin(): cUOPacket( 3 )
	{
		rawPacket[0] = (Q_UINT8)0x81;
	}
};

// 0xA8: Shard List
class cUOTxShardList: public cUOPacket
{
public:
	cUOTxShardList(): cUOPacket( 6 )
	{
		rawPacket[0] = (Q_UINT8)0xA8;
		rawPacket[3] = (Q_UINT8)0x64;
		setShort( 1, 6 ); // Packet Size
	}

	// Server ip is used for pinging
	virtual void addServer( Q_UINT16 serverIndex, QString serverName, Q_UINT8 serverFull = 0, Q_INT8 serverTimeZone = 0, Q_UINT32 serverIp = 0 );
};

// 0x8C: RelayServer
class cUOTxRelayServer: public cUOPacket
{
public:
	cUOTxRelayServer(): cUOPacket( 11 ) {
		rawPacket[ 0 ] = (Q_UINT8)0x8C;
	}

	virtual void setServerIp( Q_UINT32 data ) { setInt( 1, data ); }
	virtual void setServerPort( Q_UINT16 port ) { setShort( 5, port ); }
	virtual void setAuthId( Q_UINT32 authId ) { setInt( 7, authId ); }
};

struct stTown
{
	Q_UINT8 index;
	QString town, area;
};

struct stChar
{
	QString name;
	QString password;
};

// 0xA9: CharTownList
class cUOTxCharTownList: public cUOPacket
{
protected:
	QStringList characters;
	std::vector< stTown > towns;
	Q_UINT32 flags;
	Q_INT16 charLimit;
public:
	cUOTxCharTownList(): charLimit( -1 ), cUOPacket( 9 ), flags( 0x8 ) {}

	virtual void addCharacter( QString name );
	virtual void addTown( Q_UINT8 index, const QString &name, const QString &area );
	virtual void setCharLimit( Q_INT16 limit = -1 ) { charLimit = limit; }
	virtual void compile();
};

enum eCharChangeResult
{
	CCR_BADPASS = 0,
	CCR_DOESNTEXISTS,
	CCR_INUSE,
	CCR_NOTOLDENOUGH,
	CCR_QUEUEDFORBACKUP,
};

// 0x85 CharChangeResult
class cUOTxCharChangeResult: public cUOPacket
{
public:
	cUOTxCharChangeResult(): cUOPacket( 2 ) {
		rawPacket[ 0 ] = (Q_UINT8)0x85;
	}

	void setResult( eCharChangeResult data ) { rawPacket[ 1 ] = data; }
};

// 0x86 UpdateCharList
class cUOTxUpdateCharList: public cUOPacket
{
public:
	cUOTxUpdateCharList(): cUOPacket( 304 ) {
		rawPacket[ 0 ] = (Q_UINT8)0x86;
		setShort( 1, 304 );
	}

	void setCharacter( Q_UINT8 index, QString name );
};

// 0x1B ConfirmLogin
class cUOTxConfirmLogin: public cUOPacket
{
public:
	cUOTxConfirmLogin(): cUOPacket( 37 ) {
		rawPacket[0] = 0x1B;
	}

	void setSerial( Q_UINT32 serial ) { setInt( 1, serial ); }
	void setUnknown1( Q_UINT32 data ) { setInt( 5, data ); }
	void setBody( Q_UINT16 data ) { setShort( 9, data ); }
	void setX( Q_UINT16 data ) { setShort( 11, data ); }
	void setY( Q_UINT16 data ) { setShort( 13, data ); }
	void setZ( Q_INT16 data ) { setShort( 15, data ); }
	void setDirection( Q_UINT8 data ) { rawPacket[ 17 ] = data; }
	void setUnknown2( Q_UINT16 data ) { setShort( 18, data ); }
	void setUnknown3( Q_UINT32 data ) { setInt( 20, data ); }
	void setUnknown4( Q_UINT32 data ) { setInt( 24, data ); }
	void setFlags( Q_UINT8 data ) { rawPacket[ 28 ] = data; }
	void setHighlight( Q_UINT8 data ) { rawPacket[ 29 ] = data; }
	void setUnknown5( char data[7] ) { memcpy( &rawPacket.data()[30], data, 7 ); }
};

enum eMapType
{
	MT_FELUCCA = 0,
	MT_TRAMMEL,
	MT_ILSHENAR
};

// 0xBF Change Map ( Subcommand: 0x08 )
class cUOTxChangeMap: public cUOPacket
{
public:
	cUOTxChangeMap(): cUOPacket( 6 ) 
	{
		rawPacket[ 0 ] = 0xBF;
		setShort( 1, 6 ); // Packet Length
		setShort( 3, 0x08 ); // Subcommand
	}

	void setMap( eMapType data ) { rawPacket[ 5 ] = data; }
};

enum eSeasonType
{
	ST_SPRING = 0,
	ST_SUMMER,
	ST_FALL,
	ST_WINTER,
	ST_DESOLATION
};

// 0xBC Change Season
class cUOTxChangeSeason: public cUOPacket
{
public:
	cUOTxChangeSeason(): cUOPacket( 3 )
	{
		rawPacket[ 0 ] = 0xBC;
		rawPacket[ 1 ] = 1;
	}

	void setSeason( eSeasonType data ) { rawPacket[ 2 ] = data; }
};

enum eWeatherType
{
	WT_RAINING = 0,
	WT_FIERCESTORM,
	WT_SNOWING,
	WT_STORM,
	WT_TEMPERATURE = 0xFE,
	WT_NONE
};

// 0x65 Weather
class cUOTxWeather: public cUOPacket
{
	cUOTxWeather(): cUOPacket( 4 ) {
		rawPacket[ 0 ] = 0x65;
		rawPacket[ 1 ] = 0xFF;
	}

	void setType( eWeatherType data ) { rawPacket[ 1 ] = data; }
	void setAmount( Q_UINT8 data ) { rawPacket[ 2 ] = data; }
	void setTemperature( Q_UINT8 data ) { rawPacket[ 3 ] = data; }
};

// 0x55 StartGame
class cUOTxStartGame: public cUOPacket
{
public:
	cUOTxStartGame(): cUOPacket( 1 ) {
		rawPacket[ 0 ] = 0x55;
	}
};

// 0xBA QuestPointer
class cUOTxQuestPointer: public cUOPacket
{
public:
	cUOTxQuestPointer(): cUOPacket( 6 )
	{
		rawPacket[ 0 ] = 0xBA;
	}

	void setActive( bool data ) { rawPacket[ 1 ] = data ? 1 : 0; }
	void setX( Q_UINT16 data ) { setShort( 2, data ); }
	void setY( Q_UINT16 data ) { setShort( 4, data ); }
};

// 0xB9 ClientFeatures
class cUOTxClientFeatures: public cUOPacket
{
public:
	cUOTxClientFeatures(): cUOPacket( 3 ) {
		rawPacket[ 0 ] = (Q_UINT8)0xB9;
	};

	void setLbr( bool enable ) { enable ? rawPacket[ 2 ] |= 0x02 : rawPacket[ 2 ] &= 0xFD; }
	void setT2a( bool enable ) { enable ? rawPacket[ 2 ] |= 0x01 : rawPacket[ 2 ] &= 0xFE; }
};

#endif