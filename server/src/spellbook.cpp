//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
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

#include "spellbook.h"
#include "prototypes.h"
#include "network/uosocket.h"
#include "network/uotxpackets.h"
#include "newmagic.h"
#include "persistentbroker.h"
#include "globals.h"
#include "basechar.h"
#include "player.h"

// Library Includes
#include <qdom.h>

void cSpellBook::Init( bool mkser )
{
	cItem::Init( mkser );
	spells1_ = 0;
	spells2_ = 0;
}

void cSpellBook::addSpell( UINT8 spell )
{
	UINT32 bitmask = 0x80000000 >> ( ( 7 - spell % 8 ) + ( spell - spell % 8 ) );	
	if( spell < 32 ) spells1_ |= bitmask; 
	else spells2_ |= bitmask;
	changed( SAVE|TOOLTIP );
}

void cSpellBook::removeSpell( UINT8 spell )
{

	UINT32 bitmask = 0xFFFFFFFF - ( 0x80000000 >> ( ( 7 - spell % 8 ) + ( spell - spell % 8 ) ) );	
	if( spell < 32 ) spells1_ &= bitmask; 
	else spells2_ &= bitmask;
	changed( SAVE|TOOLTIP );
}

bool cSpellBook::hasSpell( UINT8 spell ) const
{
	UINT32 bitmask = 0x80000000 >> ( ( 7 - spell % 8 ) + ( spell - spell % 8 ) );	
	if( spell < 32 ) return spells1_ & bitmask; 
	else return spells2_ & bitmask;

}

// abstract cDefinable
void cSpellBook::processNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	QString Value = this->getNodeValue( Tag );

	// Add a spell
	if( TagName == "addspell" )
		addSpell( hex2dec( Value ).toShort() );

	// Remove a spell
	else if( TagName == "removespell" )
		removeSpell( hex2dec( Value ).toShort() );

	else
		cItem::processNode( Tag );
}

bool cSpellBook::onUse( cUObject *Target )
{
	// Check existing scripts first
	if( cItem::onUse( Target ) )
		return true;

	// We assume that target is a player here
	P_PLAYER pChar = dynamic_cast< P_PLAYER >( Target );

	if( !pChar || !pChar->socket() )
		return true;

	// First we send a multi-object-to-container packet and then open the gump
	cUOTxNewSpellbook book;

	book.setBook( serial() );
	book.setModel( id() ); // common spellbook model

	UINT16 offset = 1;

	if( type() == 1201 )
		offset = 101;
	else if( type() == 1202 )
		offset = 201;
    
	book.setOffset( offset );
	book.setSpell1( spells1() );
	book.setSpell2( spells2() );
	
	pChar->socket()->send( &book );

	cUOTxDrawContainer drawcont;
	drawcont.setGump( 0xFFFF );
	drawcont.setSerial( serial() );
	pChar->socket()->send( &drawcont );

	return true;
}

bool cSpellBook::onSingleClick( P_PLAYER Viewer )
{
	// See if there are scripted events, if not do it our way.
	if( cItem::onSingleClick( Viewer ) )
		return true;
	
	// Show the amount of spells in this spellbook and return false so the normal name is shown as well
	if( !Viewer->socket() )
		return false;

	Viewer->socket()->showSpeech( this, tr( "%1 spells" ).arg( spellCount() ) );

	return false;
}

UINT8 cSpellBook::spellCount() const
{
	UINT8 count = 0;

	for( UINT8 i = 0; i < 64; ++i )
		if( hasSpell( i ) )
			++count;

	return count;
}

static cUObject* productCreator()
{
	return new cSpellBook;
}

void cSpellBook::registerInFactory()
{
	QStringList fields, tables, conditions;
	buildSqlString( fields, tables, conditions ); // Build our SQL string
	QString sqlString = QString( "SELECT uobjectmap.serial,uobjectmap.type,%1 FROM uobjectmap,%2 WHERE uobjectmap.type = 'cSpellBook' AND %3" ).arg( fields.join( "," ) ).arg( tables.join( "," ) ).arg( conditions.join( " AND " ) );
	UObjectFactory::instance()->registerType("cSpellBook", productCreator);
	UObjectFactory::instance()->registerSqlQuery( "cSpellBook", sqlString );
}

void cSpellBook::buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions )
{
	cItem::buildSqlString( fields, tables, conditions );
	fields.push_back( "spellbooks.spells1,spellbooks.spells2" );
	tables.push_back( "spellbooks" );
	conditions.push_back( "uobjectmap.serial = spellbooks.serial" );
}

void cSpellBook::load( char **result, UINT16 &offset )
{
	cItem::load( result, offset );
	spells1_ = atoi( result[offset++] );
	spells2_ = atoi( result[offset++] );
}

void cSpellBook::save()
{
	initSave;
	setTable( "spellbooks" );
	addField( "serial", serial() );

	addField( "spells1", spells1_ );
	addField( "spells2", spells2_ );

	addCondition( "serial", serial() );
	saveFields;

	cItem::save();
}

bool cSpellBook::del()
{
	if( !isPersistent )
		return false;

	persistentBroker->addToDeleteQueue( "spellbooks", QString( "serial = '%1'" ).arg( serial() ) );

	return cItem::del();
}


