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

// Newbie Stuff Code -- By McCleod

#include "wolfpack.h"
#include "debug.h"

#undef  DBGFILE
#define DBGFILE "newbie.cpp"

void read3() // used for newbie items
{
	readscript();
	strcpy((char*)script3, (char*)temp);
}

void read4() // used for newbie items
{
	int i, loopexit = 0;
	readscript();
	i = 0;
	script3[0] = 0;
	script4[0] = 0;
	while ((temp[i] != 0) &&(temp[i] != ' ') &&(++loopexit < MAXLOOPS))
	{
		i++;
	}
	strncpy((char*)script3, (char*)temp, i);
	script3[i] = 0;
	if (script1[0] != '}' && temp[i] != 0)
		strcpy((char*)script4, (char*)(temp + i + 1));
	return;
}

int nextbestskill(CHARACTER m, int bstskll)  // Which skill is the second highest
{
	int i, a = 0, b = 0;

	P_CHAR pc = MAKE_CHARREF_LRV(m, 0);
	
	for (i = 0; i < TRUESKILLS; i++) 
	{
		if (pc->baseskill[i] > b && pc->baseskill[i] < pc->baseskill[bstskll] && bstskll != i )
		{
			a = i;
			b = pc->baseskill[i];
		}
		if ( pc->baseskill[i] == pc->baseskill[bstskll] && bstskll != i)
		{
			a = i;
			b = pc->baseskill[i];
		}
	}
	return a;
}


void newbieitems(CHARACTER c)
{
	int first, second, third, storeval, itemaddperskill, loopexit = 0;
	char sect[512];
	char whichsect[15];
	long int pos;
	int s;
	for (s = 0; s < now; s++)
	{
		if (c == currchar[s])
			break;
	}

	P_CHAR pc = MAKE_CHARREF_LR(c);
	first = bestskill(c);
	second = nextbestskill(c, first);
	third = nextbestskill(c, second);
	if (pc->baseskill[third] < 190)
		third = 46;

	for (itemaddperskill = 1; itemaddperskill <= 4; itemaddperskill++)
	{
		switch (itemaddperskill)
		{
			case 1: sprintf(whichsect, "BESTSKILL %i", first);	break;
			case 2: sprintf(whichsect, "BESTSKILL %i", second);	break;
			case 3: sprintf(whichsect, "BESTSKILL %i", third);	break;
			case 4: strcpy(whichsect, "DEFAULT");				break;
			default:
				clConsole.send("ERROR: Fallout of switch statement without default. newbie.cpp, newbieitems()/n"); // Morrolan
		}
		openscript("newbie.scp");
		sprintf(sect, whichsect);
		if (!i_scripts[newbie_script]->find(sect)) 
		{
			closescript();
			return;
		}
		do
		{
			read2();
			if (script1[0] != '}')
			{ 
				if (!(strcmp("PACKITEM", (char*)script1)))
				{
					storeval = str2num(script2);
					pos = ftell(scpfile);
					closescript();
					
					P_ITEM pi_n = Items->SpawnItemBackpack2(s, storeval, 0); // Tauriel 11-24-98
					if (pi_n != NULL)
						pi_n->priv |= 0x02; // Mark as a newbie item
					strcpy((char*)script1, "DUMMY");
					openscript("newbie.scp");
					fseek(scpfile, pos, SEEK_SET);
				}
				else if (!strcmp("BANKITEM", (char*)script1))
				{
					storeval = str2num(script2);
					pos = ftell(scpfile);
					closescript();
				
					P_ITEM pi = Items->SpawnItemBank(c, storeval); // Tauriel 11-24-98
					if (pi != NULL)
						pi->priv |= 0x02; // Mark as a newbie item
					strcpy((char*)script1, "DUMMY");
					openscript("newbie.scp");
					fseek(scpfile, pos, SEEK_SET);
				}
				else if (!strcmp("GOLD", (char*)script1))
				{
					goldamount = str2num(script2);
					pos = ftell(scpfile);
					closescript();
				    P_ITEM pi_n =Items->SpawnItem(s,DEREF_P_CHAR(pc),goldamount,"#",1,0x0E,0xED,0,0,1,0);
	                if(pi_n==NULL) return;
					pi_n->priv |= 0x02; // Mark as a newbie item
					strcpy((char*)script1, "DUMMY");
					openscript("newbie.scp");
					fseek(scpfile, pos, SEEK_SET);
				}
			}  
		}	
		while ((script1[0] != '}') &&(++loopexit < MAXLOOPS));
	    closescript();
	}
}
 

  
