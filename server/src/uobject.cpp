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

// Platform Specifics
#include "platform.h"

// Wolfpack includes
#include "coord.h"
#include "uobject.h"
#include "iserialization.h"
#include "globals.h"
#include "network.h"
#include "mapobjects.h"
#include "junk.h"
#include "defines.h"
#include "wpdefaultscript.h"
#include "wpscriptmanager.h"
#include "network/uosocket.h"
#include "wpdefmanager.h"
#include "persistentbroker.h"
#include "dbdriver.h"

// Library Includes

// Debug includes and defines
#undef  DBGFILE
#define DBGFILE "uobject.cpp"

cUObject::cUObject() :
	serial( INVALID_SERIAL ), multis( INVALID_SERIAL ), free( false ), bindmenu_( QString::null )
{
}

cUObject::cUObject( cUObject &src )
{
	// Copy Events
	this->setEvents( src.getEvents() );

	this->serial = src.serial;
	this->multis = src.multis;
	this->name = src.name;
	this->free = src.free;
}

void cUObject::init()
{
}

void cUObject::moveTo( const Coord_cl& newpos )
{
	cMapObjects::getInstance()->remove( this );
	pos = newpos;
	cMapObjects::getInstance()->add( this );
}

/*!
	Performs persistency layer loads.
*/
void cUObject::load( char **result, UINT16 &offset )
{
	name = result[offset++];
	serial = atoi(result[offset++]);
	multis = atoi(result[offset++]);
	pos.x = atoi(result[offset++]);
	pos.y = atoi(result[offset++]);
	pos.z = atoi(result[offset++]);
	pos.map = atoi(result[offset++]);
	eventList_ = QStringList::split( ",", result[offset++] );
	bindmenu_ = result[offset++];

	// Get our events
	recreateEvents();

	tags.load( serial );

	PersistentObject::load( result, offset );
}

/*!
	Performs persistency layer saves.
*/
void cUObject::save()
{
	// uobjectmap fields
	initSave;

	// If the type is changed somewhere in the code
	// That part needs to take care of delete/recreate
	// So we never update the type EVER here..
	if( !isPersistent )
	{
		setTable( "uobjectmap" );
		addField( "serial", serial );
		addStrField( "type", objectID() );
		addCondition( "serial", serial );
		saveFields;
		clearFields;
	}
	
	// uobject fields
	setTable( "uobject" );	
	addStrField( "name", name );
	addField( "serial", serial );
	addField( "multis", multis );
	addField( "pos_x", pos.x );
	addField( "pos_y", pos.y );
	addField( "pos_z", pos.z );
	addField( "pos_map", pos.map );
	addStrField( "events", eventList_.join( "," ) );
	addStrField( "bindmenu", bindmenu_ );
	addCondition( "serial", serial );
	saveFields;

	tags.save( serial );

	PersistentObject::save();

}

/*!
	Performs persistency layer deletion.
*/
bool cUObject::del()
{
	if( !isPersistent )
		return false; // We didn't need to delete the object

	persistentBroker->addToDeleteQueue( "uobject", QString( "serial = '%1'" ).arg( serial ) );
	persistentBroker->addToDeleteQueue( "uobjectmap", QString( "serial = '%1'" ).arg( serial ) );

	tags.del( serial );

	return PersistentObject::del();
}

/*!
	Builds the SQL string needed to retrieve all objects of this type.
*/
void cUObject::buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions )
{
	fields.push_back( "uobject.name,uobject.serial,uobject.multis,uobject.pos_x,uobject.pos_y,uobject.pos_z,uobject.pos_map,uobject.events,uobject.bindmenu" );
	tables.push_back( "uobject" );
	conditions.push_back( "uobjectmap.serial = uobject.serial" );
}


/*!
	Clears the script-chain
*/
void cUObject::clearEvents()
{
	scriptChain.clear();
	eventList_.clear();
}

// Method for setting a list of WPDefaultScripts
void cUObject::setEvents( std::vector< WPDefaultScript* > List )
{
	scriptChain.clear();
	eventList_.clear();

	// "Reset" the events
	if( List.size() == 0 )
		return;

	// Walk the List and add the events
	for( UI08 i = 0; i < List.size(); i++ )
		if( List[ i ] != NULL )
		{
			scriptChain.push_back( List[ i ] );
			eventList_.push_back( List[ i ]->getName() );
		}
}

// Gets a vector of all assigned events
const std::vector< WPDefaultScript* > &cUObject::getEvents( void )
{
	/*std::vector< WPDefaultScript* > List;

	for( UI08 i = 0; i < scriptChain.size(); i++ )
		List.push_back( scriptChain[ i ] );*/

	return scriptChain;
}

// Checks if the object has a specific event
bool cUObject::hasEvent( const QString& Name ) const
{
	QStringList::const_iterator it(eventList_.find( Name ));
	if ( it == eventList_.end() )
		return false;
	return true;
}

