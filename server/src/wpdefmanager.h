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
#include <qvaluevector.h>

// Foward declarations
class QStringList;
class QXmlAttributes;

// Typedefs
typedef QMap< QString, QDomElement > DefSections;

enum eDefCategory 
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
 	WPDT_CONTEXTMENU,
	WPDT_AI,
	WPDT_CHARBASE,
	WPDT_COUNT
};

class cDefManagerPrivate;

class cElement
{
private:
	struct stAttribute
	{
		QCString name;
		QString value;
	};

	QCString name_; // Tag Name spell for <spell i.e.
	QString text_; // This is not really a well implemented approach. this is subject to change

	cElement *parent_;

	unsigned int childCount_;
	cElement **children; // Array of Childs
	
	unsigned int attrCount_;
	stAttribute **attributes; // Attributes

	void freeAttributes();
	void freeChildren();

public:
	cElement();
	virtual ~cElement();

	void copyAttributes( const QXmlAttributes &attributes );
	void addChild( cElement *element );

	void removeChild( cElement *element );
	const cElement *findChild( const QString &name ) const;
	const cElement *getChild( unsigned int index ) const;
	unsigned int childCount() const;
	bool hasAttribute( const QCString &name ) const;
	const QString &getAttribute( const QCString &name, const QString &def = QString::null ) const;
	
	void setName( const QCString &data );
	const QCString &name() const;

	void setText( const QString &data );
	const QString &text() const;

	void setParent( cElement *parent );
	const cElement *parent() const;

	QString getValue() const;

	const cElement *getTopmostParent() const;
};

class WPDefManager  
{
private:
	cDefManagerPrivate *impl;

public:
	WPDefManager();
	virtual ~WPDefManager();

	void reload( void );
	void load( void );
	void unload( void );

	bool ImportSections( const QString& FileName );
	const cElement*		getDefinition( eDefCategory Type, const QString& id ) const;
	const QValueVector< cElement* > &getDefinitions( eDefCategory Type ) const;

	//const QDomElement*	getSection( eDefCategory Type, const QString& Section ) const;
	QStringList			getSections( eDefCategory Type ) const;
	QString				getRandomListEntry( const QString& ListSection );
	QStringList			getList( const QString& ListSection );
	QString				getText( const QString& TextSection ) const;

protected:
	QMap< QString, QStringList >	listcache_;
};

#endif // __WPDEFMANAGER_H__
