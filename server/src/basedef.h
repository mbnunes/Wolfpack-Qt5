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

#if !defined( __BASEDEF_H__ )
#define __BASEDEF_H__

#include "definable.h"
#include "singleton.h"
#include <qmap.h>
#include <qptrlist.h>
#include <qcstring.h>
#include <qstring.h>

class cElement;
class cPythonScript;

#undef getIntProperty
#undef getStrProperty

class cBaseDef : public cDefinable {
protected:
	// Our id
	QCString id_;

	QMap<QString, unsigned int> intproperties;
	QMap<QString, QString> properties;
	
	bool loaded;
	virtual void reset();
public:
	void processNode( const cElement* node );

	inline unsigned int getIntProperty(const QString &name, unsigned int def = 0) {
		QMap<QString, unsigned int>::const_iterator it = intproperties.find(name);
		if (it == intproperties.end()) {
			return def;
		} else {			
			return *it;
		}
	}

	inline bool hasIntProperty(const QString &name) {
		return intproperties.contains(name);
	}

	inline const QString &getStrProperty(const QString &name, const QString &def = QString::null) {
		QMap<QString, QString>::const_iterator it = properties.find(name);
		if (it == properties.end()) {
			return def;
		} else {			
			return *it;
		}
	}

	inline bool hasStrProperty(const QString &name) {
		return properties.contains(name);
	}

	inline const QCString& id() const
	{
		return id_;
	}
};

class cCharBaseDef : public cBaseDef
{
	friend class cCharBaseDefs;
protected:
	// Sounds
	unsigned short basesound_;
	unsigned char soundmode_;
	unsigned int flags_;
	unsigned char type_;
	unsigned short figurine_;
	unsigned short minDamage_;
	unsigned short maxDamage_;
	short minTaming_;
	QCString carve_;
	QCString lootPacks_;
	QCString bindmenu_;
	unsigned char controlSlots_;
	unsigned char criticalHealth_;
	QPtrList<cPythonScript> baseScripts_;
	QCString baseScriptList_;

	// Misc Properties
	void load();
	void reset();
	void refreshScripts();
public:
	cCharBaseDef( const QCString& id );
	~cCharBaseDef();

	void processNode( const cElement* node );	

	inline const QCString& baseScriptList()
	{
		load();
		return baseScriptList_;
	}

	inline const QPtrList<cPythonScript> &baseScripts()
	{
		load();
		return baseScripts_;
	}

	inline unsigned char controlSlots()
	{
		load();
		return controlSlots_;
	}

	inline unsigned char criticalHealth()
	{
		load();
		return criticalHealth_;
	}

	inline unsigned short basesound()
	{
		load();
		return basesound_;
	}

	inline unsigned char soundmode()
	{
		load();
		return soundmode_;
	}

	inline unsigned short figurine()
	{
		load();
		return figurine_;
	}

	inline unsigned int flags()
	{
		load();
		return flags_;
	}

	inline bool canFly()
	{
		load();
		return ( flags_ & 0x01 ) != 0;
	}

	inline bool antiBlink()
	{
		load();
		return ( flags_ & 0x02 ) != 0;
	}

	inline bool noCorpse()
	{
		load();
		return ( flags_ & 0x04 ) != 0;
	}

	inline unsigned short maxDamage()
	{
		load();
		return maxDamage_;
	}

	inline unsigned short minDamage()
	{
		load();
		return minDamage_;
	}

	inline short minTaming()
	{
		load();
		return minTaming_;
	}

	inline const QCString& carve()
	{
		load();
		return carve_;
	}

	inline const QCString& lootPacks()
	{
		load();
		return lootPacks_;
	}

	inline const QCString& bindmenu()
	{
		load();
		return bindmenu_;
	}
};

class cCharBaseDefs
{
protected:
	typedef QMap<QCString, cCharBaseDef*> Container;
	typedef Container::iterator Iterator;
	Container definitions;
public:
	cCharBaseDefs();
	~cCharBaseDefs();

	// Get a base definition
	// This is guaranteed to return a basedef. Even if uninitialized.
	cCharBaseDef* get( const QCString& id );

	// When reset is called, all loaded basedefs are unflagged.
	void reset();
	void refreshScripts();
};

typedef SingletonHolder<cCharBaseDefs> CharBaseDefs;

/*
	This class represents an item definition and it's
	static properties.
*/
class cItemBaseDef : public cBaseDef
{
	friend class cItemBaseDefs;
protected:
	float weight_;
	unsigned int sellprice_;
	unsigned int buyprice_;
	unsigned short type_;
	QCString bindmenu_;
	unsigned char lightsource_;
	unsigned int decaydelay_;
	unsigned int flags_;
	QPtrList<cPythonScript> baseScripts_;
	QCString baseScriptList_;

	// Misc Properties
	void load();
	void reset();
	void refreshScripts();

	inline void setWaterSource( bool data )
	{
		if ( data )
		{
			flags_ |= 0x01;
		}
		else
		{
			flags_ &= ~0x01;
		}
	}
public:
	cItemBaseDef( const QCString& id );
	~cItemBaseDef();

	void processNode( const cElement* node );

	inline const QCString& baseScriptList()
	{
		load();
		return baseScriptList_;
	}

	inline const QPtrList<cPythonScript> & baseScripts()
	{
		load();
		return baseScripts_;
	}

	inline unsigned int decaydelay()
	{
		load();
		return decaydelay_;
	}

	inline float weight()
	{
		load();
		return weight_;
	}

	inline unsigned int sellprice()
	{
		load();
		return sellprice_;
	}

	inline unsigned int buyprice()
	{
		load();
		return buyprice_;
	}

	inline unsigned short type()
	{
		load();
		return type_;
	}

	inline const QCString& bindmenu()
	{
		load();
		return bindmenu_;
	}

	inline unsigned char lightsource()
	{
		load();
		return lightsource_;
	}

	inline bool isWaterSource()
	{
		load();
		return ( flags_ & 0x01 ) != 0;
	}
};

class cItemBaseDefs
{
protected:
	typedef QMap<QCString, cItemBaseDef*> Container;
	typedef Container::iterator Iterator;
	Container definitions;
public:
	cItemBaseDefs();
	~cItemBaseDefs();

	// Get a base definition
	// This is guaranteed to return a basedef. Even if uninitialized.
	cItemBaseDef* get( const QCString& id );

	// When reset is called, all loaded basedefs are unflagged.
	void reset();
	void refreshScripts();
};

typedef SingletonHolder<cItemBaseDefs> ItemBaseDefs;

#endif
