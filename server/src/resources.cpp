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

#undef DBGFILE
#define DBGFILE "resources.cpp"


// class cResource

cResource::cResource( const QDomElement &Tag ) : amountmin_( 0 ), amountmax_( 0 ), skillid_( 0 ), refreshtime_( 0 ), veinchance_( 0 )
{
	section_ = Tag.attribute( "id" );
	applyDefinition( Tag );
	totalquota_ = 0;
	totalveinquota_ = 0;
	QValueVector< resourcespec_st >::iterator it = resourcespecs_.begin();
	while( it != resourcespecs_.end() )
	{
		totalquota_ += (*it).quota;
		totalveinquota_ += (*it).vein_quota;
		++it;
	}
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
					item.name = Value;

				else if( chTagName == "amount" )
				{
					if( childTag.hasAttribute( "min" ) && childTag.hasAttribute( "max" ) )
					{
						item.minamount_per_attempt = hex2dec( childTag.attribute( "min" ) ).toUInt();
						item.maxamount_per_attempt = hex2dec( childTag.attribute( "max" ) ).toUInt();
					}
					else
					{
						item.minamount_per_attempt = Value.toUInt();
						item.maxamount_per_attempt = item.minamount_per_attempt;
					}
				}

				else if( chTagName == "id" )
				{
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
									item.ids.push_back( (*it).toUShort() );
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
							UINT16 minid = ids[0].toUShort();
							UINT16 maxid = ids[1].toUShort();
							while( minid <= maxid )
							{
								item.ids.push_back( minid );
								++minid;
							}
						}
						else
							item.ids.push_back( (*it).toUShort() );
						++it;
					}
				}

				else if( chTagName == "color" )
				{
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
									item.colors.push_back( (*it).toUShort() );
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
							UINT16 mincol = cols[0].toUShort();
							UINT16 maxcol = cols[1].toUShort();
							while( mincol <= maxcol )
							{
								item.colors.push_back( mincol );
								++mincol;
							}
						}
						else
							item.colors.push_back( (*it).toUShort() );
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
						mapids_.push_back( (*it).toUShort() );
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
				UINT16 minid = ids[0].toUShort();
				UINT16 maxid = ids[1].toUShort();
				while( minid <= maxid )
				{
					mapids_.push_back( minid );
					++minid;
				}
			}
			else
				mapids_.push_back( (*it).toUShort() );
			++it;
		}
	}

	// <artid>0xbla</artid> (for using tools on dynamic & static items)
	else if( TagName == "artid" )
	{
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
						artids_.push_back( (*it).toUShort() );
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
				UINT16 minid = ids[0].toUShort();
				UINT16 maxid = ids[1].toUShort();
				while( minid <= maxid )
				{
					artids_.push_back( minid );
					++minid;
				}
			}
			else
				artids_.push_back( (*it).toUShort() );
			++it;
		}
	}

	// <skillid>1</skillid>
	else if( TagName == "skillid" )
		skillid_ = Value.toUShort();

	// <refreshtime>120</refreshtime> (in seconds)
	else if( TagName == "refreshtime" )
		refreshtime_ = Value.toUInt();
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

