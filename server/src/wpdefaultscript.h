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

#if !defined(__WPDEFAULTSCRIPT_H_)
#define __WPDEFAULTSCRIPT_H_

#include "typedefs.h"
#include "platform.h"

// Library Includes
#include "qstring.h"
#include "qdom.h"
#include "qregexp.h"

#include <vector>

class cUObject;
class Coord_cl;
class cUORxTarget;

class WPDefaultScript  
{
protected:
	QString Name; // Important!
	bool handleSpeech_;
	bool catchAllSpeech_;

	std::vector< UINT16 > speechKeywords_;
	std::vector< QString > speechWords_;
	std::vector< QRegExp > speechRegexp_;
public:
	void addKeyword( UINT16 data ) { speechKeywords_.push_back( data ); }
	void addWord( const QString &data ) { speechWords_.push_back( data ); }
	void addRegexp( const QRegExp &data ) { speechRegexp_.push_back( data ); }
	
	bool handleSpeech() const { return handleSpeech_; }
	bool catchAllSpeech() const { return catchAllSpeech_; }
	void setHandleSpeech( bool data ) { handleSpeech_ = data; }
	void setCatchAllSpeech( bool data ) { catchAllSpeech_ = data; }
	bool canHandleSpeech( const QString &text, std::vector< UINT16 > keywords ) {return false;};

	// Scripting Type (i.e. Python or "Default")
	virtual const QString Type( void ) {
		return "default";
	}

	WPDefaultScript(): catchAllSpeech_( false ), handleSpeech_( false ) {}
	virtual ~WPDefaultScript() {}

	// Methods for loading, unloading and reloading the scripts
	// these methods are unused for the default script
	// as it basically does nothing
	virtual void load( const QDomElement &Data ) {}
	virtual void unload( void ) {}

	// We need an identification value for the scripts
	void setName( QString Value ) { Name = Value; }
	QString getName( void ) { return Name; }

	// This event will be caled in both directions(!!)
	// That means first onUse for the character is called
	// and afterwards it's called for the item
	// That way you can add special behaviour
	virtual bool onUse( P_CHAR User, P_ITEM Used ) { return false; }

	virtual bool onSingleClick( P_ITEM Item, P_CHAR Viewer ) { return false; }
	virtual bool onSingleClick( P_CHAR Character, P_CHAR Viewer ) { return false; }

	virtual bool onCollideItem( P_CHAR Character, P_ITEM Obstacle ) { return false; }
	virtual bool onCollideChar( P_CHAR Character, P_CHAR Obstacle ) { return false; }
	virtual bool onWalk( P_CHAR Character, UI08 Direction, UI08 onSequence ) { return false; }
	virtual bool onCreate( cUObject *object, const QString &definition ) { return false; }

	// if this events returns true (handeled) then we should not display the text
	virtual bool onLogin( P_CHAR pChar ) { return false; }
	virtual bool onLogout( P_CHAR pChar ) { return false; }
	virtual bool onTalk( P_CHAR Character, char speechType, UI16 speechColor, UI16 speechFont, const QString &Text, const QString &Lang ) { return false; }
	virtual bool onWarModeToggle( P_CHAR Character, bool War ) { return false; }
	virtual bool onHelp( P_CHAR Character ) { return false; }
	virtual bool onChat( P_CHAR Character ) { return false; }
	virtual bool onSkillUse( P_CHAR Character, UI08 Skill ) { return false; }
	virtual bool onContextEntry( P_CHAR pChar, cUObject *pObject, UINT16 id ) { return false; }
	virtual bool onShowContextMenu( P_CHAR pChar, cUObject *pObject ) { return false; }

	// Speech Event
	virtual bool onSpeech( cUObject *listener, P_CHAR talker, const QString &text, std::vector< UINT16 > keywords ) { return false; }

	// Magic System
	virtual bool onBeginCast( P_CHAR pMage, UINT8 spell, UINT8 type ) { return false; }
	virtual bool onEndCast( P_CHAR pMage, UINT8 spell, UINT8 type ) { return false; }
	virtual bool onSpellCheckTarget( P_CHAR pMage, UINT8 spell, UINT8 type, cUORxTarget *target ) { return false; }
	virtual bool onSpellSuccess( P_CHAR pMage, UINT8 spell, UINT8 type, cUORxTarget* ) { return false; }
	virtual bool onSpellFailure( P_CHAR pMage, UINT8 spell, UINT8 type, cUORxTarget* ) { return false; }
};

#endif
