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

#ifndef __WPPYTHONSCRIPT_H__
#define __WPPYTHONSCRIPT_H__

// Wolfpack Includes
#include "python/engine.h"
#include "typedefs.h"

// Library Includes
#include <qstring.h>
#include <qregexp.h>
#include <qvaluevector.h>

class cUORxTarget;
class cUOTxTooltipList;
class cUOSocket;
class QDomElement;
class Coord_cl;

class cPythonScript  
{
protected:
	QString name_; // Important!
	bool handleSpeech_;
	bool catchAllSpeech_;

	QValueVector< UINT16 > speechKeywords_;
	QValueVector< QString > speechWords_;
	QValueVector< QRegExp > speechRegexp_;
	PyObject *codeModule; // This object stores the compiled Python Module

public:
	void addKeyword( UINT16 data );	
	void addWord( const QString &data );
	void addRegexp( const QRegExp &data );
	
	bool handleSpeech() const { return handleSpeech_; }
	bool catchAllSpeech() const { return catchAllSpeech_; }
	void setHandleSpeech( bool data ) { handleSpeech_ = data; }
	void setCatchAllSpeech( bool data ) { catchAllSpeech_ = data; }
	bool canHandleSpeech( const QString &text, const QValueVector< UINT16 >& keywords );

	// We need an identification value for the scripts
	void setName( const QString &value ) { name_ = value; }
	const QString &name() { return name_; }

	cPythonScript(): catchAllSpeech_( false ), handleSpeech_( false ), codeModule( 0 ) {}
	virtual ~cPythonScript() {};

	void load( const QDomElement &Data );
	void unload( void );

	// Normal Events
	bool onServerstart();

	bool onUse( P_CHAR User, P_ITEM Used );
	bool onSingleClick( P_ITEM Item, P_CHAR Viewer );
	bool onSingleClick( P_CHAR Character, P_CHAR Viewer );

	bool onCollideItem( P_CHAR Character, P_ITEM Obstacle );
	bool onCollideChar( P_CHAR Character, P_CHAR Obstacle );
	bool onWalk( P_CHAR Character, UINT8 Direction, UINT8 Sequence );
	bool onCreate( cUObject *object, const QString &definition );

	// if this events returns true (handeled) then we should not display the text
	bool onTalk( P_CHAR Character, char speechType, UINT16 speechColor, UINT16 speechFont, const QString &Text, const QString &Lang );
	bool onWarModeToggle( P_CHAR Character, bool War );
	bool onLogin( P_CHAR pChar );
	bool onLogout( P_CHAR pChar );
	bool onHelp( P_CHAR Character );
	bool onChat( P_CHAR Character );
	bool onSkillUse( P_CHAR Character, UINT8 Skill );
	bool onSkillGain( P_CHAR Character, UINT8 Skill, INT32 min, INT32 max, bool success);
	bool onStatGain( P_CHAR Character, UINT8 stat, INT8 amount);
	bool onShowPaperdoll( P_CHAR pChar, P_CHAR pOrigin );
	bool onShowSkillGump( P_CHAR pChar );
	bool onDeath( P_CHAR pChar );

	QString onShowPaperdollName( P_CHAR pChar, P_CHAR pOrigin );

	bool onContextEntry( P_CHAR pChar, cUObject *pObject, UINT16 id );
	bool onShowContextMenu( P_CHAR pChar, cUObject *pObject );

	bool onShowToolTip( P_CHAR pChar, cUObject *pObject, cUOTxTooltipList* tooltip );

	bool onSpeech( cUObject *listener, P_CHAR talker, const QString &text, const QValueVector< UINT16 >& keywords );

	// Drop/Pickup events
	bool onDropOnChar( P_CHAR pChar, P_ITEM pItem );
	bool onDropOnItem( P_ITEM pCont, P_ITEM pItem );
	bool onDropOnGround( P_ITEM pItem, const Coord_cl &pos );
	bool onPickup( P_CHAR pChar, P_ITEM pItem );

	bool onCommand( cUOSocket *socket, const QString &name, const QString &args );

	unsigned int onDamage( P_CHAR pChar, unsigned char type, unsigned int amount, cUObject *source );

	// Magic System (This should be reduced eventually. It's a bit much)
	// But as soon as the flag-system is introduced for python-script 
	// It shouldn't be that much.
	bool onCastSpell( cPlayer *player, unsigned int spell );
};

#endif // __WPPYTHONSCRIPT_H__
