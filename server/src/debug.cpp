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
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

//Adapted by TANiS - Thanks to Armageddon for the original code.

#include "wolfpack.h"
#include "debug.h"
#include "verinfo.h"
#include "srvparams.h"
#include "utilsys.h"

static long entries_e=0;
static long entries_c=0;
static long entries_w=0;
static long entries_m=0;
bool NewErrorsLogged() {return (entries_e > 0 || entries_c > 0);}
bool NewWarningsLogged() {return (entries_w > 0);}
/********************************************************
 *                                                      *
 *  Function to be called when a string is ready to be  *
 *    written to the log.  Insert access to your log in *
 *    this function.                                    *
 *                                                      *
 *  Rewritten/Improved/touched by LB 30-July 2000       *
 ********************************************************/

void MessageReady(char *OutputMessage, char type)
{
	char file_name[256];
	FILE *er_log;	

    switch (type)
	{
	   case 'E': { strcpy(file_name,"errors_log.txt");          entries_e++; break; }
	   case 'C': { strcpy(file_name,"critical_errors_log.txt"); entries_c++; break; }
	   case 'W': { strcpy(file_name,"warnings_log.txt");        entries_w++; break; }
	   case 'M': { strcpy(file_name,"messages_log.txt");        entries_m++; break; }
	} 
	
	er_log=fopen(file_name,"a");

	if (er_log==NULL)
	{
		clConsole.send("FATAL ERROR: CANT CREATE/OPEN ERROR LOGFILE, writing to stdout\n");
		clConsole.send(OutputMessage);
		return;
	}

	if (entries_c==1 && type=='C') // @serverstart, write out verison# !!!
	{
       fprintf(er_log,"\nRunning Wolfpack Version: %s\n\n",wp_version.verstring.c_str() );
       fprintf(er_log,"********************************************************************************************************************************************\n");
	   fprintf(er_log,"*** to increase the stability and quality of this software please send this file to the wolfpack developers - thanks for your support!!! ***\n");
	   fprintf(er_log,"********************************************************************************************************************************************\n\n");
	   
	}

    if ( (entries_e==1 && type=='E') || (entries_w==1 && type=='W') || (entries_m==1 && type=='M'))
	{
	   fprintf(er_log,"\nRunning Wolfpack Version: %s\n\n",wp_version.verstring.c_str() );
	}
	
	fprintf(er_log,OutputMessage); // ignoring I/O errors for now !

	if (SrvParams->errors_to_console() && type!='M')
		clConsole.send(OutputMessage);

	fclose(er_log);
}

/********************************************************
 *                                                      *
 *  Rountine to process and stamp a message.            *
 *                                                      *
 ********************************************************/
void LogMessageF(unsigned char Type, long Line, char *File, char *Message, ...)
{
	char fullMessage[512];
	char builtMessage[512];
	va_list argptr;

	va_start(argptr, Message);
	vsprintf(builtMessage, Message, argptr);
	va_end(argptr);

	char timestamp[60];

	time_t currentTime;
	tm *ct;
	time(&currentTime);
	ct = gmtime(&currentTime);
	sprintf(timestamp, "%c:[%2d.%02d][%2d:%02d:%02d]", Type, ct->tm_mday, ct->tm_mon, ct->tm_hour, ct->tm_min, ct->tm_sec);
//#endif

	if (Type=='M')
		strcpy(fullMessage,timestamp);	// no debug info for normal messages (Duke)
	else
		sprintf(fullMessage, "%s %s:%ld ", timestamp, File, Line);
	strcat(fullMessage, builtMessage);
	strcat(fullMessage, "\n");
	MessageReady(fullMessage,Type);
}

