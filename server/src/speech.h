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

// speech.h: interface for speech.cpp
//
//////////////////////////////////////////////////////////////////////

#if !defined(__SPEECH_H__)
#define __SPEECH_H__ 

// Platform specifics
#include "platform.h"


// System includes

#include <iostream>

using namespace std ;

// Class Declaration

//Wolfpack Includes
#include "wolfpack.h"
#include "SndPkg.h"
#include "debug.h"
#include "speech.h"
#include "utilsys.h"


void responsevendor(UOXSOCKET s, int vendor);
void talking(UOXSOCKET, string);

void wchar2char (const char* str);
void char2wchar (const char* str);


#endif
 
