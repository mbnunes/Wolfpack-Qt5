//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
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
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

#ifndef __RESOURCES_H
#define __RESOURCES_H

#include "definable.h"
#include "iserialization.h"
#include "items.h"
#include "wptargetrequests.h"

// Library includes
#include "qvaluevector.h"
#include "qstring.h"
#include "qstringlist.h"

class cResource : public cDefinable
{
public:
	cResource( const QDomElement &Tag );
	~cResource();

	struct resourcespec_st
	{
		QString name;
		UINT32 minamount_per_attempt;
		UINT32 maxamount_per_attempt;
		QValueVector< UINT16 > ids;
		QValueVector< UINT16 > colors;
		QValueVector< UINT16 > artids;
		QValueVector< UINT16 > mapids;
		UINT32 vein_minamount;
		UINT32 vein_maxamount;
		UINT32 vein_quota;
		UINT16 minskill;
		UINT16 maxskill;
		UINT32 quota;
	};

	// implements cDefinable
	virtual void processNode( const QDomElement &Tag );

	// Getters
	bool	deleteSource()	const { return deletesource_; }
	QValueVector< resourcespec_st > resourceSpecs() const { return resourcespecs_; }
	QString	name()	const { return name_; }

	bool	hasArtId( UINT16 id );
	bool	hasMapId( UINT16 id );

	void	handleTarget( cUOSocket* socket, Coord_cl pos, UINT16 mapid, UINT16 artid );

private:
	UINT32	amountmin_;
	UINT32	amountmax_;
	UINT32	veinchance_;
	UINT8	skillid_;
	UINT32	totalquota_;
	UINT32	totalveinquota_;
	UINT32	refreshtime_;
	QString section_;
	QString	name_;
	bool	deletesource_;
	UINT32	staminamin_;
	UINT32	staminamax_;
	UINT8	charaction_;
	UINT16	sound_;

	QValueVector< resourcespec_st > resourcespecs_;
	QValueVector< UINT16 > mapids_;
	QValueVector< UINT16 > artids_;
};

class cResourceItem : public cItem
{
public:
	cResourceItem( QString resource, UINT32 amount, UINT32 vein );
	~cResourceItem();

	// implements cSerializable
	virtual void Serialize( ISerialization &archive );
	virtual QString objectID( void ) const { return "RESOURCEITEM"; }

	// Getters
	QString	resource()	const	{ return resource_; }

private:
	QString		resource_;
};

class cAllResources : public std::map< QString, cResource* >
{
public:
	cAllResources();
	~cAllResources();

	static cAllResources *getInstance( void ) { return &instance; }

	void load();
	void unload();
	void reload();

private:
	static cAllResources instance;
};

class cFindResource : public cTargetRequest
{
public:
	cFindResource( QString resname ) : resourcename_( resname ) {}
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target );

private:
	QString resourcename_;
};

#endif

