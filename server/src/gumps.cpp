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
#include "im.h"
#include "prototypes.h"
#include "globals.h"
#include "SndPkg.h"
#include "gumps.h"
#include "guildstones.h"
#include "srvparams.h"
#include "menuactions.h"
#include "wpdefmanager.h"

#include "debug.h"

#undef  DBGFILE
#define DBGFILE "gumps.cpp"

void cGump::Button(int s, int button, SERIAL serial, char type)
{
	int i;
	P_CHAR pc_currchar = currchar[s];
	
    // if ((button)==0 || (button==1)) clConsole.send("gump-menu, type# %i closed\n",type); // lord bin
	if(button>10000) {
		i=button-10000;
		Menu(s, i, NULL);
		return;
	}

	switch (type)
	{
	case 1: // Item
		{
			switch( button )
			{
			case 0:
			case 1: break;
			case 2:		entrygump( s, serial, type, button, 50, "Enter a new name for the item. (# = default name)" );	break;
			case 3:		entrygump( s, serial, type, button, 4, "Enter the new ID number for the item in hex." );			break;
			case 4:		entrygump( s, serial, type, button, 4, "Enter the new hue for the item in hex." );				break;
			case 5:		entrygump( s, serial, type, button, 4, "Enter the new X coordinate for the item in decimal." );	break;
			case 6:		entrygump( s, serial, type, button, 4, "Enter the new Y coordinate for the item in decimal." );	break;
			case 7:		entrygump( s, serial, type, button, 4, "Enter the new Z coordinate for the item in decimal." );	break;
			case 8:		entrygump( s, serial, type, button, 4, "Enter the new type for the item in decimal." );			break;
			case 9:		entrygump( s, serial, type, button, 4, "Enter the new itemhand for the item in decimal." );		break;//Xuri
			case 10:	entrygump( s, serial, type, button, 4, "Enter the new layer for the item in decimal." );			break;
			case 11:	entrygump( s, serial, type, button, 4, "Enter the new amount for the item in decimal." );			break;
			case 12:	entrygump( s, serial, type, button, 8, "Enter the new More for the item in hex." );				break;
			case 13:	entrygump( s, serial, type, button, 8, "Enter the new MoreB for the item in hex." );				break;
			case 14:	entrygump( s, serial, type, button, 1, "Enter the new stackable toggle for the item. (0/1)" );	break;
			case 15:	entrygump( s, serial, type, button, 1, "Enter the new dyeable toggle for the item. (0/1)" );		break;
			case 16:	entrygump( s, serial, type, button, 1, "Enter the new corpse toggle for the item. (0/1)" );		break;
			case 17:	entrygump( s, serial, type, button, 5, "Enter the new LODAMAGE value for the item in decimal." );	break;//|
			case 18:	entrygump( s, serial, type, button, 5, "Enter the new HIDAMAGE value for the item in decimal." );	break;//| both of these replace the old "attack value" (Xuri)
			case 19:	entrygump( s, serial, type, button, 5, "Enter the new defence value for the item in decimal." );	break;
			case 20:	entrygump( s, serial, type, button, 1, "Enter the new magic value for the item in decimal." );	break;
			case 21:	entrygump( s, serial, type, button, 1, "Enter the new visible value for the item in decimal." );	break;
			case 22:	entrygump( s, serial, type, button, 5, "Enter the new HP value for the item in decimal." );		break;
			case 23:	entrygump( s, serial, type, button, 5, "Enter the new Max HP value for the item in decimal." );	break;
			case 24:	entrygump( s, serial, type, button, 5, "Enter the new Speed value for the item in decimal." );	break;
			case 25:	entrygump( s, serial, type, button, 5, "Enter the new Rank value for the item in decimal." );		break;
			case 26:	entrygump( s, serial, type, button, 5, "Enter the new Value for the item in decimal." );			break;
			case 27:	entrygump( s, serial, type, button, 5, "Enter the new Good value for the item in decimal." );		break;
			case 28:	entrygump( s, serial, type, button, 5, "Enter the new Made Skill value for the item in decimal." );	break;
			case 29:	entrygump( s, serial, type, button, 50, "Enter the new Creator name for the item." );				break;
			default:	clConsole.send( "Unknown button pressed %i", button );	break;																	break;
			}
			break;
		}
	case 2:
		{
			switch( button )
			{
			case 0:
			case 1: break;
			case 2:	 entrygump( s, serial, type, button, 50, "Enter a new Name for the character." );							break;
			case 3:	 entrygump( s, serial, type, button, 50, "Enter a new Title for the character." );							break;
			case 4:	 entrygump( s, serial, type, button, 4, "Enter a new X coordinate for the character in decimal." );			break;
			case 5:	 entrygump( s, serial, type, button, 4, "Enter a new Y coordinate for the character in decimal." );			break;
			case 6:	 entrygump( s, serial, type, button, 4, "Enter a new Z coordinate for the character in decimal." );			break;
			case 7:	 entrygump( s, serial, type, button, 3, "Enter a new Direction for the character in decimal." );				break;
			case 8:	 entrygump( s, serial, type, button, 4, "Enter a new Body Type for the character in hex." );					break;
			case 9:	 entrygump( s, serial, type, button, 4, "Enter a new Skin Hue for the character in hex." );					break;
			case 10: entrygump( s, serial, type, button, 3, "Enter a new Defence value for the character in decimal." );		break;
			case 11: entrygump( s, serial, type, button, 1, "Enter a new Hunger value for the character in decimal(0-6).");		break;			
			case 12: entrygump( s, serial, type, button, 5, "Enter a new Strength value for the character in decimal.");		break;			
			case 13: entrygump( s, serial, type, button, 5, "Enter a new Dexterity value for the character in decimal.");		break;			
			case 14: entrygump( s, serial, type, button, 5, "Enter a new Intelligence value for the character in decimal.");	break;
			case 15: entrygump( s, serial, type, button, 5, "Enter a new Karma value for the character in decimal.");	break;
			case 16: entrygump( s, serial, type, button, 5, "Enter a new Fame value for the character in decimal.");	break;
			case 17: entrygump( s, serial, type, button, 5, "Enter a new Kills value for the character in decimal.");	break;
			case 18: entrygump( s, serial, type, button, 5, "Enter a new Jail time for the character in seconds");	break;
			}
			break;
		}
	case 4:			// Wholist
		if(button<200)	
		{		
			button-=7;
			pc_currchar->making=button;
			Commands->WhoCommand(s,type,button);
		}
		else
		{ 
			i=pc_currchar->making;
			if (i<0)
			{
			  sysmessage(s,"selected character not found");
			  return;
			}
			serial=whomenudata[i];
		    P_CHAR pc_c = FindCharBySerial( serial ); // find selected char ...
		    if (pc_c == NULL)
			{
			  sysmessage(s,"selected character not found");
			  return;
			}
			switch(button)
			{
			case 200://gochar 
				doGmMoveEff(s); 	// have a flamestrike at origin and at player destination point 	//Aldur
				
				pc_currchar->moveTo(pc_c->pos); 
				teleport((currchar[s])); 
				
				doGmMoveEff(s); 
				break;
			case 201://xtele
				pc_c->moveTo(pc_currchar->pos);
				teleport((pc_c));
				
				break;
			case 202://jail char
				if(currchar[s]==pc_c)
				{
					sysmessage(s,"You cannot jail yourself!");					 
					break;
				}
				else
				{
					addmitem[s] = SrvParams->default_jail_time();
					Targ->JailTarget (s,pc_c->serial);
					break;
				}						 
			case 203://release				 
				Targ->ReleaseTarget(s,pc_c->serial);
				break;
			case 204:
				if(pc_c==currchar[s])
				{
					sysmessage(s,"You cannot kick yourself");
					break; // lb
				}
				else
				{
					if (!online(pc_c)) 
					{ 
						sysmessage(s,"you cant kick an offline player");
						break;
					}
					sysmessage(s, "Kicking player");				
					Network->Disconnect(calcSocketFromChar(pc_c));
					break;
				}
			default:
				clConsole.send("ERROR: Fallout of switch statement without default. WOLFPACK.cpp, gumpbutton()\n"); //Morrolan
			}
			return;
		}
		break;
	case 5:           // House
		{
			SERIAL is = calcserial(addid1[s],addid2[s],addid3[s],addid4[s]);
			P_ITEM pj = FindItemBySerial( is );
			if(pj != NULL) 
			{
				is=calcserial(pj->more1, pj->more2, pj->more3, pj->more4);
				pj = FindItemBySerial( is );
			}

			i = (buffer[s][21] << 8) + buffer[s][22];
			if (button != 20 && button != 2)
			{
				addid1[s] = static_cast<unsigned char>((pj->serial&0xFF000000)>>24);
				addid2[s] = static_cast<unsigned char>((pj->serial&0x00FF0000)>>16);
				addid3[s] = static_cast<unsigned char>((pj->serial&0x0000FF00)>>8);
				addid4[s] = static_cast<unsigned char>((pj->serial&0x000000FF));
			}
			switch (button)
			{
			case 20: // Change house sign's appearance
				if (i>0)
				{
					buffer[s][7] = addid1[s];
					buffer[s][8] = addid2[s];
					buffer[s][9] = addid3[s];
					buffer[s][10] = addid4[s];
					addx[s] = buffer[s][21];
					addy[s] = buffer[s][22];
					Targ->IDtarget(s);
					sysmessage(s, "House sign changed.");
				}
				return;
			case 0:
				return;
			case 1:  // change house name
				target(s, 0, 1, 0, 11, "Select the house sign to rename it.");
				return;
			case 2:  // Bestow ownership upon someone else
				target(s, 0, 1, 0, 227, "Select person to transfer ownership to.");
				return;
			case 3:  // Turn house into a deed
				deedhouse(s, pj);
				return;
			case 4:  // Kick someone out of house
				target(s, 0, 1, 0, 228, "Select person to eject from house.");
				return;
			case 5:  // Ban somebody
				target(s, 0, 1, 0, 229, "Select person to ban from house.");
				return;
			case 6:
			case 8: // Remove someone from house list
				target(s, 0, 1, 0, 231, "Select person to remove from ban list.");
				return;
			case 7: // Make someone a friend
				target(s, 0, 1, 0, 230, "Select person to make a friend of the house.");
				return;
			default:
				sprintf((char*)temp, "HouseGump Called - Button=%i", button);
				sysmessage(s, (char*)temp);
				return;
			}
			break;
		}
	}
}

