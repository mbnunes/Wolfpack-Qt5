//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2004 by holders identified in authors.txt
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
#include "globals.h"
#include "network.h"
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
#include "basics.h"
#include "items.h"
#include "basics.h"
#include "world.h"

// Library Includes

cUObject::cUObject() :
	serial_( INVALID_SERIAL ),
	multis_( INVALID_SERIAL ),
	free( false ),
	bindmenu_( QString::null ),
	changed_(true),
	tooltip_( 0xFFFFFFFF ),
	eventList_( QString::null ),
	name_( QString::null ),
	scriptChain( 0 )
{
}

cUObject::cUObject( const cUObject &src ) {
	// Copy Events
	scriptChain = 0;
	eventList_ = src.eventList_;
	recreateEvents();
	this->multis_ = src.multis_;
	this->name_ = src.name_;
	this->pos_  = src.pos_;
	this->tags_ = src.tags_;
	changed_ = true;
}

void cUObject::init()
{
}

void cUObject::moveTo( const Coord_cl& newpos, bool noRemove )
{
	if( !noRemove ) {
		MapObjects::instance()->remove( this );
	}

	pos_ = newpos;

	MapObjects::instance()->add( this );

	changed_ = true;
}

/*!
	Returns the distance between this object and \a d
*/
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
	name_ = ( result[offset] == 0 ) ? QString::null : QString::fromUtf8( result[offset] );
	offset++;
	serial_ = atoi(result[offset++]);
	multis_ = atoi(result[offset++]);
	dir_ = atoi( result[offset++] );
	pos_.x = atoi(result[offset++]);
	pos_.y = atoi(result[offset++]);
	pos_.z = atoi(result[offset++]);
	pos_.map = atoi(result[offset++]);
	eventList_ = ( result[offset] == 0 ) ? QString::null : QString( result[offset] );
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
		addStrField( "type", QString( objectID() ) );
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
	// We are requiring fixed order by now, so this *is* possible
	fields.push_back( "uobject.name,uobject.serial,uobject.multis,uobject.direction,uobject.pos_x,uobject.pos_y,uobject.pos_z,uobject.pos_map,uobject.events,uobject.bindmenu,uobject.havetags" );
	tables.push_back( "uobject" );
	conditions.push_back( "uobjectmap.serial = uobject.serial" );
}


/*!
	Clears the script-chain
*/
void cUObject::clearEvents()
{
	if( scriptChain )
	{
		delete [] scriptChain;
		scriptChain = 0;

		eventList_ = QString::null;
		changed_ = true;
	}
}

/*!
	Checks if the object has a specific event \a name
	\sa addEvent
*/
bool cUObject::hasEvent( const QString& name ) const
{
	if( scriptChain )
	{
		unsigned int count = reinterpret_cast< unsigned int >( scriptChain[0] );

		for( unsigned int i = 1; i <= count; ++i )
		{
			if( scriptChain[i]->name() == name )
				return true;
		}
	}

	return false;
}

/*!
	Adds an event handler to this object
*/
void cUObject::addEvent( cPythonScript *Event )
{
	if( hasEvent( Event->name() ) )
		return;

	// Reallocate the ScriptChain
	if( scriptChain )
	{
		unsigned int count = reinterpret_cast< unsigned int >( scriptChain[0] );

		// i is the count of real elements in the old array
		cPythonScript **newScriptChain = new cPythonScript* [ count + 1 ];
		memcpy( newScriptChain, scriptChain, (count+1) * sizeof( cPythonScript* ) );
		newScriptChain[ count+1 ] = Event;
		newScriptChain[ 0 ] = reinterpret_cast< cPythonScript* >( count + 1 );

		delete [] scriptChain;
		scriptChain = newScriptChain;
	}
	else
	{
		scriptChain = new cPythonScript*[2];
		scriptChain[0] = reinterpret_cast< cPythonScript* >( 1 );
		scriptChain[1] = Event;
	}

	if( eventList_ == QString::null )
		eventList_ = Event->name();
	else
		eventList_.append( "," + Event->name() );

	changed_ = true;
}

