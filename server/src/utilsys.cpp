//========================================================================
//FILE: utilsys.cpp
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
#define OBSOLETETIME
#include "utilsys.h"
//========================================================================
// Begin getNormalizedTime
UI32 getNormalizedTime()
{
	static UI32	uiOffset= 0 ;
	static bool bOneShot = false ;

	if (!bOneShot)
	{
		uiOffset = getPlatformTime() ;
		bOneShot = true ;
	}

	return getPlatformTime() - uiOffset ;
}

//Begin getPlatformTime
UI32 getPlatformTime()
{
	UI32 uiBaseTime ;
#if defined(OBSOLETETIME)
	timeb stTimeB;
	::ftime(&stTimeB);
	uiBaseTime= static_cast<UI32>(stTimeB.time*1000)+static_cast<UI32>(stTimeB.millitm);
#else
	timeval stTimeV ;
	gettimeofday(&stTimeV,NULL) ;
	uiBaseTime= static_cast<UI32>(stTimeV.tv_sec*1000)+static_cast<UI32>(stTimeV.tv_usec/1000) ;
#endif
	return uiBaseTime ;
}
//End of getPlatformTime

// Begin getPlatformDay()
UI32 getPlatformDay()
{
        UI32 uiTime ;
        uiTime = getPlatformTime() ;
        return uiTime/8640000  ; // (1sec/1000mill * 1min/60secs * 1hr/60min * 1day/24hr) ;
}

//===========================================================================
//===========================================================================
//=======================  End of utilsys.cpp  ================================
//===========================================================================
//===========================================================================
