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
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================

#include "corpse.h"
#include "network.h"
#include "network/uotxpackets.h"
#include "dbdriver.h"
#include "network/uosocket.h"
#include "persistentbroker.h"
#include "globals.h"
#include "prototypes.h"
#include "world.h"

#include <functional>
#include <algorithm>
#include <map>

#include <qdom.h>

using namespace std;

static cUObject* productCreator()
{
	return new cCorpse;
}

void cCorpse::registerInFactory()
{
	QStringList fields, tables, conditions;
	buildSqlString( fields, tables, conditions ); // Build our SQL string
	QString sqlString = QString( "SELECT uobjectmap.serial,uobjectmap.type,%1 FROM uobjectmap,%2 WHERE uobjectmap.type = 'cCorpse' AND %3" ).arg( fields.join( "," ) ).arg( tables.join( "," ) ).arg( conditions.join( " AND " ) );
	UObjectFactory::instance()->registerType("cCorpse", productCreator);
	UObjectFactory::instance()->registerSqlQuery( "cCorpse", sqlString );
}

void cCorpse::buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions )
{
	cItem::buildSqlString( fields, tables, conditions );
	fields.push_back( "corpses.bodyid,corpses.hairstyle,corpses.haircolor,corpses.beardstyle,corpses.beardcolor" );
	tables.push_back( "corpses" );
	conditions.push_back( "uobjectmap.serial = corpses.serial" );
}

void cCorpse::load( char **result, UINT16 &offset )
{
	cItem::load( result, offset );
	bodyId_ = atoi( result[offset++] );
	hairStyle_ = atoi( result[offset++] );
	hairColor_ = atoi( result[offset++] );
	beardStyle_ = atoi( result[offset++] );
	beardColor_ = atoi( result[offset++] );

	// Get the corpse equipment
	QString sql = "SELECT corpses_equipment.layer,corpses_equipment.item FROM corpses_equipment WHERE serial = '" + QString::number( serial() ) + "'";
	
	cDBResult res = persistentBroker->query( sql );

	if( !res.isValid() )
		throw persistentBroker->lastError();

	// Fetch row-by-row
	while( res.fetchrow() )
		equipment_.insert( make_pair( res.getInt( 0 ), res.getInt( 1 ) ) );

	res.free();
}

void cCorpse::save()
{
	initSave;
	setTable( "corpses" );
	
	addField( "serial", serial() );
	addField( "bodyid", bodyId_ );
	addField( "hairstyle", hairStyle_ );
	addField( "haircolor", hairColor_ );
	addField( "beardstyle", beardStyle_ );
	addField( "beardcolor", beardColor_ );

	addCondition( "serial", serial() );
	saveFields;

	// Equipment can change as well
	if( isPersistent )
		persistentBroker->executeQuery( QString( "DELETE FROM corpses_equipment WHERE serial = '%1'" ).arg( serial() ) );

	for( map< UINT8, SERIAL >::iterator it = equipment_.begin(); it != equipment_.end(); ++it )
		persistentBroker->executeQuery( QString( "INSERT INTO corpses_equipment SET serial = '%1', layer = '%2', item = '%3'" ).arg( serial() ).arg( it->first ).arg( it->second ) );

	cItem::save();
}

bool cCorpse::del()
{
	if( !isPersistent )
		return false;

	persistentBroker->addToDeleteQueue( "corpses", QString( "serial = '%1'" ).arg( serial() ) );
	persistentBroker->addToDeleteQueue( "corpses_equipment", QString( "serial = '%1'" ).arg( serial() ) );

	return cItem::del();
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
	cItem::ContainerContent content = cItem::content();

	cUOTxCorpseEquipment corpseEquip;
	cUOTxItemContent corpseContent;
	cUOTxSendItem sendItem;

	corpseEquip.setSerial( serial() );
	
	for( map< UINT8, SERIAL >::iterator it = equipment_.begin(); it != equipment_.end(); ++it )
	{
		P_ITEM pItem = World::instance()->findItem( it->second );

		if( pItem && pItem->container() == this )
		{
			corpseEquip.addItem( it->first, it->second );
			corpseContent.addItem( pItem );
		}
	}

	if( hairStyle_ )
	{
		corpseEquip.addItem( 11, 0x4FFFFFFE ); // Hair
		corpseContent.addItem( 0x4FFFFFFE, hairStyle_, hairColor_, 0, 0, 1, serial() );
	}

	if( beardStyle_ )
	{
		corpseEquip.addItem( 16, 0x4FFFFFFF ); // Beard
		corpseContent.addItem( 0x4FFFFFFF, beardStyle_, beardColor_, 0, 0, 1, serial() );
	}

	sendItem.setId( id() );
	sendItem.setAmount( bodyId_ );
	sendItem.setSerial( serial() );
	sendItem.setCoord( pos() );
	sendItem.setDirection( direction() );
	sendItem.setColor( color() );

	if( mSock )
	{
		sendTooltip( mSock );
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

stError *cCorpse::setProperty( const QString &name, const cVariant &value )
{
	SET_INT_PROPERTY( "bodyid", bodyId_ )
	else SET_INT_PROPERTY( "hairstyle", hairStyle_ )
	else SET_INT_PROPERTY( "haircolor", hairColor_ )
	else SET_INT_PROPERTY( "beardstyle", beardStyle_ )
	else SET_INT_PROPERTY( "beardcolor", beardColor_ )

	return cItem::setProperty( name, value );
}

stError *cCorpse::getProperty( const QString &name, cVariant &value ) const
{
	GET_PROPERTY( "bodyid", bodyId_ )
	GET_PROPERTY( "hairstyle", hairStyle_ )
	GET_PROPERTY( "haircolor", hairColor_ )
	GET_PROPERTY( "beardstyle", beardStyle_ )
	GET_PROPERTY( "beardcolor", beardColor_ )

	return cItem::getProperty( name, value );
}
