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

#include "multis.h"

#include "sectors.h"
#include "tilecache.h"
#include "maps.h"
#include "network/uosocket.h"
#include "multiscache.h"
#include "dbdriver.h"
#include "persistentbroker.h"
#include "targetrequests.h"
#include "accounts.h"

// Library Includes
#include <qdom.h>

// System Includes
#include <math.h>
#include <algorithm>

using namespace std;

#undef DBGFILE
#define DBGFILE "multis.cpp" 

cMulti::cMulti()
{
	cItem::Init( false );
	deedsection_ = (char*)0;
	itemsdecay_ = false;
	coowner_ = INVALID_SERIAL;
}

void cMulti::buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions )
{
	cItem::buildSqlString( fields, tables, conditions );
	fields.push_back( "multis.coowner,multis.deedsection" );
	tables.push_back( "multis" );
	conditions.push_back( "uobjectmap.serial = multis.serial" );
}

void cMulti::load( char **result, UINT16 &offset )
{
	cItem::load( result, offset );
	
	coowner_ = atoi( result[offset++] );
	deedsection_ = result[offset++];

	// Load from two additional tables here
	QString sql = "SELECT multis_bans.serial,multis_bans.ban FROM multis_bans WHERE multis_bans.serial = '" + QString::number( serial() ) + "'";

	cDBResult res = persistentBroker->query( sql );	

	while( res.fetchrow() )
	{
		// row[1] is our serial
		SERIAL banned = res.getInt( 1 );
		P_CHAR pChar = FindCharBySerial( banned );

		if( pChar )
			addBan( pChar );
	}

	res.free();

	sql = "SELECT multis_friends.serial,multis_friends.friend FROM multis_friends WHERE multis_friends.serial = '" + QString::number( serial() ) + "'";

	res = persistentBroker->query( sql );

	while( res.fetchrow() )
	{
		// row[1] is our serial
		SERIAL friendserial = res.getInt( 1 );
		P_CHAR pChar = FindCharBySerial( friendserial );

		if( pChar )
			addFriend( pChar );
	}

	res.free();
}

void cMulti::save()
{
	initSave;
	setTable( "multis" );
	
	addField( "serial", serial() );
	addField( "coowner", coowner_ );
	addStrField( "deedsection", deedsection_ );

	addCondition( "serial", serial() );
	saveFields;

	// Reset Bans+Friends
	if( isPersistent )
	{
		persistentBroker->executeQuery( QString( "DELETE FROM multis_bans WHERE serial = '%1'" ).arg( serial() ) );
		persistentBroker->executeQuery( QString( "DELETE FROM multis_friends WHERE serial = '%1'" ).arg( serial() ) );
	}
	
	// Friends + Bans
	INT32 i;
	for ( i = 0; i < bans_.size(); ++i )
		persistentBroker->executeQuery( QString( "REPLACE INTO multis_bans VALUES(%1,%2)" ).arg( serial() ).arg( bans_[i] ) );

	for ( i = 0; i < friends_.size(); ++i )
		persistentBroker->executeQuery( QString( "REPLACE INTO multis_friends VALUES(%1,%2)" ).arg( serial() ).arg( friends_[i] ) );

	cItem::save();
}

bool cMulti::del()
{
	if( !isPersistent )
		return false;

	persistentBroker->addToDeleteQueue( "multis", QString( "serial = '%1'" ).arg( serial() ) );
	persistentBroker->addToDeleteQueue( "multis_bans", QString( "serial = '%1'" ).arg( serial() ) );
	persistentBroker->addToDeleteQueue( "multis_friends", QString( "serial = '%1'" ).arg( serial() ) );

	return cItem::del();
}

void cMulti::processNode( const cElement *Tag )
{
	QString TagName = Tag->name();
	QString Value = Tag->getValue();

	// <deedsection>deedsection</deedsection> (any item section)
	if( TagName == "deedsection" )
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
	return inMulti( srcpos, this->pos(), this->id() );
}

