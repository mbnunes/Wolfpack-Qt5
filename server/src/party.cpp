/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: http://wpdev.sf.net/
 */

#include "party.h"
#include "player.h"
#include "npc.h"
#include "world.h"
#include "network/uosocket.h"
#include "network/uotxpackets.h"

cParty::cParty( P_PLAYER leader )
{
	leader_ = leader;
	addMember( leader );
}

cParty::~cParty()
{
	P_PLAYER member;
	for ( member = members_.first(); member; member = members_.next() )
	{
		member->setParty( 0 );

		if ( member->socket() )
		{
			cUOTxPartyRemoveMember updateparty;
			updateparty.setSerial( member->serial() );
			member->socket()->send( &updateparty );
			member->socket()->clilocMessage( 1005449 );
		}
	}

	for ( member = canidates_.first(); member; member = canidates_.next() )
	{
		if ( member->socket() )
		{
			cUOTxPartyRemoveMember updateparty;
			updateparty.setSerial( member->serial() );
			member->socket()->send( &updateparty );
			member->socket()->clilocMessage( 1005449 );
		}
	}
}

void cParty::addMember( P_PLAYER player, bool update )
{
	// Remove him from his old party
	if ( player->party() )
		player->party()->removeMember( player );
	else
		removeCanidate( player );

	if ( update )
		send( 1008094, QString::null, player->name(), true );

	members_.append( player );
	player->setParty( this );

	// Send the new memberlist to all members
	if ( update )
	{
		this->update();

		if ( player != leader_ && player->socket() )
			player->socket()->clilocMessage( 1005445 );
	}
}

void cParty::removeMember( P_PLAYER player, bool update )
{
	removeCanidate( player );
	members_.remove( player );
	player->setParty( 0 );

	// Update the party
	if ( update )
	{
		cUOTxPartyRemoveMember removemember;
		removemember.setSerial( player->serial() );
		for ( P_PLAYER member = members_.first(); member; member = members_.next() )
			removemember.addMember( member->serial() );

		send( &removemember );

		if ( player->socket() )
		{
			if ( player == leader_ )
				player->socket()->clilocMessage( 1005456 );

			cUOTxPartyRemoveMember updateparty;
			updateparty.setSerial( player->serial() );
			player->socket()->send( &updateparty );
		}
	}

	// Check if the party can be disbanded
	if ( player == leader_ || ( members_.count() <= 1 && canidates_.count() == 0 ) )
	{
		if ( player != leader_ && leader_->socket() )
			leader_->socket()->clilocMessage( 1005450 );

		delete this;
	}
}

void cParty::removeCanidate( P_PLAYER player )
{
	canidates_.remove( player );
}

void cParty::addCanidate( P_PLAYER player )
{
	canidates_.append( player );
}

void cParty::update()
{
	cUOTxPartyUpdate update;

	for ( P_PLAYER player = members_.first(); player; player = members_.next() )
		update.addMember( player->serial() );

	send( &update );
}

void cParty::send( cUOPacket* packet )
{
	for ( P_PLAYER player = members_.first(); player; player = members_.next() )
		if ( player->socket() )
			player->socket()->send( packet );
}

void cParty::send( unsigned int message, const QString& params, const QString& affix, bool prepend )
{
	for ( P_PLAYER player = members_.first(); player; player = members_.next() )
		if ( player->socket() )
			player->socket()->clilocMessageAffix( message, params, affix, 0x3B2, 3, 0, false, prepend );
}

void cParty::send( P_PLAYER from, const QString& message )
{
	cUOTxPartyTellMember tell;
	tell.setSerial( from->serial() );
	tell.setText( message );

	for ( P_PLAYER player = members_.first(); player; player = members_.next() )
		if ( player->socket() )
			player->socket()->send( &tell );
}

void cParty::send( P_PLAYER from, P_PLAYER target, const QString& message )
{
	cUOTxPartyTellMember tell;
	tell.setSerial( from->serial() );
	tell.setText( message );

	for ( P_PLAYER player = members_.first(); player; player = members_.next() )
		if ( player == target && player->socket() )
			player->socket()->send( &tell );
}

void cParty::setLootingAllowed( P_PLAYER player, bool allowed )
{
	if ( allowed )
	{
		if ( !lootingAllowed_.contains( player ) )
			lootingAllowed_.append( player );
	}
	else
		lootingAllowed_.remove( player );
}

