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

#include "config.h"
#include "corpse.h"
#include "network/network.h"
#include "network/uotxpackets.h"
#include "dbdriver.h"
#include "network/uosocket.h"
#include "persistentbroker.h"

#include "world.h"
#include "player.h"

#include <functional>
#include <algorithm>
#include <map>

using namespace std;

static cUObject* productCreator()
{
	return new cCorpse;
}

void cCorpse::registerInFactory()
{
	QStringList fields, tables, conditions;
	buildSqlString( fields, tables, conditions ); // Build our SQL string
	QString sqlString = QString( "SELECT %1 FROM uobjectmap,%2 WHERE uobjectmap.type = 'cCorpse' AND %3" ).arg( fields.join( "," ) ).arg( tables.join( "," ) ).arg( conditions.join( " AND " ) );
	UObjectFactory::instance()->registerType( "cCorpse", productCreator );
	UObjectFactory::instance()->registerSqlQuery( "cCorpse", sqlString );
}

void cCorpse::buildSqlString( QStringList& fields, QStringList& tables, QStringList& conditions )
{
	cItem::buildSqlString( fields, tables, conditions );
	fields.push_back( "corpses.bodyid,corpses.hairstyle,corpses.haircolor,corpses.beardstyle,corpses.beardcolor" );
	fields.push_back( "corpses.direction,corpses.charbaseid,corpses.murderer,corpses.murdertime" );
	tables.push_back( "corpses" );
	conditions.push_back( "uobjectmap.serial = corpses.serial" );
}

void cCorpse::load( char** result, UINT16& offset )
{
	cItem::load( result, offset );
	bodyId_ = atoi( result[offset++] );
	hairStyle_ = atoi( result[offset++] );
	hairColor_ = atoi( result[offset++] );
	beardStyle_ = atoi( result[offset++] );
	beardColor_ = atoi( result[offset++] );
	direction_ = atoi( result[offset++] );
	charbaseid_ = result[offset++];
	murderer_ = atoi( result[offset++] );
	murdertime_ = atoi( result[offset++] );

	// Get the corpse equipment
	QString sql = "SELECT serial,layer,item FROM corpses_equipment WHERE serial = '" + QString::number( serial() ) + "'";

	cDBResult res = PersistentBroker::instance()->query( sql );

	if ( !res.isValid() )
		throw PersistentBroker::instance()->lastError();

	// Fetch row-by-row
	while ( res.fetchrow() )
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
	addField( "direction", direction_ );
	addStrField( "charbaseid", charbaseid_ );
	addField( "murderer", murderer_ );
	addField( "murdertime", murdertime_ );

	addCondition( "serial", serial() );
	saveFields;

	// Equipment can change as well
	if ( isPersistent )
	{
		PersistentBroker::instance()->executeQuery( QString( "DELETE FROM corpses_equipment WHERE serial = '%1'" ).arg( serial() ) );
	}

	for ( map<UINT8, SERIAL>::iterator it = equipment_.begin(); it != equipment_.end(); ++it )
		PersistentBroker::instance()->executeQuery( QString( "REPLACE INTO corpses_equipment VALUES(%1,%2,%3)" ).arg( serial() ).arg( it->first ).arg( it->second ) );

	cItem::save();
}

bool cCorpse::del()
{
	if ( !isPersistent )
		return false;

	PersistentBroker::instance()->addToDeleteQueue( "corpses", QString( "serial = '%1'" ).arg( serial() ) );
	PersistentBroker::instance()->addToDeleteQueue( "corpses_equipment", QString( "serial = '%1'" ).arg( serial() ) );

	return cItem::del();
}

// abstract cDefinable
void cCorpse::processNode( const cElement* Tag )
{
	QString TagName = Tag->name();
	QString Value = Tag->value();

	if ( TagName == "bodyid" )
		bodyId_ = Value.toUShort();
	else
		cItem::processNode( Tag );
}

// override update
void cCorpse::update( cUOSocket* mSock )
{
	// Do not send a normal item update here but something else instead
	cItem::ContainerContent content = cItem::content();

	cUOTxCorpseEquipment corpseEquip;
	cUOTxItemContent corpseContent;
	cUOTxSendItem sendItem;

	corpseEquip.setSerial( serial() );

	for ( map<UINT8, SERIAL>::iterator it = equipment_.begin(); it != equipment_.end(); ++it )
	{
		P_ITEM pItem = World::instance()->findItem( it->second );

		if ( pItem && pItem->container() == this )
		{
			corpseEquip.addItem( it->first, it->second );
			corpseContent.addItem( pItem );
		}
	}

	if ( hairStyle_ )
	{
		corpseEquip.addItem( 11, 0x4FFFFFFE ); // Hair
		corpseContent.addItem( 0x4FFFFFFE, hairStyle_, hairColor_, 0, 0, 1, serial() );
	}

	if ( beardStyle_ )
	{
		corpseEquip.addItem( 16, 0x4FFFFFFF ); // Beard
		corpseContent.addItem( 0x4FFFFFFF, beardStyle_, beardColor_, 0, 0, 1, serial() );
	}

	sendItem.setId( id() );
	sendItem.setAmount( bodyId_ );
	sendItem.setSerial( serial() );
	sendItem.setCoord( pos() );
	sendItem.setDirection( direction() );
	if ( bodyId_ >= 0x190 )
	{
		sendItem.setColor( color_ | 0x8000 );
	}
	else
	{
		sendItem.setColor( color_ );
	}

	if ( mSock )
	{
		sendTooltip( mSock );
		mSock->send( &sendItem );
		mSock->send( &corpseEquip );
		mSock->send( &corpseContent );
	}
	else
	{
		for ( mSock = Network::instance()->first(); mSock; mSock = Network::instance()->next() )
		{
			if ( mSock->player() && mSock->player()->inRange( this, mSock->player()->visualRange() ) )
			{
				// Send item
				// Send corpse clothing
				// Send content
				sendTooltip( mSock );
				mSock->send( &sendItem );
				mSock->send( &corpseEquip );
				mSock->send( &corpseContent );
			}
		}
	}
}

