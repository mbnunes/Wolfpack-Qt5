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

#include "serbinfile.h"
#include "platform.h"

// Save Path
#include "junk.h"
#include "srvparams.h"

// Library Includes
#include "qfile.h"
#include "qstring.h"
#include "qmap.h"

using namespace std;

const unsigned int backuplevel = 4;

void serBinFile::setVersion(unsigned int __version)
{
	_version = __version;
}

unsigned int serBinFile::getVersion()
{
	return _version;
}

unsigned int serBinFile::size()
{
	return _count;
}

void serBinFile::prepareReading(std::string ident, int bLevel)
{
	// do not recurse forever
	if ( bLevel >= backuplevel )
		return; // nothing more can be done.

	QString fileName( QString( "%1%2" ).arg( SrvParams->worldSaveDirectory() ).arg( ident.c_str() ) );

	if ( bLevel != 0 )
		fileName += QString("-%1").arg(bLevel);
	fileName.append(".bin");
	if ( !QFile::exists(fileName) )
	{
		prepareReading( ident.c_str(), ++bLevel );
		return;
	}
	
	file.open( fileName.latin1(), ios::in | ios::binary );

	if ( !file.is_open() )
		qWarning("Failled to open file %s for reading", fileName.latin1());
	
	file.read((char*)&_version, 4);
	file.read((char*)&_count, 4);
	if ( _count == 0 ) // has not finished writting.
	{
		file.close();
		prepareReading( ident.c_str(), ++bLevel );
		return;
	}

	ISerialization::prepareReading(ident);
}

void serBinFile::prepareWritting(std::string ident)
{
	QString fileName( QString( "%1%2" ).arg( SrvParams->worldSaveDirectory() ).arg( ident.c_str() ) );

	// perform backups
	unsigned int i;
	QFile::remove( ident.c_str() + QString("-%1.%2").arg(backuplevel).arg("bin"));
	for ( i = backuplevel - 1; i > 0; --i )
	{
		QString from = ident.c_str() + QString("-%1.%2").arg(i).arg("bin");
		QString to   = ident.c_str() + QString("-%1.%2").arg(i + 1).arg("bin");
		rename( from.latin1(), to.latin1() );
	}
	rename ( ident.c_str() + QString(".bin"), ident.c_str() + QString("-1%1").arg(".bin"));
	fileName.append(".bin");
	file.open( fileName.latin1(), ios::out | ios::binary );
	if ( !file.is_open() )
		qWarning("Critical Error: could not open %s for writting", fileName.latin1());
	_count = 0;
	// First line in file is version
	file.write((char*)&_version, 4);
	file.write((char*)&_count, 4);
	ISerialization::prepareWritting(ident);
}

void serBinFile::close()
{
	if ( isWritting() )
	{
		file.seekp(4);
		file.write((char*)&_count, 4);
	}
	file.close();
}

void serBinFile::writeObjectID( const QString& data )
{
	write("objectID", (std::string)data.latin1());
	if( _objectlevel == 0 )
		++_count;
	++_objectlevel;
}

void serBinFile::write(const char* Key, std::string &data)
{
	UI32 uiSize = data.size();
	file.write((char*)&uiSize, 4);
	if (uiSize != 0)
		file.write((char*)data.c_str(), uiSize);
}

void serBinFile::write(const char* Key, unsigned int data)
{
	file.write((char*)&data, 4);
}

void serBinFile::write(const char* Key, signed int data)
{
	file.write((char*)&data, 4);
}

void serBinFile::write(const char* Key, signed short data)
{
	file.write((char*)&data, 2);
}

void serBinFile::write(const char* Key, unsigned short data)
{
	file.write((char*)&data, 2);
}

void serBinFile::write(const char* Key, unsigned char data)
{
	file.write((char*)&data, 1);
}

void serBinFile::write(const char* Key, signed char data)
{
	file.write((char*)&data, 1);
}

void serBinFile::write(const char* Key, bool data)
{
	file.write((char*)&data, 1);
}

void serBinFile::write(const char* Key, double data)
{
	file.write((char*)&data, 8);
}

void serBinFile::done()
{
	--_objectlevel;
}

void serBinFile::readObjectID(QString &data)
{
	ISerialization::read("objectID", data);
}

void serBinFile::read(const char* Key, string& data)
{
	UI32 uiSize = 0;
	file.read((char*)&uiSize, 4);
	if (!file.fail())
	{
		if ( uiSize != 0 )
		{
			char* c_Str = new char[uiSize + 1];
			file.read(c_Str, uiSize);
			c_Str[uiSize] = 0;
			data = c_Str;
			delete [] c_Str;
		}
	}
	else
		data = "";
}

void serBinFile::read(const char* Key, unsigned int  &data)
{
	file.read((char*)&data, 4);
}

void serBinFile::read(const char* Key, signed   int  &data)
{
	file.read((char*)&data, 4);
}

void serBinFile::read(const char* Key, signed short &data)
{
	file.read((char*)&data, 2);
}

void serBinFile::read(const char* Key, unsigned short &data)
{
	file.read((char*)&data, 2);
}

void serBinFile::read(const char* Key, unsigned char &data)
{
	file.read((char*)&data, 1);
}

void serBinFile::read(const char* Key, signed   char &data)
{
	file.read((char*)&data, 1);
}

void serBinFile::read(const char* Key, bool &data)
{
	file.read((char*)&data, 1);
}

void serBinFile::read(const char* Key, double &data)
{
	file.read((char*)&data, 8);
}
