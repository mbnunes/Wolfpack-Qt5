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

// rewritten, 14.06.2002, sereg
#include "boats.h"
#include "network.h"
#include "regions.h"
#include "mapstuff.h"
#include "classes.h" // only for the illegal_z!

#undef DBGFILE
#define DBGFILE "boat.cpp" 

cBoat::cBoat()
{
	// default special-item ids!
	unsigned short itemids_[6][4] =
{(unsigned short)0x3EB1,(unsigned short)0x3ED5,(unsigned short)0x3EB2,(unsigned short)0x3ED4,(unsigned short)0x3EAE,(unsigned short)0x3E4E,
 (unsigned short)0x3E8A,(unsigned short)0x3E89,(unsigned short)0x3E85,(unsigned short)0x3E84,(unsigned short)0x3E65,(unsigned short)0x3E53,
 (unsigned short)0x3EB2,(unsigned short)0x3ED4,(unsigned short)0x3EB1,(unsigned short)0x3ED5,(unsigned short)0x3EB9,(unsigned short)0x3E4B,
 (unsigned short)0x3E85,(unsigned short)0x3E84,(unsigned short)0x3E8A,(unsigned short)0x3E89,(unsigned short)0x3E93,(unsigned short)0x3E50 };

	memset( this->itemoffsets, 0, sizeof( this->itemoffsets ) );
	memcpy( &itemids_, &(this->itemids), sizeof( itemids_ ) );
}


void cBoat::build( const QDomElement &Tag, UI16 posx, UI16 posy, SI08 posz, SERIAL senderserial, SERIAL deedserial )
{
	P_CHAR pc_currchar = FindCharBySerial( senderserial );
	UOXSOCKET s = calcSocketFromChar( pc_currchar );
	P_ITEM pdeed = FindItemBySerial( deedserial );
	UI08 siproblem = 0;

	if( s != -1 )
		Xsend( s, wppause, 2 ); // pause the client till sending

	this->type_ = 117;//Boat type
	this->name_ = "a mast";//Name is something other than "%s's house"
	this->boatdir = 0; // starting with north boatdirection

	this->applyDefinition( Tag );
	if( this->multiids_.size() < 4 || !this->isValidPlace( posx, posy, posz, 0 ) )
	{
		if( s != -1 )
			sysmessage( s, "Can not build boat at this location!" );
		cItemsManager::getInstance()->unregisterItem( this );
		cItemsManager::getInstance()->deleteItem( this );
		return;
	}

	this->pos.z = Map->MapElevation( Coord_cl( pos.x, pos.y, pos.z ) );

	for( int i = 0; i < 4; i++ )
		for( int j = 0; j < 6; j++ )
			if( this->itemids[i][j] == 0 )
				siproblem = 1;

	P_ITEM pTiller = Items->SpawnItem( pc_currchar, 1, "a tiller man", 0, this->itemids[0][ TILLER_ID ], 0, 0 );
	if( !pTiller )
		siproblem = 1;
	else
	{
		pTiller->pos.x = this->pos.x + itemoffsets[0][ TILLER ][X];
		pTiller->pos.y = this->pos.y + itemoffsets[0][ TILLER ][Y];
		pTiller->pos.z = this->pos.z;
		pTiller->priv = 0;
		pTiller->setType( 117 );
		pTiller->setType2( 1 );
		pTiller->tags.set( "boatserial", this->serial );
		this->itemserials[ TILLER ] = pTiller->serial;
	}
		
	P_ITEM pPlankR = Items->SpawnItem( pc_currchar, 1, "#", 0, this->itemids[0][ PORT_P_C ], 0, 0 );
	if( !pPlankR ) 
		siproblem = 1;
	else
	{
		pPlankR->setType( 117 );
		pPlankR->setType2( 2 );
		pPlankR->pos.x = this->pos.x + itemoffsets[0][ PORT_PLANK ][X];
		pPlankR->pos.y = this->pos.y + itemoffsets[0][ PORT_PLANK ][Y];
		pPlankR->pos.z=this->pos.z;
		pPlankR->priv=0;//Nodecay
		pPlankR->tags.set( "boatserial", this->serial );
		this->itemserials[ PORT_PLANK ] = pPlankR->serial;
	}

	P_ITEM pPlankL = Items->SpawnItem( pc_currchar, 1, "#", 0, this->itemids[0][ STAR_P_C ], 0, 0 );//Plank1 is on the LEFT side of the boat
	if( !pPlankL )
		siproblem = 1;
	else
	{
		pPlankL->setType( 117 );//Boat type
		pPlankL->setType2( 3 );//Plank sub type
		pPlankL->pos.x = this->pos.x + itemoffsets[0][ STARB_PLANK ][X];
		pPlankL->pos.y = this->pos.y + itemoffsets[0][ STARB_PLANK ][Y];
		pPlankL->pos.z=this->pos.z;
		pPlankL->priv=0;
		pPlankL->tags.set( "boatserial", this->serial );
		this->itemserials[ STARB_PLANK ] = pPlankL->serial;
	}

	P_ITEM pHold = Items->SpawnItem( pc_currchar, 1, "#", 0, this->itemids[0][ HOLD_ID ], 0, 0 );
	if( !pHold )
		siproblem = 1;
	else
	{
		pHold->setType( 1 );//Conatiner
		pHold->pos.x = this->pos.x + itemoffsets[0][ HOLD ][X];
		pHold->pos.y = this->pos.y + itemoffsets[0][ HOLD ][Y];
		pHold->pos.z=this->pos.z;
		pHold->priv=0;
		pHold->tags.set( "boatserial", this->serial );
		this->itemserials[ HOLD ] = pHold->serial;
	}

	if( siproblem > 0 )
	{
		if( s != -1 )
			sysmessage( s, "Can not build boat without itemid definitions for special items!" );
		cItemsManager::getInstance()->unregisterItem( this );
		cItemsManager::getInstance()->deleteItem( this );
		Items->DeleItem( pTiller );
		Items->DeleItem( pPlankL );
		Items->DeleItem( pPlankR );
		Items->DeleItem( pHold );
		return;
	}

	P_ITEM pKey = Items->SpawnItem(s, pc_currchar, 1, "a boat key", 0, 0x10, 0x0F, 0, 1,1);
		
	pKey->tags.set( "boatserial", this->serial );
	pKey->setType( 7 );
	pKey->priv=2;
        
	P_ITEM pKey2 = Items->SpawnItem(s, pc_currchar, 1, "a boat key", 0, 0x10, 0x0F, 0,1,1);
	P_ITEM bankbox = pc_currchar->getBankBox();
	pKey2->tags.set( "boatserial", this->serial );
	pKey2->setType( 7 );
	pKey2->priv=2;
	bankbox->AddItem(pKey2);

	this->autosail_ = 0;	// khpae : not moving 0, 1-8 : moving boatdirection+1

	if( s != -1 )
		Xsend( s, restart, 2 ); // resume the client
	
	mapRegions->Add(pTiller);//Make sure everything is in da regions!
	mapRegions->Add(pPlankL);
	mapRegions->Add(pPlankR);
	mapRegions->Add(pHold);
	
    pc_currchar->SetMultiSerial(this->serial);
	this->SetOwnSerial( pc_currchar->serial );
}

