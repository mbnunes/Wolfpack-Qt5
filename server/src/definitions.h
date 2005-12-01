/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2005 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In addition to that license, if you are running this program or modified
 * versions of it on a public system you HAVE TO make the complete source of
 * the version used by you available or provide people with a location to
 * download it.
 *
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#if !defined(__WPDEFMANAGER_H__)
#define __WPDEFMANAGER_H__

#include "singleton.h"
#include "server.h"

// Library Includes
#include <qmap.h>
#include <QString>
#include <q3valuevector.h>
#include <q3valuelist.h>
//Added by qt3to4:
#include <Q3CString>

// Includes for cPythonScriptable
#include "pythonscript.h"

// Foward declarations
class QStringList;
class QXmlAttributes;
class cDefManagerPrivate;

enum eDefCategory
{
	WPDT_ITEM			= 0,
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
	WPDT_TELEPORTER,
	WPDT_QUEST,
	WPDT_COUNT
};

class cElement : public cPythonScriptable
{
private:
	struct stAttribute
	{
		Q3CString name;
		QString value;
	};

	QString name_; // Tag Name spell for <spell i.e.
	QString text_; // This is not really a well implemented approach. this is subject to change

	cElement* parent_;

	unsigned int childCount_;
	cElement** children; // Array of Childs

	unsigned int attrCount_;
	stAttribute** attributes; // Attributes

	void freeAttributes();
	void freeChildren();

public:
	cElement();
	virtual ~cElement();

	void copyAttributes( const QXmlAttributes& attributes );
	void addChild( cElement* element );

	void removeChild( cElement* element );
	const cElement* findChild( const QString& name ) const;
	const cElement* getChild( unsigned int index ) const;
	unsigned int childCount() const;
	bool hasAttribute( const Q3CString& name ) const;
	const QString& getAttribute( const Q3CString& name, const QString& def = QString::null ) const;

	void setName( const Q3CString& data );
	const QString& name() const;

	void setText( const QString& data );
	const QString& text() const;

	void setParent( cElement* parent );
	const cElement* parent() const;

	QString value() const;

	const cElement* getTopmostParent() const;

	PyObject* getPyObject();
	const char* className() const;
	bool implements( const QString& name ) const;
};

class cDefinitions : public cComponent
{
protected:
	cDefManagerPrivate* impl;
	Q3ValueList<cElement*> elements; // Instances of cElement without a parent.
	QMap<QString, QStringList> listcache_;

public:
	// Add an element to our responsibility
	inline void addElement(cElement *element) {
		elements.append(element);
	}

	typedef QMap<QString, cElement*>::iterator Iterator;

	cDefinitions();
	virtual ~cDefinitions();

	Iterator begin(eDefCategory type);
	Iterator end(eDefCategory type);

	void reload( void );
	void load( void );
	void unload( void );

	const cElement* getDefinition( eDefCategory Type, const QString& id ) const;
	const Q3ValueVector<cElement*>& getDefinitions( eDefCategory Type ) const;

	QStringList getSections( eDefCategory Type ) const;
	QString getRandomListEntry( const QString& ListSection );
	QStringList getList( const QString& ListSection );
	QString getText( const QString& TextSection ) const;
};

typedef Singleton<cDefinitions> Definitions;

#endif // __WPDEFMANAGER_H__
