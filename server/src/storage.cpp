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

#include "storage.h" 

template<class T> Storage_cl<T>::Storage_cl()
{
}

template<class T> Storage_cl<T>::~Storage_cl()
{
	//We need to delete the items we contain as well
	
	for (iterData=mapData.begin(); iterData != mapData.end(); iterData++)
	{
		delete iterData->second ;
	}
	mapData.clear() ;
}

template<class T> int Storage_cl<T>::Count(void)
{
	return mapData.size();
}

template<class T> bool Storage_cl<T>::insert(T& cData, int serial) 
{
	indices.push_back(serial);
	return mapData.insert(make_pair(serial,&cData));
}
template<class T> bool Storage_cl<T>::insert(T* ptrData, int serial) 
{
	indices.push_back(serial);
	return mapData.insert(make_pair(serial,ptrData)) ;
}

template<class T> T* Storage_cl<T>::getPtr(int serial)
{

	iterData = mapData.find(serial) ;
	T* ptrData = NULL ;
	if (iterData != mapData.end())
		ptrData = iterData->second ;
	return ptrData ;
}

template<class T> void Storage_cl<T>::remove(int serial) 
{
	// First find the ptr
	iterData = mapData.find(serial) ;
	if (iterData != mapData.end())
	{
		// Delete the actual object
		delete iterData->second ;
		// Erase it out of the map
		mapData.erase(iterData) ;
	}
}

template<class T> T* Storage_cl<T>::Next(int &offset)
{
	iterData = mapData.begin();
	advance(iterData, offset);
	if (iterData != mapData.end())
	{
		offset++;
		T* ptrData = NULL;
		ptrData = iterData->second;
		return ptrData;
	}
	else
		return NULL;
}

template<class T> T* Storage_cl<T>::operator[](long index)
{
	iterData = mapData.begin();
	T* ptrData = iterData->second;
	if(index < 0 || index >=indices.size())
	{
		LogError("Error in chars[] index!\n");
		return ptrData;
	}
	
	interData = mapData.find(indices[index]);
	if (interData == mapData.end())
	{
		LogError("Error in chars[] index!\n");
		return ptrData;
	}
	ptrData = iterData->second;
	return ptrData;
}

Container_cl::Container_cl() 
{
}

Container_cl::~Container_cl() 
{
	mapData.clear() ;
}

bool Container_cl::insert(SERIAL serContainer, SERIAL serObject) 
{
	bool bStatus = false;
	if (!find(serContainer,serObject))
	{
		// We dont want to insert the same association twice
		mapData.insert(make_pair(serContainer,serObject)) ;
		bStatus = true;
	}
	return bStatus;
}

bool Container_cl::find(SERIAL serContainer, SERIAL serObject)
{
	bool bStatus = false ;
	typedef multimap<SERIAL, SERIAL>::iterator iterSerial;
	pair<iterSerial, iterSerial> iterRange = mapData.equal_range(serContainer);
		
	for (iterData = iterRange.first; iterData != iterRange.second; ++iterData)
	{
		if (iterData->second == serObject)
		{
			bStatus = true ;
			break ;
		}
	}
	return bStatus;
}

bool Container_cl::remove(SERIAL serContainer, SERIAL serObject)
{
	bool bStatus = false ;
	typedef multimap<SERIAL, SERIAL>::iterator iterSerial;
	pair<iterSerial, iterSerial> iterRange = mapData.equal_range(serContainer);
		
	for (iterData = iterRange.first; iterData != iterRange.second; ++iterData)
	{
		if (iterData->second == serObject)
		{
			mapData.erase(iterData);
			bStatus = true;
			break ;
		}
	}
	return bStatus;
}

vector<SERIAL> Container_cl::getData(SERIAL serContainer) 
{
	vector<SERIAL> vecValue;
	typedef multimap<SERIAL, SERIAL>::iterator iterSerial;
	pair<iterSerial, iterSerial> iterRange = mapData.equal_range(serContainer);
		
	for (iterData = iterRange.first; iterData != iterRange.second; ++iterData)
	{
		vecValue.push_back(iterData->second) ;
	}
	return vecValue;
}


