//=========================================================================================
// BugReport Library
//
// Copyright (c) 2001, Wolfpack Developers (see authors.txt)
// See the LICENSE file for the (BSD) license.
//
// Based on article written by:
//     John Robbins - Microsoft Systems Journal Bugslayer Column - August '98
//
// Wolfpack Website: http://developer.berlios.de/projects/wolfpack/
//=========================================================================================

#if !defined(__CRASHHANDLER_H__)
#define __CRASHHANDLER_H__

// Type Definitions
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// The type for the filter function called by the Crash Handler API.
typedef LONG ( __stdcall *PFNCHFILTFN ) ( EXCEPTION_POINTERS* pExPtrs );


BOOL __stdcall SetCrashHandlerFilter( PFNCHFILTFN pFn );


BOOL __stdcall AddCrashHandlerLimitModule( HMODULE hMod );


UINT __stdcall GetLimitModuleCount( void ) ;

#define GLMA_SUCCESS		1
#define GLMA_BADPARAM   	-1
#define GLMA_BUFFTOOSMALL   -2
#define GLMA_FAILURE		0
int __stdcall GetLimitModulesArray( HMODULE* pahMod, UINT uiSize ) ;

/*//////////////////////////////////////////////////////////////////////
		  EXCEPTION_POINTER Translation Functions Declarations
//////////////////////////////////////////////////////////////////////*/

/*----------------------------------------------------------------------
FUNCTION		:   GetFaultReason
DISCUSSION  	:
	Returns a string that describes the fault that occured.  The return
string looks similar to the string returned by Win95's fault dialog.
The returned buffer is constant and do not change it.
	This function can only be called from the callback.
PARAMETERS  	:
	pExPtrs - The exeption pointers passed to the callback.
RETURNS 		:
	!NULL - The constant string that describes the fault.
	NULL  - There was a problem translating the string.
----------------------------------------------------------------------*/
LPCTSTR __stdcall GetFaultReason( EXCEPTION_POINTERS* pExPtrs ) ;


/*----------------------------------------------------------------------
FUNCTION		:   GetFirstStackTraceString
					GetNextStackTraceString
DISCUSSION  	:
	These functions allow you to get the stack trace information for a
crash.  Call GetFirstStackTraceString and then GetNextStackTraceString
to get the entire stack trace for a crash.
	The options GSTSO_PARAMS, GSTSO_MODULE, GSTSO_SYMBOL, and
GSTSO_SRCLINE, appear in that order in the string.
PARAMETERS  	:
	dwOpts   - The options flags  "Or" the following options together.
				0   		  - Just put the PC address in the string.
				GSTSO_PARAMS  - Include the possible params.
				GSTSO_MODULE  - Include the module name as well.
				GSTSO_SYMBOL  - Include the symbol name of the stack
								address.
				GSTSO_SRCLINE - Include source and line info of the
								stack address.
	pExtPtrs - The exception pointers passed to the crash handler
			   function.
RETURNS 		:
	!NULL - The requested stack trace string.
	NULL  - There was a problem.
----------------------------------------------------------------------*/
#define GSTSO_PARAMS	0x01
#define GSTSO_MODULE	0x02
#define GSTSO_SYMBOL	0x04
#define GSTSO_SRCLINE   0x08
LPCTSTR  __stdcall GetFirstStackTraceString( DWORD  			  dwOpts, EXCEPTION_POINTERS* pExPtrs );
LPCTSTR __stdcall GetNextStackTraceString( DWORD				dwOpts, EXCEPTION_POINTERS* pExPtrs ) ;


/*----------------------------------------------------------------------
FUNCTION		:   GetRegisterString
DISCUSSION  	:
	Returns a string with all the registers and their values.  This
function hides all the platform differences.
PARAMETERS  	:
	pExtPtrs - The exception pointers passed to the crash handler
			   function.
RETURNS 		:
	!NULL - The requested register string.
	NULL  - There was a problem.
----------------------------------------------------------------------*/
LPCTSTR __stdcall GetRegisterString( EXCEPTION_POINTERS* pExPtrs ) ;

#endif  // __CRASHHANDLER_H__


