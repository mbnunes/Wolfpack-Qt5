//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//      Copyright 2001 by holders identified in authors.txt
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
#if !defined(__SECTION_H__)
#define __SECTION_H__

// Platform specifics
#include "platform.h"


// System Includes
#include <iostream>
#include <list>
#include <string>

using namespace std ;


// Third Party includes



//Forward class declaration

class Section_cl ;

//Wolfpack Includes



//Class definitions
class Section_cl
{
public:
        /// Constructor
        Section_cl()   ;
        /// Cosntructor of itself
        Section_cl(const Section_cl& clData) ;
        /// Desctructor
        ~Section_cl() ;

        // Return the number in the list
        SI32 size() ;

        // Set the offset into the list
        void offset(UI32 siOffset) ;

        // Pop an element and increment offset

        string pop() ;
        /// Operator=
        Section_cl&    operator= (const Section_cl& clSection_cl) ;

        /// Clear out the list
        void clear() ;

		/// Insert an element into the Section_cl (goes at the end)
		void insert(string sInput) ;

private:
        // At the moment, since we dont parse the data, we do a straightforward copy
        list<string>    lisData ;

        list<string>::iterator iterData ;


};
//==========================================================================================

#endif



