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

// Books class by Lord Binary 7'th December 1999
// Implements writable books

// V0.9 5'th dec, initial version
// V1.0 7'th dec 1999, "wrapped" everything in a class, added customable number of max-pages
// V1.1 12-dec-1999 -> nasty bug fixed (item# used as "key" instead of serial#) and a few very small bugfixes

#include "books.h"
#include "network.h"
#include "scriptc.h"

#undef DBGFILE
#define DBGFILE "books.cpp"

cBooks::cBooks()//Constructor
{
	return;
}

cBooks::~cBooks()//Destructor
{

}

// opens old (readonly) books == old, bugfixed readbook function
void cBooks::openbook_old(UOXSOCKET s, P_ITEM pBook)
{
	unsigned char bookopen[10]="\x93\x40\x01\x02\x03\x00\x00\x00\x02"; //LB 7'th dec 1999, making it client 1.26 complaint
	unsigned char booktitle[61]="\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
	unsigned char bookauthor[31]="\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
	unsigned long loopexit=0;
	
	openscript("misc.scp");
	sprintf((char*)temp, "BOOK %i",	calcserial(pBook->more1, pBook->more2, pBook->more3, pBook->more4));
	if (!i_scripts[misc_script]->find((char*)temp))
	{
		closescript();
		return;
	}
	LongToCharPtr(pBook->serial, &bookopen[1]);
	do
	{
		read2();
	}
	while ( (strcmp((char*)script1, "PAGES")) && (++loopexit < MAXLOOPS) );
	bookopen[8] = (char) str2num(script2); // LB, bugfixing old code

	loopexit=0;
	do
	{
		read2();
	}
	while ( (strcmp((char*)script1, "TITLE")) && (++loopexit < MAXLOOPS) );

	strcpy((char*)(booktitle), script2);

	loopexit=0;
	do
	{
		read2();
	}
	while ( (strcmp((char*)script1, "AUTHOR")) && (++loopexit < MAXLOOPS) );

	strcpy((char*)(bookauthor), script2);
	Xsend(s, bookopen, 9); // LB, bugfixing of old code
	Xsend(s, booktitle, 60);
	Xsend(s, bookauthor, 30);
	closescript();
}

// opens new books
// writeable 1 -> open new books in writable mode
//           0 -> open new books in readonly mode
void cBooks::openbook_new(UOXSOCKET s, P_ITEM pBook, char writeable)
{
	unsigned char bookopen[10]= "\x93\x40\x01\x02\x03\x01\x01\x00\x02"; //LB 7'th dec 1999, making it client 1.26 complaint
	
	int a,b;
	short pages, bytes;
	char line[33];
	char buch[256][8][33];
	char fileName[13];
	bool bookexists = false;

	FILE *file;

	// I dont know what that new client 1.26 byte does, but it needs to be set to 1 or 2 or writing doesnt work
    // wild guess: rune books ...

	char booktitle[61]={0x00,};
	char bookauthor[31]={0x00,};

	sprintf( fileName, "%8x.bok", pBook->serial);
    file = fopen( fileName, "r+b"); // open existing file for read/write

    bookexists = (file!=NULL);

	if (bookexists)
	{
		fclose(file);
		file = NULL;
		read_author ( pBook, bookauthor ); // fetch author if existing
		read_title  ( pBook, booktitle  ); // fetch title if existing
		pages = read_number_of_pages(pBook);
	}
	else 
	{ 
		pages = pBook->morey;              // if new book get number of maxpages ...
		if (pages<1 || pages>255) 
			pages = 16;
	}
	
	// clear all buffers from previous book openings
	memset( &authorbuffer[s], '~', 32 );
	memset( &titlebuffer[s], '~', 62 );
	memset( &pagebuffer[s], '~', 511 );
	
	LongToCharPtr(pBook->serial, &bookopen[1]);
	ShortToCharPtr(pages, &bookopen[7]);

	if (writeable) 
		bookopen[5] = 1;
	else
		bookopen[5] = 0;

	Xsend(s, bookopen, 9);
	Xsend(s, booktitle, 60);
	Xsend(s, bookauthor, 30);
	
	if (!bookexists) return; // dont send book contents if the file doesnt exist (yet)!

	if (!writeable) 
		return; // if readonly book return !!

    //////////////////////////////////////////////////////////////
	// Now we HAVE to send the ENTIRE Book                     / /
	// Cauz in writeable mode the client only sends out packets  /
	// if something  gets changed                                /
	// this also means -> for each bookopening in writeable mode /
	// lots of data has to be send.                              /
	//////////////////////////////////////////////////////////////

	unsigned char bookpage_pre[10]="\x66\x01\x02\x40\x01\x02\x03\x00\x01";
	unsigned char bookpage[5]="\x00\x00\x00\x08";

    // we have to know the total size "in advance"
	// thats why i save the book data in a temporaray array to 
	// avoid reading it twice from (slow) hd

	bytes = 9;
	for (a = 1; a <= pages; a++)
	{
		bytes+=4; // 4 bytes for each page
		for (b=1;b<=8;b++)
		{
			read_line(pBook, a,b, line);
			strcpy(buch[a-1][b-1],line);
			bytes += static_cast<short>(strlen(line)+1); // plus the stringlength+null terminator per(!) row
		}
	}
	
	ShortToCharPtr(bytes, &bookpage_pre[1]);
	LongToCharPtr(pBook->serial, &bookpage_pre[3]);
	ShortToCharPtr(pages, &bookpage_pre[7]);

	Xsend(s, bookpage_pre, 9);

	for (a=1;a<=pages;a++)
	{
		
		ShortToCharPtr(a, &bookpage[0]);
		
		Xsend(s, bookpage, 4);
		
		for (int j=0;j<8;j++)
		{
			Xsend(s, buch[a-1][j], strlen(buch[a-1][j])+1);
		}
	}
}

