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
#include "../typedefs.h"
#include "../junk.h"

#include <vector>
#include "uopacket.h"

class Coord_cl;

enum eDenyLogin
{
	DL_NOACCOUNT = 0x00,
	DL_INUSE,
	DL_BLOCKED,
	DL_BADPASSWORD,
	DL_BADCOMMUNICATION,
	DL_OTHER
};

// 0x82: DenyLogin
class cUOTxDenyLogin: public cUOPacket
{
public:
	cUOTxDenyLogin(): cUOPacket( 0x82, 2 ) {}
	void setReason( eDenyLogin data ) { rawPacket[1] = data; }
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
	cUOTxConfirmLogin(): cUOPacket( 0x1B, 37 ) {}

	// This is just here for convenience !! These values are basically unused
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
	void fromChar( P_CHAR pChar );
};

// 0xBF Change Map ( Subcommand: 0x08 )
class cUOTxChangeMap: public cUOPacket
{
public:
//	enum eMapType	{ Felucca = 0, Trammel, Ilshenar };

	cUOTxChangeMap(): cUOPacket( 0xBF, 6 ) 
	{
		setShort( 1, 6 );    // Packet Length
		setShort( 3, 0x08 ); // Subcommand
	}

	void setMap( Q_UINT8 data ) { rawPacket[5] = data; }
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
	cUOTxChangeSeason(): cUOPacket( 0xBC, 3 )
	{
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
public:
	enum eWeatherType
	{
		Raining = 0, Fiercestorm, Snowing, Storm, Temperature = 0xFE, None
	};

	cUOTxWeather(): cUOPacket( 0x65, 4 ) {
		rawPacket[ 1 ] = static_cast<uchar>(0xFF);
	}

	void setType( eWeatherType data )	{ rawPacket[ 1 ] = data; }
	void setAmount( Q_UINT8 data )		{ rawPacket[ 2 ] = data; }
	void setTemperature( Q_UINT8 data ) { rawPacket[ 3 ] = data; }
};

// 0x55 StartGame
class cUOTxStartGame: public cUOPacket
{
public:
	cUOTxStartGame(): cUOPacket( 0x55, 1 ) {}
};

// 0xBA QuestPointer
class cUOTxQuestPointer: public cUOPacket
{
public:
	cUOTxQuestPointer(): cUOPacket( 0xBA, 6 ) {}

	void setActive( bool data )		{ rawPacket[ 1 ] = data ? 1 : 0; }
	void setX( Q_UINT16 data )		{ setShort( 2, data ); }
	void setY( Q_UINT16 data )		{ setShort( 4, data ); }
};

// 0xB9 ClientFeatures
class cUOTxClientFeatures: public cUOPacket
{
public:
	cUOTxClientFeatures(): cUOPacket( 0xB9, 3 ) {}

	void setLbr( bool enable ) { enable ? rawPacket[ 2 ] |= 0x02 : rawPacket[ 2 ] &= 0xFD; }
	void setT2a( bool enable ) { enable ? rawPacket[ 2 ] |= 0x01 : rawPacket[ 2 ] &= 0xFE; }
};

// 0x24 Draw Container
class cUOTxDrawContainer: public cUOPacket
{
public:
        cUOTxDrawContainer(): cUOPacket( 0x24, 7 ) {}
                                
        void setSerial( Q_UINT32 serial ) { setInt( 1, serial ); }
        void setGump( Q_UINT16 gump ) { setShort( 5, gump ); }
};
                                           
// 0x25 AddContainerItem
class cUOTxAddContainerItem: public cUOPacket
{
public:
	cUOTxAddContainerItem(): cUOPacket( 0x25, 20 ) {}
	
	void setSerial( Q_UINT32 serial ) { setInt( 1, serial ); }
	void setModel( Q_UINT16 model ) { setShort( 5, model ); }
	void setUnknown1( Q_UINT8 data ) { rawPacket[ 7 ] = data; }
	void setAmount( Q_UINT16 amount ) { setShort( 8, amount ); }
	void setX( Q_UINT16 data ) { setShort( 10, data ); }
	void setY( Q_UINT16 data ) { setShort( 12, data ); }
	void setContainer( Q_UINT32 data ) { setInt( 14, data ); }
	void setColor( Q_UINT16 data ) { setShort( 18, data ); }
	void fromItem( P_ITEM pItem );
};

// 0x26 KickPlayer

// 0x27 RejectDrag
class cUOTxRejectDrag: public cUOPacket
{
public:
	enum eRejectType
	{
		RejectDrag = 0,
		RejectDrop = 5
	};

