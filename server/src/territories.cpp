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

#include "territories.h"
#include "wpdefmanager.h"
#include "globals.h"
#include "network/uosocket.h"
#include "defines.h"
#include "utilsys.h"
#include "srvparams.h"
#include "network.h"

#include "junk.h" // needed for object SrvParams

// cTerritories

void cTerritory::init( void )
{
	cBaseRegion::init();
	midilist_ = "";
	flags_ = 0;
	guardowner_ = QString();
	snowchance_ = 50;
	rainchance_ = 50;
	guardSections_ = QStringList();
	guardSections_.push_back( "standard_guard" );
}

void cTerritory::processNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	QString Value = this->getNodeValue( Tag );

	//<guards>
	//  <npc mult="2">npcsection</npc> (mult inserts 2 same sections into the list so the probability rises!
	//	<npc><random list="npcsectionlist" /></npc>
	//  <list id="npcsectionlist" />
	//</guards>
	if( TagName == "guards" )
	{
		QDomNode childNode = Tag.firstChild();
		while( !childNode.isNull() )
		{
			if( !childNode.isElement() )
				this->guardSections_.push_back( Value );
			else if( childNode.nodeName() == "npc" )
			{
				UI32 mult = childNode.toElement().attribute( "mult" ).toInt();
				if( mult < 1 )
					mult = 1;

				for( UI32 i = 0; i < mult; i++ )
					this->guardSections_.push_back( this->getNodeValue( childNode.toElement() ) );
			}
			else if( childNode.nodeName() == "list" && childNode.attributes().contains( "id" ) )
			{
				QStringList NpcList = DefManager->getList( childNode.toElement().attribute( "id" ) );
				QStringList::iterator it = NpcList.begin();
				while( it != NpcList.end() )
				{
					this->guardSections_.push_back( *it );
					it++;
				}
			}

			childNode = childNode.nextSibling();
		}
	}

	// <guardowner>text</guardowner>
	else if( TagName == "guardowner" )
		this->guardowner_ = Value;

	// <midilist>MIDI_COMBAT</midilist>
	else if( TagName == "midilist" )
		this->midilist_ = Value;

	else if( TagName == "flags" )
	{
		flags_ = 0;

		QDomNode childNode = Tag.firstChild();
		while( !childNode.isNull() )
		{
			if( childNode.isElement() )
			{
				if( childNode.nodeName() == "guarded" )
					setGuarded( true );
				else if( childNode.nodeName() == "nomark" )
					setNoMark( true );
				else if( childNode.nodeName() == "nogate" )
					setNoGate( true );
				else if( childNode.nodeName() == "norecallout" )
					setNoRecallOut( true );
				else if( childNode.nodeName() == "norecallin" )
					setNoRecallIn( true );
				else if( childNode.nodeName() == "recallshield" )
					setRecallShield( true );
				else if( childNode.nodeName() == "noagressivemagic" )
					setNoAgressiveMagic( true );
				else if( childNode.nodeName() == "antimagic" )
					setAntiMagic( true );
				else if( childNode.nodeName() == "escortregion" )
					setValidEscortRegion( true );
				else if( childNode.nodeName() == "cave" )
					setCave( true );
				else if( childNode.nodeName() == "nomusic" )
					setNoMusic( true );
			}
			childNode = childNode.nextSibling();
		}
	}

	// <snowchance>50</snowchance>
	else if( TagName == "snowchance" )
		this->snowchance_ = Value.toUShort();

	// <rainchance>50</rainchance>
	else if( TagName == "rainchance" )
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
	else if( TagName == "tradesystem" )
	{
		QDomNode childNode = Tag.firstChild();
		while( !childNode.isNull() )
		{
			if( childNode.isElement() )
			{
				good_st goods;
				goods.buyable = 0;
				goods.sellable = 0;
				goods.rndmin = 0;
				goods.rndmax = 0;
				UI32 num = 0xFFFFFFFF;

				if( childNode.nodeName() == "good" )
				{
					if( childNode.attributes().contains( "num" ) )
						num = childNode.toElement().attribute( "num" ).toInt();
					QDomNode chchildNode = childNode.firstChild();
					while( !chchildNode.isNull() )
					{
						if( chchildNode.isElement() )
						{
							QString childValue = this->getNodeValue( chchildNode.toElement() );
							if( chchildNode.nodeName() == "buyable" )
								goods.buyable = childValue.toInt();
							else if( chchildNode.nodeName() == "sellable" )
								goods.sellable = childValue.toInt();
							else if( chchildNode.nodeName() == "randomvalue" )
							{
								goods.rndmin = chchildNode.toElement().attribute( "min" ).toInt();
								goods.rndmax = chchildNode.toElement().attribute( "max" ).toInt();
							}
						}
						chchildNode = chchildNode.nextSibling();
					}
				}

				if( num != 0xFFFFFFFF )
					this->tradesystem_[num] = goods;
			}
			childNode = childNode.nextSibling();
		}
	}

	// <region id="Cove Market Place">
	//		...region nodes...
	// </region>
	else if( TagName == "region" && Tag.attributes().contains( "id" ) )
	{
		cTerritory* toinsert_ = new cTerritory( Tag, this );
		this->subregions_.push_back( toinsert_ );
	}
	else if ( TagName == "teleport" )
	{
		if ( !Tag.attributes().contains("source") )
		{
			qWarning("ERROR: processing teleport tag, missing source attribute");
			return;
		}

		if ( !Tag.attributes().contains("destination") )
		{
			qWarning("ERROR: processing teleport tag, missing destination attribute");
			return;
		}
		Coord_cl source, destination;
		if ( !parseCoordinates( Tag.attribute( "source" ), source ) )
		{
			qWarning("ERROR: parsing source attribute, not a valid coordinate vector");
			return;
		}
		if ( !parseCoordinates( Tag.attribute( "destination" ), destination ) )
		{
			qWarning("ERROR: parsing destination attribute, not a valid coordinate vector");
			return;
		}
		teleporters_st teleporter;
		teleporter.source = source;
		teleporter.destination = destination;
		teleporters.append( teleporter );
	}
	else
		cBaseRegion::processNode( Tag );
}

