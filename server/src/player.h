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

#if !defined (CPLAYER_H_HEADER_INCLUDED)
#define CPLAYER_H_HEADER_INCLUDED

// platform includes
#include "platform.h"

// library includes

// wolfpack includes
#include "basechar.h"
#include "makemenus.h"
#include "accounts.h"

// Class for player characters. Implements cBaseChar.
class cPlayer : public cBaseChar
{
	Q_OBJECT
public:
	// con-/destructors
    cPlayer();
    cPlayer(const cPlayer& right);
    virtual ~cPlayer();

	// operators
    cPlayer& operator=(const cPlayer& right);

	// type definitions
    enum enInputMode { enNone, enRenameRune, enPricing, enDescription, enNameDeed, enHouseSign, enPageGM, enPageCouns};

	// implementation of interfaces
	static void registerInFactory();
	void load( char **, UINT16& );
	void save();
	void save( FlatStore::OutputFile*, bool first = false ) throw();
	bool load( unsigned char chunkGroup, unsigned char chunkType, FlatStore::InputFile* ) throw();
	bool postload() throw();
	bool del();

	virtual enCharTypes objectType();
	virtual void update( bool excludeself = false ); 
	virtual void resend( bool clean = true, bool excludeself = false ); 
	virtual void talk( const QString &message, UI16 color = 0xFFFF, UINT8 type = 0, bool autospam = false, cUOSocket* socket = NULL );
	virtual UINT8 notority( P_CHAR pChar );
	virtual void kill();
	virtual void showName( cUOSocket *socket );
	virtual void fight(P_CHAR pOpponent);
	virtual void soundEffect( UI16 soundId, bool hearAll = true );
	virtual void giveGold( Q_UINT32 amount, bool inBank = false );
	virtual UINT32 takeGold( UINT32 amount, bool useBank = false );

	// other public methods
	stError *setProperty( const QString &name, const cVariant &value );
	stError *getProperty( const QString &name, cVariant &value ) const;
	void turnTo( cUObject *object ); // override
	void turnTo( const Coord_cl& data ); // override
	P_NPC unmount();
	void mount( P_NPC pMount );
	bool isGM() const;
	bool isCounselor() const;
	bool isGMorCounselor() const; 
	void makeCriminal();
	void disturbMed();
	P_ITEM getBankBox( void );
	int  CountBankGold();
	bool canPickUp(cItem* pi);
	void message( const QString &message, UI16 color = 0x3B2 );
	bool online() const;
	void giveNewbieItems( Q_UINT8 skill = 0xFF );
	bool checkSkill( UI16 skill, SI32 min, SI32 max, bool advance = true ); // override

	// Wrapper events
	virtual bool onLogin( void ); // The character enters the world
	virtual bool onLogout( void ); // The character enters the world
	virtual bool onHelp( void ); // The character wants help
	virtual bool onChat( void ); // The character wants to chat
	virtual bool onShowContext( cUObject *object ); // Shows a context menu for a specific item
	bool onPickup( P_ITEM pItem );

	// getters
    AccountRecord*			account() const;
    UINT32					additionalFlags() const;
    UINT32					clientIdleTime() const;
    UINT32					logoutTime() const;
    UINT32					muteTime() const;
    UINT32					objectDelay() const;
	UINT32					trackingTime() const;
	cUOSocket*				socket() const;
	enInputMode				inputMode() const { return inputMode_; }
	SERIAL					inputItem() const;
	UINT8					visualRange() const;
	QString					profile() const;
    UINT8					fixedLightLevel() const;
	// bit flag getters
	bool					isMuted() const;
	bool					maySnoop() const;
	bool					mayBroadcast() const;
	bool					showSerials() const;
	// advanced getters for data structures
	// makemenus
	QPtrList< cMakeSection >	lastSelections( cMakeMenu* basemenu );
	cMakeSection*				lastSection( cMakeMenu* basemenu );
	// pets
	CharContainer				pets() const;

	// setters
    void setAccount(AccountRecord* data, bool moveFromAccToAcc = true);
    void setAdditionalFlags(UINT32 data);
    void setClientIdleTime(UINT32 data);
    void setLogoutTime(UINT32 data);
    void setMuteTime(UINT32 data);
    void setObjectDelay(UINT32 data);
	void setTrackingTime(UINT32 data);
	void setSocket(cUOSocket* data);
	void setInputMode(enInputMode data) { inputMode_ = data; }
	void setInputItem(SERIAL data);
	void setVisualRange(UINT8 data);
	void setProfile(const QString &data);
    void setFixedLightLevel(UINT8 data);
	// bit flag setters
	void setMuted(bool data);
	void setMaySnoop(bool data);
	void setMayBroadcast(bool data);
	void setShowSerials(bool data);

	virtual void setStamina( INT16 data, bool notify = true );