	cUOTxRejectDrag(): cUOPacket( 0x27, 2 ) {}
	
	void setRejectType( eRejectType type ) { rawPacket[1] = type; }
};

// 0x28 ClearSquare
class cUOTxClearSquare: public cUOPacket
{
public:
	cUOTxClearSquare(): cUOPacket( 0x28, 5 ) {}
	
	void setX( Q_UINT16 x ) { setShort( 1, x ); }
	void setY( Q_UINT16 y ) { setShort( 3, y ); }
};

// 0x2C Resurrection Menu
class cUOTxResurrectionMenu: public cUOPacket
{
public:
	cUOTxResurrectionMenu(): cUOPacket( 0x2C, 0x02 ) {}
};

// 0x2E CharEquipment
class cUOTxCharEquipment: public cUOPacket
{
public: 
	cUOTxCharEquipment(): cUOPacket( 0x2E, 15 ) {}
		
	void setSerial( Q_UINT32 data ) { setInt( 1, data ); }
	void setModel( Q_UINT16 model ) { setShort( 5, model ); }
	void setUnknown1( Q_UINT8 data ) { rawPacket[ 7 ] = data; }
	void setLayer( Q_UINT8 layer ) { rawPacket[ 8 ] = layer; }
	void setWearer( Q_UINT32 serial ) { setInt( 9, serial ); }
	void setColor( Q_UINT16 data ) { setShort( 13, data ); }
	void fromItem( P_ITEM pItem );
};

// 0x2F ShowBattle
class cUOTxShowBattle: public cUOPacket
{
public:
	cUOTxShowBattle(): cUOPacket( 0x2F, 10 ) {}

	void setUnknown1( Q_UINT8 data ) { rawPacket[1] = data; }
	void setAttacker( Q_UINT32 data ) { setInt( 2, data ); }
	void setVictim( Q_UINT32 data ) { setInt( 6, data ); }
};

// 0x33 Pause
class cUOTxPause: public cUOPacket
{
public:
	cUOTxPause(): cUOPacket( 0x33, 2 ) {}
	void pause( void ) { rawPacket[1] = 1; }
	void resume( void ) { rawPacket[1] = 0; }
};

// 0x3A UpdateSkill (one skill)
class cUOTxUpdateSkill: public cUOPacket
{
public:
	cUOTxUpdateSkill(): cUOPacket( 0x3A, 11 ) 
	{ 
		setShort( 1, 11 );
		rawPacket[3] = 0xFF;
	}

	enum eStatus
	{
		Up = 0,
		Down = 1,
		Locked = 2
	};

	void setId( UINT16 data ) { setShort( 4, data ); }
	void setValue( UINT16 data ) { setShort( 6, data ) ; }
	void setRealValue( UINT16 data ) { setShort( 8, data ); }
	void setStatus( eStatus data ) { rawPacket[10] = data; }
};

// 0x3A SendSkills( multiple skills )
class cUOTxSendSkills: public cUOPacket
{
public:
	cUOTxSendSkills(): cUOPacket( 0x3A, 6 ) {
		setShort( 1, 6 );
	}

	enum eStatus
	{
		Up = 0,
		Down = 1,
		Locked = 2
	};

	void addSkill( Q_UINT16 skillId, Q_UINT16 skill, Q_UINT16 realSkill, eStatus status );
	void fromChar( P_CHAR pChar );
};

// 0x20 DrawPlayer
class cUOTxDrawPlayer: public cUOPacket
{
public:
	cUOTxDrawPlayer(): cUOPacket( 0x20, 19 ) {}

	void setSerial( Q_UINT32 data ) { setInt( 1, data ); }
	void setBody( Q_UINT16 data ) { setShort( 5, data ); }
	void setUnknown1( Q_UINT8 data ) { rawPacket[ 7 ] = data; }
	void setSkin( Q_UINT16 data ) { setShort( 8, data ); }
	void setFlag( Q_UINT8 data ) { rawPacket[ 10 ] = data; } // // 10 = 0=normal, 4=poison, 0x40=attack, 0x80=hidden CHARMODE_WAR
	UINT8 flag() { return rawPacket[ 10 ]; }
	void setX( Q_UINT16 x ) { setShort( 11, x ); }
	void setY( Q_UINT16 y ) { setShort( 13, y ); }
	void setUnknown2( Q_UINT16 data ) { setShort( 15, data ); }
	void setDirection( Q_UINT8 data ) { rawPacket[ 17 ] = data; }
	void setZ( Q_INT8 data ) { rawPacket[ 18 ] = data; }

