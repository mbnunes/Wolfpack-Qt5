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

#if !defined( __PYTHON_OBJECTCACHE_H__ )
#define __PYTHON_OBJECTCACHE_H__

#include "engine.h"

#include <stdexcept>

template <class T, unsigned int MAXSIZE> 
class FixedSizePtrStack 
{
private:
	T* elements[MAXSIZE];
	int numElements;
	
    FixedSizePtrStack (const FixedSizePtrStack& src);	// disable copy
    FixedSizePtrStack& operator = (const FixedSizePtrStack& src);	// disable assignment

public:
	
	FixedSizePtrStack() : numElements(0)
	{
	}
	
    bool isEmpty()	const { return (numElements == 0); }		// check if stack is emptied
    bool isFull()	const { return (numElements == MAXSIZE); } 	// check if stack reached limit
    int size()		const { return numElements; }				// current number of elements

    void push (T* x)			// push object ( Note, this is exception safe, don't change)
    {
		if ( numElements == MAXSIZE )
			throw std::out_of_range("FixedSizePtrStack<>::push(): stack is full");
		elements[numElements] = x; // Append
		++numElements;			   // Increase number.
    }
	
    T* pop()	// pop object ( Note, this is exception safe, don't change )
    {
        if ( numElements <= 0 ) return 0;
		T* ret = elements[numElements - 1];
		--numElements;
		return ret;
    }
	
};

template< class T, unsigned int MAXSIZE >
class cObjectCache
{
private:
	FixedSizePtrStack< PyObject, MAXSIZE > stack;
	
public:
	
	virtual ~cObjectCache()
	{
		while ( !stack.isEmpty() )
			PyObject_Del( stack.pop() );
	}
	
	T *allocObj( PyTypeObject *type )
	{
		if( !stack.isEmpty() )
			return (T*)stack.pop();
		
		return PyObject_New( T, type );
	}
	
	void freeObj( PyObject *obj )
	{
		if( stack.size() >= MAXSIZE )
		{
			PyObject_Del( obj );
			return;
		}
		
		stack.push( obj );
	}
};

#endif // __PYTHON_OBJECTCACHE_H__
