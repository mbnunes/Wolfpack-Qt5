//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
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
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

// V2.0 20-jun-2002 rewritten - sereg

#include "books.h"
#include "network/uotxpackets.h"
#include "network/uosocket.h"
#include "wpdefmanager.h"

#include "qregexp.h"

#undef DBGFILE
#define DBGFILE "books.cpp"

void cBook::Serialize( ISerialization &archive )
{
	if( archive.isReading() )
	{
		archive.read( "book.title", title_ );
		archive.read( "book.author", author_ );
		UINT32 contsize = 0;
		archive.read( "book.contsize", contsize );
		content_.clear();
		UI32 i;
		for( i = 0; i < contsize; i++ )
		{
			QString currPage = (char*)0;
			archive.read( (char*)QString("book.content.page%1").arg(i).latin1(), currPage );
			while( i >= content_.size() ) // lets fill it up with empty strings
				content_.push_back( QString("") );
			content_[i] = currPage;
		}
		archive.read( "book.readonly", readonly_ );
		archive.read( "book.predefined", predefined_ );
		archive.read( "book.section", section_ );
		archive.read( "book.pages", pages_ );
	}
	else
	{
		archive.write( "book.title", title_ );
		archive.write( "book.author", author_ );
		UINT32 contsize = content_.size();
		archive.write( "book.contsize", contsize );
		UI32 i;
		for( i = 0; i < content_.size(); i++ )
		{
			archive.write( (char*)QString("book.content.page%1").arg(i).latin1(), content_[i] );
		}
		archive.write( "book.readonly", readonly_ );
		archive.write( "book.predefined", predefined_ );
		archive.write( "book.section", section_ );
		archive.write( "book.pages", pages_ );
	}
	cItem::Serialize( archive );
}

void cBook::processNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	QString Value = this->getNodeValue( Tag );

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
		QDomNode childNode = Tag.firstChild();
		while( !childNode.isNull() )
		{
			if( childNode.isElement() )
			{
				QDomElement childTag = childNode.toElement();
				QString text = childTag.text();
				text = text.replace( QRegExp( "\\t" ), "" );
				while( text.left( 1 ) == "\n" || text.left( 1 ) == "\r" )
					text = text.right( text.length()-1 );
				while( text.right( 1 ) == "\n" || text.right( 1 ) == "\r" )
					text = text.left( text.length()-1 );

				if( childTag.attributes().contains("no") )
				{
					UINT32 n = childTag.attribute( "no" ).toShort();

					// fill it up with blanks
					while( content_.size() < n )
						content_.push_back( "" );

					content_[ n - 1 ] = text;
				}
				else
					content_.push_back( text );
			}
			childNode = childNode.nextSibling();
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
}

void cBook::refresh( void )
{
	QDomElement* DefSection = DefManager->getSection( WPDT_ITEM, section_ );
	if( !DefSection->isNull() )
	{
		QDomNode childNode = DefSection->firstChild();
		while( !childNode.isNull() )
		{
			if( childNode.isElement() )
			{
				QDomElement Tag = childNode.toElement();
				QString TagName = Tag.nodeName();
				QString Value = this->getNodeValue( Tag );

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
					QStringList content = QStringList();
					QDomNode chchildNode = Tag.firstChild();
					while( !chchildNode.isNull() )
					{
						if( chchildNode.isElement() )
						{
							QDomElement chchildTag = chchildNode.toElement();
							QString text = chchildTag.text();
							text = text.replace( QRegExp( "\\t" ), "" );
							while( text.left( 1 ) == "\n" || text.left( 1 ) == "\r" )
								text = text.right( text.length()-1 );
							while( text.right( 1 ) == "\n" || text.right( 1 ) == "\r" )
								text = text.left( text.length()-1 );

							if( chchildTag.attributes().contains("no") )
							{
								UINT32 n = chchildTag.attribute( "no" ).toShort();

								// fill it up with blanks
								while( content.size() < n )
									content.push_back( "" );

								content[ n - 1 ] = text;
							}
							else
								content.push_back( text );
						}
						chchildNode = chchildNode.nextSibling();
					}
					this->content_ = content;
				}
			}
			childNode = childNode.nextSibling();
		}
	}
}

void cBook::open( cUOSocket* socket )
{
	if( this->predefined_ )
		this->refresh();

	cUOTxBookTitle openBook;
	openBook.setSerial( this->serial );
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
		readBook.setSerial( this->serial );
		readBook.setPages( content_.count() );

		std::vector< QStringList >::iterator it = lines.begin();
		i = 0;
		while( it != lines.end() )
		{
			readBook.setPage( i+1, (*it).size(), (*it) );
			i++;
			it++;
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
	readBook.setSerial( this->serial );
	readBook.setPages( 1 );

	readBook.setPage( page, lines.size(), lines );

	socket->send( &readBook );
}

