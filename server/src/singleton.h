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

#if !defined(__SINGLETONHOLDER_H__)
#define __SINGLETONHOLDER_H__

#include "platform.h"
#include <stdlib.h>

/************************************************************************/
/* LifeTime Policies                                                    */
/************************************************************************/
struct NoDestroy
{
	template<typename T>
	static void scheduleDestruction( T*, void( * )() )
	{
	}
};

struct DefaultLifeTime
{
	template<typename T>
	static void scheduleDestruction( T*, void( *pFun )() )
	{
		atexit( pFun );
	}
};

/*!
	Singleton Template Class
	Provides an unified interface and handling of uniqueness of	a Singleton.
*/
template<typename T, typename Policy = DefaultLifeTime>
class Singleton
{
public:
	inline static T* instance();
	inline static bool wasDestroyed();

protected:
	Singleton()
	{;}

	~Singleton()
	{;}

private:
	inline static void makeInstance();
	inline static void destroySingleton();

	// Forbid copies
	Singleton( Singleton& );
	Singleton& operator=( Singleton& );

	// Data
	static T* smInstance;
	static bool smDestroyed;
};

/************************************************************************/
/* Singleton Member Functions                                           */
/************************************************************************/

template<typename T, typename Policy>
T* Singleton<T, Policy>::smInstance = NULL;

template<typename T, typename Policy>
bool Singleton<T, Policy>::smDestroyed = false;

template<typename T, typename Policy>
inline T* Singleton<T, Policy>::instance()
{
	if( !smInstance )
	{
		makeInstance();
	}
	return smInstance;
}

template<typename T, typename Policy>
inline bool Singleton<T, Policy>::wasDestroyed()
{
	return smDestroyed;
}

template<typename T, typename Policy>
inline void Singleton<T, Policy>::makeInstance()
{
	if( !smInstance )
	{
		smInstance = new T();
		Policy::scheduleDestruction( smInstance, &destroySingleton );
		smDestroyed = false;
	}
}

template<typename T, class Policy>
inline void Singleton<T, Policy>::destroySingleton()
{
	delete smInstance;
	smInstance = NULL;
	smDestroyed = true;
}

#endif // __SINGLETONHOLDER_H__
