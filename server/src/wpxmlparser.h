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

#include "qdom.h"
#include "qmap.h"
#include "qstring.h"
#include "qstringlist.h"
#include "wpdefmanager.h"
#include "items.h"
#include "chars.h"

class IDefReader

class cDefinable
{
public:
	virtual ~cDefinable() {;}

	virtual void		applyNode( IDefReader &defreader );
	virtual std::string objectID( void ) = 0;
};

class IDefReader
{
public:
	IDefReader();
	virtual ~IDefReader() {};

	IDefReader( WPDEF_TYPE baseType );
	IDefReader( WPDEF_TYPE baseType, QDomElement* baseTag );

	QDomElement*		baseTag( void )						{ return basetag;	 };
	void				setBaseTag( QDomElement* newTag )	{ basetag = newTag;	 };
	WPDEF_TYPE			baseType( void )					{ return basetype;	 };
	void				setBaseType( WPDEF_TYPE newType )	{ basetype = newType;};

	bool				prepareParsing( QString Section );

	QString				processNode( QDomElement &Node );
	void				processItemContainerNode( P_ITEM contItem, QDomElement &Node );
	void				processScriptItemNode( P_ITEM madeItem, QDomElement &Node );

private:
	QDomElement*		basetag;
	WPDEF_TYPE			basetype;
};
