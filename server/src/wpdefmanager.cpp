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

#include "wpdefmanager.h"
#include "globals.h"
#include "prototypes.h"
#include "basics.h"
#include "inlines.h"
#include "wpconsole.h"
#include "commands.h"
#include "encryption.h"

// Library Includes
#include <qdom.h>
#include <qxml.h>
#include <qfile.h>
#include <qptrstack.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qvaluevector.h>

struct stCategory
{
	const char *name;
	eDefCategory key;
};

stCategory categories[] = 
{
	{ "item",			WPDT_ITEM },
	{ "script",			WPDT_SCRIPT },
	{ "npc",			WPDT_NPC },
	{ "list",			WPDT_LIST },
	{ "menu",			WPDT_MENU },
	{ "spell",			WPDT_SPELL },
	{ "acl",			WPDT_PRIVLEVEL },
	{ "spawnregion",	WPDT_SPAWNREGION },
	{ "region",			WPDT_REGION },
	{ "multi",			WPDT_MULTI },
	{ "text",			WPDT_TEXT },
	{ "startitems",		WPDT_STARTITEMS },
	{ "location",		WPDT_LOCATION },
	{ "skill",			WPDT_SKILL },
	{ "action",			WPDT_ACTION },
	{ "make",			WPDT_MAKESECTION },
	{ "makeitem",		WPDT_MAKEITEM },
	{ "useitem",		WPDT_USEITEM },
	{ "skillcheck",		WPDT_SKILLCHECK },
	{ "define",			WPDT_DEFINE },
	{ "resource",		WPDT_RESOURCE },
	{ "contextmenu",	WPDT_CONTEXTMENU },
	{ "ai",				WPDT_AI },
	{ 0,				WPDT_COUNT },
};

class cDefManagerPrivate
{
public:
	QMap< QString, cElement* > unique[WPDT_COUNT];
	QMap< unsigned int, QValueVector< cElement* > > nonunique;
	QStringList imports;
};

class cXmlHandler : public QXmlDefaultHandler
{
private:
	cDefManagerPrivate *impl;
	QPtrStack< cElement > stack;
	QString filename;
	QXmlLocator *locator;

public:
	cXmlHandler( cDefManagerPrivate *impl, const QString &filename )
	{
		this->impl = impl;
		this->filename = filename;
	};

	void setDocumentLocator( QXmlLocator * locator )
	{
		this->locator = locator;
	}

	bool endDocument()
	{
		while( stack.count() > 1 )
			stack.pop(); // The parent node should take care of deleting the children

		if( stack.count() == 1 )
			delete stack.pop();

		return true;
	}

	bool startElement( const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts )
	{
		// Some Parent Elements have special meanings
		if( stack.isEmpty() )
		{
			if( qName == "definitions" )
				return true;

			// Include another file
			if( qName == "include" )
			{
				QString value = atts.value( "file" );
				impl->imports.push_back( value );
				return true;
			}
		}

		cElement *element = new cElement;
		element->setName( localName.latin1() );
		element->copyAttributes( atts );

		// Child Element ?
		if( stack.count() > 0 )
		{			
			cElement *parent = stack.current(); // Pop the potential parent
			parent->addChild( element ); // Add the child to it's parent
			element->setParent( parent );
		}

		stack.push( element ); // Push our element (there may be children)		
		return true;
	}

	bool endElement ( const QString & namespaceURI, const QString & localName, const QString & qName )
	{
		if( stack.isEmpty() )
			return true;
				
		cElement *element = stack.pop();

		// Did we complete a parent node?
		if( stack.isEmpty() )
		{
			// Find a category node
			unsigned int i = 0;
			
			while( categories[i].name != 0 )
			{
				if( element->name() == categories[i].name )
				{
					QString tagId = element->getAttribute( "id" );

					if( tagId != QString::null )
					{
						// <script id="XXXX">...</script>
						impl->unique[ categories[i].key ].insert( tagId, element );
					}
					else
					{
						impl->nonunique[ categories[i].key ].push_back( element );
					}

					return true;
				}

				++i;
			}

			clConsole.ProgressFail();
			clConsole.log( LOG_WARNING, QString( "Unknown element: %1 [File: %1, Line: %2]\n" ).arg( element->name() ).arg( filename ).arg( locator->lineNumber() ) );
			clConsole.PrepareProgress( "Parsing Definitions" );
			
			delete element;
		}

		return true;
	}

	bool characters ( const QString & ch )
	{
		if( !stack.isEmpty() )
		{
			cElement *element = stack.pop();
			element->setText( element->text() + ch );
			stack.push( element );
		}

		return true;
	}
};

