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

#include "wpxmlparser.h"
#include "globals.h"
#include "junk.h"
#include "prototypes.h"
#include "basics.h"
#include "classes.h"

// Library Includes
#include "qdom.h"
#include "qfile.h"

cWPXMLParser::cWPXMLParser( WPDEF_TYPE baseType )
{
	this->setBaseType( baseType );
}

cWPXMLParser::cWPXMLParser( WPDEF_TYPE baseType, QDomElement *baseTag )
{
	this->setBaseTag( baseTag );
	this->setBaseType( baseType );
}

bool cWPXMLParser::prepareParsing( QString Section )
{
	QDomElement *DefSection = DefManager->getSection( this->baseType(), Section );

	if( DefSection->isNull() )
		return false;
	else
		this->setBaseTag( DefSection );

	return true;
}

void cWPXMLParser::applyNodes( P_ITEM Item, QDomElement* Node )
{
	QDomElement Section;
	if( Node == NULL )
		Section = *this->baseTag();
	else
		Section = *Node;

	QDomNodeList Tags = Section.childNodes();

	// at least do the following:
	// name + id
	UI16 i, j;

	for( i = 0; i < Tags.count(); i++ )
	{
		// as usual only elements are processed
		if( !Tags.item( i ).isElement() )
			continue;

		// we do this as we're going to modify the element
		QDomElement Tag = Tags.item( i ).toElement();
		QString TagName = Tag.nodeName();
		QString Value;
		
		// The node value is either the text or it's determined by it's subnodes
		if( Tag.hasChildNodes() && TagName != "content" )
			for( j = 0; j < Tag.childNodes().count(); j++ )
			{
				if( Tag.childNodes().item( j ).isText() )
					Value += Tag.childNodes().item( j ).toText().data();
				else if( Tag.childNodes().item( j ).isElement() )
					Value += processNode( Tag.childNodes().item( j ).toElement() );
			}
			
		// <name>my Item</name>
		if( TagName == "name" )
			Item->setName( Value );
		 
		// <identified>my magic item</identified>
		else if( TagName == "identified" )
			Item->setName2( Value.latin1() );

		// <amount>120</amount>
		else if( TagName == "amount" )
			Item->setAmount( Value.toULong() );

		// <color>480</color>
		else if( TagName == "color" )
			Item->setColor( Value.toUShort( NULL, 16 ) );

		// <events>a,b,c</events>
		//else if( TagName == "color" )
		//	Item->color = Value.toUShort( NULL, 16 );

		// <attack min="1" max="2"/>
		else if( TagName == "attack" )
		{
			if( Tag.attributes().contains( "min" ) )
				Item->setLodamage( Tag.attributeNode( "min" ).nodeValue().toInt() );

			if( Tag.attributes().contains( "max" ) )
				Item->setHidamage( Tag.attributeNode( "max" ).nodeValue().toInt() );

			// Better...
			if( Item->lodamage() > Item->hidamage() )
				Item->setHidamage( Item->lodamage() );
		}

		// <defense>10</defense>
		else if( TagName == "defense" )
			Item->def = Value.toInt();

		// <type>10</type>
		else if( TagName == "type" )
			Item->setType( Value.toUInt() );

		// <weight>10</weight>
		else if( TagName == "weight" )
			Item->setWeight( Value.toUInt() );

		// <value>10</value>
		else if( TagName == "value" )
			Item->value = Value.toInt();
		
		// <restock>10</restock>
		else if( TagName == "restock" )
			Item->restock = Value.toInt();

		// <layer>10</layer>
		else if( TagName == "layer" )
			Item->setLayer( Value.toShort() );

		// <durability>10</durabilty>
		else if( TagName == "durability" )
		{
			Item->setMaxhp( Value.toLong() );
			Item->setHp( Item->maxhp() );
		}

		// <speed>10</speed>
		else if( TagName == "speed" )
			Item->setSpeed( Value.toLong() );

		// <good>10</good>
		else if( TagName == "good" )
			Item->good = Value.toInt();

		// <lightsource>10</lightsource>
		else if( TagName == "lightsource" )
			Item->dir = Value.toUShort();

		// <more1>10</more1>
		else if( TagName == "more1" )
			Item->more1 = Value.toInt();

		// <more>10</more> <<<<< alias for more1
		else if( TagName == "more" )
			Item->more1 = Value.toInt();

		// <more2>10</more2>
		else if( TagName == "more2" )
			Item->more2 = Value.toInt();

		// <morex>10</morex>
		else if( TagName == "morex" )
			Item->morex = Value.toInt();

		// <morex>10</morex>
		else if( TagName == "morex" )
			Item->morex = Value.toInt();

		// <morez>10</morez>
		else if( TagName == "morez" )
			Item->morez = Value.toInt();

		// <morexyz>10</morexyz>
		else if( TagName == "morexyz" )
		{
			QStringList Elements = QStringList::split( ",", Value );
			if( Elements.count() == 3 )
			{
				Item->morex = Elements[ 0 ].toInt();
				Item->morey = Elements[ 1 ].toInt();
				Item->morez = Elements[ 2 ].toInt();
			}
		}

		// <movable />
		// <ownermovable />
		// <immovable />
		else if( TagName == "movable" )
			Item->magic = 1;
		else if( TagName == "immovable" )
			Item->magic = 2;
		else if( TagName == "ownermovable" )
			Item->magic = 3;

		// <decay />
		// <nodecay />
		else if( TagName == "decay" )
			Item->priv |= 0x01;
		else if( TagName == "nodecay" )
			Item->priv &= 0xFE;

		// <pile />
		// <nopile />
		else if( TagName == "pile" )
			Item->setPileable( true );

		else if( TagName == "nopile" )
			Item->setPileable( false );

		// <dispellable />
		// <notdispellable />
		else if( TagName == "dispellable" )
			Item->priv |= 0x04;
		else if( TagName == "notdispellable" )
			Item->priv &= 0xFB;

		// <newbie />
		// <notnewbie />
		else if( TagName == "newbie" )
			Item->priv |= 0x04;
		else if( TagName == "notnewbie" )
			Item->priv &= 0xFB;

		// <itemhand>2</itemhand>
		else if( TagName == "itemhand" )
			Item->setItemhand( Value.toInt() );

		// <racehate>2</racehate>
		else if( TagName == "racehate" )
			Item->setRacehate( Value.toInt() );

		// <restock>2</restock>
		else if( TagName == "restock" )
			Item->restock = Value.toInt();

		// <trigger>2</trigger>
		else if( TagName == "trigger" )
			Item->trigger = Value.toInt();

		// <triggertype>2</triggertype>
		else if( TagName == "triggertype" )
			Item->trigtype = Value.toInt();

		// <smelt>2</smelt>
		else if( TagName == "smelt" )
			Item->setSmelt( Value.toInt() );

		// <requires type="xx">2</requires>
		else if( TagName == "requires" )
		{
			if( !Tag.attributes().contains( "type" ) )
				continue;

			QString Type = Tag.attributeNode( "type" ).nodeValue();
			
			if( Type == "str" )
				Item->st = Value.toULong();
			else if( Type == "dex" )
				Item->dx = Value.toULong();
			else if( Type == "int" )
				Item->in = Value.toULong();
		}

		// <visible />
		// <invisible />
		// <ownervisible />
		else if( TagName == "invisible" )
			Item->visible = 2;
		else if( TagName == "visible" )
			Item->visible = 0;
		else if( TagName == "ownervisible" )
			Item->visible = 1;

		// <modifier type="xx">2</modifier>
		else if( TagName == "modifier" )
		{
			if( !Tag.attributes().contains( "type" ) )
				continue;

			QString Type = Tag.attributeNode( "type" ).nodeValue();
			
			if( Type == "str" )
				Item->st2 = Value.toULong();
			else if( Type == "dex" )
				Item->dx2 = Value.toULong();
			else if( Type == "int" )
				Item->in2 = Value.toULong();
		}

		// <dye />
		// <nodye />
		else if( TagName == "dye" )
			Item->dye = 1;
		else if( TagName == "nodye" )
			Item->dye = 0;

		// <id>12f9</id>
		else if( TagName == "id" )
			Item->setId( Value.toUShort( NULL, 16 ) );

		// <contains><item id="a" />...<item id="z" /></contains> (sereg)
		else if( TagName == "contains" && Tag.hasChildNodes() )
			processItemContainerNode( Item, Tag ); 
	}
}

