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

#include "uotxpackets.h"
#include "uopacket.h"

void cUOTxShardList::addServer( Q_UINT16 serverIndex, QString serverName, Q_UINT8 serverFull, Q_INT8 serverTimeZone, Q_UINT32 serverIp )
{
	// Increase the server-count
	// Offset: 4
	setShort( 4, getShort( 4 ) + 1 );

	Q_INT32 offset = rawPacket.count();
	rawPacket.resize( rawPacket.count() + 40 ); // 40 byte per server
	setShort( 1, rawPacket.count() );
	setShort( offset, serverIndex );

	if( serverName.length() > 31 ) 
		serverName = serverName.left( 31 );

	strcpy( &rawPacket.data()[ offset + 2 ], serverName.latin1() );

	rawPacket[ offset + 34 ] = serverFull;
	rawPacket[ offset + 35 ] = serverTimeZone;
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
	rawPacket.resize( 309 + ( towns.size() * 63 ) );
	rawPacket[ 0 ] = (Q_UINT8)0xA9;

	rawPacket[ 3 ] = characters.size(); // Char Count

	for( Q_UINT8 c = 0; c < 5; ++c )
		if( c < characters.size() )
		{
			strcpy( &rawPacket.data()[ 4 + ( c * 60 ) ], characters[ c ].latin1() );
			rawPacket[ 4 + ( c * 60 ) + 30 ] = 0x00; // No Password (!)
		}
		else
			rawPacket[ 4 + ( c * 60 ) ] = 0x00; // "Pad-out" the char

	// Town Count
	Q_INT32 offset = 304;
	rawPacket[ offset++ ] = towns.size();

	for( Q_UINT8 t = 0; t < towns.size(); ++t )
	{
		rawPacket[ offset ] = towns[ t ].index;
		strcpy( &rawPacket.data()[ offset + 1 ], towns[ t ].town.latin1() );
		strcpy( &rawPacket.data()[ offset + 32 ], towns[ t ].area.latin1() );
		offset += 63;
	}

	if( charLimit >= 0 )
		setInt( offset, ( charLimit << 4 ) | flags | 0x4 );
	else
		setInt( offset, flags );

	// New Packet Size
	setShort( 1, rawPacket.count() );
}

void cUOTxUpdateCharList::setCharacter( Q_UINT8 index, QString name )
{
	Q_INT32 offset = 4 + ( index * 60 );
	rawPacket[ 3 ]++;

	if( name.length() > 29 )
		name = name.left( 29 );

	strcpy( &rawPacket.data()[offset], name.latin1() );
}

void cUOTxSendSkills::addSkill( Q_UINT16 skillId, Q_UINT16 skill, Q_UINT16 realSkill, eStatus status )
{
	// Overwrite the last 2 bytes (terminator) and readd them later
	Q_INT32 offset = rawPacket.count() - 2;
	rawPacket.resize( rawPacket.count() + 7 );
	setShort( 1, rawPacket.count() );

	setShort( offset, skillId );
	setShort( offset+2, skill );
	setShort( offset+4, realSkill );
	rawPacket[ offset+6 ] = status;
	setShort( offset+7, 0 ); // Terminator
}

void cUOTxDrawChar::addEquipment( Q_UINT32 serial, Q_UINT16 model, Q_UINT8 layer, Q_UINT16 color )
{
	// Overwrite the last 4 bytes (terminator) and readd them later
	Q_INT32 offset = rawPacket.count() - 4;
	rawPacket.resize( rawPacket.count() + 9 );
	setShort( 1, rawPacket.count() );

	setInt( offset, serial );
	setShort( offset+4, model|0x8000 );
	rawPacket[offset+6] = layer;
	setShort( offset+7, color );
	setInt( offset+9, 0 ); // Terminator
}

void cUOTxUnicodeSpeech::setText( const QString &data )
{
	rawPacket.resize( 50 + (data.length()*2) );
	setShort( 1, rawPacket.count() );

	Q_INT32 offset = 48; // Pad right by one - remeber to copy one byte less
	rawPacket[ offset ] = 0x00;
	memcpy( &rawPacket.data()[ offset + 1 ], data.unicode(), (data.length()*2)-1 );

	// Add the new Terminator
	setShort( rawPacket.count() - 2, 0 );
}

// Sets all data automatically
void cUOTxConfirmLogin::fromChar( P_CHAR pChar )
{
	setSerial( pChar->serial );
	setBody( pChar->id() );
	setDirection( pChar->dir );
	setX( pChar->pos.x );
	setY( pChar->pos.y );
	setZ( pChar->pos.z );
}

void cUOTxSendSkills::fromChar( P_CHAR pChar )
{
	if( !pChar )
		return;

	for( UINT8 i = 0; i < ALLSKILLS; ++i )
		addSkill( i+1, pChar->skill( i ), pChar->baseSkill( i ), cUOTxSendSkills::Up );
}

void cUOTxContextMenu::addEntry( Q_UINT16 textId, Q_UINT16 returnVal )
{
	Q_UINT32 offset = rawPacket.count();
	
	if( rawPacket.count() > 8 )
	{
		// Not the first entry anymore
		rawPacket.resize( offset + 6 );
		setShort( offset, returnVal );
		setShort( offset+2, textId );
		setShort( offset+4, 0x0000 );
	}
	else
	{
		// First entry
		rawPacket.resize( offset + 8 );
		setShort( offset, returnVal );
		setShort( offset+2, textId );
		setShort( offset+4, 0x0020 );
		setShort( offset+6, 0xFFFF );
	}
	
	// Increase the item-count + the menu-length
    rawPacket[ 7 ]++;
	setShort( 1, rawPacket.count() );
}

