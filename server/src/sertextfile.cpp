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

#include "sertextfile.h"

using namespace std;

void serTextFile::setVersion(unsigned int __version)
{
	_version = __version;
}

unsigned int serTextFile::getVersion()
{
	return _version;
}

void serTextFile::prepareReading(std::string ident)
{
	std::string fileName(ident);
	fileName.append(".wsc");
	file.open(fileName.c_str(), ios::in);
	// First line in file is version
	file >> _version;
	ISerialization::prepareReading(ident);
}

void serTextFile::prepareWritting(std::string ident)
{
	std::string fileName(ident);
	fileName.append(".wsc");
	file.open(fileName.c_str(), ios::out);
	// First line in file is version
	file << _version << endl;
	ISerialization::prepareWritting(ident);
}

void serTextFile::close()
{
	file.close();
}

void serTextFile::writeObjectID(string data)
{
	file << "SECTION " << data << endl;
}

void serTextFile::write(std::string Key, std::string &data)
{
	file << Key << " " << data << endl;
}

void serTextFile::write(std::string Key, unsigned int data)
{
	file << Key << " " << data << endl;
}

void serTextFile::write(std::string Key, signed int data)
{
	file << Key << " " << data << endl;
}

void serTextFile::write(std::string Key, unsigned char data)
{
	file << Key << " " << data << endl;
}

void serTextFile::write(std::string Key, signed char data)
{
	file << Key << " " << data << endl;
}

void serTextFile::write(std::string Key, bool data)
{
	file << Key << " " << data << endl;
}

void serTextFile::doneWritting()
{

}

void serTextFile::readObjectID(string &data)
{
	char buffer[256];
	file.getline(buffer, 255, ' ');
	file.getline(buffer, 255);
	data = buffer;
}

void serTextFile::read(std::string Key, string& data)
{
	char buffer[256];
	file.getline(buffer, 255, ' '); // first is key;
	file.getline(buffer, 255);
	data = buffer;
}

void serTextFile::read(std::string Key, unsigned int  &data)
{
	char buffer[256];
	file.getline(buffer, 255, ' '); // first is key;
	file >> data;
}

void serTextFile::read(std::string Key, signed   int  &data)
{
	char buffer[256];
	file.getline(buffer, 255, ' '); // first is key;
	file >> data;
}

void serTextFile::read(std::string Key, unsigned char &data)
{
	char buffer[256];
	file.getline(buffer, 255, ' '); // first is key;
	file >> data;
}

void serTextFile::read(std::string Key, signed   char &data)
{
	char buffer[256];
	file.getline(buffer, 255, ' '); // first is key;
	file >> data;
}

void serTextFile::read(std::string Key, bool &data)
{
	char buffer[256];
	file.getline(buffer, 255, ' '); // first is key;
	file >> data;
}