	// advanced setters for data structures
	// makemenus
	void setLastSection( cMakeMenu* basemenu, cMakeSection* data );
	void clearLastSelections( void );
	// pets
	void addPet( P_NPC pPet, bool noOwnerChange = false );	
	void removePet( P_NPC pPet, bool noOwnerChange = false );

protected:
	// interface implementation
	static void buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions );
	virtual void processNode( const QDomElement& Tag );

	// other protected methods
	void applyStartItemDefinition( const QDomElement &Tag );

    // The account object including this char.
    // cOldChar::account_
    AccountRecord* account_;

    // time till char will be logged out
    // cOldChar::logout_
    UINT32 logoutTime_;

    // time till the char will be handled as idle.
    // compare with logoutTime! check it!
    UINT32 clientIdleTime_;

    // Time till player will become unmuted again
    // cOldChar::mutetime_
    UINT32 muteTime_;

    // Time till the player can use another object.
    UINT32 objectDelay_;

    // Additional property flags.
    // 
    // Bits:
    // 01 - muted, cOldChar::squelched_
	// 02 - may snoop, cOldChar::priv Bit 7
	// 03 - may broadcast, cOldChar::priv Bit 2
	// 04 - show serials, cOldChar::priv Bit 4
    UINT32 additionalFlags_;

	// The pets that follow the char.
	CharContainer pets_;

	// Time till the quest arrow for tracking disappears.
	UINT32 trackingTime_;

	// Network socket of the player.
	cUOSocket* socket_;

	// Input mode of the entered text.
	// 0 - none
	// 4 - rune renaming
	enInputMode inputMode_;

	// Serial of the item which is affected by the current text entry.
	SERIAL inputItem_;

	// Visual range of the player
	UINT8 visualRange_;

	// Paperdoll profile of the char
	QString profile_;

	// Last ten selections the char has made in specific MakeMenus.
	QMap< cMakeMenu*, QPtrList< cMakeSection > > lastSelections_;

    // Fixed light level. is used in dungeons or for nightsight spell.
    // cOldChar:fixedlight_
    UINT8 fixedLightLevel_;

};

inline AccountRecord* cPlayer::account() const
{
    return account_;
}

inline void cPlayer::setAccount(AccountRecord* data, bool moveFromAccToAcc)
{
	if( moveFromAccToAcc && account_ != 0 )
		account_->removeCharacter( this );

	account_ = data;

	if( account_ != 0 )
		account_->addCharacter( this );
	changed( SAVE|TOOLTIP );
}

inline UINT32 cPlayer::additionalFlags() const
{
    return additionalFlags_;
}

inline void cPlayer::setAdditionalFlags(UINT32 data)
{
    additionalFlags_ = data;
	changed( SAVE );
}

inline UINT32 cPlayer::clientIdleTime() const
{
    return clientIdleTime_;
}

inline void cPlayer::setClientIdleTime(UINT32 data)
{
    clientIdleTime_ = data;
	changed( SAVE );
}

inline UINT32 cPlayer::logoutTime() const
{
    return logoutTime_;
}

inline void cPlayer::setLogoutTime(UINT32 data)
{
    logoutTime_ = data;
	changed( SAVE );
}

inline UINT32 cPlayer::muteTime() const
{
    return muteTime_;
}

inline void cPlayer::setMuteTime(UINT32 data)
{
    muteTime_ = data;
	changed( SAVE );
}

inline UINT32 cPlayer::objectDelay() const
{
    return objectDelay_;
}

inline void cPlayer::setObjectDelay(UINT32 data)
{
    objectDelay_ = data;
}

inline UINT32 cPlayer::trackingTime() const
{
	return trackingTime_;
}

inline void cPlayer::setTrackingTime(UINT32 data)
{
    trackingTime_ = data;
}

inline cUOSocket* cPlayer::socket() const
{
	return socket_;
}

inline void cPlayer::setSocket(cUOSocket* data)
{
    socket_ = data;
	changed( SAVE );
}

inline SERIAL cPlayer::inputItem() const
{
	return inputItem_;
}

inline void cPlayer::setInputItem(SERIAL data)
{
    inputItem_ = data;
}

inline UINT8 cPlayer::visualRange() const
{
	return visualRange_;
}

inline void cPlayer::setVisualRange(UINT8 data)
{
    visualRange_ = data;
	changed( SAVE );
}

inline QString cPlayer::profile() const
{
	return profile_;
}

inline void cPlayer::setProfile(const QString &data)
{
    profile_ = data;
	changed( SAVE );
}

inline UINT8 cPlayer::fixedLightLevel() const
{
    return fixedLightLevel_;
}

inline void cPlayer::setFixedLightLevel(UINT8 data)
{
    fixedLightLevel_ = data;
	changed( SAVE );
}

inline bool cPlayer::isMuted() const
{
	return additionalFlags_ & 0x0001;
}

inline bool cPlayer::maySnoop() const
{
	return additionalFlags_ & 0x0002;
}

inline bool cPlayer::mayBroadcast() const
{
	return additionalFlags_ & 0x0004;
}

inline bool cPlayer::showSerials() const
{
	return additionalFlags_ & 0x0008;
}

inline void cPlayer::setMuted(bool data)
{
	if( data ) additionalFlags_ |= 0x0001; else additionalFlags_ &= ~0x0001; 
	changed( SAVE );
}

inline void cPlayer::setMaySnoop(bool data)
{
	if( data ) additionalFlags_ |= 0x0002; else additionalFlags_ &= ~0x0002; 
	changed( SAVE );
}

inline void cPlayer::setMayBroadcast(bool data)
{
	if( data ) additionalFlags_ |= 0x0004; else additionalFlags_ &= ~0x0004; 
	changed( SAVE );
}

inline void cPlayer::setShowSerials(bool data)
{
	if( data ) additionalFlags_ |= 0x0008; else additionalFlags_ &= ~0x0008; 
	changed( SAVE );
}

inline enCharTypes cPlayer::objectType()
{
	return enPlayer;
}

inline cBaseChar::CharContainer cPlayer::pets() const
{
	return pets_;
}


#endif /* CPLAYER_H_HEADER_INCLUDED */
