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
#include "tilecache.h"
#include "mapstuff.h"
#include "srvparams.h"
#include "network/uosocket.h"

#include "classes.h" // only for the illegal_z!

#undef DBGFILE
#define DBGFILE "boats.cpp" 

cBoat::cBoat() : cMulti()
{
	this->priv = 0;
	this->contserial = INVALID_SERIAL;
	this->boatdir = 0;
	this->autosail_ = 0;
	this->moves_ = 0;
	this->shift_ = 0;


	// default special-item ids!
	this->itemids[0][0] = this->itemids[2][2] = 0x3EB1;
	this->itemids[1][0] = this->itemids[3][2] = 0x3E8A;
	this->itemids[2][0] = this->itemids[0][2] = 0x3EB2;
	this->itemids[3][0] = this->itemids[1][2] = 0x3E85;
	
	this->itemids[0][1] = this->itemids[2][3] = 0x3ED5;
	this->itemids[1][1] = this->itemids[3][3] = 0x3E89;
	this->itemids[2][1] = this->itemids[0][3] = 0x3ED4;
	this->itemids[3][1] = this->itemids[1][3] = 0x3E84;

	this->itemids[0][4] = 0x3EAE;
	this->itemids[1][4] = 0x3E65;
	this->itemids[2][4] = 0x3EB9;
	this->itemids[3][4] = 0x3E93;

	this->itemids[0][5] = 0x3E4E;
	this->itemids[1][5] = 0x3E53;
	this->itemids[2][5] = 0x3E4B;
	this->itemids[3][5] = 0x3E50;

/*	unsigned short itemids_[6][4] =
{(unsigned short)0x3EB1,(unsigned short)0x3ED5,(unsigned short)0x3EB2,(unsigned short)0x3ED4,(unsigned short)0x3EAE,(unsigned short)0x3E4E,
 (unsigned short)0x3E8A,(unsigned short)0x3E89,(unsigned short)0x3E85,(unsigned short)0x3E84,(unsigned short)0x3E65,(unsigned short)0x3E53,
 (unsigned short)0x3EB2,(unsigned short)0x3ED4,(unsigned short)0x3EB1,(unsigned short)0x3ED5,(unsigned short)0x3EB9,(unsigned short)0x3E4B,
 (unsigned short)0x3E85,(unsigned short)0x3E84,(unsigned short)0x3E8A,(unsigned short)0x3E89,(unsigned short)0x3E93,(unsigned short)0x3E50 };

	memcpy( &itemids_, &this->itemids, 24 * sizeof( unsigned short ) );*/
	memset( this->itemoffsets, 0, 32 * sizeof( short ) );
}


