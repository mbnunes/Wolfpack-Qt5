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

#ifndef __GUMPS_H__
#define __GUMPS_H__

#include "typedefs.h"
#include "objectdef.h"

#include <QStringList>
#include <vector>
#include <map>

// Forward Declarations
class cUObject;
class cUOSocket;
class cPage;
class cSpawnRegion;


struct gumpChoice_st
{
	signed int button;
	std::vector<unsigned int> switches;
	std::map<unsigned short, QString> textentries;
};


/*****************************************************************************
  cGump class
 *****************************************************************************/

class cGump
{
protected:
	SERIAL serial_, type_;
	int x_, y_;
	QStringList layout_, text_;
	bool noMove_, noClose_, noDispose_;
public:
	cGump();
	virtual ~cGump()
	{
	}

	SERIAL serial( void ) const;
	SERIAL type( void ) const;
	int x( void ) const;
	int y( void ) const;
	QStringList layout( void ) const;
	QStringList text( void ) const;
	bool noMove( void ) const;
	bool noDispose( void ) const;
	bool noClose( void ) const;

	void setType( SERIAL data );
	void setSerial( SERIAL data );
	void setX( int data );
	void setY( int data );
	void setTimeOut( quint32 data );
	void setNoMove( bool data );
	void setNoClose( bool data );
	void setNoDispose( bool data );

	void addRawLayout( const QString& data )
	{
		layout_.push_back( data );
	}
	quint32 addRawText( const QString& data );

	virtual void handleResponse( cUOSocket* socket, const gumpChoice_st& choice );

	// Comfort Setters
	void startPage( quint32 pageId = 0 )
	{
		layout_.push_back( QString( "{page %1}" ).arg( pageId ) );
	}
	void startGroup( quint32 groupId = 0 )
	{
		layout_.push_back( QString( "{group %1}" ).arg( groupId ) );
	}
	void addText( qint32 textX, qint32 textY, const QString& data, quint16 hue = 0 )
	{
		layout_.push_back( QString( "{text %1 %2 %3 %4}" ).arg( textX ).arg( textY ).arg( hue ).arg( addRawText( data ) ) );
	}
	void addBackground( quint16 gumpId, quint32 width, quint32 height )
	{
		layout_.push_back( QString( "{resizepic 0 0 %1 %2 %3}" ).arg( gumpId ).arg( width ).arg( height ) );
	}
	void addResizeGump( Q_INT16 gumpX, Q_INT16 gumpY, quint16 gumpId, quint32 width, quint32 height )
	{
		layout_.push_back( QString( "{resizepic %1 %2 %3 %4 %5}" ).arg( gumpX ).arg( gumpY ).arg( gumpId ).arg( width ).arg( height ) );
	}
	void addCroppedText( qint32 textX, qint32 textY, quint32 width, quint32 height, const QString& data, quint16 hue = 0 );

	// Buttons
	// TODO: IMPLEMENTATION
	void addButton( qint32 buttonX, qint32 buttonY, quint16 gumpUp, quint16 gumpDown, qint32 returnCode );
	void addPageButton( qint32 buttonX, qint32 buttonY, quint16 gumpUp, quint16 gumpDown, qint32 pageId );

	// Gump Pictures
	void addGump( qint32 gumpX, qint32 gumpY, quint16 gumpId, Q_INT16 hue = -1 );
	void addTiledGump( qint32 gumpX, qint32 gumpY, qint32 width, qint32 height, quint16 gumpId, Q_INT16 hue );

	// Art-tile pictures
	void addTilePic( qint32 tileX, qint32 tileY, quint16 tileId, Q_INT16 hue = 0 )
	{
		layout_.push_back( QString( "{tilepic %1 %2 %3 %4}" ).arg( tileX ).arg( tileY ).arg( tileId ).arg( hue ) );
	}

	// Form-fields
	// 7 = x,y,widthpix,widthchars,wHue,TEXTID,startstringindex
	void addInputField( qint32 textX, qint32 textY, quint32 width, quint32 height, qint32 textId, const QString& data, Q_INT16 hue = 0 )
	{
		layout_.push_back( QString( "{textentry %1 %2 %3 %4 %5 %6 %7}" ).arg( textX ).arg( textY ).arg( width ).arg( height ).arg( hue ).arg( textId ).arg( addRawText( data ) ) );
	}
	void addCheckbox( qint32 checkX, qint32 checkY, quint16 gumpOff, quint16 gumpOn, qint32 returnVal, bool checked = false )
	{
		layout_.push_back( QString( "{checkbox %1 %2 %3 %4 %5 %6}" ).arg( checkX ).arg( checkY ).arg( gumpOff ).arg( gumpOn ).arg( checked ? 1 : 0 ).arg( returnVal ) );
	}
	void addRadioButton( qint32 radioX, qint32 radioY, quint16 gumpOff, quint16 gumpOn, qint32 returnVal, bool checked = false )
	{
		layout_.push_back( QString( "{radio %1 %2 %3 %4 %5 %6}" ).arg( radioX ).arg( radioY ).arg( gumpOff ).arg( gumpOn ).arg( checked ? 1 : 0 ).arg( returnVal ) );
	}

	// HTML Stuff
	void addHtmlGump( qint32 x, qint32 y, qint32 width, qint32 height, const QString& html, bool hasBack = false, bool canScroll = false );
	void addXmfHtmlGump( qint32 x, qint32 y, qint32 width, qint32 height, quint32 clilocid, bool hasBack = false, bool canScroll = false );
	// void addXmfHtmlColorGump( );

	// CheckerTrans
	void addCheckertrans( qint32 x, qint32 y, qint32 width, qint32 height );
};

/*****************************************************************************
  cGump inline functions
 *****************************************************************************/

inline SERIAL cGump::serial( void ) const
{
	return serial_;
}

inline SERIAL cGump::type( void ) const
{
	return type_;
}

inline int cGump::x( void ) const
{
	return x_;
}

inline int cGump::y( void ) const
{
	return y_;
}

inline QStringList cGump::layout( void ) const
{
	return layout_;
}

inline QStringList cGump::text( void ) const
{
	return text_;
}

inline bool cGump::noMove( void ) const
{
	return noMove_;
}

inline bool cGump::noDispose( void ) const
{
	return noDispose_;
}

inline bool cGump::noClose( void ) const
{
	return noClose_;
}

inline void cGump::setType( SERIAL data )
{
	type_ = data;
}

inline void cGump::setSerial( SERIAL data )
{
	serial_ = data;
}

inline void cGump::setX( int data )
{
	x_ = data;
}

inline void cGump::setY( int data )
{
	y_ = data;
}

inline void cGump::setNoMove( bool data )
{
	noMove_ = data;
}

inline void cGump::setNoClose( bool data )
{
	noClose_ = data;
}

inline void cGump::setNoDispose( bool data )
{
	noDispose_ = data;
}

class cSpawnRegionInfoGump : public cGump
{
	OBJECTDEF( cSpawnRegionInfoGump )
protected:
	cSpawnRegion* region_;

public:
	cSpawnRegionInfoGump( cSpawnRegion* region );

	virtual void handleResponse( cUOSocket* socket, const gumpChoice_st& choice );
};

#endif
