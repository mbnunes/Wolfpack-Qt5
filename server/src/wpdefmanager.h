// WPDefManager.h: Schnittstelle für die Klasse WPDefManager.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WPDEFMANAGER_H__59377C90_C75A_4AC7_8B5B_517354DC8E76__INCLUDED_)
#define AFX_WPDEFMANAGER_H__59377C90_C75A_4AC7_8B5B_517354DC8E76__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include <qdom.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>

typedef QMap< QString, QDomElement > DefSections;

enum WPDEF_TYPE 
{
	WPDT_ITEM = 0,
	WPDT_SCRIPT,
	WPDT_NPC,
	WPDT_ITEMLIST,
	WPDT_MENU,
	WPDT_RESOURCE
};

class WPDefManager  
{
private:
	// Maps
	DefSections Items;
	DefSections Scripts;
	DefSections NPCs;
	DefSections ItemLists;
	DefSections Menus;
	DefSections Resources;

	bool ImportSections( QString FileName );
	void ProcessNode( QDomElement Node );

public:
	WPDefManager() {};
	virtual ~WPDefManager() {};

	void Load( void );
	void Unload( void );

	QDomElement *getSection( WPDEF_TYPE Type, QString Section );
	QStringList getSections( WPDEF_TYPE Type );
};

#endif // !defined(AFX_WPDEFMANAGER_H__59377C90_C75A_4AC7_8B5B_517354DC8E76__INCLUDED_)
