//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
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
//	Wolfpack Homepage: http://www.wpdev.sf.net/
//========================================================================================

#ifndef DEBUG_H
#define DEBUG_H

#include <cstdio>
#include <cstdarg>

#ifndef __unix__
// #include <windows.h>
#else
 #include <time.h>
 //#include <varargs.h>
#endif
#if defined _WIN32 
 //#include <crtdbg.h>
#endif
          

//---------------------------------------------------------------------------
//     Prototypes of functions
//---------------------------------------------------------------------------
bool NewErrorsLogged();
bool NewWarningsLogged();
void MessageReady(char *OutputMessage,char type);
void LogMessageF(unsigned char Type, long Line, char *File, char *Message, ...);

//---------------------------------------------------------------------------
//     Translator macros for functions
//---------------------------------------------------------------------------
#define LogMessage(Message)	LogMessageF('M', __LINE__, DBGFILE, Message)
#define LogWarning(WarningMessage)	LogMessageF('W', __LINE__, DBGFILE, WarningMessage)
#define LogError(ErrorMessage)	LogMessageF('E', __LINE__, DBGFILE, ErrorMessage)
#define LogCritical(CriticalMessage)	LogMessageF('C', __LINE__, DBGFILE, CriticalMessage)

#define LogMessageVar(FormatString,var)	{char t[512];sprintf(t,FormatString,var);LogMessage(t);}
#define LogWarningVar(FormatString,var)	{char t[512];sprintf(t,FormatString,var);LogWarning(t);}
#define LogErrorVar(FormatString,var)	{char t[512];sprintf(t,FormatString,var);LogError(t);}
#define LogCriticalVar(FormatString,var){char t[512];sprintf(t,FormatString,var);LogCritical(t);}

#define _ ,

//#define Xsend(sock,buff,length) if (sock<0 || sock>=MAXCLIENT) LogCritical("Socket/Index confusion @ Xsend %i\n" _ sock);  else Network->xSend(sock, buff, length, 0)
#define Xsend(sock,buff,length) if ( sock>=MAXCLIENT) LogCritical("Socket/Index confusion @ Xsend %i\n" _ sock);  else Network->xSend(sock, buff, length, 0)
#define CRASH_IF_INVALID_SOCK(sock) if (sock<0 || sock>=MAXCLIENT) chars[-1000000].x=0

// Usefull to leave some FIXME comments in source (will show in the warnings window (Original from UOX3) :)
#define _QUOTE(x) # x
#define QUOTE(x) _QUOTE(x)
#define __FILE__LINE__ __FILE__ "(" QUOTE(__LINE__) ") : "

#define NOTE( x )  message( x )
#define FILE_LINE  message( __FILE__LINE__ )

#define WPTODO( x )  message( __FILE__LINE__"\n""+------------------------------------------------\n""|  TODO :   " #x "\n""+-------------------------------------------------\n" )
#define FIXME( x ) message( __FILE__LINE__"\n""+------------------------------------------------\n""|  FIXME :  " #x "\n""+-------------------------------------------------\n" )
#define wptodo( x )  message( __FILE__LINE__" TODO :   " #x "\n" ) 
#define fixme( x ) message( __FILE__LINE__" FIXME:   " #x "\n" ) 
#define note( x )  message( __FILE__LINE__" NOTE :   " #x "\n" ) 


#endif