	void fromChar( P_CHAR pChar );
};

// 0x77 UpdatePlayer
class cUOTxUpdatePlayer: public cUOPacket
{
public:
	cUOTxUpdatePlayer(): cUOPacket( 0x77, 17 ) {}

	void setSerial( Q_UINT32 data ) { setInt( 1, data ); }
	void setBody( Q_UINT16 data ) { setShort( 5, data ); }
	void setX( Q_UINT16 data ) { setShort( 7, data ); }
	void setY( Q_UINT16 data ) { setShort( 9, data ); }
	void setZ( Q_INT8 data ) { (*this)[11] = data; }
	void setDirection( Q_UINT8 data ) { (*this)[12] = data; }
	void setHue( Q_UINT16 data ) { setShort( 13, data ); }
	void setFlag( Q_UINT8 data ) { (*this)[15] = data; }
	UINT8 flag() { return (*this)[15]; }
	void setHighlight( Q_UINT8 data ) { (*this)[16] = data; }

	void fromChar( P_CHAR pChar );
};

// 0x78 DrawChar
class cUOTxDrawChar: public cUOPacket
{
public:
	cUOTxDrawChar(): cUOPacket( 0x78, 26 ) {
		setShort( 1, 26 );
		setInt( 22, 0 );
	}
	
	void setSerial( Q_UINT32 data ) { setInt( 3, data ); }
	void setModel( Q_UINT16 data ) { setShort( 7, data ); }
	void setX( Q_UINT16 data ) { setShort( 9, data ); }
	void setY( Q_UINT16 data ) { setShort( 11, data ); }	
	void setZ( Q_INT8 data ) { rawPacket[13] = data;  }
	void setDirection( Q_UINT8 data ) { rawPacket[14] = data; }
	void setColor( Q_UINT16 data ) { setShort( 15, data ); }
    void setFlag( Q_UINT8 data ) { rawPacket[17] = data; }
	UINT8 flag() { return rawPacket[17]; }
	void setHighlight( Q_UINT8 data ) { rawPacket[18] = data; }
	void fromChar( P_CHAR pChar );
	
	// The last 4 bytes are the terminator
	void addEquipment( Q_UINT32 serial, Q_UINT16 model, Q_UINT8 layer, Q_UINT16 color );
};

// 0x69: Options(?)
class cUOTxOptions: public cUOPacket
{
public:
	cUOTxOptions(): cUOPacket( 0x69, 5 ) { setShort( 1, 5 ); }
	void setOption( Q_UINT8 data ) { rawPacket[3] = data; }
};

// 0x5b GameTime
class cUOTxGameTime: public cUOPacket
{
public:
	cUOTxGameTime(): cUOPacket( 0x5b, 4 ) {}
	void setTime( Q_UINT8 hour, Q_UINT8 minute, Q_UINT8 second ) {
		rawPacket[1] = hour;
		rawPacket[2] = minute;
		rawPacket[3] = second;
	}
};

// 0xAE UnicodeSpeech
class cUOTxUnicodeSpeech: public cUOPacket
{
public:
	cUOTxUnicodeSpeech(): cUOPacket( 0xAE, 50 ) { setShort(1, 50 ); }

	enum eSpeechType
	{
		Regular = 0x00,
		Broadcast,
		Emote,
		System = 0x06,
		Whisper = 0x08,
		Yell = 0x09
	};

	void setSource( SERIAL data ) { setInt( 3, data ); }
	void setModel( Q_UINT16 data ) { setShort( 7, data ); }
	void setType( eSpeechType data ) { rawPacket[ 9 ] = data; }
	void setColor( Q_UINT16 data ) { setShort( 10, data ); }
	void setFont( Q_UINT16 font ) { setShort( 12, font ); }
	void setLanguage( const QString &data ) { memcpy( &rawPacket.data()[14], data.latin1(), MIN( data.length()+1, 4 ) ); }
	void setName( const QString &data ) { memcpy( &rawPacket.data()[18], data.latin1(), MIN( data.length()+1, 30 ) ); }
	void setText( const QString &data );
};

// 0x11 SendStats
class cUOTxSendStats: public cUOPacket
{
public:
	cUOTxSendStats(): cUOPacket( 0x11, 70 ) { setShort( 1, 0x42 ); }