void cGump::Input(int s)
{
	char type, index ;
	char *text;
	int c1,body,b,k,serial;
	
	type=buffer[s][7];
	index=buffer[s][8];
	text=(char*)&buffer[s][12];
	serial = LongFromCharPtr((unsigned char*)&buffer[s][3]);
	P_CHAR pc_currchar = currchar[s];

	if (type == 100)
	{
		cGuildStone* pStone = dynamic_cast<cGuildStone*>(FindItemBySerial(pc_currchar->guildstone));
		if ( pStone != NULL)
			pStone->GumpInput(s,type,index,text);
	}

	if (type == 1 && (pc_currchar->isGM()))//AntiChrist
	{
		P_ITEM pj = FindItemBySerial( serial );
		if ( pj == NULL ) return; //lb
		if (buffer[s][9]==0)
		{
			tweakmenu(s, pj->serial);
			return;
		}
		switch( index )
		{
		case 2:		pj->setName( (char*)text );						break;	 // Name
		case 3:		k = hex2num( text );	
					pj->setId(k);
					break;	 // ID
		case 4:		k = hex2num( text );	
					pj->setColor( k );
					break;	// Hue
		case 5:		k = str2num( text );	pj->pos.x = k;		break;	// X
		case 6:		k = str2num( text );	pj->pos.y = k;		break;	// Y
		case 7:		k = str2num( text );	pj->pos.z = k;		break;	// Z
		case 8:		k = str2num( text );	pj->type = k;		break;	 // Type
		case 9:		k = str2num( text );	pj->itmhand = k;	break;	// Itemhand - added by Xuri
		case 10:	k = str2num( text );	pj->layer = k;		break;	// Layer
		case 11:	k = str2num( text );	pj->setAmount( k );		break;	// Amount
		case 12:	k = hex2num( text );	// More
					pj->more1 = (unsigned char)(k>>24);
					pj->more2 = (unsigned char)(k>>16);
					pj->more3 = (unsigned char)(k>>8);
					pj->more4 = (unsigned char)(k%256);
					break;
		case 13: 	k = hex2num( text );	// MoreB
					pj->moreb1 = (unsigned char)(k>>24);
					pj->moreb2 = (unsigned char)(k>>16);
					pj->moreb3 = (unsigned char)(k>>8);
					pj->moreb4 = (unsigned char)(k%256);
					break;
		case 14: 	k = str2num( text );	pj->pileable = k;	break;	// Pileable
		case 15:	k = str2num( text );	pj->dye = k;		break;	// Dye
		case 16:	k = str2num( text );	pj->corpse = k;		break;	// Corpse
		case 17:	k = str2num( text );	pj->lodamage = k;	break;	// LoDamage
		case 18:	k = str2num( text );	pj->hidamage = k;	break;	// HiDamage
		case 19:	k = str2num( text );	pj->def = k;		break;	// Def
		case 20:	k = str2num( text );	pj->magic = k;		break;	// Magic
		case 21:	k = str2num( text );	pj->visible = k;	break;	// Visible
		case 22:	k = str2num( text );	pj->hp = k;			break;	// Current Hitpoints
		case 23:	k = str2num( text );	pj->maxhp = k;		break;	// MAX Hitpoints
		case 24:	k = str2num( text );	pj->spd = k;		break;	// Speed (for Combat)
		case 25:	k = str2num( text );	pj->rank = k;		break;	// Rank
		case 26:	k = str2num( text );	pj->value = k;		break;	// Value
		case 27:	k = str2num( text );	pj->good = k;		break;	// Good(for Adv.Trade system)
		case 28:	k = str2num( text );	pj->madewith = k;	break;	// Made Skill
		case 29:	pj->creator =  (char*)text;					break;	// Creator
		}
	
		RefreshItem(pj); //AntiChrist
		tweakmenu(s, pj->serial);
 }
 //if (type==2 && (pc_currchar->priv|1))//uhm?? what was that |1?! i think it should be &1...AntiChrist
 if (type==2 && (pc_currchar->isGM()))//AntiChrist
 {
	P_CHAR pc_j = FindCharBySerial( serial );
	if (pc_j == NULL) return;
	 if (buffer[s][9]==0)
	 {
		 tweakmenu(s, pc_j->serial);
		 return;
	 }
	 switch( index )
		{
		case 2:		pc_j->name = (char*)text;					break;	// Name
		case 3:		pc_j->setTitle( text );						break;	// Title
		case 4:		k = str2num( text );	pc_j->pos.x = k;	break;	// X
		case 5:		k = str2num( text );	pc_j->pos.y = k;	break;	// Y
		case 6:		k = str2num( text ); 	pc_j->pos.z = k;	pc_j->dispz = k;	break;	// Z
		case 7:		k = str2num( text );	pc_j->dir = k&0x0F;	break;// make sure the high-bits are clear // Dir
		case 8: // Body
			k = hex2num( text );
			if (k>=0x000 && k<=0x3e1) // lord binary, body-values >0x3e crash the client			                       
			{ 
				pc_j->setId(k);
				pc_j->xid = pc_j->id();
				c1 = pc_j->skin();						// transparency for mosnters allowed, not for palyers, 
		                                              // if polymorphing from monster to player we have to switch from transparent to semi-transparent
		                                              // or we have that sit-down-client crash
               b=c1&0x4000; 
			   if (b==16384 && (k >=0x0190 && k<=0x03e1))
			   {
				  if (c1!=0x8000)
				  {
                     pc_j->setSkin(0xF000);
					 pc_j->setXSkin(0xF000);
				  }
			   }			
			}
			break;
		case 9:		k = hex2num( text );		// Skin
					body=(pc_j->id1<<8)+pc_j->id2;

                    b=k&0x4000; // Lord binary --> that touchy transparency bit ! 
		                        // fixes a client crash with transparently dyed skin.
		                        // ( only with body values 190-03e1 --> no monster )
		                        // if such a char wants to sit down the client crashes.
		                        // this is a CLIENT(!) BUG I cant do anythhing  

		           if (b==16384 && (body >=0x0190 && body<=0x03e1)) k=0xf000; // but assigning the only "transparent" value that works, namly semi-trasnparency.

                   if (k!=0x8000) // 0x8000 also crashes client ...
				   {	
		              pc_j->setXSkin(k);
					  pc_j->setSkin(k);
				   }
					break;
		case 10:	k = str2num( text );	pc_j->def = k;		break;	// Defence
		case 11:	k = str2num( text );	pc_j->hunger = k;	break;	// Hunger
		case 12:	k = str2num( text );	pc_j->st = k;		break;	// Strength
		case 13:	k = str2num( text );	pc_j->setDex(k);	break;	// Dexterity
		case 14:	k = str2num( text );	pc_j->in = k;		break;	// Intelligence
		case 15:	k = str2num( text );	pc_j->karma = k;	break;	// Karma
		case 16:	k = str2num( text );	pc_j->fame = k;		break;	// Fame
		case 17:	k = str2num( text );	pc_j->kills = k;	break;	// Kills
	 	case 18:    k = str2num( text );	pc_j->jailsecs = k;	break;	// Kills
		}

	 teleport((pc_j));
	 tweakmenu(s, pc_j->serial);
	}

}

