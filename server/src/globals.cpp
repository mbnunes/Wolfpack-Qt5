//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001-2003 by holders identified in authors.txt
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

// Global Variables

#include "verinfo.h"
#include "walking.h"
#include "wpdefmanager.h"
#include "wptargetrequests.h"
#include "typedefs.h"
#include "console.h"
#include "srvparams.h"
#include "skills.h"
#include "maps.h"
#include "speech.h"

unsigned int uiCurrentTime;
wp_version_info wp_version;

// Library Includes
#include <qdatetime.h>

enServerState serverState;
QDateTime uoTime;
int autosaved;
int secure = 1;
int dosavewarning = 0;
volatile int keeprun;
unsigned int shoprestocktime = 0;
unsigned int hungerdamagetimer = 0;

cSrvParams		*SrvParams;
Maps			*Map;
cSkills			*Skills;
cSpeech	        *Speech;
WPDefManager    *DefManager;
PersistentBroker* persistentBroker;