	void setFullMode( bool mode, bool extended = false ) 
	{ 
		if( extended )
		{
			setShort( 1, 0x46 );
			rawPacket[42] = 0x03;
		}
		else
			rawPacket[42] = mode ? 0x01 : 0x00;
	}

	void setAllowRename( bool mode ) { rawPacket[41] = mode ? 0x01 : 0x00; }
	void setSerial( SERIAL serial ) { setInt( 3, serial ); }
	void setName( const QString &name ) { setAsciiString( 7, name, MIN( name.length()+1, 30 ) ); }
	void setHp( Q_UINT16 data ) { setShort( 37, data ); }
	void setMaxHp( Q_UINT16 data ) { setShort( 39, data ); }
	void setSex( bool male ) { rawPacket[43] = male ? 0 : 1; }
	void setStrength( Q_UINT16 data ) { setShort( 44, data ); }
	void setDexterity( Q_UINT16 data ) { setShort( 46, data ); }
	void setIntelligence( Q_UINT16 data ) { setShort( 48, data ); }
	void setStamina( Q_UINT16 data ) { setShort( 50, data ); }
	void setMaxStamina( Q_UINT16 data ) { setShort( 52, data ); }
	void setMana( Q_UINT16 data ) { setShort( 54, data ); }
	void setMaxMana( Q_UINT16 data ) { setShort( 56, data ); }
	void setGold( Q_UINT32 data ) { setInt( 58, data ); }
	void setArmor( Q_UINT16 data ) { setShort( 62, data ); }
	void setWeight( Q_UINT16 data ) { setShort( 64, data ); }
	// extended for newer clients
	void setStatCap( Q_UINT16 data ) { setShort( 66, data ); }
	void setPets( Q_UINT8 data ) { rawPacket[ 68 ] = data; }
	void setMaxPets( Q_UINT8 data ) { rawPacket[ 69 ] = data; }
};

// 0xBF Subcommand: 0x14
class cUOTxContextMenu: public cUOPacket 
{ 
public: 
	
	cUOTxContextMenu(): cUOPacket( 0xBF, 13 ) 
	{ 
		setShort( 1, 12 ); 
		setShort( 3, 0x14 ); 
		setShort( 5, 0x0001 ); 
	} 
	
	enum { Poplocked = 0x01, Poparrow = 0x02, Popcolor = 0x20 }; 
	
	void setSerial ( Q_UINT32 data ) { setInt( 7, data ); } 
	void setEntNum ( Q_UINT8 data ) { setShort ( 11, data ); } 
	void addEntry ( Q_UINT16 RetVal, Q_UINT16 FileID, Q_UINT16 TextID, Q_UINT16 flags=Popcolor, Q_UINT16 color=0x7FE0 );

};

// 0x21 DenyMove
class cUOTxDenyMove: public cUOPacket
{
public:
	cUOTxDenyMove(): cUOPacket( 0x21, 8 ) {}
	void setSequence( Q_UINT8 data ) { (*this)[1] = data; }
	void setCoord( Coord_cl coord );
	void setDirection( Q_UINT8 data ) { (*this)[6] = data; }
	void fromChar( P_CHAR pChar );
};

// 0x22 AcceptMove
class cUOTxAcceptMove: public cUOPacket
{
public:
	cUOTxAcceptMove(): cUOPacket( 0x22, 3 ) {}
	void setSequence( Q_UINT8 data ) { (*this)[1] = data; }
};

// 0x1D Delete object
class cUOTxDeleteObject: public cUOPacket
{
public:
	cUOTxDeleteObject(): cUOPacket( 0x1D, 5 ) {}
	void setSerial( Q_UINT32 data ) { setInt( 1, data ); }
};

// 0xA6 Tip/Notice Window
class cUOTxTipWindow : public cUOPacket
{
public:
	enum WindowType { Tip = 0, Notice };
	cUOTxTipWindow() : cUOPacket( 0xA6, 10 ) {}
	void setType( WindowType t )	{ (*this)[3] = t;	}
	void setNumber( ushort n )		{ setShort(6, n);	}
	void setMessage(QString m);
};

// 0x88 Open Paperdoll
class cUOTxOpenPaperdoll: public cUOPacket
{
public:
	cUOTxOpenPaperdoll(): cUOPacket( 0x88, 66 ) {}
	void setSerial( UINT32 data ) { setInt( 1, data ); }
	void setName( const QString &name ) { memcpy( &rawPacket.data()[5], name.latin1(), MIN( 60, name.length()+1 ) ); }
	void setFlag( UINT8 flag ) { (*this)[65] = flag; }
	UINT8 flag() { return (*this)[65]; }
	void fromChar( P_CHAR pChar );
};

// 0x93 Book Title
class cUOTxBookTitle: public cUOPacket
{
public:
	cUOTxBookTitle(): cUOPacket( 0x93, 99 ) {}

