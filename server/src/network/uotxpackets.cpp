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
//	* In addition to that license, if you are runningSteps this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================

#include "uotxpackets.h"
#include "uopacket.h"
#include "../items.h"
#include "../coord.h"
#include "../inlines.h"
#include "../basechar.h"
#include "../player.h"
#include "../globals.h"
#include "../wpdefmanager.h"

// Library Includes
#include <qstringlist.h>

void cUOTxShardList::addServer( Q_UINT16 serverIndex, QString serverName, Q_UINT8 serverFull, Q_INT8 serverTimeZone, Q_UINT32 serverIp )
{
	// Increase the server-count
	// Offset: 4
	setShort( 4, getShort( 4 ) + 1 );

	Q_INT32 offset = count();
	resize( count() + 40 ); // 40 byte per server
	setShort( 1, count() );
	setShort( offset, serverIndex );

	if( serverName.length() > 31 ) 
		serverName = serverName.left( 31 );

	setAsciiString( offset + 2, serverName.latin1(), serverName.length()+1 );

	(*this)[ offset + 34 ] = 0;
	(*this)[ offset + 35 ] = 0;
	setInt( offset + 36, serverIp );
}

void cUOTxCharTownList::addCharacter( QString name )
{
	// Trunace the name if needed
	if( name.length() > 29 )
		name = name.left( 29 );

	characters.push_back( name );
}

void cUOTxCharTownList::addTown( Q_UINT8 index, const QString &name, const QString &area )
{
	stTown town;
	town.town = ( name.length() > 30 ) ? name.left( 30 ) : name;
	town.area = ( area.length() > 30 ) ? area.left( 30 ) : area;
	town.index = index;
	towns.push_back( town );
}

void cUOTxCharTownList::compile( void )
{
	resize( 309 + ( towns.size() * 63 ) );
	(*this)[ 0 ] = (Q_UINT8)0xA9;

	(*this)[ 3 ] = characters.size(); // Char Count

	for( Q_UINT8 c = 0; c < 5; ++c )
		if( c < characters.size() )
		{
			setAsciiString( 4 + ( c * 60 ), characters[ c ].left( 29 ).latin1(), 30 );
			(*this)[ 4 + ( c * 60 ) + 30 ] = 0x00; // No Password (!)
		}
		else
			(*this)[ 4 + ( c * 60 ) ] = 0x00; // "Pad-out" the char

	// Town Count
	Q_INT32 offset = 304;
	(*this)[ offset++ ] = towns.size();

	for( Q_UINT8 t = 0; t < towns.size(); ++t )
	{
		(*this)[ offset ] = towns[ t ].index;
		setAsciiString( offset + 1, towns[ t ].town.left( 29 ).latin1(), 30 );
		setAsciiString( offset + 32, towns[ t ].area.left( 29 ).latin1(), 30 );
		offset += 63;
	}

	if( charLimit >= 0 )
		setInt( offset, ( charLimit << 4 ) | flags | 0x4 );
	else
		setInt( offset, flags );

	// New Packet Size
	setShort( 1, count() );
}

void cUOTxUpdateCharList::setCharacter( Q_UINT8 index, QString name )
{
	Q_INT32 offset = 4 + ( index * 60 );
	++(*this)[ 3 ];

	if( name.length() > 29 )
		name = name.left( 29 );

	setAsciiString(offset, name.latin1(), 30 );
}

void cUOTxSendSkills::addSkill( Q_UINT16 skillId, Q_UINT16 skill, Q_UINT16 realSkill, eStatus status, UINT16 cap )
{
	// Overwrite the last 2 bytes (terminator) and readd them later
	Q_INT32 offset = count() - 2;
	resize( count() + 9 );
	setShort( 1, count() );

	setShort( offset, skillId );
	setShort( offset+2, skill );
	setShort( offset+4, realSkill );
	(*this)[ offset+6 ] = status;

	setShort( offset+7, cap );
	setShort( offset+9, 0 ); // Terminator
}

