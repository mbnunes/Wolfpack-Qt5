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

#include "uotxpackets.h"
#include "uopacket.h"
#include "../items.h"
#include "../coord.h"
#include "../inlines.h"
#include "../basechar.h"
#include "../player.h"

#include "../definitions.h"
#include "../skills.h"

// Library Includes
#include <qstringlist.h>

void cUOTxShardList::addServer( unsigned short serverIndex, QString serverName, unsigned char serverFull, char serverTimeZone, unsigned int serverIp )
{
	// Increase the server-count
	// Offset: 4
	setShort( 4, getShort( 4 ) + 1 );

	int offset = count();
	resize( count() + 40 ); // 40 byte per server
	setShort( 1, count() );
	setShort( offset, serverIndex );

	if ( serverName.length() > 31 )
		serverName = serverName.left( 31 );

	setAsciiString( offset + 2, serverName.latin1(), serverName.length() + 1 );

	( *this )[offset + 34] = 0;
	( *this )[offset + 35] = 0;
	setInt( offset + 36, serverIp );
}

void cUOTxCharTownList::addCharacter( const QString& name )
{
	// Trunace the name if needed
	characters.push_back( name.length() > 29 ? name.left( 29 ) : name );
}

void cUOTxCharTownList::addTown( unsigned char index, const QString& name, const QString& area )
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
	( *this )[0] = ( unsigned char ) 0xA9;

	( *this )[3] = characters.size(); // Char Count

	for ( unsigned char c = 0; c < 5; ++c )
		if ( c < characters.size() )
		{
			setAsciiString( 4 + ( c * 60 ), characters[c].left( 29 ).latin1(), 30 );
			( *this )[4 + ( c * 60 ) + 30] = 0x00; // No Password (!)
		}
		else
			( *this )[4 + ( c * 60 )] = 0x00; // "Pad-out" the char

	// Town Count
	int offset = 304;
	( *this )[offset++] = towns.size();

	for ( unsigned char t = 0; t < towns.size(); ++t )
	{
		( *this )[offset] = towns[t].index;
		setAsciiString( offset + 1, towns[t].town.left( 29 ).latin1(), 30 );
		setAsciiString( offset + 32, towns[t].area.left( 29 ).latin1(), 30 );
		offset += 63;
	}

	if ( charLimit >= 0 )
		setInt( offset, ( charLimit << 4 ) | flags | 0x4 );
	else
		setInt( offset, flags );

	// New Packet Size
	setShort( 1, count() );
}

void cUOTxUpdateCharList::setCharacter( unsigned char index, QString name )
{
	int offset = 4 + ( index * 60 );
	++( *this )[3];

	if ( name.length() > 29 )
		name = name.left( 29 );

	setAsciiString( offset, name.latin1(), 30 );
}

void cUOTxSendSkills::addSkill( unsigned short skillId, unsigned short skill, unsigned short realSkill, eStatus status, unsigned short cap )
{
	// Overwrite the last 2 bytes (terminator) and readd them later
	int offset = count() - 2;
	resize( count() + 9 );
	setShort( 1, count() );

	setShort( offset, skillId );
	setShort( offset + 2, skill );
	setShort( offset + 4, realSkill );
	( *this )[offset + 6] = status;

	setShort( offset + 7, cap );
	setShort( offset + 9, 0 ); // Terminator
}

void cUOTxUnicodeSpeech::setText( const QString& data )
{
	resize( 50 + ( data.length() * 2 ) );
	setShort( 1, count() );

	int offset = 48; // Pad right by one - remeber to copy one byte less
	( *this )[offset] = 0x00;
	QString tmpData = data; // get around the const
	setUnicodeString( offset, tmpData, ( tmpData.length() * 2 ) );
	//memcpy( &rawPacket.data()[ offset + 1 ], data.unicode(), (data.length()*2)-1 );

	// Add the new Terminator
	setShort( count() - 2, 0 );
}

// Sets all data automatically
void cUOTxConfirmLogin::fromChar( P_CHAR pChar )
{
	if ( pChar->isDead() )
	{
		setBody( pChar->gender() ? 0x193 : 0x192 );
	}
	else
	{
		setBody( pChar->body() );
	}

	setSerial( pChar->serial() );
	setDirection( pChar->running() ? (pChar->direction() | 0x80) : pChar->direction() );
	setX( pChar->pos().x );
	setY( pChar->pos().y );
	setZ( pChar->pos().z );
}