	void setSerial( UINT32 data ) { setInt( 1, data ); }
	void setWriteable( bool data ) { rawPacket[5] = data ? 1 : 0; }
	void setFlag( UINT8 data ) { rawPacket[6] = data; }
	void setPages( UINT16 data ) { setShort( 7, data ); }
	void setTitle( const QString &title ) { setAsciiString( 9, title.latin1(), 60 ); }
	void setAuthor( const QString &author ) { setAsciiString( 69, author.latin1(), 30 ); }
};

// 0x66 Book Page
class cUOTxBookPage: public cUOPacket
{
public:
	// the size of this packet is variable...
	cUOTxBookPage( UINT32 size ): cUOPacket( 0x66, size ) 
	{
		currPageOffset = 0;
	}

	void setBlockSize( UINT16 data ) { setShort( 1, data ); }
	void setSerial( UINT32 data ) { setInt( 3, data ); }
	void setPages( UINT16 data ) { setShort( 7, data ); }

	void setPage( UINT16 page, UINT16 numLines, const QStringList &lines );
protected:
	UINT16 currPageOffset;
};

// 0x6D Play Music
class cUOTxPlayMusic: public cUOPacket
{
public:
	cUOTxPlayMusic(): cUOPacket( 0x6D, 3 ) {}
	void setId( UINT16 data ) { setShort( 1, data ); }
};

// 0x1d Remove Object
class cUOTxRemoveObject: public cUOPacket
{
public:
	cUOTxRemoveObject(): cUOPacket( 0x1d, 5 ) {}
	void setSerial( UINT32 data ) { setInt( 1, data ); }
};

// 0x1A SendItem
class cUOTxSendItem: public cUOPacket
{
public:
	cUOTxSendItem(): cUOPacket( 0x1A, 20 ) { setShort( 1, 20 ); }
	void setSerial( UINT32 data ) { setInt( 3, data | 0x80000000 ); }
	void setId( UINT16 data ) { setShort( 7, data ); }
	void setAmount( UINT16 data ) { setShort( 9, data ); }
	void setCoord( const Coord_cl &coord )
	{
		setShort( 11, coord.x | 0x8000 );
		setShort( 13, coord.y | 0xC000 );
		rawPacket[16] = coord.z;
	}
	void setDirection( UINT8 data ) { rawPacket[15] = data; }
	void setColor( UINT16 data ) { setShort( 17, data ); }
	void setFlags( UINT8 data ) { rawPacket[19] = data; }
	UINT8 flags() { return rawPacket[19]; }
};

// 0x6C Target
class cUOTxTarget: public cUOPacket
{
public:
	cUOTxTarget(): cUOPacket( 0x6c, 19 ) {}
	void setAllowGround( bool data ) { rawPacket[1] = data ? 1 : 0; }
	void setTargSerial( UINT32 data ) { setInt( 2, data ); }
};

// 0x99 Place
class cUOTxPlace: public cUOPacket
{
public:
	cUOTxPlace(): cUOPacket( 0x99, 26 ) { rawPacket[1] = 0x01; }
	void setTargSerial( UINT32 data ) { setInt( 2, data ); }
	void setModelID( UINT16 data ) { setShort( 18, data ); }
};

// 0x54 SoundEffect
class cUOTxSoundEffect: public cUOPacket
{
public:
	cUOTxSoundEffect(): cUOPacket( 0x54, 12 ) { rawPacket[1] = 1; }
	void setSound( UINT16 data ) { setShort( 2, data ); }
	void setUnknown( UINT16 data ) { setShort( 4, data ); }
	void setCoord( const Coord_cl &coord ) 
	{
		setShort( 6, coord.x );
		setShort( 8, coord.y );
		setShort( 10, coord.z );
	}
};


// 0x27 BounceItem
/*
Information provided by Krrios:
Byte 1 is a reason:
0: "You can not pick that up.",
1: "That is too far away.",
2: "That is out of sight.",
3: "That item dose not belong to you. You'll have to steal it.",
4: "You are already holding an item."
5: no reason
*/
enum eBounceReason
{
	BR_CANNOT_PICK_THAT_UP = 0,
	BR_OUT_OF_REACH,
	BR_OUT_OF_SIGHT,
	BR_BELONGS_TO_SOMEONE_ELSE,
	BR_ALREADY_DRAGGING,
	BR_NO_REASON
};

class cUOTxBounceItem: public cUOPacket
{
public:
	cUOTxBounceItem(): cUOPacket( 0x27, 2 ) { rawPacket[1] = BR_NO_REASON; } // better safe than sorry
	void setReason( eBounceReason reason ) { rawPacket[1] = reason; }
};

// 0xA1 Update Health
class cUOTxUpdateHealth: public cUOPacket
{
public:
	cUOTxUpdateHealth(): cUOPacket( 0xA1, 9 ) {}
	void setSerial( UINT32 data ) { setInt( 1, data ); }
	void setMaximum( UINT16 data ) { setShort( 5, data ); }
	void setCurrent( UINT16 data ) { setShort( 7, data ); }
};

// 0xA2 Update Mana
class cUOTxUpdateMana: public cUOPacket
{
public:
	cUOTxUpdateMana(): cUOPacket( 0xA2, 9 ) {}
	void setSerial( UINT32 data ) { setInt( 1, data ); }
	void setMaximum( UINT16 data ) { setShort( 5, data ); }
	void setCurrent( UINT16 data ) { setShort( 7, data ); }
};

// 0xA3 Update Stamina
class cUOTxUpdateStamina: public cUOPacket
{
public:
	cUOTxUpdateStamina(): cUOPacket( 0xA3, 9 ) {}
	void setSerial( UINT32 data ) { setInt( 1, data ); }
	void setMaximum( UINT16 data ) { setShort( 5, data ); }
	void setCurrent( UINT16 data ) { setShort( 7, data ); }
};

// 0xAA AttackResponse
class cUOTxAttackResponse: public cUOPacket
{
public:
	cUOTxAttackResponse(): cUOPacket( 0xAA, 5 ) {}
	void setSerial( UINT32 data ) { setInt( 1, data ); }
};

// 0x72 Warmode
class cUOTxWarmode: public cUOPacket
{
public:
	cUOTxWarmode(): cUOPacket( 0x72, 5 ) { rawPacket[3] = 0x33; }
	void setStatus( UINT8 data ) { rawPacket[1] = data; }
};

// 0x6E Action
class cUOTxAction: public cUOPacket
{
public:
	cUOTxAction(): cUOPacket( 0x6E, 14 ) {}
	void setSerial( UINT32 data ) { setInt( 1, data ); }
	void setAction( UINT16 data ) { setShort( 5, data ); }
	void setUnknown1( UINT8 data ) { rawPacket[7] = data; }
	void setDirection( UINT8 data ) { rawPacket[8] = data; }
	void setRepeat( UINT16 data ) { setShort( 9, data ); }
	void setBackwards( UINT8 data ) { rawPacket[11] = data; }
	void setRepeatFlag( UINT8 data ) { rawPacket[12] = data; }
	void setSpeed( UINT8 data ) { rawPacket[13] = data; }
};

// 0xAF DeathAction
class cUOTxDeathAction: public cUOPacket
{
public:
	cUOTxDeathAction(): cUOPacket( 0xAF, 13 ) {}
	void setSerial( UINT32 data ) { setInt( 1, data ); }
	void setCorpse( UINT32 data ) { setInt( 5, data ); }
	void setUnknown1( UINT32 data ) { setInt( 9, data ); }
};

// 0x3B ClearBuy
class cUOTxClearBuy: public cUOPacket
{
public:
	cUOTxClearBuy(): cUOPacket( 0x3B, 8 ) { setShort( 1, 8 ); }
	void setSerial( UINT32 data ) { setInt( 3, data ); }
};

// 0xBA QuestArrow
class cUOTxQuestArrow: public cUOPacket
{
public:
	cUOTxQuestArrow(): cUOPacket( 0xBA, 6 ) {}
	void setActive( UINT8 status ) { rawPacket[1] = status; }
	void setPos( const Coord_cl &pos ) 
	{
		setShort( 2, pos.x );
		setShort( 4, pos.y );
	}
};

// 0x89 CorpseEquipment
class cUOTxCorpseEquipment: public cUOPacket
{
public:
	cUOTxCorpseEquipment(): cUOPacket( 0x89, 8 ) { setShort( 1, 8 ); }

