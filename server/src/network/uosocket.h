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


#ifndef __UOSOCKET__
#define __UOSOCKET__

// Library Includes
#include "qcstring.h"
#include "qsocketdevice.h"
#include <stack>

// Forward Declarations
class cUOPacket;
class AccountRecord;
class cTargetRequest;
class cGump;

// Too many Forward Declarations
#include "uorxpackets.h"
#include "uotxpackets.h"
#include "../typedefs.h"
#include "../wptargetrequests.h"

class cUOSocket
{
public:
	enum eSocketState	{ Connecting = 0, LoggingIn, LoggedIn, InGame	};
private:
	QSocketDevice *_socket;
	UINT32 _rxBytes, _txBytes, _uniqueId, _tempInt;
	AccountRecord* _account;
	P_CHAR _player;
	eSocketState _state;
	UINT8 lastPacket, _viewRange, _walkSequence;
	cTargetRequest *targetRequest;
	QString _lang,_version;

	std::map< SERIAL, cGump* > gumps;
	std::stack< SERIAL > free_serials;

	bool authenticate( const QString &username, const QString &password );
	void giveNewbieItems( cUORxCreateChar *packet, Q_UINT8 skill = 0xFF );

public:
	// Temporary stuff, should be replaced by a tag-like system later
	void setTempInt( UINT32 data );
	UINT32 tempInt() const;

	Q_UINT8 walkSequence( void ) const;
	void setWalkSequence( Q_UINT8 data );

	cUOSocket( QSocketDevice *sDevice );
	virtual ~cUOSocket( void );

	QSocketDevice *socket( void ) const;
	void setSocket( QSocketDevice *data );

	eSocketState state( void ) const;
	void setState( eSocketState data );

	QString version( void ) const;
	QString lang( void ) const;

	P_CHAR player( void ) const;
	P_ITEM dragging() const;


	Q_UINT32 rxBytes( void ) const;
	Q_UINT32 txBytes( void ) const;
	Q_UINT8 viewRange( void ) const;
	void setRxBytes( Q_UINT32 data );
	void setTxBytes( Q_UINT32 data );

	Q_UINT32 uniqueId( void ) const;

	void recieve(); // Tries to recieve one packet and process it
	void send( cUOPacket *packet );
	void send( cGump	 *gump );

	// Handler
	void handleLoginRequest( cUORxLoginRequest *packet );
	void handleHardwareInfo( cUORxHardwareInfo *packet );
	void handleSelectShard( cUORxSelectShard *packet );
	void handleServerAttach( cUORxServerAttach *packet );
	void handleDeleteCharacter( cUORxDeleteCharacter *packet );
	void handlePlayCharacter( cUORxPlayCharacter *packet );
	void handleCreateChar( cUORxCreateChar *packet );
	void handleUpdateRange( cUORxUpdateRange *packet );
	void handleQuery( cUORxQuery *packet );
	void handleRequestLook( cUORxRequestLook *packet );
	void handleMultiPurpose( cUORxMultiPurpose *packet );
	void handleContextMenuRequest( cUORxContextMenuRequest *packet );
	void handleWalkRequest( cUORxWalkRequest* packet );
	void handleSetLanguage( cUORxSetLanguage* packet );
	void handleSpeechRequest( cUORxSpeechRequest* packet );
	void handleDoubleClick( cUORxDoubleClick* packet );
	void handleGetTip( cUORxGetTip* packet );
	void handleChangeWarmode( cUORxChangeWarmode* packet );
	void handleTarget( cUORxTarget *packet );
	void handleRequestAttack( cUORxRequestAttack* packet );
	void handleBookPage( cUORxBookPage* packet );
	void handleUpdateBook( cUORxUpdateBook* packet );
	void handleAction( cUORxAction* packet );
	void handleGumpResponse( cUORxGumpResponse* packet );

	// Utilities
	void updateChar( P_CHAR pChar );
	void sendChar( P_CHAR pChar );
	void showSpeech( cUObject *object, const QString &message, Q_UINT16 color = 0x3B2, Q_UINT16 font = 3, cUOTxUnicodeSpeech::eSpeechType speechType = cUOTxUnicodeSpeech::Regular );
	void sysMessage( const QString &message, Q_UINT16 color = 0x0037, UINT16 font = 3 );
	void sendCharList();
	void removeObject( cUObject *object );
	void setPlayer( P_CHAR pChar = NULL ); // Updates the current player
	void updateCharList();
	void disconnect( void ); // Call this whenever the socket should disconnect
	void playChar( P_CHAR player ); // Play a character
	bool isT2A()	{ return true; } // ???
	void sendPaperdoll( P_CHAR pChar );
	void playMusic( void );
	void sendContainer( P_ITEM pCont );
	void bounceItem( P_ITEM pItem, eBounceReason reason );
	void updatePlayer();
	void resendPlayer();
	void poll();
	void soundEffect( UINT16 soundId, cUObject *source = NULL ); 
	void attachTarget( cTargetRequest *request );
	void attachTarget( cTargetRequest *request, UINT16 multiid );
	void resendWorld( bool clean = true );
	void resync();
	void updateStamina( P_CHAR pChar = NULL );
	void updateMana( P_CHAR pChar = NULL );
	void updateHealth( P_CHAR pChar = NULL );
	void sendStatWindow( P_CHAR pChar = NULL );
	bool inRange( cUOSocket* ) const;
	void sendSkill( UINT16 skill );
	void sendBuyWindow( P_CHAR pVendor );
	void sendVendorCont( P_ITEM pItem );

	void allowMove( Q_UINT8 sequence );
	void denyMove( Q_UINT8 sequence );
};

// Inline members
inline void cUOSocket::setTempInt( UINT32 data ) 
{
	_tempInt = data; 
}

inline UINT32 cUOSocket::tempInt() const
{ 
	return _tempInt; 
}

inline Q_UINT8 cUOSocket::walkSequence( void ) const
{ 
	return _walkSequence; 
}

inline void cUOSocket::setWalkSequence( Q_UINT8 data )	
{ 
	_walkSequence = data; 
}

inline QSocketDevice* cUOSocket::socket( void ) const
{ 
	return _socket; 
}

void inline cUOSocket::setSocket( QSocketDevice *data ) 
{ 
	_socket = data; 
}

inline cUOSocket::eSocketState cUOSocket::state( void ) const
{ 
	return _state; 
}

inline void cUOSocket::setState( cUOSocket::eSocketState data ) 
{ 
	_state = data; 
}

inline QString cUOSocket::version( void ) const
{ 
	return _version; 
}

inline QString cUOSocket::lang( void ) const
{ 
	return _lang; 
}

inline P_CHAR cUOSocket::player( void ) const
{ 
	return _player; 
}

inline Q_UINT32 cUOSocket::rxBytes( void ) const
{ 
	return _rxBytes; 
}

inline Q_UINT32 cUOSocket::txBytes( void ) const
{ 
	return _txBytes; 
}

inline Q_UINT8 cUOSocket::viewRange( void ) const
{ 
	return _viewRange; 
}

inline void cUOSocket::setRxBytes( Q_UINT32 data ) 
{ 
	_rxBytes = data;
}

inline void cUOSocket::setTxBytes( Q_UINT32 data )
{ 
	_txBytes = data; 
}

inline Q_UINT32 cUOSocket::uniqueId( void ) const
{ 
	return _uniqueId; 
}


#endif