/*!
	Removes an event handler from the object
*/
void cUObject::removeEvent( const QString& name )
{
	if( scriptChain && hasEvent( name ) )
	{
		unsigned int count = reinterpret_cast< unsigned int >( scriptChain[0] );

		if( count == 1 )
		{
			clearEvents();
		}
		else
		{
			unsigned int pos = 1;

			cPythonScript **newScriptChain = new cPythonScript*[ count ];
			newScriptChain[0] = reinterpret_cast< cPythonScript* >( count - 1 );

			for( unsigned int i = 1; i < count; ++i )
			{
				if( scriptChain[i]->name() != name )
					newScriptChain[pos++] = scriptChain[i];
			}

			delete [] scriptChain;
			scriptChain = newScriptChain;
		}
	}

	if( eventList_ != QString::null )
	{
		QStringList eventList = QStringList::split( ",", eventList_ );
		eventList.remove( name );
		eventList_ = eventList.join( "," );

		if( eventList_.isEmpty() )
			eventList_ = QString::null;
	}

	changed_ = true;
}

// If the scripts are reloaded call that for each and every existing object
void cUObject::recreateEvents()
{
	// clearEvents() would flag us as changed, but we didn't
	if( eventList_ == QString::null )
		return;

	delete [] scriptChain;
	scriptChain = 0;

	// Walk the eventList and recreate
	QStringList::const_iterator myIter;
	QStringList eventList = QStringList::split( ",", eventList_ );

	for( myIter = eventList.begin(); myIter != eventList.end(); ++myIter )
	{
		cPythonScript *Event = ScriptManager::instance()->find( (*myIter).latin1() );

		if( Event )
		{
			if( !hasEvent( Event->name() ) )
			{
				// Reallocate the ScriptChain
				if( scriptChain )
				{
					unsigned int count = reinterpret_cast< unsigned int >( scriptChain[0] );

					// i is the count of real elements in the old array
					cPythonScript **newScriptChain = new cPythonScript* [ count + 2 ];
					memcpy( newScriptChain, scriptChain, (count+1) * sizeof( cPythonScript* ) );
					newScriptChain[ count+1 ] = Event;
					newScriptChain[ 0 ] = reinterpret_cast< cPythonScript* >( count + 1 );

					delete [] scriptChain;
					scriptChain = newScriptChain;
				}
				else
				{
					scriptChain = new cPythonScript*[2];
					scriptChain[0] = reinterpret_cast< cPythonScript* >( 1 );
					scriptChain[1] = Event;
				}
			}
		}
	}
}

void cUObject::processNode( const cElement *Tag )
{
	QString TagName = Tag->name();
	QString Value = Tag->value();

	if( TagName == "name" )
	{
		name_ = Value;
	}
	//<direction>SE</direction>
	else if( TagName == "direction" )
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
		QString name = Tag->getAttribute("name");
		QString value = Tag->getAttribute("value");

		if (!name.isNull()) {
			// If there is no value attribute, use the
			// tag content instead.
			if (value.isNull()) {
				value = Tag->text();

				if (value.isNull()) {
					value = "";
				}
			}

            QString type = Tag->getAttribute("type", "string");

			if (type == "int") {
				// If the value is separated by a ,
				// we assume it's a random gradient.
				// If it's separated by ; we assume it's a list of values
				// we should choose from randomly.
				int sep = value.find(',');

				if (sep != -1) {
					int min = hex2dec(value.left(sep)).toInt();
					int max = hex2dec(value.mid(sep + 1)).toInt();
					
					int value = RandomNum(min, max);
					tags_.set(name, cVariant((int)value));
				} else {
					// Choose a random value from the list.
					if (value.contains(';')) {
						QStringList values = QStringList::split(';', value);
						if (values.size() > 0) {
							value = values[RandomNum(0, values.size() - 1)];
						}
					}

					tags_.set( name, cVariant(hex2dec(value).toInt()));
				}				
			} else if (type == "float") {
				tags_.set(name, cVariant(value.toFloat()));
			} else {
				tags_.set(name, cVariant(value));
			}
		}
	}
	// <events>a,b,c</events>
	else if( TagName == "events" )
	{
		if( Value.isEmpty() && eventList_.isEmpty() )
			eventList_ = QString::null;
		else
			eventList_ = eventList_.isEmpty() ? Value : eventList_ + "," + Value;

		recreateEvents();
	}
	else
	{
		if (Value.isEmpty()) {
			Value = "1";
		}

		cVariant variant( Value );
		setProperty( TagName, variant );
	}
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
bool cUObject::inRange( cUObject *object, UINT32 range ) const
{
	if( !object )
		return false;

	Coord_cl pos = object->pos_;

	if (object->isItem()) {
		P_ITEM pItem = dynamic_cast<P_ITEM>(object);

		if (pItem) {
			P_ITEM pCont = pItem->getOutmostItem();
			P_CHAR pEquipped = pItem->getOutmostChar();

			if (pEquipped) {
				pos = pEquipped->pos();
			} else if (pCont) {
				pos = pCont->pos();
			}
		}
	}

	return pos_.distance(pos) <= range;
}