	void setSerial( UINT32 data ) { setInt( 3, data ); }
	void addItem( UINT8 layer, UINT32 serial );
};

// 0x2C CharDeath
class cUOTxCharDeath: public cUOPacket
{
public:
	cUOTxCharDeath(): cUOPacket( 0x2C, 2 ) {}
};

// 0x3C ItemContent
class cUOTxItemContent: public cUOPacket
{
public:
	cUOTxItemContent(): cUOPacket( 0x3C, 5 ) { setShort( 1, 5 ); }
	void addItem( P_ITEM pItem );
	void addItem( SERIAL serial, UINT16 id, UINT16 color, UINT16 x, UINT16 y, UINT16 amount, UINT32 container );
};

// 0x74 VendorBuy
class cUOTxVendorBuy: public cUOPacket
{
public:
	cUOTxVendorBuy(): cUOPacket( 0x74, 8 ) { setShort( 1, 8 ); }
	void setSerial( UINT32 data ) { setInt( 3, data ); }
	void addItem( UINT32 price, const QString &description );
};

// 0xB0 Gump Dialog
class cUOTxGumpDialog: public cUOPacket
{
public:
	cUOTxGumpDialog( UINT16 size ): cUOPacket( 0xB0, size ) 
	{
		setShort( 1, size );
	}
	void setSerial( UINT32 data )	{ setInt( 3, data ); }
	void setType( UINT32 data )		{ setInt( 7, data ); }
	void setX( UINT32 data )		{ setInt( 11, data ); }
	void setY( UINT32 data )		{ setInt( 15, data ); }
	void setContent( QString layout, QStringList text );
};

// 0x95 Dye Tub
class cUOTxDyeTub: public cUOPacket
{
public:
	cUOTxDyeTub(): cUOPacket( 0x95, 9 ) {}
	void setSerial( UINT32 data ) { setInt( 1, data ); }
	void setModel( UINT16 data ) { setShort( 7, data ); }
};

// 0x70 OldEffect
class cUOTxOldEffect: public cUOPacket
{
public:
	enum Type
	{
		sourceToDest = 0,
		lightning,
		stayXYZ,
		staySerial,
	};

