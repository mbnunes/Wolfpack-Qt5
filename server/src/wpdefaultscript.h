// WPDefaultScript.h: Schnittstelle für die Klasse WPDefaultScript.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__WPDEFAULTSCRIPT_H_)
#define __WPDEFAULTSCRIPT_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "wolfpack.h"
#include "typedefs.h"
#include "platform.h"

#include <qstring.h>
#include <qdom.h>

class WPDefaultScript  
{
private:
	QString Name; // Important!
public:
	// Scripting Type (i.e. Python or "Default")
	virtual const QString Type( void ) {
		return "default";
	};

	WPDefaultScript();
	virtual ~WPDefaultScript();

	// Methods for loading, unloading and reloading the scripts
	// these methods are unused for the default script
	// as it basically does nothing
	virtual void load( const QDomElement &Data );
	virtual void unload( void );
	virtual void reload( void );

	// We need an identification value for the scripts
	void setName( QString Value );
	QString getName( void );

	// This event will be caled in both directions(!!)
	// That means first onUse for the character is called
	// and afterwards it's called for the item
	// That way you can add special behaviour
	virtual bool onUse( P_CHAR User, P_ITEM Used );

	virtual bool onShowItemName( P_ITEM Item, P_CHAR Viewer );
	virtual bool onShowCharName( P_CHAR Character, P_CHAR Viewer );

	virtual void onCollideItem( P_CHAR Character, P_ITEM Obstacle );
	virtual void onCollideChar( P_CHAR Character, P_CHAR Obstacle );
	virtual bool onWalk( P_CHAR Character, UI08 Direction, UI08 onSequence );

	// if this events returns true (handeled) then we should not display the text
	virtual bool onTalk( P_CHAR Character, QString Text );

	virtual void onTalkToNPC( P_CHAR Talker, P_CHAR Character, const QString &Text );
	virtual void onTalkToItem( P_CHAR Talker, P_ITEM Item, const QString &Text );

	virtual void onWarModeToggle( P_CHAR Character, bool War );

	// Is the Client version already known to us here ???
	virtual bool onConnect( UOXSOCKET Socket, const QString &IP, const QString &Username, const QString &Password );

	virtual bool onDisconnect( UOXSOCKET Socket, QString IP );

	virtual void onEnterWorld( P_CHAR Character );

	virtual bool onHelp( P_CHAR Character );

	virtual bool onChat( P_CHAR Character );

	virtual bool onSkillUse( P_CHAR Character, UI08 Skill );
};

#endif // !defined(AFX_WPDEFAULTSCRIPT_H__FDB3420A_822D_4D37_8D60_1ED584CC02DF__INCLUDED_)