void cUObject::addEvent( WPDefaultScript *Event )
{
	if( hasEvent( Event->getName() ) )
		return;

	scriptChain.push_back( Event );
	eventList_.push_back( Event->getName() );
}

void cUObject::removeEvent( const QString& Name )
{
	std::vector< WPDefaultScript* >::iterator myIterator;

	for( myIterator = scriptChain.begin(); myIterator != scriptChain.end(); ++myIterator )
	{
		if( (*myIterator)->getName() == Name )
		{
			scriptChain.erase( myIterator );
			break;
		}
	}
 
	// I hope this works
	eventList_.remove( Name );
}

/****************************
 * 
 * Scripting events
 *
 ****************************/

bool cUObject::onUse( cUObject *Target )
{
	// If we dont have any events assigned just skip processing
	if( scriptChain.empty() )
		return false;

	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
	{
		// If we're the Character pass us as the second param
		// if not as the first
		bool Handeled = false;

		if( !this->isChar() )
			Handeled = scriptChain[ i ]->onUse( (P_CHAR)Target, (P_ITEM)this );
		else
			Handeled = scriptChain[ i ]->onUse( (P_CHAR)this, (P_ITEM)Target );

		if( Handeled )
			return true;
	}

	return false;
}

bool cUObject::onCreate( const QString &definition )
{
	// If we dont have any events assigned just skip processing
	if( scriptChain.empty() )
		return false;

	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
	{
		// If we're the Character pass us as the second param
		// if not as the first
		bool Handeled = false;

		Handeled = scriptChain[ i ]->onCreate( this, definition );

		if( Handeled )
			return true;
	}

	return false;
}

bool cUObject::onCollide( cUObject* Obstacle )
{
	// If we dont have any events assigned just skip processing
	if( scriptChain.empty() )
		return false;

	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
	{
		// Items cannot collide with items
		if( !isChar() ) // Item, so obstacle has to be character
			scriptChain[ i ]->onCollideItem( (P_CHAR)Obstacle, (P_ITEM)this );
		else
			if( Obstacle->isItem() )
				if( scriptChain[ i ]->onCollideItem( (P_CHAR)this, (P_ITEM)Obstacle ) )
					return true;

			else // Character, Character
				if( scriptChain[ i ]->onCollideChar( (P_CHAR)this, (P_CHAR)Obstacle ) )
					return true;
	}

	return false;
}

// Returns the list of events
QString cUObject::eventList( void ) const
{
	return eventList_.join( "," );
}

// If the scripts are reloaded call that for each and every existing object
void cUObject::recreateEvents( void )
{
	// Walk the eventList and recreate 
	QStringList::const_iterator myIter;

	scriptChain.clear();

	for( myIter = eventList_.begin(); myIter != eventList_.end(); ++myIter )
	{
		WPDefaultScript *myScript = ScriptManager->find( *myIter );

		// Script not found
		if( myScript == NULL )
			continue;

		scriptChain.push_back( myScript );
	}
}

void cUObject::processNode( const QDomElement &Tag )
{
	QString TagName = Tag.nodeName();
	QString Value = this->getNodeValue( Tag );

	// <tag type="string"> also type="value"
	//	    <key>multisection</key>
	//		<value>smallboat</value>
	// </tag>
	if( TagName == "tag" )
	{
		QString tkey = (char*)0;
		QString tvalue = (char*)0;
		QDomNode childNode = Tag.firstChild();
		while( !childNode.isNull() )
		{
			if( childNode.isElement() )
			{
				QDomElement childTag = childNode.toElement();
				QString childValue = this->getNodeValue( childTag );
				QString childName = childNode.nodeName();
				
				if( childName == "key" )
					tkey = childValue;	

				else if( childName == "value" )
					tvalue = childValue;
			}
			childNode = childNode.nextSibling();
		}

		if( !tkey.isNull() && !tvalue.isNull() )
		{
			if( Tag.attribute( "type" ) == "value" )
				this->tags.set( tkey, cVariant( tvalue.toInt() ) );
			else
				this->tags.set( tkey, cVariant( tvalue ) );
		}
	}
	// <events>a,b,c</events>
	else if( TagName == "events" )
	{
		eventList_ = QStringList::split( ",", Value );
		recreateEvents();
	}
}

// Remove it from all in-range sockets
void cUObject::removeFromView( bool clean )
{
	// Get Real pos
	Coord_cl mPos = pos;

	if( isItemSerial( serial ) )
	{
		P_ITEM pItem = dynamic_cast<P_ITEM>(this);
		P_ITEM pCont = pItem->getOutmostItem();
		if( pCont )
		{
			mPos = pCont->pos;
			P_CHAR pOwner = dynamic_cast<P_CHAR>( pCont->container() );
			if( pOwner )
				mPos = pOwner->pos;
		}
	}

	for( cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next() )
		if( clean || ( socket->player() && ( socket->player()->pos.distance( mPos ) <= socket->player()->VisRange() ) ) )
			socket->removeObject( this );
}