/*!
	\brief Temporary effect for canceling a party invitation after a given timeout.
*/
class cPartyCancelInvitation : public cTimer
{
protected:
	SERIAL leader;

public:
	cPartyCancelInvitation( SERIAL leader, SERIAL player )
	{
		this->leader = leader;
		destSer = player;
		setExpiretime_s( 10 );
		serializable = false;
		dispellable = false;
		objectid = "cancelpartyinvitation";
	}

	/*!
		If source is 0 and silent is true, nothing happens. Otherwise
		the canidate is removed from the party without notification.
	*/
	void Dispel( P_CHAR source, bool silent )
	{
		P_PLAYER player = dynamic_cast<P_PLAYER>( World::instance()->findChar( destSer ) );
		P_PLAYER leader = dynamic_cast<P_PLAYER>( World::instance()->findChar( this->leader ) );

		if ( leader && leader->party() && !silent )
			leader->party()->removeMember( player );
	}

	/*!
		Remove the canidate from the party and notify him.
	*/
	void Expire()
	{
		// Check if the player is still waiting for the party acceptance
		P_PLAYER leader = dynamic_cast<P_PLAYER>( World::instance()->findChar( this->leader ) );
		P_PLAYER player = dynamic_cast<P_PLAYER>( World::instance()->findChar( destSer ) );

		// Only send a decline party invitation packet if the player really is
		// still invited to join the party
		if ( leader && leader->party() && player && player->socket() && leader->party()->canidates().contains( player ) )
		{
			player->socket()->log( LOG_TRACE, QString( "Party invitation from '%1' timed out.\n" ).arg( leader->account()->login() ) );
			leader->party()->removeMember( player ); // This automatically resends and checks the party
		}
	}
};

/*!
	\brief Target request for sending a party invitation to a given player.
*/
class cPartyInvitation : public cTargetRequest
{
protected:
	SERIAL player;

public:
	cPartyInvitation( SERIAL player )
	{
		this->player = player;
	}

	bool responsed( cUOSocket* socket, cUORxTarget* target )
	{
		if ( !isCharSerial( target->serial() ) )
		{
			socket->clilocMessage( 1005442 );
		}
		else
		{
			P_PLAYER leader = dynamic_cast<P_PLAYER>( World::instance()->findChar( this->player ) );
			P_CHAR character = World::instance()->findChar( target->serial() );
			P_NPC npc = dynamic_cast<P_NPC>( character );
			P_PLAYER player = dynamic_cast<P_PLAYER>( character );

			if ( !leader || ( leader->party() && leader->party()->leader() != leader ) )
			{
				socket->clilocMessage( 1005453 );
			}
			else if ( leader->party() && leader->party()->members().count() + leader->party()->canidates().count() >= 10 )
			{
				socket->clilocMessage( 1008095 );
				// NPC targetted
			}
			else if ( npc )
			{
				if ( npc->isHuman() )
					socket->clilocMessage( 1005443, 0, npc->saycolor(), 3, npc );
				else
					socket->clilocMessage( 1005444 );
			}
			else if ( leader == player )
			{
				socket->clilocMessage( 1005439 );
			}
			else if ( player && player->party() && player->party() == leader->party() )
			{
				socket->clilocMessage( 1005440 );
			}
			else if ( player && leader->party() && leader->party()->canidates().contains( player ) )
			{
				socket->clilocMessage( 1005440 );
			}
			else if ( player && player->party() )
			{
				socket->clilocMessage( 1005441 );
			}
			else if ( player && player->socket() )
			{
				if ( !leader->party() )
				{
					new cParty( leader );
					leader->party()->update();
				}

				player->socket()->clilocMessageAffix( 1008089, 0, leader->name(), 0x3b2, 3, 0, false, true );
				leader->party()->addCanidate( player );
				socket->log( LOG_TRACE, QString( "Invited '%1' to join his party.\n" ).arg( player->account()->login() ) );

				// Send a party invitation request
				cUOTxPartyInvitation invitation;
				invitation.setSerial( leader->serial() );
				player->socket()->send( &invitation );

				// Attach a tempeffect that'll cancel the invitation after ten seconds
				Timers::instance()->insert( new cPartyCancelInvitation( leader->serial(), player->serial() ) );
			}
		}

		return true;
	}
};

