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
#ifndef __TARGETREQUEST_H__
#define __TARGETREQUEST_H__

#include "wolfpack.h"
#include "wptargetrequests.h"

class cSetPrivLvlTarget: public cTargetRequest
{
protected:
	QString plevel_;
public:
	cSetPrivLvlTarget( const QString &plevel ) { plevel_ = plevel; };

	virtual void responsed( cUOSocket *socket, cUORxTarget *target );
};

class cAddItemTarget: public cTargetRequest
{
protected:
	QString item_;
public:
	cAddItemTarget( const QString &item ) { item_ = item; };

	virtual void responsed( cUOSocket *socket, cUORxTarget *target );
};

class cAddNpcTarget: public cTargetRequest
{
protected:
	QString npc_;
public:
	cAddNpcTarget( const QString &npc ) { npc_ = npc; };

	virtual void responsed( cUOSocket *socket, cUORxTarget *target );
};

class cBuildMultiTarget: public cTargetRequest
{
protected:
	QString multisection_;
	SERIAL	deedserial_;
	SERIAL	senderserial_;
public:
	cBuildMultiTarget( const QString &multisection, const SERIAL &senderserial, const SERIAL &deedserial ) { multisection_ = multisection; senderserial_ = senderserial; deedserial_ = deedserial; };

	virtual void responsed( cUOSocket *socket, cUORxTarget *target );
};

#endif

