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

// Wolfpack Includes
#include "targetrequests.h"
#include "muls/maps.h"
#include "sectors.h"
#include "definitions.h"
#include "territories.h"
#include "items.h"
//#include "muls/tilecache.h"
#include "serverconfig.h"
#include "skills.h"
#include "combat.h"
#include "scriptmanager.h"
#include "pythonscript.h"
#include "accounts.h"
#include "npc.h"
#include "basics.h"
#include "network/network.h"

// System Includes
#include <math.h>

bool cSetTarget::responsed( cUOSocket* socket, cUORxTarget* target )
{
	P_CHAR pChar = FindCharBySerial( target->serial() );
	P_ITEM pItem = FindItemBySerial( target->serial() );

	cUObject* pObject = NULL;

	if ( pItem )
		pObject = pItem;
	else if ( pChar )
		pObject = pChar;

	// Only characters and items
	if ( !pObject )
	{
		socket->sysMessage( tr( "Please select a valid character or item" ) );
		return true;
	}

	// check for rank
	if ( pChar && pChar->objectType() == enPlayer )
	{
		P_PLAYER pp = dynamic_cast<P_PLAYER>( pChar );
		if ( pp->account()->rank() >= socket->player()->account()->rank() && pp != socket->player() )
		{
			socket->sysMessage( tr( "Better do not try that!" ) );
			return true;
		}
	}

	cVariant value( this->value );
	stError* error = pObject->setProperty( key, value );

	if ( error )
	{
		socket->sysMessage( error->text );
		delete error;
	}

	if ( pChar )
		pChar->resend();
	else if ( pItem )
		pItem->update();

	return true;
}

bool cRemoveTarget::responsed( cUOSocket* socket, cUORxTarget* target )
{
	P_CHAR pChar = FindCharBySerial( target->serial() );
	P_ITEM pItem = FindItemBySerial( target->serial() );

	if ( pChar )
	{
		pChar->remove();
	}
	else if ( pItem )
	{
		pItem->remove();
	}
	else
		socket->sysMessage( tr("You need to select either an item or a character") );
	return true;
}

bool cShowTarget::responsed( cUOSocket* socket, cUORxTarget* target )
{
	// Check for a valid target
	cUObject* pObject;
	P_ITEM pItem = FindItemBySerial( target->serial() );
	P_CHAR pChar = FindCharBySerial( target->serial() );
	if ( !pChar && !pItem )
	{
		socket->sysMessage( tr( "You have to target a valid object." ) );
		return true;
	}

	if ( pChar )
		pObject = pChar;
	else
		pObject = pItem;

	PyObject *result = pObject->getProperty( key );

	if ( !result )
	{
		socket->sysMessage( tr( "Unknown property: '%1'" ).arg( key ) );
	}
	else
	{
		if (PyUnicode_Check(result) || PyString_Check(result)) {
			socket->sysMessage( tr( "'%1' is '%2'" ).arg( key ).arg( Python2QString(result) ) );
		} else {
			PyObject *repr = PyObject_Str(result);
			QString value = Python2QString(repr);
			socket->sysMessage( tr( "'%1' is '%2'" ).arg( key ).arg( value ) );
			Py_XDECREF(repr);
		}

		Py_DECREF( result );
	}
	return true;
}