// sends a page of new readonly book to the client
void cBooks::readbook_readonly(UOXSOCKET s, P_ITEM pBook, int p) 
{

    unsigned char bookpage[14]="\x66\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x08";
	int bytes=0,a,c;
	char seite[8][33];
	char fileName[13];
	char line[33];
	FILE *file = NULL;
	
	sprintf( fileName, "%8x.bok", pBook->serial);
    file = fopen( fileName, "r+b"); // open existing file for read/write

    if (file == NULL) 
		return;
	else
	{
		fclose(file);
		file = NULL;
	}

	bytes=13;
	for (a=1;a<=8;a++)
	{
		read_line(pBook, p,a,line);
		c=strlen(line)+1;        
		strcpy(seite[a-1],line);
		bytes+=c;
	}
    
	bookpage[1]=bytes>>8;
	bookpage[2]=bytes%256;

	LongToCharPtr(pBook->serial, &bookpage[3]);

	bookpage[9]=p>>8;
	bookpage[10]=p%256;

	Xsend(s, bookpage, 13);

	for (int j=0;j<8;j++)
	{
		Xsend(s, seite[j], strlen(seite[j])+1);
	}
	
}


// old readbook function
void cBooks::readbook_readonly_old(UOXSOCKET s, P_ITEM pBook, int p)
{
	int x, y, pos, j;
	unsigned char bookpage[14]="\x66\x01\x02\x40\x01\x02\x03\x00\x01\x00\x01\x00\x01";
	unsigned long loopexit=0,loopexit2=0;
	
	openscript("misc.scp");
	sprintf((char*)temp, "BOOK %i", calcserial(pBook->more1, pBook->more2, pBook->more3, pBook->more4));
	if (!i_scripts[misc_script]->find((char*)temp))
	{
		closescript();
		return;
	}
	x=p;

	do
	{
		loopexit=0;
		do
		{
			read2();
		}
		while ( (strcmp((char*)script1, "PAGE")) && (++loopexit < MAXLOOPS) );
		x--;
	}
	while ( (x>0) && (++loopexit2 < 6666 ));

	closescript();
	openscript("misc.scp");
	sprintf((char*)temp, "PAGE %s", script2);
	if (!i_scripts[misc_script]->find((char*)temp))
	{
		closescript();
		return;
	}
	pos=ftell(scpfile);
	x=-1;
	y=-2;
	loopexit=0;
	do
	{
		read1();
		x++;
		y+=strlen((char*)script1)+1;
	}
	while ( (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS) );
	y+=13;
	fseek(scpfile, pos, SEEK_SET);
	bookpage[1]=y>>8;
	bookpage[2]=y%256;
	LongToCharPtr(pBook->serial, &bookpage[3]);
	bookpage[9]=p>>8;
	bookpage[10]=p%256;
	bookpage[11]=x>>8;
	bookpage[12]=x%256;
	Xsend(s, bookpage, 13);
	for (j=0;j<x;j++)
	{
		read1();
		Xsend(s, script1, strlen((char*)script1)+1);
	}
	closescript();
}

