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
//	GNU General Public Li678cense for more details.
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

/* House code for deed creation by Tal Strake, revised by Cironian */

//#include "wolfpack.h"
#include "house.h"
#include "iserialization.h"
#include "regions.h"
#include "srvparams.h"
#include "mapstuff.h"
#include "debug.h"
#include "utilsys.h"
#include "network.h"

#include "customtags.h"
#include "territories.h"

// System Include Files
#include <algorithm>

// Namespaces for this module
using namespace std;

#undef  DBGFILE
#define DBGFILE "house.cpp"

void cHouseItem::processNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	QString Value = this->getNodeValue( Tag );
	P_CHAR pOwner = FindCharBySerial( this->GetOwnSerial() );

	// <pack />
	if( TagName == "pack" )
	{
		P_ITEM pBackpack = Packitem( pOwner );
		if( pBackpack != NULL )
			this->setContSerial( pBackpack->serial );
	}

	// <lock />
#pragma note( "Replace with customtags! see house.h" )
	else if( TagName == "lock" )
		this->locked_ = true;

	// <position x="1" y="5" z="20" />
	else if( TagName == "position" )
	{
		this->pos.x = this->pos.x + Tag.attribute( "x" ).toUShort();
		this->pos.y = this->pos.y + Tag.attribute( "y" ).toUShort();
		this->pos.z = this->pos.z + Tag.attribute( "z" ).toUShort();
	}

	else
		cItem::processNode( Tag );
}

bool cHouse::onValidPlace()
{
    cTerritory* Region = cAllTerritories::getInstance()->region( pos.x, pos.y );
	if( Region != NULL && Region->isGuarded() && SrvParams->houseInTown() == 0 )
		return false;

	UI32 multiid = this->id() - 0x4000;

	int j;
	SI32 length;
	st_multi multi;
	UOXFile *mfile;
	Map->SeekMulti(multiid, &mfile, &length);
	length=length/sizeof(st_multi);
	if (length == -1 || length>=17000000)//Too big...
	{
		clConsole.log( QString( "cBoat::isValidPlace: Bad length in multi file. Avoiding stall." ).latin1() );
		length = 0;
	}

	SI08 mapz = 0;
	tile_st tile;
	for( j = 0; j < length; j++ )
	{
		mfile->get_st_multi(&multi);

		mapz = Map->MapElevation( Coord_cl( multi.x + pos.x, multi.y + pos.y, pos.z, pos.map ) );
		if( pos.z < mapz )
			return false;
		
		MapStaticIterator msi( Coord_cl( multi.x + pos.x, multi.y + pos.y, pos.z, pos.map ) );
		staticrecord *stat = msi.Next();
		while( stat != NULL )
		{
			msi.GetTile( &tile );
			if( multi.z > stat->zoff && multi.z < (stat->zoff + tile.height) )
				return false;
			stat = msi.Next();
		}
		
		cRegion::RegionIterator4Items ri( Coord_cl( multi.x + pos.x, multi.y + pos.y, pos.z, pos.map ) );
		for( ri.Begin(); !ri.atEnd(); ri++ ) 
		{
			P_ITEM pi = ri.GetData();
			if( pi != NULL )
			{
				Map->SeekTile( pi->id(), &tile );
				if( multi.z > pi->pos.z && multi.z < ( pi->pos.z + tile.height ) )
					return false;
			}
		}
	}
	return true;
}