void cWPXMLParser::applyNodes( P_CHAR Char, QDomElement* Node )
{
	QDomElement Section;
	if( Node == NULL )
		Section = *this->baseTag();
	else
		Section = *Node;

	QDomNodeList Tags = Section.childNodes();

	UI16 i, j;

	for( i = 0; i < Tags.count(); i++ )
	{
		if( !Tags.item( i ).isElement() )
			continue;

		QDomElement Tag = Tags.item( i ).toElement();
		QString TagName = Tag.nodeName();
		QString Value;
		QDomNodeList ChildTags;

		if( Tag.hasChildNodes() && TagName != "backpack" && TagName != "equipped" && TagName != "shopkeeper" )
			for( j = 0; j < Tag.childNodes().count(); j++ )
			{
				if( Tag.childNodes().item( j ).isText() )
					Value += Tag.childNodes().item( j ).toText().data();
				else if( Tag.childNodes().item( j ).isElement() )
					Value += processNode( Tag.childNodes().item( j ).toElement() );
			}

		//<name>my char</name>
		if( TagName == "name" )
			Char->name = Value;
		
		//<backpack>
		//	<color>0x132</color>
		//	<item id="a">
		//	...
		//	<item id="z">
		//</backpack>
		else if( TagName == "backpack" )
			if( Char->packitem == INVALID_SERIAL )
			{
				P_ITEM pBackpack = Items->SpawnItem( -1, Char, 1, "Backpack", 0, 0x0E,0x75,0,0,0);

				if( pBackpack == NULL )
				{
					Npcs->DeleteChar( Char );
					return;
				}

				Char->packitem = pBackpack->serial;
				pBackpack->pos.x = 0;
				pBackpack->pos.y = 0;
				pBackpack->pos.z = 0;
				pBackpack->setContSerial(Char->serial);
				pBackpack->setLayer( 0x15 );
				pBackpack->setType( 1 );
				pBackpack->dye=1;

				if( Tag.hasChildNodes() )
					this->applyNodes( pBackpack, &Tag );
			}

		//<carve>3</carve>
		else if( TagName == "carve" ) 
			Char->setCarve( Value.toInt() );

		//<cantrain />
		else if( TagName == "cantrain" )
			Char->setCantrain( true );

		//<direction>SE</direction>
		else if( TagName == "direction" )
			if( Value == "NE" )
				Char->dir=1;
			else if( Value == "E" )
				Char->dir=2;
			else if( Value == "SE" )
				Char->dir=3;
			else if( Value == "S" )
				Char->dir=4;
			else if( Value == "SW" )
				Char->dir=5;
			else if( Value == "W" )
				Char->dir=6;
			else if( Value == "NW" )
				Char->dir=7;
			else if( Value == "N" )
				Char->dir=0;

		//<stat type="str">100</stats>
		else if( TagName == "stat" )
			if( Tag.attributes().contains("type") )
			{
				QString statType = Tag.attributeNode("type").nodeValue();
				if( statType == "str" )
				{
					Char->st = Value.toShort();
					Char->st2 = Char->st;
					Char->hp = Char->st;
				}
				else if( statType == "dex" )
				{
					Char->setDex( Value.toShort() );
					Char->stm = Char->realDex();
				}
				else if( statType == "int" )
				{
					Char->in = Value.toShort();
					Char->in2 = Char->in;
					Char->mn = Char->in;
				}
			}

		//<defense>10</defense>
		else if( TagName == "defense" )
			Char->def = Value.toUInt();

		//<attack>10</attack>
		else if( TagName == "attack" )
			Char->att = Value.toUInt();

		//<emotecolor>0x482</emotecolor>
		else if( TagName == "emotecolor" )
			Char->emotecolor = Value.toUShort();

		//<fleeat>10</fleeat>
		else if( TagName == "fleeat" )
			Char->setFleeat( Value.toShort() );

		//<fame>8000</fame>
		else if( TagName == "fame" )
			Char->fame = Value.toInt();

		//<gold>100</gold>
		else if( TagName == "gold" )
			if( Char->packitem != INVALID_SERIAL )
			{
				P_ITEM pGold = Items->SpawnItem(Char,1,"#",1,0x0EED,0,1);
				if(pGold == NULL)
				{
					Npcs->DeleteChar(Char);
					return;
				}
				pGold->priv |= 0x01;

				pGold->setAmount( Value.toInt() );
			}
			else
				clConsole.send((char*)QString("Warning: Bad NPC Script %1: no backpack for gold.\n").arg( Section.attributeNode( "id" ).nodeValue() ).latin1());

		//<hidamage>10</hidamage>
		else if( TagName == "hidamage" )
			Char->hidamage = Value.toInt();

#pragma note("Hair color tag not yet implemented!")
/*
		//<haircolor>2</haircolor> (colorlist)
		else if( TagName == "haircolor" )
		{
			unsigned short haircolor = addrandomhaircolor(Char, (char*)Value.latin1());
			if( haircolor != -1 )
				Char->setHairColor( haircolor );
		}
*/

		//<id>0x11</id>
		else if( TagName == "id" )
		{
			Char->setId( Value.toUShort() );
			Char->xid = Char->id();
		}

		//<karma>-500</karma>
		else if( TagName == "karma" )
			Char->karma = Value.toInt();

		//<loot>3</loot>
		else if( TagName == "loot" )
			if( Char->packitem != INVALID_SERIAL )
				Npcs->AddRandomLoot( FindItemBySerial(Char->packitem), (char*)Value.latin1() );
			else
				clConsole.send((char*)QString("Warning: Bad NPC Script %1: no backpack for loot.\n").arg( Section.attributeNode( "id" ).nodeValue() ).latin1());

		//<lodamage>10</lodamage>
		else if( TagName == "lodamage" )
			Char->lodamage = Value.toInt();

		//<notrain />
		else if( TagName == "notrain" )
			Char->setCantrain( false );

		//<npcwander type="rectangle" x1="-10" x2="12" y1="5" y2="7" />
		//<......... type="rect" ... />
		//<......... type="3" ... />
		//<......... type="circle" radius="10" />
		//<......... type="2" ... />
		//<......... type="free" (or "1") />
		//<......... type="none" (or "0") />
		else if( TagName == "npcwander" )
			if( Tag.attributes().contains("type") )
			{
				QString wanderType = Tag.attributeNode("type").nodeValue();
				if( wanderType == "rectangle" || wanderType == "rect" || wanderType == "3" )
					if( Tag.attributes().contains("x1") &&
						Tag.attributes().contains("x2") &&
						Tag.attributes().contains("y1") &&
						Tag.attributes().contains("y2") )
					{
						Char->npcWander = 3;
						Char->fx1 = Char->pos.x + Tag.attributeNode("x1").nodeValue().toInt();
						Char->fx2 = Char->pos.x + Tag.attributeNode("x2").nodeValue().toInt();
						Char->fy1 = Char->pos.y + Tag.attributeNode("y1").nodeValue().toInt();
						Char->fy2 = Char->pos.y + Tag.attributeNode("y2").nodeValue().toInt();
						Char->fz1 = -1;
					}
				else if( wanderType == "circle" || wanderType == "2" )
				{
					Char->npcWander = 2;
					Char->fx1 = Char->pos.x;
					Char->fy1 = Char->pos.y;
					Char->fz1 = Char->pos.z;
					if( Tag.attributes().contains("radius") )
						Char->fx2 = Tag.attributeNode("radius").nodeValue().toInt();
					else
						Char->fx2 = 2;
				}
				else if( wanderType == "free" || wanderType == "1" )
					Char->npcWander = 1;
				else
					Char->npcWander = 0; //default
			}
		//<ai>2</ai>
		else if( TagName == "ai" )
			Char->setNpcAIType( Value.toInt() );

		//<onhorse />
		else if( TagName == "onhorse" )
			Char->setOnHorse( true );
		//<priv1>0</priv1>
		else if( TagName == "priv1" )
			Char->setPriv( Value.toUShort() );

		//<priv2>0</priv2>
		else if( TagName == "priv2" )
			Char->priv2 = Value.toUShort();
		//<poison>2</poison>
		else if( TagName == "poison" )
			Char->setPoison( Value.toInt() );
		//<reattackat>40</reattackat>
		else if( TagName == "reattackat" )
			Char->setReattackat( Value.toShort() );

		//<skin>0x342</skin>
		else if( TagName == "skin" )
		{
			Char->setSkin( Value.toUShort() );
			Char->setXSkin( Value.toUShort() );
		}

		//<shopkeeper>
		//	<sellitems>...handled like item-<contains>-section...</sellitems>
		//	<shopitems>...see above...</shopitems>
		//	<rshopitems>...see above...</rshopitems>
		//</shopkeeper>
		else if( TagName == "shopkeeper" && Tag.hasChildNodes() )
		{
			Commands->MakeShop( Char );
			for( j = 0; j < Tag.childNodes().count(); j++ )
			{
				QDomElement currNode = Tag.childNodes().item( j ).toElement();
				
				if( !currNode.hasChildNodes() )
					continue;

				unsigned char contlayer = 0;
				if( currNode.nodeName() == "rshopitems" )
					contlayer = 0x1A;
				else if( currNode.nodeName() == "shopitems" )
					contlayer = 0x1B;
				else if( currNode.nodeName() == "sellitems" )
					contlayer = 0x1C;
				else 
					continue;
				
				P_ITEM contItem = Char->GetItemOnLayer( contlayer );
				if( contItem != NULL )
					this->processItemContainerNode( contItem, currNode );
				else
					clConsole.send((char*)QString("Warning: Bad NPC Script %1: no shoppack for item.\n").arg( Section.attributeNode( "id" ).nodeValue() ).latin1());
			}
		}
		
		//<spattack>3</spattack>
		else if( TagName == "spattack" )
			Char->spattack = Value.toInt();

		//<speech>13</speech>
		else if( TagName == "speech" )
			Char->speech = Value.toUShort();

		//<split>1</split>
		else if( TagName == "split" )
			Char->setSplit( Value.toUShort() );

		//<splitchance>10</splitchance>
		else if( TagName == "splitchance" )
			Char->setSplitchnc( Value.toUShort() );

		//<saycolor>0x110</saycolor>
		else if( TagName == "saycolor" )
			Char->saycolor = Value.toUShort();

		//<spadelay>3</spadelay>
		else if( TagName == "spadelay" )
			Char->spadelay = Value.toInt();

		//<stablemaster />
		else if( TagName == "stablemaster" )
			Char->setNpc_type(1);

		//<title>the king</title>
		else if( TagName == "title" )
			Char->setTitle( Value );

		//<totame>115</totame>
		else if( TagName == "totame" )
			Char->taming = Value.toInt();

		//<trigger>3</trigger>
		else if( TagName = "trigger" )
			Char->setTrigger( Value.toInt() );

		//<trigword>abc</trigword>
		else if( TagName == "trigword" )
			Char->setTrigword( Value );

		//<skill type="alchemy">100</skill>
		//<skill type="1">100</skill>
		else if( TagName == "skill" && Tag.attributes().contains("type") )
		{
			if( Tag.attributeNode("type").nodeValue().toInt() > 0 &&
				Tag.attributeNode("type").nodeValue().toInt() <= ALLSKILLS )
				Char->setBaseSkill((Tag.attributeNode("type").nodeValue().toInt() - 1), Value.toInt());
			else
				for( j = 0; j < ALLSKILLS; j++ )
					if( Tag.attributeNode("type").nodeValue().contains( QString(skillname[j]), false ) )
						Char->setBaseSkill(j, Value.toInt());
		}

		//<equipped>
		//	<item id="a" />
		//	<item id="b" />
		//	...
		//</epuipped>
		else if( TagName == "equipped" && Tag.hasChildNodes() )
		{
			for( j = 0; j < Tag.childNodes().count(); j++ )
			{
				QDomElement currChild = Tag.childNodes().item( j ).toElement();
				if( currChild.nodeName() == "item" && currChild.attributes().contains("id") )
				{
					P_ITEM nItem = Items->createScriptItem( currChild.attributeNode("id").nodeValue() );
					if( nItem == NULL )
						continue;
					else if( nItem->layer() == 0 )
					{
						clConsole.send((char*)QString("Warning: Bad NPC Script %1: item to equip has no layer.\n").arg( Section.attributeNode( "id" ).nodeValue() ).latin1());
						Items->DeleItem( nItem );
						continue;
					}
					else
						nItem->setContSerial( Char->serial );

					if( currChild.hasChildNodes() )  // color
						this->processScriptItemNode( nItem, currChild );
				}
			}
		}
	}
}

void cWPXMLParser::processScriptItemNode( P_ITEM madeItem, QDomElement &Node )
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

void cWPXMLParser::processItemContainerNode( P_ITEM contItem, QDomElement &Node )
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
