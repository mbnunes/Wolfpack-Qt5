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

#if !defined(__GUILDS_H__)
#define __GUILDS_H__

#include "player.h"
#include "pythonscript.h"
#include "singleton.h"
#include <qmap.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qdatetime.h>

/*!
	\brief This class represents a guild and all associated information.
*/
class cGuild : public cPythonScriptable
{
public:
	enum eAlignment
	{
		Neutral		= 0,
		Evil,
		Good
	};

	/*!
		\brief This class stores additional information about members of this guild.
	*/
	class MemberInfo
	{
	private:
		bool showSign_;
		QString guildTitle_;
		QDateTime joined_;

	public:
		MemberInfo()
		{
			showSign_ = true;
			guildTitle_ = QString::null;
			joined_ = QDateTime::currentDateTime();
		}

		/*!
				\returns A timestamp indicating when the member joined the guild.
			*/
		unsigned int joined()
		{
			return joined_.toTime_t();
		}

		/*!
				\brief Specifies when the user joined the guild.
				\param time A Unix timestamp.
			*/
		void setJoined( unsigned int time )
		{
			joined_.setTime_t( time );
		}

		/*!
				\returns Whether the guild sign should be shown or not.
			*/
		inline bool showSign()
		{
			return showSign_;
		}

		/*!
				\brief Should the guildsign be shown?
				\param data The new state of this flag.
			*/
		inline void setShowSign( bool data )
		{
			showSign_ = data;
		}

		/*!
		\returns A custom guild title for this member. Otherwise QString::null.
		*/
		inline const QString& guildTitle()
		{
			return guildTitle_;
		}

		/*!
				\brief Sets a custom title for this guild member.
				\param data The custom guild title.
			*/
		inline void setGuildTitle( const QString& data )
		{
			guildTitle_ = data;
		}
	};
private:
	unsigned int serial_;
	QString name_;
	QString abbreviation_;
	QString charta_;
	QString website_;
	eAlignment alignment_;
	QDateTime founded_;
	P_ITEM guildstone_;

	P_PLAYER leader_;
	QPtrList<cPlayer> members_;
	QPtrList<cPlayer> canidates_;
	QMap<P_PLAYER, MemberInfo*> memberinfo_;
public:
	/*!
		\brief Creates a new guild.
	*/
	cGuild( bool createSerial = true );

	/*!
		\brief Clean up things upon destruction.
	*/
	~cGuild();

	/*!
		\brief Save this guild to a given database connection.
		\param connection The database connection.
	*/
	void save();

	/*!
		\brief Load this guild from a given result set which was obtained
			from the given database connection.
		\param connection The database connection.
		\param result The resultset for this guild.
	*/
	void load( const cDBResult& result );

	/*!
		\returns The guildstone for this guild. May be NULL.
	*/
	inline P_ITEM guildstone()
	{
		return guildstone_;
	}

	/*!
		\brief Sets the new guildstone for this guild.
		\param guildstone The new guildstone to set.
	*/
	inline void setGuildstone( P_ITEM guildstone )
	{
		guildstone_ = guildstone;
	}

	/*!
		\returns The serial of this guild.
	*/
	inline unsigned int serial()
	{
		return serial_;
	}

	/*!
		\brief Sets the serial of this guild.
	*/
	void setSerial( unsigned int data );

	/*!
		\returns The website for this guild.
	*/
	inline const QString& website()
	{
		return website_;
	}

	/*!
		\brief Sets a new website for this guild.
		\param site The new website.
	*/
	inline void setWebsite( const QString& site )
	{
		website_ = site;
	}

	/*!
		\returns The name of this guild.
	*/
	inline const QString& name()
	{
		return name_;
	}

	/*!
		\returns The abbreviation for this guilds name.
	*/
	inline const QString& abbreviation()
	{
		return abbreviation_;
	}

	/*!
		\returns The date &amp; time when this guild was founded.
	*/
	inline const QDateTime& founded()
	{
		return founded_;
	}

	/*!
		\brief Sets the date &amp; time when this guild was founded.
		\param founded The new founding date and time.
	*/
	inline void setFounded( const QDateTime& founded )
	{
		founded_ = founded;
	}

