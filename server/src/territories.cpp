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

#include "territories.h"
#include "definitions.h"

#include "network/uosocket.h"
#include "defines.h"
#include "config.h"
#include "network/network.h"
#include "basics.h"
#include "console.h"
#include "exceptions.h"
#include "world.h"
#include "basechar.h"
#include "player.h"
#include "inlines.h"
#include "muls/maps.h"

// needed for object Config
#include "pythonscript.h"

// cTerritories

cTerritory::cTerritory( const cElement* Tag, cBaseRegion* parent )
{
	this->init();
	if ( Tag->hasAttribute( "id" ) )
	{
		this->name_ = Tag->getAttribute( "id" );
	}
	else if ( Tag->hasAttribute( "name" ) )
	{
		this->name_ = Tag->getAttribute( "name" );
	}
	this->applyDefinition( Tag );
	this->parent_ = parent;
}

cTerritory::cTerritory()
{
	this->init();
	this->name_ = QString::null;
	this->parent_ = 0;
}

void cTerritory::init( void )
{
	cBaseRegion::init();
	midilist_ = "";
	flags_ = 0;
	guardowner_ = QString();
	snowchance_ = 50;
	rainchance_ = 50;
	guardSections_ = QStringList();
	//	guardSections_.push_back( "standard_guard" );
}

void cTerritory::processNode( const cElement* Tag )
{
	QString TagName = Tag->name();
	QString Value = Tag->value();

	//<guards>
	//  <npc mult="2">npcsection</npc> (mult inserts 2 same sections into the list so the probability rises!
	//	<npc><random list="npcsectionlist" /></npc>
	//  <list id="npcsectionlist" />
	//</guards>
	if ( TagName == "guards" )
	{
		for ( unsigned int i = 0; i < Tag->childCount(); ++i )
		{
			const cElement* childNode = Tag->getChild( i );

			if ( childNode->name() == "npc" )
			{
				UI32 mult = childNode->getAttribute( "mult" ).toInt();
				if ( mult < 1 )
					mult = 1;

				for ( UI32 i = 0; i < mult; i++ )
					this->guardSections_.push_back( childNode->value() );
			}
			else if ( childNode->name() == "list" && childNode->hasAttribute( "id" ) )
			{
				QStringList NpcList = Definitions::instance()->getList( childNode->getAttribute( "id" ) );
				QStringList::iterator it = NpcList.begin();
				while ( it != NpcList.end() )
				{
					this->guardSections_.push_back( *it );
					it++;
				}
			}
		}
	}

	// <guardowner>text</guardowner>
	else if ( TagName == "guardowner" )
		this->guardowner_ = Value;

	// <midilist>MIDI_COMBAT</midilist>
	else if ( TagName == "midilist" )
		this->midilist_ = Value;
	else if ( TagName == "flags" )
	{
		flags_ = 0;

		for ( unsigned int i = 0; i < Tag->childCount(); ++i )
		{
			const cElement* childNode = Tag->getChild( i );

			if ( childNode->name() == "guarded" )
				setGuarded( true );
			else if ( childNode->name() == "nomark" )
				setNoMark( true );
			else if ( childNode->name() == "nogate" )
				setNoGate( true );
			else if ( childNode->name() == "norecallout" )
				setNoRecallOut( true );
			else if ( childNode->name() == "norecallin" )
				setNoRecallIn( true );
			else if ( childNode->name() == "recallshield" )
				setRecallShield( true );
			else if ( childNode->name() == "noagressivemagic" )
				setNoAgressiveMagic( true );
			else if ( childNode->name() == "antimagic" )
				setAntiMagic( true );
			else if ( childNode->name() == "escortregion" )
				setValidEscortRegion( true );
			else if ( childNode->name() == "cave" )
				setCave( true );
			else if ( childNode->name() == "nomusic" )
				setNoMusic( true );
		}
	}

	// <snowchance>50</snowchance>
	else if ( TagName == "snowchance" )
		this->snowchance_ = Value.toUShort();

	// <rainchance>50</rainchance>
	else if ( TagName == "rainchance" )
		this->rainchance_ = Value.toUShort();

	// <tradesystem>
	//		<good num="1">
	//			<buyable>10</buyable>
	//			<sellable>20</sellable>
	//			<randomvalue min="10" max="20" />
	//		</good>
	//		<good ...>
	//		...
	//		</good>
	// </tradesystem>
	else if ( TagName == "tradesystem" )
	{
		for ( unsigned int i = 0; i < Tag->childCount(); ++i )
		{
			const cElement* childNode = Tag->getChild( i );

			good_st goods;
			goods.buyable = 0;
			goods.sellable = 0;
			goods.rndmin = 0;
			goods.rndmax = 0;
			UI32 num = 0xFFFFFFFF;

			if ( childNode->name() == "good" )
			{
				if ( childNode->hasAttribute( "num" ) )
					num = childNode->getAttribute( "num" ).toInt();

				for ( unsigned int j = 0; j < childNode->childCount(); ++j )
				{
					const cElement* chchildNode = childNode->getChild( j );

					QString childValue = chchildNode->value();

					if ( chchildNode->name() == "buyable" )
						goods.buyable = childValue.toInt();
					else if ( chchildNode->name() == "sellable" )
						goods.sellable = childValue.toInt();
					else if ( chchildNode->name() == "randomvalue" )
					{
						goods.rndmin = chchildNode->getAttribute( "min" ).toInt();
						goods.rndmax = chchildNode->getAttribute( "max" ).toInt();
					}
				}
			}

			if ( num != 0xFFFFFFFF )
				this->tradesystem_[num] = goods;
		}
	}

	// <region id="Cove Market Place">
	//		...region nodes...
	// </region>
	else if ( TagName == "region" )
	{
		cTerritory* toinsert_ = new cTerritory( Tag, this );
		this->subregions_.push_back( toinsert_ );
	}
	else if ( TagName == "teleport" )
	{
		if ( !Tag->hasAttribute( "source" ) )
		{
			qWarning( "ERROR: processing teleport tag, missing source attribute" );
			return;
		}

		if ( !Tag->hasAttribute( "destination" ) )
		{
			qWarning( "ERROR: processing teleport tag, missing destination attribute" );
			return;
		}
		Coord_cl source, destination;
		if ( !parseCoordinates( Tag->getAttribute( "source" ), source ) )
		{
			qWarning( "ERROR: parsing source attribute, not a valid coordinate vector" );
			return;
		}
		if ( !parseCoordinates( Tag->getAttribute( "destination" ), destination ) )
		{
			qWarning( "ERROR: parsing destination attribute, not a valid coordinate vector" );
			return;
		}
		bool bothways = Tag->hasAttribute( "bothways" );
		teleporters_st teleporter;
		teleporter.source = source;
		teleporter.destination = destination;
		teleporters.append( teleporter );
		if ( bothways )
		{
			teleporter.source = destination;
			teleporter.destination = source;
			teleporters.append( teleporter );
		}
	}
	else
		cBaseRegion::processNode( Tag );
}

