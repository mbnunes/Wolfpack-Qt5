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

#include "multis.h"

#include "regions.h"
#include "tilecache.h"
#include "mapstuff.h"

#undef DBGFILE
#define DBGFILE "multis.cpp" 

cMulti::cMulti()
{
	cItem::Init( false );
	deedsection_ = (char*)0;
	itemsdecay_ = false;
}

void cMulti::Serialize( ISerialization &archive )
{
	if( archive.isReading() )
	{
		archive.read( "multi.deedid", deedsection_ );
		unsigned int size = 0;
		register unsigned int i;
		archive.read( "multi.charcount", size );
		for( i = 0; i < size; ++i )
		{
			SERIAL charserial = INVALID_SERIAL;
			archive.read( (char*)QString("multi.char.%1").arg(i).latin1(), charserial );
			chars_.append( charserial );
		}
		archive.read( "multi.itemcount", size );
		for( i = 0; i < size; ++i )
		{
			SERIAL itemserial = INVALID_SERIAL;
			archive.read( (char*)QString("multi.item.%1").arg(i).latin1(), itemserial );
			items_.append( itemserial );
		}

		SERIAL readData;
		archive.read( "multi.bancount", size );
		for( i = 0; i < size; ++i )
		{
			archive.read( (char*)QString("multi.ban.%1").arg(i).latin1(), readData );
			bans_.push_back( readData );			
		}
		archive.read( "multi.friendcount", size);
		for( i = 0; i < size; ++i )
		{
			archive.read( (char*)QString("multi.friend.%1").arg(i).latin1(), readData );
			friends_.push_back( readData );
		}
	}
	else
	{
		archive.write( "multi.deedid", deedsection_ );
		register unsigned int i = 0;
		archive.write( "multi.charcount", chars_.size() );
		QValueList< SERIAL >::iterator it = chars_.begin();
		while( it != chars_.end() )
		{
			archive.write( (char*)QString("multi.char.%1").arg(i).latin1(), (*it) );
			++i;
			++it;
		}
		archive.write( "multi.itemcount", items_.size() );
		it = items_.begin();
		i = 0;
		while( it != items_.end() )
		{
			archive.write( (char*)QString("multi.item.%1").arg(i).latin1(), (*it) );
			++i;
			++it;
		}

		archive.write( "multi.bancount", bans_.size() );
		for ( i = 0; i < bans_.size(); ++i )
			archive.write( (char*)QString("multi.ban.%1").arg(i).latin1(), bans_[i] );
		archive.write( "multi.friendcount", friends_.size() );
		for ( i = 0; i < friends_.size(); ++i )
			archive.write( (char*)QString("multi.friend.%1").arg(i).latin1(), friends_[i] );
	}
	cItem::Serialize( archive );
}


void cMulti::processNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	QString Value = getNodeValue( Tag );

	// <deed>deedsection</deed> (any item section)
	if( TagName == "deed" )
		this->deedsection_ = Value;

	// <name>balbalab</name>
	else if( TagName == "name" )
		this->setName( Value );

	// <itemsdecay />
	else if( TagName == "itemsdecay" )
		this->itemsdecay_ = true;
}


bool cMulti::inMulti( const Coord_cl &srcpos )
{
	return inMulti( srcpos, this->pos, this->id() );
}

bool cMulti::inMulti( const Coord_cl &srcpos, const Coord_cl &multipos, UI16 id )
{
	SI32 length;
	st_multi multi;
	UOXFile *mfile;
	Map->SeekMulti( id - 0x4000, &mfile, &length );
	length = length / sizeof( st_multi );
	if (length == -1 || length >= 17000000) // Too big...
	{
		LogError( (char*)QString( "cMulti::inMulti( ... ) - Bad length in multi file. Avoiding stall.\n").latin1() );
		length = 0;
	}
	register int j;
	for ( j = 0; j < length; ++j)
	{
		mfile->get_st_multi( &multi );
		if ( multi.visible && ( multipos.x + multi.x == srcpos.x) && ( multipos.y + multi.y == srcpos.y ) )
		{
			return true;
		}
	}
	return false;
}

cMulti* cMulti::findMulti( const Coord_cl &pos )
{
	SI32 lastdistance = 30;
	cMulti* pMulti = NULL;
	SI32 currdistance;

	cRegion::RegionIterator4Items ri( pos );
	
	for( ri.Begin(); !ri.atEnd(); ri++ )
	{
		cMulti* pCurrMulti = dynamic_cast< cMulti* >(ri.GetData());
		if( pCurrMulti )
		{
			currdistance = pos.distance( pCurrMulti->pos );
			if( currdistance <= lastdistance )
			{
				lastdistance = currdistance;
				if( pCurrMulti->inMulti( pos ) )
					pMulti = pCurrMulti;
			}
		}
	}

	return pMulti;
}

void cMulti::addItem( P_ITEM pi )
{
	if( !items_.contains( pi->serial ) )
		items_.append( pi->serial );

	pi->SetMultiSerial( serial );
}