	cUOTxOldEffect(): cUOPacket( 0x70, 28 ) {}
	void setType( UINT8 data ) { rawPacket[1] = data; }
	void setSource( SERIAL data ) { setInt( 2, data ); }
	void setTarget( SERIAL data ) { setInt( 6, data ); }
	void setId( UINT16 data ) { setShort( 10, data ); }
	void setSource( const Coord_cl &pos ) {
		setShort( 12, pos.x );
		setShort( 14, pos.y );
		rawPacket[16] = pos.z;
	}
	void setTarget( const Coord_cl &pos ) {
		setShort( 17, pos.x );
		setShort( 19, pos.y );
		rawPacket[21] = pos.z;
	}
	void setSpeed( UINT8 data ) { rawPacket[22] = data; }
	void setDuration( UINT8 data ) { rawPacket[23] = data; }
	void setUnknown( UINT16 data ) { setShort( 24, data ); }
	void setFixedDirection( bool data ) { rawPacket[26] = data ? 1 : 0; }
	void setExplodes( bool data ) { rawPacket[27] = data ? 1 : 0; }
};

// 0xC0 Graphical Effect
class cUOTxEffect: public cUOPacket
{
public:
	enum Type
	{
		sourceToDest = 0,
		lightning,
		stayXYZ,
		staySerial,
	};

