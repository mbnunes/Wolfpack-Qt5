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

#include "wolfpack.h"
#include "verinfo.h"
#include "worldmain.h"
#include "debug.h"

#undef  DBGFILE
#define DBGFILE "SrvParms.cpp"


//#define REPAIRWORLDTIMER 900 // Ripper...15 mins default.


// important: loaddefault has to be called before. 
// currently it is. makes no sense to change that too.

void loadclientsallowed(void)
{
   int num_c=0, s=0, loopexit=0;
   vector<string> cls;    

   do
   {
		readFullLine();

		if(!(strcmp((char*)script1,"SERVER_DEFAULT"))) { s=1; break; }
		else if(!(strcmp((char*)script1,"ALL"))) { s=2; break; }
		else if (strlen(script1)>3) 
		{ 		
			num_c++;
			cls.push_back(script1);
		}
   }
   while ( (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS) );  
   
   if (num_c==0 && s==0) s=1; // no entry == pick server_default

   switch(s)
   {
	   case 0: 
               clientsAllowed.clear();  // delete the entry from loaddefaults
			   copy(cls.begin(), cls.end(), back_inserter(clientsAllowed) ); // copy temp vector to perm vector
		       break;

	   case 1: 
		       break; // already in vector from loaddefaults

	   case 2: 
		       clientsAllowed.clear();    // delete server_defualt entry from loaddefaults
			   clientsAllowed.push_back("ALL"); // now add the correct one
		       break;
   }

 
}

void loadserverdefaults(void)
{
    clientsAllowed.push_back("SERVER_DEFAULT");
	strcpy(server_data.specialbanktrigger,"WARE"); //AntiChrist - Special Bank word trigger
	server_data.usespecialbank=0;	//AntiChrist - 1= Special Bank enabled

	// Should we check character age for delete?

	server_data.msgboardpath[0] = 0;	   // Dupois - Added Dec 20, 1999 for message boards (current dir)
	
	server_data.msgpostaccess=0;           // Dupois - Added Dec 20, 1999 for message boards (GM only)
	server_data.msgpostremove=0;           // Dupois - Added Dec 20, 1999 for message boards (GM only)
	server_data.msgretention=30;           // Dupois - Added Dec 20, 1999 for message boards (30 Days)

    server_data.eclipsemode=0; // Light to dark..

	cwmWorldState->SetLoopSaveAmt(-1);
		
	begging_data.range=3;
	begging_data.timer=300;
	strcpy(begging_data.text[0],"Could thou spare a few coins?");
	strcpy(begging_data.text[1],"Hey buddy can you spare some gold?");
	strcpy(begging_data.text[2],"I have a family to feed, think of the children.");
}

void loadserver()
{
	// Server.scp completly reorganized by Magius(CHE) 3 September 1999
	unsigned long loopexit=0;
	do
	{
		readw2();
	

		if(!(strcmp((char*)script1,"USESPECIALBANK"))) server_data.usespecialbank=str2num(script2); //AntiChrist - Special Bank
		else if(!(strcmp((char*)script1,"SPECIALBANKTRIGGER")))  //AntiChrist - Special Bank
		{
			strcpy(server_data.specialbanktrigger,script2);
			for(int unsigned i = 0; i < strlen(server_data.specialbanktrigger); i++)
				server_data.specialbanktrigger[i] = toupper(server_data.specialbanktrigger[i]);
		}
		else if(!(strcmp((char*)script1,"ARCHIVEPATH"))) strcpy(server_data.archivepath,(char*)script2);
	
		else if(!(strcmp((char*)script1,"MSGBOARDPATH"))) strcpy(server_data.msgboardpath,(char*)script2);               // Dupois - Added Dec 20, 1999 for message boards
		else if(!(strcmp((char*)script1,"MSGPOSTACCESS"))) server_data.msgpostaccess=str2num(script2);            // Dupois - Added Dec 20, 1999 for message boards
		else if(!(strcmp((char*)script1,"MSGPOSTREMOVE"))) server_data.msgpostremove=str2num(script2);            // Dupois - Added Dec 20, 1999 for message boards
		else if(!(strcmp((char*)script1,"MSGRETENTION"))) server_data.msgretention=str2num(script2);              // Dupois - Added Dec 20, 1999 for message boards

	 
		else if(!(strcmp((char*)script1,"LOOPSAVE"))) cwmWorldState->SetLoopSaveAmt( str2num( script2 ) );
	}
	while ( (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS) );
}

