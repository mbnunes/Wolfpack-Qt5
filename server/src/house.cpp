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

/* House code for deed creation by Tal Strake, revised by Cironian */

//#include "wolfpack.h"
#include "house.h"
#include "persistentbroker.h"
#include "sectors.h"
#include "srvparams.h"
#include "maps.h"
#include "debug.h"
#include "tilecache.h"
#include "utilsys.h"
#include "network.h"
#include "network/uosocket.h"
#include "multiscache.h"
#include "basechar.h"
#include "player.h"
#include "npc.h"
#include "chars.h"
#include "customtags.h"
#include "territories.h"

// System Include Files
#include <algorithm>

// Namespaces for this module
using namespace std;

#undef  DBGFILE
#define DBGFILE "house.cpp"

void cHouse::processHouseItemNode( const cElement *Tag )
{
	P_CHAR pOwner = FindCharBySerial( ownSerial() );
	P_ITEM nItem = new cItem;

	if( !nItem )
		return;
	
	nItem->Init( true );

	nItem->applyDefinition( Tag );

	if( nItem->type() == 222 )
		nItem->setName( name() );

	nItem->SetOwnSerial( this->ownSerial() );
	addItem( nItem );
	Coord_cl npos = this->pos();

	for( unsigned int i = 0; i < Tag->childCount(); ++i )
	{
		const cElement *childTag = Tag->getChild( i );
		
		QString TagName = childTag->name();
		QString Value = childTag->getValue();

		// <pack />
		if( TagName == "pack" && pOwner )
		{
			P_ITEM pBackpack = pOwner->getBackpack();
			pBackpack->addItem( nItem );
		}

		// <lock />
		else if( TagName == "lock" )
		{
			nItem->setMagic(4);
		}

		// <secure />
		else if( TagName == "secure" )
			nItem->setMagic(3);

		// <position x="1" y="5" z="20" />
		else if( TagName == "position" )
		{
			npos.x = npos.x + childTag->getAttribute( "x" ).toShort();
			npos.y = npos.y + childTag->getAttribute( "y" ).toShort();
			npos.z = npos.z + childTag->getAttribute( "z" ).toShort();
		}
	}

	nItem->moveTo( npos );
	nItem->update();
}

bool cHouse::onValidPlace()
{
    cTerritory* Region = AllTerritories::instance()->region( pos().x, pos().y, pos().map );
	if( Region != NULL && Region->isGuarded() && SrvParams->houseInTown() == 0 )
		return false;

	const UI32 multiid = this->id() - 0x4000;

	int j;
	MultiDefinition* def = MultiCache::instance()->getMulti( multiid );
	if ( !def )
		return false;

	QValueVector<multiItem_st> multi = def->getEntries();
	SI08 mapz = 0;
	tile_st tile;
	for( j = 0; j < multi.size(); j++ )
	{
		Coord_cl multipos = Coord_cl( multi[j].x + pos().x, multi[j].y + pos().y, pos().z, pos().map );

		mapz = Map->mapElevation( multipos );
		if( pos().z < mapz )
			return false;

		land_st mapTile = TileCache::instance()->getLand( Map->seekMap( multipos ).id );
		if( mapTile.flag1 & 0x40 || mapTile.flag1 & 0x80 )
			return false;
		
		StaticsIterator msi = Map->staticsIterator( multipos );
		while( !msi.atEnd() )
		{
			tile = TileCache::instance()->getTile( msi->itemid );
			if( multi[j].z > msi->zoff && multi[j].z < (msi->zoff + tile.height) )
				return false;
			++msi;
		}
		
		RegionIterator4Items ri( multipos );
		for( ri.Begin(); !ri.atEnd(); ri++ ) 
		{
			P_ITEM pi = ri.GetData();
			if( pi && pi->multis() != serial() )
			{
				tile = TileCache::instance()->getTile( pi->id() );
				if( multi[j].z > pi->pos().z && multi[j].z < ( pi->pos().z + tile.height ) )
					return false;
			}
		}
	}
	return true;
}

void cHouse::processNode( const cElement *Tag )
{
	QString TagName = Tag->name();
	QString Value = Tag->getValue();

	// <id>16572</id>
	if( TagName == "id" )
		this->setId( Value.toUShort() );

	// <movechar x="0" y="0" z="8" />
	else if( TagName == "movechar" )
	{
		charpos_.x = Tag->getAttribute( "x" ).toInt();
		charpos_.y = Tag->getAttribute( "y" ).toInt();
		charpos_.z = Tag->getAttribute( "z" ).toInt();
	}

	// <item>
	//     <inherit id="houseitem_a" />
	// </item>
	else if( TagName == "item" )
	{
		processHouseItemNode( Tag );
	}

	// <nokey />
	else if( TagName == "nokey" )
		nokey_ = true;

	else
		cMulti::processNode( Tag );
}

