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

#if !defined( __BASEDEF_H__ )
#define __BASEDEF_H__

#include "singleton.h"

/*
	Class which contains information about characters that never changes.
 */
class cCharBaseDef
{
friend class cBaseDefManager;

protected:
	// Sounds
	unsigned short basesound_;
	unsigned char soundmode_;
	unsigned short shrinked_;
	unsigned int flags_;
	unsigned char type_;

public:
	cCharBaseDef() {}
	virtual ~cCharBaseDef() {}

	// Getters ONLY
	// These are READONLY PROPERTIES!
	unsigned short basesound() const { return basesound_; }
	unsigned char soundmode() const { return soundmode_; }
	unsigned short shrinked() const { return shrinked_; }
	unsigned int flags() const { return flags_; }
	unsigned char type() const { return type_; }

	inline bool canFly() const {
		return (flags_ & 0x01) != 0;
	}

	inline bool antiBlink() const {
		return (flags_ & 0x02) != 0;
	}

	inline bool noCorpse() const {
		return (flags_ & 0x04) != 0;
	}
};

class cBaseDefManager
{
protected:
	cCharBaseDef *chardefs[0x400]; // 0x400 entries in anim.idx

public:
	cBaseDefManager();
	virtual ~cBaseDefManager();

	cCharBaseDef *getCharBaseDef( unsigned short id );

	void load();
	
	void reload()
	{
		unload();
		load();
	}

	void unload();
};

typedef SingletonHolder< cBaseDefManager > BaseDefManager;

#endif