	cUOTxEffect(): cUOPacket( 0xC0, 36 ) {}
	void setType( UINT8 data ) { rawPacket[1] = data; }
	void setSource( SERIAL data ) { setInt( 2, data ); }
	void setTarget( SERIAL data ) { setInt( 6, data ); }
	void setId( UINT16 data ) { setShort( 10, data ); }
	void setSource( const Coord_cl &pos ) {
		setShort( 12, pos.x );
		setShort( 14, pos.y );
		rawPacket[16] = pos.z;
	}
	void setTarget( const Coord_cl &pos ) {
		setShort( 17, pos.x );
		setShort( 19, pos.y );
		rawPacket[21] = pos.z;
	}
	void setSpeed( UINT8 data ) { rawPacket[22] = data; }
	void setDuration( UINT8 data ) { rawPacket[23] = data; }
	void setUnknown( UINT16 data ) { setShort( 24, data ); }
	void setFixedDirection( bool data ) { rawPacket[26] = data ? 1 : 0; }
	void setExplodes( bool data ) { rawPacket[27] = data ? 1 : 0; }
	void setHue( UINT32 data ) { setInt( 28, data ); }
	void setRendermode( UINT32 data ) { setInt( 32, data ); }
};

// 0xBF Close Gump
class cUOTxCloseGump: public cUOPacket
{
public:
	cUOTxCloseGump(): cUOPacket( 0xBF, 13 ) { setShort( 1, 13 ); setShort( 3, 0x04 ); }
	void setType( SERIAL data ) { setInt( 5, data ); }
	void setButton( UINT32 data ) { setInt( 9, data ); }
};

// 0x6F Trade
class cUOTxTrade: public cUOPacket
{
public:
	cUOTxTrade(): cUOPacket( 0x6f, 17 ) { setShort( 1, 17 ); }
	void setName( const QString &name );
	void setPartner( SERIAL data ) { setInt( 4, data ); }
	void setBox1( SERIAL data ) { setInt( 8, data ); }
	void setBox2( SERIAL data ) { setInt( 12, data ); }
	void setAction( UINT8 data ) { rawPacket[3] = data; }
};

/* 0x1C Ascii Talk
class cUOTxTalk : public cUOPacket
{
public:
	cUOTxTalk() : cUOPacket( 0x1c, 14 ) { setShort( 1, 14 ); }
	void setSerial( SERIAL data )	{ setInt( 3, data ); }
	void setId( UINT16 data )		{ setShort( 7, data ); }
	void setType( UINT8 data )		{ rawPacket[9] = data; }
	void setColor( UINT16 data )	{ setShort( 10, data ); }
	void setFont( UINT16 data )		{ setShort( 12, data ); }
	void setName( const QString &data )	{ setAsciiString( 14, (char*)data.latin1(), 30 ); }
	void setMessage( const QString &data ) 
	{ 
		setAsciiString( 44, (char*)data.latin1(), data.length() );
		setShort( 1, getShort( 1 ) + data.length() + 1 );
	}
};*/

// 0xC1 Cliloc message
class cUOTxClilocMsg: public cUOPacket
{
public:
	cUOTxClilocMsg(): cUOPacket (0xC1,PacketLen ) { setShort( 1, PacketLen ); }

	enum { LowerLeft = 6, OnObject = 7, PacketLen = 48 };

	void setSerial ( SERIAL data ) { setInt( 3, data ); }
	void setBody ( Q_UINT16 data ) { setShort( 7, data ); }
	void setType ( Q_UINT8 data ) { setShort( 9, data ); }
	void setHue ( Q_UINT16 data ) { setShort(10, data ); }
	void setFont ( Q_UINT16 data ) { setShort(12, data ); }
	void setMsgNum ( Q_UINT32 data ) { setInt(14, data ); }
	void setName ( const QString &data ) { memcpy( &rawPacket.data()[18], data.latin1(), MIN( data.length()+1, 30 ) ); }
	void setParams ( const QString &data )
	{
		if ( data.length() > 0 )
		{
			rawPacket.resize(PacketLen+data.length()*2+2);
			memcpy( &rawPacket.data()[PacketLen], data.unicode(), data.length()*2 );
			setShort( 1, PacketLen+data.length()*2 );
			setShort ( PacketLen+data.length()*2, 0 );
		}
	}
}; 

#endif