void cUOTxSendSkills::fromChar( P_CHAR pChar )
{
	if ( !pChar )
		return;

	( *this )[3] = 0x02;

	for ( unsigned char i = 0; i < ALLSKILLS; ++i )
	{
		eStatus status;

		switch ( pChar->skillLock( i ) )
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

		addSkill( i + 1, pChar->skillValue( i ), pChar->skillValue( i ), status, pChar->skillCap( i ) );
	}
}

void cUOTxMapDiffs::addEntry( unsigned int mappatches, unsigned int staticpatches )
{
	unsigned short size = count();
	unsigned int num = getInt( 5 );

	resize( size + 8 );
	setShort( 1, size + 8 );
	setInt( size, staticpatches );
	setInt( size + 4, mappatches );
	setInt( 5, num + 1 );
}

void cUOTxContextMenu::addEntry( unsigned short RetVal, unsigned short msgID, unsigned short flags, unsigned short color )
{
	unsigned int size = count();

	++( *this )[11];


	if ( flags & Popcolor )
	{
		resize( size + 8 );
		setShort( size + 6, color );
		setShort( 1, size + 8 );
	}
	else
	{
		resize( size + 6 );
		setShort( 1, size + 6 );
	}

	setShort( size, RetVal );
	setShort( size + 2, msgID );
	setShort( size + 4, flags );
}

void cUOTxDenyMove::setCoord( const Coord_cl& coord )
{
	setShort( 2, coord.x );
	setShort( 4, coord.y );
	( *this )[7] = coord.z;
}

void cUOTxDenyMove::fromChar( P_CHAR pChar )
{
	setCoord( pChar->pos() );
	setDirection( pChar->running() ? (pChar->direction() | 0x80) : pChar->direction() );
}

void cUOTxUpdatePlayer::fromChar( P_CHAR pChar )
{
	setSerial( pChar->serial() );
	setX( pChar->pos().x );
	setY( pChar->pos().y );
	setZ( pChar->pos().z );

	if ( pChar->isDead() )
	{
		setBody( pChar->gender() ? 0x193 : 0x192 );
		setHue( 0 );
	}
	else
	{
		setBody( pChar->body() );

		if ( pChar->isHuman() && pChar->skin() > 0 )
		{
			setHue( pChar->skin() | 0x8000 );
		}
		else
		{
			setHue( pChar->skin() );
		}
	}

	// If he's runningSteps we need to take that into account here
	// ->runningSteps() is greater than zero in that case
	setDirection( pChar->running() ? (pChar->direction() | 0x80) : pChar->direction() );

	setFlag( 0 );

	if ( pChar->isAtWar() )
		setFlag( flag() | 0x40 );

	P_PLAYER player = dynamic_cast<P_PLAYER>( pChar );
	/*if ( player && !player->socket() && !player->logoutTime() )
	{
		setFlag( flag() | 0x80 );
	}*/

	if ( pChar->isHidden() || pChar->isInvisible() )
		setFlag( flag() | 0x80 );

	if ( pChar->isDead() && !pChar->isAtWar() )
		setFlag( flag() | 0x80 );

	if ( pChar->poison() >= 0 )
		setFlag( flag() | 0x04 );
}


void cUOTxDrawChar::addEquipment( unsigned int serial, unsigned short model, unsigned char layer, unsigned short color )
{
	// Overwrite the last 4 bytes (terminator) and readd them later
	int offset = count() - 4;
	resize( count() + 9 );
	setShort( 1, count() );

	setInt( offset, serial );
	setShort( offset + 4, model | 0x8000 );
	( *this )[offset + 6] = layer;
	setShort( offset + 7, color );
	setInt( offset + 9, 0 ); // Terminator
}