void cUOTxDrawChar::addEquipment( Q_UINT32 serial, Q_UINT16 model, Q_UINT8 layer, Q_UINT16 color )
{
	// Overwrite the last 4 bytes (terminator) and readd them later
	Q_INT32 offset = count() - 4;
	resize( count() + 9 );
	setShort( 1, count() );

	setInt( offset, serial );
	setShort( offset+4, model|0x8000 );
	(*this)[offset+6] = layer;
	setShort( offset+7, color );
	setInt( offset+9, 0 ); // Terminator
}

void cUOTxUnicodeSpeech::setText( const QString &data )
{
	resize( 50 + (data.length()*2) );
	setShort( 1, count() );

	Q_INT32 offset = 48; // Pad right by one - remeber to copy one byte less
	(*this)[ offset ] = 0x00;
	QString tmpData = data; // get around the const
	setUnicodeString( offset , tmpData, (tmpData.length()*2) );
	//memcpy( &rawPacket.data()[ offset + 1 ], data.unicode(), (data.length()*2)-1 );

	// Add the new Terminator
	setShort( count() - 2, 0 );
}

// Sets all data automatically
void cUOTxConfirmLogin::fromChar( P_CHAR pChar )
{
	setSerial( pChar->serial() );
	setBody( pChar->bodyID() );
	setDirection( pChar->direction() );
	setX( pChar->pos().x );
	setY( pChar->pos().y );
	setZ( pChar->pos().z );
}

void cUOTxSendSkills::fromChar( P_CHAR pChar )
{
	if( !pChar )
		return;

	(*this)[3] = 0x02;

	for( UINT8 i = 0; i < ALLSKILLS; ++i )
	{
		eStatus status;

		switch( pChar->skillLock( i ) )
		{
		case 1:
			status = Down;
			break;
		case 2:
			status = Locked;
			break;
		default:
			status = Up;
		};

		addSkill( i+1, pChar->skillValue( i ), pChar->skillValue( i ), status, pChar->skillCap( i ) );
	}
}

void cUOTxMapDiffs::addEntry( UINT32 mappatches, UINT32 staticpatches )
{
	UINT16 size = count();
	UINT32 num = getInt( 5 );

	resize( size + 8 );
	setShort( 1, size + 8 );
	setInt( size, mappatches );
	setInt( size+4, staticpatches );
	setInt( 5, num + 1 );	
}

void cUOTxContextMenu::addEntry ( Q_UINT16 RetVal, Q_UINT16 msgID, Q_UINT16 flags, Q_UINT16 color ) 
{ 
	Q_UINT32 size = count(); 
	
	++(*this)[ 11 ]; 
	
	
	if ( flags & Popcolor ) 
	{ 
		resize( size + 8 ); 
		setShort( size+6, color ); 
		setShort( 1, size + 8 ); 
	} 
	else 
	{ 
		resize( size + 6 ); 
		setShort( 1, size + 6 ); 
	} 
	
	setShort( size, RetVal ); 
	setShort( size+2, msgID ); 
	setShort( size+4, flags ); 
	
} 

void cUOTxDenyMove::setCoord( Coord_cl coord )
{
	setShort( 2, coord.x );
	setShort( 4, coord.y );
	(*this)[7] = coord.z;
}

void cUOTxDenyMove::fromChar( P_CHAR pChar )
{
	setCoord( pChar->pos() );
	setDirection( pChar->direction() );
}

void cUOTxUpdatePlayer::fromChar( P_CHAR pChar )
{
	setSerial( pChar->serial() );
	setX( pChar->pos().x );
	setY( pChar->pos().y );
	setZ( pChar->pos().z );
	setHue( pChar->skin() );
	setBody( pChar->bodyID() );
	
	// If he's runningSteps we need to take that into account here
	// ->runningSteps() is greater than zero in that case
	setDirection( pChar->runningSteps() ? pChar->direction()|0x80 : pChar->direction() );

	if( pChar->isAtWar() )
		setFlag( 0x40 );

	if( pChar->isHidden() )
		setFlag( flag() | 0x80 );

	if( pChar->isDead() && !pChar->isAtWar() )
		setFlag( flag() | 0x80 );

	if( pChar->poisoned() )
		setFlag( flag() | 0x04 );
}

