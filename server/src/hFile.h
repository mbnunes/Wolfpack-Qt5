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

#ifndef _H_FILE
#define _H_FILE

//##ModelId=3C5D92F10297
class cFile
{
public:
	//##ModelId=3C5D92F102BF
	cFile()
	{
		isOpen=false;
	}
	//##ModelId=3C5D92F102C0
	bool isOpen;
	//##ModelId=3C5D92F102D3
	char value1[512];
	//##ModelId=3C5D92F102E7
	char value2[512];
	//##ModelId=3C5D92F102FB
	bool Open(char FileName[50], char type[5]);	//type: enter the ios format, example "r+t", "r+b", etc.
	//##ModelId=3C5D92F1030F
	void Close();
	//##ModelId=3C5D92F10319
	void Read2();
	//##ModelId=3C5D92F10323
	FILE *OpenedFile;
};

//extern cFile *File;
#endif

