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
		cItem::Init( false );
		this->setType( 11 ); // book type
		this->predefined_ = false;
		this->readonly_ = false;

		this->title_ = (char*)0;
		this->author_ = (char*)0;
		this->content_ = QStringList();

		this->section_ = (char*)0;

		this->pages_ = 0;
	}

	virtual ~cBook() {;}

	// abstract cSerializable
	virtual QString objectID( void ) const { return "BOOK"; }
	virtual void	Serialize( ISerialization &archive );

	// abstract cDefinable
	virtual void	processNode( const QDomElement &Tag );

	// setters/getters
	QString		title( void )			{ return title_; }
	QString		author( void )			{ return author_; }
	QStringList	content( void )			{ return content_; }
	UINT16		pages( void )			
	{ 
		if( predefined_ )
			return content_.size();
		else
			return pages_;
	}

	bool		predefined( void )		{ return predefined_; }
	bool		readonly( void )		{ return readonly_; }

	bool		writeable( void )		{ return !predefined_ && !readonly_; }

	QString		section( void )		{ return section_; }


	void		setAuthor( QString data )
	{
		this->author_ = data;
	}

	void		setTitle( QString data )
	{
		this->title_ = data;
	}

	void		setContent( QStringList data )
	{
		this->content_ = data;
	}

	void		setPredefined( bool data )
	{
		this->predefined_ = data;
	}

	void		setSection( QString data )
	{
		this->section_ = data;
	}

	void		setPages( UINT16 data )
	{
		this->pages_ = data;
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

	QString		title_;
	QString		author_;
	QStringList	content_;

	UINT16		pages_;
};

#endif
