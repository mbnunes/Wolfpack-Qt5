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

/////////////////////
// Name:	scripts.cpp
// Purpose: functions that handle the timer controlled stuff
// History:	cut from wolfpack.cpp by Duke, 26.10.00
// Remarks:	not necessarily ALL those functions
//

#include "wolfpack.h"
#include "debug.h"
#include "scriptc.h"

#undef  DBGFILE
#define DBGFILE "scripts.cpp"

void readscript () // Read a line from the opened script file
{
	int i, valid=0;
	char c;
	temp[0]=0;

	while(!valid)
	{
		i=0;
		if (feof(scpfile)) return;
		c=(char)fgetc(scpfile);
		while (c!=10)
		{
			if (c!=13)
			{
				temp[i]=c;
				i++;
			}
			if (feof(scpfile) || i>=512) { temp[0]='E'; temp[1]='O'; temp[2]='F'; temp[3]=0; return; } // quick hack to avoid server freezes if a script doesnt end with a return, LB
			c=(char)fgetc(scpfile);
		}
		temp[i]=0;
		valid=1;
		if (temp[0]=='/' && temp[1]=='/') valid=0;
		if (temp[0]=='{') valid=0;
		if (temp[0]==0) valid=0;
	}
}

bool openscript (char *name, bool shutdownonfail) // Open script file
{
	scpfile=fopen(name,"r") ;
	 
	if (scpfile==NULL )
	{
		if (!shutdownonfail)
			return false;

		clConsole.send("ERROR: %s not found...\n",name);
		error=1;
		keeprun=0;
	}
	openings++;
	if(openings>1)
	{//AntiChrist
		sprintf((char*)temp,"WOLFPACK Script Manager ERROR: error opening %s ( openings: %i ). Last file: %s\n",name,openings,scpfilename);
		LogError((char*)temp);
		openings=1;	//reset the counter to avoid follow-up msgs (Duke,25.4.01)
	}
	strcpy(scpfilename,name);
	return true;
}


void closescript ()
{
	//AntiChrist
	if(strlen(scpfilename)>0)
	{
		if (scpfile != NULL)	// if it's not open, don't close it and don't crash ! (Duke, 6.5.2001)
			fclose(scpfile);

		scpfile = NULL ;

		openings--;
		if(openings<0)
		{
			strcpy((char*)temp,"WOLFPACK Script Manager ERROR: openings<-1!!!");
			LogError((char*)temp);
			openings=0;
		}
	} else
	{
		strcpy((char*)temp,"WOLFPACK Script Manager ERROR: no opened scripts to close!");
		LogError((char*)temp);
	}
}

void read1 () // Read script line without splitting parameters
{
	readscript();
	strcpy((char*)script1,(char*)temp);
}

void read2 () // Read line from script
{
	int i;
	readscript();
	i=0;
	script1[0]=0;
	script2[0]=0;

	while(temp[i]!=0 && temp[i]!=' ' && temp[i]!='=' && i<1024 )
	{
		i++;
	}

	strncpy((char*)script1, (char*)temp, i);
	script1[i]=0;
	if (script1[0]!='}' && temp[i]!=0) strcpy((char*)script2,(char*)( temp+i+1));
	return;
}

void readwscline () // Read line from *.WSC
{
	int i, valid=0;
	char c;
	temp[0]=0;
	while (!valid)
	{
		i=0;
		if (feof(wscfile)) return;
		c=(char)fgetc(wscfile);
		while (c!=10)
		{
			if (c!=13)
			{
				temp[i]=c;
				i++;
			}
			if (feof(wscfile) || i>=512 ) return;
			c=(char)fgetc(wscfile);
		}
		temp[i]=0;
		valid=1;
		if (temp[0]=='/' && temp[1]=='/') valid=0;
		if (temp[0]=='{') valid=0;
		if (temp[0]==0) valid=0;
		if (temp[0]==10) valid=0;
		if (temp[0]==13) valid=0;
	}
}

// reads the ENTIRE line, spaces + ='s DONT't tokenize the line, but sections, line feeds etc are ignored
// LB
void readFullLine ()
{
	int i=0;
	readwscline();
	script1[0]=0;
	while(temp[i]!=0  && i<1024 ) i++;
	strncpy((char*)script1,(char*) temp, i);
	script1[i]=0;	
	return;
}

void readw2 ()
{
	int i=0;

	readwscline();
	script1[0]=0;
	script2[0]=0;
	script3[0]=0;
	while(temp[i]!=0 && temp[i]!=' ' && temp[i]!='=' && i<1024 ) i++;
	strncpy((char*)script1,(char*) temp, i);
	script1[i]=0;
	if (script1[0]!='}' && temp[i]!=0) strcpy((char*)script2, (char*)(temp+i+1));
	return;
}


