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

#if !defined(__TERRITORIES_H__)
#define __TERRITORIES_H__

#include "baseregion.h"
#include "singleton.h"

#include <map>
#include <qmap.h>
#include <qptrlist.h>

#include <qobject.h>

struct good_st
{
	UI32 sellable;
	UI32 buyable;
	UI32 rndmin;
	UI32 rndmax;
};

class cTerritory : public cBaseRegion
{
public:
	cTerritory( const cElement* Tag, cBaseRegion* parent );
	cTerritory();

	void init( void );

	// Getters
	QString name( void ) const
	{
		return name_;
	}
	QString midilist( void ) const
	{
		return midilist_;
	}
	bool isGuarded( void ) const
	{
		return flags_ & 0x0001;
	}
	bool isNoMark( void ) const
	{
		return flags_ & 0x0002;
	}
	bool isNoGate( void ) const
	{
		return flags_ & 0x0004;
	}
	bool isNoRecallOut( void ) const
	{
		return flags_ & 0x0008;
	}
	bool isNoRecallIn( void ) const
	{
		return flags_ & 0x0010;
	}
	bool isRecallShield( void ) const
	{
		return flags_ & 0x0020;
	}
	bool isNoAgressiveMagic( void ) const
	{
		return flags_ & 0x0040;
	}
	bool isAntiMagic( void ) const
	{
		return flags_ & 0x0080;
	}
	bool isValidEscortRegion( void ) const
	{
		return flags_ & 0x0100;
	}
	bool isCave( void ) const
	{
		return flags_ & 0x0200;
	}
	bool isNoMusic( void ) const
	{
		return flags_ & 0x0400;
	}
	QString guardOwner( void ) const
	{
		return guardowner_;
	}
	UI08 snowChance( void ) const
	{
		return snowchance_;
	}
	UI08 rainChance( void ) const
	{
		return rainchance_;
	}
	bool haveTeleporters() const;
	bool findTeleporterSpot( Coord_cl& ) const;

	QString getGuardSect( void ) const;
private:
	// Setters to ease up the flag meanings
	void setGuarded( bool data )
	{
		if ( data )
			flags_ |= 0x0001;
		else
			flags_ &= ~0x0001;
	}
	void setNoMark( bool data )
	{
		if ( data )
			flags_ |= 0x0002;
		else
			flags_ &= ~0x0002;
	}
	void setNoGate( bool data )
	{
		if ( data )
			flags_ |= 0x0004;
		else
			flags_ &= ~0x0004;
	}
	void setNoRecallOut( bool data )
	{
		if ( data )
			flags_ |= 0x0008;
		else
			flags_ &= ~0x0008;
	}
	void setNoRecallIn( bool data )
	{
		if ( data )
			flags_ |= 0x0010;
		else
			flags_ &= ~0x0010;
	}
	void setRecallShield( bool data )
	{
		if ( data )
			flags_ |= 0x0020;
		else
			flags_ &= ~0x0020;
	}
	void setNoAgressiveMagic( bool data )
	{
		if ( data )
			flags_ |= 0x0040;
		else
			flags_ &= ~0x0040;
	}
	void setAntiMagic( bool data )
	{
		if ( data )
			flags_ |= 0x0080;
		else
			flags_ &= ~0x0080;
	}
	void setValidEscortRegion( bool data )
	{
		if ( data )
			flags_ |= 0x0100;
		else
			flags_ &= ~0x0100;
	}
	void setCave( bool data )
	{
		if ( data )
			flags_ |= 0x0200;
		else
			flags_ &= ~0x0200;
	}
	void setNoMusic( bool data )
	{
		if ( data )
			flags_ |= 0x0400;
		else
			flags_ &= ~0x0400;
	}

	virtual void processNode( const cElement* Tag );

private:
	QString midilist_; // midilist to play

	UINT16 flags_; // flags like guarded, mark allowed, etc. (see getters)

	QString guardowner_;
	UI08 snowchance_;
	UI08 rainchance_;

	QStringList guardSections_;

	struct teleporters_st
	{
		Coord_cl source;
		Coord_cl destination;
	};

	QValueList<teleporters_st> teleporters;
public:
	std::map<UI32, good_st> tradesystem_;
};

class cTerritories : public cComponent
{
private:
	QMap<uint, QPtrList<cTerritory> > topregions;
public:
	void reload();
	void load();
	void unload();
	void check( P_CHAR pc );

	cTerritory* region( const QString& regName );
	cTerritory* region( UI16 posx, UI16 posy, UI08 map );

	inline cTerritory* region( const Coord_cl& pos )
	{
		return region( pos.x, pos.y, pos.map );
	}
};

typedef SingletonHolder<cTerritories> Territories;

#endif
