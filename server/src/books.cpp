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

// V2.0 20-jun-2002 rewritten - sereg

#include "books.h"
#include "network/uotxpackets.h"
#include "network/uosocket.h"
#include "wpdefmanager.h"
#include "persistentbroker.h"
#include "dbdriver.h"
#include "globals.h"

#include <qregexp.h>
#include <qsqlcursor.h>

#undef DBGFILE
#define DBGFILE "books.cpp"

static cUObject* productCreator()
{
	return new cBook;
}

void cBook::registerInFactory()
{
	QStringList fields, tables, conditions;
	buildSqlString( fields, tables, conditions ); // Build our SQL string
	QString sqlString = QString( "SELECT uobjectmap.serial,uobjectmap.type,%1 FROM uobjectmap,%2 WHERE uobjectmap.type = 'cBook' AND %3" ).arg( fields.join( "," ) ).arg( tables.join( "," ) ).arg( conditions.join( " AND " ) );
	UObjectFactory::instance()->registerSqlQuery( "cBook", sqlString );

	UObjectFactory::instance()->registerType( "cBook", productCreator );
}

cBook::cBook()
{
	cItem::Init( false );
	this->setType( 11 ); // book type
	this->predefined_ = false;
	this->readonly_ = false;
	this->title_ = QString::null;
	this->author_ = QString::null;
	this->section_ = QString::null;
	this->pages_ = 16;
	this->changed( TOOLTIP );
	changed_ = true;
}

void cBook::buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions )
{
	cItem::buildSqlString( fields, tables, conditions );
	fields.push_back( "books.predefined,books.readonly,books.title,books.author,books.section,books.pages" );
	tables.push_back( "books" );
	conditions.push_back( "uobjectmap.serial = books.serial" );
}

void cBook::load( char **result, UINT16 &offset )
{
	cItem::load( result, offset );
	predefined_ = atoi( result[offset++] );
	readonly_ = atoi( result[offset++] );
	title_ = result[offset++];
	author_ = result[offset++];
	section_ = result[offset++];
	pages_ = atoi( result[offset++] );

	// Load the pages
	cDBResult res = persistentBroker->query( QString( "SELECT page,text FROM bookpages WHERE serial = '%1'" ).arg( serial() ) );

	while( res.fetchrow() )
	{
		while( content_.size() <= res.getInt( 0 ) )
			content_.push_back( "" );
		content_[ res.getInt( 0 ) ] = res.getString( 1 );
	}

	res.free();
	changed_ = false;
}

enum eChunkTypes
{
	BOOK_PREDEFINED = 0x01,	// flag
	BOOK_READONLY,			// flag
	BOOK_TITLE,				// utf8
	BOOK_AUTHOR,			// author
	BOOK_SECTION,			// utf8
	BOOK_PAGES,				// unsigned short + count * utf8
};

void cBook::save()
{
	if ( changed_ )
	{
		initSave;
		setTable( "books" );

		addField( "serial", serial() );
		addField( "predefined", predefined_ ? 1 : 0 );
		addField( "readonly", readonly_ ? 1 : 0 );
		addStrField( "title", title_ );
		addStrField( "author", author_ );
		addStrField( "section", section_ );
		addField( "pages", content_.count() );
	
		addCondition( "serial", serial() );
		saveFields;

		// Delete all Pages from the DB and reinsert them
		// The Amount of pages CAN change!
		if( isPersistent )
			persistentBroker->executeQuery( QString( "DELETE FROM bookpages WHERE serial = '%1'" ).arg( serial() ) );
	
		UINT32 i = 0;
		for ( QStringList::iterator it = content_.begin(); it != content_.end(); ++it )
		{
			persistentBroker->executeQuery( QString( "INSERT INTO bookpages VALUES (%1,%2,'%3')" ).arg( serial() ).arg( i ).arg( persistentBroker->quoteString( *it ) ) );
			++i;
		}
	}
	cItem::save();
	changed_ = false;
}

bool cBook::del()
{
	if( !isPersistent )
		return false;

	persistentBroker->addToDeleteQueue( "books", QString( "serial = '%1'" ).arg( serial() ) );
	persistentBroker->addToDeleteQueue( "bookpages", QString( "serial = '%1'" ).arg( serial() ) );
	changed_ = true;
	return cItem::del();
}

