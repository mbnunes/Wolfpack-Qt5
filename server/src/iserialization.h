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

#if !defined(__SERIMPLEMENTOR_H__)
#define __SERIMPLEMENTOR_H__

#include <string>

//#include "uobject.h"

// Forward class declarations
class cUObject;

/*!
CLASS
    

    This class provides the interface for the serialization mechanism abstracting the
	actual external device, be it a Text file, database or socket.


USAGE
    This is an abstract interface and can not be instantiated directly.

*/
//##ModelId=3C5D92EF02A8
class ISerialization
{
	friend class cUObject; // give access to doneWritting
protected:
	//##ModelId=3C5D92F0037C
	enum _enState { enReading, enWritting, enClosed };
	//##ModelId=3C5D92EF03CA
	_enState _state;
public:
	//##ModelId=3C5D92EF03DE
	ISerialization() : _state(enClosed) {}
	//##ModelId=3C5D92F00000
	virtual ~ISerialization(){}

	//##ModelId=3C5D92F0000A
	virtual void prepareReading(std::string ident) { _state = enReading; }
	//##ModelId=3C5D92F0001E
	virtual void prepareWritting(std::string ident) { _state = enWritting; }

	//##ModelId=3C5D92F00032
	virtual bool isReading() { return (_state == enReading);	}
	//##ModelId=3C5D92F0003D
	virtual bool isWritting(){ return (_state == enWritting);	}

	//##ModelId=3C5D92F00047
	virtual void close() = 0;	

	//##ModelId=3C5D92F0005A
	virtual void setVersion(unsigned int) = 0;
	//##ModelId=3C5D92F0006E
	virtual unsigned int getVersion() = 0;

	//##ModelId=3C5D92F00079
	virtual void readObject( cUObject * );
	//##ModelId=3C5D92F0008C
	virtual void writeObject( cUObject * );

	// Write Methods
	//##ModelId=3C5D92F000A1
	virtual void writeObjectID(std::string) = 0;
	//##ModelId=3C5D92F000B5
	virtual void write(std::string Key, std::string &data) = 0;
	//##ModelId=3C5D92F000D3
	virtual void write(std::string Key, unsigned int data) = 0;
	//##ModelId=3C5D92F000F1
	virtual void write(std::string Key, signed int data) = 0;
	//##ModelId=3C5D92F0010F
	virtual void write(std::string Key, signed short data) = 0;
	//##ModelId=3C5D92F00155
	virtual void write(std::string Key, unsigned short data) = 0;
	//##ModelId=3C5D92F00173
	virtual void write(std::string Key, unsigned char data) = 0;
	//##ModelId=3C5D92F00191
	virtual void write(std::string Key, signed char data) = 0;
	//##ModelId=3C5D92F001AF
	virtual void write(std::string Key, bool data) = 0;

	// Read Methods
	//##ModelId=3C5D92F001CD
	virtual void readObjectID(std::string &data) = 0;

	//##ModelId=3C5D92F001E1
	virtual void read(std::string Key, std::string   &data) = 0;
	//##ModelId=3C5D92F001FF
	virtual void read(std::string Key, unsigned int  &data) = 0;
	//##ModelId=3C5D92F0021D
	virtual void read(std::string Key, signed   int  &data) = 0;
	//##ModelId=3C5D92F0023B
	virtual void read(std::string Key, signed short  &data) = 0;
	//##ModelId=3C5D92F00259
	virtual void read(std::string Key, unsigned short &data) = 0;
	//##ModelId=3C5D92F00281
	virtual void read(std::string Key, unsigned char &data) = 0;
	//##ModelId=3C5D92F0029F
	virtual void read(std::string Key, signed   char &data) = 0;
	//##ModelId=3C5D92F002BD
	virtual void read(std::string Key, bool          &data) = 0;

protected:
	//##ModelId=3C5D92F002DB
	virtual void doneWritting() {};
};

#endif //__SERIMPLEMENTOR_H__

