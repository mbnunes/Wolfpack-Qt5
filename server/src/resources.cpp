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
#include "mapstuff.h"
#include "regions.h"
#include "itemsmgr.h"
#include "skills.h"
#include "srvparams.h"

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
	applyDefinition( Tag );
}

cResource::~cResource()
{
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

void cResource::handleTarget( cUOSocket* socket, Coord_cl pos, UINT16 mapid, UINT16 artid )
{
	P_CHAR pc = socket->player();
	if( !socket || !pc )
		return;

	cResourceItem* pResItem = NULL;
	cRegion::RegionIterator4Items ri( pos );
	for (ri.Begin(); !ri.atEnd() && !pResItem; ri++)
	{
		P_ITEM pi = ri.GetData();
		if( pi && pi->pos.x == pos.x && pi->pos.y == pos.y && pi->objectID() == "RESOURCEITEM" )
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
		socket->sysMessage( "You cannot find anything here!" );
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
				pResItem->serial = cItemsManager::getInstance()->getUnusedSerial();
				cItemsManager::getInstance()->registerItem( pResItem );
				pResItem->pos = pos;
				mapRegions->Add( pResItem );
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
	amount = pResItem->morex;
	if( amount == 0 )
	{
		pc->action( charaction_ );
		pc->soundEffect( sound_, true );
		socket->sysMessage( tr( "It seems as if no resources were left!" ) );
		return;
	}
	vein = pResItem->morey;


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
		pc->stm -= RandomNum( staminamin_, staminamax_ );
	else
		pc->stm -= staminamin_;
	if( pc->stm < 0 )
		pc->stm = 0;

	if( !Skills->CheckSkill( pc, skillid_, item.minskill, item.maxskill ) )
	{
		pc->action( charaction_ );
		pc->soundEffect( sound_, true );
		socket->sysMessage( tr("You failed to find anything!") );
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
		socket->sysMessage( tr("You were not able to find anything!") );
		return;
	}
	
	amount -= spawnamount;
	pResItem->morex = amount;
	if( amount == 0 )
	{
		pResItem->decaytime = uiCurrentTime + refreshtime_ * MY_CLOCKS_PER_SEC;
		pResItem->priv |= 0x01; // let the item decay
		pResItem->update();
	}

	if( !item.name.isNull() || !item.name.isEmpty() )
		socket->sysMessage( tr("You have found %1 %2 %3%4 !").arg( spawnamount ).arg( item.name ).arg( name_ ).arg( spawnamount > 1 ? tr("s") : QString("") ) );
	else
		socket->sysMessage( tr("You have found %1 %2%3 !").arg( spawnamount ).arg( name_ ).arg( spawnamount > 1 ? tr("s") : QString("") ) );

	pc->action( charaction_ );
	pc->soundEffect( sound_, true );

	while( spawnamount > 0 )
	{
		UINT16 id = item.ids[ RandomNum( 0, item.ids.size()-1 ) ];
		UINT16 color = item.colors[ RandomNum( 0, item.colors.size()-1 ) ];
		P_ITEM pi = Items->SpawnItem( pc, 1, (char*)QString("%1 %2").arg( item.name ).arg( name_ ).latin1(), true, id, color, true );
/*		if( pi )
		{
			pi->pos = pc->pos;
			mapRegions->Add( pi );
			pi->update();
		}*/
			
		spawnamount -= 1;
	}
}


// class cResourceItem

cResourceItem::cResourceItem( QString resource, UINT32 amount, UINT32 vein )
{
	cItem::Init( false );
	resource_ = resource;
	if( SrvParams->resitemdecaytime() == 0 )
		priv = 0; // nodecay
	else
	{
		priv |= 0x01;
		decaytime = uiCurrentTime + SrvParams->resitemdecaytime() * MY_CLOCKS_PER_SEC;
	}
	morex = amount;
	morey = vein;
	this->setId( 0x1f1e );
	this->amount_ = 1;
	this->setName( tr("resitem: %1").arg(resource) );
	this->setName2(tr("#"));
	this->visible = 2; // gm visible
}

cResourceItem::~cResourceItem()
{
}

void cResourceItem::Serialize( ISerialization &archive )
{
	cSerializable::Serialize( archive );
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
		QDomElement* DefSection = DefManager->getSection( WPDT_RESOURCE, (*it) );
		cResource* pResource = new cResource( *DefSection );
		if( pResource )
		{
			insert( make_pair< QString, cResource* >( (*it), pResource ) );
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
	Coord_cl pos = pc->pos;
	pos.x = target->x();
	pos.y = target->y();
	pos.z = target->z();

	if( pc->skilldelay > uiCurrentTime && !pc->isGM() )
	{
		socket->sysMessage( tr( "You must wait a few moments before using another skill." ) );
		return true;
	}
	
	if( ( pc->pos.distance( pos ) > 4 ) ) //|| !lineOfSight( pChar->pos, pos, DOORS|ROOFING_SLANTED|WALLS_CHIMNEYS ) )
	{
		socket->sysMessage( tr( "You can't reach this" ) );
		return false;
	}
	
	// TODO: Check if there IS what we have targetted at the given position
	// If not we eiter have wrong statics clientside or a 
	// cheater.

	cAllResources::iterator it = cAllResources::getInstance()->find( resourcename_ );
	if( it != cAllResources::getInstance()->end() )
	{
		cResource* pResource = it->second;
		if( !target->model() && !target->serial() ) // map tile
		{
			map_st mapTile = Map->SeekMap( pos );
			if( pResource->hasMapId( mapTile.id ) )
				pResource->handleTarget( socket, pos, mapTile.id, 0 );
			else
				socket->sysMessage( tr("You cannot find anything here!") );

			pc->skilldelay = uiCurrentTime + SrvParams->skillDelay() * MY_CLOCKS_PER_SEC;
			return true;
		}
		else if( target->model() )
		{
			if( pResource->hasArtId( target->model() ) )
			{
				pResource->handleTarget( socket, pos, 0, target->model() );
				if( pResource->deleteSource() && target->serial() )
				{
					P_ITEM pi = FindItemBySerial( target->serial() );
					if( pi )
						Items->DeleItem( pi );
				}
			}
			else
				socket->sysMessage( tr("You cannot find anything here!") );

			pc->skilldelay = uiCurrentTime + SrvParams->skillDelay() * MY_CLOCKS_PER_SEC;
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

// Singleton
cAllResources cAllResources::instance;
