//========================================================================
//FILE: mstring.h
//========================================================================
//	Copyright (c) 2001 by Sheppard Norfleet and Charles Kerr
//  All Rights Reserved
// 
//	Redistribution and use in source and binary forms, with or without
//	modification, are permitted provided the following conditions are met:
//
//	Redistributions of source code must retain the above copyright notice,
//	this list of conditions and the following disclaimer. Redistributions
//	in binary form must reproduce the above copyright notice, this list of
//	conditions and the following disclaimer in the documentation and/or
//	other materials provided with the distribution.
//
//	Neither the name of the SWORDS  nor the names of its contributors may
//	be used to endorse or promote products derived from this software
//	without specific prior written permission. 
//
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  `AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//	A PARTICULAR PURPOSE ARE DISCLAIMED. .
//========================================================================
//Directives
#ifndef	IN_MSTRING_H
#define IN_MSTRING_H
//========================================================================
//Platform Specs
#include "platform.h"
//========================================================================
//System Files
#include <iosfwd>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
using namespace std;
//========================================================================
// Third party includes

// Forward class declaration
class mstring ;

// SWORDS Includes
#include "whitespace.h"
#include "stream.h" 
///CLASSDEF:



class mstring : public string
{
public:
//	Constructors
	/// Blank Constructor
	mstring() ;
	/// Constructor that is a copy of the existing string
	mstring(const string&) ;
	
	// Constructor that initialise based on size (broken on msvc6)
	//mstring(const string& clString, UI32 uiIdx) ;
	mstring(const string& clString, UI32 uiIdx, UI32 uiLength) ;
	///Constructor that is populated from the null terminated char string
	mstring(char*);
	///Creates a string  using the char string and the number of characters specified
	mstring(char*,UI32);
	///Destructor
	~mstring();
	///	Retrieve the unicode version
	//	this is mainly for future growth.  Since internally we dont keep it in unicode.
	//	We could and should, and hopefully this class will be expanded to do that (or the string class 
	//	will grow to do it for us.  Anyway, just convert the character representation to high byte
	//	of 0.

	char* unicode() ;
	
	///The reverse, take in a unicode character array
	void unicode(const char* szBuffer, UI32 uiLen) ;
	
	// formatting methods
	
	void	trim() ;
	vector<mstring> split(const mstring & clSep, const UI16 uiAmount=0) ;
	vector<mstring> split( char* szSep,const UI16 uiAmount=0) ;
	vector<mstring> splitAny(const mstring & clSep,const UI16 uiAmount=0) ;
	vector<mstring> splitAny( char* szSep,const UI16 uiAmount=0) ;
	void  cleanup();

	//Methods to set binary numbers to a string
	///  Convert a byte	
	mstring & setNum(UI08 uiValue, UI16 uiBase=10) ;
	/// Convert a 16 bit integer
	mstring & setNum(UI16 uiValue, UI16 uiBase=10) ;
	/// Convert a 32 bit integer
	mstring & setNum(UI32 uiValue, UI16 uiBase=10) ;
	mstring & setNum(SI08 uiValue, UI16 uiBase=10) ;
	mstring & setNum(SI16 uiValue, UI16 uiBase=10) ;
	mstring & setNum(SI32 uiValue, UI16 uiBase=10) ;	
	mstring & appendNum(UI08 uiValue, UI16 uiBase=10) ;
	/// Convert a 16 bit integer
	mstring & appendNum(UI16 uiValue, UI16 uiBase=10) ;
	/// Convert a 32 bit integer
	mstring & appendNum(UI32 uiValue, UI16 uiBase=10) ;
	mstring & appendNum(SI08 uiValue, UI16 uiBase=10) ;
	mstring & appendNum(SI16 uiValue, UI16 uiBase=10) ;
	mstring & appendNum(SI32 uiValue, UI16 uiBase=10) ;	
	//Methods to convert the string value to binary number	
	UI32 toUI32(UI16 uiBase=10);
	UI16 toUI16(UI16 uiBase=10);
	UI08 toUI08(UI16 uiBase=10);
	
	SI32 toSI32(UI16 uiBase=10);
	SI16 toSI16(UI16 uiBase=10);
	SI08 toSI08(UI16 uiBase=10);

	string lower() ;

	string upper() ;

	bool  compare(string) ;

	string pop(char*  szSep) ;
	string pop(string sSep) ;

	//Static members
	static vector<mstring> split(const mstring & clSep, const mstring & clString,const UI16 uiAmount=0) ;
	static vector<mstring> split( char* szSep, const mstring clString,const UI16 uiAmount=0) ;
	static vector<mstring> splitAny(const mstring & clSep, const mstring & clString,const UI16 uiAmount=0) ;
	static vector<mstring> splitAny( char* szSep, const mstring clString,const UI16 uiAmount=0) ;

	static mstring  trim(mstring&);
	
	static mstring  cleanup(mstring);

	static string	lower(string&) ;
	static string   upper(string&) ;


};
	
bool nocase_compare(SI08 szA, SI08 szB) ;
//========================================================================
//========================================================================
//=========================  End of mstring.h  ===========================
//========================================================================
//========================================================================
#endif

