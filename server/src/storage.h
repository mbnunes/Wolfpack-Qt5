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
//	Wolfpack Homepage: http://wpdev.sf.net/
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


//##ModelId=3C5D92CB03AA
template<class T> class Storage_cl
{
public:
	//##ModelId=3C5D92CB03D2
	Storage_cl();
	//##ModelId=3C5D92CB03D3
	~Storage_cl();
	//##ModelId=3C5D92CB03DC
	bool insert(T& cData, int serial);
	//##ModelId=3C5D92CC000A
	bool insert(T* ptrData, int serial);
	//##ModelId=3C5D92CC0027
	int Count(void);
	//##ModelId=3C5D92CC003B
	T* getPtr(int serial);
	//##ModelId=3C5D92CC0046
	T* Next(int &offset);
	
	//##ModelId=3C5D92CC0059
	void remove(int serial);
	//##ModelId=3C5D92CC006D
	T* operator[](long index);
	
private:
	//##ModelId=3C5D92CC00AB
	map<int, T*> mapData;
	//##ModelId=3C5D92CC00D2
	map<int, T*>::iterator iterData;
	//##ModelId=3C5D92CC0118
	vector<int> indices;
};

//##ModelId=3C5D92CC0185
class Container_cl
{
public:
	//##ModelId=3C5D92CC01A3
	Container_cl();
	//##ModelId=3C5D92CC01AD
	~Container_cl();
	//##ModelId=3C5D92CC01B7
	bool insert(SERIAL serContainer, SERIAL serObject);
	//##ModelId=3C5D92CC01CB
	vector<SERIAL> getData(SERIAL serContainer);
	//##ModelId=3C5D92CC01DF
	bool find(SERIAL serContainer, SERIAL serObject);
	//##ModelId=3C5D92CC01F3
	bool remove(SERIAL serContainer, SERIAL serObject);
	
private:
	//##ModelId=3C5D92CC023A
	multimap<SERIAL, SERIAL> mapData;
	//##ModelId=3C5D92CC0262
	multimap<SERIAL, SERIAL>::iterator iterData;
};

#endif // __STORAGE_H__