void cUObject::lightning( unsigned short hue )
{
	cUOTxEffect effect;
	effect.setType( ET_LIGHTNING );
	effect.setSource( serial_ );
	effect.setSourcePos( pos_ );
	effect.setTargetPos( pos_ );
	effect.setHue( hue );

	cUOSocket *mSock = 0;
	for( mSock = cNetwork::instance()->first(); mSock; mSock = cNetwork::instance()->next() )
	{
		if( mSock->player() && dist( mSock->player() ) < mSock->player()->visualRange() )
			mSock->send( &effect );
	}
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
	effect.setDuration(1);
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
	effect.setDuration(1);
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
			cPythonScript *script = ScriptManager::instance()->find( (*it).latin1() );
			if( script )
				addEvent( script );
			else
				PROPERTY_ERROR( -3, QString( "Script not found: '%1'" ).arg( *it ) )
		}
		return 0;
	}

	return cPythonScriptable::setProperty(name, value);
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

	return cPythonScriptable::getProperty(name, value);
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

const cVariant &cUObject::getTag( const QString& key ) const
{
	return tags_.get( key );
}

bool cUObject::hasTag( const QString& key ) const
{
//	changed_ = true;
	return tags_.has( key );
}

void cUObject::setTag( const QString& key, const cVariant& value )
{
	tags_.set( key, value );
	changed_ = true;
}

void cUObject::removeTag( const QString& key )
{
	changed_ = true;
	tags_.remove( key );
}

QStringList cUObject::getTags() const
{
	return tags_.getKeys();
}

void cUObject::resendTooltip() {
	// Either Attach or Refresh the Data
	if (tooltip_ == 0xFFFFFFFF) {
		tooltip_ = World::instance()->getUnusedTooltip();

		cUOTxAttachTooltip attach;
		attach.setId(tooltip_);
		attach.setSerial(serial_);

		for (cUOSocket *s = cNetwork::instance()->first(); s; s = cNetwork::instance()->next()) {
			if (s->player() && s->player()->inRange(this, s->player()->visualRange())) {
				s->addTooltip(tooltip_);
				s->send(&attach);
			}
		}
	} else {
		cUOTxTooltipList tooltip;

		for (cUOSocket *s = cNetwork::instance()->first(); s; s = cNetwork::instance()->next()) {
			if (s->player() && s->player()->inRange(this, s->player()->visualRange())) {
				createTooltip(tooltip, s->player());
				s->send(&tooltip);
			}
		}
	}
}

/****************************
 *
 * Scripting events
 *
 ****************************/
bool cUObject::onCreate( const QString &definition )
{
	cPythonScript *global = ScriptManager::instance()->getGlobalHook( EVENT_CREATE );
	bool result = false;

	if( scriptChain || global )
	{
		PyObject *args = Py_BuildValue( "O&s", PyGetObjectObject, this, definition.latin1() );

		result = cPythonScript::callChainedEventHandler( EVENT_CREATE, scriptChain, args );

		if( !result && global )
			result = global->callEventHandler( EVENT_CREATE, args );

		Py_DECREF( args );
	}

	return result;
}

bool cUObject::onShowTooltip( P_PLAYER sender, cUOTxTooltipList* tooltip )
{
	cPythonScript *global = ScriptManager::instance()->getGlobalHook( EVENT_SHOWTOOLTIP );
	bool result = false;

	if( scriptChain || global )
	{
		PyObject *args = Py_BuildValue( "O&O&O&", PyGetCharObject, sender, PyGetObjectObject, this, PyGetTooltipObject, tooltip );

		result = cPythonScript::callChainedEventHandler( EVENT_SHOWTOOLTIP, scriptChain, args );

		if( !result && global )
			result = global->callEventHandler( EVENT_SHOWTOOLTIP, args );

		Py_DECREF( args );
	}

	return result;
}

void cUObject::createTooltip(cUOTxTooltipList &tooltip, cPlayer *player) {
	tooltip.resize(19); // Resize to the original size
	tooltip.setSerial(serial_);
	tooltip.setId(tooltip_);
}