QString cTerritory::getGuardSect( void ) const
{
	if ( guardSections_.count() > 0 )
		return this->guardSections_[RandomNum( 0, this->guardSections_.size() - 1 )];
	else
		return ( char * ) 0;
}

bool cTerritory::haveTeleporters() const
{
	return !teleporters.isEmpty();
}

bool cTerritory::findTeleporterSpot( Coord_cl& d ) const
{
	QValueList<teleporters_st>::const_iterator it( teleporters.begin() );
	QValueList<teleporters_st>::const_iterator end( teleporters.end() );
	for ( ; it != end; ++it )
	{
		if ( d == ( *it ).source )
		{
			d = ( *it ).destination;
			break;
		}
	}
	return it != end;
}

// cTerritories

void cTerritories::unload()
{
	topregions.clear();
	cComponent::unload();
}

void cTerritories::load()
{
	// Make sure that there is one top level region for each map
	// Insert it at the beginning (last overrides first).
	for ( unsigned char i = 0; i <= 3; ++i )
	{
		if ( Maps::instance()->hasMap( i ) )
		{
			cTerritory* territory = new cTerritory();
			cBaseRegion::rect_st rect;
			rect.map = i;
			rect.x1 = 0;
			rect.y1 = 0;
			rect.x2 = Maps::instance()->mapTileWidth( i ) * 8;
			rect.y2 = Maps::instance()->mapTileHeight( i ) * 8;
			territory->rectangles().append( rect );
			topregions[i].append( territory );
		}
	}

	const QValueVector<cElement*>& elements = Definitions::instance()->getDefinitions( WPDT_REGION );

	QValueVector<cElement*>::const_iterator it( elements.begin() );
	while ( it != elements.end() )
	{
		cTerritory* territory = new cTerritory( *it, 0 );

		if ( territory->rectangles().empty() )
		{
			Console::instance()->log(LOG_WARNING, QString("Region %1 lacks rectangle tag, ignoring region.\n").arg(territory->name()));
			delete territory;
		}
		else
		{
			unsigned char map = territory->rectangles()[0].map;

			if ( !topregions.contains( map ) )
			{
				topregions[map].setAutoDelete( true );
			}

			topregions[map].append( territory );
		}
		++it;
	}

	cComponent::load();
}