// Recursive Function for Importing Script Sections
bool WPDefManager::ImportSections( const QString& FileName )
{
	QFile File( FileName );

    if ( !File.open( IO_ReadOnly ) )
	{
		clConsole.ProgressFail();
	
		clConsole.send( "Unable to open " );
		clConsole.send( FileName );
		clConsole.send( "!\n" );
		return false;
	}

	QXmlInputSource input( &File );	
	QXmlSimpleReader reader;
	reader.setFeature( "http://trolltech.com/xml/features/report-whitespace-only-CharData", false );

	cXmlHandler *handler = new cXmlHandler( impl, FileName );
	reader.setContentHandler( handler );

	reader.parse( &input, false );

	delete handler;

    File.close();
	return true;
}

void WPDefManager::unload( void )
{
	// Clear the nodes
	unsigned int i;
	
	for( i = 0; i < WPDT_COUNT; ++i )
	{
		QMap< QString, cElement* >::iterator it2;
		for( it2 = impl->unique[i].begin(); it2 != impl->unique[i].end(); ++it2 )
			delete it2.data();

		impl->unique[i].clear();

		QValueVector< cElement* >::iterator it;
		for( it = impl->nonunique[i].begin(); it != impl->nonunique[i].end(); ++it )
			delete *it;

		impl->nonunique[i].clear();
	}
	
	impl->imports.clear();
}

void WPDefManager::reload( void )
{
	unload();
	load();
	KeyManager::instance()->load();
}

// Load the Definitions
void WPDefManager::load( void )
{
	clConsole.PrepareProgress( "Loading Definitions" );
	
	ImportSections( "definitions.xml" );

	unsigned int i = 0;
	while( i < impl->imports.size() )
	{
		ImportSections( impl->imports[i] );
		++i;
	}

	clConsole.ProgressDone();

	cCommands::instance()->loadACLs();
}

// Returns a list of section-names found
QStringList WPDefManager::getSections( eDefCategory Type ) const
{
	// This is a VERY VERY slow function!
	QStringList result;

	QMap< QString, cElement* >::iterator it = impl->unique[ Type ].begin();

	while( it != impl->unique[ Type ].end() )
	{
		result.push_back( it.key() );
		it++;
	}

	return result;
}

QString	WPDefManager::getRandomListEntry( const QString& ListSection ) const
{
	QStringList list = this->getList( ListSection );
	if( list.isEmpty() )
		return QString();
	else
		return list[ RandomNum( 0, list.size()-1 ) ];
}

QStringList	WPDefManager::getList( const QString& ListSection ) const
{
	const cElement* DefSection = getDefinition( WPDT_LIST, ListSection );
	QStringList list;
	QString data;

	if( !DefSection )
		return list;

	for( unsigned int i = 0; i < DefSection->childCount(); ++i )
	{
		const cElement *childTag = DefSection->getChild( i );

		// Using the nodename is a very very bad habit
		// if the name of the node is "item" then
		// use the node value instead
	
		if( childTag->name() == "item" )
			data = childTag->text();
		else
			data = childTag->name();

		int mult = childTag->getAttribute( "mult" ).toInt();
		if( mult <= 0 )
			mult = 1;
		int i = 0;
		while( i < mult )
		{
			list.push_back( data );
			++i;
		}
	}

	return list;
}

QString WPDefManager::getText( const QString& TextSection ) const
{
	/*const QDomElement* DefSection = this->getSection( WPDT_TEXT, TextSection );
	if( DefSection->isNull() )
		return QString();
	else
	{
		QString text = DefSection->text();
		if( text.left( 1 ) == "\n" || text.left( 0 ) == "\r" )
			text = text.right( text.length()-1 );
		text = text.replace( QRegExp( "\\t" ), "" );
		if( text.right( 1 ) == "\n" || text.right( 1 ) == "\r" )
			text = text.left( text.length()-1 );
		return text;
	}*/
	return QString();
}

WPDefManager::WPDefManager()
{
	impl = new cDefManagerPrivate;
}

WPDefManager::~WPDefManager()
{
	unload();
	delete impl;
}


const cElement* WPDefManager::getDefinition( eDefCategory type, const QString& id ) const
{
	QMap< QString, cElement* >::const_iterator it = impl->unique[ type ].find( id );

	if( it == impl->unique[ type ].end() )
		return 0;
	else
		return it.data();
}

const QValueVector< cElement* > &WPDefManager::getDefinitions( eDefCategory type ) const
{
	return impl->nonunique[ type ];
}

/*
 *	Implementation of cElement
 */
void cElement::freeAttributes()
{
	if( attributes != 0 )
	{
		for( unsigned int i = 0; i < attrCount_; ++i )
			delete attributes[i];

		attrCount_ = 0;
		free( attributes );
		attributes = 0;
	}
}

void cElement::freeChildren()
{
	if( children != 0 )
	{
		for( unsigned int i = 0; i < childCount_; ++i )
			delete children[i];

		childCount_ = 0;
		free( children );
		children = 0;
	}
}