void cHouse::build( const cElement *Tag, UI16 posx, UI16 posy, SI08 posz, SERIAL senderserial, SERIAL deedserial )
{
	P_PLAYER pc_currchar = dynamic_cast<P_PLAYER>( FindCharBySerial( senderserial ) );
	if ( !pc_currchar )
		return;
	cUOSocket* socket = pc_currchar->socket();

	this->setSerial( World::instance()->findItemSerial() );
	this->SetOwnSerial( senderserial );
	this->setPriv( 0 );
	this->MoveTo( posx, posy, posz );

	this->applyDefinition( Tag );

	if( !this->onValidPlace() )
	{
		if( socket )
			socket->sysMessage( tr("Can not build a house at this location!") );
		this->remove();
		return;
	}

	this->update();

	P_ITEM pDeed = FindItemBySerial( deedserial );
	if( pDeed != NULL )
		pDeed->remove();

	if( !nokey_ )
		createKeys( pc_currchar, tr( "house key" ) );

	RegionIterator4Items ri( this->pos() );
	for(ri.Begin(); !ri.atEnd(); ri++)
	{
		P_ITEM si = ri.GetData();
		si->update();
	}
		
	pc_currchar->MoveTo( this->pos().x + charpos_.x, this->pos().y + charpos_.y, this->pos().z + charpos_.z );
	pc_currchar->resend();
}

void cHouse::remove( void )
{
	removeKeys();
/*	RegionIterator4Chars ri(this->pos());
	for (ri.Begin(); !ri.atEnd(); ri++)
	{
		P_NPC pc = dynamic_cast<P_NPC>(ri.GetData());
		if ( !pc )
			continue;

		if(pc->npcaitype() == 17 && pc->multis() == this->serial())
			cCharStuff::DeleteChar(pc);
	}*/
	RegionIterator4Items rii(this->pos());
	for(rii.Begin(); !rii.atEnd(); rii++)
	{
		P_ITEM pi = rii.GetData();
		if(pi->multis() == this->serial() && pi->serial() != this->serial() && pi->type() != 202)
			pi->remove();
	}
}

void cHouse::toDeed( cUOSocket* socket )
{
	P_CHAR pc_currchar = socket->player();
	if( !pc_currchar )
		return;
	P_ITEM pBackpack = pc_currchar->getBackpack();

/*	RegionIterator4Chars ri(this->pos());
	for (ri.Begin(); !ri.atEnd(); ri++)
	{
		P_NPC pc = dynamic_cast<P_NPC>(ri.GetData());
		if ( !pc )
			continue;
		if( pBackpack && pc->npcaitype() == 17 && pc->multis() == this->serial() )
		{
			P_ITEM pPvDeed = cItem::createFromScript( "14f0" );
			if( pPvDeed )
			{
				pPvDeed->setName( tr("A vendor deed for %1").arg( pc->name() ) );
				pPvDeed->setType( 217 );
				pPvDeed->setBuyprice( 2000 );
				pPvDeed->setSellprice( 1000 );
				pPvDeed->update();
				socket->sysMessage( tr("Packed up vendor %1.").arg(pc->name()) );
			}
		}
	}*/

	this->remove();
	
	if( this->deedsection_.isNull() || this->deedsection_.isEmpty() || !pBackpack )
	{
		this->remove();
		return;
	}

	P_ITEM pDeed = cItem::createFromScript( this->deedsection_ );
	if( pDeed ) 
	{
		pBackpack->addItem( pDeed );
		pDeed->update();
	}
	this->remove();
	socket->sysMessage( tr("You turned the boat into a deed.") );
}

void cHouse::buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions )
{
	cMulti::buildSqlString( fields, tables, conditions );
	fields.push_back( "houses.nokey,houses.charpos_x,houses.charpos_y,houses.charpos_z" );
	tables.push_back( "houses" );
	conditions.push_back( "uobjectmap.serial = houses.serial" );
}

void cHouse::load( char **result, UINT16 &offset )
{
	cMulti::load( result, offset );
	nokey_ = atoi( result[offset++] );
	charpos_.x = atoi( result[offset++] );
	charpos_.y = atoi( result[offset++] );
	charpos_.z = atoi( result[offset++] );
}

void cHouse::save()
{
	initSave;
	setTable( "houses" );
	
	addField( "serial", serial() );
	addField( "nokey", nokey_ );
	addField( "charpos_x", charpos_.x );
	addField( "charpos_y", charpos_.y );
	addField( "charpos_z", charpos_.z );

	addCondition( "serial", serial() );
	saveFields;

	cMulti::save();
}

bool cHouse::del()
{
	if( !isPersistent )
		return false;

	persistentBroker->addToDeleteQueue( "houses", QString( "serial = '%1'" ).arg( serial() ) );

	return cMulti::del();
}

static cUObject* productCreator()
{
	return new cHouse;
}

void cHouse::registerInFactory()
{
	QStringList fields, tables, conditions;
	buildSqlString( fields, tables, conditions ); // Build our SQL string
	QString sqlString = QString( "SELECT %1 FROM uobjectmap,%2 WHERE uobjectmap.type = 'cHouse' AND %3" ).arg( fields.join( "," ) ).arg( tables.join( "," ) ).arg( conditions.join( " AND " ) );
	UObjectFactory::instance()->registerType("cHouse", productCreator);
	UObjectFactory::instance()->registerSqlQuery( "cHouse", sqlString );
}

void cHouse::sendCH( cUOSocket* socket )
{
	cUOTxSendItem deed;

	deed.setSerial( serial() );
	deed.setId( id() );
	deed.setAmount( 1 );
	deed.setCoord( pos() );

	cUOTxCustomHouse customhouse;
	customhouse.setSerial( this->serial() );
	customhouse.setCompression( 0 );	
	customhouse.setRevision( this->revision() );

	for( UINT32 i = 0; i < chtiles_.count(); i++ )
		customhouse.addTile( chtiles_[i].model(), chtiles_[i].pos() );
	
	socket->send( &deed );
	socket->send( &customhouse );
}