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

#include "resources.h"

#include "globals.h"
#include "prototypes.h"
#include "wpdefmanager.h"
#include "chars.h"
#include "network/uosocket.h"
#include "maps.h"
#include "mapobjects.h"
#include "itemsmgr.h"
#include "skills.h"
#include "srvparams.h"

//System Includes
#include <math.h>

#undef DBGFILE
#define DBGFILE "resources.cpp"


// class cResource

cResource::cResource( const QDomElement &Tag ) : amountmin_( 0 ), amountmax_( 0 ), skillid_( 0 ), refreshtime_( 0 ), veinchance_( 0 )
{
	section_ = Tag.attribute( "id" );
	totalquota_ = 0;
	totalveinquota_ = 0;
	name_ = "";
	deletesource_ = false;
	charaction_ = 0;
	sound_ = 0;
	staminamax_ = 0;
	staminamin_ = 0;
	failmsg_ = "";
	succmsg_ = "";
	emptymsg_ = "";
	applyDefinition( Tag );
}

void cResource::processNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	QString Value = getNodeValue( Tag );

	// <amount>20</amount>
	// <amount min="10" max="20" />
	if( TagName == "amount" )
	{
		if( Tag.hasAttribute( "min" ) && Tag.hasAttribute( "max" ) )
		{
			amountmin_ = hex2dec( Tag.attribute( "min" ) ).toUInt();
			amountmax_ = hex2dec( Tag.attribute( "max" ) ).toUInt();
		}
		else
		{
			amountmin_ = Value.toUInt();
			amountmax_ = amountmin_;
		}
	}

	// <veinchance>5</veinchance>  (in percent)
	else if( TagName == "veinchance" )
		veinchance_ = Value.toUInt();

	// <item>
	//    <name>Mythril</name> (name of the res)
	//    <amount min="1" max="2" /> (how many res per attempt)
	//    <id>0x0191-0x0193</id> (ids, lists, getlist, scopes)
	//    <color><getlist id="colors_mythril" /></color>
	//    <vein min="50" max="80" quota="2" /> (how many res of this kind in a vein)
	//    <skill min="100" max="400" />
	//    <quota>30</quota> (relative chance to find this material)
	//    <artid>0xspecial:)</artid>
	//    <mapid>0xveryspecial</mapid>
	// </item>
	else if( TagName == "item" )
	{
		resourcespec_st item;
		item.conversion.rate = 0;
		item.makeskillmod = 1.0f;
		item.makeuseamountmod = 1.0f;
		item.maxamount_per_attempt = 1;
		item.minamount_per_attempt = 1;
		item.name = QString::null;
		item.definition = QString::null;
		item.quota = 0;
		item.vein_maxamount = 0;
		item.vein_minamount = 0;
		item.vein_quota = 0;

		QDomNode childNode = Tag.firstChild();
		while( !childNode.isNull() )
		{
			if( childNode.isElement() )
			{
				QDomElement childTag = childNode.toElement();
				QString chTagName = childTag.nodeName();
				QString chValue = getNodeValue( childTag );

				if( chTagName == "name" )
					item.name = chValue;
				
				else if( chTagName == "definition" )
					item.definition = chValue;

				else if( chTagName == "amount" )
				{
					if( childTag.hasAttribute( "min" ) && childTag.hasAttribute( "max" ) )
					{
						item.minamount_per_attempt = hex2dec( childTag.attribute( "min" ) ).toUInt();
						item.maxamount_per_attempt = hex2dec( childTag.attribute( "max" ) ).toUInt();
					}
					else
					{
						item.minamount_per_attempt = chValue.toUInt();
						item.maxamount_per_attempt = item.minamount_per_attempt;
					}
				}

				else if( chTagName == "id" )
				{
					Value = childTag.text();
					QDomNode chchNode = childTag.firstChild();
					while( !chchNode.isNull() )
					{
						if( chchNode.isElement() )
						{
							QDomElement chchTag = chchNode.toElement();
							QString chchTagName = chchTag.nodeName();
							QString chchValue = getNodeValue( chchTag );
							if( chchTagName == "getlist" && chchTag.hasAttribute( "id" ) )
							{
								QStringList list = DefManager->getList( chchTag.attribute( "id" ) );
								QStringList::const_iterator it = list.begin();
								while( it != list.end() )
								{
									item.ids.push_back( hex2dec((*it)).toUShort() );
									++it;
								}
							}
						}
						chchNode = chchNode.nextSibling();
					}

					QStringList idstr;
					if( Value.contains( "," ) )
					{
						idstr = QStringList::split( ",", Value );
					}
					else
						idstr.push_back( Value );

					QStringList::const_iterator it = idstr.begin();
					while( it != idstr.end() )
					{
						if( (*it).contains( "-" ) )
						{
							QStringList ids = QStringList::split( "-", (*it) );
							UINT16 minid = hex2dec(ids[0]).toUShort();
							UINT16 maxid = hex2dec(ids[1]).toUShort();
							while( minid <= maxid )
							{
								item.ids.push_back( minid );
								++minid;
							}
						}
						else
							item.ids.push_back( hex2dec((*it)).toUShort() );
						++it;
					}
				}

				else if( chTagName == "color" )
				{
					Value = childTag.text();
					QDomNode chchNode = childTag.firstChild();
					while( !chchNode.isNull() )
					{
						if( chchNode.isElement() )
						{
							QDomElement chchTag = chchNode.toElement();
							QString chchTagName = chchTag.nodeName();
							QString chchValue = getNodeValue( chchTag );
							if( chchTagName == "getlist" && chchTag.hasAttribute( "id" ) )
							{
								QStringList list = DefManager->getList( chchTag.attribute( "id" ) );
								QStringList::const_iterator it = list.begin();
								while( it != list.end() )
								{
									item.colors.push_back( hex2dec((*it)).toUShort() );
									++it;
								}
							}
						}
						chchNode = chchNode.nextSibling();
					}

					QStringList colstr;
					if( Value.contains( "," ) )
					{
						colstr = QStringList::split( ",", Value );
					}
					else
						colstr.push_back( Value );

					QStringList::const_iterator it = colstr.begin();
					while( it != colstr.end() )
					{
						if( (*it).contains( "-" ) )
						{
							QStringList cols = QStringList::split( "-", (*it) );
							UINT16 mincol = hex2dec(cols[0]).toUShort();
							UINT16 maxcol = hex2dec(cols[1]).toUShort();
							while( mincol <= maxcol )
							{
								item.colors.push_back( mincol );
								++mincol;
							}
						}
						else
							item.colors.push_back( hex2dec((*it)).toUShort() );
						++it;
					}
				}

				else if( chTagName == "artid" )
				{
					Value = childTag.text();
					QDomNode chchNode = childTag.firstChild();
					while( !chchNode.isNull() )
					{
						if( chchNode.isElement() )
						{
							QDomElement chchTag = chchNode.toElement();
							QString chchTagName = chchTag.nodeName();
							QString chchValue = getNodeValue( chchTag );
							if( chchTagName == "getlist" && chchTag.hasAttribute( "id" ) )
							{
								QStringList list = DefManager->getList( chchTag.attribute( "id" ) );
								QStringList::const_iterator it = list.begin();
								while( it != list.end() )
								{
									item.artids.push_back( hex2dec((*it)).toUShort() );
									++it;
								}
							}
						}
						chchNode = chchNode.nextSibling();
					}

					QStringList idstr;
					if( Value.contains( "," ) )
					{
						idstr = QStringList::split( ",", Value );
					}
					else
						idstr.push_back( Value );

					QStringList::const_iterator it = idstr.begin();
					while( it != idstr.end() )
					{
						if( (*it).contains( "-" ) )
						{
							QStringList ids = QStringList::split( "-", (*it) );
							UINT16 minid = hex2dec(ids[0]).toUShort();
							UINT16 maxid = hex2dec(ids[1]).toUShort();
							while( minid <= maxid )
							{
								item.artids.push_back( minid );
								++minid;
							}
						}
						else
							item.artids.push_back( hex2dec((*it)).toUShort() );
						++it;
					}
				}

				else if( chTagName == "mapid" )
				{
					Value = childTag.text();
					QDomNode chchNode = childTag.firstChild();
					while( !chchNode.isNull() )
					{
						if( chchNode.isElement() )
						{
							QDomElement chchTag = chchNode.toElement();
							QString chchTagName = chchTag.nodeName();
							QString chchValue = getNodeValue( chchTag );
							if( chchTagName == "getlist" && chchTag.hasAttribute( "id" ) )
							{
								QStringList list = DefManager->getList( chchTag.attribute( "id" ) );
								QStringList::const_iterator it = list.begin();
								while( it != list.end() )
								{
									item.mapids.push_back( hex2dec((*it)).toUShort() );
									++it;
								}
							}
						}
						chchNode = chchNode.nextSibling();
					}

					QStringList idstr;
					if( Value.contains( "," ) )
					{
						idstr = QStringList::split( ",", Value );
					}
					else
						idstr.push_back( Value );

					QStringList::const_iterator it = idstr.begin();
					while( it != idstr.end() )
					{
						if( (*it).contains( "-" ) )
						{
							QStringList ids = QStringList::split( "-", (*it) );
							UINT16 minid = hex2dec(ids[0]).toUShort();
							UINT16 maxid = hex2dec(ids[1]).toUShort();
							while( minid <= maxid )
							{
								item.mapids.push_back( minid );
								++minid;
							}
						}
						else
							item.mapids.push_back( hex2dec((*it)).toUShort() );
						++it;
					}
				}

				else if( chTagName == "vein" )
				{
					item.vein_minamount = hex2dec( childTag.attribute( "min" ) ).toUInt();
					item.vein_maxamount = hex2dec( childTag.attribute( "max" ) ).toUInt();
					item.vein_quota = hex2dec( childTag.attribute( "quota" ) ).toUInt() + totalveinquota_;
					totalveinquota_ = item.vein_quota;
				}

				else if( chTagName == "skill" )
				{
					item.minskill = hex2dec( childTag.attribute( "min" ) ).toUShort();
					item.maxskill = hex2dec( childTag.attribute( "max" ) ).toUShort();
				}

				else if( chTagName == "quota" )
				{
					item.quota = chValue.toUInt() + totalquota_;
					totalquota_ = item.quota;
				}

				else if( chTagName == "conversion" )
				{
					if( childTag.hasAttribute("srccolors") )
					{
						Value = childTag.attribute("srccolors");
	
						QStringList idstr;
						if( Value.contains( "," ) )
						{
							idstr = QStringList::split( ",", Value );
						}
						else
							idstr.push_back( Value );

						QStringList::const_iterator it = idstr.begin();
						while( it != idstr.end() )
						{
							if( (*it).contains( "-" ) )
							{
								QStringList ids = QStringList::split( "-", (*it) );
								UINT16 minid = hex2dec(ids[0]).toUShort();
								UINT16 maxid = hex2dec(ids[1]).toUShort();
								while( minid <= maxid )
								{
									item.conversion.sourcecolors.push_back( minid );
									sourcecolors_.insert( minid );
									++minid;
								}
							}
							else
							{
								item.conversion.sourcecolors.push_back( hex2dec((*it)).toUShort() );
								sourcecolors_.insert( hex2dec((*it)).toUShort() );
							}
							++it;
						}
					}
					
					if( childTag.hasAttribute("srcids") )
					{
						Value = childTag.attribute("srcids");
	
						QStringList idstr;
						if( Value.contains( "," ) )
						{
							idstr = QStringList::split( ",", Value );
						}
						else
							idstr.push_back( Value );

						QStringList::const_iterator it = idstr.begin();
						while( it != idstr.end() )
						{
							if( (*it).contains( "-" ) )
							{
								QStringList ids = QStringList::split( "-", (*it) );
								UINT16 minid = hex2dec(ids[0]).toUShort();
								UINT16 maxid = hex2dec(ids[1]).toUShort();
								while( minid <= maxid )
								{
									item.conversion.sourceids.push_back( minid );
									sourceids_.insert( minid );
									++minid;
								}
							}
							else
							{
								item.conversion.sourceids.push_back( hex2dec((*it)).toUShort() );
								sourceids_.insert( hex2dec((*it)).toUShort() );
							}
							++it;
						}
					}

					if( childTag.hasAttribute( "rate" ) )
						item.conversion.rate = childTag.attribute( "rate" ).toFloat();
				}
			
				// <modifier type="useamount">1.3</modifier>
				// <modifier type="skill">1.4</modifier>
				else if( chTagName == "modifier" )
				{
					if( childTag.attribute("type") == "useamount" )
						item.makeuseamountmod = chValue.toFloat();
					else if( childTag.attribute("type") == "skill" )
						item.makeskillmod = chValue.toFloat();
				}
			}

			childNode = childNode.nextSibling();
		}

		resourcespecs_.push_back( item );
	}

	// <mapid>0xbla</mapid> (for using tools on map tiles)
	else if( TagName == "mapid" )
	{
		Value = Tag.text();
		QDomNode chNode = Tag.firstChild();
		while( !chNode.isNull() )
		{
			if( chNode.isElement() )
			{
				QDomElement chTag = chNode.toElement();
				QString chTagName = chTag.nodeName();
				QString chValue = getNodeValue( chTag );
				if( chTagName == "getlist" && chTag.hasAttribute( "id" ) )
				{
					QStringList list = DefManager->getList( chTag.attribute( "id" ) );
					QStringList::const_iterator it = list.begin();
					while( it != list.end() )
					{
						mapids_.push_back( hex2dec((*it)).toUShort() );
						++it;
					}
				}
			}
			chNode = chNode.nextSibling();
		}

		QStringList idstr;
		if( Value.contains( "," ) )
		{
			idstr = QStringList::split( ",", Value );
		}
		else
			idstr.push_back( Value );

		QStringList::const_iterator it = idstr.begin();
		while( it != idstr.end() )
		{
			if( (*it).contains( "-" ) )
			{
				QStringList ids = QStringList::split( "-", (*it) );
				UINT16 minid = hex2dec(ids[0]).toUShort();
				UINT16 maxid = hex2dec(ids[1]).toUShort();
				while( minid <= maxid )
				{
					mapids_.push_back( minid );
					++minid;
				}
			}
			else
				mapids_.push_back( hex2dec( (*it) ).toUShort() );
			++it;
		}
	}

	// <artid>0xbla</artid> (for using tools on dynamic & static items)
	else if( TagName == "artid" )
	{
		Value = Tag.text();
		QDomNode chNode = Tag.firstChild();
		while( !chNode.isNull() )
		{
			if( chNode.isElement() )
			{
				QDomElement chTag = chNode.toElement();
				QString chTagName = chTag.nodeName();
				QString chValue = getNodeValue( chTag );
				if( chTagName == "getlist" && chTag.hasAttribute( "id" ) )
				{
					QStringList list = DefManager->getList( chTag.attribute( "id" ) );
					QStringList::const_iterator it = list.begin();
					while( it != list.end() )
					{
						artids_.push_back( hex2dec( (*it) ).toUShort() );
						++it;
					}
				}
			}
			chNode = chNode.nextSibling();
		}

		QStringList idstr;
		if( Value.contains( "," ) )
		{
			idstr = QStringList::split( ",", Value );
		}
		else
			idstr.push_back( Value );

		QStringList::const_iterator it = idstr.begin();
		while( it != idstr.end() )
		{
			if( (*it).contains( "-" ) )
			{
				QStringList ids = QStringList::split( "-", (*it) );
				UINT16 minid = hex2dec( ids[0] ).toUShort();
				UINT16 maxid = hex2dec( ids[1] ).toUShort();
				while( minid <= maxid )
				{
					artids_.push_back( minid );
					++minid;
				}
			}
			else
				artids_.push_back( hex2dec( (*it) ).toUShort() );
			++it;
		}
	}

	// <skillid>1</skillid>
	else if( TagName == "skillid" )
		skillid_ = Value.toUShort();

	// <refreshtime>120</refreshtime> (in seconds)
	else if( TagName == "refreshtime" )
		refreshtime_ = Value.toUInt();

	// <name>ore</name>
	else if( TagName == "name" )
		name_ = Value;

	// <deletesource /> (only works with dynamics)
	else if( TagName == "deletesource" )
		deletesource_ = true;

	// <stamina>1</stamina>
	// <stamina min="1" max="2" />
	else if( TagName == "stamina" )
	{
		if( Tag.hasAttribute( "min" ) && Tag.hasAttribute( "max" ) )
		{
			staminamin_ = hex2dec( Tag.attribute( "min" ) ).toUInt();
			staminamax_ = hex2dec( Tag.attribute( "max" ) ).toUInt();
		}
		else
		{
			staminamin_ = Value.toUInt();
			staminamax_ = amountmin_;
		}
	}

	// <charaction>0x0E</charaction>
	else if( TagName == "charaction" )
		charaction_ = Value.toUShort();

	// <sound>0x01</sound>
	else if( TagName == "sound" )
		sound_ = Value.toUShort();

	// <failmsg>text</failmsg>
	else if( TagName == "failmsg" )
		failmsg_ = Value;

	// <succmsg>text</succmsg>
	else if( TagName == "succmsg" )
		succmsg_ = Value;

	// <emptymsg>text</emptymsg>
	else if( TagName == "emptymsg" )
		emptymsg_ = Value;
}