void cBoat::processNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	QString Value = this->getNodeValue( Tag );

	// <ids north="0x4021" east=".." south=".." west="..">16572</ids>
	if( TagName == "ids" )
	{
		this->multiids_.push_back( hex2dec(Tag.attribute( "north" )).toUShort() );
		this->multiids_.push_back( hex2dec(Tag.attribute( "east" )).toUShort() );
		this->multiids_.push_back( hex2dec(Tag.attribute( "south" )).toUShort() );
		this->multiids_.push_back( hex2dec(Tag.attribute( "west" )).toUShort() );
	}

	// <special_items>
	//		<tillerman>
	//			<ids north="0x3E4B" east="..." south="..." west="..." />
	//			<offsets>
	//				<x north=".." east=".." .. />
	//				<y ... />
	//			</offsets>
	//		</tillerman>
	//		<hold>
	//			see above...
	//		</hold>
	//		<planks>
	//			<port_closed>
	//				see above...
	//			</port_closed>
	//			<port_opened>
	//				see above..
	//			</port_opended>
	//			<star_closed>
	//				...
	//			</star_closed>
	//			<star_opened>
	//				...
	//			</star_opened>
	//		</planks>
	// </special_items>
	else if( TagName == "special_items" )
	{
		QDomNode childNode = Tag.firstChild();
		while( !childNode.isNull() )
		{
			if( childNode.isElement() )
			{
				UI08 item = 0xFF;
				QDomElement childTag = childNode.toElement();
				if( childTag.nodeName() == "tillerman" )
				{
					item = TILLER_ID;
				}
				else if( childTag.nodeName() == "hold" )
				{
					item = HOLD_ID;
				}
				else if( childTag.nodeName() == "planks" )
				{
					QDomNode chchildNode = childTag.firstChild();
					while( !chchildNode.isNull() )
					{
						item = 0xFF;
						if( chchildNode.isElement() )
						{
							QDomElement chchildTag = chchildNode.toElement();
							if( chchildTag.nodeName() == "port_closed" )
								item = PORT_P_C;
							else if( chchildTag.nodeName() == "port_opened" )
								item = PORT_P_O;
							else if( chchildTag.nodeName() == "star_closed" )
								item = STAR_P_C;
							else if( chchildTag.nodeName() == "star_opened" )
								item = STAR_P_O;
						}
						if( item < 0xFF )
						{
							QDomNode chchchildNode = chchildNode.firstChild();
							while( !chchchildNode.isNull() )
							{
								if( chchchildNode.isElement() )
									this->processSpecialItemNode( chchchildNode.toElement(), item );
								chchchildNode = chchchildNode.nextSibling();
							}
						}
						chchildNode = chchildNode.nextSibling();
					}
					item = 0xFF;
				}

				if( item < 0xFF )
				{
					QDomNode chchildNode = childTag.firstChild();
					while( !chchildNode.isNull() )
					{
						if( chchildNode.isElement() )
							this->processSpecialItemNode( chchildNode.toElement(), item );
						chchildNode = chchildNode.nextSibling();
					}
				}
			}
			childNode = childNode.nextSibling();
		}
	}



	// <deed>deedsection</deed> (any item section)
	else if( TagName == "deed" )
		this->deedsection_ = Value;

	// <name>balbalab</name>
	else if( TagName == "name" )
		this->setName( Value );
}

