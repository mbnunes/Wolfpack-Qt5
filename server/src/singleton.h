//
// Derived from Wolfpack (http://www.wpdev.org)
// 

/*
	Based on the ideas from Modern C++ Design.
*/

#if !defined(__SINGLETONHOLDER_H__)
#define __SINGLETONHOLDER_H__

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