void cHouse::processNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	QString Value = this->getNodeValue( Tag );

	// <id>16572</id>
	if( TagName == "id" )
		this->setId( Value.toUShort() );

	// <space x="3" y="3" />
	else if( TagName == "space" )
	{
		space_.x = Tag.attribute( "x" ).toInt();
		space_.y = Tag.attribute( "y" ).toInt();
	}

	// <movechar x="0" y="0" z="8" />
	else if( TagName == "movechar" )
	{
		charpos_.x = Tag.attribute( "x" ).toInt();
		charpos_.y = Tag.attribute( "y" ).toInt();
		charpos_.z = Tag.attribute( "z" ).toInt();
	}

	// <itemsdecay />
	else if( TagName == "itemsdecay" )
		this->itemsdecay_ = true;

	// <houseitem>
	//     <inherit id="houseitem_a" />
	// </houseitem>
	else if( TagName == "houseitem" )
	{
		cHouseItem* phi = new cHouseItem;
		if( phi != NULL )
		{
			cItemsManager::getInstance()->registerItem( phi );
			phi->SetOwnSerial( this->ownserial );
			phi->SetMultiSerial( this->serial );
			phi->applyDefinition( Tag );

			if (phi->isInWorld()) 
				mapRegions->Add(phi);  //add to mapRegions
		}
	}

	// <deed>deedsection</deed> (any item section)
	else if( TagName == "deed" )
		this->deedsection_ = Value;

	// <nokey />
	else if( TagName == "nokey" )
		nokey_ = true;

	// <name>balbalab</name>
	else if( TagName == "name" )
		this->setName( Value );

	// <lockdownamount>3</lockdownamount>
	else if( TagName == "lockdownamount" )
		this->lockdownamount_ = Value.toInt();

	// <secureamount>5</secureamount>
	else if( TagName == "secureamount" )
		this->secureamount_ = Value.toInt();
}

void cHouse::build( const QDomElement &Tag, UI16 posx, UI16 posy, SI08 posz, SERIAL senderserial, SERIAL deedserial )
{
	P_CHAR pc_currchar = FindCharBySerial( senderserial );
	UOXSOCKET s = calcSocketFromChar( pc_currchar );

	this->applyDefinition( Tag );
	this->MoveTo( posx, posy, posz );
	if( !this->onValidPlace() )
	{
		if( s != -1 )
			sysmessage(s,"Can not build a house at that location!");
		cItemsManager::getInstance()->unregisterItem( this );
		cItemsManager::getInstance()->deleteItem( this );
		return;
	}
	this->SetOwnSerial( senderserial );
	this->priv = 0;
	this->setName( QString( "%1's house" ).arg( pc_currchar->name.c_str() ) );
	RefreshItem( this );

	P_ITEM pDeed = FindItemBySerial( deedserial );
	if( pDeed != NULL )
		Items->DeleItem( pDeed );

	if( !this->nokey_ )
	{
		P_ITEM pKey = Items->SpawnItem(s, pc_currchar, 1, "a house key", 0, 0x10, 0x0F, 0, 1,1);
		
		pKey->tags.set( "house_serial", this->serial );
		pKey->setType( 7 );
		pKey->priv=2;
        
		P_ITEM pKey2 = Items->SpawnItem(s, pc_currchar, 1, "a house key", 0, 0x10, 0x0F, 0,1,1);
		P_ITEM bankbox = pc_currchar->getBankBox();
		pKey2->tags.set( "house_serial", this->serial );
		pKey2->setType( 7 );
		pKey2->priv=2;
		bankbox->AddItem(pKey2);
	}

	cRegion::RegionIterator4Items ri(this->pos);
	for(ri.Begin(); !ri.atEnd(); ri++)
	{
		P_ITEM si = ri.GetData();
		sendinrange(si);
	}
		
	pc_currchar->MoveTo( pc_currchar->pos.x + charpos_.x, pc_currchar->pos.y + charpos_.y, pc_currchar->pos.z + charpos_.z );
}

void cHouse::removeKeys( void )
{
	AllItemsIterator iter_items;
	for( iter_items.Begin(); !iter_items.atEnd(); ++iter_items )
	{
		P_ITEM pi = iter_items.GetData();
		if( pi->type() == 7 && pi->tags.get( "house" ).isValid() && pi->tags.get( "house" ).toUInt() == this->serial )
			Items->DeleItem( pi );
	}
}

void cHouse::remove( void )
{
	this->removeKeys();

	cRegion::RegionIterator4Chars ri(this->pos);
	for (ri.Begin(); !ri.atEnd(); ri++)
	{
		P_CHAR pc = ri.GetData();
		if(pc->npcaitype() == 17 && pc->multis == this->serial)
			Npcs->DeleteChar(pc);
	}
	cRegion::RegionIterator4Items rii(this->pos);
	for(rii.Begin(); !rii.atEnd(); ri++)
	{
		P_ITEM pi = rii.GetData();
		if(pi->multis == this->serial && pi->type() != 202)
			Items->DeleItem(pi);
	}
}