void loadbegging()
{
	unsigned long loopexit=0;
	do
	{
		readw2();

		if(!(strcmp((char*)script1,"BEGGING_TIME"))) begging_data.timer=str2num(script2);
		else if(!(strcmp((char*)script1,"BEGGING_RANGE"))) begging_data.range=str2num(script2);
		else if(!(strcmp((char*)script1,"BEGGING_TEXT0"))) strcpy(begging_data.text[0],(char*)script2);
		else if(!(strcmp((char*)script1,"BEGGING_TEXT1"))) strcpy(begging_data.text[1],(char*)script2);
		else if(!(strcmp((char*)script1,"BEGGING_TEXT2"))) strcpy(begging_data.text[2],(char*)script2);
	}
	while ( (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS) );
}

void loadtime_light()
{
	unsigned long loopexit=0;
	do
	{
		readw2();
		if(!(strcmp((char*)script1,"DAY"))) day=str2num(script2);
		else if(!(strcmp((char*)script1,"HOUR"))) hour=str2num(script2);
		else if(!(strcmp((char*)script1,"MINUTE"))) minute=str2num(script2);
		else if(!(strcmp((char*)script1,"AMPM"))) ampm=str2num(script2);
		else if(!(strcmp((char*)script1,"MOON1UPDATE"))) moon1update=str2num(script2);
		else if(!(strcmp((char*)script1,"MOON2UPDATE"))) moon2update=str2num(script2);
		else if(!(strcmp((char*)script1,"MOON1"))) moon1=str2num(script2);
		else if(!(strcmp((char*)script1,"MOON2"))) moon2=str2num(script2);
		else if(!(strcmp((char*)script1,"DUNGEONLIGHTLEVEL"))) dungeonlightlevel=str2num(script2);
		else if(!(strcmp((char*)script1,"WORLDFIXEDLEVEL"))) worldfixedlevel=str2num(script2);
		else if(!(strcmp((char*)script1,"WORLDCURLEVEL"))) worldcurlevel=str2num(script2);
		else if(!(strcmp((char*)script1,"WORLDBRIGHTLEVEL"))) worldbrightlevel=str2num(script2);
		else if(!(strcmp((char*)script1,"WORLDDARKLEVEL"))) worlddarklevel=str2num(script2);
		else if(!(strcmp((char*)script1,"SECONDSPERUOMINUTE"))) secondsperuominute=str2num(script2);
		else if(!(strcmp((char*)script1,"SEASON"))) season=str2num(script2);
	}
	while ( (strcmp((char*)script1, "}")) && (++loopexit < MAXLOOPS));
}

void loadserverscript(char *fn) // Load a server script
{
	unsigned long loopexit=0;
	wscfile = fopen(fn, "r");
	if(wscfile==NULL)
	{
		clConsole.send("<%s> not found...defaults are used\n",fn);
		return;
	}
	do
	{
		readw2();
		if (!(strcmp((char*)script1, "SECTION")))
		{
			if(!(strcmp((char*)script2, "SERVER"))) loadserver();
			else if(!(strcmp((char*)script2, "CLIENTS_ALLOWED"))) loadclientsallowed();
			else if(!(strcmp((char*)script2, "BEGGING"))) loadbegging();
			else if(!(strcmp((char*)script2, "TIME_LIGHT"))) loadtime_light();
			// added by Magius(CHE)
			// end addons by Magius(CHE)
		}
	}
	while ( (strcmp((char*)script1, "EOF")) && (++loopexit < MAXLOOPS) );
	fclose(wscfile);
	wscfile = NULL;
}