void cUOTxDenyMove::setCoord( Coord_cl coord )
{
		setShort( 2, coord.x );
		setShort( 4, coord.y );
		(*this)[7] = coord.z;
}

void cUOTxDenyMove::fromChar( P_CHAR pChar )
{
	setCoord( pChar->pos );
	setDirection( pChar->dir );
}

void cUOTxUpdatePlayer::fromChar( P_CHAR pChar )
{
	setSerial( pChar->serial );
	setX( pChar->pos.x );
	setY( pChar->pos.y );
	setZ( pChar->pos.z );
	setHue( pChar->skin() );
	setBody( pChar->id() );
	
	// If he's running we need to take that into account here
	// ->running() is greater than zero in that case
	setDirection( pChar->running() ? pChar->dir|0x80 : pChar->dir );

	if( pChar->war )
		setFlag( 0x40 );

	if( pChar->hidden() )
		setFlag( flag() | 0x80 );

	if( pChar->dead && !pChar->war )
		setFlag( flag() | 0x80 );

	if( pChar->poisoned() )
		setFlag( flag() | 0x04 );
	// Set Flag and Highlight color
	/*
	//if (pc->npcaitype==0x02) extmove[16]=6; else extmove[16]=1;
			int guild;
			//chars[i].flag=0x04;       // everyone should be blue on default
			guild = GuildCompare( pc, us );
			if( us->kills > SrvParams->maxkills() ) extmove[16]=6;
			else if (guild==1)//Same guild (Green)
				extmove[16]=2;
			else if (guild==2) // Enemy guild.. set to orange
				extmove[16]=5;
			else
			{
				switch(us->flag())
				{//1=blue 2=green 5=orange 6=Red 7=Transparent(Like skin 66 77a)
				case 0x01: extmove[16]=6; break;// If a bad, show as red.
				case 0x04: extmove[16]=1; break;// If a good, show as blue.
				case 0x08: extmove[16]=2; break; //green (guilds)
				case 0x10: extmove[16]=5; break;//orange (guilds)
				default:extmove[16]=3; break;//grey
				}
		}*/
}

void cUOTxDrawChar::fromChar( P_CHAR pChar )
{
	setSerial( pChar->serial );
	setModel( pChar->id() );
	setX( pChar->pos.x );
	setY( pChar->pos.y );
	setZ( pChar->pos.z );
	setDirection( pChar->dir );
	setColor( pChar->skin() );
	setFlags( 0 ); // NEED TO SET FLAGS
	setHightlight( 0 ); // NEED TO SET HIGHLIGHT

	// Add our equipment - This does not seem to work !?
	vector< SERIAL > equipment = contsp.getData( pChar->serial );
	bool layers[0x20] = {0,};

	for( Q_UINT32 i = 0; i < equipment.size(); ++i )
	{
		P_ITEM pItem = FindItemBySerial( equipment[i] );
		if( !pItem )
			continue;

		// Only send visible layers
		// 0x19 = horse layer
		if( pItem->layer() > 0x19 )
			continue;

		// Only send items once
		if( layers[ pItem->layer() ] )
			continue;

		layers[ pItem->layer() ] = true;

		addEquipment( pItem->serial, pItem->id(), pItem->layer(), pItem->color() );
	}
}

void cUOTxCharEquipment::fromItem( P_ITEM pItem )
{
	setSerial( pItem->serial );
	setModel( pItem->id() );
	setLayer( pItem->layer() );
	setWearer( pItem->contserial );
	setColor( pItem->color() );
}

// Sends an update of ourself
void cUOTxDrawPlayer::fromChar( P_CHAR pChar )
{
	setSerial( pChar->serial );
	setBody( pChar->id() );
	setSkin( pChar->skin() );
	setFlags( 0x00 );
	setX( pChar->pos.x );
	setY( pChar->pos.y );
	setZ( pChar->pos.z );
	setDirection( pChar->dir );
	//void setFlags( Q_UINT8 data ) { rawPacket[ 10 ] = data; } // // 10 = 0=normal, 4=poison, 0x40=attack, 0x80=hidden CHARMODE_WAR
}

void cUOTxTipWindow::setMessage( QString m )
{
	ushort length = m.length();
	rawPacket.resize( length + 10 );
	setShort(1, length + 10 );
	setShort(8, length );
	setAsciiString(10, m.latin1(), length);
}

void cUOTxAddContainerItem::fromItem( P_ITEM pItem )
{
	setSerial( pItem->serial );
	setModel( pItem->id() );
	setAmount( pItem->amount() );
	setX( pItem->pos.x );
	setY( pItem->pos.y );
	setContainer( pItem->contserial );
	setColor( pItem->color() );
}

void cUOTxPopupMenu::addEntry( UINT16 entryId, UINT16 stringId, bool flagged )
{
	rawPacket[ 11 ]++;

	UINT32 offset = rawPacket.size();
	rawPacket.resize( rawPacket.size() + ( flagged ? 8 : 6 ) );

	setShort( offset, entryId );
	setShort( offset+2, stringId );
	setShort( offset+4, flagged ? 0x20 : 0x00 );
	if( flagged )
		setShort( offset+6, 0xFFFF );

	setShort( 1, rawPacket.count() );
}

void cUOTxOpenPaperdoll::fromChar( P_CHAR pChar )
{
	setSerial( pChar->serial );
	setName( pChar->name.c_str() );

	if( pChar->war )
		setFlag( 0x40 );

	if( pChar->hidden() )
		setFlag( flag() | 0x80 );

	if( pChar->dead && !pChar->war )
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
