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

#if !defined(__BOOKS_H__)
#define __BOOKS_H__

// Library includes
#include <qstring.h>

// Wolfpack includes
#include "items.h"

// Forward Declarations
class cElement;
class QStringList;

class cBook : public cItem
{
	Q_OBJECT
private:
	static void buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions );

public:
	cBook();

	// abstract cDefinable
	virtual void	processNode( const cElement *Tag );

	// PersistentObject
	static void registerInFactory();
	void load( char **, UINT16& );
	void save();
	bool del();

	virtual void flagUnchanged() { changed_ = false; cItem::flagUnchanged();	}

	// FlatStore
	void save( FlatStore::OutputFile*, bool first = false ) throw();
	bool load( unsigned char chunkGroup, unsigned char chunkType, FlatStore::InputFile* ) throw();
	bool postload() throw();
	
	// setters/getters
	QString		title( void )	const;
	QString		author( void )	const;
	QStringList	content( void )	const;
	UINT16		pages( void )	const;		
	bool		predefined( void )	const;
	bool		readonly( void )	const;
	bool		writeable( void )	const;
	QString		section( void )	const;

	void		setAuthor( const QString& data );
	void		setTitle( const QString& data );
	void		setContent( const QStringList& data );
	void		setPredefined( bool data );
	void		setSection( const QString& data );
	void		setPages( UINT16 data );

	// networking
	void		open( cUOSocket* socket );
	void		readPage( cUOSocket* socket, UINT32 page );

	// methods for predefined books
	void		refresh( void );

	virtual stError		*setProperty( const QString &name, const cVariant &value );
	virtual stError		*getProperty( const QString &name, cVariant &value ) const;
private:
	bool		predefined_;
	bool		readonly_;

	QString		section_; // textsection for predefined books

	QString		title_;
	QString		author_;
	QStringList	content_;

	UINT16		pages_;
	bool		changed_;
};

// Inline methods
inline QString cBook::title( void )			const	{ return title_; }
inline QString cBook::author( void )		const	{ return author_; }
inline QStringList cBook::content( void )	const	{ return content_; }
inline UINT16 cBook::pages( void )			const		
{ 
	if( predefined_ )
		return content_.size();
	else
		return pages_;
}
inline bool cBook::predefined( void )		const	{ return predefined_; }
inline bool cBook::readonly( void )			const	{ return readonly_; }
inline bool cBook::writeable( void )		const	{ return !predefined_ && !readonly_; }
inline QString cBook::section( void )		const	{ return section_; }

inline void cBook::setAuthor( const QString& data )			{ this->author_ = data; changed_ = true; changed( TOOLTIP ); }
inline void cBook::setTitle( const QString& data )			{ this->title_ = data; changed_ = true; changed( TOOLTIP );  }
inline void cBook::setContent( const QStringList& data )	{ this->content_ = data; changed_ = true; }
inline void cBook::setPredefined( bool data )				{ this->predefined_ = data; changed_ = true; } 
inline void cBook::setSection( const QString& data )		{ this->section_ = data; changed_ = true; }
inline void cBook::setPages( UINT16 data )					{ this->pages_ = data; changed_ = true; changed( TOOLTIP ); }

#endif
