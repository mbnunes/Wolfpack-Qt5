//========================================================================
//File: platform.h
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
#ifndef IN_PLATFORM_H
#define IN_PLATFORM_H
//========================================================================
enum enByteOrder{ littleEndian, bigEndian };
#define BYTEORDER littleEndian 
// We have to worry about Intel's compilier as well

#if defined(_MSC_VER) | defined(__INTEL_COMPILER) | defined(__BORLANDC__)

//PRAGMAS

//#pragma pack(1) 
#pragma pack(8)

// no global pack  for windows !
// packing is activated on per struct basis
// #include start_pack
// struct bla
// {
// ...
// } PACK
// #include stop_pack
//
// if we want global packing for windows
// (for linux its not possible due to bus errors you'll get then)
// you'd have to de-activate global packing BEFORE including any(!) stl headers

#pragma warning(disable: 4514)
#pragma warning(disable: 4786)
#pragma warning(disable: 4710)
#pragma warning(disable: 4702)

// The intel compilier doesnt like this one
#pragma warning(disable: 985) 
//#pragma optimize("y", off)
#define OBSOLETETIME
#define PACK

#endif
//========================================================================
/// GCC
#if defined(__GNUG__)

#if defined(WIN32) // mingw needs these
#define OBSOLETETIME
#endif

#define PACK __attribute__((packed))
#if __GNUC__ > 2 && !defined(WIN32) 
#undef OBSOLETESTREAM
#else
#if __GNUC_MINOR__ < 95
#define in_addr_t UI32
#endif
#define OLDGCC
#define stringstream strstream
#define sstream strstream
#define OBSOLETESTREAM
#endif
#endif
// Determine os we are one
#if defined(__unix__)
#define _POSIX
#else
#if !defined(WIN32)
#define WIN32
#endif
#endif


//========================================================================
typedef unsigned long	UI32;
typedef unsigned short	UI16;
typedef unsigned char	UI08;
typedef signed long		SI32;
typedef signed short	SI16;
typedef signed char		SI08;
typedef float			RF32;
typedef double			RF64;
//========================================================================
//========================================================================
//========================================================================
//======================  End of platform.h  =============================
//========================================================================
//========================================================================

#endif

