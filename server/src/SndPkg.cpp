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

/////////////////////
// Name:	SndPkg.cpp
// Purpose: functions that send packages to the Client
// History:	cut from wolfpack.cpp by Duke, 25.10.00
// Remarks:	not necessarily ALL those functions
//

#include "wolfpack.h"
#include "utilsys.h"
#include "debug.h"
#include "basics.h"
#include "SndPkg.h"
#include "itemid.h"
#include "guildstones.h"
#include "combat.h"
#include "mapobjects.h"
#include "srvparams.h"
#include "network.h"
#include "classes.h"
#include "territories.h"
#include "wpdefmanager.h"
#include "skills.h"

#include "network/uotxpackets.h"
#include "network/uosocket.h"

#undef  DBGFILE
#define DBGFILE "SndPkg.cpp"


void sysbroadcast( const QString& txt ) // System broadcast in bold text
{
	for( cUOSocket *socket = cNetwork::instance()->first(); socket; socket = cNetwork::instance()->next() )
		socket->sysMessage( txt, 0x84d, 0 );
}

void sysmessage(UOXSOCKET s, const QString& txt)
{
	sysmessage(s, (char*)txt.latin1());
}

void sysmessage(UOXSOCKET s, short color, const QString& txt)
{
	sysmessage(s, color, (char*)txt.latin1());
}

void sysmessage(UOXSOCKET s, char *txt, ...) // System message (In lower left corner)
{
	qWarning("sendmessage disabled, use cUOSocket::sendMessage() instead");
}

void sysmessage(UOXSOCKET s, short color, char *txt, ...) // System message (In lower left corner)
{
	qWarning("sendmessage disabled, use cUOSocket::sendMessage() instead");
}

void tileeffect(int x, int y, int z, char eff1, char eff2, char speed, char loop)
{//AntiChrist
	char effect[29];
	int i, j;
	for (i=0;i<29;i++)
	{
		effect[i]=0;
	}
	effect[0]=0x70; // Effect message
	effect[1]=0x02; // Stay at x, y, z effect
	//[2] to [9] are	not applicable here.
	effect[10]=eff1;// Object id of the effect
	effect[11]=eff2;
	effect[12]=x>>8;
	effect[13]=x%256;
	effect[14]=y>>8;
	effect[15]=y%256;
	effect[16]=z;
	//[17] to [21] are not applicable here.
	effect[22]=speed;
	effect[23]=loop; // 0 is really long.	1 is the shortest.
	//[24] to [25] are not applicable here.
	effect[26]=1; // LB possible client crashfix
	effect[27]=0;

	for (j=0;j<now;j++)
	{
		if (perm[j])
		{
			if(abs(currchar[j]->pos.x-x)<=VISRANGE && abs(currchar[j]->pos.y-y)<=VISRANGE) 
				Xsend(j, effect, 28);
		}

	}
}

void teleport(P_CHAR pc) // Teleports character to its current set coordinates
{
	pc->resend( true );
	
	if( pc->socket() )
		pc->socket()->resendWorld( false );

	cAllTerritories::getInstance()->check( pc );
}

void npctalk_runic(int s, P_CHAR pc_npc, const char *txt,char antispam) // NPC speech
{
	int tl;
	char machwas;

	if (pc_npc == NULL || s==-1) return; //lb

	if (antispam)
	{
		if (pc_npc->antispamtimer()<uiCurrentTime)
		{
			pc_npc->setAntispamtimer(uiCurrentTime+MY_CLOCKS_PER_SEC*10);
			machwas=1;
		} else machwas=0;
	} else machwas=1;

	if (machwas)
	{
	/*	tl=44+strlen(txt)+1;
		talk[1]=tl>>8;
		talk[2]=tl%256;
		LongToCharPtr(pc_npc->serial,  &talk[3]);
		ShortToCharPtr(pc_npc->serial, &talk[7]);
		talk[9]=0;

		// color here

		talk[10]=0;
		talk[11]=1; // black

		// big problems with contrast/readability, plz find out a better one, LB !

		talk[12]=0;
		talk[13]=8;

		Xsend(s, talk, 14);
		Xsend(s, (void*)pc_npc->name.latin1(), 30);
		Xsend(s, txt, strlen(txt)+1);*/
	}
}

void npcemote(int s, P_CHAR pc_npc, const char *txt, char antispam) // NPC speech
{
	int tl;
	char machwas;

	if (antispam)
	{
		if (pc_npc->antispamtimer()<uiCurrentTime)
		{
			pc_npc->setAntispamtimer(uiCurrentTime+MY_CLOCKS_PER_SEC*10);
			machwas=1;
		} else machwas=0;
	} else machwas=1;

	if (machwas)
	{
		pc_npc->emote( txt );
	}
}

void endtrade(SERIAL serial)
{
}

void PlayDeathSound( P_CHAR pc )
{
    if ( pc == NULL )
		return;

	if (pc->xid()==0x0191)
	{
		switch(RandomNum(0, 3)) // AntiChrist - uses all the sound effects
		{
		case 0:		pc->soundEffect( 0x0150 );	break;// Female Death
		case 1:		pc->soundEffect( 0x0151 );	break;// Female Death
		case 2:		pc->soundEffect( 0x0152 );	break;// Female Death
		case 3:		pc->soundEffect( 0x0153 );	break;// Female Death
		}
	}
	else if (pc->xid()==0x0190)
	{
		switch( RandomNum(0, 3) ) // AntiChrist - uses all the sound effects
		{
		case 0:		pc->soundEffect( 0x015A );	break;// Male Death
		case 1:		pc->soundEffect( 0x015B );	break;// Male Death
		case 2:		pc->soundEffect( 0x015C );	break;// Male Death
		case 3:		pc->soundEffect( 0x015D );	break;// Male Death
		}
	}
	else
	{
		playmonstersound( pc, pc->xid(), SND_DIE );
	}
}