bool cMulti::inMulti( const Coord_cl &srcpos, const Coord_cl &multipos, UI16 id )
{

	MultiDefinition* def = MultiCache::instance()->getMulti( id - 0x4000 );
	if ( !def )
		return false;
	return def->inMulti( srcpos.x - multipos.x, srcpos.y - multipos.y );
}

cMulti* cMulti::findMulti( const Coord_cl &pos )
{
	SI32 lastdistance = 30;
	cMulti* pMulti = NULL;
	SI32 currdistance;

	RegionIterator4Items ri( pos );
	
	for( ri.Begin(); !ri.atEnd(); ri++ )
	{
		cMulti* pCurrMulti = dynamic_cast< cMulti* >(ri.GetData());
		if( pCurrMulti )
		{
			currdistance = pos.distance( pCurrMulti->pos() );
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
	if( !items_.contains( pi->serial() ) )
		items_.append( pi->serial() );

	pi->SetMultiSerial( serial() );
}

void cMulti::removeItem( P_ITEM pi )
{
	items_.remove( pi->serial() );
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
			if( !inMulti( pi->pos() ) )
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
	if( !chars_.contains( pc->serial() ) )
		chars_.append( pc->serial() );
	
	pc->SetMultiSerial( serial() );
}

void cMulti::removeChar( P_CHAR pc )
{
	chars_.remove( pc->serial() );
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
			if( !inMulti( pc->pos() ) )
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
	return binary_search(bans_.begin(), bans_.end(), pc->serial());
}

bool cMulti::isFriend(P_CHAR pc)
{
	return binary_search(friends_.begin(), friends_.end(), pc->serial());
}

void cMulti::addBan(P_CHAR pc)
{
	if( find(bans_.begin(), bans_.end(), pc->serial()) == bans_.end() )
	{
		bans_.push_back(pc->serial());
		sort(bans_.begin(), bans_.end());
	}
}

void cMulti::addFriend(P_CHAR pc)
{	
	if( find(friends_.begin(), friends_.end(), pc->serial()) == friends_.end() )
	{
		friends_.push_back(pc->serial());
		sort(friends_.begin(), friends_.end());
	}
}

void cMulti::removeBan(P_CHAR pc)
{
	vector<SERIAL>::iterator it = find(bans_.begin(), bans_.end(), pc->serial());
	bans_.erase(it);
}

void cMulti::removeFriend(P_CHAR pc)
{
	vector<SERIAL>::iterator it = find(friends_.begin(), friends_.end(), pc->serial());
	friends_.erase(it);
}

void cMulti::removeBan( SERIAL serial )
{
	vector<SERIAL>::iterator it = find(bans_.begin(), bans_.end(), serial);
	bans_.erase(it);
}

void cMulti::removeFriend( SERIAL serial )
{
	vector<SERIAL>::iterator it = find(friends_.begin(), friends_.end(), serial);
	friends_.erase(it);
}

void cMulti::createKeys( P_PLAYER pc, const QString &name )
{
	if( !pc )
		return;

	P_ITEM pBackpack = pc->getBackpack();
	P_ITEM pBankbox = pc->getBankBox();

	if( !pBackpack && !pBankbox )
		return;

	P_ITEM pKey = cItem::createFromScript( "100f" );
	if( pKey )
	{
		pKey->setTag( "linkserial", this->serial() );
		pKey->setType( 7 );
		pKey->setNewbie( true );
		pKey->setName( name );
		if( pBackpack )
			pBackpack->addItem( pKey );
		else 
			pBankbox->addItem( pKey );
	}

	// just create 3 additional keys...
	for( register int i = 0; i < 3; ++i )
	{
		pKey = cItem::createFromScript( "100f" );
		if( pKey )
		{
			pKey->setTag( "linkserial", this->serial() );
			pKey->setType( 7 );
//			pKey->priv = 2; dont newbie these 3 bank box keys
			pKey->setName( name );
			if( pBankbox )
				pBankbox->addItem( pKey );
			else
				pBackpack->addItem( pKey );
		}
	}
}

void cMulti::removeKeys( void )
{
	QPtrList< cItem > todelete;
	cItemIterator iter_items;
	P_ITEM pi;
	for( pi = iter_items.first(); pi; pi = iter_items.next() )
	{
		if( pi && pi->type() == 7 && pi->getTag( "linkserial" ).isValid() && pi->getTag( "linkserial" ).toInt() == this->serial() )
			todelete.append( pi );
	}
	QPtrListIterator< cItem > it( todelete );
	while( it.current() )
	{
		it.current()->remove();
		++it;
	}
}

P_ITEM cMulti::findKey( P_CHAR pc )
{
	P_ITEM pi = NULL;
	bool found = false;
	P_ITEM pBackpack = pc->getBackpack();
	cItem::ContainerContent container = pBackpack->content();
	cItem::ContainerContent::iterator it = container.begin();;
	while( it != container.end() )
	{
		pi = *it;
		if( !pi ) 
		{
			++it;
			continue;
		}
		
		if( pi->type() == 7 ) 
		{
			if( pi->getTag( "linkserial" ).isValid() )
			{
				SERIAL si = pi->getTag( "linkserial" ).toInt();
				if( si == this->serial() ) 
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
		return 0;
}


bool cMulti::authorized( P_PLAYER pc )
{
	return ( pc->isGMorCounselor() || owner() == pc || findKey( pc ) || isFriend( pc ) );
}


P_CHAR cMulti::coOwner( void )	
{ 
	return FindCharBySerial( coowner_ ); 
}

void cMulti::setCoOwner( P_CHAR pc )
{ 
	if( pc ) 
		coowner_ = pc->serial(); 
}

void cMulti::setName( const QString nValue )
{
	this->name_ = nValue;
	QValueList< SERIAL >::iterator it = items_.begin();
	while( it != items_.end() )
	{
		P_ITEM pi = FindItemBySerial( *it );
		if( pi && pi->type() == 222 )
			pi->setName( nValue );
		++it;
	}
}

cMultiGump::cMultiGump( SERIAL charSerial, SERIAL multiSerial )
{
	P_PLAYER pChar = dynamic_cast<P_PLAYER>(FindCharBySerial( charSerial ));
	if( !pChar )
		return;

	cUOSocket* socket = pChar->socket();
	if( !pChar->socket() || !pChar->account() )
		return;

	cMulti* pMulti = dynamic_cast< cMulti* >( FindItemBySerial( multiSerial ) );
	if( !pMulti )
		return;

	P_CHAR pOwner = pMulti->owner();
	QString ownername;
	if( pOwner )
		ownername = pOwner->name();

	P_CHAR pCoOwner = pMulti->coOwner();
	QString coownername;
	if( pCoOwner )
		coownername = pCoOwner->name();

	char_ = charSerial;
	multi_ = multiSerial;

	startPage();
	addResizeGump( 0, 40, 0xA28, 450, 410 ); //Background
	addGump( 105, 18, 0x58B ); // Fancy top-bar
	addGump( 182, 0, 0x589 ); // "Button" like gump
	addGump( 193, 10, 0x15E9 ); // "Button" like gump
	addText( 190, 90, tr( "Owner menu" ), 0x530 );

	// Apply button
	addButton( 50, 400, 0xEF, 0xF0, 2 ); 
	// OK button
	addButton( 120, 400, 0xF9, 0xF8, 1 ); 
	// Cancel button
	addButton( 190, 400, 0xF3, 0xF1, 0 ); 

	bool authban = false;
	bool authfriend = false;

	friends.clear();
	bans.clear();

	if( pChar->account()->authorized( "multi", "friendlist" ) )
	{
		authfriend = true;
		std::vector< SERIAL > friendserials = pMulti->friends();
		std::vector< SERIAL >::iterator it = friendserials.begin();
		while( it != friendserials.end() )
		{
			P_CHAR pc = FindCharBySerial( *it );
			if( pc )
			{
				friends.resize( friends.size()+1 );
				friends.insert( friends.size()-1, pc );
			}
			else
				pMulti->removeFriend( *it );
			++it;
		}
	}

	if( pChar->account()->authorized( "multi", "banlist" ) )
	{
		authban = true;
		std::vector< SERIAL > banserials = pMulti->bans();
		std::vector< SERIAL >::iterator it = banserials.begin();
		while( it != banserials.end() )
		{
			P_CHAR pc = FindCharBySerial( *it );
			if( pc )
			{
				bans.resize( bans.size()+1 );
				bans.insert( bans.size()-1, pc );
			}
			else
				pMulti->removeBan( *it );
			++it;
		}
	}

	UI32 pages = 1;
	UI32 banpages = (UI32)ceil( (double)friends.count() / 10.0f );
	UI32 friendpages = (UI32)ceil( (double)bans.count() / 10.0f );
	if( banpages == 0 && authban )
		banpages = 1;
	if( friendpages == 0 && authfriend )
		friendpages = 1;
	pages = pages + banpages + friendpages;

	UI32 page_ = 1;

	startPage( 1 );

	addResizeGump( 145, 120, 0xBB8, 265, 20 );
	addText( 60, 120, tr( "Name" ), 0x834 );
	addInputField( 150, 120, 250, 16,  1, pMulti->name(), 0x834 );

	addResizeGump( 145, 140, 0xBB8, 265, 20 );
	addText( 60, 140, tr( "Owner" ), 0x834 );
	addText( 150, 140, ownername, 0x834 );
	if( pChar == pOwner || pChar->isGM() )
		addButton( 20, 140, 0xFA5, 0xFA7, 3 );

	addResizeGump( 145, 160, 0xBB8, 265, 20 );
	addText( 60, 160, tr( "Co-Owner" ), 0x834 );
	addText( 150, 160, coownername, 0x834 );
	if( pChar == pOwner || pChar->isGM() )
		addButton( 20, 160, 0xFA5, 0xFA7, 4 );

	UI32 y = 200;
	if( pChar->account()->authorized( "multi", "lockdown" ) )
	{
		addText( 60, y, tr("Lock/Unlock item"), 0x834 );
		addButton( 20, y, 0xFA5, 0xFA7, 5 );
		y += 20;
	}

	if( authban )
	{
		addText( 60, y, tr( "Ban List" ), 0x834 );
		addPageButton( 20, y, 0xFA5, 0xFA7, 2);
		y += 20;
	}

	if( authfriend )
	{
		addText( 60, y, tr( "Friend List" ), 0x834 );
		addPageButton( 20, y, 0xFA5, 0xFA7, 2+banpages);
		y += 20;
	}

	if( pChar->account()->authorized( "multi", "deed" ) )
	{
		addText( 60, y, tr("Turn to deed"), 0x834 );
		addButton( 20, y, 0xFA5, 0xFA7, 6 );
		y += 20;
	}

	addText( 310, 400, tr( "Page %1 of %2" ).arg( page_ ).arg( pages ), 0x834 );
	// prev page
	if( page_ > 1 )
		addPageButton( 270, 400, 0x0FC, 0x0FC, page_-1 );
	// next page
	if( page_ < pages )
		addPageButton( 290, 400, 0x0FA, 0x0FA, page_+1 );

	++page_;
	while( page_ <= pages )
	{
		if( page_ <= banpages+1 )
		{
			startPage( page_ );
			register unsigned int i = (page_-2) * 10;
			while( i < (page_-1) * 10 && i < bans.size() )
			{
				UI32 offset = i - (page_-2) * 10;
				addText( 60, 140+offset*20, QString(bans[ i ]->name()), 0x834 );
				addButton( 20, 140+offset*20, 0xFB1, 0xFB3, 10+i ); 
				++i;
			}
			addText( 40, 120, tr("Ban List"), 0x834 );
			addText( 60, 370, tr("Ban target"), 0x834 );
			addButton( 20, 370, 0xFA5, 0xFA7, 7 );
		}
		else if( page_ <= banpages+friendpages+1 )
		{
			startPage( page_ );
			register unsigned int i = (page_-banpages-2) * 10;
			while( i < (page_-banpages-1) * 10 && i < friends.size() )
			{
				UI32 offset = i - (page_-banpages-2) * 10;
				addText( 60, 140+offset*20, QString(friends[ i ]->name()), 0x834 );
				addButton( 20, 140+offset*20, 0xFB1, 0xFB3, 10+i+bans.size() ); 
				++i;
			}
			addText( 40, 120, tr("Friend List"), 0x834 );
			addText( 60, 370, tr("Add friend"), 0x834 );
			addButton( 20, 370, 0xFA5, 0xFA7, 8 );
		}
		addText( 310, 400, tr( "Page %1 of %2" ).arg( page_ ).arg( pages ), 0x834 );
		// prev page
		if( page_ > 1 )
			addPageButton( 270, 400, 0x0FC, 0x0FC, page_-1 );
		// next page
		if( page_ < pages )
			addPageButton( 290, 400, 0x0FA, 0x0FA, page_+1 );
		++page_;
	}
}

void cMultiGump::handleResponse( cUOSocket* socket, gumpChoice_st choice )
{
	if( choice.button == 0 ) // canceled
		return;

	P_PLAYER pChar = dynamic_cast<P_PLAYER>(FindCharBySerial( char_ ));
	if( !pChar )
		return;

	if( !socket || !pChar->account() )
		return;

	cMulti* pMulti = dynamic_cast< cMulti* >( FindItemBySerial( multi_ ) );
	if( !pMulti )
		return;

	if( choice.button == 1 || choice.button == 2 ) // ok
	{
		std::map< UINT16, QString >::iterator it = choice.textentries.begin();
		while( it != choice.textentries.end() )
		{
			switch( it->first )
			{
			case 1:
				pMulti->setName( it->second );
				break;
			}
			++it;
		}

		if( choice.button == 2 ) // apply
		{
			cMultiGump* pGump = new cMultiGump( char_, multi_ );
			socket->send( pGump );
		}

		return;
	}
	else if( choice.button == 3 || choice.button == 4 )
	{
		socket->sysMessage( tr("Select a person to make %1 of this house!").arg( choice.button == 3 ? tr("owner") : tr("co-owner") ) );
		cSetMultiOwnerTarget* pTargetRequest = new cSetMultiOwnerTarget( multi_, choice.button == 4 );
		socket->attachTarget( pTargetRequest );
	}	
	else if( choice.button == 5 )
	{
		socket->sysMessage( tr("Select an item to lock / unlock!") );
		cMultiChangeLockTarget* pTargetRequest = new cMultiChangeLockTarget( multi_ );
		socket->attachTarget( pTargetRequest );
	}
	else if( choice.button == 6 )
	{
		pMulti->toDeed( socket );
	}
	else if( choice.button == 7 )
	{
		socket->sysMessage( tr("Select a char to add to the list of banned!" ) );
		cMultiAddToListTarget* pTargetRequest = new cMultiAddToListTarget( multi_, true );
		socket->attachTarget( pTargetRequest );
	}
	else if( choice.button == 8 )
	{
		socket->sysMessage( tr("Select a char to add to the list of friends!" ) );
		cMultiAddToListTarget* pTargetRequest = new cMultiAddToListTarget( multi_ );
		socket->attachTarget( pTargetRequest );
	}
	else if( choice.button >= 10 )
	{
		if( choice.button < 10 + bans.size() )
		{
			pMulti->removeBan( bans[ choice.button - 10 ] );
		}
		else if( choice.button < 10 + bans.size() + friends.size() )
		{
			pMulti->removeFriend( friends[ choice.button - 10 - bans.size() ] );
		}
		cMultiGump* pGump = new cMultiGump( char_, multi_ );
		socket->send( pGump );
		return;
	}
}