// writes changes to a writable book to the bok file.		
void cBooks::readbook_writeable(UOXSOCKET s, P_ITEM pBook, int p, int l)
{
	int ii=0,lines_processed=0,lin=0;
	char line[34],ch;
	unsigned long loopexit=0;
	
	if (a_t) write_title(pBook,s); // if title was changed by writer write the changes "down"
	if (a_t) write_author(pBook,s); // if author was changed by writer write the changes "down" to the bok-file
	
	while (lines_processed<l && (++loopexit < MAXLOOPS) )
	{
		if (ii>511) lines_processed=l; // avoid crash if client sends out inconsitent data
		ch=pagebuffer[s][ii];
		if (lin<33) line[lin]=ch; else line[33]=ch;
		ii++;
		lin++;
		if (ch==0) 
		{
			lines_processed++;
			lin=0;
			
			write_line(pBook, p, lines_processed, line,s);
		}
	}
	
	a_t=0; // dont re-write author and title if not necassairy
}

// private methods here

// writes the author into the corresponding-bok file
// PRE: packets 0x93 needs to be send by client BEFORE its called. 
// (and its data copied to the authorbuffer)

void cBooks::write_author(P_ITEM pBook,UOXSOCKET s)
{
	FILE *file = NULL;
	char fileName[13];  // Standard 8.3 file name
	int newbook=0,Offset;
	
	sprintf( fileName, "%8x.bok", pBook->serial);
	
	file = fopen( fileName, "r+b"); // open existing file for read/write
	
	if (file == NULL) // If the BOK file does not exist -> that book must be new
		// or the file got deleted -> lets try to create it
	{
		newbook=1;
		if (make_new_book_file(fileName,pBook)==-1)
		{
			return;
		}
		
		file = fopen( fileName, "r+b"); // open existing file for read/write (now it should exist)
		if (file==NULL)                 
		{
			strcpy((char*)temp,"couldnt write to bok file\n");
			LogError((char*)temp);
			return;
		}
	}
	
	Offset=62; // position filepointer to the author-place
	if ( fseek(file, Offset, SEEK_SET) ) LogError("failed to seek to bok file\n");
	
	authorbuffer[s][31]='\n';
	
	if ( fwrite(authorbuffer[s], sizeof(char), 32, file) != 32 ) 
	{
		strcpy((char*)temp,"coudnt write to book file\n");
		LogError((char*)temp);
		return;
	}			
	
	fclose(file);
	file = NULL;
	
}

void cBooks::write_title(P_ITEM pBook,UOXSOCKET s)
{
	FILE *file;
	char fileName[13];  // Standard 8.3 file name
	int newbook=0,Offset;
	
	sprintf( fileName, "%8x.bok", pBook->serial);
	
	file = fopen( fileName, "r+b"); // open existing file for read/write
	
	
	if (file == NULL) // If the BOK file does not exist -> that book must be new
		// or the file got deleted -> lets try to create it
	{
		newbook=1;
		if (make_new_book_file(fileName,pBook)==-1) 
			
		{
			return;
		}
		
		file = fopen( fileName, "r+b"); // open existing file for read/write (now it should exist)
		if (file==NULL)                 
		{
			strcpy((char*)temp,"couldnt write to bok file\n");
			LogError((char*)temp);
			return;
		}
	}
	
	Offset=0; // position filepointer to the title-place
	if ( fseek(file, Offset, SEEK_SET) ) LogError("failed to seek to bok file\n");
	
	titlebuffer[s][61]='\n';
	
	if ( fwrite(titlebuffer[s], sizeof(char), 62, file) != 62 ) 
	{
		strcpy((char*)temp,"coudnt write to book file\n");
		LogError((char*)temp);
		return;
	}			
	
	fclose(file);
	file = NULL;
}