void cGump::Menu(UOXSOCKET s, int m, P_ITEM it)
{
	char sect[512];
	unsigned long loopexit=0;
	short int length, length2, textlines=0;
	unsigned int i;
	bool house_gump=false, new_decay=false;
	int is,ds;
	char tt[255];
	char tt2[255];
	P_CHAR pc_currchar = currchar[s];
	P_ITEM pj = NULL;

	openscript("gumps.scp");
	sprintf(sect, "GUMPMENU %i", m);
	if (!i_scripts[gump_script]->find(sect))
	{
		closescript();
		return;
	}

	length=21;
	length2=1;

	loopexit=0;
	do
	{
		read1();
		if ((script1[0]!='}')&&(strncmp((char*)script1, "type ", 5)!=0))
		{

			/*sprintf(tt,"pos1: %s\n",script1);
			LogMessage(tt);*/

			length+=strlen((char*)script1)+4;
			length2+=strlen((char*)script1)+4;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	length+=3;

	sprintf(sect, "GUMPTEXT %i", m);
	if (!i_scripts[gump_script]->find(sect))
	{
		closescript();
		return;
	}

	loopexit=0;
	do
	{
		read1();
		if (script1[0]!='}')
		{
			/*sprintf(tt,"pos2: %s\n",script1);
			LogMessage(tt);*/

			if (!strcmp((char*)script1,"Decay Status :")) { new_decay=true; }
	
			length+=(strlen((char*)script1)*2)+2;
			textlines++;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	gump1[1]=length>>8; // total length
	gump1[2]=length%256;
	LongToCharPtr(pc_currchar->serial, &gump1[3]);
	gump1[7]=0;
	gump1[8]=0;
	gump1[9]=0;
	gump1[10]=0x12; // Gump Number
	gump1[19]=length2>>8; // command section length
	gump1[20]=length2%256;

	// remark LB, 
	// 11-14 .. offset x
	// 15-18 .. offset y

	sprintf(sect, "GUMPMENU %i", m);
	if (!i_scripts[gump_script]->find(sect))
	{
		closescript();
		return;
	}
	// typecode setting - Crackerjack 8/8/99
	read1();
	if(!strncmp((char*)script1, "type ", 5)) {
		gump1[10]=atoi((char*)&script1[5]);
		script1[0]='}';
		house_gump=true;
	}

    if (house_gump && new_decay) // showing house decay status, LB 23'th september 2000
	{

        // 1) get house item#  

		
		if (it != NULL) // should be !=-1 for house's gumps (only!), but to be on safe side, lets check it.
		{
       	    pj = it;
		    if(pj != NULL) 
			{
			   is = calcserial(pj->more1, pj->more2, pj->more3, pj->more4);
			   pj = FindItemBySerial( is );			  
			}

            // 2) calc decay % number	

			if (pj!= NULL) 
			{   if (SrvParams->housedecay_secs()!=0)				
				ds=((pj->time_unused)*100)/(SrvParams->housedecay_secs());
			    else ds=-1;
			    numtostr(ds,  tt);
			} else ds=-1;

			//if (j>-1) clConsole.send("decay: %i proz house_age: %i max_age: %i \n",ds,items[j].time_unused,server_data.housedecay_secs);

            // 3) adjust sizes because "decay status :" string is scripted, but the % number added dynamically      
	  
			if (ds>=0 && ds<10)        length+=8;    // space digit space % =4*2
			else if (ds>=10 && ds<100) length+=10;   // space digit digit space % = 5*2
			else if (ds>=100)          length+=12;   // space digit digit digit space % = 6*2

			if (ds==-1) length+=10;
					
	        gump1[1]=length>>8;
	        gump1[2]=length%256;
		}
	}

	Xsend(s, gump1, 21);

	loopexit=0;
	do
	{
		if(script1[0]!='}') {
			sprintf(sect, "{ %s }", script1);

			Xsend(s, sect, strlen(sect));
		}
		read1();
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	// remark: gump2[0]==0, = termination of the sequence above
	gump2[1]=textlines>>8;
	gump2[2]=textlines%256;
	Xsend(s, gump2, 3);

	sprintf(sect, "GUMPTEXT %i", m);
	if (!i_scripts[gump_script]->find(sect))
	{
		closescript();
		return;
	}

	loopexit=0;
	do
	{
		read1();
		if (script1[0]!='}')
		{

			/*sprintf(tt,"pos3: %s\n",script1);
			LogMessage(tt);*/

			if (house_gump && pj != NULL)
			{
				if (!strcmp((char*)script1,"Decay Status :"))
				{				 
				   strcpy(tt2,(char*)script1);
				   strcat(tt," %");
                   strcat((char*)script1, " ");
				   strcat((char*)script1,tt);
				   //clConsole.send("final string: %s\n",script1);
				   //clConsole.send("orig_len : %i new_len: %i\n",strlen(tt2),strlen(script1));
				}
			}

			gump3[0]=strlen((char*)script1)>>8;
			gump3[1]=strlen((char*)script1)%256;
			Xsend(s, gump3, 2);
			gump3[0]=0;
			for (i=0;i<strlen((char*)script1);i++)
			{
				gump3[1]=script1[i];
				Xsend(s, gump3, 2);
			}
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
	closescript();
}



void whomenu(int s, int type) //WhoList--By Homey-- Thx Zip and Taur helping me on this
{
	char sect[512];
	short int length, length2, textlines;
	int i,k,j,x;
	unsigned int line;
	static char menuarray1[10*(MAXCLIENT)+50][50]; // there shoundbe be more than 5*MAXLIENT palyers, 7 to be on save side
	static char menuarray[10*(MAXCLIENT)+50][50];
	unsigned int linecount=0;
	unsigned int linecount1=0,pagenum=1,position=40,linenum=1,buttonnum=7;
	
	j=0;
	for (k=0;k<now;k++)
	{
		if(online(currchar[k])) 
			j++;
	}
	
	//--static pages
	strcpy(menuarray[linecount++], "nomove");
	strcpy(menuarray[linecount++], "noclose");
	strcpy(menuarray[linecount++], "page 0");
	strcpy(menuarray[linecount++], "resizepic 0 0 5120 320 340");    //The background
	strcpy(menuarray[linecount++], "button 20 300 2130 2129 1 0 1"); //OKAY
	strcpy(menuarray[linecount++], "text 20 10 300 0");           //text <Spaces from Left> <Space from top> <Length, Color?> <# in order>		
	sprintf(menuarray[linecount++], "page %i",pagenum);
	//--Start User List
	
	k=0;
	AllCharsIterator iter_char;
	for(iter_char.Begin(); !iter_char.atEnd(); iter_char++)
	{
		P_CHAR toCheck = iter_char.GetData();
		if (toCheck->account() != -1 && !toCheck->free)
		{
			if(k>0 && (!(k%10)))
			{
				position=40;
				pagenum++;
				sprintf(menuarray[linecount++], "page %i",pagenum);
			}
			
			k++;
			
			sprintf(menuarray[linecount++], "text 40 %i 300 %i",position,linenum); //usernames
			sprintf(menuarray[linecount++], "button 20 %i 1209 1210 1 0 %i",position,buttonnum);
			position+=20;
			linenum++;
			buttonnum++;
		}
	}
	
	
	//clConsole.send("k:%i pages: %i\n",k,pagenum);
	
	pagenum=1; //lets make some damn buttons
	for (i=0;i<k;i+=10)
	{			
		sprintf(menuarray[linecount++], "page %i", pagenum);
		if (i>=10)
			sprintf(menuarray[linecount++], "button 150 300 2223 2223  0 %i",pagenum-1); //back button
		if ((k>10) && ((i+10)<k))
			sprintf(menuarray[linecount++], "button %i 300 2224 2224  0 %i", 150+(20*(pagenum>1)),pagenum+1); //forward button
		pagenum++;
	}		
	
	length=21;
	length2=1;
	
	for(line=0;line<linecount;line++)
	{
		
		if (strlen(menuarray[line])==0)
			break;
		{
			length+=strlen(menuarray[line])+4;
			length2+=strlen(menuarray[line])+4;
		}
	}
	
	length+=3;
	textlines=0;
	line=0;
	
	sprintf(menuarray1[linecount1++], "Users currently online: %i",j);
				
	//Start user list
	
	x=0;
	for (i=0;i<now;i++)
	{
		if (online(currchar[i])) 
		{
			sprintf(menuarray1[linecount1++], "Player %s [online]",currchar[i]->name.c_str());
			whomenudata[x++]=currchar[i]->serial;
		}
	}		
	for(iter_char.Begin(); !iter_char.atEnd(); iter_char++)
	{
		P_CHAR toCheck = iter_char.GetData();
		if (toCheck->account() != -1 && !toCheck->free && !online(toCheck)) { 
			sprintf(menuarray1[linecount1++], "Player: %s [offline]",toCheck->name.c_str());
			whomenudata[x++]=toCheck->serial;
			//clConsole.send("name: %s\n",chars[i].name);
		}
	}
	for(line=0;line<linecount1;line++)
	{
		
		if (strlen(menuarray1[line])==0)
			break;
		{
			length+=strlen(menuarray1[line])*2 +2;
			textlines++;
		}
	}
	
	gump1[1]=length>>8;
	gump1[2]=length%256;
	gump1[7]=0;
	gump1[8]=0;
	gump1[9]=0;
	gump1[10]=type; // Gump Number
	gump1[19]=length2>>8;
	gump1[20]=length2%256;
	Xsend(s, gump1, 21);
	
	for(line=0;line<linecount;line++)
	{
		sprintf(sect, "{ %s }", menuarray[line]);
		Xsend(s, sect, strlen(sect));
	}
	
	gump2[1]=textlines>>8;
	gump2[2]=textlines%256;
	
	Xsend(s, gump2, 3);
	
	for(line=0;line<linecount1;line++)
	{
		if (strlen(menuarray1[line])==0)
			break;
		else
		{
			gump3[0]=strlen(menuarray1[line])>>8;
			gump3[1]=strlen(menuarray1[line])%256;
			Xsend(s, gump3, 2);
			gump3[0]=0;
			unsigned int i;
			for (i=0;i<strlen(menuarray1[line]);i++)
			{
				gump3[1]=menuarray1[line][i];
				Xsend(s, gump3, 2);
			}
		}
	}
}

void playermenu(int s, int type) //WhoList2 with offline players--By Ripper
// added also logged out chars+bugfixing , LB
{
	char sect[512];
	short int length, length2, textlines;
	int i,k,j,x;
	unsigned int line;
	static char menuarray1[10*(MAXCLIENT)+50][50]; // there shoundbe be more than 5*MAXLIENT palyers, 7 to be on save side
	static char menuarray[10*(MAXCLIENT)+50][50];
	unsigned int linecount=0;
	unsigned int linecount1=0,pagenum=1,position=40,linenum=1,buttonnum=7;
	
	j=0;
	for (k=0;k<now;k++)
	{
		if(online(currchar[k])) 
			j++;
	}
			   
		//--static pages
		strcpy(menuarray[linecount++], "nomove");
		strcpy(menuarray[linecount++], "noclose");
		strcpy(menuarray[linecount++], "page 0");
		strcpy(menuarray[linecount++], "resizepic 0 0 5120 320 340");    //The background
		strcpy(menuarray[linecount++], "button 20 300 2130 2129 1 0 1"); //OKAY
		strcpy(menuarray[linecount++], "text 20 10 300 0");           //text <Spaces from Left> <Space from top> <Length, Color?> <# in order>		
		sprintf(menuarray[linecount++], "page %i",pagenum);
		//--Start User List

		k=0;
		for(i=0;i<now;i++)
		{
			if ((currchar[i]->account() != -1 && !currchar[i]->free && online(currchar[i])))
			{
			  if(k>0 && (!(k%10)))
			  {
				position=40;
				pagenum++;
				sprintf(menuarray[linecount++], "page %i",pagenum);
			  }

			  k++;

			  sprintf(menuarray[linecount++], "text 40 %i 300 %i",position,linenum); //usernames
			  sprintf(menuarray[linecount++], "button 20 %i 1209 1210 1 0 %i",position,buttonnum);
			  position+=20;
			  linenum++;
			  buttonnum++;
			}
		}
	

		//clConsole.send("k:%i pages: %i\n",k,pagenum);

		pagenum=1; //lets make some damn buttons
		for (i=0;i<k;i+=10)
		{			
			   sprintf(menuarray[linecount++], "page %i", pagenum);
			   if (i>=10)
				sprintf(menuarray[linecount++], "button 150 300 2223 2223  0 %i",pagenum-1); //back button
			   if ((k>10) && ((i+10)<k))
			   sprintf(menuarray[linecount++], "button %i 300 2224 2224  0 %i", 150+(20*(pagenum>1)),pagenum+1); //forward button
			   pagenum++;
		}		
						
		length=21;
		length2=1;
		
		for(line=0;line<linecount;line++)
		{
			
			if (strlen(menuarray[line])==0)
				break;
			{
				length+=strlen(menuarray[line])+4;
				length2+=strlen(menuarray[line])+4;
			}
		}
		
		length+=3;
		textlines=0;
		line=0;
		
		sprintf(menuarray1[linecount1++], "Users currently online: %i",j);
				
		//Start user list

		x=0;
		for (i=0;i<now;i++)
			if (online(currchar[i])) { 
				sprintf(menuarray1[linecount1++], "Player %s [online]",currchar[i]->name.c_str());
				whomenudata[x++]=currchar[i]->serial;
			}	
		
	
		for(line=0;line<linecount1;line++)
		{
			
			if (strlen(menuarray1[line])==0)
				break;
			{
				length+=strlen(menuarray1[line])*2 +2;
				textlines++;
			}
		}
		
		gump1[1]=length>>8;
		gump1[2]=length%256;
		gump1[7]=0;
		gump1[8]=0;
		gump1[9]=0;
		gump1[10]=type; // Gump Number
		gump1[19]=length2>>8;
		gump1[20]=length2%256;
		Xsend(s, gump1, 21);
		
		for(line=0;line<linecount;line++)
		{
			sprintf(sect, "{ %s }", menuarray[line]);
		    Xsend(s, sect, strlen(sect));
		}
		
		gump2[1]=textlines>>8;
		gump2[2]=textlines%256;
		
		Xsend(s, gump2, 3);
		
		for(line=0;line<linecount1;line++)
		{
			if (strlen(menuarray1[line])==0)
				break;
			{
				gump3[0]=strlen(menuarray1[line])>>8;
				gump3[1]=strlen(menuarray1[line])%256;
				Xsend(s, gump3, 2);
				gump3[0]=0;
				for (i=0;i<strlen(menuarray1[line]);i++)
				{
					gump3[1]=menuarray1[line][i];
					Xsend(s, gump3, 2);
				}
			}
		}
}


void tline( int line, SERIAL serial )
{
	char type;
	if (isCharSerial(serial))
		type = 2;
	else
		type = 1;
	line--; if( line == 0 ) strcpy( (char*)script1, "page 0" );
	line--; if( line == 0 ) strcpy( (char*)script1, "resizepic 0 0 2520 400 350" );
	line--; if( line == 0 ) strcpy( (char*)script1, "text 40 10 32 0" );
	line--; if( line == 0 ) strcpy( (char*)script1, "button 35 280 2130 2129 1 0 1" );
	if( type == 1 ) 
	{ 
		line--; 
		P_ITEM pi = FindItemBySerial(serial);
		if( line == 0 ) 
			sprintf( (char*)script1, "tilepic 300 180 %i", pi->id()); 
	}
	line--; if( line == 0 ) strcpy( (char*)script1, "page 1" );
	line--; if( line == 0 ) strcpy( (char*)script1, "button 366 320 2224 2224 0 2");
	line--; if( line == 0 ) strcpy( (char*)script1, "text 288 317 32 1");
	line--; if( line == 0 ) strcpy( (char*)script1, "text 90 70 0 3");
	line--; if( line == 0 ) strcpy( (char*)script1, "text 200 70 16 4");
	line--; if( line == 0 ) strcpy( (char*)script1, "button 30 70 2116 2115 1 0 2");
	line--; if( line == 0 ) strcpy( (char*)script1, "text 90 95 0 5");
	line--; if( line == 0 ) strcpy( (char*)script1, "text 200 95 16 6");
	line--; if( line == 0 ) strcpy( (char*)script1, "button 30 95 2116 2115 1 0 3");
	line--; if( line == 0 ) strcpy( (char*)script1, "text 90 120 0 7");
	line--; if( line == 0 ) strcpy( (char*)script1, "text 200 120 16 8");
	line--; if( line == 0 ) strcpy( (char*)script1, "button 30 120 2116 2115 1 0 4");
	line--; if( line == 0 ) strcpy( (char*)script1, "text 90 145 0 9");
	line--; if( line == 0 ) strcpy( (char*)script1, "text 200 145 16 10");
	line--; if( line == 0 ) strcpy( (char*)script1, "button 30 145 2116 2115 1 0 5");
	line--; if( line == 0 ) strcpy( (char*)script1, "text 90 170 0 11");
	line--; if( line == 0 ) strcpy( (char*)script1, "text 200 170 16 12");
	line--; if( line == 0 ) strcpy( (char*)script1, "button 30 170 2116 2115 1 0 6");
	line--; if( line == 0 ) strcpy( (char*)script1, "text 90 195 0 13");
	line--; if( line == 0 ) strcpy( (char*)script1, "text 200 195 16 14");
	line--; if( line == 0 ) strcpy( (char*)script1, "button 30 195 2116 2115 1 0 7");
	line--; if( line == 0 ) strcpy( (char*)script1, "text 90 220 0 15");
	line--; if( line == 0 ) strcpy( (char*)script1, "text 200 220 16 16");
	line--; if( line == 0 ) strcpy( (char*)script1, "button 30 220 2116 2115 1 0 8");
	if( type == 2 )
	{
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 245 0 17");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 245 16 18");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 245 2116 2115 1 0 9");
	}
	if( type == 1 )
	{
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 245 0 19");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 245 16 20");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 245 2116 2115 1 0 10");
	}
	line--; if( line == 0 ) strcpy( (char*)script1, "page 2");
	line--; if( line == 0 ) strcpy( (char*)script1, "button 40 320 2223 2223 0 1");
	line--; if( line == 0 ) strcpy( (char*)script1, "text 65 317 32 2");
	line--; if( line == 0 ) strcpy( (char*)script1, "button 366 320 2224 2224 0 3");
	line--; if( line == 0 ) strcpy( (char*)script1, "text 288 317 32 1");
	if( type == 1 )
	{
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 70 0 17");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 70 16 18");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 70 2116 2115 1 0 9");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 95 0 21");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 95 16 22");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 95 2116 2115 1 0 11");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 120 0 23");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 120 16 24");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 120 2116 2115 1 0 12");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 145 0 25");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 145 16 26");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 145 2116 2115 1 0 13");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 170 0 27");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 170 16 28");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 170 2116 2115 1 0 14");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 195 0 29");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 195 16 30");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 195 2116 2115 1 0 15");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 220 0 31");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 220 16 32");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 220 2116 2115 1 0 16");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 245 0 33");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 245 16 34");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 245 2116 2115 1 0 17");
	}
	if( type == 2 )
	{
		line--; if( line==0 ) strcpy( (char*)script1, "text 90 70 0 19");
		line--; if( line==0 ) strcpy( (char*)script1, "text 200 70 16 20");
		line--; if( line==0 ) strcpy( (char*)script1, "button 30 70 2116 2115 1 0 10");
		line--; if( line==0 ) strcpy( (char*)script1, "text 90 95 0 21");
		line--; if( line==0 ) strcpy( (char*)script1, "text 200 95 16 22");
		line--; if( line==0 ) strcpy( (char*)script1, "button 30 95 2116 2115 1 0 11");
		line--; if( line==0 ) strcpy( (char*)script1, "text 90 120 0 23");
		line--; if( line==0 ) strcpy( (char*)script1, "text 200 120 16 24");
		line--; if( line==0 ) strcpy( (char*)script1, "button 30 120 2116 2115 1 0 12");
		line--; if( line==0 ) strcpy( (char*)script1, "text 90 145 0 25");
		line--; if( line==0 ) strcpy( (char*)script1, "text 200 145 16 26");
		line--; if( line==0 ) strcpy( (char*)script1, "button 30 145 2116 2115 1 0 13");
		line--; if( line==0 ) strcpy( (char*)script1, "text 90 170 0 27");
		line--; if( line==0 ) strcpy( (char*)script1, "text 200 170 16 28");
		line--; if( line==0 ) strcpy( (char*)script1, "button 30 170 2116 2115 1 0 14");
		//Karma
		line--; if( line==0 ) strcpy( (char*)script1, "text 90 195 0 29");
		line--; if( line==0 ) strcpy( (char*)script1, "text 200 195 16 30");
		line--; if( line==0 ) strcpy( (char*)script1, "button 30 195 2116 2115 1 0 15");
		//Fame
		line--; if( line==0 ) strcpy( (char*)script1, "text 90 220 0 31");
		line--; if( line==0 ) strcpy( (char*)script1, "text 200 220 16 32");
		line--; if( line==0 ) strcpy( (char*)script1, "button 30 220 2116 2115 1 0 16");
		//Kills
		line--; if( line==0 ) strcpy( (char*)script1, "text 90 245 0 33");
		line--; if( line==0 ) strcpy( (char*)script1, "text 200 245 16 34");
		line--; if( line==0 ) strcpy( (char*)script1, "button 30 245 2116 2115 1 0 17");

	}
	line--; if( line == 0 ) strcpy( (char*)script1, "page 3");
	line--; if( line == 0 ) strcpy( (char*)script1, "button 40 320 2223 2223 0 2");
	line--; if( line == 0 ) strcpy( (char*)script1, "text 65 317 32 2");
	line--;	if( line == 0 ) strcpy( (char*)script1, "button 366 320 2224 2224 0 4" );
	line--; if( line == 0 ) strcpy( (char*)script1, "text 288 317 32 1" );
	if( type == 1 )
	{
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 70 0 35");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 70 16 36");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 70 2116 2115 1 0 18");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 95 0 37");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 95 16 38");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 95 2116 2115 1 0 19");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 120 0 39");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 120 16 40");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 120 2116 2115 1 0 20");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 145 0 41");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 145 16 42");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 145 2116 2115 1 0 21");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 170 0 43");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 170 16 44");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 170 2116 2115 1 0 22");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 195 0 45");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 195 16 46");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 195 2116 2115 1 0 23");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 220 0 47");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 220 16 48");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 220 2116 2115 1 0 24");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 245 0 49");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 245 16 50");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 245 2116 2115 1 0 25");
	}
	if( type == 2 )
	{
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 70 0 35");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 70 16 36");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 70 2116 2115 1 0 18");

	}
	line--; if( line == 0 ) strcpy( (char*)script1, "page 4");
	line--; if( line == 0 ) strcpy( (char*)script1, "button 40 320 2223 2223 0 3");
	line--; if( line == 0 ) strcpy( (char*)script1, "text 65 317 32 2");
	if (type==1)
	{
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 70 0 51");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 70 16 52");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 70 2116 2115 1 0 26");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 95 0 53");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 95 16 54");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 95 2116 2115 1 0 27");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 120 0 55");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 120 16 56");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 120 2116 2115 1 0 28");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 90 145 0 57");
		line--; if( line == 0 ) strcpy( (char*)script1, "text 200 145 16 58");
		line--; if( line == 0 ) strcpy( (char*)script1, "button 30 145 2116 2115 1 0 29");

		//end addons by Xuri
	}
	line--; if (line==0) strcpy( (char*)script1,  "}");
}

