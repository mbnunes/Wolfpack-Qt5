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


/*----------------------------------------------------------------------
 John Robbins - Microsoft Systems Journal Bugslayer Column - August '98

CONDITIONAL COMPILATION :
    WORK_AROUND_SRCLINE_BUG - Define this to work around the
                              SymGetLineFromAddr bug where PDB file
                              lookups fail after the first lookup.
----------------------------------------------------------------------*/

#if defined(CRASHHANDLER)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wintrust.h>

#include <imagehlp.h>
#include <assert.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>

#include "crashhandler.h"
#include "internal.h"

#include <qmap.h>
#include <qstring.h>
#include <qvaluevector.h>

QValueVector<QPair<unsigned int, unsigned int> > moduleBases;

DWORD CALLBACK MySymGetModuleBase(HANDLE hProcess, DWORD dwAddr) {
    DWORD dwTemp = SymGetModuleBase (GetCurrentProcess(), dwAddr);

	// Fallback
	if (!dwTemp) {
		QValueVector<QPair<unsigned int, unsigned int> >::iterator it;
		for (it = moduleBases.begin(); it != moduleBases.end(); ++it) {
			if (dwAddr >= it->first && dwAddr <= it->first + it->second) {
				dwTemp = it->first;
				break;
			}
		}
	}

	return dwTemp;
}

static PFNCHFILTFN g_pfnCallBack = NULL;	// The filter function.
static LPTOP_LEVEL_EXCEPTION_FILTER g_pfnOrigFilt = NULL; // The original exception filter.
static HMODULE * g_ahMod = NULL;			// The array of modules to limit Crash Handler to.
static UINT g_uiModCount = 0;				// The size, in items, of g_ahMod.
#define BUFF_SIZE 1024						
static TCHAR g_szBuff [ BUFF_SIZE ];		// The static buffer returned by various functions.  This avoids putting things on the stack.
#define SYM_BUFF_SIZE 512
static BYTE g_stSymbol [ SYM_BUFF_SIZE ];	// The static symbol lookup buffer.  This gets casted to make it work.
static IMAGEHLP_LINE g_stLine;				// The static source and line structure.
static STACKFRAME g_stFrame;				// The stack frame used in walking the stack.
static bool g_bSymEngInit = FALSE;			// The flag that indicates that the symbol engine as been initialized.

// The pointer to the SymGetLineFromAddr function I GetProcAddress out
//  of IMAGEHLP.DLL in case the user has an older version that does not
//  support the new extensions.
typedef BOOL (WINAPI *PFNSYMGETLINEFROMADDR)(HANDLE, DWORD, LPDWORD, PIMAGEHLP_LINE);
static PFNSYMGETLINEFROMADDR g_pfnSymGetLineFromAddr = NULL ;

// The flag that says if I have already done the GetProcAddress on
//  g_pfnSymGetLineFromAddr.
static BOOL g_bLookedForSymFuncs = FALSE ;


// Forwards

// The exception handler.
LONG __stdcall CrashHandlerExceptionFilter ( EXCEPTION_POINTERS *
                                             pExPtrs              ) ;

// Converts a simple exception to a string value.
LPCTSTR ConvertSimpleException ( DWORD dwExcept ) ;

// The internal function that does all the stack walking.
LPCTSTR __stdcall
            InternalGetStackTraceString ( DWORD                dwOpts  ,
                                          EXCEPTION_POINTERS * pExPtrs);

// The internal SymGetLineFromAddr function.
BOOL InternalSymGetLineFromAddr ( IN  HANDLE          hProcess        ,
                                  IN  DWORD           dwAddr          ,
                                  OUT PDWORD          pdwDisplacement ,
                                  OUT PIMAGEHLP_LINE  Line            );

// Initializes the symbol engine if needed.
void InitSymEng ( void );

// Cleans up the symbol engine if needed.
void CleanupSymEng ( void );

