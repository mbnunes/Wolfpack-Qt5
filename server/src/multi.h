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

#if !defined(__MULTI_H__)
#define __MULTI_H__

#include "uobject.h"
#include "items.h"
#include <qstring.h>
#include <qptrlist.h>

class cMulti : public cItem
{
private:
	static unsigned char classid;

protected:
	// Objects in the Multi.
	// Items that are in the Multi
	// are removed along with it.
	QPtrList<cUObject> objects;

public:
	unsigned char getClassid() {
		return cMulti::classid;
	}

	const char* objectID() const
	{
		return "cMulti";
	}

	cMulti();
	virtual ~cMulti();

	/*static void buildSqlString(QStringList &fields, QStringList &tables, QStringList &conditions);
	void load( char **, UINT16& );
	void save();
	bool del();*/
	void save(cBufferedWriter &writer);

	// Find at certain position
	static cMulti* find( const Coord_cl& pos );

	// Register in load factory
	static void registerInFactory();

	// Property Interface Methods
	stError* setProperty( const QString& name, const cVariant& value );
	PyObject *getProperty(const QString& name);

	// Python Interface Methods
	PyObject* getPyObject();
	const char* className() const;

	// Object List
	void addObject( cUObject* object );
	void removeObject( cUObject* object );
	inline const QPtrList<cUObject>& getObjects()
	{
		return objects;
	}

	// Remove Handler
	void remove();

	// Move all contained items along.
	// Also recheck our position in the multi grid.
	void moveTo( const Coord_cl&, bool noRemove = false );

	// Create a multi from a script definition
	static cMulti* createFromScript( const QString& section );

	// See if a certain coordinate is within the multi
	bool inMulti( const Coord_cl& pos );
};

#endif