void cTerritories::check( P_CHAR pc )
{
	cUOSocket* socket = NULL;
	if ( pc->objectType() == enPlayer )
		socket = dynamic_cast<P_PLAYER>( pc )->socket();
	cTerritory* currRegion = this->region( pc->pos().x, pc->pos().y, pc->pos().map );
	cTerritory* lastRegion = pc->region();

	if ( !currRegion )
		return;

	if ( !lastRegion )
	{
		pc->setRegion( currRegion );
		return;
	}

	if ( currRegion != lastRegion )
	{
		pc->setRegion( currRegion );

		if ( socket )
		{
			// If the last region was a cave or if the new region is a cave,
			// update the lightlevel.
			if ( ( currRegion->isCave() && !lastRegion->isCave() ) || ( !currRegion->isCave() && lastRegion->isCave() ) )
			{
				socket->updateLightLevel();
			}

			socket->playMusic();
		}

		PyObject* args = Py_BuildValue( "(NNN)", PyGetCharObject( pc ), PyGetRegionObject( lastRegion ), PyGetRegionObject( currRegion ) );
		if ( !cPythonScript::callChainedEventHandler( EVENT_CHANGEREGION, pc->getEvents(), args ) && socket )
		{
			if ( lastRegion && !lastRegion->name().isEmpty() )
				socket->sysMessage( tr( "You have left %1." ).arg( lastRegion->name() ) );

			if ( currRegion && !currRegion->name().isEmpty() )
				socket->sysMessage( tr( "You have entered %1." ).arg( currRegion->name() ) );

			if ( ( currRegion->isGuarded() != lastRegion->isGuarded() ) || ( currRegion->isGuarded() && ( currRegion->guardOwner() != lastRegion->guardOwner() ) ) )
			{
				if ( currRegion->isGuarded() )
				{
					if ( currRegion->guardOwner().isEmpty() )
						socket->clilocMessage( 500112 ); // You are now under the protection of the town guards
					else
						socket->sysMessage( currRegion->guardOwner() );
				}
				else
				{
					if ( lastRegion->guardOwner().isEmpty() )
						socket->clilocMessage( 500113 ); // You have left the protection of the town guards.
					else
						socket->sysMessage( lastRegion->guardOwner() );
				}
			}
		}
		Py_DECREF( args );
	}
}

cTerritory* cTerritories::region( const QString& regName )
{
	cTerritory* result = 0;
	QMap<uint, QPtrList<cTerritory> >::iterator it( topregions.begin() );
	for ( ; it != topregions.end(); ++it )
	{
		// search all topregions of that map
		for ( cTerritory*region = it.data().first(); region; region = it.data().next() )
		{
			region = dynamic_cast<cTerritory*>( region->region( regName ) );
			if ( region )
			{
				result = region;
			}
		}
	}
	return result;
}

cTerritory* cTerritories::region( UI16 posx, UI16 posy, UI08 map )
{
	QMap<uint, QPtrList<cTerritory> >::iterator it( topregions.find( map ) );
	cTerritory* result = 0;

	if ( it != topregions.end() )
	{
		// search all topregions of that map
		for ( cTerritory*region = it.data().first(); region; region = it.data().next() )
		{
			region = dynamic_cast<cTerritory*>( region->region( posx, posy, map ) );
			if ( region )
			{
				result = region;
			}
		}
	}

	return result;
}

void cTerritories::reload()
{
	unload();
	load();

	// Update the Regions
	cCharIterator iter;
	for ( P_CHAR pChar = iter.first(); pChar; pChar = iter.next() )
	{
		cTerritory* region = this->region( pChar->pos().x, pChar->pos().y, pChar->pos().map );
		pChar->setRegion( region );
	}
}
