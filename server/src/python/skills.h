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

class cBaseChar;

#include "../defines.h"
#include "../skills.h"

#include "../network/uosocket.h"
#include "../typedefs.h"
#include "../basechar.h"
#include "../player.h"
#include "utilities.h"

typedef struct
{
	PyObject_HEAD;
	P_CHAR pChar;
	Q_UINT8 type; // 0: Value; 1: Cap; 2: Lock
} wpSkills;

int wpSkills_length( wpSkills* self )
{
	Q_UNUSED( self );
	return ALLSKILLS;
}

PyObject* wpSkills_get( wpSkills* self, int skill )
{
	if ( !self->pChar || self->pChar->free || skill >= ALLSKILLS )
		return PyInt_FromLong( -1 );

	if ( self->type == 0 )
		return PyInt_FromLong( self->pChar->skillValue( skill ) );
	else if ( self->type == 1 )
		return PyInt_FromLong( self->pChar->skillCap( skill ) );
	else
		return PyInt_FromLong( self->pChar->skillLock( skill ) );
}

PyObject* wpSkills_set( wpSkills* self, int skill, PyObject* pValue )
{
	if ( !self->pChar || self->pChar->free || skill >= ALLSKILLS || !PyInt_Check( pValue ) )
	{
		PyErr_BadArgument();
		return 0;
	}

	Q_UINT16 value = PyInt_AsLong( pValue );

	if ( self->type == 0 )
		self->pChar->setSkillValue( skill, value );
	else if ( self->type == 1 )
		self->pChar->setSkillCap( skill, value );
	else if ( self->type == 2 )
		self->pChar->setSkillLock( skill, value );

	P_PLAYER pPlayer = dynamic_cast<P_PLAYER>( self->pChar );

	if ( pPlayer && pPlayer->socket() )
	{
		pPlayer->socket()->sendSkill( skill );
	}

	return 0;
}

static PySequenceMethods wpSkillsSequence =
{
( inquiry ) wpSkills_length,
0,
0,
( intargfunc ) wpSkills_get,
0,
( intobjargproc ) wpSkills_set,
0,
0,
0,
0,
};

static PyTypeObject wpSkillsType =
{
PyObject_HEAD_INIT( NULL )
0,
"wpSkills",
sizeof( wpSkillsType ),
0,
wpDealloc,
0,
0,
0,
0,
0,
0,& wpSkillsSequence,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,0,0,
0,0,0,
0,0,0,
};
