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

// Platform Specifics
#include "platform.h"

// Wolfpack includes
#include "coord.h"
#include "uobject.h"
#include "iserialization.h"
#include "globals.h"
#include "network.h"
#include "wolfpack.h"
#include "defines.h"
#include "pythonscript.h"
#include "scriptmanager.h"
#include "network/uosocket.h"
#include "network/uotxpackets.h"
#include "wpdefmanager.h"
#include "persistentbroker.h"
#include "dbdriver.h"
#include "basechar.h"
#include "sectors.h"
#include "player.h"

// Library Includes

// Debug includes and defines
#undef  DBGFILE
#define DBGFILE "uobject.cpp"

cUObject::cUObject() :
	serial_( INVALID_SERIAL ), 
	multis_( INVALID_SERIAL ), 
	free( false ), 
	bindmenu_( QString::null ), 
	changed_(true), 
	tooltip_( 0xFFFFFFFF ),
	eventList_( QString::null )
{
}

cUObject::cUObject( cUObject &src )
{
	// Copy Events
	eventList_ = src.eventList_;
	recreateEvents();

	this->serial_ = src.serial_;
	this->multis_ = src.multis_;
	this->name_ = src.name_;
	this->pos_  = src.pos_;
	this->free = src.free;
	this->changed( TOOLTIP );
	tooltip_ = src.tooltip_;
	changed_ = true;
}

void cUObject::init()
{
}

void cUObject::moveTo( const Coord_cl& newpos, bool noRemove )
{
	if( !noRemove )
		MapObjects::instance()->remove( this );

	pos_ = newpos;

	MapObjects::instance()->add( this );

	changed_ = true;
}

unsigned int cUObject::dist(cUObject* d) const
{	
	if ( !d )
		return ~0;
	return pos_.distance(d->pos_);		
}


/*!
	Performs persistency layer loads.
*/
void cUObject::load( char **result, UINT16 &offset )
{
	name_ = result[offset++];
	serial_ = atoi(result[offset++]);
	multis_ = atoi(result[offset++]);
	dir_ = atoi( result[offset++] );
	pos_.x = atoi(result[offset++]);
	pos_.y = atoi(result[offset++]);
	pos_.z = atoi(result[offset++]);
	pos_.map = atoi(result[offset++]);
	eventList_ = result[offset] == 0 ? QString::null : result[offset];
	offset++;
	bindmenu_ = result[offset++];
	bool havetags_ = atoi( result[offset++] );

	// Get our events
	recreateEvents();
	if( havetags_ )
		tags_.load( serial_ );
	
	PersistentObject::load( result, offset );
}

/*!
	Performs persistency layer saves.
*/
void cUObject::save()
{
	bool havetags_ = ( tags_.size() > 0 );
	// uobjectmap fields

	// If the type is changed somewhere in the code
	// That part needs to take care of delete/recreate
	// So we never update the type EVER here..
	if( !isPersistent )
	{
		initSave;
		setTable( "uobjectmap" );
		addField( "serial", serial_ );
		addStrField( "type", objectID() );
		addCondition( "serial", serial_ );
		saveFields;
		clearFields;
	}
	
	// uobject fields
	if ( changed_ )
	{
		initSave;
		setTable( "uobject" );	
		addStrField( "name", name_ );
		addField( "serial", serial_ );
		addField( "multis", multis_ );
		addField( "direction", dir_);
		addField( "pos_x", pos_.x );
		addField( "pos_y", pos_.y );
		addField( "pos_z", pos_.z );
		addField( "pos_map", pos_.map );
		addStrField( "events", eventList_ == QString::null ? QString( "" ) : eventList_ );
		addStrField( "bindmenu", bindmenu_ );
		addCondition( "serial", serial_ );
		addField( "havetags", havetags_ );
		saveFields;
	}
	if( havetags_ )
	{
		tags_.save( serial_ );
	}

	PersistentObject::save();
	flagUnchanged(); // This is the botton of the chain, now go up and flag everyone unchanged.
}

/*!
	Performs persistency layer deletion.
*/
bool cUObject::del()
{
	if( !isPersistent )
		return false; // We didn't need to delete the object

	persistentBroker->addToDeleteQueue( "uobject", QString( "serial = '%1'" ).arg( serial_ ) );
	persistentBroker->addToDeleteQueue( "uobjectmap", QString( "serial = '%1'" ).arg( serial_ ) );

	if( tags_.size() > 0 )
		tags_.del( serial_ );

	changed_ = true;

	return PersistentObject::del();
}