void ttext(int line, SERIAL serial)
{
	char type;
	if (isCharSerial(serial))
		type = 2;
	else
		type = 1;

	if (type==1) { line--; if( line == 0 ) strcpy( (char*)script1, "Item Properties"); }
	if (type==2) { line--; if( line == 0 ) strcpy( (char*)script1, "Character Properties"); }
	line--; if( line == 0 ) strcpy( (char*)script1, "Next page");
	line--; if( line == 0 ) strcpy( (char*)script1, "Previous page");
	if (type==1)
	{
		P_ITEM pj = FindItemBySerial(serial);
		if (pj == NULL)
			return;
		line--; if( line == 0 ) strcpy( (char*)script1, "Name");
		line--; if( line == 0 ) strcpy( (char*)script1, pj->name().ascii() );
		line--; if( line == 0 ) strcpy( (char*)script1, "ID");
		line--; if( line == 0 ) sprintf( (char*)script1,"0x%x (%i)", pj->id(), pj->id());
		line--; if( line == 0 ) strcpy( (char*)script1, "Hue");
		line--; if( line == 0 ) sprintf( (char*)script1,"0x%x (%i)", pj->color, pj->color);
		line--; if( line == 0 ) strcpy( (char*)script1, "X");
		line--; if( line == 0 ) sprintf( (char*)script1,"%i (0x%x)", pj->pos.x, pj->pos.x);
		line--; if( line == 0 ) strcpy( (char*)script1, "Y");
		line--; if( line == 0 ) sprintf( (char*)script1,"%i (0x%x)", pj->pos.y, pj->pos.y);
		line--; if( line == 0 ) strcpy( (char*)script1, "Z");
		line--; if( line == 0 ) sprintf( (char*)script1,"%i (0x%x)", pj->pos.z, pj->pos.z);
		line--; if( line == 0 ) strcpy( (char*)script1, "Type");
		line--; if( line == 0 ) sprintf( (char*)script1,"%i", pj->type);
		line--; if( line == 0 ) strcpy( (char*)script1,  "ItemHand" );
		line--; if( line == 0 ) sprintf( (char*)script1, "%i", pj->itmhand );
		line--; if( line == 0 ) strcpy( (char*)script1, "Layer");
		line--; if( line == 0 ) sprintf( (char*)script1,"%i (0x%x)", pj->layer, pj->layer);
		line--; if( line == 0 ) strcpy( (char*)script1, "Amount");
		line--; if( line == 0 ) sprintf( (char*)script1,"%i", pj->amount);
		line--; if( line == 0 ) strcpy( (char*)script1, "More");
		line--; if( line == 0 ) sprintf( (char*)script1,"0x%x", (pj->more1<<24)+(pj->more2<<16)+(pj->more3<<8)+pj->more4);
		line--; if( line == 0 ) strcpy( (char*)script1, "MoreB");
		line--; if( line == 0 ) sprintf( (char*)script1,"0x%x", (pj->moreb1<<24)+(pj->moreb2<<16)+(pj->moreb3<<8)+pj->moreb4);
		line--; if( line == 0 ) strcpy( (char*)script1, "Stackable");
		line--; if( line == 0 ) sprintf( (char*)script1,"%i", pj->pileable);
		line--; if( line == 0 ) strcpy( (char*)script1, "Dyeable");
		line--; if( line == 0 ) sprintf( (char*)script1,"%i", pj->dye );
		line--; if( line == 0 ) strcpy( (char*)script1, "Corpse");
		line--; if( line == 0 ) sprintf( (char*)script1,"%i", pj->corpse );
		line--; if( line == 0 ) strcpy((char*) script1, "LoDamage" );
		line--; if( line == 0 ) sprintf( (char*)script1,"%i", pj->lodamage );
		line--; if( line == 0 ) strcpy( (char*)script1, "HiDamage" );
		line--; if( line == 0 ) sprintf( (char*)script1,"%i", pj->hidamage );
		line--; if( line == 0 ) strcpy( (char*)script1, "Defense");
		line--; if( line == 0 ) sprintf( (char*)script1,"%i", pj->def);
		line--; if( line == 0 ) strcpy( (char*)script1, "Magic");
		line--; if( line == 0 ) sprintf( (char*)script1,"%i", pj->magic);
		line--; if( line == 0 ) strcpy( (char*)script1, "Visible");
		line--; if( line == 0 ) sprintf( (char*)script1,"%i", pj->visible);
		line--; if( line == 0 ) strcpy( (char*)script1, "HitPoints" );
		line--; if( line == 0 ) sprintf( (char*)script1, "%i", pj->hp );
		line--; if( line == 0 ) strcpy( (char*)script1, "MaxHitPoints" );
		line--; if( line == 0 ) sprintf((char*) script1, "%i", pj->maxhp );
		line--; if( line == 0 ) strcpy((char*) script1, "Speed" );
		line--; if( line == 0 ) sprintf((char*) script1, "%i", pj->spd );
		line--; if( line == 0 ) strcpy( (char*)script1, "Rank");
		line--; if( line == 0 ) sprintf((char*) script1,"%i", pj->rank);
		line--; if( line == 0 ) strcpy((char*) script1, "Value");
		line--; if( line == 0 ) sprintf((char*) script1,"%i", pj->value);
		line--; if( line == 0 ) strcpy((char*) script1, "Good" );
		line--; if( line == 0 ) sprintf( (char*)script1,"%i", pj->good );
		line--; if( line == 0 ) strcpy( (char*)script1, "Made Skill" );
		line--; if( line == 0 ) sprintf( (char*)script1, "%i", pj->madewith );
		line--; if( line == 0 ) strcpy((char*) script1, "Creator" );
		line--; if( line == 0 ) strcpy((char*)script1, pj->creator.c_str() );

	}
	if( type == 2 )
	{
		P_CHAR pc_j = FindCharBySerial(serial);
		if (pc_j == NULL)
			return;
		line--; if( line == 0 ) strcpy( (char*)script1, "Name" );
		line--; if( line == 0 ) strcpy( (char*)script1,  pc_j->name.c_str() );
		line--; if( line == 0 ) strcpy( (char*)script1, "Title" );
		line--; if( line == 0 ) strcpy((char*) script1,  pc_j->title().latin1() );
		line--; if( line == 0 ) strcpy( (char*)script1, "X" );
		line--; if( line == 0 ) sprintf((char*) script1,"%i", pc_j->pos.x );
		line--; if( line == 0 ) strcpy((char*) script1, "Y" );
		line--; if( line == 0 ) sprintf( (char*)script1,"%i", pc_j->pos.y );
		line--; if( line == 0 ) strcpy( (char*)script1, "Z" );
		line--; if( line == 0 ) sprintf( (char*)script1,"%i", pc_j->pos.z );
		line--; if( line == 0 ) strcpy( (char*)script1, "Direction" );
		line--; if( line == 0 ) sprintf( (char*)script1,"%i", pc_j->dir );
		line--; if( line == 0 ) strcpy( (char*)script1, "Body" );
		line--; if( line == 0 ) sprintf((char*) script1,"(0x%x) %i", pc_j->id(), pc_j->id() );
		line--; if( line == 0 ) strcpy((char*) script1, "Skin" );
		line--; if( line == 0 ) sprintf( (char*)script1, "(0x%x) %i", pc_j->skin, pc_j->skin );
		line--; if( line == 0 ) strcpy( (char*)script1, "Defence" );
		line--; if( line == 0 ) sprintf((char*) script1,"%i", pc_j->def );
		line--; if( line == 0 ) strcpy( (char*)script1, "Hunger" );
		line--; if( line == 0 ) sprintf( (char*)script1,"%i", pc_j->hunger );
		line--; if( line == 0 ) strcpy( (char*)script1, "Strength" );
		line--; if( line == 0 ) sprintf((char*) script1,"%i", pc_j->st );
		line--; if( line == 0 ) strcpy( (char*)script1, "Dexterity" );
		line--; if( line == 0 ) sprintf( (char*)script1,"%i", pc_j->realDex() );
		line--; if( line == 0 ) strcpy( (char*)script1, "Intelligence" );
		line--; if( line == 0 ) sprintf( (char*)script1,"%i", pc_j->in );
		line--; if( line == 0 ) strcpy( (char*)script1, "Karma" );
		line--; if( line == 0 ) sprintf( (char*)script1,"%i", pc_j->karma );
		line--; if( line == 0 ) strcpy( (char*)script1, "Fame" );
		line--; if( line == 0 ) sprintf( (char*)script1,"%i", pc_j->fame );
		line--; if( line == 0 ) strcpy( (char*)script1, "Kills" );
		line--; if( line == 0 ) sprintf( (char*)script1,"%i", pc_j->kills );
		line--; if( line == 0 ) strcpy( (char*)script1, "Jail time" );
		line--; if( line == 0 ) sprintf( (char*)script1,"%i", pc_j->jailsecs );
		
	}
	line--; if( line == 0) strcpy( (char*)script1, "}");
}


