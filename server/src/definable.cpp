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
//	Wolfpack Homepage: http://www.wpdev.sf.net/
//========================================================================================

#include "definable.h"
#include "wpdefmanager.h"
#include "globals.h"
#include "basics.h"
#include "prototypes.h"

void cDefinable::applyDefinition( const QDomElement& sectionNode )
{
	QDomNode TagNode = sectionNode.firstChild();
	while( !TagNode.isNull() )
	{
		if( TagNode.isElement() )
			this->processNode( TagNode.toElement() );

		TagNode = TagNode.nextSibling();
	}
}

QString cDefinable::getNodeValue( const QDomElement &Tag )
{
	QString Value = QString();
	
	if( !Tag.hasChildNodes() )
		return "";
	else
	{
		QDomNode childNode = Tag.firstChild();
		while( !childNode.isNull() )
		{
			if( !childNode.isElement() )
			{
				if( childNode.isText() )
					Value += childNode.toText().data();
				childNode = childNode.nextSibling();
				continue;
			}
			QDomElement childTag = childNode.toElement();
			if( childTag.nodeName() == "random" )
			{
				if( childTag.attributes().contains("min") && childTag.attributes().contains("max") )
					Value += QString("%1").arg( RandomNum( childTag.attributeNode("min").nodeValue().toInt(), childTag.attributeNode("max").nodeValue().toInt() ) );
				else if( childTag.attributes().contains("valuelist") )
				{
					QStringList RandValues = QStringList::split(",", childTag.attributeNode("list").nodeValue());
					Value += RandValues[ RandomNum(0,RandValues.size()-1) ];
				}
				else if( childTag.attributes().contains( "list" ) )
				{
					Value += DefManager->getRandomListEntry( childTag.attribute( "list" ) );
				}
				else if( childTag.attributes().contains("dice") )
					Value += QString("%1").arg(rollDice(childTag.attributeNode("dice").nodeValue()));
				else
					Value += QString("0");
			}

			// Process the childnodes
			QDomNodeList childNodes = childTag.childNodes();

			for( int i = 0; i < childNodes.count(); i++ )
			{
				if( !childNodes.item( i ).isElement() )
					continue;

				Value += this->getNodeValue( childNodes.item( i ).toElement() );
			}
			childNode = childNode.nextSibling();
		}
	}
	return hex2dec( Value );
}

// global
QString hex2dec( QString value )
{
	bool ok;
	if( (value.left( 2 ) == "0x" || value.left( 2 ) == "0X") )
		return QString("%1").arg(value.right( value.length()-2 ).toInt( &ok, 16 ));
	else 
		return value;
}