// Cleanup static class
#pragma warning (disable : 4073)
#pragma init_seg(lib)
struct CleanUpHandler
{
    ~CleanUpHandler ( void )
    {
        // Is there any outstanding memory allocations?
        if ( NULL != g_ahMod )
        {
            assert( HeapFree ( GetProcessHeap(), 0, g_ahMod ) );
            g_ahMod = 0;
        }
        if ( g_pfnOrigFilt != 0 )
        {
            // Set the handler back to what it originally was.
            SetUnhandledExceptionFilter ( g_pfnOrigFilt );
        }
    }
};

static CleanUpHandler cleanUp;


/*!
	Sets the filter function that will be called when there is a fatal
	crash. The \a pFn function will only be called if the crash occur 
	in one of the modules passed to AddCrashHandlerLimitModule. If no
	modules have been defined to narrow down the interested modules, the
	the callback filter function \a pFn will always be called.
	If \a pFn is 0, the Crash Handler filter function will be removed.
	An return value of true means the callback was succefully installed.
	\sa AddCrashHandlerLimitModule
*/
BOOL __stdcall SetCrashHandlerFilter ( PFNCHFILTFN pFn )
{
    // It's OK to have a NULL parameter because this will unhook the
    //  callback.
    if ( pFn == 0 )
    {
        if ( g_pfnOrigFilt != 0 )
        {
            // Put the original one back.
            SetUnhandledExceptionFilter ( g_pfnOrigFilt );
            g_pfnOrigFilt = 0 ;
            if ( g_ahMod != 0 )
            {
                free ( g_ahMod );
                g_ahMod = 0;
            }
            g_pfnCallBack = 0;
        }
    }
    else
    {
        g_pfnCallBack = pFn;

        // If this is the first time that CrashHandler has been called
        //  set the exception filter and save off the previous handler.
        if ( g_pfnOrigFilt == 0 )
        {
            g_pfnOrigFilt = SetUnhandledExceptionFilter(CrashHandlerExceptionFilter);
        }
    }
    return TRUE;
}

/*!
	Adds a module to the list of modules that CrashHandler will trigger the
	callback function in the event of a crash. If no module is added, the
	callback is triggered for all crashes. Limiting the specific modules
	allows the crash handler to be installed for just the modules you are
	responsible for. The function will return true if the operation was
	successfull.
*/
BOOL __stdcall AddCrashHandlerLimitModule ( HMODULE hMod )
{
    // Check the obvious cases.
    assert( NULL != hMod );
    if ( NULL == hMod )
    {
        return ( FALSE );
    }

    // TODO TODO
    //  Do the check that hMod really is a PE module.

    // Allocate a temporary version.  This must be allocated into memory
    //  that is guaranteed to be around even if the process is toasting.
    //  This means the RTL heap is probably already gone so I do it out
    //  of the process heap.
    HMODULE * phTemp = (HMODULE*) HeapAlloc ( GetProcessHeap(), HEAP_ZERO_MEMORY | HEAP_GENERATE_EXCEPTIONS,
                                (sizeof(HMODULE)*(g_uiModCount+1))  ) ;
    assert ( phTemp != 0 ) ;
    if ( phTemp == 0 )
    {
        //TRACE0 ( "Serious trouble in the house! - malloc failed!!!\n" );
        return FALSE;
    }

    if ( NULL == g_ahMod )
    {
        g_ahMod = phTemp;
        g_ahMod[0] = hMod;
        ++g_uiModCount;
    }
    else
    {
        // Copy the old values.
        CopyMemory ( phTemp, g_ahMod, sizeof ( HMODULE ) * g_uiModCount );
        g_ahMod = phTemp;
        g_ahMod[g_uiModCount] = hMod;
        ++g_uiModCount;
    }
    return TRUE;
}

/*!
	Returns the number of limit modules for the Crash Handler.
*/
UINT __stdcall GetLimitModuleCount ( void )
{
    return g_uiModCount;
}

