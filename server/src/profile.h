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
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#if !defined(__PROFILE_H__)
#define __PROFILE_H__

enum eProfileKeys {
	PF_NICENESS = 0,
	PF_SPAWNCHECK,
	PF_DECAYCHECK,
	PF_WORLDSAVE,
	PF_UOTIMECHECK,
	PF_COMBATCHECK,
	PF_TIMERSCHECK,
	PF_AICHECK,
	PF_NPCCHECK,
	PF_PLAYERCHECK,
	PF_REGENERATION,
	PF_AICHECKFINDACTION,
	PF_AICHECKEXECUTEACTION,
	PF_COUNT,
};

//#define ENABLE_PROFILING 0

#if defined(ENABLE_PROFILING)
void startProfiling(eProfileKeys key);
void stopProfiling(eProfileKeys key);
void dumpProfilingInfo();
void clearProfilingInfo();
#else
#define startProfiling(key)
#define stopProfiling(key)
#define dumpProfilingInfo()
#define clearProfilingInfo()
#endif

#endif