/*!
	Builds the SQL string needed to retrieve all objects of this type.
*/
void cUObject::buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions )
{
	fields.push_back( "uobject.name,uobject.serial,uobject.multis,uobject.direction,uobject.pos_x,uobject.pos_y,uobject.pos_z,uobject.pos_map,uobject.events,uobject.bindmenu,uobject.havetags" );
	tables.push_back( "uobject" );
	conditions.push_back( "uobjectmap.serial = uobject.serial" );
}


/*!
	Clears the script-chain
*/
void cUObject::clearEvents()
{
	scriptChain.clear();
	eventList_ = QString::null;
	changed_ = true;
}

// Method for setting a list of cPythonScripts
void cUObject::setEvents( std::vector< cPythonScript* > List )
{
	scriptChain.clear();
	eventList_ = QString::null;

	// "Reset" the events
	if( List.size() == 0 )
		return;

	// Walk the List and add the events
	for( UI08 i = 0; i < List.size(); i++ )
		if( List[ i ] != NULL )
		{
			scriptChain.push_back( List[ i ] );

			if( eventList_ == QString::null )
				eventList_ = List[ i ]->name();
			else 
				eventList_.append( "," + List[ i ]->name() );
		}
	changed_ = true;
}

// Gets a vector of all assigned events
const std::vector< cPythonScript* > &cUObject::getEvents( void )
{
	return scriptChain;
}

// Checks if the object has a specific event
bool cUObject::hasEvent( const QString& Name ) const
{
	QStringList eventList = QStringList::split( ",", eventList_ );

	return eventList.find( Name ) != eventList.end();
}

void cUObject::addEvent( cPythonScript *Event )
{
	if( hasEvent( Event->name() ) )
		return;

	scriptChain.push_back( Event );

	if( eventList_ == QString::null )
		eventList_ = Event->name();
	else
		eventList_.append( "," + Event->name() );

	changed_ = true;
}

void cUObject::removeEvent( const QString& Name )
{
	std::vector< cPythonScript* >::iterator myIterator;

	for( myIterator = scriptChain.begin(); myIterator != scriptChain.end(); ++myIterator )
	{
		if( (*myIterator)->name() == Name )
		{
			scriptChain.erase( myIterator );
			break;
		}
	}
 
	// I hope this works
	QStringList eventList = QStringList::split( ",", eventList_ );
	eventList.remove( Name );
	eventList_ = eventList.join( "," );

	if( eventList_.isEmpty() )
		eventList_ = QString::null;

	changed_ = true;
}

/****************************
 * 
 * Scripting events
 *
 ****************************/

bool cUObject::onUse( cUObject *Target )
{
	// If we got ANY events process them in order
	for( UI08 i = 0; i < scriptChain.size(); i++ )
	{
		// If we're the Character pass us as the second param
		// if not as the first
		bool Handeled = false;

		if( !this->isChar() )
			Handeled = scriptChain[ i ]->onUse( (P_PLAYER)Target, (P_ITEM)this );
		else
			Handeled = scriptChain[ i ]->onUse( (P_PLAYER)this, (P_ITEM)Target );

		if( Handeled )
			return true;
	}

	// Try to process the hooks then
	QValueVector< cPythonScript* > hooks;
	QValueVector< cPythonScript* >::const_iterator it;

	hooks = ScriptManager->getGlobalHooks( OBJECT_OBJECT, EVENT_USE );
	for( it = hooks.begin(); it != hooks.end(); ++it )
	{
		if( !this->isChar() )
			(*it)->onUse( (P_PLAYER)Target, (P_ITEM)this );
		else
			(*it)->onUse( (P_PLAYER)this, (P_ITEM)Target );
	}

	if( isChar() )
	{
		hooks = ScriptManager->getGlobalHooks( OBJECT_CHAR, EVENT_USE );
		for( it = hooks.begin(); it != hooks.end(); ++it )
				(*it)->onUse( (P_PLAYER)this, (P_ITEM)Target );
	}
	
	if( isItem() )
	{
		hooks = ScriptManager->getGlobalHooks( OBJECT_ITEM, EVENT_USE );
		for( it = hooks.begin(); it != hooks.end(); ++it )
			(*it)->onUse( (P_PLAYER)Target, (P_ITEM)this );
	}

	return false;
}

bool cUObject::onCreate( const QString &definition )
{
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

	// Try to process the hooks then
	QValueVector< cPythonScript* > hooks;
	QValueVector< cPythonScript* >::const_iterator it;

	hooks = ScriptManager->getGlobalHooks( OBJECT_OBJECT, EVENT_CREATE );
	for( it = hooks.begin(); it != hooks.end(); ++it )
		(*it)->onCreate( this, definition );

	if( isChar() )
	{
		hooks = ScriptManager->getGlobalHooks( OBJECT_CHAR, EVENT_CREATE );
		for( it = hooks.begin(); it != hooks.end(); ++it )
			(*it)->onCreate( this, definition );
	}
	
	if( isItem() )
	{
		hooks = ScriptManager->getGlobalHooks( OBJECT_ITEM, EVENT_CREATE );
		for( it = hooks.begin(); it != hooks.end(); ++it )
			(*it)->onCreate( this, definition );
	}

	return false;
}