// Checks if the specified object is in range
bool cUObject::inRange( const cUObject *object, UINT32 range ) const
{
	if( !object ) 
		return false;

	return ( pos.distance( object->pos ) <= range );
}

/*!
	Displays an effect emitting from this object toward another item or character
*/
void cUObject::effect( UINT16 id, cUObject *target, bool fixedDirection, bool explodes, UINT8 speed, UINT16 hue, UINT16 renderMode )
{
	if( !target )
		return;

	cUOTxEffect effect;
	effect.setType( ET_MOVING );
	effect.setSource( serial );
	effect.setTarget( target->serial );
	effect.setSourcePos( pos );
	effect.setTargetPos( target->pos );
	effect.setId( id );
    effect.setSpeed( speed );
	effect.setExplodes( explodes );
	effect.setFixedDirection( fixedDirection );
	effect.setHue( hue );
	effect.setRenderMode( renderMode );

	cUOSocket *mSock = 0;
	for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		if( !mSock->player() )
			continue;

		// The Socket has to be either in range of Source or Target
		if( mSock->player()->inRange( this, mSock->player()->VisRange() ) || mSock->player()->inRange( target, mSock->player()->VisRange() ) )
			mSock->send( &effect );
	}
}

/*!
	Displays an effect emitting from this object and moving towards a specific location.
*/
void cUObject::effect( UINT16 id, const Coord_cl &target, bool fixedDirection, bool explodes, UINT8 speed, UINT16 hue, UINT16 renderMode )
{
	cUOTxEffect effect;
	effect.setType( ET_MOVING );
	effect.setSource( serial );
	effect.setSourcePos( pos );
	effect.setTargetPos( target );
	effect.setId( id );
    effect.setSpeed( speed );
	effect.setExplodes( explodes );
	effect.setFixedDirection( fixedDirection );
	effect.setHue( hue );
	effect.setRenderMode( renderMode );

	cUOSocket *mSock = 0;
	for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		if( !mSock->player() )
			continue;

		// The Socket has to be either in range of Source or Target
		if( mSock->player()->inRange( this, mSock->player()->VisRange() ) || ( mSock->player()->pos.distance( target ) <= mSock->player()->VisRange() ) )
			mSock->send( &effect );
	}
}

/*!
	Displays an effect moving with this object.
*/
void cUObject::effect( UINT16 id, UINT8 speed, UINT8 duration, UINT16 hue, UINT16 renderMode )
{
	cUOTxEffect effect;
	effect.setType( ET_STAYSOURCESER );
	effect.setSource( serial );
	effect.setSourcePos( pos );
	effect.setTargetPos( pos );
	effect.setId( id );
    effect.setSpeed( speed );
	effect.setDuration( duration );
	effect.setFixedDirection( true );
	effect.setHue( hue );
	effect.setRenderMode( renderMode );

	cUOSocket *mSock = 0;
	for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		if( mSock->player() && mSock->player()->inRange( this, mSock->player()->VisRange() ) )
			mSock->send( &effect );
	}
}

// Simple setting and getting of properties for scripts and the set command.
stError *cUObject::setProperty( const QString &name, const cVariant &value )
{

	SET_STR_PROPERTY( "bindmenu", bindmenu_ )
	else SET_INT_PROPERTY( "serial", serial )
	else SET_INT_PROPERTY( "multi", multis )
	else SET_BOOL_PROPERTY( "free", free )
	else SET_STR_PROPERTY( "name", this->name )

	else if( name == "pos" )
	{
		Coord_cl pos;
		if( !parseCoordinates( value.toString(), pos ) )
			PROPERTY_ERROR( -3, QString( "Invalid coordinate value: '%1'" ).arg( value.toString() ) )
		moveTo( pos );
		return 0;
	}

	// Trying to set new Eventlist
	else if( name == "eventlist" )
	{
		clearEvents();
		QStringList list = QStringList::split( ",", value.toString() );
		for( QStringList::const_iterator it = list.begin(); it != list.end(); ++it )
		{
			WPDefaultScript *script = ScriptManager->find( *it );
			if( script )
				addEvent( script );
			else
				PROPERTY_ERROR( -3, QString( "Script not found: '%1'" ).arg( *it ) )
		}
		return 0;
	}

	PROPERTY_ERROR( -1, QString( "Property not found: '%1'" ).arg( name ) )
}

stError *cUObject::getProperty( const QString &name, cVariant &value ) const
{
	GET_PROPERTY( "bindmenu", bindmenu_ )
	else GET_PROPERTY( "serial", serial )
	else GET_PROPERTY( "multi", FindItemBySerial( multis ) )
	else GET_PROPERTY( "free", free ? 1 : 0 )
	else GET_PROPERTY( "name", this->name )
	else GET_PROPERTY( "pos", pos )
	else GET_PROPERTY( "eventlist", eventList_.join(",") );

	PROPERTY_ERROR( -1, QString( "Property not found: '%1'" ).arg( name ) )
}
