//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
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

#if !defined( __PYTHON_OBJECTCACHE_H__ )
#define __PYTHON_OBJECTCACHE_H__

#include "engine.h"

template <class T, unsigned int S> 
class FixedSizePtrStack 
{
private:
	T **base, **top, **end;
	
    FixedSizePtrStack (const FixedSizePtrStack& src);	// disable copy
    FixedSizePtrStack& operator = (const FixedSizePtrStack& src);	// disable assignment

public:
	
	FixedSizePtrStack() : base(0), top(0), end(0)
	{
		base = new T*[S];
		top = base;
		end = base + S;
	}
	
    ~FixedSizePtrStack (void)
    {
		delete [] base;
    }

    bool isEmpty() const { return (top == base); }	// check if stack is emptied
    bool isFull() const { return (top == end); } 	// check if stack reached limit
    int size() const { return int(top-base); }  // current number of elements

    void push (T* x)			// push object
    {
		if (top < end) *top++ = x;
    }
	
    T* pop (void)	// pop object - return as function
    {
        if (top > base) return *--top;
		else if ( top == base ) return *base;
		return 0;
    }
	
};

template< class T, unsigned int size >
class cObjectCache
{
private:
	FixedSizePtrStack< PyObject, size > stack;
	
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
		if( stack.size() >= size )
		{
			PyObject_Del( obj );
			return;
		}
		
		stack.push( obj );
	}
};

#endif // __PYTHON_OBJECTCACHE_H__