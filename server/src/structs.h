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

#if !defined(__STRUCTS_H__)
#define __STRUCTS_H__

// Platform Specifics
#include "platform.h"

// System Headers
#include <map>

// Library Headers
#include <qstring.h>

// Wolfpack Headers
#include "defines.h"
#include "typedefs.h"
#include "coord.h"


struct move_st
{
	int effect[18];
};

struct stat_st
{
	int effect[17];
};

struct sound_st
{
	int effect[2];
};

struct location_st
{
	int x1;
	int y1;
	int x2;
	int y2;
	// char region;
	QString region;
};

struct reag_st
{
	int ginseng;
	int moss;
	int drake;
	int pearl;
	int silk;
	int ash;
	int shade;
	int garlic;
};

struct tele_locations_st
{
	Coord_cl destination, origem;
};

struct ServerList_st
{
	Q_UINT16 uiIndex;
	QString sServer;
	Q_UINT8 uiFull;
	Q_UINT8 uiTime;
	Q_UINT32 ip;
	QCString sIP;
	Q_UINT16 uiPort;
};

#endif

