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

#include "admin.h"



cAdmin::cAdmin()
{

}

void cAdmin::ReadIni()
{
	int keep,loopexit=0;
	unsigned int uiTempi;

	infile = fopen("wolfpack.ini","r");
	if (infile==NULL)
	{
		clConsole.send("ERROR: WOLFPACK.INI not found...\n");
		error=1;
		keeprun=0;
		return;
	}

	keep=1;
	serv.clear();
	do
	{
		ReadString();
		ServerList_st currentserv; // Servers list

		if ((temp[0]=='#')&&(temp[1]=='#')&&(temp[2]=='#'))
		{
			keep=0;
		}
		else
		{
			currentserv.sServer = temp;
			ReadString();
			currentserv.sIP = temp;
			ReadString();
			currentserv.uiPort = (short) str2num(temp);

			serv.push_back(currentserv);
		}
	}

	while ( (keep) &&  (serv.size()<MAXSERV) );
	keep=1;

	if (serv.size()>=MAXSERV) { keeprun=0; error=1; return; }

	clConsole.send("servers: %i\n",serv.size());

	startcount=0; 
	loopexit=0;
	do
	{
		ReadString();
		if ((temp[0]=='#')&&(temp[1]=='#')&&(temp[2]=='#'))
		{
			keep=0;
		}
		else
		{
			  for (uiTempi=0;uiTempi<=strlen((char*)temp);uiTempi++) start[startcount][0][uiTempi]=temp[uiTempi];
			  ReadString();
			  for (uiTempi=0;uiTempi<=strlen((char*)temp);uiTempi++) start[startcount][1][uiTempi]=temp[uiTempi];
			  ReadString();
			  for (uiTempi=0;uiTempi<=strlen((char*)temp);uiTempi++) start[startcount][2][uiTempi]=temp[uiTempi];
			  ReadString();
			  for (uiTempi=0;uiTempi<=strlen((char*)temp);uiTempi++) start[startcount][3][uiTempi]=temp[uiTempi];
			  ReadString();
			  for (uiTempi=0;uiTempi<=strlen((char*)temp);uiTempi++) start[startcount][4][uiTempi]=temp[uiTempi];          
			  startcount++;
		}
	}
	while ( (keep) && (++loopexit < MAXLOOPS) );

			  
	startcount=9;			  
    
	ReadString();
	strcpy(Map->mapname, temp);
	ReadString();
	strcpy(Map->sidxname, temp);
	ReadString();
	strcpy(Map->statname, temp);
	ReadString();
	strcpy(Map->vername, temp);
	ReadString();
	strcpy(Map->tilename, temp);
	ReadString();
	strcpy(Map->multiname, temp);
	ReadString();
	strcpy(Map->midxname, temp);
	ReadString();
	strcpy(saveintervalstr, temp);
	saveinterval=atoi(saveintervalstr);
	ReadString();
	heartbeat=false;
	if (temp[0]=='1') heartbeat=true;
	ReadString();
	strcpy((char*)defaultpriv1str, temp);
	defaultpriv1=hstr2num(defaultpriv1str);
	ReadString();
	strcpy((char*)defaultpriv2str, temp);
	defaultpriv2=hstr2num(defaultpriv2str);

	// name resovling of server-address, LB 7-JULY 2000

	unsigned int i;
	unsigned long ip;		
	sockaddr_in m_sin;
	hostent *hpe;
	
    m_sin.sin_family = AF_INET;   
	
	for (i = 0; i < serv.size(); i++)
	{	  
		ip = inet_addr(serv[i].sIP.c_str());
		
		if (ip == INADDR_NONE) // adresse-name instead of ip adress given ! trnslate to ip string
		{
			clConsole.send("host: %s\n", serv[i].sIP.c_str());
			hpe = gethostbyname(serv[i].sIP.c_str());
			
			if (hpe == NULL)  
			{ 				  
#if	defined(__unix__)
				// We should be able to use the xti error functions, cant find them so...
				// sprintf(temp,"warning: %d resolving name: %s\n", t_srerror(t_errno),name) ;
				sprintf((char*)temp, "warning: Error desolving name: %s : %s\n", serv[i].sIP.c_str(), hstrerror(h_errno));
#else 
				sprintf((char*)temp, "warning: %d resolving name: %s\n", WSAGetLastError(), serv[i].sIP.c_str());
#endif	   
				LogWarning((char*)temp);
				LogWarning("switching to localhost\n");
				
				serv[i].sIP = "127.0.0.1";
			}
			else
			{
				memcpy((char*)&(m_sin.sin_addr), hpe->h_addr, hpe->h_length);
				
				serv[i].sIP = sock_ntop ( m_sin );
			} // end else resolvable
		}
		clConsole.send("server IP: %s\n", serv[i].sIP.c_str());	
		clConsole.send("server port: %i\n", serv[i].uiPort);
	} // end server loop
	
	// end name resovling

	fclose(infile);
	infile = NULL;
}

void cAdmin::ReadString()
{
	int i=0;
	char c;

	while ((c= (char) fgetc(infile)) != 10) // no inf-loop possible
	{
		if (c!=13)
		{
			temp[i]=c;
			i++;
		}
		if (feof(infile))
		{
			LogError("ERROR: readstring() unexpectedly reached EOF.\n");
			LogError("There is an error at the end of you wolfpack.ini file.\n");
			clConsole.send("Fatal error @ wolfpack.ini\n");
			#if defined(__unix__)
			endScrn();
			#endif
			exit(1);
		}
	}
	temp[i]=0;
}