void cUOTxDrawChar::fromChar( P_CHAR pChar )
{
	setSerial( pChar->serial() );

	if ( pChar->isDead() )
	{
		setModel( pChar->gender() ? 0x193 : 0x192 );
		setColor( 0 );
	}
	else
	{
		setModel( pChar->body() );

		if ( pChar->isHuman() && pChar->skin() > 0 )
		{
			setColor( pChar->skin() | 0x8000 );
		}
		else
		{
			setColor( pChar->skin() );
		}
	}

	setX( pChar->pos().x );
	setY( pChar->pos().y );
	setZ( pChar->pos().z );
	setDirection( pChar->running() ? (pChar->direction() | 0x80) : pChar->direction() );
	setFlag( 0 );

	if ( pChar->isAtWar() && !pChar->isDead() )
		setFlag( 0x40 );

	P_PLAYER player = dynamic_cast<P_PLAYER>( pChar );
	/*if ( player && !player->socket() && !player->logoutTime() )
	{
		setFlag( flag() | 0x80 );
	}*/

	if ( pChar->isHidden() || pChar->isInvisible() )
		setFlag( flag() | 0x80 );

	if ( pChar->isDead() && !pChar->isAtWar() )
		setFlag( flag() | 0x80 );

	if ( pChar->poison() >= 0 )
		setFlag( flag() | 0x04 );

	// Add our equipment - This does not seem to work !?
	bool layers[0x20] =
	{
	0,
	};

	cBaseChar::ItemContainer container( pChar->content() );
	cBaseChar::ItemContainer::const_iterator it( container.begin() );
	cBaseChar::ItemContainer::const_iterator end( container.end() );
	for ( ; it != end; ++it )
	{
		P_ITEM pItem = *it;
		if ( !pItem )
			continue;

		// Only send visible layers
		// 0x19 = horse layer
		// -> Shop containers need to be send
		if ( pItem->layer() > 0x19 && pItem->layer() != 0x1A && pItem->layer() != 0x1B && pItem->layer() != 0x1C )
			continue;

		// Only send items once
		if ( layers[pItem->layer()] )
			continue;

		layers[pItem->layer()] = true;

		addEquipment( pItem->serial(), pItem->id(), pItem->layer(), pItem->color() );
	}
}

void cUOTxCharEquipment::fromItem( P_ITEM pItem )
{
	if ( !pItem->container() )
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

	if ( pChar->isDead() )
	{
		setBody( pChar->gender() ? 0x193 : 0x192 );
	}
	else
	{
		setBody( pChar->body() );

		if ( pChar->isHuman() && pChar->skin() > 0 )
		{
			setSkin( pChar->skin() | 0x8000 );
		}
		else
		{
			setSkin( pChar->skin() );
		}
	}

	setFlag( 0 );

	if ( pChar->isAtWar() )
		setFlag( 0x40 );

	P_PLAYER player = dynamic_cast<P_PLAYER>( pChar );
	/*if ( player && !player->socket() && !player->logoutTime() )
	{
		setFlag( flag() | 0x80 );
	}*/

	if ( pChar->isHidden() || pChar->isInvisible() )
		setFlag( flag() | 0x80 );

	if ( pChar->isDead() && !pChar->isAtWar() )
		setFlag( flag() | 0x80 );

	if ( pChar->poison() >= 0 )
		setFlag( flag() | 0x04 );

	setX( pChar->pos().x );
	setY( pChar->pos().y );
	setZ( pChar->pos().z );
	setDirection( pChar->running() ? (pChar->direction() | 0x80) : pChar->direction() );
	//void setFlags( unsigned char data ) { rawPacket[ 10 ] = data; } // // 10 = 0=normal, 4=poison, 9 = invul,0x40=attack, 0x80=hidden CHARMODE_WAR
}

void cUOTxTipWindow::setMessage( const QCString& m )
{
	unsigned short length = m.length();
	resize( length + 11 );
	setShort( 1, length + 11 );
	setShort( 8, length );
	setAsciiString( 10, m.data(), length + 1 );
}

void cUOTxAddContainerItem::fromItem( P_ITEM pItem )
{
	setSerial( pItem->serial() );
	setModel( pItem->id() );
	setAmount( pItem->amount() );
	setX( pItem->pos().x );
	setY( pItem->pos().y );

	if ( pItem->container() )
		setContainer( pItem->container()->serial() );
	else
		setContainer( INVALID_SERIAL );

	setColor( pItem->color() );
}