void cParty::handlePacket( cUOSocket* socket, cUOPacket* packet )
{
	unsigned char subcommand = ( *packet )[5];
	QString message;
	P_PLAYER leader = 0;
	P_PLAYER player = socket->player();

	switch ( subcommand )
	{
		// Add a member to the party
	case 1:
		socket->clilocMessage( 1005454 );
		socket->attachTarget( new cPartyInvitation( player->serial() ) );
		break;

		// Remove member from party.
	case 2:
		if ( packet->size() == 10 )
		{
			P_PLAYER target = dynamic_cast<P_PLAYER>( World::instance()->findChar( packet->getInt( 6 ) ) );

			if ( target && player->party() && target->party() )
			{
				if ( player->party() == target->party() && player->party()->leader() == player )
				{
					socket->log( LOG_TRACE, QString( "Removed '%1' from the party.\n" ).arg( target->account()->login() ) );
					player->party()->removeMember( target );
				}
				else if ( target == player )
				{
					socket->log( LOG_TRACE, QString( "Left the party.\n" ).arg( player->account()->login() ) );
					player->party()->removeMember( target );
				}
			}
		}
		break;

		// Send a single party member a message
	case 3:
		if ( player->party() )
		{
			P_PLAYER target = dynamic_cast<P_PLAYER>( World::instance()->findChar( packet->getInt( 6 ) ) );
			if ( target )
			{
				socket->log( LOG_TRACE, QString( "Told '%1' in party '%2'.\n" ).arg( target->account()->login() ).arg( message ) );
				QString message = packet->getUnicodeString( 10, packet->size() - 10 );
				player->party()->send( player, target, message );
			}
		}
		break;

		// Send the whole party a message
	case 4:
		if ( player->party() )
		{
			QString message = packet->getUnicodeString( 6, packet->size() - 6 );
			socket->log( LOG_TRACE, QString( "Told the whole party: '%1'.\n" ).arg( message ) );
			player->party()->send( player, message );
		}
		break;

		// Allow or Disallow my party from looting my corpse
	case 6:
		if ( player->party() && packet->size() == 7 )
		{
			bool allowed = ( *packet )[6] != 0;
			player->party()->setLootingAllowed( player, allowed );
			socket->clilocMessage( allowed ? 1005447 : 1005448 );
		}
		break;

		// Accept party invitation
	case 8:
		Timers::instance()->dispel( player, 0, "cancelpartyinvitation", true, false );

		leader = dynamic_cast<P_PLAYER>( World::instance()->findChar( packet->getInt( 6 ) ) );
		if ( leader && leader->party() && leader->party()->canidates().contains( player ) )
		{
			leader->party()->addMember( player );
			socket->log( QString( "Accepted party invitation from '%1'.\n" ).arg( leader->account()->login() ) );
		}
		break;

		// Decline party invitation
	case 9:
		Timers::instance()->dispel( player, 0, "cancelpartyinvitation", true, false );

		leader = dynamic_cast<P_PLAYER>( World::instance()->findChar( packet->getInt( 6 ) ) );
		if ( leader && leader->party() && leader->party()->canidates().contains( player ) )
		{
			leader->socket()->clilocMessageAffix( 1008091, 0, player->name(), 0x3b2, 3, 0, false, true );
			leader->party()->removeMember( player );
			socket->clilocMessage( 1008092 );

			socket->log( LOG_TRACE, QString( "Declined party invitation from '%1'.\n" ).arg( leader->account()->login() ) );
		}
		break;

	default:
		message.sprintf( "Receieved unknown party subcommand: 0x%02x", subcommand );
		message += packet->dump( packet->uncompressed() ) + "\n";
		socket->log( LOG_WARNING, message );
		break;
	};
}

/*
	\object party
	\description This object type represents a party.
*/
struct wpParty
{
	PyObject_HEAD;
	cParty* party;
};

static PyObject* wpParty_getAttr( wpParty* self, char* name );
static int wpParty_setAttr( wpParty* self, char* name, PyObject* value );
static int wpParty_compare( PyObject* a, PyObject* b );

/*!
	The python type object for the party type.
*/
PyTypeObject wpPartyType =
{
	PyObject_HEAD_INIT( NULL )
	0, 
	"party", 
	sizeof( wpPartyType ), 
	0, 
	wpDealloc, 
	0, 
	( getattrfunc ) wpParty_getAttr, 
	( setattrfunc ) wpParty_setAttr, 
	wpParty_compare,
	0,
};

static int wpParty_compare( PyObject* a, PyObject* b )
{
	if ( a->ob_type != &wpPartyType || b->ob_type != &wpPartyType )
		return -1;

	return !( ( ( wpParty * ) a )->party == ( ( wpParty * ) b )->party );
}