void cBook::processNode( const cElement *Tag )
{
	QString TagName = Tag->name();
	QString Value = Tag->getValue();

	//	<title>blabla</title>
	if( TagName == "title" )
	{
		setTitle( Value );
	}

	//	<author>blabla</author>
	else if( TagName == "author" )
	{
		setAuthor( Value );
	}

	//	<content>
	//		<page no="1">
	//			sdjkjsdk
	//			asdjkasdjk
	//		</page>
	//		<page>
	//		...
	//		</page>
	//	</content>
	else if( TagName == "content" )
	{
		for( unsigned int i = 0; i < Tag->childCount(); ++i )
		{
			const cElement *childTag = Tag->getChild( i );

			QString text = childTag->text();
			text = text.replace( QRegExp( "\\t" ), "" );
			while( text.left( 1 ) == "\n" || text.left( 1 ) == "\r" )
				text = text.right( text.length()-1 );
			while( text.right( 1 ) == "\n" || text.right( 1 ) == "\r" )
				text = text.left( text.length()-1 );

			if( childTag->hasAttribute( "no" ) )
			{
				UINT32 n = childTag->getAttribute( "no" ).toShort();
				while( content_.size() <= n )
					content_.push_back( "" );
				content_[ n - 1 ] = text;
			}
			else
				content_.push_back( text );
		}
	}

	//	<readonly />
	else if( TagName == "readonly" )
		readonly_ = true;

	//	<predefined />
	else if( TagName == "predefined" )
		predefined_ = true;

	//	<pages>16</pages>
	else if( TagName == "pages" )
		pages_ = Value.toShort();

	else
		cItem::processNode( Tag );
	changed_ = true;
}

void cBook::refresh( void )
{
	const cElement* section = DefManager->getDefinition( WPDT_ITEM, section_ );
	applyDefinition( section );
}

void cBook::open( cUOSocket* socket )
{
	if( this->predefined_ )
		this->refresh();

	cUOTxBookTitle openBook;
	openBook.setSerial( this->serial() );
	openBook.setWriteable( writeable() );
	if( writeable() )
		openBook.setFlag( 1 );
	openBook.setPages( pages() );
	openBook.setTitle( title_ );
	openBook.setAuthor( author_ );

	socket->send( &openBook );

	// for writeable books we have to send the entire book...
	if( writeable() ) 
	{
		std::vector< QStringList > lines;
		UINT32 i, size = 9;
		for( i = 0; i < content_.count(); i++ )
		{
			QStringList tmpLines = QStringList::split( "\n", content_[i], true );
			size += 4;
			QStringList::const_iterator it = tmpLines.begin();
			while( it != tmpLines.end() )
			{
				size += (*it).length()+1; //null terminated lines!
				it++;
			}
			lines.push_back( tmpLines );
		}

		cUOTxBookPage readBook( size );
		readBook.setBlockSize( (UINT16)size );
		readBook.setSerial( this->serial() );
		readBook.setPages( content_.count() );

		std::vector< QStringList >::iterator it = lines.begin();
		i = 0;
		while( it != lines.end() )
		{
			readBook.setPage( i+1, (*it).size(), (*it) );
			++i;
			++it;
		}

		socket->send( &readBook );
	}
}

void cBook::readPage( cUOSocket *socket, UINT32 page )
{
	if( this->predefined_ )
		this->refresh();

	if( page > content_.size() )
		return;

	QStringList lines = QStringList::split( "\n", content_[page-1], true );

	UINT32 size = 13;
	QStringList::const_iterator it = lines.begin();
	while( it != lines.end() )
	{
		size += (*it).length()+1; //null terminated lines!
		it++;
	}

	cUOTxBookPage readBook( size );

	readBook.setBlockSize( (UINT16)size );
	readBook.setSerial( this->serial() );
	readBook.setPages( 1 );

	readBook.setPage( page, lines.size(), lines );

	socket->send( &readBook );
}

stError *cBook::setProperty( const QString &name, const cVariant &value )
{
	changed( TOOLTIP );
	changed_ = true;
	SET_BOOL_PROPERTY( "readonly", readonly_ )
	else SET_BOOL_PROPERTY( "predefined", predefined_ )
	else SET_STR_PROPERTY( "section", section_ )
	else SET_STR_PROPERTY( "title", title_ )
	else SET_STR_PROPERTY( "author", author_ )
	else SET_INT_PROPERTY( "pages", pages_ )

	return cItem::setProperty( name, value );
}

stError *cBook::getProperty( const QString &name, cVariant &value ) const
{
	GET_PROPERTY( "readonly", readonly_ )
	else GET_PROPERTY( "predefined", predefined_ )
	else GET_PROPERTY( "section", predefined_ )
	else GET_PROPERTY( "title", predefined_ )
	else GET_PROPERTY( "author", predefined_ )
	else GET_PROPERTY( "pages", pages_ )

	return cItem::getProperty( name, value );
}