cElement::cElement()
{
	childCount_ = 0;
	attrCount_ = 0;
	name_ = QString::null;
	text_ = QString::null;
	children = 0;
	attributes = 0;
	parent_ = 0;
};

cElement::~cElement()
{
	freeAttributes();
	freeChildren();

	if( parent_ )
		parent_->removeChild( this );
}

void cElement::copyAttributes( const QXmlAttributes &attributes )
{
	freeAttributes();
	attrCount_ = attributes.count();

	if( attrCount_> 0 )
	{
		this->attributes = (stAttribute**)malloc( sizeof( stAttribute* ) * attrCount_ );
	
		for( unsigned int i = 0; i < attrCount_; ++i )
		{
			this->attributes[i] = new stAttribute;
			this->attributes[i]->name = attributes.localName( i );
			this->attributes[i]->value = attributes.value( i );
		}
	}
}

void cElement::addChild( cElement *element )
{
	if( children == 0 )
	{
		childCount_ = 1;
		children = (cElement**)malloc( sizeof( cElement* ) );
		children[0] = element;
	}
	else
	{
		children = (cElement**)realloc( children, sizeof( cElement* ) * ( childCount_ + 1 ) );
		children[childCount_++] = element;
	}
}

void cElement::removeChild( cElement *element )
{
	for( unsigned int i = 0; i < childCount_; ++i )
	{
		if( children[i] == element )
		{
			// Found the element we want to delete
			unsigned int offset = 0;
			
			cElement **newChildren = (cElement**)malloc( ( childCount_ - 1 ) * sizeof( cElement* ) );

			for( unsigned int j = 0; j < childCount_; ++j )
				if( children[j] != element )
					newChildren[offset++] = children[j];

			childCount_--;
			free( children );
			children = newChildren;
		}
	}
}

bool cElement::hasAttribute( const QCString &name ) const
{
	for( unsigned int i = 0; i < attrCount_; ++i )
		if( attributes[i]->name == name )
			return true;

	return false;
}

const QString &cElement::getAttribute( const QCString &name, const QString &def ) const
{
	for( unsigned int i = 0; i < attrCount_; ++i )
		if( attributes[i]->name == name )
			return attributes[i]->value;

	return def;
}

void cElement::setName( const QCString &data )
{
	name_ = data;
}

QString cElement::name() const
{
	return name_;
}

void cElement::setText( const QString &data )
{
	text_ = data;
}

const QString &cElement::text() const
{
	return text_;
}

void cElement::setParent( cElement *parent )
{
	parent_ = parent;
}

cElement *cElement::parent() const
{
	return parent_;
}

const cElement *cElement::findChild( const QString &name ) const
{
	for( unsigned int i = 0; i < childCount_; ++i )
	{
		if( children[i]->name() == name )
			return children[i];
	}
	
	return 0;
}

const cElement *cElement::getChild( unsigned int index ) const
{
	if( index >= childCount_ )
		return 0;

	return children[index];
}

unsigned int cElement::childCount() const
{
	return childCount_;
}

QString cElement::getValue() const
{
	QString Value = text_;

	for( unsigned int i = 0; i < childCount(); ++i )
	{
		const cElement *childTag = getChild( i );
		
		if( childTag->name() == "random" )
		{
			if( childTag->hasAttribute( "min" ) && childTag->hasAttribute( "max" ) )
			{
				QString min = childTag->getAttribute( "min" );
				QString max = childTag->getAttribute( "max" );

				if( min.contains( "." ) || max.contains( "." ) )
					Value += QString::number( RandomNum( min.toFloat(), max.toFloat() ) );
				else
					Value += QString::number( RandomNum( min.toInt(), max.toInt() ) );
			}
			else if( childTag->hasAttribute("valuelist") )
			{
				QStringList RandValues = QStringList::split( ",", childTag->getAttribute( "valuelist" ) );
				Value += RandValues[ RandomNum( 0, RandValues.size() - 1 ) ];
			}
			else if( childTag->hasAttribute( "list" ) )
			{
				Value += DefManager->getRandomListEntry( childTag->getAttribute( "list" ) );
			}
			else if( childTag->hasAttribute( "dice" ) )
			{
				Value += QString::number( rollDice( childTag->getAttribute( "dice" ) ) );
			}
			else if( childTag->hasAttribute( "value" ) )
			{
				QStringList parts = QStringList::split( "-", childTag->getAttribute( "value", "0-0" ) );
				
				if( parts.count() >= 2 )
				{
					QString min = parts[0];
					QString max = parts[1];

					if( max.contains( "." ) || min.contains( "." ) )
						Value += QString::number( RandomNum( min.toFloat(), max.toFloat() ) );
					else
						Value += QString::number( RandomNum( min.toInt(), max.toInt() ) );

				}
			}
			else
				Value += QString( "0" );
		}
	}

	return hex2dec( Value );
}
