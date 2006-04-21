/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2006 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In addition to that license, if you are running this program or modified
 * versions of it on a public system you HAVE TO make the complete source of
 * the version used by you available or provide people with a location to
 * download it.
 *
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#include "basics.h"
#include "guilds.h"

#include "persistentbroker.h"
#include "dbdriver.h"
#include "world.h"
#include "items.h"

#include <QList>
#include <QSqlQuery>
#include <QVariant>

cGuilds::~cGuilds()
{
	qDeleteAll( guilds );
}

unsigned int cGuilds::findFreeSerial()
{
	unsigned int serial = 1;

	for ( iterator it = begin(); it != end(); ++it )
	{
		if ( it.value()->serial() >= serial )
		{
			serial = it.value()->serial() + 1;
		}
	}

	return serial;
}

void cGuilds::save()
{
	// Clear the tables first: guilds are not saved incremental.
	if (!PersistentBroker::instance()->executeQuery( "TRUNCATE guilds;" )) {
		PersistentBroker::instance()->executeQuery( "DELETE FROM guilds;" );
	}
	if (!PersistentBroker::instance()->executeQuery( "TRUNCATE guilds_members;" )) {
		PersistentBroker::instance()->executeQuery( "DELETE FROM guilds_members;" );
	}
	if (!PersistentBroker::instance()->executeQuery( "TRUNCATE guilds_canidates;" )) {
		PersistentBroker::instance()->executeQuery( "DELETE FROM guilds_canidates;" );
	}
	if (!PersistentBroker::instance()->executeQuery( "TRUNCATE guilds_allies;" )) {
		PersistentBroker::instance()->executeQuery( "DELETE FROM guilds_allies;" );
	}
	if (!PersistentBroker::instance()->executeQuery( "TRUNCATE guilds_enemies;" )) {
		PersistentBroker::instance()->executeQuery( "DELETE FROM guilds_enemies;" );
	}
	
	for ( iterator it = begin(); it != end(); ++it )
	{
		it.value()->save();
	}
}

void cGuilds::load()
{
	// Get all guilds from the database
	QSqlQuery result( "SELECT serial,name,abbreviation,charta,website,alignment,leader,founded,guildstone FROM guilds" );

	while ( result.next() )
	{
		cGuild* guild = new cGuild( false );
		guild->load( result );
	}
}

void cGuild::load( const QSqlQuery& result )
{
	serial_ = result.value( 0 ).toInt();
	name_ = result.value( 1 ).toString();
	abbreviation_ = result.value( 2 ).toString();
	charta_ = result.value( 3 ).toString();
	website_ = result.value( 4 ).toString();
	alignment_ = ( eAlignment ) result.value( 5 ).toInt();
	leader_ = dynamic_cast<P_PLAYER>( World::instance()->findChar( result.value( 6 ).toInt() ) );
	founded_.setTime_t( result.value( 7 ).toInt() );
	guildstone_ = World::instance()->findItem( result.value( 8 ).toInt() );

	// Load members and canidates
	QSqlQuery members( QString( "SELECT player,showsign,guildtitle,joined FROM guilds_members WHERE guild = %1" ).arg( serial_ ) );

	while ( members.next() )
	{
		P_PLAYER player = dynamic_cast<P_PLAYER>( World::instance()->findChar( members.value( 0 ).toInt() ) );

		if ( player )
		{
			player->setGuild( this );
			members_.append( player );

			MemberInfo* info = new MemberInfo;
			info->setShowSign( members.value( 1 ).toInt() != 0 );
			info->setGuildTitle( members.value( 2 ).toString() );
			info->setJoined( members.value( 3 ).toInt() );
			memberinfo_.insert( player, info );
		}
	}

	QSqlQuery canidates( QString( "SELECT player FROM guilds_canidates WHERE guild = %1" ).arg( serial_ ) );

	while ( canidates.next() )
	{
		P_PLAYER player = dynamic_cast<P_PLAYER>( World::instance()->findChar( canidates.value( 0 ).toInt() ) );

		if ( player )
		{
			player->setGuild( this );
			canidates_.append( player );
		}
	}

	// Clear the leader if he's not a member of the guild
	if ( !members_.contains( leader_ ) )
	{
		leader_ = 0;
	}

	Guilds::instance()->registerGuild( this );

	QSqlQuery allies( QString( "SELECT ally FROM guilds_allies WHERE guild = %1" ).arg( serial_ ) );

	while ( allies.next() )
	{
		allies_.append( allies.value( 0 ).toInt() );
	}

	QSqlQuery enemies( QString( "SELECT enemy FROM guilds_enemies WHERE guild = %1" ).arg( serial_ ) );

	while ( enemies.next() )
	{
		enemies_.append( enemies.value( 0 ).toInt() );
	}
}