void cBoat::build( const QDomElement &Tag, UI16 posx, UI16 posy, SI08 posz, SERIAL senderserial, SERIAL deedserial )
{
	P_CHAR pc_currchar = FindCharBySerial( senderserial );
	cUOSocket* socket = pc_currchar->socket();
	P_ITEM pdeed = FindItemBySerial( deedserial );
	UI08 siproblem = 0;

	if( socket )
	{
		// pause the client till sending
		cUOTxPause* uoPacket = new cUOTxPause();
		uoPacket->pause();
		socket->send( uoPacket );
	}

	this->type_ = 117;//Boat type
	this->name_ = tr("a mast");
	this->boatdir = 0; // starting with north boatdirection

	this->applyDefinition( Tag );
	this->serial = cItemsManager::getInstance()->getUnusedSerial();
	cItemsManager::getInstance()->registerItem( this );
	if( this->multiids_.size() < 4 || !this->isValidPlace( posx, posy, posz, 0 ) )
	{
		if( socket )
		{
			cUOTxPause* uoPacket = new cUOTxPause();
			uoPacket->resume();
			socket->send( uoPacket );
			socket->sysMessage( tr("Can not build boat at this location!") );
		}
		cItemsManager::getInstance()->unregisterItem( this );
		cItemsManager::getInstance()->deleteItem( this );
		return;
	}

	this->setId( multiids_[0] );
	this->pos.x = posx;
	this->pos.y = posy;
	this->pos.z = posz;

	for( int i = 0; i < 4; ++i )
	{
		for( int j = 0; j < 6; ++j )
		{
			if( this->itemids[i][j] == 0 )
			{
				siproblem = 1;
			}
		}
	}

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
		pTiller->tags.set( "tiller", 1 );
		pTiller->tags.set( "boatserial", this->serial );
		this->itemserials[ TILLER ] = pTiller->serial;
		pTiller->gatetime=(unsigned int)(uiCurrentTime + (double)(SrvParams->boatSpeed()*MY_CLOCKS_PER_SEC));
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
		pPlankR->pos.z = this->pos.z;
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
		pPlankL->pos.z = this->pos.z;
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
		pHold->pos.z = this->pos.z;
		pHold->priv=0;
		pHold->tags.set( "boatserial", this->serial );
		this->itemserials[ HOLD ] = pHold->serial;
	}

	if( siproblem > 0 )
	{
		if( socket )
		{
			cUOTxPause* uoPacket = new cUOTxPause();
			uoPacket->resume();
			socket->send( uoPacket );
			socket->sysMessage( tr("Can not build boat without itemid definitions for special items!") );
		}
		cItemsManager::getInstance()->unregisterItem( this );
		cItemsManager::getInstance()->deleteItem( this );
		Items->DeleItem( pTiller );
		Items->DeleItem( pPlankL );
		Items->DeleItem( pPlankR );
		Items->DeleItem( pHold );
		return;
	}

	P_ITEM pKey = Items->SpawnItem(-1, pc_currchar, 1, "a boat key", 0, 0x10, 0x0F, 0, 1,1);
		
	pKey->tags.set( "boatserial", this->serial );
	pKey->setType( 7 );
	pKey->priv=2;
        
	P_ITEM pKey2 = Items->SpawnItem(-1, pc_currchar, 1, "a boat key", 0, 0x10, 0x0F, 0,1,1);
	P_ITEM bankbox = pc_currchar->getBankBox();
	pKey2->tags.set( "boatserial", this->serial );
	pKey2->setType( 7 );
	pKey2->priv=2;
	bankbox->AddItem(pKey2);

	this->autosail_ = 0;	// khpae : not moving 0, 1-8 : moving boatdirection+1

	mapRegions->Add(pTiller);//Make sure everything is in da regions!
	mapRegions->Add(pPlankL);
	mapRegions->Add(pPlankR);
	mapRegions->Add(pHold);
	mapRegions->Add(this);

	pTiller->update();
	pPlankL->update();
	pPlankR->update();
	pHold->update();
	this->update();

	P_ITEM pDeed = FindItemBySerial( deedserial );
	if( pDeed != NULL )
		Items->DeleItem( pDeed );

	if( socket )
	{
		// resume client
		cUOTxPause* uoPacket = new cUOTxPause();
		uoPacket->resume();
		socket->send( uoPacket );
	}
	
//    pc_currchar->SetMultiSerial(this->serial);
	this->SetOwnSerial( pc_currchar->serial );
}

void cBoat::processNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	QString Value = this->getNodeValue( Tag );

	// <ids north="0x4021" east=".." south=".." west=".." />
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
					this->itemoffsets[0][ item ][ coord ] = childTag.attribute( "north" ).toShort();
					this->itemoffsets[1][ item ][ coord ] = childTag.attribute( "east" ).toShort();
					this->itemoffsets[2][ item ][ coord ] = childTag.attribute( "south" ).toShort();
					this->itemoffsets[3][ item ][ coord ] = childTag.attribute( "west" ).toShort();
				}
			}
			childNode = childNode.nextSibling();
		}
	}
}