bool cResource::hasArtId( UINT16 id )
{
	QValueVector< UINT16 >::iterator it = artids_.begin();
	while( it != artids_.end() )
	{
		if( (*it) == id )
			return true;
		++it;
	}
	return false;
}

bool cResource::hasMapId( UINT16 id )
{
	QValueVector< UINT16 >::iterator it = mapids_.begin();
	while( it != mapids_.end() )
	{
		if( (*it) == id )
			return true;
		++it;
	}
	return false;
}

void cResource::handleFindTarget( cUOSocket* socket, Coord_cl pos, UINT16 mapid, UINT16 artid )
{
	P_CHAR pc = socket->player();
	if( !socket || !pc )
		return;

	cResourceItem* pResItem = NULL;
	RegionIterator4Items ri( pos );
	for (ri.Begin(); !ri.atEnd() && !pResItem; ri++)
	{
		P_ITEM pi = ri.GetData();
		if( pi && pi->pos().x == pos.x && pi->pos().y == pos.y && pi->objectID() == "cResourceItem" )
		{
			pResItem = dynamic_cast< cResourceItem* >(pi);
			if( pResItem->resource() != section_ )
				pResItem = NULL;
		}
	}

	resourcespec_st item;
	QMap< UINT32, resourcespec_st > possible_resspecs;
	QMap< UINT32, resourcespec_st >::iterator mit;
	QValueVector< resourcespec_st >::iterator it;
	QValueVector< UINT16 >::iterator vit;

	it = resourcespecs_.begin();
	UINT32 i = 0;
	// this one checks which items can be found on the artid/mapid
	while( it != resourcespecs_.end() )
	{
		if( (*it).artids.size() == 0 && (*it).mapids.size() == 0 )
			possible_resspecs.insert( i, (*it) );
		else if( artid > 0 )
		{
			if( (*it).artids.size() == 0 )
				possible_resspecs.insert( i, (*it) );
			else
			{
				vit = (*it).artids.begin();
				while( vit != (*it).artids.end() )
				{
					if( (*vit) == artid )
					{
						possible_resspecs.insert( i, (*it) );
						break;
					}
					++vit;
				}
			}
		}
		else if( mapid > 0 )
		{
			if( (*it).mapids.size() == 0 )
				possible_resspecs.insert( i, (*it) );
			else
			{
				vit = (*it).mapids.begin();
				while( vit != (*it).mapids.end() )
				{
					if( (*vit) == mapid )
					{
						possible_resspecs.insert( i, (*it) );
						break;
					}
					++vit;
				}
			}
		}
		++i;
		++it;
	}

	if( possible_resspecs.size() == 0 )
	{
		pc->action( charaction_ );
		pc->soundEffect( sound_, true );
		if( failmsg_.left( 7 ) == "cliloc:" )
		{
			QString id = failmsg_.right( failmsg_.length() - 7 );
			QStringList parts = QStringList::split( ",", id );

			if( parts.count() == 3 )
			{
				INT32 type = parts[0].toInt();
				INT32 file = parts[1].toInt();
				INT32 id = parts[2].toInt();

				socket->clilocMessage( type, file, id );
			}
		}
		else
			socket->sysMessage( failmsg_ );
		return;
	}

	UINT32 amount = 0;
	UINT32 vein = 0;
	// if there is no resourceitem, create a new one
	if( !pResItem )
	{
		if( RandomNum( 0, 100 ) <= veinchance_ && totalveinquota_ > 0 )
		{
			UINT32 veinquota = RandomNum( 1, totalveinquota_ );
			mit = possible_resspecs.begin();
			while( mit != possible_resspecs.end() )
			{
				vein = mit.key()+1;
				if( veinquota <= mit.data().vein_quota )
				{
					item = mit.data();
					break;
				}
				++mit;
			}
			if( item.vein_maxamount > item.vein_minamount )
				amount = RandomNum( item.vein_minamount, item.vein_maxamount );
			else
				amount = item.vein_minamount;
		}
		else
		{
			if( amountmax_ > amountmin_ )
				amount = RandomNum( amountmin_, amountmax_ );
			else
				amount = amountmin_;
		}

		if( amount > 0 )
		{
			pResItem = new cResourceItem( section_, amount, vein );
			if( pResItem )
			{
				pResItem->serial = ItemsManager::instance()->getUnusedSerial();
				ItemsManager::instance()->registerItem( pResItem );
				pResItem->setPos( pos );
				MapObjects::instance()->add( pResItem );
				pResItem->update();
			}
		}
		else
		{
			if( vein > 0 )
				clConsole.send( tr("ERROR: Wrong vein-amount definition in resource def. %1 (item %2)!").arg(section_).arg(vein) );
			else
				clConsole.send( tr("ERROR: Wrong amount definition in resource def. %1!").arg(section_) );
			return;
		}
	}

	// the amount is stored in morex, vein in morey!
	amount = pResItem->morex();
	if( amount == 0 )
	{
		pc->action( charaction_ );
		pc->soundEffect( sound_, true );
		if( emptymsg_.left( 7 ) == "cliloc:" )
		{
			QString id = emptymsg_.right( emptymsg_.length() - 7 );
			QStringList parts = QStringList::split( ",", id );

			if( parts.count() == 3 )
			{
				INT32 type = parts[0].toInt();
				INT32 file = parts[1].toInt();
				INT32 id = parts[2].toInt();

				socket->clilocMessage( type, file, id );
			}
		}
		else
			socket->sysMessage( emptymsg_ );
		return;
	}
	vein = pResItem->morey();


	if( vein > 0 && vein <= resourcespecs_.size() && possible_resspecs.find( vein-1 ) != possible_resspecs.end() )
		item = resourcespecs_[ vein-1 ];
	else
	{
		if( totalquota_ > 0 )
		{
			UINT32 quota = RandomNum( 1, totalquota_ );
			mit = possible_resspecs.begin();
			while( mit != possible_resspecs.end() )
			{
				if( quota <= mit.data().quota )
				{
					item = mit.data();
					break;
				}
				++mit;
			}
		}
		else
		{
			while( (mit = possible_resspecs.find( RandomNum( 0, resourcespecs_.size()-1 ) )) == possible_resspecs.end() )
				;
			item = mit.data();
		}
	}

	UINT32 spawnamount = 0;
	if( item.maxamount_per_attempt > item.minamount_per_attempt )
		spawnamount = RandomNum( item.minamount_per_attempt, item.maxamount_per_attempt );
	else
		spawnamount = item.minamount_per_attempt;

	if( staminamax_ > staminamin_ )
		pc->setStm(pc->stm() - RandomNum( staminamin_, staminamax_ ) );
	else
		pc->setStm( pc->stm() - staminamin_ );	
	if( pc->stm() < 0 )
		pc->setStm(0);

	if( !pc->checkSkill( skillid_, item.minskill, item.maxskill ) )
	{
		pc->action( charaction_ );
		pc->soundEffect( sound_, true );
		if( failmsg_.left( 7 ) == "cliloc:" )
		{
			QString id = failmsg_.right( failmsg_.length() - 7 );
			QStringList parts = QStringList::split( ",", id );

			if( parts.count() == 3 )
			{
				INT32 type = parts[0].toInt();
				INT32 file = parts[1].toInt();
				INT32 id = parts[2].toInt();

				socket->clilocMessage( type, file, id );
			}
		}
		else
			socket->sysMessage( failmsg_ );
		return;
	}

	if( pc->skill( skillid_ ) < item.minskill )
		spawnamount = 0;
	else if( pc->skill( skillid_ ) < item.maxskill && item.maxskill > item.minskill )
	{
		spawnamount = (UINT16)( ceil( (double)(pc->skill( skillid_ )-item.minskill) / (double)(item.maxskill - item.minskill) * (double)spawnamount) );
		if( spawnamount == 0 )
		{
			spawnamount = 1;
		}
	}

	if( spawnamount > amount )
		spawnamount = amount;
	
	if( spawnamount == 0 )
	{
		pc->action( charaction_ );
		pc->soundEffect( sound_, true );
		if( failmsg_.left( 7 ) == "cliloc:" )
		{
			QString id = failmsg_.right( failmsg_.length() - 7 );
			QStringList parts = QStringList::split( ",", id );

			if( parts.count() == 3 )
			{
				INT32 type = parts[0].toInt();
				INT32 file = parts[1].toInt();
				INT32 id = parts[2].toInt();

				socket->clilocMessage( type, file, id );
			}
		}
		else
			socket->sysMessage( failmsg_ );
		return;
	}
	
	amount -= spawnamount;
	pResItem->setMoreX(amount);
	if( amount == 0 )
	{
		pResItem->setDecayTime(uiCurrentTime + refreshtime_ * MY_CLOCKS_PER_SEC);
		pResItem->priv |= 0x01; // let the item decay
		pResItem->update();
	}

	if( succmsg_.left( 7 ) == "cliloc:" )
	{
		QString id = succmsg_.right( succmsg_.length() - 7 );
		QStringList parts = QStringList::split( ",", id );

			if( parts.count() == 3 )
			{
				INT32 type = parts[0].toInt();
				INT32 file = parts[1].toInt();
				INT32 id = parts[2].toInt();

				socket->clilocMessage( type, file, id );
			}
	}
	else
	{
		if( !item.name.isNull() || !item.name.isEmpty() )
			socket->sysMessage( tr("%5 %1 %2 %3%4 !").arg( spawnamount ).arg( item.name ).arg( name_ ).arg( spawnamount > 1 ? tr("s") : QString("") ).arg( succmsg_ ) );
		else
			socket->sysMessage( tr("%4 %1 %2%3 !").arg( spawnamount ).arg( name_ ).arg( spawnamount > 1 ? tr("s") : QString("") ).arg( succmsg_ ) );
	}

	pc->action( charaction_ );
	pc->soundEffect( sound_, true );

	P_ITEM pi;
	P_ITEM pBackpack = pc->getBackpack();

	if( !pBackpack )
		return;

	while( spawnamount > 0 )
	{
		UINT16 id = item.ids[ RandomNum( 0, item.ids.size()-1 ) ];
		UINT16 color = item.colors[ RandomNum( 0, item.colors.size()-1 ) ];

		if( !item.definition.isNull() )
		{
			pi = Items->createScriptItem( item.definition );
			pi->setId( id );
			pi->setColor( color );

			if( !item.name.isNull() )
				pi->setName( item.name );

			pBackpack->addItem( pi );
			socket->sendStatWindow();
		}

		spawnamount -= 1;
	}
}