void tweakmenu(UOXSOCKET s, SERIAL serial)
{
	char sect[512];
	short int length, length2, textlines;
	int line,loopexit=0;

	char type;
	if (isCharSerial(serial))
		type = 2;
	else
		type = 1;


	length=21;
	length2=1;
	line=0;

	do
	{
		line++;
		tline(line, serial);
		if (script1[0]!='}')
		{
			length+=strlen((char*)script1)+4;
			length2+=strlen((char*)script1)+4;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	length+=3;
	textlines=0;
	line=0;


	loopexit=0;
	do
	{
		line++;
		ttext(line, serial);
		if (script1[0]!='}')
		{
			length+=(strlen((char*)script1)*2)+2;
			textlines++;
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );

	gump1[1]=length>>8;
	gump1[2]=length%256;
	LongToCharPtr(serial, &gump1[3]);
	gump1[7]=0;
	gump1[8]=0;
	gump1[9]=0;
	gump1[10]=type; // Gump Number
	gump1[19]=length2>>8;
	gump1[20]=length2%256;
	Xsend(s, gump1, 21);
	line=0;

	loopexit=0;
	do
	{
		line++;
		tline(line, serial);
		if (script1[0]!='}')
		{
			sprintf(sect, "{ %s }", script1);
			Xsend(s, sect, strlen(sect));
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );

	gump2[1]=textlines>>8;
	gump2[2]=textlines%256;
	Xsend(s, gump2, 3);
	line=0;

	loopexit=0;
	do
	{
		line++;
		ttext(line, serial);
		if (script1[0]!='}')
		{
			gump3[0]=strlen((char*)script1)>>8;
			gump3[1]=strlen((char*)script1)%256;
			Xsend(s, gump3, 2);
			gump3[0]=0;
			unsigned int i;
			for (i=0;i<strlen((char*)script1);i++)
			{
				gump3[1]=script1[i];
				Xsend(s, gump3, 2);
			}
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
}

void entrygump(int s, SERIAL serial, unsigned char type, char index, short int maxlength, char *text1)
{
	short int length;
	char textentry1[12]="\xAB\x01\x02\x01\x02\x03\x04\x00\x01\x12\x34";
	char textentry2[9]="\x01\x01\x00\x00\x12\x34\x12\x34";
	
	sprintf((char*)temp, "(%i chars max)", maxlength);
	length=11+strlen((char*)text1)+1+8+strlen((char*)temp)+1;
	textentry1[1]=length>>8;
	textentry1[2]=length%256;
	LongToCharPtr(serial, (unsigned char*)textentry1+3);
	textentry1[7]=type;
	textentry1[8]=index;
	textentry1[9]=(strlen(text1)+1)>>8;
	textentry1[10]=(strlen(text1)+1)%256;
	Xsend(s, textentry1, 11);
	Xsend(s, text1, strlen(text1)+1);
	textentry2[4]=maxlength>>8;
	textentry2[5]=maxlength%256;
	textentry2[6]=(strlen((char*)temp)+1)>>8;
	textentry2[7]=(strlen((char*)temp)+1)%256;
	Xsend(s, textentry2, 8);
	Xsend(s, temp, strlen((char*)temp)+1);
}

// Parses an Action node
bool DisposeAction( UOXSOCKET Socket, QDomElement Action )
{
	if( Action.nodeName() == "useitem" )
		return useItem( Socket, Action );

	else if( Action.nodeName() == "hasitem" )
		return hasItem( Socket, Action );

	else if( Action.nodeName() == "makeitem" )
		return makeItem( Socket, Action );

	else if( Action.nodeName() == "checkskill" )
		return checkSkill( Socket, Action );

	return true;
}

// REWRITTEN BY DARKSTORM
// \x7D, this packet is the reply
// to the craft-menus
void MenuChoice( UOXSOCKET Socket ) 
{ 
	P_CHAR pc_currchar = currchar[ Socket ];

	UI16 Menu = ( buffer[ Socket ][5] << 8 ) + buffer[ Socket ][6];
	UI16 Choice = ( buffer[ Socket ][7] << 8 ) + buffer[ Socket ][8];

	// Guildstone Menus
	// Maybe we should change that to 0xFE so we can maintain a given style
	if ( (UI08)( Menu >> 8 ) == 0xFE )
	{
		// Get the Guildstone item
		cGuildStone* pStone = dynamic_cast<cGuildStone*>( FindItemBySerial( pc_currchar->guildstone ) );
		
		if ( pStone != NULL )
			pStone->GumpChoice( Socket, Menu, Choice );
		// ok, ok, not too much sphere ;)
		// else
		//	sysmessage( s, "Unexpected target info" );

		// If it's a Guildstone Menu i dont think we need to process anyhing else
		return;
	}

	// we have nothing to do
	if( Choice == 0 )
		return;

	// Retrieve our menu
	Choice--;
	QString MenuID;
	MenuID.sprintf( "%i", Menu );

	QDomElement *MenuNode = DefManager->getSection( WPDT_MENU, MenuID );

	if( MenuNode->isNull() )
	{
		sysmessage( Socket, "No such menu: %i", Menu );
		return;
	}

	// Get the Entry which was selected
	if( Choice+1 > MenuNode->childNodes().count() ) 
	{
		sysmessage( Socket, "Invalid Choice (%i) selected for Menu %i", Choice, Menu );
		return;
	}
	
	QDomElement Entry = MenuNode->childNodes().item( Choice ).toElement();

	if( Entry.isNull() )
	{
		sysmessage( Socket, "Invalid Choice (%i) selected for Menu %i", Choice, Menu );
		return;
	}

	// Simple processing: Enter a new Submenu
	if( Entry.nodeName() == "submenu" )
	{
		if( !Entry.attributes().contains( "id" ) )
		{
			sysmessage( Socket, "Invalid Submenu found in Menu %i", Menu );
			return;
		}

		QString SubMenu = Entry.attributeNode( "id" ).nodeValue();
		ShowMenu( Socket, SubMenu.toUShort() );
		return;
	}

	// We now have only one choice left (actions)
	if( Entry.nodeName() != "actions" )
	{
		sysmessage( Socket, "Invalid Entry (%s) selected in Menu %i", Entry.nodeName().latin1(), Menu );
		return;
	}

	// Walk all required actions
	// if one of them fails then quit the
	// action-chain
	for( UI32 i = 0; i < Entry.childNodes().count(); i++ )
	{
		if( !Entry.childNodes().item( i ).isElement() )
			continue;

		QDomElement Action = Entry.childNodes().item( i ).toElement();

		if( Action.isNull() )
			continue;

		if( !DisposeAction( Socket, Action ) )
			return;
	}

	// DO WE NEED THAT ANYMORE !?!
	// A maximum of 256 Item-menus ->
	// 0xFFxx -> Ss
	/*if( (UI08)( Menu >> 8 ) == 0xFF )
	{
		if ( im_choice( s, Menu, Choice ) == 0 ) 
			return;
	}*/

	// Polymorph:
	// Magic->Polymorph(s,POLYMORPHMENUOFFSET,sub); 
	
	// Potions:
	// Skills->DoPotion(s, main-1246, sub, FindItemBySerial(calcserial(addid1[s], addid2[s], addid3[s], addid4[s])));

	// Tracking:
	//	if(!Skills->CheckSkill(currchar[s], TRACKING, 0, 1000))
	// {
	//	sysmessage(s,"You fail your attempt at tracking.");
	//	return;
	// }
	// Skills->TrackingMenu(s,sub-1);
}

void ShowMenu( UOXSOCKET Socket, UI16 Menu ) // Menus for item creation
{
	P_CHAR pc_currchar = currchar[ Socket ];

	QString MenuID;
	MenuID.sprintf( "%d", Menu );

	// Get our node
	QDomNode *NodePtr = DefManager->getSection( WPDT_MENU, MenuID );
	QDomElement Node = NodePtr->toElement();
	
	if( Node.isNull() )
	{
		sysmessage( Socket, "There is no such menu: %i\n", Menu );
		return;
	}

	// If the menu Requires special clearance make sure
	// it requirements are met
	if( Node.attributes().contains( "menupriv" ) )
	{
		bool Passed = false;

		// If we're on account 0 you can always access the menu
		if( pc_currchar->menupriv() == -1 ) 
			Passed = true;
		else
		{
			QStringList MenuPrivs = QStringList::split( ",", Node.attributeNode( "menupriv" ).nodeValue() );

			// No Priv requirements -> pass!
			if( MenuPrivs.empty() )
				Passed = true;

			// Check if our character meets one of the requirements
			for( UI08 i = 0; i < MenuPrivs.count(); i++ )
				if( pc_currchar->menupriv() == MenuPrivs[ i ].toInt() )
					Passed = true;
		}

		if( !Passed )
		{
			sysmessage( Socket, "You do not meet the requirements to display this menu" );
			return;
		}
	}

	// Get the Menu Name
	if( !Node.attributes().contains( "name" ) )
	{
		sysmessage( Socket, "An error occured while parsing menu: %i\n", Menu );
		return;
	}

	QString MenuName = Node.attributes().namedItem( "name" ).toAttr().nodeValue();
	QByteArray ByteArray;

	// we've got 
	// a 9 byte header, 1 byte stringlength and xx string
	ByteArray.resize( 9 + 1 + MenuName.length() + 1 );
	ByteArray.fill( 0 );

	// Basic Packet information
	// Packet: x7C
	// BYTE cmd = x7C
	// BYTE[2] blockSize x0000
	// BYTE[4] dialogID x01020304
	// BYTE[2] menuid x0064
	ByteArray[ 0 ] = 0x7C; // Packet ID

	// Skip 2 bytes for the packet length
	ByteArray[ 3 ] = (UI08)( pc_currchar->serial >> 24 );
	ByteArray[ 4 ] = (UI08)( pc_currchar->serial >> 16 );
	ByteArray[ 5 ] = (UI08)( pc_currchar->serial >> 8 );
	ByteArray[ 6 ] = (UI08)( pc_currchar->serial );

	// Menu-ID
	UI16 RealID = MenuID.toULong();

	ByteArray[ 7 ] = (UI08)( RealID >> 8 );
	ByteArray[ 8 ] = (UI08)( RealID );

	// BYTE length of question
	// BYTE[length of question] question text
	ByteArray[ 9 ] = (UI08)( MenuName.length() );

        UI08 i;

	for( i = 0; i < MenuName.length(); i++ )
	{
		ByteArray[ 10 + i ] = MenuName.at( i ).latin1();
	}

	// Current Position = Length() - 1
	int CurrentIndex = ByteArray.count();
	UI08 Responses = 0;

	// BYTE[2] model id # of shown item (if grey menu -- then always 0x00 as msb)
	// BYTE[2] unknown2 (00 00 check or not?)
	// BYTE response text length
	// BYTE[response text length] response text
	for( i = 0; i < Node.childNodes().count(); i++ )
	{
		QDomNode SubNode = Node.childNodes().item( i );

		if( !SubNode.isElement() )
			continue;

		if( ( SubNode.toElement().tagName() != "submenu" ) && ( SubNode.toElement().tagName() != "actions" ) )
			continue;

		// Display the item __ONLY__ if the user meets the requirements
		// I.e. Skill or Resources
		//
		// ^^^^ This is DISABLED as it uses up too much CPU time
		//      to check for the requirements each time
		UI16 ModelID = 0;
		QString ResponseText;

		// Small Icon in the horizontal scrolling menu
		if( SubNode.toElement().attributes().contains( "model" ) )
			ModelID = SubNode.toElement().attributeNode( "model" ).nodeValue().toUShort( NULL, 16 );
		
		// Response Text
		if( SubNode.toElement().attributes().contains( "name" ) )
			ResponseText = SubNode.toElement().attributeNode( "name" ).nodeValue();

		// Resize the ByteArray to meet the new conditions
		// Current Length + 2 model-id + 2 unknown + 1 textlength + textlength
		ByteArray.resize( ByteArray.size() + 4 + 1 + ResponseText.length() );
		
		// Model ID
		ByteArray[ CurrentIndex++ ] = (UI08)( ModelID >> 8 );
		ByteArray[ CurrentIndex++ ] = (UI08)( ModelID );

		// Model Hue
		QString Hue = "0";

		if( SubNode.toElement().attributes().contains( "hue" ) )
			Hue = SubNode.toElement().attributeNode( "hue" ).nodeValue();

		UI16 RealHue = Hue.toUShort( NULL, 16 );

		ByteArray[ CurrentIndex++ ] = (UI08)( RealHue >> 8 );
		ByteArray[ CurrentIndex++ ] = (UI08)( RealHue );

		// "Copy" in the Response Text
		ByteArray[ CurrentIndex++ ] = (UI08)( ResponseText.length() );

		for( UI08 j = 0; j < ResponseText.length(); j++ )
		{
			ByteArray[ CurrentIndex++ ] = ResponseText.at( j ).latin1();
		}

		Responses++;
	}

	// Number of Responses
	ByteArray[ (int)( 9 + 1 + MenuName.length() ) ] = Responses;

	// Set the packet length
	UI16 PacketLength = ByteArray.count();
	ByteArray[ 1 ] = (UI08)( PacketLength >> 8 );
	ByteArray[ 2 ] = (UI08)( PacketLength % 256 );

	Xsend( Socket, ByteArray.data(), ByteArray.count() );
}

void cGump::Open(int s, P_CHAR pc, int num1, int num2)
{
	unsigned char shopgumpopen[8]="\x24\x00\x00\x00\x01\x00\x30";
	LongToCharPtr(pc->serial, &shopgumpopen[1]);
	shopgumpopen[5]=num1;
	shopgumpopen[6]=num2;
	Xsend(s, shopgumpopen, 7);
}