void cBoat::processSpecialItemNode( const QDomElement &Tag, UI08 item )
{
	QString TagName = Tag.nodeName();

	if( TagName == "ids" )
	{
		this->itemids[0][ item ] = hex2dec(Tag.attribute( "north" )).toUShort();
		this->itemids[1][ item ] = hex2dec(Tag.attribute( "east" )).toUShort();
		this->itemids[2][ item ] = hex2dec(Tag.attribute( "south" )).toUShort();
		this->itemids[3][ item ] = hex2dec(Tag.attribute( "west" )).toUShort();
	}

	else if( TagName == "offsets" )
	{
		if( item == 1 )
			item = PORT_PLANK;
		else if( item == 2 || item == 3 )
			item = STARB_PLANK;
		else if( item == 4 )
			item = HOLD;
		else if( item == 5 )
			item = TILLER;
	
		QDomNode childNode = Tag.firstChild();
		while( !childNode.isNull() )
		{
			if( childNode.isElement() )
			{
				UI08 coord = 0xFF;
				QDomElement childTag = childNode.toElement();
				if( childTag.nodeName() == "x" )
				{
					coord = X;
				}
				else if( childTag.nodeName() == "y" )
				{
					coord = Y;
				}
				
				if( coord < 0xFF )
				{
					this->itemoffsets[0][ item ][ coord ] = childTag.attribute( "north" ).toUShort();
					this->itemoffsets[1][ item ][ coord ] = childTag.attribute( "east" ).toUShort();
					this->itemoffsets[2][ item ][ coord ] = childTag.attribute( "south" ).toUShort();
					this->itemoffsets[3][ item ][ coord ] = childTag.attribute( "west" ).toUShort();
				}
			}
			childNode = childNode.nextSibling();
		}
	}
}

bool cBoat::isValidPlace( UI16 posx, UI16 posy, SI08 posz, UI08 boatdir )
{
	UI32 multiid = this->multiids_[ ( boatdir + 1 ) / 2 - 1 ] - 0x4000;

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

	map_st map;
	land_st land;
	tile_st tile;
	for( j = 0; j < length; j++ )
	{
		mfile->get_st_multi(&multi);
		map = Map->SeekMap( Coord_cl( multi.x + posx, multi.y + posy, pos.z, pos.map ) );
		Map->SeekLand (map.id, &land);
		if (!(land.flag1 & 0x80))
			return true;
		
		MapStaticIterator msi( Coord_cl( multi.x + posx, multi.y + posy, pos.z, pos.map ) );
		staticrecord *stat = msi.Next();
		while( stat != NULL )
		{
			msi.GetTile( &tile );
			if( !(tile.flag1 & 0x80) && ( (stat->zoff + tile.height) <= 70 ) )
				return true;
/*			else if( strcmp( (char*)tile.name, "water" ) != 0 )
				return true; ???????????? non-english users ?????*/
			stat = msi.Next();
		}
		
		cRegion::RegionIterator4Items ri( Coord_cl( multi.x + posx, multi.y + posy, pos.z, pos.map ) );
		for( ri.Begin(); !ri.atEnd(); ri++ ) 
		{
			P_ITEM pi = ri.GetData();
			if( ( pi != NULL ) && ( pi->pos.x == (multi.x + posx) ) && ( pi->pos.y == (multi.y + posy) ) ) 
				return true;
		}
	}
	return false;
}

void cBoat::turn( SI08 turn )
{
	P_ITEM pTiller = FindItemBySerial( this->itemserials[ TILLER ] );
	if( pTiller == NULL )
		return;
	
	P_ITEM pPortplank = FindItemBySerial( this->itemserials[ PORT_PLANK ] );
	if( pPortplank == NULL ) 
		return;

	P_ITEM pStarplank = FindItemBySerial( this->itemserials[ STARB_PLANK ] );
	if( pStarplank == NULL ) 
		return;

	P_ITEM pHold = FindItemBySerial( this->itemserials[ HOLD ] );
	if( pHold == NULL ) 
		return;

	QString errormsg = (char*)0;
	UI08 newboatdir = 0;

	if( turn == 0 )
		return;
	else if( turn > 0 ) // clockwise
	{
		if( this->boatdir >= 6 )
			newboatdir = 0;
		else
			newboatdir+=2;
	}
	else if( turn < 0 ) 
	{
		if( this->boatdir <= 1 )
			newboatdir = 7;
		else
			newboatdir-=2;
	}

	if( !this->isValidPlace( pos.x, pos.y, pos.z, newboatdir ) )
		errormsg = "Arr, something's in the way!";

	// first pause all clients in visrange
	vector< UOXSOCKET > socketsinrange; // sockets of the chars in visrange
	vector< UOXSOCKET >::iterator iter_sock;

	cRegion::RegionIterator4Chars ri( pos );
	for( ri.Begin(); !ri.atEnd(); ri++ ) 
	{
		P_CHAR pc = ri.GetData();
		if( pc != NULL ) 
		{
			UOXSOCKET s = calcSocketFromChar( pc );
			if( s != -1 )
			{
				if( errormsg.isNull() || errormsg.isEmpty() )
				{
					Xsend( s, wppause, 2 );
					socketsinrange.push_back( s );
				}
				else
					itemtalk( s, pTiller, (char*)errormsg.latin1() );
			}
		}
	}

	if( !errormsg.isNull() || !errormsg.isEmpty() )
		return;

	UI08 shortboatdir = ( ( newboatdir + 1 ) / 2 ) - 1;
	this->id_ = this->multiids_[ shortboatdir ];
	this->boatdir = newboatdir;
	
	// turn all items and chars on the boat and send them
	vector< SERIAL > vecEntries = imultisp.getData( this->serial );
	vector< SERIAL >::iterator it = vecEntries.begin();
	while( it != vecEntries.end() )
	{
		SI08 dx = 0, dy = 0;
		P_CHAR pc = FindCharBySerial( *it );
		if( pc != NULL )
		{
			mapRegions->Remove( pc );
			dx = pc->pos.x - this->pos.x;
			dy = pc->pos.y - this->pos.y;
			if( turn > 0 )
			{
				pc->pos.x += dy * (-1);
				pc->pos.y += dx;
			}
			else
			{
				pc->pos.x += dy;
				pc->pos.y += dx * (-1);
			}
			mapRegions->Add( pc );

			extmove[0]=0x77;
			LongToCharPtr(pc->serial, &extmove[1]);
			ShortToCharPtr(pc->id(),  &extmove[5]);
			extmove[7]=pc->pos.x>>8;
			extmove[8]=pc->pos.x%256;
			extmove[9]=pc->pos.y>>8;
			extmove[10]=pc->pos.y%256;
			extmove[11]=pc->pos.z;
			extmove[12]=pc->dir|0x80;

			extmove[13]=0;
			extmove[14]=0;

			extmove[15]=0;
			extmove[16]=0;

			iter_sock = socketsinrange.begin();
			while( iter_sock != socketsinrange.end() )
			{
				Xsend( *iter_sock, extmove, 17 );
				iter_sock++;
			}
		}
		else
		{
			P_ITEM pi = FindItemBySerial( *it );
			if( pi != NULL )
			{
				mapRegions->Remove( pi );
				dx = pi->pos.x - this->pos.x;
				dy = pi->pos.y - this->pos.y;

				if( turn > 0 )
				{
					pi->pos.x += dy * (-1);
					pi->pos.y += dx;
				}
				else
				{
					pi->pos.x += dy;
					pi->pos.y += dx * (-1);
				}
				mapRegions->Add( pi );

				iter_sock = socketsinrange.begin();
				while( iter_sock != socketsinrange.end() )
				{
					senditem( *iter_sock, pi );
					iter_sock++;
				}
			}
		}
		it++;
	}

	// change positions and ids of the special items
	pPortplank->MoveTo( pos.x + itemoffsets[shortboatdir][PORT_PLANK][X],
		pos.y + itemoffsets[shortboatdir][PORT_PLANK][Y],
		pPortplank->pos.z );
	pPortplank->setId( itemids[shortboatdir][PORT_P_C] );
	
	pStarplank->MoveTo( pos.x + itemoffsets[shortboatdir][STARB_PLANK][X],
		pos.y + itemoffsets[shortboatdir][STARB_PLANK][Y],
		pStarplank->pos.z );
	pStarplank->setId( itemids[shortboatdir][STAR_P_C] );
	
	pTiller->MoveTo( pos.x + itemoffsets[shortboatdir][TILLER][X],
		pos.y + itemoffsets[shortboatdir][TILLER_ID][Y],
		pTiller->pos.z );
	pTiller->setId( itemids[shortboatdir][TILLER] );
	
	pHold->MoveTo( pos.x + itemoffsets[shortboatdir][HOLD][X],
		pos.y + itemoffsets[shortboatdir][HOLD][Y],
		pHold->pos.z );
	pHold->setId( itemids[shortboatdir][HOLD_ID] );
	
	iter_sock = socketsinrange.begin();
	while( iter_sock != socketsinrange.end() )
	{
		senditem( *iter_sock, pPortplank );
		senditem( *iter_sock, pStarplank );
		senditem( *iter_sock, pTiller );
		senditem( *iter_sock, pHold );
		senditem( *iter_sock, this );
		Xsend( *iter_sock, restart, 2 );
		iter_sock++;
	}
}