void cUOTxOpenPaperdoll::fromChar( P_CHAR pChar, P_CHAR pOrigin )
{
	setSerial( pChar->serial() );

	QString nameByScript = pChar->onShowPaperdollName( pOrigin );

	if ( !nameByScript.isNull() )
	{
		// Replace non displayable characters
		setName( makeAscii(nameByScript) );
	}
	else
	{
		QString title = pChar->title();
		if ( title.isEmpty() )
		{
			title = Skills::instance()->getSkillTitle( pChar );
		}

		QStringList titles = Definitions::instance()->getList( "REPUTATION_TITLES" );

		// Calculate the position inside the list
		unsigned int position;

		if ( pChar->karma() >= 10000 )
		{
			position = 0;
		}
		else if ( pChar->karma() >= 5000 )
		{
			position = 1;
		}
		else if ( pChar->karma() >= 2500 )
		{
			position = 2;
		}
		else if ( pChar->karma() >= 1250 )
		{
			position = 3;
		}
		else if ( pChar->karma() >= 625 )
		{
			position = 4;
		}
		else if ( pChar->karma() <= -625 )
		{
			position = 6;
		}
		else if ( pChar->karma() <= -1250 )
		{
			position = 7;
		}
		else if ( pChar->karma() <= -2500 )
		{
			position = 8;
		}
		else if ( pChar->karma() <= -5000 )
		{
			position = 9;
		}
		else if ( pChar->karma() <= -10000 )
		{
			position = 10;
		}
		else
		{
			position = 5;
		}

		position = ( position * 5 ) + QMIN( 4, pChar->fame() / 2500 );

		if ( pChar->objectType() != enNPC && position < titles.size() )
		{
			QString prefix = titles[position];
			if ( prefix.length() > 0 )
			{
				prefix.prepend( "The " );
				prefix.append( " " );
			}

			// Lord/Lady Title
			if ( pChar->fame() >= 10000 )
			{
				prefix.append( pChar->gender() ? tr( "Lady" ) : tr( "Lord" ) );
				prefix.append( " " );
			}

			setName( makeAscii( prefix + pChar->name() + ( title.isEmpty() ? QString( "" ) : ", " + title ) ) );
		}
		else
		{
			setName( makeAscii( pChar->name() + ( title.isEmpty() ? QString( "" ) : ", " + title ) ) );
		}
	}

	setFlag( 0 );

	if ( pChar->isAtWar() )
		setFlag( 0x40 );

	/*if (pChar->isInvulnerable()) {
		setFlag(flag() | 0x08);
	}*/

	P_PLAYER player = dynamic_cast<P_PLAYER>( pChar );
	/*if ( player && !player->socket() && !player->logoutTime() )
	{
		setFlag( flag() | 0x80 );
	}*/

	if ( pChar->isHidden() || pChar->isInvisible() )
		setFlag( flag() | 0x80 );

	if ( pChar->isDead() && !pChar->isAtWar() )
		setFlag( flag() | 0x80 );

	if ( pChar->poison() >= 0 )
		setFlag( flag() | 0x04 );
}

void cUOTxCorpseEquipment::addItem( unsigned char layer, unsigned int serial )
{
	int offset = count() - 1;
	resize( size() + 5 );
	setShort( 1, size() );

	( *this )[offset] = layer;
	setInt( offset + 1, serial );
	( *this )[offset + 5] = 0;
}

void cUOTxItemContent::addItem( P_ITEM pItem )
{
	if ( !pItem )
		return;
	SERIAL contserial = INVALID_SERIAL;
	if ( pItem->container() )
		contserial = pItem->container()->serial();

	addItem( pItem->serial(), pItem->id(), pItem->color(), pItem->pos().x, pItem->pos().y, pItem->amount(), contserial );
}

void cUOTxItemContent::addItem( SERIAL serial, unsigned short id, unsigned short color, unsigned short x, unsigned short y, unsigned short amount, unsigned int container )
{
	int offset = size();
	resize( size() + 19 );
	setShort( 3, getShort( 3 ) + 1 );
	setShort( 1, size() );

	setInt( offset, serial );
	setShort( offset + 4, id );
	( *this )[offset + 6] = 0;
	setShort( offset + 7, amount );
	setShort( offset + 9, x );
	setShort( offset + 11, y );
	setInt( offset + 13, container );
	setShort( offset + 17, color );
}

void cUOTxVendorBuy::addItem( unsigned int price, const QString& description )
{
	int offset = size();
	resize( size() + 5 + description.length() + 1 ); // Null terminate it for gods-sake
	setShort( 1, size() );

	// Add the item itself
	setInt( offset, price );
	( *this )[offset + 4] = description.length() + 1;
	setAsciiString( offset + 5, description.latin1(), description.length() + 1 );

	++( *this )[7]; // Increase item count
}

void cUOTxGumpDialog::setContent( const QString& layout, const QStringList& text )
{
	//	QString layout = gump->layout().join( "" );
	setShort( 19, layout.length() + 1 );
	setAsciiString( 21, layout.latin1(), layout.length() + 1 );

	// Send the unicode text-lines

	setShort( 22 + layout.length(), text.count() );

	unsigned int offset = 22 + layout.length() + 2;
	QStringList::const_iterator it = text.begin();
	while ( it != text.end() )
	{
		QString line = ( *it );
		setShort( offset, line.length() );
		setUnicodeString( offset + 2, line, line.length() * 2 );
		offset += line.length() * 2 + 2;
		it++;
	}
}

void cUOTxTrade::setName( const QString& name )
{
	( *this )[16] = 1;
	resize( size() + name.length() + 1 );
	setShort( 1, size() );
	setAsciiString( 17, name.latin1(), name.length() + 1 );
	//strcpy( &rawPacket.data()[17], name.latin1() );
}

