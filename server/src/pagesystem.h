/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: http://wpdev.sf.net/
 */

#ifndef __PAGESYSTEM_H__
#define __PAGESYSTEM_H__

// Wolfpack includes
#include "typedefs.h"

#include "definitions.h"
#include "persistentbroker.h"
#include "dbdriver.h"
#include "basics.h"

// Library includes
#include "qstring.h"
#include "qstringlist.h"
#include <deque>

class cPage
{
private:
	SERIAL charserial_;
	WPPAGE_TYPE pagetype_;
	QString pagetime_;
	Coord_cl pagepos_;
	QString content_;
	UINT32 pagecateg_;
public:
	cPage( SERIAL charserial, WPPAGE_TYPE pagetype, QString content, Coord_cl pagepos )
	{
		charserial_ = charserial;
		pagetype_ = pagetype;
		pagetime_ = QDateTime::currentDateTime().toString();
		pagepos_ = pagepos;
		content_ = content;
		pagecateg_ = 0;
	}

	cPage()
	{
	}

	~cPage()
	{
	}

	SERIAL charSerial() const
	{
		return charserial_;
	}
	WPPAGE_TYPE pageType() const
	{
		return pagetype_;
	}
	QString pageTime() const
	{
		return pagetime_;
	}
	Coord_cl pagePos() const
	{
		return pagepos_;
	}
	QString content() const
	{
		return content_;
	}
	UINT32 pageCategory() const
	{
		return pagecateg_;
	}

	void setCharSerial( SERIAL data )
	{
		charserial_ = data;
	}
	void setPageType( WPPAGE_TYPE data )
	{
		pagetype_ = data;
	}
	void setPagePos( Coord_cl data )
	{
		pagepos_ = data;
	}
	void setPageTime( void )
	{
		pagetime_ = QDateTime::currentDateTime().toString();
	}
	void setPageTime( QString data )
	{
		pagetime_ = data;
	}
	void setContent( QString data )
	{
		content_ = data;
	}
	void setPageCategory( UINT32 data )
	{
		pagecateg_ = data;
	}
};

class cPagesManager : public std::deque<cPage*>
{
protected:
	cPagesManager()
	{
	}
	cPagesManager( cPagesManager& )
	{
	} // Unallow copy constructor
	cPagesManager& operator=( cPagesManager& )
	{
		return *this;
	} // Unallow Assignment
public:
	void save()
	{
		PersistentBroker::instance()->executeQuery( "DELETE FROM pages" );

		int i = 0;
		cPagesManager::const_iterator it = begin();
		while ( it != end() )
		{
			cPage* page = *it;

			QString sql( "REPLACE INTO pages SET charserial = '%1', pagetype = '%2', pagetime = '%3', pagepos = '%4', content = '%5', category = '%6', pageorder = '%7'" );
			sql = sql.arg( page->charSerial() ).arg( page->pageType() ).arg( PersistentBroker::instance()->quoteString( page->pageTime() ) ).arg( QString( "%1,%2,%3,%4" ).arg( page->pagePos().x ).arg( page->pagePos().y ).arg( page->pagePos().z ).arg( page->pagePos().map ) ).arg( PersistentBroker::instance()->quoteString( page->content() ) ).arg( page->pageCategory() ).arg( i++ );
			PersistentBroker::instance()->executeQuery( sql );

			++it;
		}
	}

	void load()
	{
		cDBResult result = PersistentBroker::instance()->query( "SELECT charserial,pagetype,pagetime,pagepos,content,category,pageorder FROM pages ORDER BY pageorder ASC" );
		while ( result.fetchrow() )
		{
			char** row = result.data();
			cPage* page = new cPage;
			page->setCharSerial( atoi( row[0] ) );
			page->setPageType( ( WPPAGE_TYPE ) atoi( row[1] ) );
			page->setPageTime( row[2] );
			Coord_cl pos;
			parseCoordinates( row[3], pos );
			page->setPagePos( pos );
			page->setContent( row[4] );
			page->setPageCategory( atoi( row[5] ) );
			push_back( page );
		}
		result.free();
	}

	~cPagesManager()
	{
		cPagesManager::iterator it = begin();
		while ( it != end() )
		{
			delete ( *it );
			++it;
		}
	}

	static cPagesManager* getInstance()
	{
		static cPagesManager thePagesManager;
		return &thePagesManager;
	}

	/*	void insert( cPage* page )
		{
			bool stillexists = false;
			cPagesManager::iterator it = begin();
			while( it != end() )
			{
				if( (*it)->charSerial() == page->charSerial() )
				{
					stillexists = true;
					(*it) = page;
				}
				it++;
			}
			if( !stillexists )
				push_back( page );
		}*/

	bool contains( const cPage* page ) const
	{
		cPagesManager::const_iterator it = begin();
		while ( it != end() )
		{
			if ( ( *it ) == page )
				return true;
			++it;
		}
		return false;
	}

	void moveOnTop( cPage* page )
	{
		cPagesManager::iterator it = begin();
		while ( it != end() )
		{
			if ( ( *it ) == page )
			{
				erase( it );
				break;
			}
			it++;
		}
		push_front( page );
	}

	void remove( cPage* page )
	{
		cPagesManager::iterator it = begin();
		while ( it != end() )
		{
			if ( ( *it ) == page )
			{
				erase( it );
				break;
			}
			++it;
		}
		delete page;
	}

	cPage* find( SERIAL charserial ) const
	{
		cPagesManager::const_iterator it = begin();
		while ( it != end() )
		{
			if ( ( *it )->charSerial() == charserial )
				return ( *it );
			++it;
		}
		return NULL;
	}

	QStringList categories( void )
	{
		QStringList categories = Definitions::instance()->getList( "PAGE_CATEGORIES" );
		if ( categories.count() == 0 )
			categories.push_back( "none" );
		return categories;
	}
};

#endif