void loadserverscript() // Load server script
{
	loadserverscript("server.scp");
	loadserverscript("shards_server.scp");
}
void saveserverscript(void)
{
	FILE *file;
	file = fopen("server.scp", "w");
    vector<string>::const_iterator vis; 
	
	fprintf(file, "// WOLFPACK Server Script\n");
	fprintf(file, "// Generated by %s Version %s\n\n",  wp_version.betareleasestring.c_str() , wp_version.verstring.c_str() );

	fprintf(file, "SECTION CLIENTS_ALLOWED\n");
	fprintf(file, "{\n");	
   
    for (vis=clientsAllowed.begin(); vis != clientsAllowed.end();  ++vis) 
    {
	  fprintf(file, "%s\n", vis->c_str()); 
    }  

	fprintf(file, "}\n\n");

	
	fprintf(file, "SECTION SERVER\n");
	fprintf(file, "{\n");

	fprintf(file, "USESPECIALBANK %i\n",server_data.usespecialbank);	//AntiChrist - specialbank
	fprintf(file, "SPECIALBANKTRIGGER %s\n",server_data.specialbanktrigger);
	fprintf(file, "ARCHIVEPATH %s\n",server_data.archivepath);
    
	fprintf(file, "MSGBOARDPATH %s\n",server_data.msgboardpath);              // Dupois - Added Dec 20, 1999 for msgboard.cpp
	fprintf(file, "MSGPOSTACCESS %i\n",server_data.msgpostaccess);            // Dupois - Added Dec 20, 1999 for msgboard.cpp
	fprintf(file, "MSGPOSTREMOVE %i\n",server_data.msgpostremove);            // Dupois - Added Dec 20, 1999 for msgboard.cpp
	fprintf(file, "MSGRETENTION %i\n",server_data.msgretention);              // Dupois - Added Dec 20, 1999 for msgboard.cpp
	
	fprintf(file, "LOOPSAVE %i\n",cwmWorldState->LoopSaveAmt());
	fprintf(file, "}\n\n");
	
	fprintf(file, "SECTION BEGGING\n");
	fprintf(file, "{\n");

	fprintf(file, "BEGGING_RANGE %i\n",begging_data.range);
	fprintf(file, "BEGGING_TIME %i\n",begging_data.timer);
	fprintf(file, "BEGGING_TEXT0 %s\n",begging_data.text[0]);
	fprintf(file, "BEGGING_TEXT1 %s\n",begging_data.text[1]);
	fprintf(file, "BEGGING_TEXT2 %s\n",begging_data.text[2]);
	fprintf(file, "}\n\n");
	
	fprintf(file, "SECTION TIME_LIGHT\n");
	fprintf(file, "{\n");
	fprintf(file, "DAY %i\n", day);
	fprintf(file, "HOUR %i\n", hour);
	fprintf(file, "MINUTE %i\n", minute);
	fprintf(file, "AMPM %i\n", ampm);
	fprintf(file, "MOON1UPDATE %i\n", moon1update);
	fprintf(file, "MOON2UPDATE %i\n", moon2update);
	fprintf(file, "MOON1 %i\n", moon1);
	fprintf(file, "MOON2 %i\n", moon2);
	fprintf(file, "DUNGEONLIGHTLEVEL %i\n", dungeonlightlevel);
	fprintf(file, "WORLDFIXEDLEVEL %i\n", worldfixedlevel);
	fprintf(file, "WORLDCURLEVEL %i\n", worldcurlevel);
	fprintf(file, "WORLDBRIGHTLEVEL %i\n", worldbrightlevel);
	fprintf(file, "WORLDDARKLEVEL %i\n", worlddarklevel);
	fprintf(file, "SECONDSPERUOMINUTE %i\n", secondsperuominute); // lb
	fprintf(file, "SEASON %i\n", season); // lb
	fprintf(file, "}\n\n");
	
	fprintf(file, "EOF\n\n");
	fclose(file);
	file = NULL;
}

