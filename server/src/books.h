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

#if !defined(__BOOKS_H__)
#define __BOOKS_H__




// Book class by Lord Binary, 7'th december 1999

// new books readonly -> morex  999
// new books writeable -> morex 666
// old books from misc.scp, readonly: neither type 666 nor 999, section# = more-value
// of course ALL books need a type of 11

// note to everyone who wants to learn C++ -> this is NOT stricly OOP and has no good OOD.
// hence no real good example for OO-P :)
// for example inheritence could be used, public attributes arnt nice etc,etc.


// Platform Specifics
#include "platform.h"

// System Includes
#include <iostream>

using namespace std ;

// Forward class declaration
class cBooks ;

// Wolfpack Includes
#include "wolfpack.h"
#include "debug.h"

// Class defination

//##ModelId=3C5D932B03E5
class cBooks
{
	private:
		// that private methods read and write from the *.bok files
	//##ModelId=3C5D932C0025
		void write_author(P_ITEM pBook,UOXSOCKET s);
	//##ModelId=3C5D932C0043
		void write_title(P_ITEM pBook, UOXSOCKET s);
	//##ModelId=3C5D932C0061
		void write_line(P_ITEM pBook, int page, int line, char linestr[34], UOXSOCKET s);
		
	//##ModelId=3C5D932C0094
		void read_author(P_ITEM pBook,char auth[31]);
	//##ModelId=3C5D932C00E3
		void read_title(P_ITEM pBook,char title[61]);
	//##ModelId=3C5D932C0101
		int  read_number_of_pages(P_ITEM pBook);
	//##ModelId=3C5D932C0115
		void read_line(P_ITEM pBook, int page,int linenumber, char line[33]);

	//##ModelId=3C5D932C0179
		signed char make_new_book_file(char *fileName, P_ITEM pBook); // "formats and creates a new bok file"
					
	public:
	//##ModelId=3C5D932C0237
		char a_t; // flag -> set if author and title changed		
	//##ModelId=3C5D932C0255
		char authorbuffer[MAXCLIENT][32]; 
	//##ModelId=3C5D932C0291
		char titlebuffer[MAXCLIENT][62];
	//##ModelId=3C5D932C02B0
		char pagebuffer[MAXCLIENT][512]; //i think 256 is enough (8 lines *32 chars per line = 256, but i took 512 to be on the safe side and avoid crashes 
	
	//##ModelId=3C5D932C02C4
	    cBooks();

	//##ModelId=3C5D932C02CE
        void openbook_old(UOXSOCKET s, P_ITEM pBook); // opens old-readonly books, takes data from misc.scp
		
	//##ModelId=3C5D932C02E2
		void openbook_new(UOXSOCKET s, P_ITEM pBook,char writeable); // opens new books

	//##ModelId=3C5D932C0300
		void readbook_readonly_old(UOXSOCKET s, P_ITEM pBook, int p); // reads books from misc.scp, readonly = old books
		                                                        
	//##ModelId=3C5D932C031F
		void readbook_readonly(UOXSOCKET s, P_ITEM pBook, int p);     // reads new books readonly ( from *.bok file )
		                                                     		                                                      
	//##ModelId=3C5D932C033D
		void readbook_writeable(UOXSOCKET s, P_ITEM pBook, int p, int l); // writes changes to a new book opened in writable mode 

	//##ModelId=3C5D932C0364
		void delete_bokfile(P_ITEM pBook); // deletes bok-file.

	//##ModelId=3C5D932C0378
		virtual ~cBooks();
        	
};
#endif
