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
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#include "serverconfig.h"
#include "corpse.h"
#include "network/network.h"
#include "network/uotxpackets.h"
#include "basics.h"
#include "dbdriver.h"
#include "network/uosocket.h"
#include "persistentbroker.h"

#include "world.h"
#include "player.h"

#include <functional>
#include <algorithm>

using namespace std;

static FactoryRegistration<cCorpse> registration( "cCorpse" );

unsigned char cCorpse::classid;

void cCorpse::buildSqlString( const char* objectid, QStringList& fields, QStringList& tables, QStringList& conditions )
{
	cItem::buildSqlString( objectid, fields, tables, conditions );
	fields.push_back( "corpses.bodyid,corpses.hairstyle,corpses.haircolor,corpses.beardstyle,corpses.beardcolor" );
	fields.push_back( "corpses.direction,corpses.charbaseid,corpses.murderer,corpses.murdertime" );
	tables.push_back( "corpses" );
	conditions.push_back( "uobjectmap.serial = corpses.serial" );
}

void cCorpse::load( cBufferedReader& reader, unsigned int version )
{
	cItem::load( reader, version );
	bodyId_ = reader.readShort();
	hairStyle_ = reader.readShort();
	hairColor_ = reader.readShort();
	beardStyle_ = reader.readShort();
	beardColor_ = reader.readShort();
	direction_ = reader.readByte();
	charbaseid_ = reader.readAscii();
	murderer_ = reader.readInt();
	murdertime_ = reader.readInt();

	// Write a serial for every possible layer (fixed block size)
	unsigned char layer;
	for ( layer = cBaseChar::SingleHandedWeapon; layer <= cBaseChar::Mount; ++layer )
	{
		SERIAL serial = reader.readInt();
		if ( serial != INVALID_SERIAL )
		{
			equipment_.insert( layer, serial );
		}
	}
}

void cCorpse::save( cBufferedWriter& writer, unsigned int version )
{
	cItem::save( writer, version );
	writer.writeShort( bodyId_ );
	writer.writeShort( hairStyle_ );
	writer.writeShort( hairColor_ );
	writer.writeShort( beardStyle_ );
	writer.writeShort( beardColor_ );
	writer.writeByte( direction_ );
	writer.writeAscii( charbaseid_ );
	writer.writeInt( murderer_ );
	writer.writeInt( murdertime_ );

	// Write a serial for every possible layer (fixed block size)
	unsigned char layer;
	for ( layer = cBaseChar::SingleHandedWeapon; layer <= cBaseChar::Mount; ++layer )
	{
		if ( equipment_.contains( layer ) )
		{
			writer.writeInt( equipment_[layer] );
		}
		else
		{
			writer.writeInt( INVALID_SERIAL );
		}
	}
}

void cCorpse::load( char** result, Q_UINT16& offset )
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
		equipment_.insert( res.getInt( 0 ), res.getInt( 1 ) );

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

	for ( QMap<Q_UINT8, SERIAL>::iterator it = equipment_.begin(); it != equipment_.end(); ++it )
		PersistentBroker::instance()->executeQuery( QString( "REPLACE INTO corpses_equipment VALUES(%1,%2,%3)" ).arg( serial() ).arg( it.key() ).arg( it.data() ) );

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
	cUOTxCorpseEquipment corpseEquip;
	cUOTxItemContent corpseContent;
	cUOTxSendItem sendItem;

	corpseEquip.setSerial( serial() );

	for ( QMap<Q_UINT8, SERIAL>::iterator it = equipment_.begin(); it != equipment_.end(); ++it )
	{
		P_ITEM pItem = World::instance()->findItem( it.data() );

		if ( pItem && pItem->container() == this )
		{
			corpseEquip.addItem( it.key(), it.data() );
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

SERIAL cCorpse::getEquipment( Q_UINT8 layer )
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

void cCorpse::addEquipment( Q_UINT8 layer, SERIAL serial )
{
	if ( equipment_.find( layer ) != equipment_.end() )
	{
		equipment_[layer] = serial;
		return;
	}

	equipment_.insert( layer, serial );
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
	setBaseid( "2006" );
}

stError* cCorpse::setProperty( const QString& name, const cVariant& value )
{
	/*
		\property item.bodyid The body if of the dead creature.
		This property only exists for corpses.
	*/
	SET_INT_PROPERTY( "bodyid", bodyId_ )
	else
			/*
				\property item.hairstyle The id of the hairstyle displayed on the corpse.
				For no hair use 0.
				This property only exists for corpses.
			*/
		SET_INT_PROPERTY( "hairstyle", hairStyle_ )
	else
			/*
				\property item.haircolor The color of the hair displayed on the corpse.
				This property only exists for corpses.
			*/
		SET_INT_PROPERTY( "haircolor", hairColor_ )
	else
			/*
				\property item.beardstyle The id of the beardstyle displayed on the corpse.
				For no beard use 0.
				This property only exists for corpses.
			*/
		SET_INT_PROPERTY( "beardstyle", beardStyle_ )
	else
			/*
				\property item.beardcolor The color of the beard displayed on the corpse.
				This property only exists for corpses.
			*/
		SET_INT_PROPERTY( "beardcolor", beardColor_ )

		/*
		\property item.murderer The character who killed this creature. May be None if the
		character has been deleted or the owner of this corpse accidently died.
		This property only exists for corpses.
		*/
	else if ( name == "murderer" )
	{
		P_CHAR pChar = value.toChar();
		if ( pChar )
		{
			murderer_ = pChar->serial();
		}
		else
		{
			murderer_ = INVALID_SERIAL;
		}
	}
	/*
		\property item.murdertime The time when the murder was comitted in seconds
		since the epoch (UNIX timestamp).
		This property only exists for corpses.
	*/
	else
		SET_INT_PROPERTY( "murdertime", murdertime_ )

		/*
		\property item.direction The direction this corpse is facing.
		This property only exists for corpses.
		*/
	else
		SET_INT_PROPERTY( "direction", direction_ )

		/*
		\property item.charbaseid The npc definition id of the murdererd creature.
		This is used to derive the carve section for this corpse.
		This property only exists for corpses.
		*/
	else
		SET_STR_PROPERTY( "charbaseid", charbaseid_ )

	return cItem::setProperty( name, value );
}

PyObject* cCorpse::getProperty( const QString& name )
{
	PY_PROPERTY( "bodyid", bodyId_ )
	PY_PROPERTY( "hairstyle", hairStyle_ )
	PY_PROPERTY( "haircolor", hairColor_ )
	PY_PROPERTY( "beardstyle", beardStyle_ )
	PY_PROPERTY( "beardcolor", beardColor_ )
	PY_PROPERTY( "murderer", FindCharBySerial( murderer_ ) )
	PY_PROPERTY( "murdertime", murdertime_ )
	PY_PROPERTY( "direction", direction_ )
	PY_PROPERTY( "charbaseid", charbaseid_ )
	return cItem::getProperty( name );
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

unsigned int cCorpse::decayDelay()
{
	unsigned int delay = cItem::decayDelay();

	if ( delay == 0 )
	{
		return 0;
	}

	return Config::instance()->corpseDecayTime() * MY_CLOCKS_PER_SEC;
}
