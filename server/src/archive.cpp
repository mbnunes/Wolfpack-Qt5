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


#include "archive.h"

#undef  DBGFILE
#define DBGFILE "archive.cpp"


// Author: Dupois 
// Rewritten by Lord Binary, fixed a crash when using it in  Unix enviroments, 
// and adopted it to Zippy's new file format
// the filenames MUST have be of the form x.y
// but this is the case,cause the files are wpitems.wsc and wppcs.wsc
   
void fileArchive(char *pFile2Archive_chars, char *pFile2Archive_items, char *pArchiveDir)
{
	char ext[255] = "";
	char ext2[255] = "";
	char *pext = NULL;
	char *pext2 = NULL;
    char destfile[255]="";
	char destfile2[255]="";
	time_t mytime;
	tm *ptime;


    /*************** chars archiving ************************/

	char ch = '.';
	int p=0,loopexit=0;

	// Determine the extension of the file and save the extension into array ext

	pext = (strchr(pFile2Archive_chars, ch)); //charsfile extension
	if (pext != NULL)
	{
		while ( (pext[p] != '\0') && (++loopexit < MAXLOOPS) )
		{
			ext[p]=pext[p];
			p++;
		}
	}

	p=0; loopexit=0;

	pext2 = (strchr(pFile2Archive_items, ch)); // itemfile extension
	if (pext2 != NULL)
	{
		while ((pext2[p] != '\0') && (++loopexit < MAXLOOPS) )
		{
			ext2[p]=pext2[p];
			p++;
		}
	}


	// Create the destination file name

	// charsfile string
	strcpy(destfile, pArchiveDir);	
	p=strlen(destfile);
	if (destfile[p-1]!='\\') { destfile[p]='\\';destfile[p+1]=0; }
	strcat(destfile, pFile2Archive_chars);
	strcat(destfile, "-");

	// itemsfile string
	strcpy(destfile2, pArchiveDir);
	p=strlen(destfile2);
	if (destfile2[p-1]!='\\') { destfile2[p]='\\';destfile2[p+1]=0; }
	strcat(destfile2, pFile2Archive_items);
	strcat(destfile2, "-");

    time(&mytime);
	ptime = localtime(&mytime); // get local time
	char *timenow = asctime(ptime);   // convert it to a string
    
	for (unsigned int a = 0; a < strlen(timenow); a++) 
		if (timenow[a]==' ' || timenow[a]=='\n' || timenow[a]==':') timenow[a]='-'; // overwritng the characters that arnt allowed in paths

	strcat(destfile, timenow);
	strcat(destfile, ext);

	strcat(destfile2,timenow);
	strcat(destfile2,ext2);

	// Rename/Move the pFile2Archive to destfile

	if( rename( pFile2Archive_chars, destfile ) != 0 )
	{
		sprintf((char*)temp,"warning-could not rename/move file.\nFile '%s' or directory '%s' may not exist.\n", pFile2Archive_chars, pArchiveDir); 
		LogWarning( (char*)temp );
	}
	else
		clConsole.send( "WOLFPACK: '%s' renamed/moved to '%s'\n", pFile2Archive_chars, destfile );


    if( rename( pFile2Archive_items, destfile2 ) != 0 )
	{
		sprintf((char*)temp, "warning-could not rename/move file.\nFile '%s' or directory '%s' may not exist.\n", pFile2Archive_items, pArchiveDir );
		LogWarning( (char*)temp );
	}
	else
		clConsole.send( "WOLFPACK: '%s' renamed/moved to '%s'\n", pFile2Archive_items, destfile2 );


	clConsole.send("finished backup\n");

	return;
}