bool cUObject::onCollide( cUObject* Obstacle )
{
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

	// Try to process the hooks then
	QValueVector< cPythonScript* > hooks;
	QValueVector< cPythonScript* >::const_iterator it;

	hooks = ScriptManager->getGlobalHooks( OBJECT_OBJECT, EVENT_COLLIDE );
	for( it = hooks.begin(); it != hooks.end(); ++it )
	{
		// Items cannot collide with items
		if( !isChar() ) // Item, so obstacle has to be character
			(*it)->onCollideItem( (P_CHAR)Obstacle, (P_ITEM)this );
		else
		{
			if( Obstacle->isItem() )
				(*it)->onCollideItem( (P_CHAR)this, (P_ITEM)Obstacle );
			else
				(*it)->onCollideChar( (P_CHAR)this, (P_CHAR)Obstacle );
		}
	}

	if( isChar() )
	{
		hooks = ScriptManager->getGlobalHooks( OBJECT_CHAR, EVENT_COLLIDE );
		for( it = hooks.begin(); it != hooks.end(); ++it )
		{
			if( Obstacle->isItem() )
				(*it)->onCollideItem( (P_CHAR)this, (P_ITEM)Obstacle );
			else
				(*it)->onCollideChar( (P_CHAR)this, (P_CHAR)Obstacle );
		}
	}
	
	if( isItem() )
	{
		hooks = ScriptManager->getGlobalHooks( OBJECT_ITEM, EVENT_COLLIDE );
		for( it = hooks.begin(); it != hooks.end(); ++it )
			(*it)->onCollideItem( (P_CHAR)Obstacle, (P_ITEM)this );
	}

	return false;
}


// Returns the list of events
QString cUObject::eventList( void ) const
{
	return eventList_;
}

// If the scripts are reloaded call that for each and every existing object
void cUObject::recreateEvents( void )
{
	scriptChain.clear();

	if( eventList_ == QString::null )
		return;
	
	// Walk the eventList and recreate 
	QStringList::const_iterator myIter;
	QStringList eventList = QStringList::split( ",", eventList_ );

	for( myIter = eventList.begin(); myIter != eventList.end(); ++myIter )
	{
		cPythonScript *myScript = ScriptManager->find( *myIter );

		// Script not found
		if( myScript == NULL )
			continue;

		scriptChain.push_back( myScript );
	}
}

void cUObject::processNode( const cElement *Tag )
{
	QString TagName = Tag->name();
	QString Value = Tag->getValue();

	//<direction>SE</direction>
	if( TagName == "direction" )
	{
		if( Value == "NE" )
			this->dir_ = 1;
		else if( Value == "E" )
			this->dir_ = 2;
		else if( Value == "SE" )
			this->dir_ = 3;
		else if( Value == "S" )
			this->dir_ = 4;
		else if( Value == "SW" )
			this->dir_ = 5;
		else if( Value == "W" )
			this->dir_ = 6;
		else if( Value == "NW" )
			this->dir_ = 7;
		else if( Value == "N" )
			this->dir_ = 0;
		else
			this->dir_ = Value.toUShort();
	}

	// <tag type="string"> also type="value"
	//	    <key>multisection</key>
	//		<value>smallboat</value>
	// </tag>
	else if( TagName == "tag" )
	{
		QString tkey, tvalue;

		for( unsigned int i = 0; i < Tag->childCount(); ++i )
		{
			const cElement *childTag = Tag->getChild( i );

			QString childValue = childTag->getValue();
			QString childName = childTag->name();
				
			if( childName == "key" )
				tkey = childValue;	

			else if( childName == "value" )
				tvalue = childValue;
		}

		if( !tkey.isNull() && !tvalue.isNull() )
		{
			if( Tag->getAttribute( "type" ) == "value" )
				this->tags_.set( tkey, cVariant( tvalue.toInt() ) );
			else
				this->tags_.set( tkey, cVariant( tvalue ) );
		}
	}
	// <events>a,b,c</events>
	else if( TagName == "events" )
	{
		if( Value.isEmpty() )
			eventList_ = QString::null;
		else
			eventList_ = Value;

		recreateEvents();
	}
	else 
		setProperty( TagName, Value );
//		qWarning( tr("Unknown tag %1").arg(TagName) );
}