void cUOTxProfile::setInfo( const QString& title, const QString& staticText, const QString& dynamicText )
{
	// Reset to the correct size
	unsigned short size = 13 + title.length() + ( staticText.length() * 2 ) + ( dynamicText.length() * 2 );
	setShort( 1, size );
	resize( size );

	setAsciiString( 7, title.latin1(), title.length() + 1 );
	( *this )[7 + title.length()] = 0; // Null Terminator

	setUnicodeString( 8 + title.length(), staticText, staticText.length() * 2 );
	setShort( 8 + title.length() + ( staticText.length() * 2 ), 0 );

	setUnicodeString( 10 + title.length() + ( staticText.length() * 2 ), dynamicText, dynamicText.length() * 2 );
	setShort( 10 + title.length() + ( staticText.length() * 2 ) + ( dynamicText.length() * 2 ), 0 );
}

void cUOTxSendItem::setCoord( const Coord_cl& coord )
{
	setShort( 11, coord.x | 0x8000 );
	setShort( 13, coord.y | 0xC000 );
	( *this )[16] = coord.z;
}

void cUOTxSoundEffect::setCoord( const Coord_cl& coord )
{
	setShort( 6, coord.x );
	setShort( 8, coord.y );
	setShort( 10, coord.z );
}

void cUOTxItemTarget::addItem( unsigned short id, short deltaX, short deltaY, short deltaZ, unsigned short hue )
{
	// Add 10 Bytes
	int offset = size();
	resize( offset + 10 );
	setShort( 1, size() );
	setShort( 14, getShort( 14 ) + 1 );
	setShort( offset, id );
	setShort( offset + 2, deltaX );
	setShort( offset + 4, deltaY );
	setShort( offset + 6, deltaZ );
	setShort( offset + 8, hue );
}

void cUOTxTooltipList::addLine( unsigned int id, const QString& params )
{
	// 4 + 2 + length * 2
	unsigned int offset = size() - 4;
	resize( size() + ( 4 + 2 + params.length() * 2 ) );
	setShort( 1, size() );

	// Add our Line
	setInt( offset, id );
	setShort( offset + 4, params.length() * 2 );

	setUnicodeString( offset + 6, params, params.length() * 2, true );

	// Terminator
	setInt( size() - 4, 0 );
}

void cUOTxCustomHouse::addTile( unsigned short id, short x, short y, short z )
{
	unsigned int sz = size() + 5;
	resize( sz );
	// Increase counters
	setShort( 1, sz );					// total len
	setShort( 13, getShort( 13 ) + 1 );	// tiles count
	setShort( 15, sz - 0x11 );			// tiles data len
	setShort( sz - 5, id );
	( *this )[sz - 3] = x;
	( *this )[sz - 2] = y;
	( *this )[sz - 1] = z;
}

void cUOTxSellList::addItem( unsigned int serial, unsigned short id, unsigned short hue, unsigned short amount, unsigned short value, const QString& name )
{
	unsigned int offset = size();
	unsigned int sz = size() + name.length() + 1 + 14; // null terminated ascii string
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

void cUOTxPartyUpdate::addMember( SERIAL serial )
{
	resize( size() + 4 );
	setShort( 1, size() );
	setInt( size() - 4, serial );
	( *this )[6]++;
}

void cUOTxPartyRemoveMember::addMember( SERIAL serial )
{
	resize( size() + 4 );
	setShort( 1, size() );
	setInt( size() - 4, serial );
	( *this )[6]++;
}

void cUOTxPartyTellMember::setText( const QString& message )
{
	resize( 12 + message.length() * 2 );
	setShort( 1, size() );
	setUnicodeString( 10, message, message.length() * 2, false );
	setShort( size() - 2, 0 ); // Null termination
}

void cUOTxAsciiSpeech::setMessage( const QString& data )
{
	resize( 45 + data.length() );
	setShort( 1, 45 + data.length() );
	setAsciiString( 44, data.latin1(), data.length() + 1 );
	( *this )[44 + data.length()] = 0; // Null Termination
}

void cUOTxClilocMsg::setParams( const QString& data )
{
	if ( data.length() > 0 )
	{
		resize( PacketLen + data.length() * 2 );
		QString tmpData = data; // go around the const stuff.
		setUnicodeString( 48, tmpData, data.length() * 2, true );
		setShort( 1, PacketLen + data.length() * 2 );
		setShort( 48 + data.length() * 2, 0 );
	}
}