SERIAL cCorpse::getEquipment( UINT8 layer )
{
	if ( equipment_.find( layer ) == equipment_.end() )
	{
		return INVALID_SERIAL;
	}
	else
	{
		return equipment_[layer];
	}
}

void cCorpse::addEquipment( UINT8 layer, SERIAL serial )
{
	if ( equipment_.find( layer ) != equipment_.end() )
	{
		equipment_[layer] = serial;
		return;
	}

	equipment_.insert( make_pair( layer, serial ) );
}

cCorpse::cCorpse( bool init )
{
	if ( init )
		cItem::Init( true );

	bodyId_ = 0x190;
	hairStyle_ = 0;
	hairColor_ = 0;
	beardStyle_ = 0;
	beardColor_ = 0;
	murderer_ = INVALID_SERIAL;
	murdertime_ = 0;
	direction_ = 0;
	id_ = 0x2006;
	setBaseid("2006");
}

stError* cCorpse::setProperty( const QString& name, const cVariant& value )
{
	/*
		\property corpse.bodyid The body if of the dead creature.
	*/
	SET_INT_PROPERTY( "bodyid", bodyId_ )
	else
	/*
		\property corpse.hairstyle The id of the hairstyle displayed on the corpse.
		For no hair use 0.
	*/
		SET_INT_PROPERTY( "hairstyle", hairStyle_ )
	else
	/*
		\property corpse.haircolor The color of the hair displayed on the corpse.
	*/
		SET_INT_PROPERTY( "haircolor", hairColor_ )
	else
	/*
		\property corpse.beardstyle The id of the beardstyle displayed on the corpse.
		For no beard use 0.
	*/
		SET_INT_PROPERTY( "beardstyle", beardStyle_ )
	else
	/*
		\property corpse.beardcolor The color of the beard displayed on the corpse.
	*/
		SET_INT_PROPERTY( "beardcolor", beardColor_ )

	/*
		\property corpse.murderer The character who killed this creature. May be None if the
		character has been deleted or the owner of this corpse accidently died.
	*/
	else if (name == "murderer") 
	{
		P_CHAR pChar = value.toChar();
		if (pChar) {
			murderer_ = pChar->serial();
		} else {
			murderer_ = INVALID_SERIAL;
		}
	}		
	/*
		\property corpse.murdertime The time when the murder was comitted in seconds 
		since the epoch (UNIX timestamp).
	*/
	else
		SET_INT_PROPERTY( "murdertime", murdertime_ )

	/*
		\property corpse.direction The direction this corpse is facing.
	*/
	else
		SET_INT_PROPERTY( "direction", direction_ )

	/*
		\property corpse.charbaseid The npc definition id of the murdererd creature.
		This is used to derive the carve section for this corpse.
	*/
	else
		SET_STR_PROPERTY( "charbaseid", charbaseid_ )

	return cItem::setProperty( name, value );
}

stError* cCorpse::getProperty( const QString& name, cVariant& value )
{
	GET_PROPERTY( "bodyid", bodyId_ )
	GET_PROPERTY( "hairstyle", hairStyle_ )
	GET_PROPERTY( "haircolor", hairColor_ )
	GET_PROPERTY( "beardstyle", beardStyle_ )
	GET_PROPERTY( "beardcolor", beardColor_ )
	GET_PROPERTY( "murderer", FindCharBySerial(murderer_) )
	GET_PROPERTY( "murdertime", (int) murdertime_ )
	GET_PROPERTY( "direction", (int) direction_ )
	GET_PROPERTY( "charbaseid", charbaseid_ )

	return cItem::getProperty( name, value );
}

void cCorpse::createTooltip( cUOTxTooltipList& tooltip, cPlayer* player )
{
	cUObject::createTooltip( tooltip, player );

	unsigned int notoriety = 0;

	if ( tags_.has( "notoriety" ) )
	{
		notoriety = tags_.get( "notoriety" ).toInt();
	}

	QString color;

	switch ( notoriety )
	{
	case 1:
		color = "#00FFFF";
		break;

	case 2:
		color = "#10d010";
		break;

	case 3:
	case 4:
		color = "#d0d0d0";
		break;

	case 5:
		color = "#ff9900";
		break;

	case 6:
		color = "#d01010";
		break;

	default:
		color = QString::null;
		break;
	};

	if ( color.isEmpty() )
	{
		tooltip.addLine( 1050045, QString( " \tThe Remains Of %1\t " ).arg( name_ ) );
	}
	else
	{
		tooltip.addLine( 1050045, QString( "<basefont color=\"%1\"> \tThe Remains Of %2\t " ).arg( color ).arg( name_ ) );
	}
}

unsigned int cCorpse::decayDelay() {
	unsigned int delay = cItem::decayDelay();

	if (delay == 0) {
		return 0;
	}

	return Config::instance()->corpseDecayTime() * MY_CLOCKS_PER_SEC;
}