bool cBoat::move( void )
{
	if( this->autosail_ > 0 )
	{
		// do autosail stuff HERE, that means find out the new boatdirection, turn the boat if necissary and change the mappin stuff
	}

	SI08 dx=0,dy=0;
	QString errormsg = (char*)0;

	P_ITEM pTiller = FindItemBySerial( this->itemserials[ TILLER ] );
	if( pTiller == NULL )
		return false;
	
	P_ITEM pPortplank = FindItemBySerial( this->itemserials[ PORT_PLANK ] );
	if( pPortplank == NULL ) 
		return false;

	P_ITEM pStarplank = FindItemBySerial( this->itemserials[ STARB_PLANK ] );
	if( pStarplank == NULL ) 
		return false;

	P_ITEM pHold = FindItemBySerial( this->itemserials[ HOLD ] );
	if( pHold == NULL ) 
		return false;

	//Xsend(s,wppause,2);

	switch( this->boatdir )
	{
	case 0: 
		dy--;
		break;
	case 2:
		dx++;
		break;
	case 4: 
		dy++;
		break;
	case 6: 
		dx--;
		break;
	default:
		clConsole.log( QString( "WARNING: cBoat::Move: invalid boatdirection caught (boatdir: %1, serial: %2), corrected to north boatdir!").arg(this->boatdir).arg(this->serial).latin1() );
		this->boatdir = 0;
		dy--;
		break;
	}

	if( ( this->pos.x+dx<=200 || this->pos.x+dx>=6000) && (this->pos.y+dy<=200 || this->pos.y+dy>=4900)) //bugfix LB
	{
		this->setType2( 9 );
		errormsg = "Arr, Sir, we've hit rough waters!";
	}
	else if( !this->isValidPlace( pos.x+dx, pos.y+dy, pos.z, boatdir ) )
	{
		this->setType2( 9 );
		errormsg = "Arr, somethings in the way!";
	}

	// Move all the special items
	// first pause all clients in visrange
	vector< UOXSOCKET > socketsinrange; // sockets of the chars in visrange
	vector< UOXSOCKET >::iterator iter_sock;
	vector< SERIAL > vecEntries = imultisp.getData( this->serial );
	vector< SERIAL >::iterator it;

	cRegion::RegionIterator4Chars ri( pos );
	for( ri.Begin(); !ri.atEnd(); ri++ ) 
	{
		P_CHAR pc = ri.GetData();
		if( pc != NULL ) 
		{
			UOXSOCKET s = calcSocketFromChar( pc );
			if( s != -1 )
			{
				if( errormsg.isNull() && errormsg.isEmpty() )
				{
					Xsend( s, wppause, 2 );
					socketsinrange.push_back( s );
				}
				else
					itemtalk( s, pTiller, (char*)errormsg.latin1() );
			}
		}
	}

	if( !errormsg.isNull() || !errormsg.isEmpty() )
		return false;

	Coord_cl desloc( dx, dy, 0 );
	this->moveTo( pos + desloc );
	pTiller->moveTo( pTiller->pos + desloc );
	pPortplank->moveTo( pPortplank->pos + desloc );
	pStarplank->moveTo( pStarplank->pos + desloc );
	pHold->moveTo( pHold->pos + desloc );

	it = vecEntries.begin();
	while( it != vecEntries.end() )
	{
		P_ITEM pi = FindItemBySerial( *it );
		if( pi != NULL )
		{
			pi->MoveTo( pi->pos.x + dx, pi->pos.y + dy, pi->pos.z );

			iter_sock = socketsinrange.begin();
			while( iter_sock != socketsinrange.end() )
			{
				senditem( *iter_sock, pi );
				iter_sock++;
			}
		}
		else
		{
			P_CHAR pc = FindCharBySerial( *it );
			if( pc != NULL )
			{
				pc->moveTo( pc->pos + desloc );

				extmove[0]=0x77;
				LongToCharPtr(pc->serial, &extmove[1]);
				ShortToCharPtr(pc->id(),  &extmove[5]);
				extmove[7]=pc->pos.x>>8;
				extmove[8]=pc->pos.x%256;
				extmove[9]=pc->pos.y>>8;
				extmove[10]=pc->pos.y%256;
				extmove[11]=pc->pos.z;
				extmove[12]=pc->dir|0x80;

				extmove[13]=0;
				extmove[14]=0;

				extmove[15]=0;
				extmove[16]=0;

				iter_sock = socketsinrange.begin();
				while( iter_sock != socketsinrange.end() )
				{
					Xsend( *iter_sock, extmove, 17 );
					iter_sock++;
				}
			}
		}
		it++;
	}

	iter_sock = socketsinrange.begin();
	while( iter_sock != socketsinrange.end() )
	{
		senditem( *iter_sock, pPortplank );
		senditem( *iter_sock, pStarplank );
		senditem( *iter_sock, pTiller );
		senditem( *iter_sock, pHold );
		senditem( *iter_sock, this );
		Xsend( *iter_sock, restart, 2 );
		iter_sock++;
	}

	return true;
}