void cUOTxDrawChar::fromChar( P_CHAR pChar )
{
	setSerial( pChar->serial() );
	setModel( pChar->bodyID() );
	setX( pChar->pos().x );
	setY( pChar->pos().y );
	setZ( pChar->pos().z );
	setDirection( pChar->direction() );
	setColor( pChar->skin() );

	if( pChar->isAtWar() )
		setFlag( 0x40 );

	if( pChar->isHidden() || ( pChar->objectType() == enPlayer && !dynamic_cast<P_PLAYER>(pChar)->socket() ) )
		setFlag( flag() | 0x80 );

	if( pChar->isDead() && !pChar->isAtWar() )
		setFlag( flag() | 0x80 );

	if( pChar->poisoned() )
		setFlag( flag() | 0x04 );

	// Add our equipment - This does not seem to work !?
	bool layers[0x20] = {0,};

	cBaseChar::ItemContainer container(pChar->content());
	cBaseChar::ItemContainer::const_iterator it (container.begin());
	cBaseChar::ItemContainer::const_iterator end(container.end());
	for (; it != end; ++it )
	{
		P_ITEM pItem = *it;
		if( !pItem )
			continue;

		// Only send visible layers
		// 0x19 = horse layer
		// -> Shop containers need to be send
		if( pItem->layer() > 0x19 && pItem->layer() != 0x1A && pItem->layer() != 0x1B && pItem->layer() != 0x1C )
			continue;

		// Only send items once
		if( layers[ pItem->layer() ] )
			continue;

		layers[ pItem->layer() ] = true;

		addEquipment( pItem->serial(), pItem->id(), pItem->layer(), pItem->color() );
	}
}

void cUOTxCharEquipment::fromItem( P_ITEM pItem )
{
	if( !pItem->container() )
		return;

	setSerial( pItem->serial() );
	setModel( pItem->id() );
	setLayer( pItem->layer() );
	setWearer( pItem->container()->serial() );
	setColor( pItem->color() );
}

// Sends an update of ourself
void cUOTxDrawPlayer::fromChar( P_CHAR pChar )
{
	setSerial( pChar->serial() );
	setBody( pChar->bodyID() );
	setSkin( pChar->skin() );
	
	if( pChar->isAtWar() )
		setFlag( 0x40 );

	if( pChar->isHidden() )
		setFlag( flag() | 0x80 );

	if( pChar->isDead() && !pChar->isAtWar() )
		setFlag( flag() | 0x80 );

	if( pChar->poisoned() )
		setFlag( flag() | 0x04 );

	setX( pChar->pos().x );
	setY( pChar->pos().y );
	setZ( pChar->pos().z );
	setDirection( pChar->direction() );
	//void setFlags( Q_UINT8 data ) { rawPacket[ 10 ] = data; } // // 10 = 0=normal, 4=poison, 0x40=attack, 0x80=hidden CHARMODE_WAR
}

void cUOTxTipWindow::setMessage( const QCString& m )
{
	ushort length = m.length();
	resize( length + 11 );
	setShort(1, length + 11 );
	setShort(8, length );
	setAsciiString(10, m.data(), length+1);
}

void cUOTxAddContainerItem::fromItem( P_ITEM pItem )
{
	setSerial( pItem->serial() );
	setModel( pItem->id() );
	setAmount( pItem->amount() );
	setX( pItem->pos().x );
	setY( pItem->pos().y );

	if( pItem->container() )
		setContainer( pItem->container()->serial() );
	else
		setContainer( INVALID_SERIAL );
	
	setColor( pItem->color() );
}