/*!
	Returns the limit modules currently active. \a pahMod consists of
	an output argument where the array will be copied into and \a uiSize
	will be feed with the size of the array.
	This function returns the status of the operation as one of:
    GLMA_SUCCESS        - The limit items where copied.
    GLMA_BADPARAM       - The parameter(s) were invalid.
    GLMA_BUFFTOOSMALL   - The ahMod was too small to copy all the
                          values.
    GLMA_FAILURE        - There was a major problem.
*/
int __stdcall GetLimitModulesArray ( HMODULE * pahMod , UINT uiSize )
{
    int iRet;

    __try
    {
        assert( !IsBadWritePtr ( pahMod, uiSize * sizeof ( HMODULE ) ) );
        if ( IsBadWritePtr ( pahMod, uiSize * sizeof ( HMODULE ) ) )
        {
            iRet = GLMA_BADPARAM;
            __leave;
        }

        if ( uiSize < g_uiModCount )
        {
            iRet = GLMA_BUFFTOOSMALL;
            __leave;
        }

        CopyMemory ( pahMod, g_ahMod, sizeof ( HMODULE ) * g_uiModCount );

        iRet = GLMA_SUCCESS;
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        iRet = GLMA_FAILURE;
    }
    return iRet;
}

LONG __stdcall CrashHandlerExceptionFilter (EXCEPTION_POINTERS* pExPtrs)
{
    LONG lRet = EXCEPTION_CONTINUE_SEARCH;

    __try
    {
        if ( g_pfnCallBack != 0 )
        {

            // The symbol engine has to be initialized here so that
            //  I can look up the base module information for the
            //  crash address as well as just get the symbol engine
            //  ready.
            InitSymEng();

            // Check the g_ahMod list.
            bool callIt = false;
            if ( !g_uiModCount )
            {
                callIt = true;
            }
            else
            {
                HINSTANCE baseAddr = (HINSTANCE) MySymGetModuleBase(GetCurrentProcess(), (DWORD)pExPtrs->ExceptionRecord->ExceptionAddress);
                if ( NULL != baseAddr )
                {
                    for ( unsigned int i = 0; i < g_uiModCount; ++i )
                    {
                        if ( g_ahMod[i] == baseAddr )
                        {
                            callIt = true;
                            break;
                        }
                    }
                }
            }
            if ( callIt )
            {
                // Check that the filter function still exists in memory
                //  before I call it.  The user might have forgotten to
                //  unregister and the filter function is invalid
                //  because it got unloaded.  Of course, if something
                //  loaded back into the same address, there is not much
                //  I can do.
                if ( !IsBadCodePtr ( (FARPROC)g_pfnCallBack ) )
                {
                    lRet = g_pfnCallBack ( pExPtrs );
                }
            }
            else
            {
                // Call the previous filter but only after it checks
                //  out.  I am just being a little paranoid.
                if ( !IsBadCodePtr ( (FARPROC)g_pfnOrigFilt ) )
                {
                    lRet = g_pfnOrigFilt ( pExPtrs );
                }
            }
            CleanupSymEng ( );
        }
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        lRet = EXCEPTION_CONTINUE_SEARCH;
    }
    return lRet;
}