void cGuild::save()
{
	QStringList fields;
	fields.append( QString::number( serial_ ) );
	fields.append( QString( "'%1'" ).arg( PersistentBroker::instance()->quoteString( name_ ) ) );
	fields.append( QString( "'%1'" ).arg( PersistentBroker::instance()->quoteString( abbreviation_ ) ) );
	fields.append( QString( "'%1'" ).arg( PersistentBroker::instance()->quoteString( charta_ ) ) );
	fields.append( QString( "'%1'" ).arg( PersistentBroker::instance()->quoteString( website_ ) ) );
	fields.append( QString::number( alignment_ ) );
	if ( leader_ )
	{
		fields.append( QString::number( leader_->serial() ) );
	}
	else
	{
		fields.append( "-1" );
	}
	fields.append( QString::number( founded_.toTime_t() ) );
	if ( guildstone_ )
	{
		fields.append( QString::number( guildstone_->serial() ) );
	}
	else
	{
		fields.append( "-1" );
	}

	PersistentBroker::instance()->executeQuery( QString( "INSERT INTO guilds VALUES(%1);" ).arg( fields.join( "," ) ) );

	// Save Members/Canidates
	P_PLAYER player;

	foreach ( player, members_ )
	{
		MemberInfo* info = getMemberInfo( player );
		PersistentBroker::instance()->executeQuery( QString( "INSERT INTO guilds_members VALUES(%1,%2,%3,'%4',%5);" )
			.arg( serial_ ).arg( player->serial() ).arg( info->showSign() ? 1 : 0 )
			.arg( PersistentBroker::instance()->quoteString( info->guildTitle() ) )
			.arg( info->joined() ) );
	}

	foreach ( player, canidates_ )
	{
		PersistentBroker::instance()->executeQuery( QString( "INSERT INTO guilds_canidates VALUES(%1,%2);" ).arg( serial_ ).arg( player->serial() ) );
	}

	QList<unsigned int>::iterator it;

	for (it = allies_.begin(); it != allies_.end(); ++it) {
		unsigned int serial = *it;
		PersistentBroker::instance()->executeQuery( QString( "INSERT INTO guilds_allies VALUES(%1,%2);" ).arg( serial_ ).arg( serial ) );
	}

	for (it = enemies_.begin(); it != enemies_.end(); ++it) {
		unsigned int serial = *it;
		PersistentBroker::instance()->executeQuery( QString( "INSERT INTO guilds_enemies VALUES(%1,%2);" ).arg( serial_ ).arg( serial ) );
	}
}

cGuild::cGuild( bool createSerial )
{
	if ( createSerial )
	{
		serial_ = Guilds::instance()->findFreeSerial();
		Guilds::instance()->registerGuild( this );
	}
	else
	{
		serial_ = 0;
	}

	alignment_ = Neutral;
	name_ = QString::null;
	abbreviation_ = QString::null;
	website_ = QString::null;
	charta_ = QString::null;
	founded_ = QDateTime::currentDateTime();
	guildstone_ = 0;
	leader_ = 0;
}

cGuild::~cGuild()
{
	P_PLAYER player;

	foreach ( player, members_ )
	{
		player->setGuild( 0 );

		// Remove the MemberInfo structure
		MemberInfo* info = getMemberInfo( player );
		delete info;
		memberinfo_.remove( player );
	}

	foreach ( player, canidates_ )
	{
		player->setGuild( 0 );

		// Remove the MemberInfo structure
		MemberInfo* info = getMemberInfo( player );
		delete info;
		memberinfo_.remove( player );
	}

	QList<unsigned int>::iterator it;

	for (it = allies_.begin(); it != allies_.end(); ++it) {
		unsigned int serial = *it;
		cGuild *guild = Guilds::instance()->findGuild(serial);
		if (guild && guild != this) {
			guild->allies_.removeAll(serial_);
		}
	}

	for (it = enemies_.begin(); it != enemies_.end(); ++it) {
		unsigned int serial = *it;
		cGuild *guild = Guilds::instance()->findGuild(serial);
		if (guild && guild != this) {
			guild->enemies_.removeAll(serial_);
		}
	}
}

