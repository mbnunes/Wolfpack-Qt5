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
//	Wolfpack Homepage: http://www.wpdev.com/
//========================================================================================


#if !defined(__STORAGE_H__)
#define __STORAGE_H__

// Platform specifics
#include "platform.h"

// System includes

#include <iostream>
#include <map>
#include <vector>

using namespace std;

// Forward class definition


// wolfpack includes
#include "typedefs.h"

// Class definition


template<class T> class Storage_cl
{
public:
	Storage_cl();
	~Storage_cl();
	bool insert(T& cData, int serial);
	bool insert(T* ptrData, int serial);
	int Count(void);
	T* getPtr(int serial);
	T* Next(int &offset);
	
	void remove(int serial);
	T* operator[](long index);
	
private:
	map<int, T*> mapData;
	map<int, T*>::iterator iterData;
	vector<int> indices;
};

class Container_cl
{
public:
	Container_cl();
	~Container_cl();
	bool insert(SERIAL serContainer, SERIAL serObject);
	vector<SERIAL> getData(SERIAL serContainer);
	bool find(SERIAL serContainer, SERIAL serObject);
	bool remove(SERIAL serContainer, SERIAL serObject);
	
private:
	multimap<SERIAL, SERIAL> mapData;
	multimap<SERIAL, SERIAL>::iterator iterData;
};

#endif // __STORAGE_H__

