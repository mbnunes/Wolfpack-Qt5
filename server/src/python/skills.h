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

class cChar;
typedef cChar* P_CHAR;

#include "../defines.h"
#include "../chars.h"
#include "utilities.h"

typedef struct {
    PyObject_HEAD;
	P_CHAR pChar;
	bool base;
} wpSkills;

int wpSkills_length( wpSkills *self )
{
	Q_UNUSED(self);
	return ALLSKILLS;
}

PyObject *wpSkills_get( wpSkills *self, int skill )
{
	if( !self->pChar || self->pChar->free || skill >= ALLSKILLS )
		return PyInt_FromLong( -1 );

	if( self->base )
		return PyInt_FromLong( self->pChar->baseSkill( skill ) );
	else
		return PyInt_FromLong( self->pChar->skill( skill ) );
}

PyObject *wpSkills_set( wpSkills *self, int skill, PyObject *pValue )
{
	if( !self->pChar || self->pChar->free || skill >= ALLSKILLS || !PyInt_Check( pValue ) )
		return PyFalse;

	UINT16 value = PyInt_AsLong( pValue );

	if( self->base )
		self->pChar->setBaseSkill( skill, value );
	else
		self->pChar->setSkill( skill, value );

	return PyTrue;
}

static PySequenceMethods wpSkillsSequence = {
	(inquiry)wpSkills_length,
	0,		
	0,		
	(intargfunc)wpSkills_get,
	0,
	(intobjargproc)wpSkills_set,
};

static PyTypeObject wpSkillsType = {
    PyObject_HEAD_INIT(NULL)
    0,
    "WPSkills",
    sizeof(wpSkillsType),
    0,
	wpDealloc,
    0,
    0,
    0,
    0,
    0,
    0,
    &wpSkillsSequence,
};
