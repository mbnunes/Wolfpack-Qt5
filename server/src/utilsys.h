//========================================================================
//FILE: utilsys.h
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
#ifndef	IN_UTILSYS_H
#define IN_UTILSYS_H
//========================================================================
//Platform Specs
#include "platform.h"
//========================================================================
//System Files

#include <time.h>
// Includes we need for time handling
#if defined(OBSOLETETIME)
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
//=========================================================================
//Prototypes
UI32 getNormalizedTime();
UI32 getPlatformTime();
UI32 getPlatformDay();
//========================================================================
//========================================================================
//=======================  End of utilsys.h  ===============================
//========================================================================
//========================================================================
#endif