void cUOTxOpenPaperdoll::fromChar( P_CHAR pChar, P_CHAR pOrigin )
{
	setSerial( pChar->serial() );

	QString nameByScript = pChar->onShowPaperdollName(pOrigin);

	if( !nameByScript.isNull() ) {
		setName( nameByScript );
	} else {
		QStringList titles = DefManager->getList("REPUTATION_TITLES");
		
		// Calculate the position inside the list
		unsigned int position;

		if (pChar->karma() >= 10000) {
			position = 0;
		} else if (pChar->karma() >= 5000) {
			position = 1;
		} else if (pChar->karma() >= 2500) {
			position = 2;
		} else if (pChar->karma() >= 1250) {
			position = 3;
		} else if (pChar->karma() >= 625) {
			position = 4;
		} else if (pChar->karma() <= -625) {
			position = 6;
		} else if (pChar->karma() <= -1250) {
			position = 7;
		} else if (pChar->karma() <= -2500) {
			position = 8;
		} else if (pChar->karma() <= -5000) {
			position = 9;
		} else if (pChar->karma() <= -10000) {
			position = 10;
		} else {
			position = 5;
		}

		position = (position * 5) + QMIN(4, pChar->fame() / 2500);

		if (position < titles.size()) {
			QString prefix = titles[position];
			if (prefix.length() > 0) {
				prefix.prepend("The ");
				prefix.append(" ");
			}

			// Lord/Lady Title
			if (pChar->fame() == 10000) {    
				prefix.append(pChar->gender() ? tr("Lady") : tr("Lord"));
				prefix.append(" ");
			}

			setName( prefix + pChar->name() + ( pChar->title().isEmpty() ? QString("") : ", " + pChar->title() ) );
		} else {
			setName( pChar->name() + ( pChar->title().isEmpty() ? QString("") : ", " + pChar->title() ) );
		}		
	}

	if( pChar->isAtWar() )
		setFlag( 0x40 );

	if( pChar->isHidden() )
		setFlag( flag() | 0x80 );

	if( pChar->isDead() && !pChar->isAtWar() )
		setFlag( flag() | 0x80 );

	if( pChar->poisoned() )
		setFlag( flag() | 0x04 );
}

void cUOTxBookPage::setPage( UINT16 page, UINT16 numLines, const QStringList &lines )
{
	setShort(  9 + currPageOffset, page );

	if( numLines == 0 )
	{
		setShort( 11 + currPageOffset, (UINT16)-1 ); // -1 if no lines!
		currPageOffset += 4;
	}
	else
	{
		setShort( 11 + currPageOffset, numLines );
		
		UINT32 currLineOffset = 0;
		QStringList::const_iterator it = lines.begin();
		while( it != lines.end() )
		{
			setAsciiString( 13 + currPageOffset + currLineOffset, (*it).latin1(), (*it).length()+1 );
			currLineOffset += (*it).length() + 1;
			it++;
		}
		currPageOffset += 4 + currLineOffset;
	}
}

void cUOTxCorpseEquipment::addItem( UINT8 layer, UINT32 serial )
{
	INT32 offset = count()-1;
	resize( size() + 5 );
	setShort( 1, size() );

	(*this)[ offset ] = layer;
	setInt( offset+1, serial );
	(*this)[ offset + 5 ] = 0;
}

void cUOTxItemContent::addItem( P_ITEM pItem )
{
	if( !pItem )
		return;
	SERIAL contserial = -1;
	if ( pItem->container() )
		contserial = pItem->container()->serial();

	addItem( pItem->serial(), pItem->id(), pItem->color(), pItem->pos().x, pItem->pos().y, pItem->amount(), contserial );
}

void cUOTxItemContent::addItem( SERIAL serial, UINT16 id, UINT16 color, UINT16 x, UINT16 y, UINT16 amount, UINT32 container )
{
	INT32 offset = size();
	resize( size() + 19 );
	setShort( 3, getShort( 3 ) + 1 );
	setShort( 1, size() );

	setInt( offset, serial );	
	setShort( offset+4, id );
	(*this)[ offset+6] = 0;
	setShort( offset+7, amount );
	setShort( offset+9, x );
	setShort( offset+11, y );
	setInt( offset+13, container );
	setShort( offset+17, color );
}

void cUOTxVendorBuy::addItem( UINT32 price, const QString &description )
{
	INT32 offset = size();
	resize( size() + 5 + description.length() + 1 ); // Null terminate it for gods-sake
	setShort( 1, size() );
	
	// Add the item itself
	setInt( offset, price );
	(*this)[ offset+4 ] = description.length() + 1;
	setAsciiString( offset + 5, description.latin1(), description.length() + 1 );
	
	++(*this)[7]; // Increase item count
}