void cBoat::handlePlankClick( UOXSOCKET s, P_ITEM pplank )
{
	P_CHAR pc_currchar = currchar[s];
	if( pc_currchar == NULL )
		return;

	bool charonboat = false;

	vector< SERIAL > vecEntries = imultisp.getData( this->serial );
	vector< SERIAL >::iterator it = vecEntries.begin();
	while( it != vecEntries.end() )
	{
		if( *it == pc_currchar->serial )
			charonboat = true;
		it++;
	}

	if( !charonboat )
	{
		vector< SERIAL > vecCown = cownsp.getData( pc_currchar->serial );
		it = vecCown.begin();
		while( it != vecCown.end() )
		{
			P_CHAR pc = FindCharBySerial( *it );
			if( pc != NULL )
			{
				if( pc->isNpc() && pc_currchar->Owns( pc ) )
				{
					pc->moveTo( pos + Coord_cl( 1, 1, 2 ) );
					pc->SetMultiSerial( this->serial );
					teleport( pc );
				}
			}
			it++;
		}

		// khpae
		UI16 x, y;
		SI08 z;
		switch( this->boatdir ) 
		{
		case 0:
		case 4:
			x = ( pos.x + pplank->pos.x ) / 2;
			y = ( pos.y + pplank->pos.y ) / 2;
			break;
		case 2:
		case 6:
			x = ( pos.x + pplank->pos.x ) / 2;
			y = ( pos.y + pplank->pos.y ) / 2;
			break;
		default:
			return;
		}
		
		z = pos.z + 3;
		pc_currchar->MoveTo( x, y, z );
		teleport( pc_currchar );
		pc_currchar->SetMultiSerial( this->serial ); // set chars->multis value
		sysmessage( s, "You entered a boat" );
	}
	else
	{
		if( leave( s, pplank ) ) 
		{
			sysmessage( s, "You left the boat." );
		} 
		else 
		{
			sysmessage( s, "You cannot get off here!" );
		}
	}
}

