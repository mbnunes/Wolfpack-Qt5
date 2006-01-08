/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2006 by holders identified in AUTHORS.txt
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

enum eProfileKeys
{
	PF_NICENESS					= 0,
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
	PF_AICHECKNPCS,
	PF_AICHECKITEMS,
	PF_COUNT,
};

#ifdef ENABLE_PROFILING
void startProfiling( eProfileKeys key );
void stopProfiling( eProfileKeys key );
void dumpProfilingInfo();
void clearProfilingInfo();
#else
#define startProfiling(key)
#define stopProfiling(key)
#define dumpProfilingInfo()
#define clearProfilingInfo()
#endif

/**
* Those macros help profiling using QTime.
* They allow to sum up the time taken by a given bit of code
* in a method called several times.
* This way one can find out which low-level method used by a high-level
* method is taking most of its time
*
*
*  Declare the var somewhere, out of any method:
*   int pr_theMethod = 0;
*  (the name pr_* helps finding and removing all of this before committing)
*
*  Then in the method, around the code to be timed:
*   PROFILE_METHOD_BEGIN( pr_theMethod );
*   ...
*   PROFILE_METHOD_END( pr_theMethod );
*
*  And finally, to see the result, put this call in a method
*  called after all that (destructor, program exit...)
*   PROFILE_METHOD_PRINT( pr_theMethod, "theMethod" );
*
*/
#include <QTime>
#define PROFILE_METHOD_BEGIN(sym) extern int sym; QTime profile_dt##sym; profile_dt##sym.start();
#define PROFILE_METHOD_END(sym) extern int sym; sym += profile_dt##sym.elapsed();
#define PROFILE_METHOD_PRINT(sym, name) extern int sym; Console::instance()->send( QString("%1 took %2 milliseconds\n").arg( name ).arg( sym ) );


#endif