void cGuild::setSerial( unsigned int data )
{
	if ( serial_ != data && data )
	{
		if ( serial_ )
		{
			Guilds::instance()->unregisterGuild( this );
		}

		serial_ = data;
		Guilds::instance()->registerGuild( this );
	}
}

void cGuilds::registerGuild( cGuild* guild )
{
	iterator it = guilds.find( guild->serial() );

	if ( it != guilds.end() )
	{
		if ( it.value() != guild )
		{
			delete it.value();
			guilds.erase( it );
		}
		else
		{
			return; // Already registered
		}
	}

	guilds.insert( guild->serial(), guild );
}

void cGuilds::unregisterGuild( cGuild* guild )
{
	guilds.remove( guild->serial() );
}

cGuild* cGuilds::findGuild( unsigned int serial )
{
	cGuild* result = 0;

	if ( serial )
	{
		iterator it = guilds.find( serial );

		if ( it != end() )
		{
			result = it.value();
		}
	}

	return result;
}

void cGuild::addMember( P_PLAYER member, MemberInfo* info )
{
	if ( !members_.contains( member ) )
	{
		canidates_.removeAll( member );
		members_.append( member );

		if ( memberinfo_.contains( member ) )
		{
			delete memberinfo_[member];
			memberinfo_.remove( member );
		}

		if ( !info )
		{
			info = new MemberInfo;
		}

		memberinfo_.insert( member, info );
		member->setGuild( this );
	}
}

void cGuild::removeMember( P_PLAYER member )
{
	canidates_.removeAll( member );
	members_.removeAll( member );
	member->setGuild( 0 );
	memberinfo_.remove( member );

	if ( leader_ == member )
	{
		leader_ = 0;
	}
}

void cGuild::addCanidate( P_PLAYER canidate )
{
	// Only add him to the canidates if he is not a member already
	if ( !members_.contains( canidate ) )
	{
		canidates_.append( canidate );
		canidate->setGuild( this );
	}
}

void cGuild::removeCanidate( P_PLAYER canidate )
{
	members_.removeAll( canidate );
	canidates_.removeAll( canidate );
	canidate->setGuild( 0 );
}

const char* cGuild::className() const
{
	return "guild";
}

bool cGuild::implements( const QString& name ) const
{
	if ( name == cGuild::className() )
	{
		return true;
	}
	else
	{
		return cPythonScriptable::implements( name );
	}
}

/*!
	The python object structure for a guild.
*/
struct wpGuild
{
	PyObject_HEAD;
	cGuild* guild;
};

static PyObject* wpGuild_getAttr( wpGuild* self, char* name );
static int wpGuild_setAttr( wpGuild* self, char* name, PyObject* value );
static int wpGuild_compare( PyObject* a, PyObject* b );

