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
#include "debug.h"
#undef DBGFILE
#define DBGFILE "qsf.cpp"

static char tmsg[200] ;
void readqline(FILE* fp) // Read a line from a .qsf file
{
	int i, valid=0;
	char c;
	temp[0]=0;
	while (!valid)
	{
		i=0;
		if (feof(fp)) return;
		c=(char)fgetc(fp);
		while (c!=10)
		{
			if (c!=13)
			{
				temp[i]=c;
				i++;
			}
			if (feof(fp)) return;
			c=(char)fgetc(fp);
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

void readq2(FILE* fp)
{
	int i=0;
	
	readqline(fp);
	script1[0]=0;
	script2[0]=0;
	script3[0]=0;
	while(temp[i]!=0 && temp[i]!=' ' && temp[i]!='=') i++;
	strncpy((char*)script1, (char*)temp, i);
	script1[i]=0;
	if (script1[0]!='}' && temp[i]!=0) strcpy((char*)script2, (char*)(temp+i+1));
	return;
}

void readq3(FILE* fp)
{
	int i=0,j;
	
	readqline(fp);
	script1[0]=0;
	script2[0]=0;
	script3[0]=0;
	while(temp[i]!=0 && temp[i]!=' ' && temp[i]!='=') i++;
	strncpy((char*)script1,(char*) temp, i);
	script1[i]=0;
	if (script1[0]=='}' || temp[i]==0) return;
	i++;
	j=i;
	while(temp[i]!=0 && temp[i]!=' ' && temp[i]!='=') i++;
	strncpy((char*)script2, (char*)(temp+j), i-j);
	script2[i-j]=0;
	strcpy((char*)script3, (char*)(temp+i+1));
}

void loadQitem(FILE *fp,char *fn)
{
	int item_scp_no=0;
	Coord_cl pos;
	do
	{
		readq2(fp);
		if		(!(strcmp((char*)script1,"}"))) continue;
		else if	(!(strcmp((char*)script1,"ITEM_SCP"))) item_scp_no=str2num(script2);
		else if (!(strcmp((char*)script1,"X"))) pos.x=str2num(script2);
		else if (!(strcmp((char*)script1,"Y"))) pos.y=str2num(script2);
		else if (!(strcmp((char*)script1,"Z"))) pos.z=str2num(script2);
		else
		{
			sprintf(tmsg,"invalid keyword <%s> detected in file <%s>. Ignored!",script2,fn);
			LogWarning(tmsg);
		}
	}
	while (strcmp((char*)script1, "}"));

	if (item_scp_no > 0 && pos.x > 0 && pos.y > 0)	// seems to be a valid entry :)
	{
		P_ITEM pi=Items->CreateFromScript(-1,item_scp_no);
		pi->moveTo(pos);
	}
}

void loadQnpc(FILE *fp,char *fn)
{
	int npc_scp_no=0;
	Coord_cl pos;
	do
	{
		readq2(fp);
		if		(!(strcmp((char*)script1,"}"))) continue;
		else if	(!(strcmp((char*)script1,"NPC_SCP"))) npc_scp_no=str2num(script2);
		else if (!(strcmp((char*)script1,"X"))) pos.x=str2num(script2);
		else if (!(strcmp((char*)script1,"Y"))) pos.y=str2num(script2);
		else if (!(strcmp((char*)script1,"Z"))) pos.z=str2num(script2);
		else
		{
			sprintf(tmsg,"invalid keyword <%s> detected in file <%s>. Ignored!",script2,fn);
			LogWarning(tmsg);
		}
	}
	while (strcmp((char*)script1, "}"));

	if (npc_scp_no > 0 && pos.x > 0 && pos.y > 0)	// seems to be a valid entry :)
	{
		signed char zmap = Map->MapElevation(pos);
		Npcs->AddNPCxyz(-1,npc_scp_no,0,pos.x,pos.y,zmap);
	}
}

void qsfLoad(char *fn, short depth) // Load a quest script file
{
	if (depth>9)
	{
		LogWarning("nesting level has reached 10! Probably recursive includes! include aborted!");
		return;
	}
	FILE *fp=fopen(fn, "r");
	if(fp==NULL)
	{
		sprintf(tmsg,"<%s> not found...nothing to load",fn);
		LogWarning(tmsg);
		return;
	}
	else
	{
		sprintf(tmsg,"including <%s>",fn);
		LogMessage(tmsg);
	}

	do
	{
		readq2(fp);
		if (!(strcmp((char*)script1, "SECTION")))
		{
			if		(!(strcmp((char*)script2, "QITEM"))) loadQitem(fp,fn);
			else if	(!(strcmp((char*)script2, "QNPC"))) loadQnpc(fp,fn);
			else
			{
				sprintf(tmsg,"invalid keyword <%s> detected in file <%s>",script2,fn);
				LogWarning(tmsg);
			}
		}
		else
			if (!(strcmp((char*)script1, "#include")))
				qsfLoad((char*)script2,depth+1);
	}
	while (strcmp((char*)script1, "EOF") && !feof(fp) );
	fclose(fp);
}
