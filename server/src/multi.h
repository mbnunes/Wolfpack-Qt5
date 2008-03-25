/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2007 by holders identified in AUTHORS.txt
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

#if !defined(__MULTI_H__)
#define __MULTI_H__

#include "uobject.h"
#include "items.h"
#include <QString>
#include <QList>

class cMulti : public cItem
{
private:
	static unsigned char classid;
	unsigned char changed_ : 1;

	void flagChanged()
	{
		changed_ = true;
	} // easier to debug, compiler should make it inline;

protected:
	// Objects in the Multi.
	// Items that are in the Multi
	// are removed along with it.
	QList<cUObject*> objects;
	
	// To read the rectangle tag.
	virtual void processNode( const cElement* Tag, uint hash = 0 );
	// To load data from MultiArea tag.
	virtual void postload( unsigned int version );
	// Multi size and height
	short x1_;
	short y1_;
	short x2_;
	short y2_;
	
	ushort h_;

public:
	static void setClassid( unsigned char id )
	{
		cMulti::classid = id;
	}

	unsigned char getClassid()
	{
		return cMulti::classid;
	}

	const char* objectID() const
	{
		return "cMulti";
	}

	cMulti();
	virtual ~cMulti();

	// This static function can be used to check if the given multi can be placed at the given position
	static bool canPlace( const Coord& pos, unsigned short multiid, QList<cUObject*>& moveOut, unsigned short yard = 5 );
	static bool canPlaceBoat( const Coord& pos, unsigned short multiid, QList<cUObject*>& moveOut );

	static void buildSqlString( const char* objectid, QStringList& fields, QStringList& tables, QStringList& conditions );
	/*
	void load( char **, quint16& );
	void save();
	bool del();*/
	void save( cBufferedWriter& writer );

	// Find at certain position
	static cMulti* find( const Coord& pos );

	// Property Interface Methods
	stError* setProperty( const QString& name, const cVariant& value );
	PyObject* getProperty( const QString& name, uint hash = 0 );

	// Python Interface Methods
	PyObject* getPyObject();
	const char* className() const;

	// Object List
	void addObject( cUObject* object );
	void removeObject( cUObject* object );
	inline const QList<cUObject*>& getObjects()
	{
		return objects;
	}

	// Remove Handler
	void remove();

	// Move all contained items along.
	// Also recheck our position in the multi grid.
	void moveTo( const Coord& );

	// Create a multi from a script definition
	static cMulti* createFromScript( const QString& section );

	// See if a certain coordinate is within the multi
	bool inMulti( const Coord& pos );

	// Boat Specifics
	bool canBoatMoveTo( const Coord& pos );

	// Multi size and height
	void setX1(short value)
	{
		this->x1_ = value;
	} 
	void setY1(short value)
	{
		this->y1_ = value;
	} 
	void setX2(short value)
	{
		this->x2_ = value;
	} 
	void setY2(short value)
	{
		this->y2_ = value;
	} 
	void setHeight(ushort value)
	{
		this->h_ = value;
	}

	short getX1() const
	{
		return x1_;
	} 
	short getY1() const
	{
		return y1_;
	} 
	short getX2() const
	{
		return x2_;
	} 
	short getY2() const
	{
		return y2_;
	} 
	ushort getHeight() const
	{
		return h_;
	} 
	
};

#endif
