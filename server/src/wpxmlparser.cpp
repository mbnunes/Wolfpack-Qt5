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

#include "globals.h"
#include "junk.h"
#include "prototypes.h"
#include "basics.h"
#include "classes.h"

// Library Includes
#include "qdom.h"
#include "qfile.h"

IDefReader::IDefReader( WPDEF_TYPE baseType )
{
	this->setBaseType( baseType );
}

IDefReader::IDefReader( WPDEF_TYPE baseType, QDomElement *baseTag )
{
	this->setBaseTag( baseTag );
	this->setBaseType( baseType );
}

bool IDefReader::prepareParsing( QString Section )
{
	QDomElement *DefSection = DefManager->getSection( this->baseType(), Section );

	if( DefSection->isNull() )
		return false;
	else
		this->setBaseTag( DefSection );

	return true;
}


void IDefReader::applyNodes( P_CHAR Char, QDomElement* Node )
{

}

void IDefReader::processScriptItemNode( P_ITEM madeItem, QDomElement &Node )
{
	for( UI16 k = 0; k < Node.childNodes().count(); k++ )
	{
		QDomElement currChild = Node.childNodes().item( k ).toElement();
		if( currChild.nodeName() == "amount" )
		{
			QString Value = QString();
			UI16 i = 0;
			if( currChild.hasChildNodes() ) // <random> i.e.
				for( i = 0; i < currChild.childNodes().count(); i++ )
				{
					if( currChild.childNodes().item( i ).isText() )
						Value += currChild.childNodes().item( i ).toText().data();
					else if( currChild.childNodes().item( i ).isElement() )
						Value += processNode( currChild.childNodes().item( i ).toElement() );
				}
			else
				Value = currChild.nodeValue();

			if( Value.toInt() < 1 )
				Value = QString("1");

			if( madeItem->pileable() )
				madeItem->setAmount( Value.toInt() );
			else
				for( i = 1; i < Value.toInt(); i++ ) //dupe it n-1 times
					Commands->DupeItem(-1, madeItem, 1);
		}
		else if( currChild.nodeName() == "color" ) //process <color> tags
		{
			QString Value = QString();
			if( currChild.hasChildNodes() ) // colorlist or random i.e.
				for( UI16 i = 0; i < currChild.childNodes().count(); i++ )
				{
					if( currChild.childNodes().item( i ).isText() )
						Value += currChild.childNodes().item( i ).toText().data();
					else if( currChild.childNodes().item( i ).isElement() )
						Value += processNode( currChild.childNodes().item( i ).toElement() );
				}
			else
				Value = currChild.nodeValue();
			
			if( Value.toInt() < 0 )
				Value = QString("0");

			madeItem->setColor( Value.toInt() );
		}
		else if( currChild.nodeName() == "inherit" && currChild.attributes().contains("id") )
		{
			QDomElement* derivalSection = DefManager->getSection( WPDT_ITEM, currChild.attribute("id") );
			if( !derivalSection->isNull() )
				this->applyNodes( madeItem, derivalSection );
		}
	}
}

void IDefReader::processItemContainerNode( P_ITEM contItem, QDomElement &Node )
{
	//item container can be scripted like this:
	/*
	<contains>
		<item><inherit list="myList" /></item>
		<item><inherit id="myItem1" /><amount><random ... /></amount><color><colorlist><random...></colorlist></color></item>
		...
	</contains>
	*/
	for( UI16 j = 0; j < Node.childNodes().count(); j++ )
		if( Node.childNodes().item( j ).toElement().nodeName() == "item" )
		{
			P_ITEM nItem = Items->MemItemFree();
	
			if( nItem == NULL )
				continue;

			nItem->Init( true );
			cItemsManager::getInstance()->registerItem( nItem );

			this->applyNodes( nItem, &Node.childNodes().item( j ).toElement() );	
			nItem->setContSerial( contItem->serial );
		}
}

QString IDefReader::processNode( QDomElement &Node )
{

}