/*!
  Exception Pointer Translation function.
*/
LPCTSTR __stdcall GetFaultReason ( EXCEPTION_POINTERS * pExPtrs )
{
    assert ( FALSE == IsBadReadPtr ( pExPtrs, sizeof(EXCEPTION_POINTERS ) ) ) ;
    if ( TRUE == IsBadReadPtr ( pExPtrs, sizeof ( EXCEPTION_POINTERS ) ) )
    {
        //TRACE0 ( "Bad parameter to GetFaultReasonA\n" ) ;
        return ( NULL ) ;
    }

    // The value that holds the return.
    LPCTSTR szRet;

    __try
    {

        // Initialize the symbol engine in case it is not initialized.
        InitSymEng();

        // The current position in the buffer.
        int iCurr = 0 ;
        // A temp value holder.  This is to keep the stack usage to a
        //  minimum.
        DWORD dwTemp;

        //iCurr += BSUGetModuleBaseName ( GetCurrentProcess(), NULL, g_szBuff, BUFF_SIZE );
        //iCurr += wsprintf ( g_szBuff + iCurr , _T ( " caused a " ) ) ;

        dwTemp = (DWORD)ConvertSimpleException(pExPtrs->ExceptionRecord->ExceptionCode);

        if ( dwTemp )
        {
            iCurr += wsprintf ( g_szBuff + iCurr, _T ( "%s" ), dwTemp );
        }
        else
        {
            iCurr += (FormatMessage( FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
                                     GetModuleHandle (_T("NTDLL.DLL")), pExPtrs->ExceptionRecord->ExceptionCode,
                                     0, g_szBuff + iCurr, BUFF_SIZE, 0) * sizeof ( TCHAR ) );
        }

        assert ( iCurr < BUFF_SIZE );

        iCurr += wsprintf ( g_szBuff + iCurr , _T ( " in module " ) );

		dwTemp = MySymGetModuleBase(GetCurrentProcess(), (DWORD)pExPtrs->ExceptionRecord->ExceptionAddress);
        assert ( NULL != dwTemp );

        if ( !dwTemp )
        {
            iCurr += wsprintf( g_szBuff + iCurr , _T( "<UNKNOWN>" ) );
        }
        else
        {
            iCurr += BSUGetModuleBaseName ( GetCurrentProcess(), (HINSTANCE)dwTemp, g_szBuff + iCurr, BUFF_SIZE - iCurr);
        }

    #ifdef _ALPHA_
        iCurr += wsprintf ( g_szBuff + iCurr    ,
                            _T ( " at %08X" )   ,
                            pExPtrs->ExceptionRecord->ExceptionAddress);
    #else
        iCurr += wsprintf ( g_szBuff + iCurr                ,
                            _T ( " at %04X:%08X" )          ,
                            pExPtrs->ContextRecord->SegCs   ,
                            pExPtrs->ExceptionRecord->ExceptionAddress);
    #endif

        szRet = g_szBuff;
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        assert ( FALSE );
        szRet = NULL;
    }
    return ( szRet );
}

LPCTSTR __stdcall
             GetFirstStackTraceString ( DWORD                dwOpts  ,
                                        EXCEPTION_POINTERS * pExPtrs  )
{
    // All of the error checking is in the InternalGetStackTraceString
    //  function.

    // Initialize the STACKFRAME structure.
    ZeroMemory ( &g_stFrame , sizeof ( STACKFRAME ) ) ;

    #ifdef _X86_
    g_stFrame.AddrPC.Offset       = pExPtrs->ContextRecord->Eip ;
    g_stFrame.AddrPC.Mode         = AddrModeFlat                ;
    g_stFrame.AddrStack.Offset    = pExPtrs->ContextRecord->Esp ;
    g_stFrame.AddrStack.Mode      = AddrModeFlat                ;
    g_stFrame.AddrFrame.Offset    = pExPtrs->ContextRecord->Ebp ;
    g_stFrame.AddrFrame.Mode      = AddrModeFlat                ;
    #else
    g_stFrame.AddrPC.Offset       = (DWORD)pExPtrs->ContextRecord->Fir ;
    g_stFrame.AddrPC.Mode         = AddrModeFlat ;
    g_stFrame.AddrReturn.Offset   =
                                   (DWORD)pExPtrs->ContextRecord->IntRa;
    g_stFrame.AddrReturn.Mode     = AddrModeFlat ;
    g_stFrame.AddrStack.Offset    =
                                   (DWORD)pExPtrs->ContextRecord->IntSp;
    g_stFrame.AddrStack.Mode      = AddrModeFlat ;
    g_stFrame.AddrFrame.Offset    =
                                   (DWORD)pExPtrs->ContextRecord->IntFp;
    g_stFrame.AddrFrame.Mode      = AddrModeFlat ;
    #endif

    return ( InternalGetStackTraceString ( dwOpts , pExPtrs ) ) ;
}