/*
	\method party.tellsingle
	\param source A <object id="char">char</object> object for the source of the message.
	\param target A <object id="char">char</object> object for the target of the message.
	\param message A string containing the message to be sent.
	\description Send a single party member a partymessage from a given source.
*/
static PyObject* wpParty_tellsingle( wpParty* self, PyObject* args )
{
	P_CHAR source, target;
	char* message;

	if ( PyArg_ParseTuple( args, "O&O&es:party.tellsingle(source, target, message)", PyConvertChar, &source, PyConvertChar, &target, "utf-8", &message ) )
	{
		P_PLAYER psource = dynamic_cast<P_PLAYER>( source );
		P_PLAYER ptarget = dynamic_cast<P_PLAYER>( target );

		if ( psource && ptarget )
			self->party->send( psource, ptarget, QString::fromUtf8( message ) );

		PyMem_Free( message );

		Py_RETURN_NONE;
	}

	return 0;
}

/*
	\method party.tellsingle
	\param source A <object id="char">char</object> object for the source of the message.
	\param message A string containing the message to be sent.
	\description Send a message to all party members from a single source.
*/
static PyObject* wpParty_tellall( wpParty* self, PyObject* args )
{
	P_CHAR source;
	char* message;

	if ( PyArg_ParseTuple( args, "O&es:party.tellsingle(source, message)", PyConvertChar, &source, "utf-8", &message ) )
	{
		P_PLAYER psource = dynamic_cast<P_PLAYER>( source );

		if ( psource )
			self->party->send( psource, QString::fromUtf8( message ) );

		PyMem_Free( message );

		Py_RETURN_TRUE;
	}

	return 0;
}

static PyMethodDef wpPartyMethods[] =
{
	{ "tellall", ( getattrofunc ) wpParty_tellall, METH_VARARGS, 0 }, { "tellsingle", ( getattrofunc ) wpParty_tellsingle, METH_VARARGS, 0 }, { 0, 0, 0, 0 }
};

static PyObject* wpParty_getAttr( wpParty* self, char* name )
{
	cParty* party = self->party;

	/*
		\rproperty party.leader A <object id="char">char</object> object for the leader of this party.
	*/
	if ( !strcmp( name, "leader" ) )
	{
		return party->leader()->getPyObject();
	}
	/*
		\rproperty party.members A list of <object id="char">char</object> objects for the members in this party.
	*/
	else if ( !strcmp( name, "members" ) )
	{
		QPtrList<cPlayer> members = party->members();
		PyObject* list = PyList_New( 0 );
		for ( P_PLAYER member = members.first(); member; member = members.next() )
			PyList_Append( list, member->getPyObject() );
		return list;
	}
	/*
		\rproperty party.canidates A list of <object id="char">char</object> objects for the canidates in this party.
	*/
	else if ( !strcmp( name, "canidates" ) )
	{
		QPtrList<cPlayer> canidates = party->canidates();
		PyObject* list = PyList_New( 0 );
		for ( P_PLAYER canidate = canidates.first(); canidate; canidate = canidates.next() )
			PyList_Append( list, canidate->getPyObject() );
		return list;
	}
	/*
		\rproperty party.lootingallowed A list of <object id="char">char</object> objects for the members of this party who allowed looting their corpse.
	*/
	else if ( !strcmp( name, "lootingallowed" ) )
	{
		QPtrList<cPlayer> lootlist = party->lootingAllowed();
		PyObject* list = PyList_New( 0 );
		for ( P_PLAYER member = lootlist.first(); member; member = lootlist.next() )
			PyList_Append( list, member->getPyObject() );
		return list;
	}

	return Py_FindMethod( wpPartyMethods, ( PyObject * ) self, name );
}

static int wpParty_setAttr( wpParty* self, char* name, PyObject* value )
{
	Q_UNUSED(self);
	Q_UNUSED(name);
	Q_UNUSED(value);
	return 0;
}

const char* cParty::className() const
{
	return "party";
}

PyObject* cParty::getPyObject()
{
	wpParty* returnVal = PyObject_New( wpParty, &wpPartyType );
	returnVal->party = this;
	return ( PyObject * ) returnVal;
}

bool cParty::implements( const QString& name ) const
{
	if ( name == className() )
		return true;
	else
		return cPythonScriptable::implements( name );
}
