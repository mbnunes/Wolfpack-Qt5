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

#if !defined(__PARTY_H__)
#define __PARTY_H__

#include "pythonscript.h"
#include "typedefs.h"
#include <qptrlist.h>
#include <qstring.h>

/*!
	\brief This class encapsulates a party.
*/
class cParty : public cPythonScriptable 
{
protected:
	P_PLAYER leader_;
	QPtrList<cPlayer> members_;
	QPtrList<cPlayer> canidates_;
	QPtrList<cPlayer> lootingAllowed_;

public:
	/*!
		\brief Create a new party.
	*/
	cParty(P_PLAYER leader);

	/*!
		\brief Disband the party.
	*/
	~cParty();

	/*!
		\brief Add the \a player to this party and remove him from his old.
	*/
	void addMember(P_PLAYER player, bool update = true);

	/*!
		\brief Remove the \a player from this party.
	*/
	void removeMember(P_PLAYER player, bool update = true);

	/*!
		\brief Adds a player to the list of canidates for this party.
	*/
	void addCanidate(P_PLAYER player);

	/*!
		\brief Removes a player from the list of canidates for this party.
	*/
	void removeCanidate(P_PLAYER player);

	/*!
		\returns The leader of this party.
	*/
	inline P_PLAYER leader() const {
		return leader_;
	}

	/*!
		\returns The members of this party.
	*/
	inline QPtrList<cPlayer> members() const {
		return members_;
	}

	/*!
		\returns The canidates for this party.
	*/
	inline QPtrList<cPlayer> canidates() const {
		return canidates_;
	}

	/*!
		\returns A list of players whose corpses may be looted.
	*/
	inline QPtrList<cPlayer> lootingAllowed() const {
		return lootingAllowed_;
	}

	/*!
		\brief Allows or disallows looting the corpse of a member of this party.
		\param player The affected player.
		\param allowed Whether looting the corpse is allowed or not.
	*/
	void setLootingAllowed(P_PLAYER player, bool allowed);
    
	/*!
		\brief Sends an updated list of party members to all members.
	*/
	void update();

	/*!
		\brief Sends a packet to all party members.
	*/
	void send(cUOPacket *packet);

	/*!
		\brief Sends a localized message to all party members.
	*/
	void send(unsigned int message, const QString &params = QString::null, const QString &affix = QString::null, bool prepend = false);

	/*!
		\brief Sends a public message to the party.
		\param from The source of the message.
		\param message The text of the message.
	*/
	void send(P_PLAYER from, const QString &message);

	/*!
		\brief Sends a private message to a party member.
		\param from The source of this message.
		\param to The target of this message.
		\param message The text of this message.
	*/
	void send(P_PLAYER from, P_PLAYER target, const QString &message);

	/*!
		\brief Processes a party packet sent by the specified player.
		\param socket The socket this packet came from.
		\param packet The packet that was sent.
	*/
	static void handlePacket(cUOSocket *socket, cUOPacket *packet);

	// Inherited methods from cPythonScriptable
	const char *className() const;
	PyObject *getPyObject();
	bool implements(const QString &name) const;
};

#endif