#pragma note( "check char stuff BEFORE and AFTER this method is called!" )
P_ITEM cHouse::toDeed( UOXSOCKET s )
{
	//sprintf((char*)temp, "Demolishing %s", pHouse->name().ascii() );
	//sysmessage( s, (char*)temp );
	//...
	//pc->pos.z = pc->dispz = Map->MapElevation(pc->pos);
	//teleport(pc);

	P_CHAR pc_currchar = currchar[ s ];

	cRegion::RegionIterator4Chars ri(this->pos);
	for (ri.Begin(); !ri.atEnd(); ri++)
	{
		P_CHAR pc = ri.GetData();
		if(pc->npcaitype() == 17 && pc->multis == this->serial)
		{
			P_ITEM pPvDeed = Items->SpawnItem(pc_currchar, 1, (char*)QString("A vendor deed for %1").arg( pc->name.c_str() ).latin1(), 0, 0x14F0, 0, true);
			pPvDeed->setType( 217 );
			pPvDeed->value = 2000;
			RefreshItem( pPvDeed );
			sysmessage(s, QString("Packed up vendor %1.").arg(pc->name.c_str()) );
		}
	}

	this->removeKeys();
	this->remove();

	
	if( this->deedsection_.isNull() || this->deedsection_.isEmpty() )
		return NULL;
	P_ITEM pDeed = Items->createScriptItem( this->deedsection_ );
	if( pDeed == NULL ) 
		return NULL;
	else
	{
		pDeed->setContSerial( pc_currchar->packitem );
		RefreshItem( pDeed );
	}

	return pDeed;
}

bool cHouse::isBanned(P_CHAR pc)
{
	return binary_search(bans.begin(), bans.end(), pc->serial);
}

bool cHouse::isFriend(P_CHAR pc)
{
	return binary_search(friends.begin(), friends.end(), pc->serial);
}

void cHouse::addBan(P_CHAR pc)
{
	bans.push_back(pc->serial);
	sort(bans.begin(), bans.end());
}

void cHouse::addFriend(P_CHAR pc)
{	
	friends.push_back(pc->serial);
	sort(friends.begin(), friends.end());
}

void cHouse::removeBan(P_CHAR pc)
{
	vector<SERIAL>::iterator it = find(bans.begin(), bans.end(), pc->serial);
	bans.erase(it);
}

void cHouse::removeFriend(P_CHAR pc)
{
	vector<SERIAL>::iterator it = find(friends.begin(), friends.end(), pc->serial);
	friends.erase(it);
}

void cHouse::Serialize(ISerialization &archive)
{
	if (archive.isReading())
	{
		archive.read( "deed.id", deedsection_ );
		archive.read( "lockdownamt", lockdownamount_ );
		archive.read( "secureamt", secureamount_ );
		archive.read( "nokey", nokey_ );
		archive.read( "space.x", space_.x );
		archive.read( "space.y", space_.y );
		archive.read( "charpos.x", charpos_.x );
		archive.read( "charpos.y", charpos_.y );
		archive.read( "charpos.z", charpos_.z );

		unsigned int amount = 0;
		register unsigned int i;
		SERIAL readData;
		archive.read("banamount", amount);
		for (i = 0; i < amount; ++i)
		{
			archive.read("ban", readData);
			bans.push_back(readData);			
		}
		archive.read("friendamount", amount);
		for (i = 0; i < amount; ++i)
		{
			archive.read("friend", readData);
			friends.push_back(readData);
		}
	}
	else if ( archive.isWritting())
	{
		archive.write( "deed.id", deedsection_ );
		archive.write( "lockdownamt", lockdownamount_ );
		archive.write( "secureamt", secureamount_ );
		archive.write( "nokey", nokey_ );
		archive.write( "space.x", space_.x );
		archive.write( "space.y", space_.y );
		archive.write( "charpos.x", charpos_.x );
		archive.write( "charpos.y", charpos_.y );
		archive.write( "charpos.z", charpos_.z );

		register unsigned int i;
		archive.write("banamount", bans.size());
		for ( i = 0; i < bans.size(); ++i )
			archive.write("ban", bans[i]);
		archive.write("friendamount", friends.size());
		for ( i = 0; i < friends.size(); ++i )
			archive.write("friend", friends[i]);
	}
	cItem::Serialize(archive); // Call base class method too.
}

QString cHouse::objectID() const
{
	return "HOUSE";
}

