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

#include "iserialization.h"
#include "uobject.h"
#include "tmpeff.h"

/*!
 * Reads the entiere object out of the serialization stream.
 * The object must be already instantiated and be of the proper time
 * according to the value returned by readObjectID().
 *
 * @param *base : Pointer to object to be loaded.
 *
 * @return void  : none
 *
 * @see readObjectID
 */
void ISerialization::readObject( cSerializable *base )
{
	base->Serialize(*this);
}

/*!
 * Writes the entiere object to the serialization stream.
 *
 * @param *base : Pointer to object to be saved.
 *
 * @return void  : none
 *
 * @see readObjec
 */
void ISerialization::writeObject( cSerializable *base )
{
	writeObjectID( base->objectID() );
	base->Serialize(*this);
}


void cSerializable::Serialize( ISerialization &archive )
{
	if ( archive.isWritting())
		archive.doneWritting();
}