void cResource::handleConversionTarget( cUOSocket* socket, Coord_cl pos, cItem* pSource, UINT16 mapid, UINT16 artid )
{
	P_CHAR pc = socket->player();
	if( !socket || !pc || !pSource )
		return;

	std::set< UINT16 >::iterator sit = sourceids_.find( pSource->id() );
	if( sit == sourceids_.end() )
	{
		if( emptymsg_.left( 7 ) == "cliloc:" )
		{
			QString id = emptymsg_.right( emptymsg_.length() - 7 );
			QStringList parts = QStringList::split( ",", id );

			if( parts.count() == 3 )
			{
				INT32 type = parts[0].toInt();
				INT32 file = parts[1].toInt();
				INT32 id = parts[2].toInt();

				socket->clilocMessage( type, file, id );
			}
		}
		else
			socket->sysMessage( emptymsg_ );
		return;
	}
	else
	{
		sit = sourcecolors_.find( pSource->color() );
		if( sit == sourcecolors_.end() )
		{
			if( emptymsg_.left( 7 ) == "cliloc:" )
			{
				QString id = emptymsg_.right( emptymsg_.length() - 7 );
				QStringList parts = QStringList::split( ",", id );

				if( parts.count() == 3 )
				{
					INT32 type = parts[0].toInt();
					INT32 file = parts[1].toInt();
					INT32 id = parts[2].toInt();

					socket->clilocMessage( type, file, id );
				}
			}
			else
				socket->sysMessage( emptymsg_ );
			return;
		}
	}

	resourcespec_st item;
	QValueVector< resourcespec_st > possible_resspecs;
	QValueVector< resourcespec_st >::iterator it;
	QValueVector< UINT16 >::iterator vit;

	it = resourcespecs_.begin();
	// this one checks which items can be found on the artid/mapid
	while( it != resourcespecs_.end() )
	{
		bool isValidSpot = false;
		if( (*it).conversion.sourceids.size() == 0 || (*it).conversion.sourcecolors.size() == 0 )
		{
			++it;
			continue;
		}
		else if( (*it).artids.size() == 0 && (*it).mapids.size() == 0 )
		{
			isValidSpot = true;
		}
		else if( artid > 0 )
		{
			if( (*it).artids.size() == 0 )
				isValidSpot = true;
			else
			{
				vit = (*it).artids.begin();
				while( vit != (*it).artids.end() )
				{
					if( (*vit) == artid )
					{
						isValidSpot = true;
						break;
					}
					++vit;
				}
			}
		}
		else if( mapid > 0 )
		{
			if( (*it).mapids.size() == 0 )
				isValidSpot = true;
			else
			{
				vit = (*it).mapids.begin();
				while( vit != (*it).mapids.end() )
				{
					if( (*vit) == mapid )
					{
						isValidSpot = true;
						break;
					}
					++vit;
				}
			}
		}

		if( isValidSpot )
		{
			bool isValidId = false;
			vit = (*it).conversion.sourceids.begin();
			while( vit != (*it).conversion.sourceids.end() )
			{
				if( (*vit) == pSource->id() )
				{
					isValidId = true;
					break;
				}
				++vit;
			}
			if( isValidId )
			{
				vit = (*it).conversion.sourcecolors.begin();
				while( vit != (*it).conversion.sourcecolors.end() )
				{
					if( (*vit) == pSource->color() )
					{
						possible_resspecs.push_back( (*it) );
						break;
					}
					++vit;
				}
			}
		}
		++it;
	}

	if( possible_resspecs.size() == 0 )
	{
		if( emptymsg_.left( 7 ) == "cliloc:" )
		{
			QString id = emptymsg_.right( emptymsg_.length() - 7 );
			QStringList parts = QStringList::split( ",", id );

			if( parts.count() == 3 )
			{
				INT32 type = parts[0].toInt();
				INT32 file = parts[1].toInt();
				INT32 id = parts[2].toInt();

				socket->clilocMessage( type, file, id );
			}
		}
		else
			socket->sysMessage( emptymsg_ );
		return;
	}

	item.conversion.rate = 0;
	if( totalquota_ > 0 )
	{
		UINT32 quota = RandomNum( 1, totalquota_ );
		it = possible_resspecs.begin();
		while( it != possible_resspecs.end() )
		{
			if( quota <= (*it).quota )
			{
				item = (*it);
				break;
			}
			++it;
		}
	}

	if( item.conversion.rate == 0 )
	{
		item = possible_resspecs[ RandomNum( 0, possible_resspecs.size()-1 ) ];
	}

	UINT32 spawnamount = (UINT32)ceil((float)pSource->amount() * item.conversion.rate);

	if( staminamax_ > staminamin_ )
//		pc->stm -= RandomNum( staminamin_, staminamax_ );
		pc->setStm(pc->stm() - RandomNum( staminamin_, staminamax_ ) );
	else
//		pc->stm -= staminamin_;
		pc->setStm( pc->stm() - staminamin_ );
	if( pc->stm() < 0 )
		pc->setStm(0);

	if( !pc->checkSkill( skillid_, item.minskill, item.maxskill ) )
	{
		pc->action( charaction_ );
		pc->soundEffect( sound_, true );
		UINT16 delamount = RandomNum( 1, pSource->amount() );
		if( delamount == pSource->amount() )
			Items->DeleItem( pSource );
		else
			pSource->setAmount( pSource->amount() - delamount );
		if( failmsg_.left( 7 ) == "cliloc:" )
		{
			QString id = failmsg_.right( failmsg_.length() - 7 );
			QStringList parts = QStringList::split( ",", id );

			if( parts.count() == 3 )
			{
				INT32 type = parts[0].toInt();
				INT32 file = parts[1].toInt();
				INT32 id = parts[2].toInt();

				socket->clilocMessage( type, file, id );
			}
		}
		else
			socket->sysMessage( failmsg_ );
		return;
	}

	if( pc->skill( skillid_ ) < item.minskill )
		spawnamount = 0;
	else if( pc->skill( skillid_ ) < item.maxskill && item.maxskill > item.minskill )
	{
		spawnamount = (UINT16)( ceil( (double)(pc->skill( skillid_ )-item.minskill) / (double)(item.maxskill - item.minskill) * (double)spawnamount) );
		if( spawnamount == 0 )
		{
			spawnamount = 1;
		}
	}

	if( spawnamount == 0 )
	{
		pc->action( charaction_ );
		pc->soundEffect( sound_, true );
		Items->DeleItem( pSource );
		if( failmsg_.left( 7 ) == "cliloc:" )
		{
			QString id = failmsg_.right( failmsg_.length() - 7 );
			QStringList parts = QStringList::split( ",", id );

			if( parts.count() == 3 )
			{
				INT32 type = parts[0].toInt();
				INT32 file = parts[1].toInt();
				INT32 id = parts[2].toInt();

				socket->clilocMessage( type, file, id );
			}
		}
		else
			socket->sysMessage( failmsg_ );
		return;
	}
	
	Items->DeleItem( pSource );
	
	if( succmsg_.left( 7 ) == "cliloc:" )
	{
		QString id = succmsg_.right( succmsg_.length() - 7 );
		QStringList parts = QStringList::split( ",", id );

		if( parts.count() == 3 )
		{
			INT32 type = parts[0].toInt();
			INT32 file = parts[1].toInt();
			INT32 id = parts[2].toInt();

			socket->clilocMessage( type, file, id );
		}
	}
	else
	{
		if( !item.name.isNull() || !item.name.isEmpty() )
			socket->sysMessage( tr("%5 %1 %2 %3%4 !").arg( spawnamount ).arg( item.name ).arg( name_ ).arg( spawnamount > 1 ? tr("s") : QString("") ).arg( succmsg_ ) );
		else
			socket->sysMessage( tr("%4 %1 %2%3 !").arg( spawnamount ).arg( name_ ).arg( spawnamount > 1 ? tr("s") : QString("") ).arg( succmsg_ ) );
	}

	P_ITEM pi;
	while( spawnamount > 0 )
	{
		UINT16 id = item.ids[ RandomNum( 0, item.ids.size()-1 ) ];
		UINT16 color = item.colors[ RandomNum( 0, item.colors.size()-1 ) ];
		if( !item.name.isNull() )
			pi = Items->SpawnItem( pc, 1, (char*)QString("%1 %2").arg( item.name ).arg( name_ ).latin1(), true, id, color, true );
		else
			pi = Items->SpawnItem( pc, 1, (char*)QString("%1").arg( name_ ).latin1(), true, id, color, true );

		spawnamount -= 1;
	}
}

