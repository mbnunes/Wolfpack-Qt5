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
//	Wolfpack Homepage: http://www.wpdev.com/
//========================================================================================

#ifndef __ITEM_ARRAY
#define __ITEM_ARRAY

#include "wolfpack.h" // needs global var schei___ and item_st *TYPE*
#include "debug.h"    // for logg macro

// typedefs

// macros

#define MAKE_ITEM_REF(i) ItemArray->MakeItemref(i)
#define LOG_INVALID_I_REF(err,meSSage) if(err=ItemArray->GetError()) { strcpy(schei___, meSSage); strcat(schei___," errorcode:%i\n"); LogCritical(schei___ _ err); } // strcpy stuff to prevent const string crashes 
#define GET_I_ERROR(err) err=ItemArray->GetError()

#define MAKE_ITEMREF_LOGGED(i,err) ItemArray->MakeItemref(i); err=ItemArray->GetError(); if (err) { strcpy(schei___, "invalid item index "); strcat(schei___," errorcode:%i\n"); LogCritical(schei___ _ err); } 
#define MAKE_ITEMREF_C(i)			ItemArray->MakeItemref(i); int err=ItemArray->GetError(); if (err) { continue;}
//#define MAKE_ITEMREF_LR(i)			ItemArray->MakeItemref(i); {int err=ItemArray->GetError(); if (err) { strcpy(schei___, "invalid item index "); strcat(schei___," errorcode:%i\n"); LogCritical(schei___ _ err); return;} }
//#define MAKE_ITEMREF_LRV(i,retval)	ItemArray->MakeItemref(i); {int err=ItemArray->GetError(); if (err) { strcpy(schei___, "invalid item index "); strcat(schei___," errorcode:%i\n"); LogCritical(schei___ _ err); return retval;} }
#define MAKE_ITEMREF_LR(i)			ItemArray->MakeItemref(i); {int err=ItemArray->GetError(); if (err) { strcpy(schei___, "invalid item index <%i>\n"); LogCritical(schei___ _ i); return;} }
#define MAKE_ITEMREF_LRV(i,retval)	ItemArray->MakeItemref(i); {int err=ItemArray->GetError(); if (err) { strcpy(schei___, "invalid item index <%i>\n"); LogCritical(schei___ _ i); return retval;} }

#define DEREF_P_ITEM(pi) ((pi-realitems)-I_W_O_1)

// Duke, 21.11.2000: the next defines are intended to replace itemcount loops as follows:
//		for (P_ITEM pi=pFirstItem;pi < pEndOfItems;pi++)
#define pFirstItem (&items[0])
#define pEndOfItems (pFirstItem+itemcount)

// class definiton

class cItemArray
{
   private:
		int error;

   public:		
	    inline int GetError() { return error; }
		P_ITEM MakeItemref(ITEM i);

		cItemArray() ;
		virtual ~cItemArray();

};

#endif

