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

///////////////////////////
//   Item_arry class     //
//   Authors: LB & Duke  //
///////////////////////////

#include "wolfpack.h"
#include "char_array.h"
#include "debug.h"

#undef  DBGFILE
#define DBGFILE "char_array.cpp"


// Construction
cCharArray::cCharArray()
{
  //error=false;
}

// destruction
cCharArray::~cCharArray()
{
	;
}

P_CHAR cCharArray::MakeCharref(CHARACTER i)
{
  error=0; // clear previous state, 0=no error

  if ( i<0 || i>=charcount)
  {
	  error=1;
	  return NULL;
  }

  return &realchars[i+C_W_O_1]; // plus wrapping offset, see charwrap.cpp for details
  
}

