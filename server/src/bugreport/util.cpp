
#if defined(CRASHHANDLER)

//=========================================================================================
// BugReport Library
//
// Copyright (c) 2001, Wolfpack Developers (see authors.txt)
// See the LICENSE file for the (BSD) license.
//
// Based on article written by: 
//     John Robbins - Microsoft Systems Journal Bugslayer Column - August '98
//
// Wolfpack Website: http://wpdev.sf.net/
//=========================================================================================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include "internal.h"

// Indicates that the version information is valid.
static BOOL g_bHasVersion = FALSE;
// Indicates NT or 95/98.
static BOOL g_bIsNT = TRUE;

// Forwards
static BOOL IsNT ( void );
// The Win95 version of GetModuleBaseName.
static DWORD __stdcall Win95GetModuleBaseName ( HANDLE  hProcess   ,
                                                HMODULE hModule    ,
                                                LPSTR   lpBaseName ,
                                                DWORD   nSize       ) ;

/*//////////////////////////////////////////////////////////////////////
                        Function Implementation
//////////////////////////////////////////////////////////////////////*/

DWORD __stdcall BSUGetModuleBaseName ( HANDLE hProcess, HMODULE hModule, LPTSTR lpBaseName, DWORD nSize )
{
    if ( IsNT() )
    {
        // Call the NT version.  It is in NT4ProcessInfo because that is
        //  where all the PSAPI wrappers are kept.
        return ( NTGetModuleBaseName ( hProcess, hModule, lpBaseName, nSize ) );
    }
    return ( Win95GetModuleBaseName ( hProcess, hModule, lpBaseName, nSize ) );

}

static DWORD __stdcall
            Win95GetModuleBaseName ( HANDLE  /*hProcess*/  ,
                                     HMODULE hModule       ,
                                     LPSTR   lpBaseName    ,
                                     DWORD   nSize          )
{
    assert ( FALSE == IsBadWritePtr ( lpBaseName , nSize ) ) ;
    if ( TRUE == IsBadWritePtr ( lpBaseName , nSize ) )
    {
        //TRACE0 ( "Win95GetModuleBaseName Invalid string buffer\n" ) ;
        SetLastError ( ERROR_INVALID_PARAMETER );
        return ( 0 ) ;
    }

    // This could blow the stack...
    char szBuff[MAX_PATH + 1];
    DWORD dwRet = GetModuleFileName ( hModule , szBuff , MAX_PATH ) ;
    assert ( 0 != dwRet ) ;
    if ( dwRet == 0 )
    {
        return 0;
    }

    // Find the last '\' mark.
    char * pStart = strrchr ( szBuff , '\\' );
    int iMin;
    if ( NULL != pStart )
    {
        // Move up one character.
        pStart++ ;
        //lint -e666
        iMin = min ( (int)nSize , (lstrlen ( pStart ) + 1) ) ;
        //lint +e666
        lstrcpyn ( lpBaseName , pStart , iMin ) ;
    }
    else
    {
        // Copy the szBuff buffer in.
        //lint -e666
        iMin = min ( (int)nSize , (lstrlen ( szBuff ) + 1) ) ;
        //lint +e666
        lstrcpyn ( lpBaseName , szBuff , iMin ) ;
    }
    // Always NULL terminate.
    lpBaseName[iMin] = '\0' ;
    return ( (DWORD)(iMin - 1) );
}

/*!
  \Internal
  Determines if it's running from inside windows NT as there are differences about
  where PSAPI wrappers are kept
*/
static BOOL IsNT ( void )
{
    if ( TRUE == g_bHasVersion )
    {
        return ( TRUE == g_bIsNT );
    }

    OSVERSIONINFO stOSVI;

    memset ( &stOSVI , NULL , sizeof ( OSVERSIONINFO ) );
    stOSVI.dwOSVersionInfoSize = sizeof ( OSVERSIONINFO );

    BOOL bRet = GetVersionEx ( &stOSVI );
    assert ( TRUE == bRet );
    if ( !bRet )
    {
        //TRACE0 ( "GetVersionEx failed!\n" );
        return ( FALSE );
    }

    // Check the version and call the appropriate thing.
    if ( VER_PLATFORM_WIN32_NT == stOSVI.dwPlatformId )
    {
        g_bIsNT = TRUE;
    }
    else
    {
        g_bIsNT = FALSE;
    }
    g_bHasVersion = TRUE;
    return ( TRUE == g_bIsNT );
}

#endif