void cUOTxGumpDialog::setContent( const QString& layout, const QStringList& text )
{
//	QString layout = gump->layout().join( "" );
	setShort( 19, layout.length()+1 );
	setAsciiString( 21, layout.latin1(), layout.length()+1 );

	// Send the unicode text-lines

	setShort( 22 + layout.length(), text.count() );

	Q_UINT32 offset = 22 + layout.length() + 2;
	QStringList::const_iterator it = text.begin();
	while( it != text.end() )
	{
		QString line = (*it);
		setShort( offset, line.length() );
		setUnicodeString( offset+2, line, line.length()*2 );
		offset += line.length() * 2 + 2;
		it++;
	}
}

void cUOTxTrade::setName( const QString &name )
{
	(*this)[16] = 1;
	resize( size() + name.length() + 1 );
	setShort( 1, size() );
	setAsciiString( 17, name.latin1(), name.length()+1 );
	//strcpy( &rawPacket.data()[17], name.latin1() );
}

void cUOTxProfile::setInfo( const QString& title, const QString& staticText, const QString& dynamicText )
{
	// Reset to the correct size
	UINT16 size = 13 + title.length() + ( staticText.length() * 2 ) + ( dynamicText.length() * 2 );
	setShort( 1, size );
	resize( size );

	setAsciiString( 7, title.latin1(), title.length()+1 );
	(*this)[7+title.length()] = 0; // Null Terminator

	setUnicodeString( 8+title.length(), staticText, staticText.length()*2 );
	setShort( 8+title.length()+(staticText.length()*2), 0 );

	setUnicodeString( 10+title.length()+(staticText.length()*2), dynamicText, dynamicText.length()*2 );
	setShort( 10+title.length()+(staticText.length()*2)+(dynamicText.length()*2), 0 );
}

void cUOTxSendItem::setCoord( const Coord_cl &coord )
{
	setShort( 11, coord.x | 0x8000 );
	setShort( 13, coord.y | 0xC000 );
	(*this)[16] = coord.z;
}

void cUOTxSoundEffect::setCoord( const Coord_cl &coord )
{
	setShort( 6, coord.x );
	setShort( 8, coord.y );
	setShort( 10, coord.z );
}

void cUOTxItemTarget::addItem( UINT16 id, INT16 deltaX, INT16 deltaY, INT16 deltaZ, UINT16 hue )
{
	// Add 10 Bytes
	INT32 offset = size();
	resize( offset + 10 );
	setShort( offset, id );
	setShort( offset+2, deltaX );
	setShort( offset+4, deltaY );
	setShort( offset+6, deltaZ );
	setShort( offset+8, hue );
}

void cUOTxTooltipList::addLine( UINT32 id, const QString& params )
{
	// 4 + 2 + length * 2
	UINT32 offset = size() - 4;
	resize( size() + ( 4 + 2 + params.length() * 2 ) );
	setShort( 1, size() );

	// Add our Line
	setInt( offset, id );
	setShort( offset + 4, params.length() * 2 );
	
	setUnicodeString( offset + 6, params, params.length() * 2, true );

	// Terminator
	setInt( size() - 4, 0 );
}

void cUOTxCustomHouse::addTile( UINT16 id, short x, short y, short z )
{
	UINT32 sz = size() + 5;
	resize( sz );
	// Increase counters
	setShort( 1, sz );					// total len
	setShort( 13, getShort( 13 ) + 1 );	// tiles count
	setShort( 15, sz - 0x11 );			// tiles data len
	setShort( sz - 5, id );
	(*this)[sz - 3] = x;
	(*this)[sz - 2] = y;
	(*this)[sz - 1] = z;
}

void cUOTxSellList::addItem( UINT32 serial, UINT16 id, UINT16 hue, 
							UINT16 amount, UINT16 value, const QString &name )
{
	UINT32 offset = size();
	UINT32 sz = size() + name.length() + 1 + 14; // null terminated ascii string
	resize( sz );
	setShort( 1, sz );

	setShort( 7, getShort( 7 ) + 1 ); // increase number of items
	setInt( offset, serial );
	setShort( offset + 4, id );
	setShort( offset + 6, hue );
	setShort( offset + 8, amount );
	setShort( offset + 10, value );
	setShort( offset + 12, name.length() + 1 );
	setAsciiString( offset + 14, name.latin1(), name.length() + 1 );
}