// class cResourceItem

cResourceItem::cResourceItem( const QString& resource, UINT32 amount, UINT32 vein )
{
	cItem::Init( false );
	resource_ = resource;
	if( SrvParams->resitemdecaytime() == 0 )
		priv = 0; // nodecay
	else
	{
		priv |= 0x01;
		setDecayTime(uiCurrentTime + SrvParams->resitemdecaytime() * MY_CLOCKS_PER_SEC);
	}
	setMoreX(amount);
	setMoreY(vein);
	this->setId( 0x1ea7 );
	this->amount_ = 1;
	this->setName( tr("resitem: %1").arg(resource) );
	this->setName2("#");
	this->visible = 2; // gm visible
}

// class cAllResources

cAllResources::cAllResources()
{
}

cAllResources::~cAllResources()
{
	unload();
}

void cAllResources::load()
{
	QStringList sections = DefManager->getSections( WPDT_RESOURCE );
	QStringList::const_iterator it = sections.begin();
	while( it != sections.end() )
	{
		const QDomElement* DefSection = DefManager->getSection( WPDT_RESOURCE, (*it) );
		cResource* pResource = new cResource( *DefSection );
		if( pResource )
		{
			insert( make_pair< QString, cResource* >( (*it), pResource ) );

			QDomDocument doc( "definitions" );
			QValueVector< cResource::resourcespec_st > specs = pResource->resourceSpecs();
			QValueVector< cResource::resourcespec_st >::iterator sit = specs.begin();
			while( sit != specs.end() )
			{
				QDomElement section = doc.createElement( "item" );
				section.setAttribute( "id", QString("%1_%2").arg( (*sit).name.lower() ).arg( (*it) ) );
				doc.appendChild( section );
				QDomElement itemid = doc.createElement( "id" );
				section.appendChild( itemid );
				QDomText itemidtxt = doc.createTextNode( QString::number( (*sit).ids[0] ) );
				itemid.appendChild( itemidtxt );
				QDomElement itemcolor = doc.createElement( "color" );
				section.appendChild( itemcolor );
				QDomText itemcolortxt = doc.createTextNode( QString::number( (*sit).colors[0] ) );
				itemcolor.appendChild(itemcolortxt );
				QDomElement itemname = doc.createElement( "name" );
				section.appendChild( itemname );
				QDomText itemnametxt = doc.createTextNode( QString("%1 %2").arg( (*sit).name ).arg( pResource->name() ) );
				itemname.appendChild( itemnametxt );

				DefManager->ProcessNode( section );

				++sit;
			}
		}
		++it;
	}
}

