#include "wpdefaultscript.h"

WPDefaultScript::WPDefaultScript()
{
}

WPDefaultScript::~WPDefaultScript()
{

}

void WPDefaultScript::load( const QDomElement &Data )
{
	return;
}

void WPDefaultScript::unload( void )
{
	return;
}

void WPDefaultScript::reload( void )
{
	return;
}

void WPDefaultScript::setName( QString Value )
{
	Name = Value;
}

QString WPDefaultScript::getName( void )
{
	return Name;
}

bool WPDefaultScript::onUse( P_CHAR User, P_ITEM Used )
{
	return false;
}

// Two Versions
bool WPDefaultScript::onShowItemName( P_ITEM Item, P_CHAR Viewer )
{
	return false;
}

bool WPDefaultScript::onShowCharName( P_CHAR Character, P_CHAR Viewer )
{
	return false;
}

void WPDefaultScript::onCollideItem( P_CHAR Character, P_ITEM Obstacle )
{
}

void WPDefaultScript::onCollideChar( P_CHAR Character, P_CHAR Obstacle )
{
}

bool WPDefaultScript::onWalk( P_CHAR Character, UI08 Direction, UI08 onSequence )
{
	return true;
}


// if this events returns true (handeled) then we should not display the text
bool WPDefaultScript::onTalk( P_CHAR Character, QString Text )
{
	return true;
}


void WPDefaultScript::onTalkToNPC( P_CHAR Talker, P_CHAR Character, const QString &Text )
{
}

void WPDefaultScript::onTalkToItem( P_CHAR Talker, P_ITEM Item, const QString &Text )
{
}


void WPDefaultScript::onWarModeToggle( P_CHAR Character, bool War )
{
}


// Is the Client version already known to us here ???
bool WPDefaultScript::onConnect( UOXSOCKET Socket, const QString &IP, const QString &Username, const QString &Password )
{
	return false;
}


bool WPDefaultScript::onDisconnect( UOXSOCKET Socket, QString IP )
{
	return false;
}


void WPDefaultScript::onEnterWorld( P_CHAR Character )
{
}


bool WPDefaultScript::onHelp( P_CHAR Character )
{
	return false;
}


bool WPDefaultScript::onChat( P_CHAR Character )
{
	return false;
}


bool WPDefaultScript::onSkillUse( P_CHAR Character, UI08 Skill )
{
	return false;
}
