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

#include "qstring.h"

// Forward class declarations
class ISerialization;

/*!
CLASS
    

    This class provides the interface for persistent objects...


USAGE
	Classes who require persistance should inherit off from cSerializable and reimplement
	\sa objectID and \sa Serializable methods.
*/
class cSerializable
{
public:
	virtual ~cSerializable() {}

	virtual void		Serialize( ISerialization &archive );
	virtual QString		objectID( void ) const = 0;
};

/*!
CLASS
    

    This class provides the interface for the serialization mechanism abstracting the
	actual external device, be it a Text file, database or socket.


USAGE
    This is an abstract interface and can not be instantiated directly.

*/
class ISerialization
{
	friend class cSerializable; // give access to doneWritting
protected:
	enum _enState { enReading, enWritting, enClosed };
	_enState _state;
public:
	ISerialization() : _state(enClosed) {}
	virtual ~ISerialization(){}

	virtual void prepareReading(std::string ident, int bLevel = 0 ) { _state = enReading; }
	virtual void prepareWritting(std::string ident) { _state = enWritting; }

	virtual bool isReading() { return (_state == enReading);	}
	virtual bool isWritting(){ return (_state == enWritting);	}

	virtual void close() = 0;	

	virtual void setVersion(unsigned int) = 0;
	virtual unsigned int size() = 0;
	virtual unsigned int getVersion() = 0;

	virtual void readObject( cSerializable * );
	virtual void writeObject( cSerializable * );

	// Write Methods
	virtual void writeObjectID( const QString& ) = 0;
	virtual void write(const char* Key, std::string &data) = 0;
	virtual void write(const char* Key, unsigned int data) = 0;
	virtual void write(const char* Key, signed int data) = 0;
	virtual void write(const char* Key, signed short data) = 0;
	virtual void write(const char* Key, unsigned short data) = 0;
	virtual void write(const char* Key, unsigned char data) = 0;
	virtual void write(const char* Key, signed char data) = 0;
	virtual void write(const char* Key, bool data) = 0;
	virtual void write(const char* Key, double data) = 0;

	// Read Methods
	virtual void readObjectID(QString &data) = 0;

	virtual void read(const char* Key, std::string   &data) = 0;
	virtual void read(const char* Key, unsigned int  &data) = 0;
	virtual void read(const char* Key, signed   int  &data) = 0;
	virtual void read(const char* Key, signed short  &data) = 0;
	virtual void read(const char* Key, unsigned short &data) = 0;
	virtual void read(const char* Key, unsigned char &data) = 0;
	virtual void read(const char* Key, signed   char &data) = 0;
	virtual void read(const char* Key, bool          &data) = 0;
	virtual void read(const char* Key, double		 &data) = 0;

	// I hate to polute it but, as a temp measure:
	void read( const char* Key, QString &data )
	{
		std::string temp;
		read( Key, temp );
		data = temp.c_str();
	}

	void write(const char* Key, QString &data)
	{
		std::string temp;

		if( data.latin1() == NULL )
			temp = "";
		else
			temp = data.latin1();

		write( Key, temp );
	}

protected:
	virtual void doneWritting() {};
};

#endif //__SERIMPLEMENTOR_H__

