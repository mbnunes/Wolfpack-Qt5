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

/*
	Based on the ideas from Modern C++ Design.
*/

#if !defined(__SINGLETONHOLDER_H__)
#define __SINGLETONHOLDER_H__

#include "platform.h" // just to silent the warnings.
#include <stdlib.h>

/*****************************************************************************
  LifeTime Policies
 *****************************************************************************/

struct NoDestroy
{
	template < class T >
	static void scheduleDestruction(T*, void(*)()) {}
};

struct DefaultLifeTime
{
	template <class T >
	static void scheduleDestruction(T*, void(*pFun)())
	{
		atexit(pFun);
	}
};

/*!
	Singleton Holder class
	This templated class provides an unified interface and handling of uniqueness of
	a Singleton.
*/
template
<
	typename T,
	class LifetimePolicy = DefaultLifeTime
>
class SingletonHolder
{
private:
	static void makeInstance();
	static void destroySingleton();

	// Protection
	SingletonHolder();

	// Data
	typedef T* ptrInstanceType;
	static ptrInstanceType ptrInstance;

public:
	static T* instance();
	static bool destroyed_;
};


/*****************************************************************************
  SingletonHolder member functions
 *****************************************************************************/


// Singleton Holder Data
template<typename T,class L>
typename SingletonHolder<T, L>::ptrInstanceType SingletonHolder<T,L>::ptrInstance = 0;
template<typename T,class L>
bool SingletonHolder<T,L>::destroyed_ = false;

// Singleton Members
#if defined(_DEBUG)
#include <typeinfo>
#endif

template
<
	typename T,
	class L
>
inline T* SingletonHolder<T, L>::instance()
{
	if ( !ptrInstance )
	{
		makeInstance();
	}
	return ptrInstance;
}

template
<
	typename T,
	typename LifetimePolicy
>
void SingletonHolder<T, LifetimePolicy>::makeInstance()
{
	if ( !ptrInstance )
	{
		ptrInstance = new T();
		LifetimePolicy::scheduleDestruction(ptrInstance, &destroySingleton);
		destroyed_ = false;
	}
}

template
<
	typename T,
	class LifetimePolicy
>
void SingletonHolder<T, LifetimePolicy>::destroySingleton()
{
	delete ptrInstance;
	ptrInstance = 0;
	destroyed_ = true;
}

#endif // __SINGLETONHOLDER_H__