void cMulti::removeItem( P_ITEM pi )
{
	items_.remove( pi->serial );
	pi->SetMultiSerial( INVALID_SERIAL );
}

void cMulti::checkItems()
{
	QValueList< SERIAL > toremove;
	QValueList< SERIAL >::iterator it = items_.begin();
	while( it != items_.end() )
	{
		P_ITEM pi = FindItemBySerial( *it );
		if( !pi )
			toremove.append( *it );
		else
		{
			if( !inMulti( pi->pos ) )
				toremove.append( *it );
		}
		++it;
	}
	it = toremove.begin();
	while( it != toremove.end() )
	{
		items_.remove( *it );
		++it;
	}
}

void cMulti::addChar( P_CHAR pc )
{
	if( !chars_.contains( pc->serial ) )
		chars_.append( pc->serial );
	
	pc->SetMultiSerial( serial );
}

void cMulti::removeChar( P_CHAR pc )
{
	chars_.remove( pc->serial );
	pc->SetMultiSerial( INVALID_SERIAL );
}

void cMulti::checkChars()
{
	QValueList< SERIAL > toremove;
	QValueList< SERIAL >::iterator it = chars_.begin();
	while( it != chars_.end() )
	{
		P_CHAR pc = FindCharBySerial( *it );
		if( !pc )
			toremove.append( *it );
		else
		{
			if( !inMulti( pc->pos ) )
				toremove.append( *it );
		}
		++it;
	}
	it = toremove.begin();
	while( it != toremove.end() )
	{
		chars_.remove( *it );
		++it;
	}
}

bool cMulti::isBanned(P_CHAR pc)
{
	return binary_search(bans_.begin(), bans_.end(), pc->serial);
}

bool cMulti::isFriend(P_CHAR pc)
{
	return binary_search(friends_.begin(), friends_.end(), pc->serial);
}

void cMulti::addBan(P_CHAR pc)
{
	bans_.push_back(pc->serial);
	sort(bans_.begin(), bans_.end());
}

void cMulti::addFriend(P_CHAR pc)
{	
	friends_.push_back(pc->serial);
	sort(friends_.begin(), friends_.end());
}

void cMulti::removeBan(P_CHAR pc)
{
	vector<SERIAL>::iterator it = find(bans_.begin(), bans_.end(), pc->serial);
	bans_.erase(it);
}

void cMulti::removeFriend(P_CHAR pc)
{
	vector<SERIAL>::iterator it = find(friends_.begin(), friends_.end(), pc->serial);
	friends_.erase(it);
}

void cMulti::createKeys( P_CHAR pc, const QString &name )
{
	if( !pc )
		return;

	P_ITEM pBackpack = FindItemBySerial( pc->packitem );
	P_ITEM pBankbox = pc->getBankBox();

	P_ITEM pKey = Items->createScriptItem( "100f" );
	if( pKey )
	{
		pKey->tags.set( "linkserial", this->serial );
		pKey->setType( 7 );
		pKey->priv = 2;
		pKey->setName( name );
		if( pBackpack )
			pBackpack->AddItem( pKey );
		else
			pBankbox->AddItem( pKey );
	}

	// just create 3 additional keys...
	for( register int i = 0; i < 3; ++i )
	{
		pKey = Items->createScriptItem( "100f" );
		if( pKey )
		{
			pKey->tags.set( "linkserial", this->serial );
			pKey->setType( 7 );
			pKey->priv = 2;
			pKey->setName( name );
			pBankbox->AddItem( pKey );
		}
	}
}

void cMulti::removeKeys( void )
{
	AllItemsIterator iter_items;
	for( iter_items.Begin(); !iter_items.atEnd(); ++iter_items )
	{
		P_ITEM pi = iter_items.GetData();
		if( pi->type() == 7 && pi->tags.get( "linkserial" ).isValid() && pi->tags.get( "linkserial" ).toUInt() == this->serial )
			Items->DeleItem( pi );
	}
}

P_ITEM cMulti::findKey( P_CHAR pc )
{
	P_ITEM pi = NULL;
	bool found = false;
	vector<SERIAL> vpack = contsp.getData( pc->packitem );
	vector<SERIAL>::iterator it = vpack.begin();
	while( it != vpack.end() )
	{
		pi = FindItemBySerial( *it );
		if( !pi ) 
		{
			contsp.remove( pc->packitem, (*it) );
			++it;
			continue;
		}
		
		if( pi->type() == 7 ) 
		{
			if( pi->tags.get( "linkserial" ).isValid() )
			{
				SERIAL si = pi->tags.get( "linkserial" ).toUInt();
				if( si == this->serial ) 
				{
					found = true;
					break;
				}
			}
		}

		++it;
	}

	if( found )
		return pi;
	else
		return NULL;
}


bool cMulti::authorized( P_CHAR pc )
{
	return ( pc->isGMorCounselor() || ownserial == pc->serial || findKey( pc ) || isFriend( pc ) );
}
