//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//  Copyright 2001-2004 by holders identified in authors.txt
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
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================

// speech.h: interface for speech.cpp
//
//////////////////////////////////////////////////////////////////////

#if !defined(__SPEECH_H__)
#define __SPEECH_H__ 

// Platform specifics
#include "platform.h"

// System includes
#include <vector>

// QT
#include <qvaluevector.h>

//Wolfpack Includes
#include "speech.h"
#include "typedefs.h"

bool VendorChkName( P_CHAR pVendor, const QString& comm );

class cSpeech
{
public:
	bool response( cUOSocket *socket, P_PLAYER pPlayer, const QString& comm, QValueVector< UINT16 > &keywords );
    void talking( P_PLAYER pChar, const QString &lang, const QString &speech, QValueVector< UINT16 > &keywords, UINT16 color, UINT16 font, UINT8 type );
};

extern cSpeech	*Speech;


#endif
 