void cAllResources::unload()
{
	iterator it = begin();
	while( it != end() )
	{
		delete it->second;
		++it;
	}
	clear();
}

void cAllResources::reload()
{
	unload();
	load();
}


// class cFindResource

bool cFindResource::responsed( cUOSocket *socket, cUORxTarget *target )
{
	if( target->x() == 0xFFFF || target->y() == 0xFFFF || target->z() == 0xFF || !socket->player() )
		return true;

	P_CHAR pc = socket->player();
	Coord_cl pos = pc->pos();
	pos.x = target->x();
	pos.y = target->y();
	pos.z = target->z();

	if( pc->skilldelay() > uiCurrentTime && !pc->isGM() )
	{
		socket->sysMessage( tr( "You must wait a few moments before using another skill." ) );
		return true;
	}
	
	if( ( pc->pos().distance( pos ) > 4 ) ) //|| !lineOfSight( pChar->pos(), pos, DOORS|ROOFING_SLANTED|WALLS_CHIMNEYS ) )
	{
		socket->sysMessage( tr( "You can't reach this" ) );
		return false;
	}
	
	// Check if there IS what we have targetted at the given position
	// If not we eiter have wrong statics clientside or a 
	// cheater.
	if( target->serial() )
	{
		P_ITEM pTarget = FindItemBySerial( target->serial() );
		if( !pTarget || pTarget->pos() != pos )
			return true;
	}
	else if( target->model() )
	{
		StaticsIterator msi = Map->staticsIterator(pos);
		bool found = false;
		while( !msi.atEnd() )
		{
			if( msi->itemid == target->model() )
			{
				found = true;
				break;
			}
			++msi;
		}
		if( !found )
			return true;
	}

	cAllResources::iterator it = Resources::instance()->find( resourcename_ );
	if( it != Resources::instance()->end() )
	{
		cResource* pResource = it->second;
		if( !target->model() && !target->serial() ) // map tile
		{
			map_st mapTile = Map->seekMap( pos );
			if( pResource->hasMapId( mapTile.id ) )
				pResource->handleFindTarget( socket, pos, mapTile.id, 0 );
			else
				socket->sysMessage( tr("You cannot find anything here!") );

			pc->setSkillDelay( uiCurrentTime + SrvParams->skillDelay() * MY_CLOCKS_PER_SEC );
			return true;
		}
		else if( target->model() )
		{
			if( pResource->hasArtId( target->model() ) )
			{
				pResource->handleFindTarget( socket, pos, 0, target->model() );
				if( pResource->deleteSource() && target->serial() )
				{
					P_ITEM pi = FindItemBySerial( target->serial() );
					if( pi )
						Items->DeleItem( pi );
				}
			}
			else
				socket->sysMessage( tr("You cannot find anything here!") );

			pc->setSkillDelay( uiCurrentTime + SrvParams->skillDelay() * MY_CLOCKS_PER_SEC );
			return true;
		}
		return false;
	}
	else
	{
		clConsole.send( tr("ERROR: Resource definition %1 not found!").arg( resourcename_ ) );
		return true;
	}
}


