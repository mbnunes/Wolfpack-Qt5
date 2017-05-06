/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2017 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: https://github.com/Mutilador/Wolfpack
 */

#include "serverconfig.h"
#include "corpse.h"
#include "network/network.h"
#include "network/uotxpackets.h"
#include "basics.h"
#include "dbdriver.h"
#include "network/uosocket.h"
#include "persistentbroker.h"
#include "console.h"
#include "world.h"
#include "player.h"

#include <functional>
#include <algorithm>

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

using namespace std;

static FactoryRegistration<cCorpse> registration( "cCorpse" );

unsigned char cCorpse::classid;

cCorpse::cCorpse( bool init )
{
	if ( init )
		cItem::Init( true );

	bodyId_ = 0x190;
	murderer_ = INVALID_SERIAL;
	murdertime_ = 0;
	direction_ = 0;
	id_ = 0x2006;
	setBaseid( "2006" );
}

// static definitions
QSqlQuery * cCorpse::insertQuery_ = NULL;
QSqlQuery * cCorpse::updateQuery_ = NULL;
QSqlQuery * cCorpse::insertEquipmentQuery_ = NULL;
QSqlQuery * cCorpse::deleteEquipmentQuery_ = NULL;

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
	/*hairStyle_ =*/ reader.readShort();
	/*hairColor_ =*/ reader.readShort();
	/*beardStyle_ =*/ reader.readShort();
	/*beardColor_ =*/ reader.readShort();
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
	if ( free )
	{
		Console::instance()->log( LOG_WARNING, tr( "Skipping corpse 0x%1 during save process because it's already freed.\n" ).arg( serial_, 0, 16 ) );
	}
	else if ( container_ && container_->free )
	{
		Console::instance()->log( LOG_WARNING, tr( "Skipping corpse 0x%1 during save process because it's in a freed container.\n" ).arg( serial_, 0, 16 ) );
	}
	else
	{
		cItem::save( writer, version );
		writer.writeShort( bodyId_ );
		writer.writeShort( 0 /*hairStyle_*/ );
		writer.writeShort( 0 /*hairColor_*/ );
		writer.writeShort( 0 /*beardStyle_*/ );
		writer.writeShort( 0 /*beardColor_*/ );
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
}

void cCorpse::load( QSqlQuery& result, ushort& offset )
{
	cItem::load( result, offset );
	bodyId_ = result.value( offset++ ).toInt();
	/*hairStyle_ =*/ result.value( offset++ ).toInt();
	/*hairColor_ =*/ result.value( offset++ ).toInt();
	/*beardStyle_ =*/ result.value( offset++ ).toInt();
	/*beardColor_ =*/ result.value( offset++ ).toInt();
	direction_ = result.value( offset++ ).toInt();
	charbaseid_ = result.value( offset++ ).toByteArray();
	murderer_ = result.value( offset++ ).toInt();
	murdertime_ = result.value( offset++ ).toInt();

	// Get the corpse equipment
	QSqlQuery query( QString( "SELECT serial,layer,item FROM corpses_equipment WHERE serial = %1" ).arg( serial() ) );

	if ( !query.isActive() )
		throw wpException( query.lastError().text() );

	// Fetch row-by-row
	while ( query.next() )
		equipment_.insert( query.value( 1 ).toInt(), query.value( 2 ).toInt() );
}

void cCorpse::save()
{
	QSqlQuery * q;

	if ( changed_ )
	{
		if ( isPersistent )
			q = cCorpse::getUpdateQuery();
		else
			q = cCorpse::getInsertQuery();
		q->addBindValue( serial() );
		q->addBindValue( bodyId_ );
		q->addBindValue( 0 /*hairStyle_*/ );
		q->addBindValue( 0 /*hairColor_*/ );
		q->addBindValue( 0 /*beardStyle_*/ );
		q->addBindValue( 0 /*beardColor_*/ );
		q->addBindValue( direction_ );
		q->addBindValue( QString( charbaseid_ ) );
		q->addBindValue( murderer_ );
		q->addBindValue( murdertime_ );
		if ( isPersistent )
			q->addBindValue( serial() );
		q->exec();
	}

	// Equipment can change as well
	if ( isPersistent )
	{
		q = cCorpse::getDeleteEquipmentQuery();
		q->addBindValue( serial() );
		q->exec();
	}

	q = cCorpse::getInsertEquipmentQuery();
	for ( QMap<quint8, SERIAL>::iterator it = equipment_.begin(); it != equipment_.end(); ++it )
	{
		q->addBindValue( serial() );
		q->addBindValue( it.key() ); 
		q->addBindValue( it.value() );
		q->exec();
	}

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
	if (id_ != 0x2006) {
		cItem::update(mSock);
	} else {
		cUOTxCorpseEquipment corpseEquip;
		//Old Client - cUOTxItemContent corpseContent;
		cUOTxNewItemContent corpseContent;
		cUOTxSendItem sendItem;

		corpseEquip.setSerial( serial() );

		for ( QMap<quint8, SERIAL>::iterator it = equipment_.begin(); it != equipment_.end(); ++it )
		{
			P_ITEM pItem = World::instance()->findItem( it.value() );

			if ( pItem && pItem->container() == this )
			{
				corpseEquip.addItem( it.key(), it.value() );
				corpseContent.addItem( pItem );
			}
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
			QList<cUOSocket*> sockets = Network::instance()->sockets();
			foreach ( mSock, sockets )
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
}

SERIAL cCorpse::getEquipment( quint8 layer )
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

void cCorpse::addEquipment( quint8 layer, SERIAL serial )
{
	if ( equipment_.find( layer ) != equipment_.end() )
	{
		equipment_[layer] = serial;
		return;
	}

	equipment_.insert( layer, serial );
}

stError* cCorpse::setProperty( const QString& name, const cVariant& value )
{
	changed( TOOLTIP );
	flagChanged();
	/*
		\property item.bodyid The body if of the dead creature.
		This property only exists for corpses.
	*/
	SET_INT_PROPERTY( "bodyid", bodyId_ )
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
		return 0;
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
	else if( charbaseid_ == "charbaseid" ) 
	{
		QString text = value.toString();
		if( text == QString::null )
			PROPERTY_ERROR( -2, "String expected" );
		charbaseid_ = text.toLatin1();
		return 0; 
	}

	return cItem::setProperty( name, value );
}

PyObject* cCorpse::getProperty( const QString& name, uint hash )
{
	PY_PROPERTY( "bodyid", bodyId_ )
	PY_PROPERTY( "murderer", FindCharBySerial( murderer_ ) )
	PY_PROPERTY( "murdertime", murdertime_ )
	PY_PROPERTY( "direction", direction_ )
	PY_PROPERTY( "charbaseid", charbaseid_ )
	return cItem::getProperty( name, hash );
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

	// Add count of items and their total weight.
	unsigned int count = content_.count();
	unsigned int weight = (( totalweight_ >= 0.0f ) ? ( unsigned int ) floor( totalweight_ ) : 0 );
	tooltip.addLine( 1050044, QString( "%1\t%2" ).arg( count ).arg( weight ) );
}

unsigned int cCorpse::decayDelay()
{
	unsigned int delay = cItem::decayDelay();

	if ( delay == 0 )
	{
		return 0;
	}

	// Player Corpse?
	P_PLAYER player = dynamic_cast<P_PLAYER>(owner());

	if (player) {
		return Config::instance()->playerCorpseDecayTime() * MY_CLOCKS_PER_SEC;
	} else {
		return Config::instance()->npcCorpseDecayTime() * MY_CLOCKS_PER_SEC;
	}
}