bool cBoat::isValidPlace( UI16 posx, UI16 posy, SI08 posz, UI08 boatdir )
{
	UI32 multiid = this->multiids_[ boatdir / 2 ] - 0x4000;

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
	bool mapblocks = false;
	for( j = 0; j < length; ++j )
	{
		mfile->get_st_multi(&multi);
		map = Map->SeekMap( Coord_cl( multi.x + posx, multi.y + posy, pos.z, pos.map ) );
		land = cTileCache::instance()->getLand( map.id );
		MapStaticIterator msi( Coord_cl( multi.x + posx, multi.y + posy, pos.z, pos.map ) );
		mapblocks = !(land.flag1 & 0x80);

		staticrecord *stat = msi.Next();
		while( stat != NULL )
		{
			msi.GetTile( &tile );
			if( !(tile.flag1 & 0x80) && ( pos.z >= stat->zoff && pos.z <= (stat->zoff+70) ) && mapblocks )
				return false;
			if( mapblocks )
				mapblocks = false;
/*			else if( strcmp( (char*)tile.name, "water" ) != 0 )
				return true; ???????????? non-english users ?????*/
			stat = msi.Next();
		}
		if( mapblocks )
			return false;
		
		/*
		cRegion::RegionIterator4Items ri( Coord_cl( multi.x + posx, multi.y + posy, pos.z, pos.map ) );
		for( ri.Begin(); !ri.atEnd(); ri++ ) 
		{
			P_ITEM pi = ri.GetData();
			if( ( pi != NULL ) && ( pi->serial != this->serial ) && ( pi->pos.x == (multi.x + posx) ) && ( pi->pos.y == (multi.y + posy) ) ) 
				return false;
		}
		*/
	}
	return true;
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
	UI08 newboatdir = this->boatdir;

	if( turn == 0 )
		return;
	else if( turn > 0 ) // clockwise
	{
		if( this->boatdir >= 5 )
			newboatdir = 0;
		else
			newboatdir+=2;
	}
	else if( turn < 0 ) 
	{
		if( this->boatdir <= 1 )
			newboatdir = 6;
		else
			newboatdir-=2;
	}

	if( !this->isValidPlace( pos.x, pos.y, pos.z, newboatdir ) )
		errormsg = "Arr, something's in the way!";

	// first pause all clients in visrange
	QPtrList< cUOSocket > socketsinrange; // sockets of the chars in visrange

	cRegion::RegionIterator4Chars ri( pos );
	for( ri.Begin(); !ri.atEnd(); ri++ ) 
	{
		P_CHAR pc = ri.GetData();
		if( pc != NULL ) 
		{
			cUOSocket* socket = pc->socket();
			if( socket )
			{
				if( errormsg.isNull() || errormsg.isEmpty() )
				{
					cUOTxPause* uoPacket = new cUOTxPause();
					uoPacket->pause();
					socket->send( uoPacket );
					socketsinrange.append( socket );
				}
				else
					pTiller->talk( errormsg, 0x481, 0 );
			}
		}
	}

	if( !errormsg.isNull() || !errormsg.isEmpty() )
		return;

	UI08 shortboatdir = newboatdir / 2;
	this->id_ = this->multiids_[ shortboatdir ];
	this->boatdir = newboatdir;
	
	// turn all items and chars on the boat and send them
	QValueList< SERIAL >::iterator it = chars_.begin();
	while( it != chars_.end() )
	{
		SI08 dx = 0, dy = 0;
		P_CHAR pc = FindCharBySerial( *it );
		if( pc != NULL )
		{
			UI16 newx = pc->pos.x; 
			UI16 newy = pc->pos.y;
			dx = this->pos.x - newx;
			dy = this->pos.y - newy;
			if( turn > 0 )
			{
				newx = pos.x + dy;
				newy = pos.y - dx;
			}
			else
			{
				newx = pos.x - dy;
				newy = pos.y + dx;
			}
			pc->MoveTo( newx, newy, pos.z );

			cUOTxDrawChar* drawChar = new cUOTxDrawChar();
			drawChar->fromChar( pc );


			QPtrListIterator< cUOSocket > iter_sock( socketsinrange );
			while( iter_sock.current() )
			{
				iter_sock.current()->removeObject( pc );
				if( iter_sock.current() == pc->socket() )
					iter_sock.current()->resendPlayer();
				if( !( ( pc->isHidden() || ( pc->dead && !pc->war ) ) && !iter_sock.current()->player()->isGMorCounselor() ) )
				{
					drawChar->setHighlight( pc->notority( iter_sock.current()->player() ) );
					iter_sock.current()->send( new cUOTxDrawChar( *drawChar ) );
				}
				++iter_sock;
			}

			delete drawChar;
		}
		++it;
	}

	QPtrList< cItem > founditems;
	cRegion::RegionIterator4Items rj( pos );
	for( rj.Begin(); !rj.atEnd(); rj++ ) 
	{
		P_ITEM pi = rj.GetData();
		if( pi && inMulti( pi->pos ) )
			founditems.append( pi );
	}

	P_ITEM pi = NULL;
	QPtrListIterator< cItem > cit( founditems );
	while( pi = cit.current() )
	{
		SI08 dx = 0, dy = 0;
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

		QPtrListIterator< cUOSocket > iter_sock( socketsinrange );
		while( iter_sock.current() )
		{
			pi->update( iter_sock.current() );
			++iter_sock;
		}
	++cit;
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
		pos.y + itemoffsets[shortboatdir][TILLER][Y],
		pTiller->pos.z );
	pTiller->setId( itemids[shortboatdir][TILLER_ID] );
	
	pHold->MoveTo( pos.x + itemoffsets[shortboatdir][HOLD][X],
		pos.y + itemoffsets[shortboatdir][HOLD][Y],
		pHold->pos.z );
	pHold->setId( itemids[shortboatdir][HOLD_ID] );

	QPtrListIterator< cUOSocket > iter_sock( socketsinrange );
	cUOSocket* mSock;
	while( mSock = iter_sock.current() )
	{
		pPortplank->update( mSock );
		pStarplank->update( mSock );
		pTiller->update( mSock );
		pHold->update( mSock );
		this->update( mSock );
		cUOTxPause* uoPacket = new cUOTxPause();
		uoPacket->resume();
		mSock->send( uoPacket );
		++iter_sock;
	}
}