LPCTSTR __stdcall
             GetNextStackTraceString ( DWORD                dwOpts  ,
                                       EXCEPTION_POINTERS * pExPtrs  )
{
    // All error checking is in InternalGetStackTraceString.
    // Assume that GetFirstStackTraceString has already initialized the
    //  stack frame information.
    return ( InternalGetStackTraceString ( dwOpts , pExPtrs ) ) ;
}


// The internal function that does all the stack walking.
LPCTSTR __stdcall
          InternalGetStackTraceString ( DWORD                dwOpts  ,
                                        EXCEPTION_POINTERS * pExPtrs  )
{

    assert ( FALSE == IsBadReadPtr ( pExPtrs                      ,
                                     sizeof (EXCEPTION_POINTERS )));
    if ( TRUE == IsBadReadPtr ( pExPtrs                      ,
                                sizeof ( EXCEPTION_POINTERS ) ) )
    {
        //TRACE0 ( "GetStackTraceString - invalid pExPtrs!\n" ) ;
        return ( NULL ) ;
    }

    // The value that is returned.
    LPCTSTR szRet ;
    // A temporary for all to use.  This saves stack space.
    DWORD dwTemp ;

    __try
    {
        // Initialize the symbol engine in case it is not initialized.
        InitSymEng ( ) ;

#ifdef _ALPHA_
#define CH_MACHINE IMAGE_FILE_MACHINE_ALPHA
#else
#define CH_MACHINE IMAGE_FILE_MACHINE_I386
#endif
        // Note:  If the source and line functions are used, then
        //        StackWalk can access violate.
        BOOL bSWRet = StackWalk ( CH_MACHINE                ,
                                  GetCurrentProcess ( )     ,
                                  GetCurrentThread ( )      ,
                                  &g_stFrame                ,
                                  pExPtrs->ContextRecord    ,
                                  NULL                      ,
                                  SymFunctionTableAccess    ,
                                  MySymGetModuleBase          ,
                                  NULL                       ) ;
        if ( ( FALSE == bSWRet ) || ( 0 == g_stFrame.AddrFrame.Offset ))
        {
            szRet = NULL ;
            __leave ;
        }

        int iCurr = 0 ;
/*
        // At a minimum, put the address in.
#ifdef _ALPHA_
        iCurr += wsprintf ( g_szBuff + iCurr        ,
                            _T ( "0x%08X" )         ,
                            g_stFrame.AddrPC.Offset  ) ;
#else
        iCurr += wsprintf ( g_szBuff + iCurr              ,
                            _T ( "%04X:%08X" )            ,
                            pExPtrs->ContextRecord->SegCs ,
                            g_stFrame.AddrPC.Offset        ) ;
#endif
*/
        // Do the parameters?
        /*if ( GSTSO_PARAMS == ( dwOpts & GSTSO_PARAMS ) )
        {
            iCurr += wsprintf ( g_szBuff + iCurr          ,
                                _T ( " (0x%08X 0x%08X "\
                                      "0x%08X 0x%08X)"  ) ,
                                g_stFrame.Params[ 0 ]     ,
                                g_stFrame.Params[ 1 ]     ,
                                g_stFrame.Params[ 2 ]     ,
                                g_stFrame.Params[ 3 ]      ) ;
        }*/

        if ( GSTSO_MODULE == ( dwOpts & GSTSO_MODULE ) )
        {
            iCurr += wsprintf ( g_szBuff + iCurr  , _T ( " " ) ) ;

            dwTemp = MySymGetModuleBase ( GetCurrentProcess ( )   ,
                                        g_stFrame.AddrPC.Offset  ) ;

            assert ( NULL != dwTemp ) ;

            if ( NULL == dwTemp )
            {
                iCurr += wsprintf ( g_szBuff + iCurr  ,
                                    _T ( "<UNKNOWN>" ) ) ;
            }
            else
            {
                iCurr += BSUGetModuleBaseName ( GetCurrentProcess ( ) ,
                                                (HINSTANCE)dwTemp     ,
                                                g_szBuff + iCurr      ,
                                                BUFF_SIZE - iCurr     );
            }
        }

        assert ( iCurr < BUFF_SIZE ) ;
        DWORD dwDisp ;

        if ( GSTSO_SYMBOL == ( dwOpts & GSTSO_SYMBOL ) )
        {

            // Start looking up the exception address.
            //lint -e545
            PIMAGEHLP_SYMBOL pSym = (PIMAGEHLP_SYMBOL)&g_stSymbol ;
            //lint +e545
            ZeroMemory ( pSym , SYM_BUFF_SIZE ) ;
            pSym->SizeOfStruct = sizeof ( IMAGEHLP_SYMBOL ) ;
            pSym->MaxNameLength = SYM_BUFF_SIZE -
                                  sizeof ( IMAGEHLP_SYMBOL ) ;

            if ( TRUE ==
                  SymGetSymFromAddr ( GetCurrentProcess ( )   ,
                                      g_stFrame.AddrPC.Offset ,
                                      &dwDisp                 ,
                                      pSym                     ) )
            {
                iCurr += wsprintf ( g_szBuff + iCurr , _T ( ", " ) ) ;

                // Copy no more than there is room for.
                dwTemp = lstrlen ( pSym->Name ) ;
                if ( dwTemp > (DWORD)( BUFF_SIZE - iCurr - 20 ) )
                {
                    lstrcpyn ( g_szBuff + iCurr      ,
                               pSym->Name            ,
                               BUFF_SIZE - iCurr - 1  ) ;
                    // Gotta leave now.
                    szRet = g_szBuff ;
                    __leave ;
                }
                else
                {
                    if ( dwDisp > 0 )
                    {
                        iCurr += wsprintf ( g_szBuff + iCurr         ,
                                            _T ( "%s()+%d byte(s)" ) ,
                                            pSym->Name               ,
                                            dwDisp                    );
                    }
                    else
                    {
                        iCurr += wsprintf ( g_szBuff + iCurr ,
                                            _T ( "%s" )      ,
                                            pSym->Name        ) ;
                    }
                }
            }
            else
            {
				iCurr += wsprintf ( g_szBuff + iCurr              ,
                            _T ( ", %04X:%08X" )            ,
                            pExPtrs->ContextRecord->SegCs ,
                            g_stFrame.AddrPC.Offset        ) ;

                // If the symbol was not found, the source and line will
                //  not be found either so leave now.
                szRet = g_szBuff ;
                __leave ;
            }

        }

        if ( GSTSO_SRCLINE == ( dwOpts & GSTSO_SRCLINE ) )
        {
            ZeroMemory ( &g_stLine , sizeof ( IMAGEHLP_LINE ) ) ;
            g_stLine.SizeOfStruct = sizeof ( IMAGEHLP_LINE ) ;

            if ( TRUE ==
                   InternalSymGetLineFromAddr ( GetCurrentProcess ( )  ,
                                                g_stFrame.AddrPC.Offset,
                                                &dwDisp                ,
                                                &g_stLine             ))
            {
                iCurr += wsprintf ( g_szBuff + iCurr , _T ( ", " ) ) ;

                // Copy no more than there is room for.
                dwTemp = lstrlen ( g_stLine.FileName ) ;
                if ( dwTemp > (DWORD)( BUFF_SIZE - iCurr - 25 ) )
                {
                    lstrcpyn ( g_szBuff + iCurr      ,
                               g_stLine.FileName     ,
                               BUFF_SIZE - iCurr - 1  ) ;
                    // Gotta leave now.
                    szRet = g_szBuff ;
                    __leave ;
                }
                else
                {
                    if ( dwDisp > 0 )
                    {
                        iCurr += wsprintf(g_szBuff + iCurr             ,
                                          _T ("%s:%d + %d byte(s)"),
                                          g_stLine.FileName            ,
                                          g_stLine.LineNumber          ,
                                          dwDisp                      );
                    }
                    else
                    {
                        iCurr += wsprintf ( g_szBuff + iCurr     ,
                                            _T ( "%s:%d" ) ,
                                            g_stLine.FileName    ,
                                            g_stLine.LineNumber   ) ;
                    }
                }
            }
        }

        szRet = g_szBuff ;
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        assert ( FALSE ) ;
        szRet = NULL ;
    }
    return ( szRet ) ;
}