void cResource::handleTarget( cUOSocket* socket, Coord_cl pos )
{
	P_CHAR pc = socket->player();
	if( !socket || !pc )
		return;

	cResourceItem* pResItem = NULL;
	cRegion::RegionIterator4Items ri( pos );
	for (ri.Begin(); !ri.atEnd() && !pResItem; ri++)
	{
		P_ITEM pi = ri.GetData();
		if( pi && pi->pos.x == pos.x && pi->pos.y == pos.y && pi->objectID() == "RESOURCE" )
		{
			pResItem = dynamic_cast< cResourceItem* >(pi);
			if( pResItem->resource() != section_ )
				pResItem = NULL;
		}
	}

	resourcespec_st item;
	QValueVector< resourcespec_st >::iterator it;
	UINT32 amount = 0;
	UINT32 vein = 0;
	// if there is no resourceitem, create a new one
	if( !pResItem )
	{
		if( RandomNum( 0, 100 ) <= veinchance_ && totalveinquota_ > 0 )
		{
			UINT32 veinquota = RandomNum( 1, totalveinquota_ );
			it = resourcespecs_.begin();
			while( it != resourcespecs_.end() )
			{
				vein++;
				if( veinquota <= (*it).vein_quota )
				{
					item = (*it);
					break;
				}
				++it;
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
				cItemsManager::getInstance()->registerItem( pResItem );
				mapRegions->Add( pResItem );
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
		socket->sysMessage( tr( "There are no resources left!" ) );
		return;
	}
	vein = pResItem->morey;


	if( vein > 0 && vein <= resourcespecs_.size() )
		item = resourcespecs_[ vein-1 ];
	else
	{
		if( totalquota_ > 0 )
		{
			UINT32 quota = RandomNum( 1, totalquota_ );
			it = resourcespecs_.begin();
			while( it != resourcespecs_.end() )
			{
				if( quota <= (*it).quota )
				{
					item = (*it);
					break;
				}
				++it;
			}
		}
		else
		{
			item = resourcespecs_[ RandomNum( 0, resourcespecs_.size()-1 ) ];
		}
	}

	UINT32 spawnamount = 0;
	if( item.maxamount_per_attempt > item.minamount_per_attempt )
		spawnamount = RandomNum( item.minamount_per_attempt, item.maxamount_per_attempt );
	else
		spawnamount = item.minamount_per_attempt;

	if( pc->skill( skillid_ ) >= item.minskill )
		spawnamount = 0;
	else if( pc->skill( skillid_ ) < item.maxskill && item.maxskill > item.minskill )
	{
		spawnamount = (UINT16)( ceil( (double)(pc->skill( skillid_ )-item.minskill) / (double)(item.maxskill - item.minskill) * (double)spawnamount) );
	}

	if( spawnamount > amount )
		spawnamount = amount;
	
	if( spawnamount == 0 )
	{
		socket->sysMessage( tr("You were not able to find anything!") );
		return;
	}
	
	amount -= spawnamount;
	pResItem->morex = amount;
	if( amount == 0 )
	{
		pResItem->decaytime = uiCurrentTime + refreshtime_ * MY_CLOCKS_PER_SEC;
		pResItem->priv &= 0xFE; // let the item decay
		pResItem->free = false;		
	}
	pResItem->update();

	socket->sysMessage( tr("You have found %1 %2%3 !").arg( spawnamount ).arg( item.name ).arg( spawnamount > 1 ? tr("s") : QString("") ) );

	while( spawnamount > 0 )
	{
		UINT16 id = item.ids[ RandomNum( 0, item.ids.size()-1 ) ];
		UINT16 color = item.colors[ RandomNum( 0, item.colors.size()-1 ) ];
		P_ITEM pi = Items->SpawnItem( pc, 1, (char*)item.name.latin1(), true, id, color, false );
		if( pi )
		{
			pi->pos = pc->pos;
			mapRegions->Add( pi );
			pi->update();
		}
			
		spawnamount -= 1;
	}
}


// class cResourceItem

cResourceItem::cResourceItem( QString resource, UINT32 amount, UINT32 vein )
{
	cItem::Init( false );
	resource_ = resource;
	priv |= 0x01; // nodecay
	free = true;
	morex = amount;
	morey = vein;
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

	Coord_cl pos = socket->player()->pos;
	pos.x = target->x();
	pos.y = target->y();
	pos.z = target->z();

	cAllResources::iterator it = cAllResources::getInstance()->find( resourcename_ );
	if( it != cAllResources::getInstance()->end() )
	{
		cResource* pResource = it->second;
		if( !target->model() && !target->serial() ) // map tile
		{
			map_st mapTile = Map->SeekMap( pos );
			if( pResource->hasMapId( mapTile.id ) )
				pResource->handleTarget( socket, pos );
			else
				socket->sysMessage( tr("You cannot find anything here!") );
			return true;
		}
		else if( target->model() )
		{
			if( pResource->hasArtId( target->model() ) )
				pResource->handleTarget( socket, pos );
			else
				socket->sysMessage( tr("You cannot find anything here!") );
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