bool cBoat::move( void )
{
	if( this->moves_ == 0 && this->shift_ == 0 )
		return false;

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
		dx += shift_;
		dy -= moves_;
		break;
	case 2:
		dx += moves_;
		dy += shift_;
		break;
	case 4: 
		dx -= shift_;
		dy += moves_;
		break;
	case 6: 
		dx -= moves_;
		dy -= shift_;
		break;
	default:
		clConsole.log( QString( "WARNING: cBoat::Move: invalid boatdirection caught (boatdir: %1, serial: %2), corrected to north boatdir!").arg(this->boatdir).arg(this->serial).latin1() );
		this->boatdir = 0;
		dy -= moves_;
		break;
	}

	if( ( this->pos.x+dx<=200 || this->pos.x+dx>=6000) && (this->pos.y+dy<=200 || this->pos.y+dy>=4900)) //bugfix LB
	{
		errormsg = "Arr, Sir, we've hit rough waters!";
		moves_ = 0;
		shift_ = 0;
	}
	else if( !this->isValidPlace( pos.x+dx, pos.y+dy, pos.z, boatdir ) )
	{
		errormsg = "Arr, somethings in the way!";
		moves_ = 0;
		shift_ = 0;
	}

	// Move all the special items
	// first pause all clients in visrange
	QPtrList< cUOSocket > socketsinrange; // sockets of the chars in visrange

	cRegion::RegionIterator4Chars ri( pos );
	for( ri.Begin(); !ri.atEnd(); ri++ ) 
	{
		P_CHAR pc = ri.GetData();
		if( pc != NULL ) 
		{
			cUOSocket* socket = pc->socket();
			if( socket )
			{
				if( errormsg.isNull() && errormsg.isEmpty() )
				{
					cUOTxPause* uoPacket = new cUOTxPause();
					uoPacket->pause();
					socket->send( uoPacket );
					socketsinrange.append( socket );
				}
				else
					pTiller->talk( errormsg, 0x481, 0 );
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

	QValueList< SERIAL >::iterator it = chars_.begin();
	while( it != chars_.end() )
	{
		P_CHAR pc = FindCharBySerial( *it );
		if( pc )
		{
			pc->MoveTo( pc->pos.x + dx, pc->pos.y + dy, pc->pos.z );

			cUOTxDrawChar* drawChar = new cUOTxDrawChar();
			drawChar->fromChar( pc );

			QPtrListIterator< cUOSocket > iter_sock( socketsinrange );
			while( iter_sock.current() )
			{
				iter_sock.current()->removeObject( pc );
				if( iter_sock.current() == pc->socket() )
					iter_sock.current()->resendPlayer();
				if( !( ( pc->isHidden() || ( pc->dead && !pc->war ) ) && !iter_sock.current()->player()->isGMorCounselor() ) )
				{
					drawChar->setHighlight( pc->notority( iter_sock.current()->player() ) );
					iter_sock.current()->send( new cUOTxDrawChar( *drawChar ) );
				}
				++iter_sock;
			}

			delete drawChar;
		}
		++it;
	}

	QPtrList< cItem > founditems;
	cRegion::RegionIterator4Items rj( pos );
	for( rj.Begin(); !rj.atEnd(); rj++ ) 
	{
		P_ITEM pi = rj.GetData();
		if( pi && inMulti( pi->pos ) )
		{
			bool boatitem = false;
			for( int i = 0; i < 4; ++i )
			{
				if( itemserials[i] == pi->serial )
				{
					boatitem = true;
					break;
				}
			}
			if( !boatitem )
				founditems.append( pi );
		}
	}


	P_ITEM pi = NULL;
	QPtrListIterator< cItem > cit( founditems );
	while( pi = cit.current() )
	{
		pi->MoveTo( pi->pos.x + dx, pi->pos.y + dy, pi->pos.z );


		QPtrListIterator< cUOSocket> iter_sock( socketsinrange );
		while( iter_sock.current() )
		{
			pi->update( iter_sock.current() );
			++iter_sock;
		}
		++cit;
	}

	QPtrListIterator< cUOSocket > iter_sock( socketsinrange );
	cUOSocket* mSock;
	while( mSock = iter_sock.current() )
	{
		pPortplank->update( mSock );
		pStarplank->update( mSock );
		pTiller->update( mSock );
		pHold->update( mSock );
		this->update( mSock );
		cUOTxPause* uoPacket = new cUOTxPause();
		uoPacket->resume();
		mSock->send( uoPacket );
		++iter_sock;
	}

	return true;
}

void cBoat::handlePlankClick( cUOSocket* socket, P_ITEM pplank )
{
	P_CHAR pc_currchar = socket->player();
	if( !pc_currchar )
		return;

	bool charonboat = false;

	QValueList< SERIAL >::iterator it = chars_.begin();
	while( it != chars_.end() )
	{
		if( *it == pc_currchar->serial )
			charonboat = true;
		++it;
	}

	if( !charonboat )
	{
		vector< SERIAL > vecCown = cownsp.getData( pc_currchar->serial );
		vector< SERIAL >::iterator vit = vecCown.begin();
		while( vit != vecCown.end() )
		{
			P_CHAR pc = FindCharBySerial( *vit );
			if( pc != NULL )
			{
				if( pc->isNpc() && pc_currchar->Owns( pc ) )
				{
					pc->moveTo( pos + Coord_cl( 1, 1, 2 ) );
					pc->SetMultiSerial( this->serial );
					this->chars_.append( pc->serial );
					pc->update();
				}
			}
			vit++;
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
		this->chars_.append( pc_currchar->serial );
		socket->sysMessage( tr("You entered a boat") );
	}
	else
	{
		if( leave( socket, pplank ) ) 
		{
			socket->sysMessage( tr("You left the boat.") );
		} 
		else 
		{
			socket->sysMessage( tr("You cannot get off here!") );
		}
	}
}

bool cBoat::leave( cUOSocket* socket, P_ITEM pplank )
{
	P_CHAR pc_currchar = socket->player();
	if( !pc_currchar )
		return false;

	UI16 x, y, x0, y0, x1, y1;
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
	map_st mapt;
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
				pc->SetMultiSerial( INVALID_SERIAL );
				this->chars_.remove( pc->serial );
				pc->update();
			}
		}
		it++;
	}
	pc_currchar->MoveTo( x, y, z );
	teleport( pc_currchar );
	pc_currchar->SetMultiSerial( INVALID_SERIAL );
	this->chars_.remove( pc_currchar->serial );
	return true;
}