// class cConvertResource

bool cConvertResource::responsed( cUOSocket *socket, cUORxTarget *target )
{
	if( target->x() == 0xFFFF || target->y() == 0xFFFF || target->z() == 0xFF || !socket->player() )
		return true;

	P_ITEM pi = FindItemBySerial( sourceserial_ );
	if( !pi )
		return true;

	P_CHAR pc = socket->player();
	Coord_cl pos = pc->pos();
	pos.x = target->x();
	pos.y = target->y();
	pos.z = target->z();

	if( pc->skilldelay() > uiCurrentTime && !pc->isGM() )
	{
		socket->sysMessage( tr( "You must wait a few moments before using another skill." ) );
		return true;
	}
	
	if( ( pc->pos().distance( pos ) > 4 ) ) //|| !lineOfSight( pChar->pos(), pos, DOORS|ROOFING_SLANTED|WALLS_CHIMNEYS ) )
	{
		socket->sysMessage( tr( "You can't reach this" ) );
		return false;
	}
	
	// Check if there IS what we have targetted at the given position
	// If not we eiter have wrong statics clientside or a 
	// cheater.
	if( target->serial() )
	{
		P_ITEM pTarget = FindItemBySerial( target->serial() );
		if( !pTarget || pTarget->pos() != pos )
			return true;
	}
	else if( target->model() )
	{
		StaticsIterator msi = Map->staticsIterator(pos);
		bool found = false;
		while( !msi.atEnd() )
		{
			if( msi->itemid == target->model() )
			{
				found = true;
				break;
			}
			++msi;
		}
		if( !found )
			return true;
	}

	cAllResources::iterator it = Resources::instance()->find( resourcename_ );
	if( it != Resources::instance()->end() )
	{
		cResource* pResource = it->second;
		if( !target->model() && !target->serial() ) // map tile
		{
			map_st mapTile = Map->seekMap( pos );
			if( pResource->hasMapId( mapTile.id ) )
				pResource->handleConversionTarget( socket, pos, pi, mapTile.id, 0 );
			else
				socket->sysMessage( tr("You cannot use this here!") );

			pc->setSkillDelay( uiCurrentTime + SrvParams->skillDelay() * MY_CLOCKS_PER_SEC );
			return true;
		}
		else if( target->model() )
		{
			if( pResource->hasArtId( target->model() ) )
			{
				pResource->handleConversionTarget( socket, pos, pi, 0, target->model() );
			}
			else
				socket->sysMessage( tr("You cannot use this here!") );

			pc->setSkillDelay( uiCurrentTime + SrvParams->skillDelay() * MY_CLOCKS_PER_SEC );
			return true;
		}
		return false;
	}
	else
	{
		clConsole.send( tr("ERROR: Resource definition %1 not found!").arg( resourcename_ ) );
		return true;
	}
}

static cUObject *productCreator()
{
	return new cResourceItem;
}

void cResourceItem::registerInFactory()
{
	QStringList fields, tables, conditions;
	buildSqlString( fields, tables, conditions ); // Build our SQL string
	QString sqlString = QString( "SELECT uobjectmap.serial,uobjectmap.type,%1 FROM uobjectmap,%2 WHERE uobjectmap.type = 'cResourceItem' AND %3" ).arg( fields.join( "," ) ).arg( tables.join( "," ) ).arg( conditions.join( " AND " ) );
	UObjectFactory::instance()->registerType("cResourceItem", productCreator);
	UObjectFactory::instance()->registerSqlQuery( "cResourceItem", sqlString );
}
