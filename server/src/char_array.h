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
//	Wolfpack Homepage: http://www.wpdev.sf.net/
//========================================================================================

#ifndef __CHAR_ARRAY
#define __CHAR_ARRAY

#include "wolfpack.h" // needs global var schei___ and item_st *TYPE*
#include "debug.h"    // for logg macro

// typedefs

typedef       cChar *P_CHAR;
typedef const cChar *PC_CHAR;

// macros

#define MAKE_CHAR_REF(i) CharArray->MakeCharref(i)
#define LOG_INVALID_C_REF(err,meSSage) if(err=CharArray->GetError()) { strcpy(schei___, meSSage); strcat(schei___," errorcode:%i\n"); LogCritical(schei___ _ err); } // strcpy stuff to prevent const string crashes 
#define GET_C_ERROR(err) err=CharArray->GetError()

//#define MAKE_CHARREF_LOGGED(i,err)  CharArray->MakeCharref(i); err=CharArray->GetError(); if (err) { strcpy(schei___, "invalid char index "); strcat(schei___," errorcode:%i\n"); LogCritical(schei___ _ err); } 
//#define MAKE_CHARREF_C(i)			CharArray->MakeCharref(i); int err=CharArray->GetError(); if (err) { continue;}
//#define MAKE_CHARREF_LR(i)			CharArray->MakeCharref(i); {int err=CharArray->GetError(); if (err) { strcpy(schei___, "invalid char index <%i>\n"); LogCritical(schei___ _ i); return;} }
//#define MAKE_CHARREF_LRV(i,retval)	CharArray->MakeCharref(i); {int err=CharArray->GetError(); if (err) { strcpy(schei___, "invalid char index <%i>\n"); LogCritical(schei___ _ i); return retval;} }

#define DEREF_P_CHAR(pc) ((pc-realchars)-C_W_O_1)
// class definiton

class cCharArray
{
   private:
		int error;

   public:		
	    inline int GetError() { return error; }
		P_CHAR MakeCharref(CHARACTER i);

		cCharArray() ;
		virtual ~cCharArray();

};

#endif

