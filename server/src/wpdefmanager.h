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

#if !defined(__WPDEFMANAGER_H__)
#define __WPDEFMANAGER_H__

// Library Includes
#include <qdom.h>
#include <qmap.h>
#include <qstring.h>

// Foward declarations
class QStringList;

// Typedefs
typedef QMap< QString, QDomElement > DefSections;

enum WPDEF_TYPE 
{
	WPDT_ITEM = 0,
	WPDT_SCRIPT,
	WPDT_NPC,
	WPDT_LIST,
	WPDT_MENU,
	WPDT_SPELL,
	WPDT_PRIVLEVEL,
	WPDT_SPAWNREGION,
	WPDT_REGION,
	WPDT_MULTI,
	WPDT_TEXT,
	WPDT_STARTITEMS,
	WPDT_LOCATION,
	WPDT_SKILL,
	WPDT_ACTION,
	WPDT_MAKESECTION,
	WPDT_MAKEITEM,
	WPDT_USEITEM,
	WPDT_SKILLCHECK,
	WPDT_DEFINE,
	WPDT_RESOURCE,
 	WPDT_CONTEXTMENU
};

class WPDefManager  
{
private:
	// Maps
	DefSections Items;
	DefSections Scripts;
	DefSections NPCs;
	DefSections StringLists;
	DefSections Menus;
	DefSections Spells;
	DefSections PrivLevels;
	DefSections SpawnRegions;
	DefSections Regions;
	DefSections Multis;
	DefSections Texts;
	DefSections StartItems;
	DefSections Locations;
	DefSections Skills;
	DefSections Actions;
	DefSections MakeSections;
	DefSections MakeItems;
	DefSections UseItems;
	DefSections SkillChecks;
	DefSections Defines;
	DefSections Resources;
 	DefSections ContextMenus;

	bool ImportSections( const QString& FileName );

public:
	WPDefManager() {};
	virtual ~WPDefManager() {};

	void reload( void );
	void load( void );
	void unload( void );
	void unload( WPDEF_TYPE );

	void ProcessNode( const QDomElement& Node );

	const QDomElement*	getSection( WPDEF_TYPE Type, const QString& Section ) const;
	QStringList			getSections( WPDEF_TYPE Type ) const;
	QString				getRandomListEntry( const QString& ListSection ) const;
	QStringList			getList( const QString& ListSection ) const;
	QString				getText( const QString& TextSection ) const;
};

#endif // __WPDEFMANAGER_H__