//lint -e527
LPCTSTR ConvertSimpleException ( DWORD dwExcept )
{
    switch ( dwExcept )
    {
        case EXCEPTION_ACCESS_VIOLATION:      return ( _T ( "EXCEPTION_ACCESS_VIOLATION" ) ) ;
        case EXCEPTION_DATATYPE_MISALIGNMENT: return ( _T ( "EXCEPTION_DATATYPE_MISALIGNMENT" ) ) ;
        case EXCEPTION_BREAKPOINT:            return ( _T ( "EXCEPTION_BREAKPOINT" ) ) ;
        case EXCEPTION_SINGLE_STEP:           return ( _T ( "EXCEPTION_SINGLE_STEP" ) ) ;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return ( _T ( "EXCEPTION_ARRAY_BOUNDS_EXCEEDED" ) ) ;
        case EXCEPTION_FLT_DENORMAL_OPERAND: return ( _T ( "EXCEPTION_FLT_DENORMAL_OPERAND" ) ) ;
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:  return ( _T ( "EXCEPTION_FLT_DIVIDE_BY_ZERO" ) ) ;
        case EXCEPTION_FLT_INEXACT_RESULT:  return ( _T ( "EXCEPTION_FLT_INEXACT_RESULT" ) ) ;
        case EXCEPTION_FLT_INVALID_OPERATION: return ( _T ( "EXCEPTION_FLT_INVALID_OPERATION" ) ) ;
        case EXCEPTION_FLT_OVERFLOW:        return ( _T ( "EXCEPTION_FLT_OVERFLOW" ) ) ;
        case EXCEPTION_FLT_STACK_CHECK:     return ( _T ( "EXCEPTION_FLT_STACK_CHECK" ) ) ;
        case EXCEPTION_FLT_UNDERFLOW:       return ( _T ( "EXCEPTION_FLT_UNDERFLOW" ) ) ;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:  return ( _T ( "EXCEPTION_INT_DIVIDE_BY_ZERO" ) ) ;
        case EXCEPTION_INT_OVERFLOW:        return ( _T ( "EXCEPTION_INT_OVERFLOW" ) ) ;
        case EXCEPTION_PRIV_INSTRUCTION:    return ( _T ( "EXCEPTION_PRIV_INSTRUCTION" ) ) ;
        case EXCEPTION_IN_PAGE_ERROR:       return ( _T ( "EXCEPTION_IN_PAGE_ERROR" ) ) ;
        case EXCEPTION_ILLEGAL_INSTRUCTION: return ( _T ( "EXCEPTION_ILLEGAL_INSTRUCTION" ) ) ;
        case EXCEPTION_NONCONTINUABLE_EXCEPTION:  return ( _T ( "EXCEPTION_NONCONTINUABLE_EXCEPTION" ) ) ;
        case EXCEPTION_STACK_OVERFLOW:      return ( _T ( "EXCEPTION_STACK_OVERFLOW" ) ) ;
        case EXCEPTION_INVALID_DISPOSITION:	return ( _T ( "EXCEPTION_INVALID_DISPOSITION" ) );
        case EXCEPTION_GUARD_PAGE:			return ( _T ( "EXCEPTION_GUARD_PAGE" ) );
        case EXCEPTION_INVALID_HANDLE:		return ( _T ( "EXCEPTION_INVALID_HANDLE" ) );
        default:				            return ( 0 );
    }
}