bool cBoat::leave( UOXSOCKET s, P_ITEM pplank )
{
	P_CHAR pc_currchar = currchar[s];
	if( pc_currchar == NULL )
	{
		return false;
	}

	UI16 x, y, x0, y0, x1, y1, dx, dy;
	switch( this->boatdir ) 
	{
	case 0:
	case 4:
		x0 = ( pos.x > pplank->pos.x ) ? pplank->pos.x - 1 : pplank->pos.x + 1;
		y0 = pplank->pos.y - 2;
		x1 = ( pos.x > pplank->pos.x ) ? pplank->pos.x - 2 : pplank->pos.x + 2;
		y1 = y0 + 5;
		break;
	case 2:
	case 6:
		x0 = pplank->pos.x - 2;
		y0 = ( pos.y > pplank->pos.y ) ? pplank->pos.y - 1 : pplank->pos.y + 1;
		x1 = x0 + 5;
		y1 = ( pos.y > pplank->pos.y ) ? pplank->pos.y - 2 : pplank->pos.y + 2;
		break;
	default:
		return false;
	}
	UI16 tmp;
	if (x0 > x1) 
	{
		tmp = x0;
		x0 = x1;
		x1 = tmp;
	}
	if (y0 > y1) 
	{
		tmp = y0;
		y0 = y1;
		y1 = tmp;
	}
	signed char sz, mz, z;
	bool check = false;
	land_st landt;
	map_st mapt;
	tile_st tilet;
	int loopexit = 0;
	for( x = x0; x <= x1; x++ ) 
	{
		for( y = y0; y <= y1; y++) 
		{
			sz = Map->StaticTop( Coord_cl( x, y, 0, 0) );
			mz = Map->MapElevation( Coord_cl( x, y, 0, 0) );
			if( (sz == illegal_z) && (mz != -5) ) 
			{
				z = mz;
				check = true;
				break;
			} 
			else if( (sz != illegal_z) && (sz != -5) ) 
			{
				z = sz;
				check = true;
				break;
			}
		}

		if( check ) 
			break;

	}

	if( !check ) 
		return false;

	vector< SERIAL > vecCown = cownsp.getData( pc_currchar->serial );
	vector< SERIAL >::iterator it = vecCown.begin();
	while( it != vecCown.end() )
	{
		P_CHAR pc = FindCharBySerial( *it );
		if( pc != NULL )
		{
			if( pc->isNpc() && pc_currchar->Owns( pc ) && inrange1p( pc_currchar, pc ) )
			{
				pc->MoveTo( x, y, z );
				cmultisp.remove( this->serial, pc->serial );
				pc->SetMultiSerial( INVALID_SERIAL );
				teleport( pc );
			}
		}
		it++;
	}
	pc_currchar->MoveTo( x, y, z );
	teleport( pc_currchar );
	cmultisp.remove( this->serial, pc_currchar->serial );
	pc_currchar->SetMultiSerial( INVALID_SERIAL );
	return true;
}

void cBoat::switchPlankState( P_ITEM pplank ) //Open, or close the plank (called from keytarget() )
{
	if( !pplank->tags.get( "boatserial" ).isValid() || pplank->tags.get( "boatserial" ).toUInt() != this->serial )
		return;

	UI08 shortboatdir = (this->boatdir + 1) / 2 - 1;
	if( pplank->id() == this->itemids[ shortboatdir ][PORT_P_C] )
		pplank->setId( this->itemids[ shortboatdir ][PORT_P_O] ); 
	else if( pplank->id() == this->itemids[ shortboatdir ][PORT_P_O] )
		pplank->setId( this->itemids[ shortboatdir ][PORT_P_C] ); 
	else if( pplank->id() == this->itemids[ shortboatdir ][STAR_P_C] )
		pplank->setId( this->itemids[ shortboatdir ][STAR_P_O] ); 
	else if( pplank->id() == this->itemids[ shortboatdir ][STAR_P_O] )
		pplank->setId( this->itemids[ shortboatdir ][STAR_P_C] ); 
}

/*
// khpae : initial setup for auto sailing
void cBoat::setAutoSail (UOXSOCKET s, P_ITEM pMap, P_ITEM pTiller) {
	P_CHAR pc = currchar[s];
	if (pc == NULL) {
		return;
	}

	if( !pTiller->tags.get( "boatserial" ).isValid() )
		return;

	if (!pMap->mapNumPin) {
		itemtalk (s, pTiller, "Sir, there's no ship course.");
		return;
	}
	if (pc->multis == INVALID_SERIAL) {
		sysmessage (s, "You must be on the boat to do that.");
		return;
	}
	SERIAL bserial = pTiller->tags.get( "boatserial" ).toUInt();
	if (bserial != pc->multis) {
		sysmessage (s, "You must be on the boat to do that.");
		return;
	}
	int x0 = (pMap->more1<<8) | pMap->more2;
	int y0 = (pMap->more3<<8) | pMap->more4;
	int x1 = (pMap->moreb1<<8) | pMap->moreb2;
	int y1 = (pMap->moreb3<<8) | pMap->moreb4;
	int width = 134 * (pMap->morez + 1);
	int i, posx, posy;
	for (i=0; i<pMap->mapNumPin; i++) {
		posx = x0 + pMap->mapPinXY[i][0]*(x1-x0) / width;
		posy = y0 + pMap->mapPinXY[i][1]*(y1-y0) / width;
		this->mapPinXY[i][0] = (unsigned short)posx;
		this->mapPinXY[i][1] = (unsigned short)posy;
	}
	itemtalk (s, pTiller, "Aye, Sir.");
	this->autosail_ = this->boatdir + 1;
	this->mapNumPin = pMap->mapNumPin;
}
*/

