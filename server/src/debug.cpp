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
//	Wolfpack Homepage: http://www.wpdev.com/
//========================================================================================

//Adapted by TANiS - Thanks to Armageddon for the original code.

#include "wolfpack.h"
#include "debug.h"
#include "verinfo.h"
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
	char b1[16],b2[16],b3[16],b4[16];	
	FILE *er_log;	

	unsigned long int ip = inet_addr(serv[0].sIP.c_str());
	char i1,i2,i3,i4;

	i1=(char) (ip>>24);
	i2=(char) (ip>>16);
	i3=(char) (ip>>8);
	i4=(char) (ip%256);

	numtostr(i4 , b1);
	numtostr(i3 , b2);
	numtostr(i2, b3);
	numtostr(i1, b4);

    switch (type)
	{
	   case 'E': { strcpy(file_name,"errors_log_");          entries_e++; break; }
	   case 'C': { strcpy(file_name,"critical_errors_log_"); entries_c++; break; }
	   case 'W': { strcpy(file_name,"warnings_log_");        entries_w++; break; }
	   case 'M': { strcpy(file_name,"messages_log_");        entries_m++; break; }
	} 
	
    strcat(file_name,b1);strcat(file_name,"_");
    strcat(file_name,b2);strcat(file_name,"_");
    strcat(file_name,b3);strcat(file_name,"_");
    strcat(file_name,b4);strcat(file_name,".txt");

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

	if (SrvParms->errors_to_console && type!='M')
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