QString cTerritory::getGuardSect( void )
{
	return this->guardSections_[ RandomNum( 0, this->guardSections_.size()-1 ) ];
}

bool cTerritory::haveTeleporters() const
{
	return !teleporters.isEmpty();
}

bool cTerritory::findTeleporterSpot( Coord_cl& d ) const
{
	QValueList< teleporters_st >::const_iterator it(teleporters.begin());
	QValueList< teleporters_st >::const_iterator end(teleporters.end());
	for ( ; it != end; ++it )
	{
		if ( d == (*it).source )
		{
			d = (*it).destination;
			break;
		}
	}
	return it != end;
}

// cAllTerritories

void cAllTerritories::load( void )
{
	UI32 starttime = getNormalizedTime();
	QStringList DefSections = DefManager->getSections( WPDT_REGION );
	clConsole.PrepareProgress( "Loading regions" );

	if( DefSections.isEmpty() )
	{
		clConsole.ProgressFail();
		clConsole.error( "no regions defined! you need one region at least!" );
		error = 1;
		return;
	}
	
	QStringList::iterator it( DefSections.begin() );
	while( it != DefSections.end() )
	{
		const QDomElement* DefSection = DefManager->getSection( WPDT_REGION, *it );
		cTerritory* territory = new cTerritory( *DefSection, 0 );
		if ( territory->rectangles().empty() )
		{
			clConsole.send( tr("Warning: Top level region %1 lacks rectangle tag, ignoring region").arg(territory->name()) );
			delete territory;
		}
		else
		{
			topregions.insert( territory->rectangles()[0].map, territory );
		}
		++it;
	}

	UI32 endtime = getNormalizedTime();
	clConsole.ProgressDone();

	clConsole.send( tr( "Loaded %1 regions in %2 sec.\n" ).arg( this->count() ).arg( (float)((float)endtime - (float)starttime) / MY_CLOCKS_PER_SEC ) );
}

void cAllTerritories::check( P_CHAR pc )
{
	cUOSocket *socket = pc->socket();
	cTerritory* currRegion = this->region( pc->pos.x, pc->pos.y, pc->pos.map );
	cTerritory* lastRegion = pc->region();

	if( !currRegion )
		return;

	if( !lastRegion )
	{
		pc->setRegion( currRegion );
		return;
	}

	if( currRegion != lastRegion )
	{
		if( socket )
		{
			if( lastRegion )
				socket->sysMessage( tr( "You have left %1." ).arg( lastRegion->name() ), 0x37 );

			if( currRegion )
				socket->sysMessage( tr( "You have entered %1." ).arg( currRegion->name() ), 0x37 );

			if( (currRegion->isGuarded() && !lastRegion->isGuarded()) ||
				(!currRegion->isGuarded() && lastRegion->isGuarded()) ||
				(currRegion->isGuarded() && ( currRegion->guardOwner() != lastRegion->guardOwner() )) )
			{
				if( currRegion->isGuarded() )
				{
					if( currRegion->guardOwner().isEmpty() )
						socket->sysMessage( tr( "You have a safe feeling." ), 0x37 );
					else
						socket->sysMessage( tr( "You are now under the protection of %1 guards." ).arg( currRegion->guardOwner() ), 0x37 );
				}
				else
				{
					if( lastRegion->guardOwner().isEmpty() )
						socket->sysMessage( tr( "You no longer have a safe feeling." ), 0x37 );
					else
						socket->sysMessage( tr( "You are no longer under the protection of %1 guards." ).arg( lastRegion->guardOwner() ), 0x37 );
				}
			}
		}

		pc->setRegion( currRegion );

		if( socket )
			socket->playMusic();
	}
}

// Singleton
cAllTerritories cAllTerritories::instance;