// Remove it from all in-range sockets
void cUObject::removeFromView( bool clean )
{
	// Get Real pos
	Coord_cl mPos = pos_;

	if( isItemSerial( serial_ ) )
	{
		P_ITEM pItem = dynamic_cast<P_ITEM>(this);
		P_ITEM pCont = pItem->getOutmostItem();
		if( pCont )
		{
			mPos = pCont->pos();
			P_CHAR pOwner = dynamic_cast<P_CHAR>( pCont->container() );
			if( pOwner )
				mPos = pOwner->pos();
		}
	}

	for( cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next() )
		if( clean || ( socket->player() && ( socket->player()->pos().distance( mPos ) <= socket->player()->visualRange() ) ) )
			socket->removeObject( this );
}

// Checks if the specified object is in range
bool cUObject::inRange( const cUObject *object, UINT32 range ) const
{
	if( !object ) 
		return false;

	return ( pos_.distance( object->pos_ ) <= range );
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
	effect.setSource( serial_ );
	effect.setTarget( target->serial_ );
	effect.setSourcePos( pos_ );
	effect.setTargetPos( target->pos_ );
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
		if( mSock->player()->inRange( this, mSock->player()->visualRange() ) || mSock->player()->inRange( target, mSock->player()->visualRange() ) )
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
	effect.setSource( serial_ );
	effect.setSourcePos( pos_ );
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
		if( mSock->player()->inRange( this, mSock->player()->visualRange() ) || ( mSock->player()->pos().distance( target ) <= mSock->player()->visualRange() ) )
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
	effect.setSource( serial_ );
	effect.setSourcePos( pos_ );
	effect.setTargetPos( pos_ );
	effect.setId( id );
    effect.setSpeed( speed );
	effect.setDuration( duration );
	effect.setFixedDirection( true );
	effect.setHue( hue );
	effect.setRenderMode( renderMode );

	cUOSocket *mSock = 0;
	for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		if( mSock->player() && mSock->player()->inRange( this, mSock->player()->visualRange() ) )
			mSock->send( &effect );
	}
}

// Simple setting and getting of properties for scripts and the set command.
stError *cUObject::setProperty( const QString &name, const cVariant &value )
{
	changed( TOOLTIP );
	changed_ = true;
	SET_STR_PROPERTY( "bindmenu", bindmenu_ )
	else SET_INT_PROPERTY( "serial", serial_ )
	else SET_INT_PROPERTY( "multi", multis_ )
	else SET_INT_PROPERTY( "direction", dir_ )
	else SET_BOOL_PROPERTY( "free", free )
	else SET_STR_PROPERTY( "name", this->name_ )

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
			cPythonScript *script = ScriptManager->find( *it );
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
	else GET_PROPERTY( "serial", serial_ )
	else GET_PROPERTY( "multi", FindItemBySerial( multis_ ) )
	else GET_PROPERTY( "free", free ? 1 : 0 )
	else GET_PROPERTY( "name", this->name() )
	else GET_PROPERTY( "pos", pos() )
	else GET_PROPERTY( "eventlist", eventList_ == QString::null ? QString( "" ) : eventList_ )
	else GET_PROPERTY( "direction", dir_ )

	PROPERTY_ERROR( -1, QString( "Property not found: '%1'" ).arg( name ) )
}

void cUObject::sendTooltip( cUOSocket* mSock )
{
	if( tooltip_ == 0xFFFFFFFF )
	{
		tooltip_ = World::instance()->getUnusedTooltip(); 
		setTooltip( tooltip_ );
	}

	cUOTxAttachTooltip tooltip;

	tooltip.setId( tooltip_ );
	tooltip.setSerial( serial() );

	if( tooltip_ >= mSock->toolTips()->size() || !mSock->haveTooltip( tooltip_ ) )
	{
		mSock->addTooltip( tooltip_ );
		mSock->send( &tooltip );
	}
}

void cUObject::changed( uint state )
{
//	if( state & SAVE ) changed_ = true;
	if( state & TOOLTIP ) tooltip_ = 0xFFFFFFFF;
}

/*!
	Returns the direction from this object to another \s d object
*/
char cUObject::direction( cUObject* d ) const
{
	int dir = -1;
	int xdif = d->pos().x - this->pos().x;
	int ydif = d->pos().y - this->pos().y;

	if ((xdif==0)&&(ydif<0)) dir=0;
	else if ((xdif>0)&&(ydif<0)) dir=1;
	else if ((xdif>0)&&(ydif==0)) dir=2;
	else if ((xdif>0)&&(ydif>0)) dir=3;
	else if ((xdif==0)&&(ydif>0)) dir=4;
	else if ((xdif<0)&&(ydif>0)) dir=5;
	else if ((xdif<0)&&(ydif==0)) dir=6;
	else if ((xdif<0)&&(ydif<0)) dir=7;
	else dir=-1;

	return dir;
}
