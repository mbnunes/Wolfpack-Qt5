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
//	Wolfpack Homepage: http://www.wpdev.sf.net/
//========================================================================================

#include "corpse.h"
#include "network.h"
#include "network/uotxpackets.h"
#include "network/uosocket.h"

// abstract cSerializable
void cCorpse::Serialize( ISerialization &archive )
{
	if( archive.isReading() )
	{
		archive.read( "bodyid", bodyId_ );
		archive.read( "hairstyle", hairStyle_ );
		archive.read( "haircolor", hairColor_ );
		archive.read( "beardstyle", beardStyle_ );
		archive.read( "beardcolor", beardColor_ );

		unsigned int equipCount = 0;
		archive.read( "equipcount", equipCount );

		for( UINT8 i = 0; i < equipCount; ++i )
		{
			UINT8 eLayer;
			UINT32 eSerial;
			archive.read( (char*)QString( "equiplayer.%1" ).arg( i ).latin1(), eLayer );
			archive.read( (char*)QString( "equipitem.%1" ).arg( i ).latin1(), eSerial );
			equipment_.insert( make_pair( eLayer, eSerial ) );
		}
	}
	else if( archive.isWritting() )
	{
		archive.write( "bodyid", bodyId_ );
		archive.write( "hairstyle", hairStyle_ );
		archive.write( "haircolor", hairColor_ );
		archive.write( "beardstyle", beardStyle_ );
		archive.write( "beardcolor", beardColor_ );
		archive.write( "equipcount", equipment_.size() );

		UINT8 i = 0;
		for( map< UINT8, SERIAL >::iterator it = equipment_.begin(); it != equipment_.end(); ++it )
		{
			archive.write( (char*)QString( "equiplayer.%1" ).arg( i ).latin1(), it->first );
			archive.write( (char*)QString( "equipitem.%1" ).arg( i ).latin1(), it->second );
			++i;
		}
	}

	cItem::Serialize( archive );
}

// abstract cDefinable
void cCorpse::processNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	QString Value = this->getNodeValue( Tag );

	if( TagName == "bodyid" )
		bodyId_ = Value.toUShort();

	else
		cItem::processNode( Tag );
}

// override update
void cCorpse::update( cUOSocket *mSock )
{
	// Do not send a normal item update here but something else instead
	vector< SERIAL > content = contsp.getData( serial );

	cUOTxCorpseEquipment corpseEquip;
	cUOTxItemContent corpseContent;
	cUOTxSendItem sendItem;

	corpseEquip.setSerial( serial );
	
	for( map< UINT8, SERIAL >::iterator it = equipment_.begin(); it != equipment_.end(); ++it )
	{
		// Only add it to the equipment if it's still in there
		if( find( content.begin(), content.end(), it->second ) != content.end() )
		{
			P_ITEM pItem = FindItemBySerial( it->second );

			if( pItem )
			{
				corpseEquip.addItem( it->first, it->second );
				corpseContent.addItem( pItem );
			}			
		}
	}

	if( hairStyle_ )
	{
		corpseEquip.addItem( 11, 0x4FFFFFFE ); // Hair
		corpseContent.addItem( 0x4FFFFFFE, hairStyle_, hairColor_, 0, 0, 1, serial );
	}

	if( beardStyle_ )
	{
		corpseEquip.addItem( 16, 0x4FFFFFFF ); // Beard
		corpseContent.addItem( 0x4FFFFFFF, beardStyle_, beardColor_, 0, 0, 1, serial );
	}

	sendItem.setId( id() );
	sendItem.setAmount( bodyId_ );
	sendItem.setSerial( serial );
	sendItem.setCoord( pos );
	sendItem.setDirection( dir );
	sendItem.setColor( color() );

	if( mSock )
	{
		mSock->send( &sendItem );
		mSock->send( &corpseEquip );
		mSock->send( &corpseContent );
	}
	else
	{
		for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
		{
			if( mSock->player() && mSock->player()->inRange( this, mSock->player()->VisRange() ) )
			{
				// Send item
				// Send corpse clothing
				// Send content
				mSock->send( &sendItem );
				mSock->send( &corpseEquip );
				mSock->send( &corpseContent );
			}
		}
	}
}

void cCorpse::addEquipment( UINT8 layer, SERIAL serial )
{
	if( equipment_.find( layer ) != equipment_.end() )
		return;

	equipment_.insert( make_pair( layer, serial ) );
}

cCorpse::cCorpse( bool init )
{
	if( init )
		cItem::Init( true );

	bodyId_ = 0x190;
	hairStyle_ = 0;
	hairColor_ = 0;
	beardStyle_ = 0;
	beardColor_ = 0;
	id_ = 0x2006;
	setCorpse( 1 );
	setType( 1 );
}