void cBooks::write_line(P_ITEM pBook, int page, int line, char linestr[34], UOXSOCKET s)
{
	
	FILE *file;
	char fileName[13];  // Standard 8.3 file name
	int newbook=0,Offset;
	
	sprintf( fileName, "%8x.bok", pBook->serial);
	
	file = fopen( fileName, "r+b"); // open existing file for read/write
	
	if (file == NULL) // If the BOK file does not exist -> that book must be new
		// or the file got deleted -> lets try to create it
	{
		newbook=1;
		
		if (make_new_book_file(fileName,pBook)==-1) 
			
		{
			return;
		}
		
		file = fopen( fileName, "r+b"); // open existing file for read/write (now it should exist)
		if (file==NULL)                 
		{
			strcpy((char*)temp,"couldnt write to bok file\n");
			LogError((char*)temp);
			return;
		}
	}
	
	Offset=273*page+34*line-207; // wohoooo, what a neat geeky formula :)
	
	if ( fseek(file, Offset, SEEK_SET) ) LogError("failed to seek to bok file");
	
	linestr[33]='\n';
	
	if ( fwrite(linestr, sizeof(char), 34, file) != 34 ) 
	{
		LogError("coudnt write to book file\n");
		return;
	}			
	
	fclose(file);
	file = NULL;
}

void cBooks::read_author(P_ITEM pBook,char auth[31])
{
	
	FILE *file;
	char fileName[13];  
	int Offset;
	
	sprintf( fileName, "%8x.bok", pBook->serial);
	
	file = fopen( fileName, "r+b"); // open existing file for read/write
	
	if (file == NULL) 
	{
		LogError("couldnt read bok file\n");
		return;
	}
	
	Offset=62; // position filepointer to the author-place
	if ( fseek(file, Offset, SEEK_SET) ) LogError("failed to seek to bok file\n");
	
	if ( fread(auth, sizeof(char), 31, file) != 31 )  // read it
	{
		LogError("coudnt write to book file\n");
		return;
	}
	
	// clear garbage after strign termination
	int end=0; 
	for (int a=0;a<31;a++)
	{
		if (auth==0) end=1;
		if (end) auth[a]=0;
	}
	
	fclose(file);
	file = NULL;
}

void cBooks::read_title(P_ITEM pBook,char title[61])
{
	FILE *file;
	char fileName[13];  
	int Offset;
	
	sprintf( fileName, "%8x.bok", pBook->serial);
	
	file = fopen( fileName, "r+b"); // open existing file for read/write
	
	if (file == NULL) 
	{
		LogError("couldnt read bok file\n");
		return;
	}
	
	Offset=0; // position filepointer to the title place
	if ( fseek(file, Offset, SEEK_SET) ) LogError("failed to seek to bok file\n");
	
	if ( fread(title, sizeof(char), 61, file) != 61 )  // read it
	{
		LogError("coudnt write to book file\n");
		return;
	}
	
	// clear garbage after strign termination
	int end=0; 
	for (int a=0;a<61;a++)
	{
		if (title==0) end=1;
		if (end) title[a]=0;
	}
	
	fclose(file);
	file = NULL;
}