BOOL InternalSymGetLineFromAddr ( IN  HANDLE          hProcess        ,
                                  IN  DWORD           dwAddr          ,
                                  OUT PDWORD          pdwDisplacement ,
                                  OUT PIMAGEHLP_LINE  Line            )
{
    // Have I already done the GetProcAddress?
    if ( FALSE == g_bLookedForSymFuncs )
    {
        g_bLookedForSymFuncs = TRUE ;
        g_pfnSymGetLineFromAddr = (PFNSYMGETLINEFROMADDR)
                     GetProcAddress(GetModuleHandle(_T("IMAGEHLP.DLL")),
                                   "SymGetLineFromAddr"               );
    }
    if ( NULL != g_pfnSymGetLineFromAddr )
    {
#ifdef WORK_AROUND_SRCLINE_BUG

        // The problem is that the symbol engine only finds those source
        //  line addresses (after the first lookup) that fall exactly on
        //  a zero displacement.  I will walk backwards 100 bytes to
        //  find the line and return the proper displacement.
        DWORD dwTempDis = 0 ;
        while ( FALSE == g_pfnSymGetLineFromAddr ( hProcess        ,
                                                   dwAddr -
                                                    dwTempDis      ,
                                                   pdwDisplacement ,
                                                   Line             ) )
        {
            dwTempDis += 1 ;
            if ( 100 == dwTempDis )
            {
                return ( FALSE ) ;
            }
        }

        // It was found and the source line information is correct so
        //  change the displacement if it was looked up multiple times.
        if ( 0 != dwTempDis )
        {
            *pdwDisplacement = dwTempDis ;
        }
        return ( TRUE ) ;

#else  // WORK_AROUND_SRCLINE_BUG
        return ( g_pfnSymGetLineFromAddr ( hProcess         ,
                                           dwAddr           ,
                                           pdwDisplacement  ,
                                           Line              ) ) ;
#endif
    }
    return ( FALSE ) ;
}

