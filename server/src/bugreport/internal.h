
#if !defined(__INTERNAL_H__)
#define __INTERNAL_H__

DWORD __stdcall BSUGetModuleBaseName( HANDLE  hProcess, HMODULE hModule, LPTSTR  lpBaseName, DWORD   nSize );

DWORD __stdcall NTGetModuleBaseName( HANDLE  hProcess, HMODULE hModule, LPTSTR  lpBaseName, DWORD   nSize );

#endif // __INTERNAL_H__