char cBoat::speechInput( UOXSOCKET s, const QString& msg )//See if they said a command. msg must already be capitalized
{
	P_CHAR pc_currchar = currchar[s];
	SERIAL serial;
	char msg2[512];

	if( s == INVALID_UOXSOCKET ) 
		return 0;

	//get the tiller man's item #
	serial = this->itemserials[ TILLER ];
	if ( serial == INVALID_SERIAL ) 
		return 0;
	P_ITEM tiller = FindItemBySerial( serial );
	if ( tiller == NULL ) 
		return 0;

	// khpae - command add
	if((msg.find("ONE")!= string::npos) || (msg.find("DRIFT")!=string::npos))
	{
		if (msg.find ("FORWARD LEFT") != string::npos) {
			boatdir -= 1;
			if (boatdir < 0) {
				boatdir += 8;
			}
			if (move ()) {
				itemtalk (s, tiller, "Aye, sir.");
			}
			this->type2_ = 9;
			return 1;
		} else if (msg.find ("FORWARD RIGHT") != string::npos) {
			boatdir += 1;
			if (boatdir > 7) {
				boatdir -= 8;
			}
			if (move ()) {
				itemtalk (s, tiller, "Aye, sir.");
			}
			this->type2_ = 9;
			return 1;
		} else if (msg.find ("BACKWARD RIGHT") != string::npos) {
			boatdir += 3;
			if (boatdir > 7) {
				boatdir -= 8;
			}
			if (move ()) {
				itemtalk (s, tiller, "Aye, sir.");
			}
			this->type2_ = 9;
			return 1;
		} else if (msg.find ("BACKWARD LEFT") != string::npos) {
			boatdir += 5;
			if (boatdir > 7) {
				boatdir -= 8;
			}
			if (move ()) {
				itemtalk (s, tiller, "Aye, sir.");
			}
			this->type2_ = 9;
			return 1;
		} else
		if (msg.find ("FORWARD") != string::npos) {
			if (move ()) {
				itemtalk (s, tiller, "Aye, sir.");
			}
			this->type2_ = 9;
		} else if (msg.find ("BACKWARD") != string::npos) {
			boatdir -= 4;
			if (boatdir < 0) {
				boatdir += 8;
			}
			if (move ()) {
				itemtalk (s, tiller, "Aye, sir.");
			}
			this->type2_ = 9;
		} else 
		if(msg.find("LEFT")!=string::npos)
		{
			boatdir-=2;
			if(boatdir<0) boatdir+=8;			
			if (move()) {
				itemtalk(s, tiller, "Aye, sir.");
			}
			// khpae
			this->type2_ = 9; // stop
			return 1;

		} else if(msg.find("RIGHT")!=string::npos)
		{
			boatdir+=2;
			if(boatdir>=8) boatdir-=8; 			
			if (move()) {
				itemtalk(s, tiller, "Aye, sir.");
			}
			// khpae
			this->type2_ = 9;
			return 1;
		}
	} else if (msg.find ("FORWARD RIGHT") != string::npos) {
		boatdir = this->boatdir + 1;
		if (boatdir > 7) {
			boatdir -= 8;
		}
		if (move ()) {
			itemtalk (s, tiller, "Aye, sir.");
			this->type2_ = 2;
		} else {
			this->type2_ = 9;
		}
		return 1;
	} else if (msg.find ("FORWARD LEFT") != string::npos) {
		boatdir = this->boatdir - 1;
		if (boatdir < 0) {
			boatdir += 8;
		}
		if (move ()) {
			itemtalk (s, tiller, "Aye, sir.");
			this->type2_ = 8;
		} else {
			this->type2_ = 9;
		}
		return 1;
	} else if (msg.find ("BACKWARD RIGHT") != string::npos) {
		boatdir = this->boatdir + 3;
		if (boatdir > 7) {
			boatdir -= 8;
		}
		if (move ()) {
			itemtalk (s, tiller, "Aye, sir.");
			this->type2_ = 4;
		} else {
			this->type2_ = 9;
		}
		return 1;
	} else if (msg.find ("BACKWARD LEFT") != string::npos) {
		boatdir = this->boatdir - 3;
		if (boatdir < 0) {
			boatdir += 8;
		}
		if (move ()) {
			itemtalk (s, tiller, "Aye, sir.");
			this->type2_ = 6;
		} else {
			this->type2_ = 9;
		}
		return 1;
	} else
	if((msg.find("FORWARD")!= string::npos) || (msg.find("UNFURL SAIL")!=string::npos))
	{
		if (move()) {
			itemtalk(s, tiller, "Aye, sir.");
			this->type2_=1;//Moving : khpae - moving the same boatdirection of the boat
		} else {
			this->type2_ = 9;	// stop
		}
		return 1;
	} else if(msg.find("BACKWARD")!= string::npos)
	{
		if(boatdir >= 4) boatdir-=4;
		else boatdir+=4;
		if (move()) {
			itemtalk(s, tiller, "Aye, sir.");
			this->type2_=5;//Moving backward // khpae : changed from 2 to 5
		} else {
			this->type2_ = 9;
		}
		return 1;
	}
	else if((msg.find("STOP")!=string::npos) || (msg.find("FURL SAIL")!=string::npos))
	{ 
		this->type2_=9;
		if (this->doesAutoSail()) {
			this->autosail_ = 0;
		}
		itemtalk(s, tiller, "Aye, sir."); 
		return 1;
	}//Moving is type2 1 and 2, so stop is 0 :-)
	
	else if(((msg.find("TURN")!=string::npos) && ((msg.find("AROUND")!=string::npos) || (msg.find("LEFT")!=string::npos) || (msg.find("RIGHT")!=string::npos)))
		|| (msg.find("PORT")!=string::npos) || (msg.find("STARBOARD")!=string::npos) || (msg.find("COME ABOUT")!=string::npos))
	{
		if((msg.find("RIGHT")!=string::npos) || (msg.find("STARBOARD")!=string::npos)) 
		{
			// khpae
			boatdir+=2; if(boatdir>7) boatdir-=8;
			int tx=0,ty=0;
			turn(1);
			this->type2_ = 9;
			return 1;
		}
		else if((msg.find("LEFT")!=string::npos) || (msg.find("PORT")!=string::npos)) 
		{
			// khpae
			boatdir-=2; if(boatdir<0) boatdir+=8;
			int tx=0,ty=0;
			turn(-1);
			this->type2_ = 9;
			return 1;
		}
		else if((msg.find("COME ABOUT")!=string::npos) || (msg.find("AROUND")!=string::npos))
		{
			turn(1);
			turn(1);
			itemtalk(s, tiller, "Aye, sir.");
			return 1;
		}
	}
	else if(msg.find("SET NAME")!=string::npos)
	{
		tiller->setName( QString("a ship named %1").arg(msg2+8) );
		return 1;
	}
	// khpae - bugfix
	else if (msg.find ("LEFT") != string::npos) {
		boatdir-=2;
		if (boatdir<0) boatdir+=8;  
		if (move ()) {
			itemtalk(s, tiller, "Aye, sir.");
			this->type2_ = 7;
		} else {
			this->type2_ = 9;
		}
		return 1;
	} else if (msg.find ("RIGHT") != string::npos) {
		boatdir+=2;
		if (boatdir>=8) boatdir-=8;
		if (move ()) {
			itemtalk (s, tiller, "Aye, sir.");
			this->type2_ = 3;
		} else {
			this->type2_ = 9;
		}
		return 1;
	}
	return 0;
}

