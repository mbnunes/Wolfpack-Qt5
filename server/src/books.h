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

#if !defined(__BOOKS_H__)
#define __BOOKS_H__

// Library includes
#include "qdom.h"
#include "qstring.h"

// Wolfpack includes
#include "items.h"

// Forward Declarations
class ISerialization;

class cBook : public cItem
{
public:
	cBook()
	{
		cItem::Init();
		this->setType( 11 ); // book type
		this->predefined_ = false;
		this->readonly_ = false;

		this->setTitle( (char*)0 );
		this->setAuthor( (char*)0 );
		this->setContent( QStringList() );

		this->section_ = (char*)0;
	}

	virtual ~cBook() {;}

	// abstract cSerializable
	virtual QString objectID( void ) const { return "BOOK"; }
	virtual void	Serialize( ISerialization &archive );

	// abstract cDefinable
	virtual void	processNode( const QDomElement &Tag );

	// setters/getters
	QString		title( void )			{ return this->tags.get( "title" ).toString(); }
	QString		author( void )			{ return this->tags.get( "author" ).toString(); }
	QStringList	content( void )			{ return this->tags.get( "content" ).toStringList(); }

	bool		predefined( void )		{ return predefined_; }
	bool		readonly( void )		{ return readonly_; }

	QString		section( void )		{ return section_; }


	bool		setAuthor( QString data )
	{
		if( !this->readonly_ )
			this->tags.set( "author", data );

		return this->readonly_ && !this->predefined_;
	}

	bool		setTitle( QString data )
	{
		if( !this->readonly_ )
			this->tags.set( "title", data );

		return this->readonly_ && !this->predefined_;
	}

	bool		setContent( QStringList data )
	{
		if( !this->readonly_ )
			this->tags.set( "content", data );

		return this->readonly_ && !this->predefined_;
	}

	void		setPredefined( bool data )
	{
		this->predefined_ = data;
	}

	void		setSection( QString data )
	{
		this->section_ = data;
	}

	// networking
	void		open( cUOSocket* socket );
	void		readPage( cUOSocket* socket, UINT32 page );

	// methods for predefined books
	void		refresh( void );

private:
	bool		predefined_;
	bool		readonly_;

	QString		section_; // textsection for predefined books
};

/*
// Book class by Lord Binary, 7'th december 1999

// new books readonly -> morex  999
// new books writeable -> morex 666
// old books from misc.scp, readonly: neither type 666 nor 999, section# = more-value
// of course ALL books need a type of 11

// note to everyone who wants to learn C++ -> this is NOT stricly OOP and has no good OOD.
// hence no real good example for OO-P :)
// for example inheritence could be used, public attributes arnt nice etc,etc.


// Platform Specifics
#include "platform.h"

// System Includes
#include <iostream>

using namespace std ;

// Forward class declaration
class cBooks ;

// Wolfpack Includes
#include "wolfpack.h"
#include "debug.h"

// Class defination

class cBooks
{
	private:
		// that private methods read and write from the *.bok files
		void write_author(P_ITEM pBook,UOXSOCKET s);
		void write_title(P_ITEM pBook, UOXSOCKET s);
		void write_line(P_ITEM pBook, int page, int line, char linestr[34], UOXSOCKET s);
		void read_author(P_ITEM pBook,char auth[31]);
		void read_title(P_ITEM pBook,char title[61]);
		int  read_number_of_pages(P_ITEM pBook);
		void read_line(P_ITEM pBook, int page,int linenumber, char line[33]);
		signed char make_new_book_file(char *fileName, P_ITEM pBook); // "formats and creates a new bok file"
					
	public:
		char a_t; // flag -> set if author and title changed		
		char authorbuffer[MAXCLIENT][32]; 
		char titlebuffer[MAXCLIENT][62];
		char pagebuffer[MAXCLIENT][512]; //i think 256 is enough (8 lines *32 chars per line = 256, but i took 512 to be on the safe side and avoid crashes 
        void openbook_old(UOXSOCKET s, P_ITEM pBook); // opens old-readonly books, takes data from misc.scp
		void openbook_new(UOXSOCKET s, P_ITEM pBook,char writeable); // opens new books
		void readbook_readonly_old(UOXSOCKET s, P_ITEM pBook, int p); // reads books from misc.scp, readonly = old books
		void readbook_readonly(UOXSOCKET s, P_ITEM pBook, int p);     // reads new books readonly ( from *.bok file )
		void readbook_writeable(UOXSOCKET s, P_ITEM pBook, int p, int l); // writes changes to a new book opened in writable mode 
		void delete_bokfile(P_ITEM pBook); // deletes bok-file.
		virtual ~cBooks() {};
        	
};
*/

#endif
