//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001-2003 by holders identified in authors.txt
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
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================

#ifndef __VERINFO_H
#define __VERINFO_H

// Wolfpack Includes
#include "platform.h"

//System Includes
#include <string>


// Library Include
#include <qstring.h>

///// do not chang those

#define WPV_RELEASE    0x01
#define WPV_INERNAL    0x02 
#define WPV_BETA       0x04
#define WPV_SPECIAL    0x08
#define WPV_MAPILSH    0x10 //  if not set BRIT map else ILSH
#define WPV_REQXWOLF   0x20
#define WPV_GUI        0x40
#define WPV_NONDEVTEAM 0x80

#define WOLF      0x0
#define NOXWIZARD 0x1
#define AVANOON   0x2
#define UOX3      0x3 // if they add wip one day

/// end of do not change

/////////////////////////////////////////////////////////////////////////////////////
/// purpose of 32 bit version encoding: external tools that use WIP like X-WOLF 
/// being able to detect wolfpack version+variant correctly
////////////////////////////////////////////////////////////////////////////////////

/*

  Wolfpack Version: - 32 bit encoding ( vernumber in version struct )

  g5                 g4                g3             g2               g1         
u u u u       u u u u u u u u     u u u u u u    u u u u u u u    u u u u u u u
(28-31)           (20-27)           (14-19)         (7-13)             (0-6)


g1 (bit 0-6)    ->  major version, 0...127

g2 (bit 7-13)   ->  subVersion, 0..126, 127: no subversion

g3 (bit 14-19)  ->  subsubverison, 0..62, 63: no subsubverison

g4: flags

0x01 -> supported / official release
0x02 -> internal
0x04 -> beta
0x08 -> special
0x10 -> ILSHENAR or BRITANNIA MAP SERVER ? (0x10 -> ILSH, else BRIT)
0x20 -> requires X-Wolf if windows version of server
0x40 -> GUI version
0x80 -> nonofficial (if not set -> official wolfpack team version, if set non official team version)

of course ALL can be combined.

g5: Wolfpack derivates

0 -> official wolfpack
1 -> noxwizard
2 -> avanoon
3 -> UOX3 if it adds WIP in future. not likely though :>

4..15: open


*/

/// note: no code for offical encoding yet, but will be added soon, LB 30-Sept 2001

struct wpversion_32bit
{
	unsigned char major_version;
    unsigned char sub_version;
	unsigned char subsub_version;
	unsigned char flags;
	unsigned char derivate;
	wpversion_32bit() : flags(0) { ; }

};


// changed all the #defines dealing with verison stuff to a struct, LB
struct wp_version_info
{
    public:

	   unsigned int    vernumber;
	   wpversion_32bit verstruct;
	   std::string verstring; 
	   std::string productstring;
	   std::string betareleasestring;
	   QString clientsupportedstring;
	   std::string timezonestring;
	   std::string compiledbystring;
	   std::string codersstring;
	   std::string emailstring;

	   wp_version_info()
	   {
		   /////////////////////////////////////

		   verstruct.major_version  = 12;
		   verstruct.sub_version    = 9;
		   verstruct.subsub_version = 3; // no subsub

		   verstruct.flags    = WPV_RELEASE;
		   verstruct.derivate = WOLF;
		  
           vernumber = to_32bitver(verstruct);		   
		  
		   /// code for auto generation of verstring, productstring and betareleasestring coming soon (LB) ...
		   verstring = "12.9.3";  // Wolfpack version 
		   productstring = "Wolfpack"; // selfexplainatory, isnt't it :)
		   betareleasestring = "Beta"; // release status

		   //////////////////////////////////////////////////

		   clientsupportedstring = "3.0.8z";

		   // SUPPORTED_CLIENT is part of the client version control system.
           // this is NOT necassairily the lastest client.
           // don't change it unless you are sure that its packets structure is in synch with network.cpp etc.
           // until I (LB) don't leave the wolfpack project, I want to be the only(!) one who
           // changes that value. thx for your understanding =:)		 

		   timezonestring = "GMT+1";   // your timezone

		   #ifdef __unix__
             compiledbystring = "Wolfpack" ;	// person that compiled linux version
           #else
             compiledbystring = "Wolfpack" ;	// person that compiled windows version
           #endif
           codersstring = "Wolfpack Developers"; // Keep in alphabetic order.

           emailstring = "http://www.wpdev.org/";		  
	   }

	   virtual ~wp_version_info() { ; }

	   private:
       unsigned int to_32bitver(wpversion_32bit data)
	   {
	       unsigned int result, r1,r2,r3,r4,r5;	
           r1 =  ( (unsigned int) ( data.major_version  & 0x7f ) ) << 0;
	       r2 =  ( (unsigned int) ( data.sub_version    & 0x3f ) ) << 7;
	       r3 =  ( (unsigned int) ( data.subsub_version & 0x7f ) ) << 13;
	       r4 =  ( (unsigned int) ( data.flags          & 0xff ) ) << 20;   
           r5 =  ( (unsigned int) ( data.derivate       & 0x0f ) ) << 28;		
	       result = r1+r2+r3+r4+r5 ;
	       return result;
	   }


       wpversion_32bit from_32bitver( unsigned long int indata)
	   {
	       wpversion_32bit wpv32;
	       wpv32.major_version  = static_cast<UI08> ( (indata & 0x0000007f) >> 0);
	       wpv32.sub_version    = static_cast<UI08> ( (indata & 0x00001fc0) >> 7);
	       wpv32.subsub_version = static_cast<UI08> ( (indata & 0x000fe000) >> 13);
	       wpv32.flags          = static_cast<UI08> ( (indata & 0x0ff00000) >> 20);
	       wpv32.derivate       = static_cast<UI08> ( (indata & 0xf0000000) >> 28);
	       return wpv32;
	   }

};
extern wp_version_info wp_version;

#endif