void readw3 ()
{
	int i=0,j;

	readwscline();
	script1[0]=0;
	script2[0]=0;
	script3[0]=0;
	while(temp[i]!=0 && temp[i]!=' ' && temp[i]!='=' && i<1024 ) i++;
	strncpy((char*)script1, (char*)temp, i);
	script1[i]=0;
	if (script1[0]=='}' || temp[i]==0) return;
	i++;
	j=i;
	while(temp[i]!=0 && temp[i]!=' ' && temp[i]!='=' && i <1024 ) i++;
	strncpy((char*)script2, (char*)(temp+j), i-j);
	script2[i-j]=0;
	strcpy((char*)script3, (char*)(temp+i+1));
}

void scriptlist(int x, int spc, int all)
{
/*	int pos, i;
	char file[512];

	openscript("items.scp");
	sprintf((char*)temp, "ITEMMENU %i", x);
	if (!i_scripts[items_script]->find((char*)temp))
	{
		closescript();
		if (n_scripts[custom_item_script][0]!=0)
		{
			openscript(n_scripts[custom_item_script]);
			if (!i_scripts[custom_item_script]->find((char*)temp))
			{
				closescript();//AntiChrist
				return;
			}
			else strcpy(file,n_scripts[custom_item_script]);
		} else return;
	} else strcpy(file, "items.scp");
	read1();

	unsigned long loopexit=0;
	do
	{
		read2();
		if (script1[0]!='}')
		{
			strcpy((char*)temp,(char*)script2);
			read2();
			if ((all)||(!(strcmp("ITEMMENU",(char*)script1))))
			{
				for (i=0;i<spc;i++) fprintf(lstfile, " ");
				fprintf(lstfile, "%s (%s %s)\n",temp,script1,script2);
				if (!(strcmp("ITEMMENU",(char*)script1)))
				{
					pos=ftell(scpfile);
					closescript();
					scriptlist(str2num(script2), spc+2, all);
					openscript(file);
					fseek(scpfile, pos, SEEK_SET);
					strcpy((char*)script1, "DUMMY");
				}
			}
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
	closescript();
	*/
}

void scriptmax(char *txt)
{
/*	int ok, i, x, highest, current;
	char str[512];
	Script *pScp;

	highest=-1;
	if (strcmp("ITEM",txt)&&strcmp("ITEMMENU",txt))
		pScp=i_scripts[npc_script];
	else
		pScp=i_scripts[items_script];
	sprintf(str, "SECTION %s ", txt);

	unsigned long loopexit=0;
	do
	{
		pScp->NextLine();
		ok=1;
		x=strlen(str);
		if (strlen((char*)temp)<x) x=strlen((char*)temp);
		for (i=0;i<x;i++) if (str[i]!=temp[i]) ok=0;
		if (ok==1)
		{
			current=str2num(&temp[x]);
			if (current==highest) clConsole.send("WARNING: Duplicate section %s %i\n",txt,highest);
			if (current>highest) highest=current;
		}
	}
	while ((strcmp("EOF",(char*)temp)) && (++loopexit < MAXLOOPS) );
	pScp->Close();
	fprintf(lstfile, " %s: %i\n", txt, highest);
	*/
}

void gettokennum(char * s, int num)
{
	int i, j;

	memset(&gettokenstr[0], 0, 255);

	i=0;

	unsigned long loopexit=0;
	while(num!=0 && (++loopexit < MAXLOOPS) )
	{
		if (s[i]==0)
		{
			num--;
		}
		else
		{
			if (s[i]==' ' && i!=0 && s[i-1]!=' ')
			{
				num--;
			}
			i++;
		}
	}
	j=0;

	loopexit=0;
	while(num!=-1 && (++loopexit < MAXLOOPS) )
	{
		if (s[i]==0)
		{
			num--;
		}
		else
		{
			if (s[i]==' ' && i!=0 && s[i-1]!=' ')
			{
				num--;
			}
			else
			{
				gettokenstr[j]=s[i];
				j++;
			}
			i++;
		}
	}
}

// Added by Krozy on 7-Sep-98
// New getstatskillvalue function.
// Takes a string, gets the tokens.
// If its one value - It returns that value.
// If its two values - It gets a random number between the values
short getstatskillvalue(char *stringguy) 
{
	char values[512];
	short lovalue,hivalue, retcode;

	strcpy(values, stringguy);
	gettokennum(values, 0);
	lovalue = static_cast<short>(str2num(gettokenstr));
	gettokennum(values, 1);
	hivalue = static_cast<short>(str2num(gettokenstr));

	if (hivalue) 
	{
		retcode = RandomNum(lovalue, hivalue);
	} else {
		retcode = lovalue;
	}
	return retcode;
}