	/*!
		\brief Changes the name of this guild.
	*/
	inline void setName( const QString& data )
	{
		name_ = data;
	}

	/*!
		\brief Changes the abbreviation for this guild.
	*/
	inline void setAbbreviation( const QString& data )
	{
		abbreviation_ = data;
	}

	/*!
		\returns The charta of this guild.
	*/
	inline const QString& charta()
	{
		return charta_;
	}

	/*!
		\brief Changes the charta for this guild.
	*/
	inline void setCharta( const QString& data )
	{
		charta_ = data;
	}

	/*!
		\returns The alignment for this guild.
	*/
	inline eAlignment alignment()
	{
		return alignment_;
	}

	/*!
		\brief Sets the alignment for this guild.
	*/
	inline void setAlignment( eAlignment data )
	{
		alignment_ = data;
	}

	/*!
		\returns The leader of this guild.
	*/
	inline P_PLAYER leader()
	{
		return leader_;
	}

	/*!
		\brief Sets a new leader for this guild.
	*/
	inline void setLeader( P_PLAYER player )
	{
		leader_ = player;
	}

	/*!
		\returns A QPtrList containing all members of this guild.
	*/
	inline QPtrList<cPlayer>& members()
	{
		return members_;
	}

	/*!
		\returns A QPtrList of canidates for this guild.
	*/
	inline QPtrList<cPlayer>& canidates()
	{
		return canidates_;
	}

	/*!
		\brief Adds a member to this guild.
		\param member The member you want to add to this guild.
	*/
	void addMember( P_PLAYER member, MemberInfo* info = 0 );

	/*!
		\brief Removes a member from this guild.
		\param member The member you want to remove from this guild.
	*/
	void removeMember( P_PLAYER member );

	/*!
		\brief Adds a canidate to this guild.
		\param member The canidate you want to add to this guild.
	*/
	void addCanidate( P_PLAYER canidate );

	/*!
		\brief Removes a canidate from this guild.
		\param member The member you want to remove from this guild.
	*/
	void removeCanidate( P_PLAYER canidate );

	/*!
		\param The member you want to retrieve the information about.
		\returns The additional member information for the specified member.
	*/
	inline MemberInfo* getMemberInfo( P_PLAYER player )
	{
		if ( !memberinfo_.contains( player ) )
		{
			return 0;
		}
		else
		{
			return memberinfo_.find( player ).data();
		}
	}

	// Methods inherited from cPythonScriptable
	const char* className() const;
	PyObject* getPyObject();
	bool implements( const QString& name ) const;
};

/*!
	\brief This class manages all guilds.
*/
class cGuilds
{
private:
	QMap<unsigned int, cGuild*> guilds;

public:
	typedef QMap<unsigned int, cGuild*>::iterator iterator;

	/*!
		\brief Destructor. Delets all remaining guilds.
	*/
	~cGuilds();

	/*!
		\returns An iterator to the beginning of the internal guild mapping.
	*/
	inline iterator begin()
	{
		return guilds.begin();
	}

	/*!
		\returns An iterator to the end of the internal guild mapping.
	*/
	inline iterator end()
	{
		return guilds.end();
	}

	/*!
		\returns A free serial for a new guild.
	*/
	unsigned int findFreeSerial();

	/*!
		\brief Registers a guild with this manager.
		\param guild The guild to register.
	*/
	void registerGuild( cGuild* guild );

	/*!
		\brief Unregisters a guild in this manager.
		\param guild The guild to unregister.
	*/
	void unregisterGuild( cGuild* guild );

	/*!
		\brief Finds a guild by it's serial in this manager.
		\param serial The serial to search for.
		\returns A guild object or if no guild is found a null pointer.
	*/
	cGuild* findGuild( unsigned int serial );

	/*!
		\brief Loads the guilds from a given database connection.
		\param connection A database connection.
	*/
	void load();

	/*!
		\brief Saves the guilds to a given database connection.
		\param connection A database connection.
	*/
	void save();
};

/*!
	\brief SingletonHolder type instance for the guild manager.
*/
typedef SingletonHolder<cGuilds> Guilds;

#endif
