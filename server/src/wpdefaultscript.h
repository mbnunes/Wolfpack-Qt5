//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
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

#if !defined(__WPDEFAULTSCRIPT_H_)
#define __WPDEFAULTSCRIPT_H_

#include "typedefs.h"
#include "platform.h"
#include "coord.h"

// Library Includes
#include <qstring.h>
#include <qdom.h>
#include <qregexp.h>
#include <qvaluevector.h>

#include <vector>

class cUObject;
class cUOSocket;
class cUOTxTooltipList;
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
	bool canHandleSpeech( const QString &text, const QValueVector< UINT16 >& keywords );

	// Scripting Type (i.e. Python or "Default")
	virtual const QString Type( void ) {
		return "default";
	}

	WPDefaultScript(): catchAllSpeech_( false ), handleSpeech_( false ) {}
	virtual ~WPDefaultScript() {}

	// Methods for loading, unloading and reloading the scripts
	// these methods are unused for the default script
	// as it basically does nothing
	virtual void load( const QDomElement &Data ) { Q_UNUSED(Data); }
	virtual void unload( void ) {}

	// We need an identification value for the scripts
	void setName( QString Value ) { Name = Value; }
	QString getName( void ) { return Name; }

	// This event will be caled in both directions(!!)
	// That means first onUse for the character is called
	// and afterwards it's called for the item
	// That way you can add special behaviour
	virtual bool onServerstart() { return false; }
	virtual bool onUse( P_CHAR User, P_ITEM Used ) { Q_UNUSED(User); Q_UNUSED(Used); return false; }

	virtual bool onSingleClick( P_ITEM Item, P_CHAR Viewer ) { Q_UNUSED(Item); Q_UNUSED(Viewer); return false; }
	virtual bool onSingleClick( P_CHAR Character, P_CHAR Viewer ) { Q_UNUSED(Character); Q_UNUSED(Viewer); return false; }

	virtual bool onCollideItem( P_CHAR Character, P_ITEM Obstacle ) { Q_UNUSED(Character); Q_UNUSED(Obstacle); return false; }
	virtual bool onCollideChar( P_CHAR Character, P_CHAR Obstacle ) { Q_UNUSED(Character); Q_UNUSED(Obstacle); return false; }
	virtual bool onWalk( P_CHAR Character, UI08 Direction, UI08 onSequence ) { Q_UNUSED(Character); Q_UNUSED(Direction); Q_UNUSED(onSequence); return false; }
	virtual bool onCreate( cUObject *object, const QString &definition ) { Q_UNUSED(object); Q_UNUSED(definition); return false; }

	// if this events returns true (handeled) then we should not display the text
	virtual bool onLogin( P_CHAR pChar ) { Q_UNUSED(pChar); return false; }
	virtual bool onLogout( P_CHAR pChar ) { Q_UNUSED(pChar); return false; }
	virtual bool onTalk( P_CHAR Character, char speechType, UI16 speechColor, UI16 speechFont, const QString &Text, const QString &Lang ) { Q_UNUSED(Character); Q_UNUSED(speechType); Q_UNUSED(speechColor); Q_UNUSED(speechFont); Q_UNUSED(Text); Q_UNUSED(Lang); return false; }
	virtual bool onWarModeToggle( P_CHAR Character, bool War ) { Q_UNUSED(Character); Q_UNUSED(War); return false; }
	virtual unsigned int onDamage( P_CHAR pChar, unsigned char type, unsigned int amount, cUObject *source ) { Q_UNUSED( pChar ); Q_UNUSED( type ); Q_UNUSED( source ); return amount; }
	virtual bool onHelp( P_CHAR Character ) { Q_UNUSED(Character); return false; }
	virtual bool onChat( P_CHAR Character ) { Q_UNUSED(Character); return false; }
	virtual bool onSkillUse( P_CHAR Character, UI08 Skill ) { Q_UNUSED(Character); Q_UNUSED(Skill); return false; }
	virtual bool onSkillGain( P_CHAR Character, UI08 Skill, SI32 min, SI32 max, bool success ) { Q_UNUSED(Character); Q_UNUSED(Skill); Q_UNUSED(min); Q_UNUSED(max); Q_UNUSED(success); return false; }
	virtual bool onStatGain( P_CHAR Character, UI08 stat, SI08 amount ) { Q_UNUSED(Character); Q_UNUSED(stat); Q_UNUSED(amount); return false; }
	virtual bool onContextEntry( P_CHAR pChar, cUObject *pObject, UINT16 id ) { Q_UNUSED(pChar); Q_UNUSED(pObject); Q_UNUSED(id); return false; }
	virtual bool onShowContextMenu( P_CHAR pChar, cUObject *pObject ) { Q_UNUSED(pChar); Q_UNUSED(pObject); return false; }
	virtual bool onShowToolTip( P_CHAR pChar, cUObject *pObject, cUOTxTooltipList* tooltip ) { Q_UNUSED(pChar); Q_UNUSED(pObject); return false; }
	virtual bool onShowPaperdoll( P_CHAR pChar, P_CHAR pOrigin ) { Q_UNUSED( pChar ); Q_UNUSED( pOrigin ); return false; }
	virtual bool onShowSkillGump( P_CHAR pChar ) { Q_UNUSED( pChar ); return false; }
	virtual bool onDeath( P_CHAR pChar ) { Q_UNUSED( pChar ); return false; }
	virtual QString onShowPaperdollName( P_CHAR pChar, P_CHAR pOrigin ) { Q_UNUSED( pChar ); Q_UNUSED( pOrigin ); return (char*)0; }

	// Drop/Pickup Events
	virtual bool onDropOnChar( P_CHAR pChar, P_ITEM pItem ) { Q_UNUSED(pChar); Q_UNUSED(pItem); return false; }
	virtual bool onDropOnItem( P_ITEM pCont, P_ITEM pItem ) { Q_UNUSED(pCont); Q_UNUSED(pItem); return false; }
	virtual bool onDropOnGround( P_ITEM pItem, const Coord_cl &pos ) { Q_UNUSED(pItem); Q_UNUSED(pos); return false; }
	virtual bool onPickup( P_CHAR pChar, P_ITEM pItem ) { Q_UNUSED(pChar); Q_UNUSED(pItem); return false; }

	// Speech Event
	virtual bool onSpeech( cUObject *listener, P_CHAR talker, const QString &text, const QValueVector< UINT16 >& keywords ) { Q_UNUSED(listener); Q_UNUSED(talker); Q_UNUSED(text); Q_UNUSED(keywords); return false; }
	virtual bool onConsoleOutput( const QString &line ) { Q_UNUSED(line); return false; }
	virtual bool onCommand( cUOSocket *socket, const QString &name, const QString &args ) { Q_UNUSED(socket); Q_UNUSED(name); Q_UNUSED(args); return false; }

	// Magic System
	virtual bool onCastSpell( cPlayer *player, unsigned int spell ) { Q_UNUSED( player ); Q_UNUSED( spell ); return false; }
};

#endif
