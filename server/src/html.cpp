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
#include "utilsys.h"
#include "debug.h"
#undef  DBGFILE
#define DBGFILE "html.cpp"


void offlinehtml()//HTML
{
	char sect[512], hfile[512];
	unsigned int total,hr,min,sec,loopexit=0; //bugfix LB
	FILE *html;
	
	total=(uiCurrentTime-starttime)/MY_CLOCKS_PER_SEC;
	hr=total/3600;
	total-=hr*3600;
	min=total/60;
	total-=min*60;
	sec=total;

	strcpy(sect,"OFFLINE_PAGE");
	openscript("htmlstrm.scp");
	if(!i_scripts[html_script]->find(sect))
	{
		closescript();
		return;
	}
	read1();
	strcpy(hfile, script1);
	html = fopen(hfile, "w");
	if (html == NULL) // LB
	{ 
		clConsole.send("Could not create html file, plz check htmlstrm.scp\n"); 
		closescript();
		return;
	}

	do {
		read2();
		if(!(strcmp((char*)script1,"LINE"))) fprintf(html,(char*)script2);
		else if(!(strcmp((char*)script1,"TIME"))) fprintf(html,(getRealTimeString()).c_str());
		else if(!(strcmp((char*)script1,"UPTIME"))) fprintf(html,"%i:%i:%i",hr,min,sec);
	} while( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	fclose(html);
	closescript();
}

void updatehtml()//HTML
{
	double eps=0.00000000001;
	char sect[512],hfile[512],sh[3],sm[3],ss[3];
	int a, n=0;
	//unsigned long int ip;
	int gm=0,cns=0,ccount=0,icount=0,npccount=0,loopexit=0;
	unsigned long int total,hr,min,sec; // bugfix, LB
	FILE *html;

	strcpy(sect,"ONLINE_PAGE");
	openscript("htmlstrm.scp");
	if(!i_scripts[html_script]->find(sect))
	{
		closescript();
		return;
	}
	read1();
	strcpy(hfile, script1);

	//html=fopen(hfile,"w+"); 
	//a=remove(hfile);
	//clConsole.send("html-a: %i %s\n",a,hfile);

	html=fopen(hfile,"w");  // remove old one first 
	                        
	if (html == NULL) // LB
	{ 
		clConsole.send("Could not create html file, please check htmlstrm.scp\n");
		closescript();
		return;
	}


	do {
		read2();
		if(!(strcmp((char*)script1,"LINE"))) fprintf(html,"%s\n",script2);
		else if(!(strcmp((char*)script1,"TIME"))) fprintf(html,"%s <BR>",(getRealTimeString()).c_str());
		else if(!(strcmp((char*)script1,"NOW")))
		{
			if(online(currchar[n])) //bugfix LB
			{
				fprintf(html, currchar[n]->name.c_str());
				n++;
			}
		}
		else if(!(strcmp((char*)script1,"WHOLIST")))
		{
			a=0;
			for (n=0;n<now;n++)
			{ 
				if (online(currchar[n])) // bugfix, LB
				{
					a++;
					fprintf(html,"%i) %s <BR>\n",a, currchar[n]->name.c_str()); // bugfix lb
				}
			}
		}
		else if(!(strcmp((char*)script1,"NOWNUM"))) fprintf(html,"%i",now);
		else if(!(strcmp((char*)script1,"ACCOUNTNUM"))) fprintf(html,"%i",Accounts->Count());
		else if(!(strcmp((char*)script1,"CHARCOUNT")))
		{
			if(ccount==0)
			{
				npccount=0;
				AllCharsIterator iter_char;
				for (iter_char.Begin(); !iter_char.atEnd(); iter_char++)
				{
					P_CHAR toCheck = iter_char.GetData();
					if(!toCheck->free) 
					{
						ccount++;
						if(toCheck->isNpc()) 
							npccount++;
					}
				}
			}
			fprintf(html,"%i",ccount);
		}
		else if(!(strcmp((char*)script1,"NPCS")))
		{
			if(npccount==0)
			{
				ccount=0;
				AllCharsIterator iter_char;
				for (iter_char.Begin(); !iter_char.atEnd(); iter_char++)
				{
					P_CHAR toCheck = iter_char.GetData();
					if(!toCheck->free) 
					{
						ccount++;
						if(toCheck->isNpc()) 
							npccount++; //bugfix LB
					}
				}
			}
			fprintf(html,"%i",npccount);
		}
		else if(!(strcmp((char*)script1,"ITEMCOUNT"))) 
		{
			icount=0;
			AllItemsIterator iter_items;
			for(iter_items.Begin(); !iter_items.atEnd(); iter_items++)
			{
				P_ITEM pi = iter_items.GetData();
				if(!pi->free) 
					icount++;
			}
			fprintf(html,"%i",icount);
		}
		else if(!(strcmp((char*)script1,"UPTIME")))
		{
			total=(uiCurrentTime-starttime)/MY_CLOCKS_PER_SEC;
			hr=total/3600;
			if(hr<10 && hr<=60) sprintf(sh,"0%i",hr);
			else sprintf(sh,"%i",hr);
			total-=hr*3600;
			min=total/60;
			if(min<10 && min<=60) sprintf(sm,"0%i",min);
			else sprintf(sm,"%i",min);
			total-=min*60;
			sec=total;
			if(sec<10 && sec <=60) sprintf(ss,"0%i",sec);
			else sprintf(ss,"%i",sec);
			fprintf(html,"%s:%s:%s",sh,sm,ss);	
		}
		else if(!(strcmp((char*)script1,"IP")))
		{
			//ip=inet_addr(serv[str2num(script2)-1][1]);
			fprintf(html, serv[str2num(script2)-1].sIP.c_str());
		}
		else if(!(strcmp((char*)script1,"GMNUM")))
		{
			if(gm==0)
			{
				register int a;
				for(a = 0; a < now; ++a)
				{
					if ( !currchar[a] )
						continue;
					if( perm[a] && currchar[a]->isGM() ) gm++;
					else if( perm[a] && currchar[a]->isCounselor() ) cns++; //bugfix LB
				}
			}
			fprintf(html,"%i",gm);
		}
		else if(!(strcmp((char*)script1,"CNSNUM")))
		{
			if(cns==0)
			{
				for(a=0;a<now;a++)
				{
					if(currchar[a]->isGM() && perm[a]) gm++;
					else if(currchar[a]->isCounselor() && perm[a]) cns++; //bugfix LB
				}
			}
			fprintf(html,"%i",cns);
		}
		else if(!(strcmp((char*)script1,"PDUMP"))) 
		{
			fprintf(html,"Network code: %fmsec [%i samples] <BR>",(float)((float)networkTime/(float)networkTimeCount),  networkTimeCount);
			fprintf(html,"Timer code: %fmsec [%i samples] <BR>" , (float)((float)timerTime/(float)timerTimeCount) , timerTimeCount);
			fprintf(html,"Auto code: %fmsec [%i samples] <BR>" , (float)((float)autoTime/(float)autoTimeCount) , autoTimeCount);
			fprintf(html,"Loop Time: %fmsec [%i samples] <BR>" , (float)((float)loopTime/(float)loopTimeCount) , loopTimeCount);
			fprintf(html,"Characters: %i/Dynamic    Items: %i/Dynamic <BR>" , cCharsManager::getInstance()->size(), cItemsManager::getInstance()->size());
			if (!(loopTime <eps ||  loopTimeCount<eps)) //Bugfix LB
				fprintf(html,"Simulation Cycles: %f per sec <BR>" , (1000.0*(1.0/(float)((float)loopTime/(float)loopTimeCount))));
			else fprintf(html,"Simulation Cylces: too fast to be measured <BR>");
			
		}
		else if(!(strcmp((char*)script1,"SIMCYC"))) // bugfix LB
		{
			if (!(loopTime <eps ||  loopTimeCount<eps))
				fprintf(html,"%f" , (1000.0*(1.0/(float)((float)loopTime/(float)loopTimeCount))));
			else fprintf(html,"too fast to be measured");
		}
		else if(!(strcmp((char*)script1,"UDTIME")))	fprintf(html,"%f",(float)(SrvParms->html/60));

		#ifndef __unix__
			if(!(strcmp((char*)script1,"VER"))) fprintf(html,"%s %s [WIN32]", wp_version.betareleasestring.c_str(), wp_version.verstring.c_str() );
		#else
			if(!(strcmp((char*)script1,"VER"))) fprintf(html,"%s %s [LINUX]",  wp_version.betareleasestring.c_str(), wp_version.verstring.c_str() );
		#endif
	} while( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	fclose(html);
	closescript();
}