BOOL CALLBACK EnumerateLoadedModulesProc(
  PSTR ModuleName,
  DWORD ModuleBase,
  ULONG ModuleSize,
  PVOID UserContext
  ) {
	  if (!SymLoadModule(GetCurrentProcess(), NULL, "wolfpack.pdb", ModuleName, ModuleBase, ModuleSize)) {
		  /*char message[512];
		  sprintf(message, "Couldn't load symbol information for module %s (0x%x).", ModuleName, GetLastError());
		  MessageBox(0, message, "Failure", 0);*/
	  }

	  moduleBases.append(QPair<unsigned int, unsigned int>(ModuleBase, ModuleSize));
	  return TRUE;
  }

  // Initializes the symbol engine if needed.
void InitSymEng ( void )
{
    if ( !g_bSymEngInit )
    {
        // Set up the symbol engine.
        DWORD dwOpts = SymGetOptions();
		//SymRegisterCallback(GetCurrentProcess(), (PSYMBOL_REGISTERED_CALLBACK)symCallback, 0);

        // Always defer loading to make life faster.
        SymSetOptions(dwOpts|SYMOPT_LOAD_LINES/*SYMOPT_DEFERRED_LOADS|*/);

        // Initialize the symbol engine.
        SymInitialize(GetCurrentProcess(), "C:\\Wolfpack\\", FALSE);
		EnumerateLoadedModules(GetCurrentProcess(), EnumerateLoadedModulesProc, 0);
    }
    g_bSymEngInit = true;
}

// Cleans up the symbol engine if needed.
void CleanupSymEng ( void )
{
    if ( g_bSymEngInit )
    {
        assert( SymCleanup ( GetCurrentProcess ( ) ) );
    }
    g_bSymEngInit = false;
}

#endif