int cBooks::read_number_of_pages(P_ITEM pBook)
{
	FILE *file;
	char fileName[13];  
	int Offset;
	char num[5];
	
	sprintf( fileName, "%8x.bok", pBook->serial);
	
	file = fopen( fileName, "r+b"); // open existing file for read/write
	
	if (file == NULL) 
	{
		LogError("couldnt read bok file\n");
		return 1;
	}
	
	Offset=94; // position filepointer to the number of pages place
	if ( fseek(file, Offset, SEEK_SET) ) LogError("failed to seek to bok file\n");
	
	if ( fread(num, sizeof(char), 5, file) != 5 )  // read it
	{
		LogError("coudnt write to book file\n");
		return 1;
	}
	
	// clear garbage after string termination
	int end=0;
	int a ; 
	for (a=0;a<5;a++)
	{
		if (num==0) end=1;
		if (end) num[a]=0;
	}
	
	fclose(file);
	file = NULL;
	
	a=str2num(num);
	if (a<1 || a>255) return 255;
	return a;
}
	

// page+linumber=1 indexed ! (as returned from client)
void cBooks::read_line(P_ITEM pBook, int page,int linenumber, char line[33])
{
	FILE *file;
	char fileName[13];  
	int Offset;
	
	sprintf( fileName, "%8x.bok", pBook->serial);
	
	file = fopen( fileName, "r+b"); // open existing file for read/write
	
	if (file == NULL) 
	{
		LogError("couldnt read bok file\n");
		return;
	}
	
	Offset=273*page+34*linenumber-207; // wohoooo, what a neat geeky formula :)
	if ( fseek(file, Offset, SEEK_SET) ) LogError("failed to seek to bok file\n");
	
	if ( fread(line, sizeof(char), 33, file) != 33 )  // read it
	{
		LogError("coudnt write to book file\n");
		return;
	}
	
	// clear garbage after strign termination
	int end=0; 
	for (int a=0;a<33;a++)
	{
		if (line==0) end=1;
		if (end) line[a]=0;
	}
	
	fclose(file);
	file = NULL;
}

void cBooks::delete_bokfile(P_ITEM pBook)
{
	char fileName[13];    
	sprintf( fileName, "%8x.bok", pBook->serial);  
	remove(fileName);
}

// "Formats" a newly created book-file
// This NEEDS to be done with ANY new book file.

signed char cBooks::make_new_book_file(char *fileName, P_ITEM pBook)
{
	FILE *file;
	
	file = fopen( fileName, "w+b"); // create new file
	int i,maxpages;
	char ch;
	char author[33];
	char title[63];
	char line[35];
	char num[5];
	
	if (file == NULL) 
	{
		LogError("cant create new book file\n");
		return-1;
	}
	
	author[0]='.';author[1]=0;author[31]='\n';
	title[0]='.';title[1]=0;title[61]='\n';
	line[0]='.';line[1]=0;line[33]='\n';
	
	for (i=2;i<=60;i++) title[i]=32;
	for (i=2;i<=30;i++) author[i]=32;
	for (i=2;i<=32;i++) line[i]=32;
	
	if ( fwrite(&title, sizeof(char), 62, file) != 62 ) 
	{
		LogError("coudnt write to book file\n");
		return -1;
	}
	
	if ( fwrite(&author, sizeof(char), 32, file) != 32 ) 
	{
		LogError("coudnt write to book file\n");
		return -1;
	}
	
	maxpages=pBook->morey; // morey=maxpages
	if (maxpages<1 || maxpages>255) maxpages=16; // default = 16 pages
	
	numtostr(maxpages,num); // danger,danger, didnt this cause probs under LINUX ???
	// sorry, i cant test ???
	
	num[4]='\n';
	
	if ( fwrite(num, sizeof(char), 5, file) != 5 )  // writens number
	{
		LogError("coudnt write to book file\n");
		return -1;
	}
	
	for (int j=0;j<maxpages;j++) // page loop
	{
		ch='\n'; // each page gets a cr
		if ( fwrite(&ch, sizeof(char), 1, file) != 1 ) 
		{
			LogError("coudnt write to book file\n");
			return -1;
		}
		
		for (int l=0;l<8;l++) // each page has 8 lines
		{	         
			line[0]=0;
			if ( fwrite(&line, sizeof(char), 34, file) != 34 ) 
			{
				LogError("coudnt write to book file\n");
				return -1;
			}			
		}
		
	}
	
	fclose(file);
	return 0;
}

