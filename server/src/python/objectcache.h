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
			throw std::out_of_range(std::string("FixedSizePtrStack<>::push(): stack is full"));
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

template<class T, unsigned int MAXSIZE>
class cObjectCache {
private:
	QPtrList<PyObject> objects;

	// Search for an object for which only we
	// hold a reference count. The reference count
	// is increased for the object.
	PyObject *findFreeObject() {
        PyObject *obj;
		for (obj = objects.first(); obj; obj = objects.next()) {
			if (obj->ob_refcnt == 1) {
				Py_INCREF(obj);
				return obj;
			}
		}
		return 0;
	}

public:
	virtual ~cObjectCache() {
		clear();
	}

	void clear() {
		PyObject *obj;
		for (obj = objects.first(); obj; obj = objects.next()) {
			Py_DECREF(obj);
		}
		objects.clear();
	}

	cObjectCache() {
		objects.setAutoDelete(false);
	}
	
	T *allocObj(PyTypeObject *type) {
		T *obj = (T*)findFreeObject();

		if (!obj) {
			obj = PyObject_New(T, type);
			if (objects.count() < MAXSIZE) {
				Py_INCREF(obj);
				objects.append((PyObject*)obj);
			}
		}
		
		return obj;
	}
};

#endif // __PYTHON_OBJECTCACHE_H__
