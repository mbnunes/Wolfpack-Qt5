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
#include "defines.h"
#include "utilsys.h"
#include "srvparams.h"
#include "network.h"

#include "junk.h" // needed for object SrvParams

// cTerritories

void cTerritory::Init( void )
{
	cBaseRegion::Init();
	midilist_ = 0;
	guarded_ = false;
	mark_ = true;
	gate_ = true;
	recall_ = true;
	rain_ = true;
	snow_ = true;
	magicdamage_ = true;
	escort_ = false;
	magic_ = true;
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

			childNode.nextSibling();
		}
	}

	// <escortregion />
	else if( TagName == "escortregion" )
		this->escort_ = true;
	// scps:	validEscortRegion[escortRegions] = i;
	// should be implemented as a method of cAllTerritories to check for the tag!

	// <guardowner>text</guardowner>
	else if( TagName == "guardowner" )
		this->guardowner_ = Value;

	// <midilist>1</midilist>
	else if( TagName == "midilist" )
		this->midilist_ = Value.toInt();

	// <guarded />
	else if( TagName == "guarded" )
		this->guarded_ = false;

	// <nomagicdamage />
	else if( TagName == "nomagicdamage" )
		this->magicdamage_ = false;

	// <nomagic />
	else if( TagName == "nomagic" )
		this->magic_ = false;

	// <nomark />
	else if( TagName == "nomark" )
		this->mark_ = false;

	// <nogate />
	else if( TagName == "nogate" )
		this->gate_ = false;

	// <norecall />
	else if( TagName == "norecall" )
		this->recall_ = false;

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

	else
		cBaseRegion::processNode( Tag );
}

QString cTerritory::getGuardSect( void )
{
	return this->guardSections_[ RandomNum( 0, this->guardSections_.size()-1 ) ];
}

bool	cTerritory::contains( UI16 posx, UI16 posy )
{
	for( UI32 i = 0; i < this->x1_.size(); i++ )
	{
		if( ( ( posx >= x1_[i] && posx <= x2_[i] ) ||
			( posx >= x2_[i] && posx <= x2_[i] ) ) &&
			( ( posy >= y1_[i] && posy <= y2_[i] ) ||
			( posy >= y2_[i] && posy <= y2_[i] ) ) )
			return true;
	}
	return false;
}

// cAllTerritories

cAllTerritories::~cAllTerritories( void )
{
	iterator it = this->begin();
	while( it != this->end() )
	{
		delete it->second; // delete the cTerritory objects from the stack!
		it++;
	}
}

void cAllTerritories::Load( void )
{
	UI32 starttime = getNormalizedTime();
	QStringList DefSections = DefManager->getSections( WPDT_REGION );
	clConsole.PrepareProgress( "Loading regions..." );

	QStringList::iterator it = DefSections.begin();
	while( it != DefSections.end() )
	{
		QDomElement* DefSection = DefManager->getSection( WPDT_REGION, *it );

		pair< QString, cTerritory* > toInsert( *it, new cTerritory( *DefSection ) );
		this->insert( toInsert );

		it++;
	}

	UI32 endtime = getNormalizedTime();
	clConsole.ProgressDone();
	clConsole.send( QString( "Loaded %1 regions in %2 sec.\n" ).arg( DefSections.size() ).arg( (float)((float)endtime - (float)starttime) / MY_CLOCKS_PER_SEC ) );
}

cTerritory*	cAllTerritories::region( QString regName )
{
	iterator it = this->find( regName );
	if( it != this->end() )
		return dynamic_cast< cTerritory* >(it->second);
	else
		return NULL;
}

cTerritory* cAllTerritories::region( UI16 posx, UI16 posy )
{
	iterator it = this->begin();
	cTerritory* Region = NULL;
	while( it != this->end() && Region == NULL )
	{
		if( dynamic_cast< cTerritory* >(it->second)->contains( posx, posy ) )
			Region = dynamic_cast< cTerritory* >(it->second);
		it++;
	}

	return Region;
}

void cAllTerritories::Check( P_CHAR pc )
{
	UOXSOCKET s = calcSocketFromChar( pc );
	UI32 j;
	cTerritory* currRegion = cAllTerritories::getInstance()->region( pc->pos.x, pc->pos.y );
	cTerritory* lastRegion = cAllTerritories::getInstance()->region( pc->region );

	if( currRegion != lastRegion )
	{
		if( s != -1 )
		{
			if( lastRegion != NULL )
			{
				sprintf((char*)temp, "You have left %s.", lastRegion->name().latin1() );
				sysmessage(s, 0x37, (char*)temp);
			}
			if( currRegion != NULL )
			{
				sprintf((char*)temp, "You have entered %s.", currRegion->name().latin1() );
				sysmessage(s, 0x37, (char*)temp);
			}
			j=strcmp( currRegion->guardOwner(), lastRegion->guardOwner());
			if( (currRegion->isGuarded() && !lastRegion->isGuarded()) ||
				(!currRegion->isGuarded() && lastRegion->isGuarded()) ||
				(currRegion->isGuarded() && j) )
			{
				if( currRegion->isGuarded() )
				{
					if( currRegion->guardOwner().isEmpty() )
					{
						sysmessage(s, 0x37, "You are now under the protection of the guards.");
					}
					else
					{
						sprintf((char*)temp, "You are now under the protection of %s guards.", currRegion->guardOwner().latin1() );
						sysmessage(s, 0x37, (char*) temp);
					}
				}
				else
				{
					if( lastRegion->guardOwner().isEmpty() )
					{
						sysmessage(s, 0x37, "You are no longer under the protection of the guards.");
					}
					else
					{
						sprintf((char*)temp, "You are no longer under the protection of %s guards.", lastRegion->guardOwner().latin1() );
						sysmessage(s, 0x37, (char*)temp);
					}
				}
			}
		}

		pc->region=currRegion->name();
		if( s!=-1 ) 
		{
			dosocketmidi(s);
		}
		if( indungeon(pc) ) 
		{
			dolight( s, SrvParams->dungeonLightLevel() );
		}
	}
}

// Singleton
cAllTerritories cAllTerritories::instance;