// khpae - make deed from a boat
void cBoat::toDeed( UOXSOCKET s ) {
	P_CHAR pc = currchar[s];
	if (pc == NULL) {
		return;
	}
	// if player is in boat
	if (pc->multis == this->serial) {
		sysmessage (s, "You must leave the boat to deed it.");
		return;
	}
	// check the player has the boat key
	P_ITEM bpack = Packitem (pc);
	if (bpack == NULL) {
		return;
	}
	vector<SERIAL> vpack = contsp.getData (bpack->serial);
	P_ITEM pi = NULL;
	bool found = false;
	int in;
	for (in=0; in<vpack.size (); in++) {
		pi = FindItemBySerial (vpack[in]);
		if (pi == NULL) {
			contsp.remove (bpack->serial, vpack[in]);
			continue;
		}
		if (pi->type() == 7) {
			if( pi->tags.get( "boatserial" ).isValid() )
			{
				SERIAL si = pi->tags.get( "boatserial" ).toUInt();
				if (si == this->serial) {
					found = true;
					break;
				}
			}
		}
	}
	if ((!found) || (pi==NULL)) {
		sysmessage (s, "You don't have the boat key.");
		return;
	}
	// if any pcs / npcs / items are in the boat, it cannot be deed.
	vector<SERIAL> vitem = imultisp.getData (this->serial);
	if (vitem.size () > 0) {
		sysmessage (s, "You can only deed with empty boat (remove items).");
		return;
	}
	vector<SERIAL> vchar = cmultisp.getData (this->serial);
	if (vchar.size () > 0) {
		sysmessage (s, "You can only deed with empty boat (remove pc/npcs.");
		return;
	}
	// add deed
	P_ITEM bdeed = Items->SpawnItemBackpack2 (s, this->deedsection_, 0);
	if (bdeed == NULL) {
		sysmessage (s, "There's problem with deed boat. Please contact Game Master.");
		return;
	}
	// remove key
	Items->DeleItem (pi);
	// remove all other keys for this ship
	AllItemsIterator iter_items;
	for (iter_items.Begin (); !iter_items.atEnd (); ++iter_items) {
		P_ITEM boatKey = iter_items.GetData();
		if ((boatKey->type()==7) && boatKey->tags.get( "boatserial" ).isValid() && boatKey->tags.get( "boatserial" ).toUInt() == this->serial ) {
			--iter_items;
			Items->DeleItem (boatKey);
		}
	}

	P_ITEM pTiller = FindItemBySerial( this->itemserials[ TILLER ] );
	if( pTiller != NULL )
		Items->DeleItem( pTiller );
	
	P_ITEM pPortplank = FindItemBySerial( this->itemserials[ PORT_PLANK ] );
	if( pPortplank != NULL ) 
		Items->DeleItem( pPortplank );

	P_ITEM pStarplank = FindItemBySerial( this->itemserials[ STARB_PLANK ] );
	if( pStarplank != NULL ) 
		Items->DeleItem( pStarplank );

	P_ITEM pHold = FindItemBySerial( this->itemserials[ HOLD ] );
	if( pHold != NULL ) 
		Items->DeleItem( pHold );

	Items->DeleItem (this);
	pc->SetMultiSerial( INVALID_SERIAL );
	sysmessage (s, "You deed the boat.");
}


P_ITEM findmulti(Coord_cl pos) //Sortta like getboat() only more general... use this for other multi stuff!
{
	int lastdist = 30;
	P_ITEM multi = NULL;
	int ret;

	cRegion::RegionIterator4Items ri(pos);
	
	for (ri.Begin(); !ri.atEnd(); ri++)
	{
		P_ITEM mapitem = ri.GetData();
		if (mapitem != NULL)
		{
			if (mapitem->isMulti())
			{
				ret = pos.distance(mapitem->pos);
				if (ret <= lastdist)
				{
					lastdist = ret;
					if (inmulti(pos, mapitem))
						multi = mapitem;
				}
			}
		}
	}

	return multi;
}

void sendinrange(P_ITEM pi)//Send this item to all online people in range
{//(Decided this was better than writting 1000 for loops to send an item.
	for(int a=0;a<now;a++)
	{
		if(perm[a] && iteminrange(a, pi, VISRANGE))
			senditem(a, pi);
	}
}

bool inmulti(Coord_cl pos, P_ITEM pi)//see if they are in the multi at these chords (Z is NOT checked right now)
{
	int j;
	SI32 length;			// signed long int on Intel
	st_multi multi;
	UOXFile *mfile;
	Map->SeekMulti(pi->id()-0x4000, &mfile, &length);
	length=length/sizeof(st_multi);
	if (length == -1 || length>=17000000)//Too big...
	{
		LogError( (char*)QString("inmulti() - Bad length in multi file. Avoiding stall. (Item Name: %1)\n").arg(pi->name()).latin1() ); // changed by Magius(CHE) (1)
		length = 0;
	}
	for (j=0;j<length;j++)
	{
		mfile->get_st_multi(&multi);
		if ((multi.visible)&&(pi->pos.x+multi.x == pos.x) && (pi->pos.y+multi.y == pos.y))
		{
			return true;
		}
	}
	return false;
}
