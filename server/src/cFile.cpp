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

#include "wolfpack.h"
#include "hFile.h"
#include "debug.h"

#undef  DBGFILE
#define DBGFILE "CFile.cpp"

bool cFile::Open(char FileName[20], char type[5]) //type: ASCII=0   BINARY=1
{	
	if((OpenedFile = fopen(FileName, type))==NULL)
	{
		clConsole.send("Error: Opening: %s\n", FileName);
		return false;
	}
	fseek(OpenedFile, 0, SEEK_SET);
	isOpen=true;
	return true;
}

void cFile::Close()
{
	if(OpenedFile==NULL)
		return;	
	fclose(OpenedFile);
	OpenedFile = NULL;
	isOpen=false;
}
	
void cFile::Read2()
{
	int i, keep=0,loopexit=0;
	char c;
	char temp[1024]={' '};
	while(!keep && (++loopexit < MAXLOOPS) )
	{
		i=0;
		if (feof(OpenedFile)) return;
		c=(char)fgetc(OpenedFile);
		while (c!=10)
		{
			if (c!=13 && c!='[' && c!=']')
			{
				temp[i]=c;
				i++;
			}
			if (feof(OpenedFile)) return;
			c=(char)fgetc(OpenedFile);
		}
		temp[i]=0;
		keep=1;
		if (temp[0]=='/' && temp[1]=='/') keep=0;
		if (temp[0]==0) keep=0;
	}
	i=0;
	value1[0]=0;
	value2[0]=0;

	while(temp[i]!=0 && temp[i]!='=' && i<1024 )
	{
		i++;
	}

	strncpy(value1, temp, i);
	value1[i]=0;
	if (value1[0]!='[' && temp[i]!=0) strcpy(value2, temp+i+1);
	return;
}