/*
	\object guild
	\description This object type represents a guild.
*/
PyTypeObject wpGuildType =
{
PyObject_HEAD_INIT( NULL )
0,
"guild",
sizeof( wpGuildType ),
0,
wpDealloc,
0,
( getattrfunc ) wpGuild_getAttr,
( setattrfunc ) wpGuild_setAttr,
wpGuild_compare,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

static int wpGuild_compare( PyObject* a, PyObject* b )
{
	if ( a->ob_type != &wpGuildType || b->ob_type != &wpGuildType )
		return -1;

	return !( ( ( wpGuild * ) a )->guild == ( ( wpGuild * ) b )->guild );
}

/*
	\method guild.addmember
	\param player A <object id="char">char</object> object for a player.
	\return False if a npc was passed, True otherwise.
	\description This method adds a member to this guild.
*/
PyObject* wpGuild_addmember( wpGuild* self, PyObject* args )
{
	P_CHAR character;
	if ( PyArg_ParseTuple( args, "O&:guild.addmember(char)", PyConvertChar, &character ) )
	{
		P_PLAYER player = dynamic_cast<P_PLAYER>( character );
		if ( player )
		{
			self->guild->addMember( player );
			Py_RETURN_TRUE;
		}
		else
		{
			Py_RETURN_FALSE;
		}
	}
	return 0;
}

/*
	\method guild.removemember
	\param player A <object id="char">char</object> object for a player.
	\return False if a npc was passed, True otherwise.
	\description This method removes a member from this guild.
*/
PyObject* wpGuild_removemember( wpGuild* self, PyObject* args )
{
	P_CHAR character;
	if ( PyArg_ParseTuple( args, "O&:guild.removemember(char)", PyConvertChar, &character ) )
	{
		P_PLAYER player = dynamic_cast<P_PLAYER>( character );
		if ( player )
		{
			self->guild->removeMember( player );
			Py_RETURN_TRUE;
		}
		else
		{
			Py_RETURN_FALSE;
		}
	}
	return 0;
}

/*
	\method guild.addcanidate
	\param player A <object id="char">char</object> object for a player.
	\return False if a npc was passed, True otherwise.
	\description This method adds a canidate to this guild.
*/
PyObject* wpGuild_addcanidate( wpGuild* self, PyObject* args )
{
	P_CHAR character;
	if ( PyArg_ParseTuple( args, "O&:guild.addcanidate(char)", PyConvertChar, &character ) )
	{
		P_PLAYER player = dynamic_cast<P_PLAYER>( character );
		if ( player )
		{
			self->guild->addCanidate( player );
			Py_RETURN_TRUE;
		}
		else
		{
			Py_RETURN_FALSE;
		}
	}
	return 0;
}

/*
	\method guild.removecanidate
	\param player A <object id="char">char</object> object for a player.
	\return False if a npc was passed, True otherwise.
	\description This method removes a canidate from this guild.
*/
PyObject* wpGuild_removecanidate( wpGuild* self, PyObject* args )
{
	P_CHAR character;
	if ( PyArg_ParseTuple( args, "O&:guild.removecanidate(char)", PyConvertChar, &character ) )
	{
		P_PLAYER player = dynamic_cast<P_PLAYER>( character );
		if ( player )
		{
			self->guild->removeCanidate( player );
			Py_RETURN_TRUE;
		}
		else
		{
			Py_RETURN_FALSE;
		}
	}
	return 0;
}

/*
	\method guild.setmemberinfo
	\param player A <object id="char">char</object> object for a player.
	\param info A dictionary with represents the member information for the given player in this guilds.
	The following keys are valid:
	- <code>showsign</code> Set this item to True if the guild abbreviation should be appended to the players name.
	- <code>guildtitle</code> If this item is not None, it will be shown as this members title within the guild.
	- <code>joined</code> This is a UNIX timestamp representing the time and date when this member joined the guild.
	\description This method changes the member information structure for a member of this guild.
*/
PyObject* wpGuild_setmemberinfo( wpGuild* self, PyObject* args )
{
	P_PLAYER player;
	PyObject* dict;

	if ( !PyArg_ParseTuple( args, "O&O!:guild.setmemberinfo(player, info)", PyConvertPlayer, &player, &PyDict_Type, &dict ) )
	{
		return 0;
	}

	cGuild::MemberInfo* info = self->guild->getMemberInfo( player );

	if ( !info )
	{
		PyErr_SetString( PyExc_RuntimeError, "The supplied character is not a member of this guild." );
		return 0;
	}

	PyObject* showsign = PyDict_GetItemString( dict, "showsign" );
	if ( showsign )
	{
		info->setShowSign( PyObject_IsTrue( showsign ) );
	}

	PyObject* guildtitle = PyDict_GetItemString( dict, "guildtitle" );
	if ( guildtitle )
	{
		info->setGuildTitle( Python2QString( guildtitle ) );
	}

	PyObject* joined = PyDict_GetItemString( dict, "joined" );
	if ( joined )
	{
		if ( PyInt_Check( joined ) )
		{
			info->setJoined( ( unsigned int ) PyInt_AsLong( joined ) );
		}
		else if ( PyLong_Check( joined ) )
		{
			info->setJoined( ( unsigned int ) floor( PyFloat_AsDouble( joined ) ) );
		}
	}

	Py_RETURN_TRUE;
}

/*
	\method guild.getmemberinfo
	\param player A <object id="char">char</object> object for a player.
	\return A dictionary with data from the member information structure for the given player in this guilds.
	The following keys are valid:
	- <code>showsign</code> This item is True if the guilds abbreviation is appended to this players name.
	- <code>guildtitle</code> This item is the players title within the guild. It is a unicode string.
	- <code>joined</code> This is a UNIX timestamp representing the time and date when this member joined the guild.
	\description This method retrieves the memebr information structure for a member of this guild.
*/
PyObject* wpGuild_getmemberinfo( wpGuild* self, PyObject* args )
{
	P_PLAYER player;

	if ( !PyArg_ParseTuple( args, "O&:guild.getmemberinfo(player)", PyConvertPlayer, &player ) )
	{
		return 0;
	}

	cGuild::MemberInfo* info = self->guild->getMemberInfo( player );

	if ( !info )
	{
		PyErr_SetString( PyExc_RuntimeError, "The supplied character is not a member of this guild." );
		return 0;
	}

	// Return a dictionary with three elements
	PyObject* result = PyDict_New();
	PyDict_SetStolenItem( result, "showsign", PyInt_FromLong( info->showSign() ? 1 : 0 ) );
	PyDict_SetStolenItem( result, "joined", PyInt_FromLong( info->joined() ) );
	PyDict_SetStolenItem( result, "guildtitle", QString2Python( info->guildTitle() ) );
	return result;
}

/*
	\method guild.delete
	\description Delete this guild.
*/
PyObject* wpGuild_delete( wpGuild* self, PyObject* args )
{
	Q_UNUSED( args );
	Guilds::instance()->unregisterGuild( self->guild );
	delete self->guild;
	self->guild = 0;
	Py_RETURN_NONE;
}

/*
	\method guild.addally
	\param ally The other guild.
	\description Add an ally to this guild and remove it from the enemy list if neccesary.
*/
PyObject* wpGuild_addally( wpGuild* self, PyObject* args )
{
	wpGuild *pyguild;

	if ( !PyArg_ParseTuple( args, "O!:guild.addally(ally)", &wpGuildType, &pyguild ) ) {
		return 0;
	}

	if (pyguild->guild) {
		self->guild->addAlly(pyguild->guild);
	}

	Py_RETURN_NONE;
}

/*
	\method guild.addenemy
	\param enemy The other guild.
	\description Add an enemy to this guild and remove it from the ally list if neccesary.
*/
PyObject* wpGuild_addenemy( wpGuild* self, PyObject* args )
{
	wpGuild *pyguild;

	if ( !PyArg_ParseTuple( args, "O!:guild.addenemy(enemy)", &wpGuildType, &pyguild ) ) {
		return 0;
	}

	if (pyguild->guild) {
		self->guild->addEnemy(pyguild->guild);
	}

	Py_RETURN_NONE;
}

/*
	\method guild.removeally
	\param ally The other guild.
	\description Remove an ally from the ally list of this guild.
*/
PyObject* wpGuild_removeally( wpGuild* self, PyObject* args )
{
	wpGuild *pyguild;

	if ( !PyArg_ParseTuple( args, "O!:guild.removeally(ally)", &wpGuildType, &pyguild ) ) {
		return 0;
	}

	if (pyguild->guild) {
		self->guild->removeAlly(pyguild->guild);
	}

	Py_RETURN_NONE;
}

/*
	\method guild.removeenemy
	\param enemy The other guild.
	\description Remove an enemy from the enemy list of this guild.
*/
PyObject* wpGuild_removeenemy( wpGuild* self, PyObject* args )
{
	wpGuild *pyguild;

	if ( !PyArg_ParseTuple( args, "O!:guild.removeenemy(enemy)", &wpGuildType, &pyguild ) ) {
		return 0;
	}

	if (pyguild->guild) {
		self->guild->removeEnemy(pyguild->guild);
	}

	Py_RETURN_NONE;
}

/*
	\method guild.isallied
	\param guild The other guild.
	\return True if the guilds are allied. False otherwise.
	\description Checks if this guild is allied with the given guild.
*/
PyObject* wpGuild_isallied( wpGuild* self, PyObject* args )
{
	wpGuild *pyguild;

	if ( !PyArg_ParseTuple( args, "O!:guild.isallied(guild)", &wpGuildType, &pyguild ) ) {
		return 0;
	}

	if (pyguild->guild) {
		if (self->guild->isAllied(pyguild->guild)) {
			Py_RETURN_TRUE;
		}
	}

	Py_RETURN_FALSE;
}

/*
	\method guild.isatwar
	\param guild The other guild.
	\return True if the guilds are at war. False otherwise.
	\description Checks if this guild is at war with the given guild.
*/
PyObject* wpGuild_isatwar( wpGuild* self, PyObject* args )
{
	wpGuild *pyguild;

	if ( !PyArg_ParseTuple( args, "O!:guild.isatwar(guild)", &wpGuildType, &pyguild ) ) {
		return 0;
	}

	if (pyguild->guild) {
		if (self->guild->isAtWar(pyguild->guild)) {
			Py_RETURN_TRUE;
		}
	}

	Py_RETURN_FALSE;
}

static PyMethodDef wpGuildMethods[] =
{
	{"delete", ( getattrofunc ) wpGuild_delete, METH_VARARGS, 0},
	{"addmember", ( getattrofunc ) wpGuild_addmember, METH_VARARGS, 0},
	{"removemember", ( getattrofunc ) wpGuild_removemember, METH_VARARGS, 0},
	{"addcanidate", ( getattrofunc ) wpGuild_addcanidate, METH_VARARGS, 0},
	{"removecanidate", ( getattrofunc ) wpGuild_removecanidate, METH_VARARGS, 0},
	{"getmemberinfo", ( getattrofunc ) wpGuild_getmemberinfo, METH_VARARGS, 0},
	{"setmemberinfo", ( getattrofunc ) wpGuild_setmemberinfo, METH_VARARGS, 0},
	{"addally", ( getattrofunc ) wpGuild_addally, METH_VARARGS, 0},
	{"addenemy", ( getattrofunc ) wpGuild_addenemy, METH_VARARGS, 0},
	{"removeally", ( getattrofunc ) wpGuild_removeally, METH_VARARGS, 0},
	{"removeenemy", ( getattrofunc ) wpGuild_removeenemy, METH_VARARGS, 0},
	{"isatwar", ( getattrofunc ) wpGuild_isatwar, METH_VARARGS, 0},
	{"isallied", ( getattrofunc ) wpGuild_isallied, METH_VARARGS, 0},
	{0, 0, 0, 0}
};

static PyObject* wpGuild_getAttr( wpGuild* self, char* name )
{
	if ( !self->guild )
	{
		PyErr_SetString( PyExc_RuntimeError, "Calling member functions after deletion." );
		return 0;
	}

	/*
		\rproperty guild.members This property is a list of <object id="char">char</object> objects for every member of the guild.
	*/
	if ( !strcmp( name, "members" ) )
	{
		QList<cPlayer*>& members = self->guild->members();
		PyObject* list = PyTuple_New( members.count() );
		unsigned int i = 0;

		foreach ( P_PLAYER player, members )
		{
			PyTuple_SetItem( list, i++, player->getPyObject() );
		}

		return list;
	}
	/*
		\rproperty guild.canidates This property is a list of <object id="char">char</object> objects for every canidate of the guild.
	*/
	else if ( !strcmp( name, "canidates" ) )
	{
		QList<cPlayer*>& canidates = self->guild->canidates();
		PyObject* list = PyTuple_New( canidates.count() );
		unsigned int i = 0;

		foreach ( P_PLAYER player, canidates )
		{
			PyTuple_SetItem( list, i++, player->getPyObject() );
		}

		return list;
	}
	/*
		\property guild.leader This property is a <object id="char">char</object> object for the leader of the guild. This can also be None if there
		is no leader.
	*/
	else if ( !strcmp( name, "leader" ) )
	{
		if ( !self->guild->leader() )
		{
			Py_RETURN_NONE;
		}
		else
		{
			return self->guild->leader()->getPyObject();
		}
	}
	/*
		\property guild.name This is the name of this guild.
	*/
	else if ( !strcmp( name, "name" ) )
	{
		const QString& value = self->guild->name();
		return QString2Python( value );
	}
	/*
		\property guild.abbreviation This is the abbreviation of this guilds name.
	*/
	else if ( !strcmp( name, "abbreviation" ) )
	{
		const QString& value = self->guild->abbreviation();
		return QString2Python( value );
	}
	/*
		\property guild.charta This is the charta of this guild.
	*/
	else if ( !strcmp( name, "charta" ) )
	{
		const QString& value = self->guild->charta();
		return QString2Python( value );
	}
	/*
		\property guild.website This is the URL of the website for this guild.
	*/
	else if ( !strcmp( name, "website" ) )
	{
		const QString& value = self->guild->website();
		return QString2Python( value );
	}
	/*
		\property guild.alignment This integer value indicates the alignment of the guild.
		<code>0: Neutral
		1: Evil
		2: Good</code>
	*/
	else if ( !strcmp( name, "alignment" ) )
	{
		return PyInt_FromLong( self->guild->alignment() );
	}
	/*
		\property guild.serial This is the unique integer id for this guild. It can be used to
		retrieve a guild object by using the wolfpack.findguild function.
	*/
	else if ( !strcmp( name, "serial" ) )
	{
		return PyInt_FromLong( self->guild->serial() );
	}
	/*
		\property guild.founded This property is a UNIX timestamp indicating when this guild was founded.
	*/
	else if ( !strcmp( name, "founded" ) )
	{
		return PyInt_FromLong( self->guild->founded().toTime_t() );
	}
	/*
		\rproperty guild.allies A tuple of serials of guilds that are allied with this guild.
	*/
	else if ( !strcmp( name, "allies" ) )
	{
		const QList<unsigned int> &allies = self->guild->allies();
		QList<unsigned int>::const_iterator it;

		PyObject *result = PyTuple_New( allies.size() );
		unsigned int i = 0;

		for ( it = allies.begin(); it != allies.end(); ++it ) {
			cGuild *other = Guilds::instance()->findGuild(*it);

			if (other) {
				PyTuple_SetItem(result, i++, other->getPyObject());
			} else {
				Py_INCREF(Py_None);
				PyTuple_SetItem(result, i++, Py_None);
			}
		}

		return result;
	}

	/*
		\rproperty guild.enemies A tuple of serials of guilds that are at war with this guild.
	*/
	else if ( !strcmp( name, "enemies" ) )
	{
		const QList<unsigned int> &enemies = self->guild->enemies();
		QList<unsigned int>::const_iterator it;

		PyObject *result = PyTuple_New( enemies.size() );
		unsigned int i = 0;

		for ( it = enemies.begin(); it != enemies.end(); ++it ) {
			cGuild *other = Guilds::instance()->findGuild(*it);

			if (other) {
				PyTuple_SetItem(result, i++, other->getPyObject());
			} else {
				Py_INCREF(Py_None);
				PyTuple_SetItem(result, i++, Py_None);
			}
		}

		return result;
	}

	/*
		\property guild.guildstone This is the main guildstone for this guild. It can be None if there is no main guildstone for this guild.
	*/
	else if ( !strcmp( name, "guildstone" ) )
	{
		if ( self->guild->guildstone() )
		{
			return self->guild->guildstone()->getPyObject();
		}
		else
		{
			Py_RETURN_NONE;
		}
	}

	return Py_FindMethod( wpGuildMethods, ( PyObject * ) self, name );
}

static int wpGuild_setAttr( wpGuild* self, char* name, PyObject* value )
{
	if ( !self->guild )
	{
		PyErr_SetString( PyExc_RuntimeError, "Calling member functions after deletion." );
		return 0;
	}

	if ( !strcmp( name, "leader" ) )
	{
		P_PLAYER leader = dynamic_cast<P_PLAYER>( getWpChar( value ) );
		self->guild->setLeader( leader );
	}
	else if ( !strcmp( name, "guildstone" ) )
	{
		P_ITEM guildstone = getWpItem( value );
		self->guild->setGuildstone( guildstone );
	}
	else if ( !strcmp( name, "name" ) )
	{
		self->guild->setName( Python2QString( value ) );
	}
	else if ( !strcmp( name, "abbreviation" ) )
	{
		self->guild->setAbbreviation( Python2QString( value ) );
	}
	else if ( !strcmp( name, "charta" ) )
	{
		self->guild->setCharta( Python2QString( value ) );
	}
	else if ( !strcmp( name, "website" ) )
	{
		self->guild->setWebsite( Python2QString( value ) );
	}
	else if ( !strcmp( name, "alignment" ) )
	{
		if ( PyInt_Check( value ) )
		{
			self->guild->setAlignment( ( cGuild::eAlignment ) PyInt_AsLong( value ) );
		}
	}
	else if ( !strcmp( name, "founded" ) )
	{
		if ( PyInt_Check( value ) )
		{
			QDateTime founded;
			founded.setTime_t( PyInt_AsLong( value ) );
			self->guild->setFounded( founded );
		}
		else if ( PyFloat_Check( value ) )
		{
			QDateTime founded;
			founded.setTime_t( ( int ) ceil( PyFloat_AsDouble( value ) ) );
			self->guild->setFounded( founded );
		}
	}
	else
	{
		return -1;
	}

	return 0;
}

// Python Implementation
PyObject* cGuild::getPyObject()
{
	wpGuild* returnVal = PyObject_New( wpGuild, &wpGuildType );
	returnVal->guild = this;
	return ( PyObject * ) returnVal;
}

void cGuild::load( cBufferedReader& reader, unsigned int version )
{
	serial_ = reader.readInt();
	name_ = reader.readUtf8();
	abbreviation_ = reader.readUtf8();
	charta_ = reader.readUtf8();
	website_ = reader.readUtf8();
	alignment_ = ( eAlignment ) reader.readByte();
	leader_ = dynamic_cast<P_PLAYER>( World::instance()->findChar( reader.readInt() ) );
	founded_.setTime_t( reader.readInt() );
	guildstone_ = World::instance()->findItem( reader.readInt() );

	// Save Members/Canidates

	int count, i;
	count = reader.readInt();
	for ( i = 0; i < count; ++i )
	{
		P_PLAYER player = dynamic_cast<P_PLAYER>( World::instance()->findChar( reader.readInt() ) );

		MemberInfo* info = new MemberInfo;
		info->setShowSign( reader.readBool() );
		info->setGuildTitle( reader.readUtf8() );
		info->setJoined( reader.readInt() );

		if ( player )
		{
			player->setGuild( this );
			members_.append( player );
			memberinfo_.insert( player, info );
		}
		else
		{
			delete info;
		}
	}

	count = reader.readInt();
	for ( i = 0; i < count; ++i )
	{
		P_PLAYER player = dynamic_cast<P_PLAYER>( World::instance()->findChar( reader.readInt() ) );

		if ( player )
		{
			addCanidate( player );
		}
	}

	if (version >= 10) {
		count = reader.readInt();
		for ( i = 0; i < count; ++i ) {
			allies_.append( reader.readInt() );
		}

		count = reader.readInt();
		for ( i = 0; i < count; ++i ) {
			enemies_.append( reader.readInt() );
		}
	}
}

void cGuild::save( cBufferedWriter& writer, unsigned int version )
{
	writer.writeByte( 0xFD );

	writer.writeInt( serial_ );
	writer.writeUtf8( name_ );
	writer.writeUtf8( abbreviation_ );
	writer.writeUtf8( charta_ );
	writer.writeUtf8( website_ );
	writer.writeByte( alignment_ );
	writer.writeInt( leader_ ? leader_->serial() : INVALID_SERIAL );
	writer.writeInt( founded_.toTime_t() );
	writer.writeInt( guildstone_ ? guildstone_->serial() : INVALID_SERIAL );

	// Save Members/Canidates
	P_PLAYER player;

	writer.writeInt( members_.count() );
	foreach ( player, members_ )
	{
		MemberInfo* info = getMemberInfo( player );
		writer.writeInt( player->serial() );
		writer.writeBool( info->showSign() );
		writer.writeUtf8( info->guildTitle() );
		writer.writeInt( info->joined() );
	}

	writer.writeInt( canidates_.count() );
	foreach ( player, canidates_ )
	{
		writer.writeInt( player->serial() );
	}

	if (version >= 10) {
		QList<unsigned int>::iterator it;

		writer.writeInt(allies_.count());
		for (it = allies_.begin(); it != allies_.end(); ++it) {
			writer.writeInt(*it);
		}

		writer.writeInt(enemies_.count());
		for (it = enemies_.begin(); it != enemies_.end(); ++it) {
			writer.writeInt(*it);
		}
	}
}

void cGuild::addEnemy(cGuild *enemy) {
	if (enemy != this) {
		removeAlly(enemy);

		// Add them to our enemy list
		if (!enemies_.contains(enemy->serial())) {
			enemies_.append(enemy->serial());
		}

		// Add us to their enemy list.
		if (!enemy->enemies_.contains(serial_)) {
			enemy->enemies_.append(serial_);
		}
	}
}

void cGuild::addAlly(cGuild *ally) {
	if (ally != this) {
		removeEnemy(ally); // Remove them first

		// Add them to our ally list
		if (!allies_.contains(ally->serial())) {
			allies_.append(ally->serial());
		}

		// Add us to their ally list
		if (!ally->allies_.contains(serial_)) {
			ally->allies_.append(serial_);
		}
	}
}

void cGuild::removeAlly(cGuild *ally) {
	allies_.removeAll( ally->serial() );
	ally->allies_.removeAll( serial_ ); // Remove us from their list
}

void cGuild::removeEnemy(cGuild *enemy) {
	enemies_.removeAll( enemy->serial() );
	enemy->enemies_.removeAll( serial_ ); // Remove us from their list
}

bool cGuild::isAllied(cGuild *other) {
	return allies_.contains(other->serial());
}

bool cGuild::isAtWar(cGuild *other) {
	return enemies_.contains(other->serial());
}