void cBoat::switchPlankState( P_ITEM pplank ) //Open, or close the plank (called from keytarget() )
{
	if( !pplank->tags.get( "boatserial" ).isValid() || pplank->tags.get( "boatserial" ).toUInt() != this->serial )
		return;

	UI08 shortboatdir = this->boatdir / 2;
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

char cBoat::speechInput( cUOSocket* socket, const QString& msg )//See if they said a command. msg must already be capitalized
{
	SERIAL serial;

	if( !socket ) 
		return 0;

	//get the tiller man's item #
	serial = this->itemserials[ TILLER ];
	if ( serial == INVALID_SERIAL ) 
		return 0;
	P_ITEM tiller = FindItemBySerial( serial );
	if ( tiller == NULL ) 
		return 0;

	if(msg.contains("FORWARD LEFT")) {
		this->shift_ = -1;
		this->moves_ = 1;
		if( move() )
			tiller->talk( tr("Aye, sir."), 0x481, 0 );
		return 1;
	} else if (msg.contains ("FORWARD RIGHT") ) {
		this->shift_ = 1;
		this->moves_ = 1;
		if( move() )
			tiller->talk( tr("Aye, sir."), 0x481, 0 );
		return 1;
	} else if (msg.contains ("BACKWARD RIGHT") ) {
		this->shift_ = 1;
		this->moves_ = -1;
		tiller->talk( tr("Aye, sir."), 0x481, 0 );
		return 1;
	} else if (msg.contains ("BACKWARD LEFT")) {
		this->shift_ = -1;
		this->moves_ = -1;
		if( move() )
			tiller->talk( tr("Aye, sir."), 0x481, 0 );
		return 1;
	} else	if (msg.contains ("FORWARD")) {
		this->moves_ = 1;
		if( move() )
			tiller->talk( tr("Aye, sir."), 0x481, 0 );
	} else if (msg.contains ("BACKWARD")) {
		this->moves_ = -1;
		if( move() )
			tiller->talk( tr("Aye, sir."), 0x481, 0 );
	} else if(msg.contains("LEFT") && !msg.contains("TURN"))	{
		this->shift_ = -1;
		if( move() )
			tiller->talk( tr("Aye, sir."), 0x481, 0 );
		return 1;
	} else if(msg.contains("RIGHT") && !msg.contains("TURN")) {
		this->shift_ = 1;
		if( move() )
			tiller->talk( tr("Aye, sir."), 0x481, 0 );
		return 1;
	}

	if(msg.contains("ONE") || msg.contains("DRIFT") ||
		msg.contains("STOP") || msg.contains("FURL SAIL") ) // reset to 0
	{
		this->autosail_ = 0;
		this->shift_ = 0;
		this->moves_ = 0;
		tiller->talk( tr("Aye, sir."), 0x481, 0 );
		return 1;
	}

	if( msg.contains("TURN") )
	{
		if( msg.contains("RIGHT") || msg.contains("STARBOARD") )
		{
			this->turn(1);
			tiller->talk( tr("Aye, sir."), 0x481, 0 );
			return 1;
		}
		else if( msg.contains("LEFT") || msg.contains("PORT") )
		{
			this->turn(-1);
			tiller->talk( tr("Aye, sir."), 0x481, 0 );
			return 1;
		}
		else if( msg.contains("COME ABOUT") || msg.contains("AROUND") )
		{
			turn(1);
			turn(1);
			tiller->talk( tr("Aye, sir."), 0x481, 0 );
			return 1;
		}
	}

	if(msg.contains("SET NAME"))
	{
		tiller->setName( tr("a ship named %1").arg(msg.right(msg.length()-8)) );
		return 1;
	}

	return 0;
}

// khpae - make deed from a boat
void cBoat::toDeed( cUOSocket* socket ) {
	P_CHAR pc = socket->player();
	if (pc == NULL) {
		return;
	}
	// if player is in boat
	if (pc->multis == this->serial) {
		socket->sysMessage ( tr("You must leave the boat to deed it.") );
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
		socket->sysMessage ( tr("You don't have the boat key.") );
		return;
	}
	// if any pcs / npcs / items are in the boat, it cannot be deed.
/*	if (items_.size () > 0) {
		socket->sysMessage ( tr("You can only deed with empty boat (remove items).") );
		return;
	}*/
	if (chars_.size () > 0) {
		socket->sysMessage ( tr("You can only deed with empty boat (remove pc/npcs).") );
		return;
	}
	// add deed
	P_ITEM bdeed = Items->SpawnItemBackpack2 (-1, this->deedsection_, 0);
	if (bdeed == NULL) {
		socket->sysMessage ( tr("There's problem with deed boat. Please contact Game Master.") );
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
	socket->sysMessage ( tr("You deed the boat.") );
}

void cBoat::Serialize( ISerialization &archive )
{
	if( archive.isReading() )
	{
		archive.read( "boat.autosail", autosail_ );
		archive.read( "boat.dir", boatdir );
		int i,j,k;
		for( i = 0; i < 8; i++ )
		{
			int currid = 0;
			archive.read( (char*)QString("boat.multiid%1").arg(i).latin1(), currid );
			this->multiids_.push_back( currid );
		}
		for( i = 0; i < 4; i++ )
		{
			for( j = 0; j < 4; j++ )
			{
				for( k = 0; k < 2; k++ )
				{
					signed short curroffset = 0;
					archive.read( (char*)QString("boat.itemoffset.%1.%2.%3").arg(i).arg(j).arg(k).latin1(), curroffset );
					this->itemoffsets[i][j][k] = curroffset;
				}
			}
		}
		for( i = 0; i < 4; i++ )
		{
			for( j = 0; j < 6; j++ )
			{
				unsigned short currid = 0;
				archive.read( (char*)QString("boat.itemid.%1.%2").arg(i).arg(j).latin1(), currid );
				this->itemids[i][j] = currid;
			}
		}

		for( i = 0; i < 4; ++i )
		{
			int serial = 0;
			archive.read( (char*)QString("boat.itemserial.%1").arg(i).latin1(), serial );
			this->itemserials[ i ] = serial;
		}
	}
	else
	{
		archive.write( "boat.autosail", autosail_ );
		archive.write( "boat.dir", boatdir );
		int i, j, k;
		for( i = 0; i < 8; i++ )
		{
			int currid = this->multiids_[i];
			archive.write( (char*)QString("boat.multiid%1").arg(i).latin1(), currid );
		}
		for( i = 0; i < 4; i++ )
		{
			for( j = 0; j < 4; j++ )
			{
				for( k = 0; k < 2; k++ )
				{
					archive.write( (char*)QString("boat.itemoffset.%1.%2.%3").arg(i).arg(j).arg(k).latin1(), this->itemoffsets[i][j][k] );
				}
			}
		}
		for( i = 0; i < 4; i ++ )
		{
			for( j = 0; j < 6; j++ )
			{
				archive.write( (char*)QString("boat.itemid.%1.%2").arg(i).arg(j).latin1(), this->itemids[i][j] );
			}
		}

		for( i = 0; i < 4; ++i )
		{
			archive.write( (char*)QString("boat.itemserial.%1").arg(i).latin1(), this->itemserials[i] );
		}

	}
	cItem::Serialize( archive );
}


