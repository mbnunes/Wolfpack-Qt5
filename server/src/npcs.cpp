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

// Platform Includes
#include "platform.h"

// Wolfpack Includes
#include "wolfpack.h"
#include "itemid.h"
#include "sregions.h"
#include "SndPkg.h"
#include "debug.h"
#include "utilsys.h"
#include "walking2.h"
#include "regions.h"
#include "wpdefmanager.h"
#include "wpdefaultscript.h"

#include <string>

#undef  DBGFILE
#define DBGFILE "npcs.cpp"

#define CHAR_RESERVE 100	// minimum of free slots that should be left in the array.
							// otherwise, more memory will be allocated in the mainloop (Duke)

unsigned short addrandomcolor(P_CHAR pc_s, char *colorlist)
{
	char sect[512];
	int i = 0,j = 0,storeval = 0;
	openscript("colors.scp");
	sprintf(sect, "RANDOMCOLOR %s", colorlist);
	if (!i_scripts[colors_script]->find(sect))
	{
		closescript();

		//sprintf(pc_s->name, "Error Colorlist %s Not Found(1)", colorlist);
		// LB: wtf should this do apart from crashing ? copying an error message in a chars name ??
		// very weired! think it should look like this:
		if( pc_s != NULL )
			clConsole.send("Error Colorlist %s not found on character: %s\n",colorlist,pc_s->name.c_str());

		return 0;
	}
	unsigned long loopexit=0;
	do
	{
		read1();
		if (script1[0]!='}')
		{
			i++;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	closescript();
	if(i>0)
	{
		i=rand()%i;
		i++;
		openscript("colors.scp");
		if(!i_scripts[colors_script]->find(sect))
		{
			closescript();
			//sprintf(pc_s->name, "Error Colorlist %s Not Found(2)", colorlist);
			if( pc_s != NULL )
				clConsole.send("Error Colorlist %s not found on character: %s\n",colorlist,pc_s->name.c_str());
			return 0;
		}
		loopexit=0;
		do
		{
			read1();
			if (script1[0]!='}')
			{
				j++;
				if(j==i)
				{
					storeval=hex2num(script1);
				}
			}
		}
		while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
		closescript();
	}
	return (storeval);
}

static int addrandomhaircolor(P_CHAR pc_s, char *colorlist)
{
	char sect[512];
	int i,j,haircolor = 0x044e;
	i=0; j=0;
	openscript("colors.scp");
	sprintf(sect, "RANDOMCOLOR %s", colorlist);
	if (!i_scripts[colors_script]->find(sect))
	{
		closescript();
		clConsole.send("Error Colorlist %s not found on character: %s\n",colorlist,pc_s->name.c_str());
		return 0;
	}
	unsigned long loopexit=0;
	do
	{
		read1();
		if (script1[0]!='}')
		{
			i++;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
	closescript();
	if(i>0)
	{
		i=rand()%i;
		i++;
		openscript("colors.scp");
		if(!i_scripts[colors_script]->find(sect))
		{
			closescript();
			//sprintf(pc_s->name, "Error Colorlist %s Not Found(2)", colorlist);
			clConsole.send("Error Colorlist %s not found on character: %s\n",colorlist,pc_s->name.c_str());
			return 0;
		}
		loopexit=0;
		do
		{
			read1();
			if (script1[0]!='}')
			{
				j++;
				if(j==i)
				{
					haircolor=hex2num(script1);
				}
			}
		}
		while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
		closescript();
	}
	return (haircolor);
}

void setrandomname(P_CHAR pc_s, char * namelist)
{
	char sect[512];
	int i=0,j=0;

	sprintf(sect, "RANDOMNAME %s", namelist);
	Script *pScpBase=i_scripts[npc_script];
	Script *pScp=pScpBase->Select(sect,custom_npc_script);
	if (!pScp)
	{
		pc_s->name = "Error Namelist";
		return;
	}

	unsigned long loopexit=0;
	do
	{
		pScp->NextLine();
		if (script1[0]!='}')
		{
			i++;
		}
	}
	while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
	pScp->Close();

	if(i>0)
	{
		i=rand()%(i);
		pScp=pScpBase->Select(sect,custom_npc_script);
		if (!pScp) return;

		loopexit=0;
		do
		{
			pScp->NextLine();
			if (script1[0]!='}')
			{
				if(j==i)
				{
					pc_s->name = (char*)script1;
					break;
				}
				else j++;
			}
		}
		while ((script1[0]!='}') && (++loopexit < MAXLOOPS) );
		pScp->Close();
	}
}

void cCharStuff::DeleteChar (P_CHAR pc_k) // Delete character
{
	int j;//,serial; //Zippy lag
	//int ptr,ci;

	LongToCharPtr(pc_k->serial, &removeitem[1]);

	if ( pc_k->spawnregion() > 0 )
	{
		spawnregion[pc_k->spawnregion()].current--;
	}


	if (pc_k->spawnSerial() != INVALID_SERIAL) 
		cspawnsp.remove(pc_k->spawnSerial(), pc_k->serial);
	if (pc_k->ownserial != INVALID_SERIAL) 
		cownsp.remove(pc_k->ownserial, pc_k->serial);
	
	for (j=0;j<now;j++)
	{
		if (perm[j]) 
			Xsend(j, removeitem, 5);		
	}
	
	if (pc_k != NULL) 
		mapRegions->Remove(pc_k); // taking it out of mapregions BEFORE x,y changed, LB
	
	pc_k->free = true;
	cCharsManager::getInstance()->deleteChar( pc_k );
}

P_CHAR cCharStuff::MemCharFree()			// Find a free char slot
{
	P_CHAR pc = new cChar;
	return pc;
}

P_ITEM cCharStuff::AddRandomLoot(P_ITEM pBackpack, char * lootlist)
{
	char sect[512];
	int i,j, storeval,loopexit=0;
	P_ITEM retitem = NULL;
	storeval=-1;
	i=0; j=0;

	sprintf(sect, "LOOTLIST %s", lootlist);
	
	Script *pScpBase=i_scripts[npc_script];
	Script *pScp=pScpBase->Select(sect,custom_npc_script);
	if (!pScp) return NULL;

	loopexit=0;
	do
	{
		pScp->NextLine();
		if (script1[0]!='}')
		{
			i++; // Count number of entries on list.
		}
	} while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
	pScp->Close();

	if(i>0)
	{
		i=rand()%(i);
		pScp=pScpBase->Select(sect,custom_npc_script);

		loopexit=0;
		do
		{
			pScp->NextLine();
			if (script1[0]!='}')
			{
				if(j==i)
				{
					storeval=str2num(script1);	//script1 = ITEM#

					scpMark m=pScp->Suspend();
					retitem = Targ->AddMenuTarget(-1, 0, storeval);
					pScp->Resume(m);

					if(retitem!=NULL)
					{
						retitem->pos.x=50+(rand()%80);
						retitem->pos.y=50+(rand()%80);
						retitem->pos.z=9;
						retitem->setContSerial(pBackpack->serial);
					}
					break;;    
				}
				else j++;
			}
		}	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
		pScp->Close();
	}
	return retitem;
}

/*** s: socket ***/
int cCharStuff::getRandomNPC(char * npclist)
{
	//This function gets the random npc number from the list and recalls
	//addrespawnnpc passing the new number
	char sect[512];
	unsigned int uiTempList[100];
	int i=0,k=0;
	sprintf(sect, "NPCLIST %s", npclist);

	Script *pScpBase=i_scripts[npc_script];
	Script *pScp=pScpBase->Select(sect,custom_npc_script);
	if (!pScp) return 0;

	unsigned long loopexit=0;
	do
	{
		pScp->NextLine();
		if (script1[0]!='}')
		{
			uiTempList[i]=str2num(script1);
			i++;
		}
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS));
	pScp->Close();

	if(i>0)
	{
		i=rand()%(i);
		k=uiTempList[i];
	}
	return k;
}

void cCharStuff::Split(P_CHAR pc_k) // For NPCs That Split during combat
{
	int z;

	if ( pc_k == NULL ) return;
	
	P_CHAR pc_c = new cChar(*pc_k);
	if ( pc_c == NULL ) return;
//	pc_c->Init();
	pc_c->setSerial(cCharsManager::getInstance()->getUnusedSerial());
	pc_c->ftarg = INVALID_SERIAL;
	pc_c->moveTo(pc_k->pos + Coord_cl(1, 0, 0) );
	pc_c->kills = 0;
	pc_c->hp = pc_k->st;
	pc_c->stm = pc_k->realDex();
	pc_c->mn = pc_k->in;
	z=rand()%35;
	if (z == 5) 
		pc_c->setSplit(1); 
	else 
		pc_c->setSplit(0);	
	updatechar(pc_c);
}

// Applies a script-section to a char
void cCharStuff::applyNpcSection( P_CHAR Char, const QString &Section )
{
	QDomElement *NpcSection = DefManager->getSection( WPDT_NPC, Section );

	if( NpcSection->isNull() )
	{
		clConsole.log( "Unable to create unscripted NPC: %s", Section.latin1() );
		return;
	}

	QDomNodeList Tags = NpcSection->childNodes();

	UI16 i, j;

	for( i = 0; i < Tags.count(); i++ )
	{
		if( !Tags.item( i ).isElement() )
			continue;

		QDomElement Tag = Tags.item( i ).toElement();
		QString TagName = Tag.nodeName();
		QString Value;
		QDomNodeList ChildTags;

		if( Tag.hasChildNodes() && TagName != "backpack" && TagName != "equipped" && TagName != "shopkeeper" )
			for( j = 0; j < Tag.childNodes().count(); j++ )
			{
				if( Tag.childNodes().item( j ).isText() )
					Value += Tag.childNodes().item( j ).toText().data();
				else if( Tag.childNodes().item( j ).isElement() )
					Value += processNode( Tag.childNodes().item( j ).toElement() );
			}

		//<name>my char</name>
		if( TagName == "name" )
			Char->name = Value;
		
		//<backpack>
		//	<color>0x132</color>
		//	<item id="a">
		//	...
		//	<item id="z">
		//</backpack>
		else if( TagName == "backpack" )
			if( Char->packitem == INVALID_SERIAL )
			{
				P_ITEM pBackpack = Items->SpawnItem( -1, Char, 1, "Backpack", 0, 0x0E,0x75,0,0,0);

				if( pBackpack == NULL )
				{
					Npcs->DeleteChar( Char );
					return;
				}

				Char->packitem = pBackpack->serial;
				pBackpack->pos.x = 0;
				pBackpack->pos.y = 0;
				pBackpack->pos.z = 0;
				pBackpack->setContSerial(Char->serial);
				pBackpack->setLayer( 0x15 );
				pBackpack->setType( 1 );
				pBackpack->dye=1;

				if( Tag.hasChildNodes() )
					Items->applyItemSection(pBackpack, Tag);
			}

		//<carve>3</carve>
		else if( TagName == "carve" ) 
			Char->setCarve( Value.toInt() );

		//<cantrain />
		else if( TagName == "cantrain" )
			Char->setCantrain( true );

		//<direction>SE</direction>
		else if( TagName == "direction" )
			if( Value == "NE" )
				Char->dir=1;
			else if( Value == "E" )
				Char->dir=2;
			else if( Value == "SE" )
				Char->dir=3;
			else if( Value == "S" )
				Char->dir=4;
			else if( Value == "SW" )
				Char->dir=5;
			else if( Value == "W" )
				Char->dir=6;
			else if( Value == "NW" )
				Char->dir=7;
			else if( Value == "N" )
				Char->dir=0;

		//<stat type="str">100</stats>
		else if( TagName == "stat" )
			if( Tag.attributes().contains("type") )
			{
				QString statType = Tag.attributeNode("type").nodeValue();
				if( statType == "str" )
				{
					Char->st = Value.toShort();
					Char->st2 = Char->st;
					Char->hp = Char->st;
				}
				else if( statType == "dex" )
				{
					Char->setDex( Value.toShort() );
					Char->stm = Char->realDex();
				}
				else if( statType == "int" )
				{
					Char->in = Value.toShort();
					Char->in2 = Char->in;
					Char->mn = Char->in;
				}
			}

		//<defense>10</defense>
		else if( TagName == "defense" )
			Char->def = Value.toUInt();

		//<attack>10</attack>
		else if( TagName == "attack" )
			Char->att = Value.toUInt();

		//<emotecolor>0x482</emotecolor>
		else if( TagName == "emotecolor" )
			Char->emotecolor = Value.toUShort();

		//<fleeat>10</fleeat>
		else if( TagName == "fleeat" )
			Char->setFleeat( Value.toShort() );

		//<fame>8000</fame>
		else if( TagName == "fame" )
			Char->fame = Value.toInt();

		//<gold>100</gold>
		else if( TagName == "gold" )
			if( Char->packitem != INVALID_SERIAL )
			{
				P_ITEM pGold = Items->SpawnItem(Char,1,"#",1,0x0EED,0,1);
				if(pGold == NULL)
				{
					Npcs->DeleteChar(Char);
					return;
				}
				pGold->priv |= 0x01;

				pGold->setAmount( Value.toInt() );
			}
			else
				clConsole.send((char*)QString("Warning: Bad NPC Script %1: no backpack for gold.\n").arg( Section ).latin1());

		//<hidamage>10</hidamage>
		else if( TagName == "hidamage" )
			Char->hidamage = Value.toInt();

		//<haircolor>2</haircolor> (colorlist)
		else if( TagName == "haircolor" )
		{
			unsigned short haircolor = addrandomhaircolor(Char, (char*)Value.latin1());
			if( haircolor != -1 )
				Char->setHairColor( haircolor );
		}

		//<id>0x11</id>
		else if( TagName == "id" )
		{
			Char->setId( Value.toUShort() );
			Char->xid = Char->id();
		}

		//<karma>-500</karma>
		else if( TagName == "karma" )
			Char->karma = Value.toInt();

		//<loot>3</loot>
		else if( TagName == "loot" )
			if( Char->packitem != INVALID_SERIAL )
				Npcs->AddRandomLoot( FindItemBySerial(Char->packitem), (char*)Value.latin1() );
			else
				clConsole.send((char*)QString("Warning: Bad NPC Script %1: no backpack for loot.\n").arg( Section ).latin1());

		//<lodamage>10</lodamage>
		else if( TagName == "lodamage" )
			Char->lodamage = Value.toInt();

		//<notrain />
		else if( TagName == "notrain" )
			Char->setCantrain( false );

		//<npcwander type="rectangle" x1="-10" x2="12" y1="5" y2="7" />
		//<......... type="rect" ... />
		//<......... type="3" ... />
		//<......... type="circle" radius="10" />
		//<......... type="2" ... />
		//<......... type="free" (or "1") />
		//<......... type="none" (or "0") />
		else if( TagName == "npcwander" )
			if( Tag.attributes().contains("type") )
			{
				QString wanderType = Tag.attributeNode("type").nodeValue();
				if( wanderType == "rectangle" || wanderType == "rect" || wanderType == "3" )
					if( Tag.attributes().contains("x1") &&
						Tag.attributes().contains("x2") &&
						Tag.attributes().contains("y1") &&
						Tag.attributes().contains("y2") )
					{
						Char->npcWander = 3;
						Char->fx1 = Char->pos.x + Tag.attributeNode("x1").nodeValue().toInt();
						Char->fx2 = Char->pos.x + Tag.attributeNode("x2").nodeValue().toInt();
						Char->fy1 = Char->pos.y + Tag.attributeNode("y1").nodeValue().toInt();
						Char->fy2 = Char->pos.y + Tag.attributeNode("y2").nodeValue().toInt();
						Char->fz1 = -1;
					}
				else if( wanderType == "circle" || wanderType == "2" )
				{
					Char->npcWander = 2;
					Char->fx1 = Char->pos.x;
					Char->fy1 = Char->pos.y;
					Char->fz1 = Char->pos.z;
					if( Tag.attributes().contains("radius") )
						Char->fx2 = Tag.attributeNode("radius").nodeValue().toInt();
					else
						Char->fx2 = 2;
				}
				else if( wanderType == "free" || wanderType == "1" )
					Char->npcWander = 1;
				else
					Char->npcWander = 0; //default
			}
		//<ai>2</ai>
		else if( TagName == "ai" )
			Char->setNpcAIType( Value.toInt() );

		//<onhorse />
		else if( TagName == "onhorse" )
			Char->setOnHorse( true );
		//<priv1>0</priv1>
		else if( TagName == "priv1" )
			Char->setPriv( Value.toUShort() );

		//<priv2>0</priv2>
		else if( TagName == "priv2" )
			Char->priv2 = Value.toUShort();
		//<poison>2</poison>
		else if( TagName == "poison" )
			Char->setPoison( Value.toInt() );
		//<reattackat>40</reattackat>
		else if( TagName == "reattackat" )
			Char->setReattackat( Value.toShort() );

		//<skin>0x342</skin>
		else if( TagName == "skin" )
		{
			Char->setSkin( Value.toUShort() );
			Char->setXSkin( Value.toUShort() );
		}

		//<shopkeeper>
		//	<sellitems>...handled like item-<contains>-section...</sellitems>
		//	<shopitems>...see above...</shopitems>
		//	<rshopitems>...see above...</rshopitems>
		//</shopkeeper>
		else if( TagName == "shopkeeper" && Tag.hasChildNodes() )
		{
			Commands->MakeShop( Char );
			for( j = 0; j < Tag.childNodes().count(); j++ )
			{
				QDomElement currNode = Tag.childNodes().item( j ).toElement();
				
				if( !currNode.hasChildNodes() )
					continue;

				unsigned char contlayer = 0;
				if( currNode.nodeName() == "rshopitems" )
					contlayer = 0x1A;
				else if( currNode.nodeName() == "shopitems" )
					contlayer = 0x1B;
				else if( currNode.nodeName() == "sellitems" )
					contlayer = 0x1C;
				else 
					continue;
				
				P_ITEM contItem = Char->GetItemOnLayer( contlayer );
				if( contItem != NULL )
					Items->processItemContainerNode( contItem, currNode );
				else
					clConsole.send((char*)QString("Warning: Bad NPC Script %1: no shoppack for item.\n").arg( Section ).latin1());
			}
		}
		
		//<spattack>3</spattack>
		else if( TagName == "spattack" )
			Char->spattack = Value.toInt();

		//<speech>13</speech>
		else if( TagName == "speech" )
			Char->speech = Value.toUShort();

		//<split>1</split>
		else if( TagName == "split" )
			Char->setSplit( Value.toUShort() );

		//<splitchance>10</splitchance>
		else if( TagName == "splitchance" )
			Char->setSplitchnc( Value.toUShort() );

		//<saycolor>0x110</saycolor>
		else if( TagName == "saycolor" )
			Char->saycolor = Value.toUShort();

		//<spadelay>3</spadelay>
		else if( TagName == "spadelay" )
			Char->spadelay = Value.toInt();

		//<stablemaster />
		else if( TagName == "stablemaster" )
			Char->setNpc_type(1);

		//<title>the king</title>
		else if( TagName == "title" )
			Char->setTitle( Value );

		//<totame>115</totame>
		else if( TagName == "totame" )
			Char->taming = Value.toInt();

		//<trigger>3</trigger>
		else if( TagName = "trigger" )
			Char->setTrigger( Value.toInt() );

		//<trigword>abc</trigword>
		else if( TagName == "trigword" )
			Char->setTrigword( Value );

		//<skill type="alchemy">100</skill>
		//<skill type="1">100</skill>
		else if( TagName == "skill" && Tag.attributes().contains("type") )
		{
			if( Tag.attributeNode("type").nodeValue().toInt() > 0 &&
				Tag.attributeNode("type").nodeValue().toInt() <= ALLSKILLS )
				Char->setBaseSkill((Tag.attributeNode("type").nodeValue().toInt() - 1), Value.toInt());
			else
				for( j = 0; j < ALLSKILLS; j++ )
					if( Tag.attributeNode("type").nodeValue().contains( QString(skillname[j]), false ) )
						Char->setBaseSkill(j, Value.toInt());
		}

		//<equipped>
		//	<item id="a" />
		//	<item id="b" />
		//	...
		//</epuipped>
		else if( TagName == "equipped" && Tag.hasChildNodes() )
		{
			for( j = 0; j < Tag.childNodes().count(); j++ )
			{
				QDomElement currChild = Tag.childNodes().item( j ).toElement();
				if( currChild.nodeName() == "item" && currChild.attributes().contains("id") )
				{
					P_ITEM nItem = Items->createScriptItem( currChild.attributeNode("id").nodeValue() );
					if( nItem == NULL )
						continue;
					else if( nItem->layer() == 0 )
					{
						clConsole.send((char*)QString("Warning: Bad NPC Script %1: item to equip has no layer.\n").arg( Section ).latin1());
						Items->DeleItem( nItem );
						continue;
					}
					else
						nItem->setContSerial( Char->serial );

					if( currChild.hasChildNodes() )  // color
						Items->processScriptItemNode( nItem, currChild );
				}
			}
		}
	}
}

P_CHAR cCharStuff::createScriptNpc( int s, P_ITEM pi_i, QString Section, int posx, int posy, signed char posz )
{
	if( Section.length() == 0 )
		return NULL;

	int k=0, xos=0, yos=0, lb;
	short postype;				// determines how xyz of the new NPC are set, see below
	unsigned long loopexit=0;

	P_CHAR nChar = MemCharFree();
	nChar->Init( true );
	cCharsManager::getInstance()->registerChar( nChar );

	nChar->setPriv(0x10);
	nChar->npc=1;
	nChar->att=1;
	nChar->def=1;
	nChar->setSpawnSerial( INVALID_SERIAL );	

	if (posx > 0 && posy > 0)
 		postype = 3;	// take position from parms
	else if ( s != -1 && pi_i == NULL)
		postype = 2;	// take position from socket's buffer
	else if ( s == -1 && pi_i != NULL)
		postype = 1;	// take position from items[i]
	else
	{
		clConsole.send("ERROR: bad parms in call to createScriptNpc\n");
		return NULL;
	}

	// Now that we have created the NPC, lets place him
	switch (postype)
	{
	case 1:					// take position from (spawning) item
		if (triggerx)
		{
			nChar->pos.x=triggerx;
			nChar->pos.y=triggery;
			nChar->dispz=nChar->pos.z=triggerz;
			triggerx = nChar->serial;
		} else
		{
	   /*Zippy's Code chages for area spawns --> (Type 69) xos and yos (X OffSet, Y OffSet) 
	   are used to find a random number that is then added to the spawner's x and y (Using 
	   the spawner's z) and then place the NPC anywhere in a square around the spawner. 
	   This square is random anywhere from -10 to +10 from the spawner's location (for x and 
	   y) If the place chosen is not a valid position (the NPC can't walk there) then a new 
	   place will be chosen, if a valid place cannot be found in a certain # of tries (50), 
		   the NPC will be placed directly on the spawner and the server op will be warned. */
			if ((pi_i->type() == 69 || pi_i->type() == 125)&& pi_i->isInWorld())
			{
				if (pi_i->more3==0) pi_i->more3=10;
				if (pi_i->more4==0) pi_i->more4=10;
			   //signed char z, ztemp, found;
	   
				k=0;
				do
				{
					if (k>=50) //this CAN be a bit laggy. adjust as nessicary
					{
						clConsole.send("WOLFPACK: Problem area spawner found at [%i,%i,%i]. NPC placed at default location.\n",pi_i->pos.x,pi_i->pos.y,pi_i->pos.z);
						xos=0;
						yos=0;
						break;
					}
					xos=RandomNum(-pi_i->more3,pi_i->more3);
					yos=RandomNum(-pi_i->more4,pi_i->more4);
					// clConsole.send("Spawning at Offset %i,%i (%i,%i,%i) [-%i,%i <-> -%i,%i]. [Loop #: %i]\n",xos,yos,pi_i->x+xos,pi_i->y+yos,pi_i->z,pi_i->more3,pi_i->more3,pi_i->more4,pi_i->more4,k); /** lord binary, changed %s to %i, crash when uncommented ! **/
					k++;
					if ((pi_i->pos.x+xos<1) || (pi_i->pos.y+yos<1)) lb=0; /* lord binary, fixes crash when calling npcvalid with negative coordiantes */
					else lb = Movement->validNPCMove(pi_i->pos.x+xos,pi_i->pos.y+yos,pi_i->pos.z, nChar);				 
				   
					//Bug fix Monsters spawning on water:
					MapStaticIterator msi(pi_i->pos + Coord_cl(xos, yos, 0));

					staticrecord *stat;
					loopexit=0;
					while ( (stat = msi.Next()) && (++loopexit < MAXLOOPS) )
					{
						tile_st tile;
						msi.GetTile(&tile);
						if(!(strcmp((char *) tile.name, "water")))//Water
						{//Don't spawn on water tiles... Just add other stuff here you don't want spawned on.
							lb=0;
						}
					}
				} while (!lb);
			} // end Zippy's changes (exept for all the +xos and +yos around here....)

			nChar->pos.x=pi_i->pos.x+xos;
			nChar->pos.y=pi_i->pos.y+yos;
			nChar->dispz=nChar->pos.z=pi_i->pos.z;
			nChar->SetSpawnSerial(pi_i->serial);
			if ( pi_i->type() == 125 )
			{
				MsgBoardQuestEscortCreate( nChar );
			}
		} // end of if !triggerx
		break;
	case 2: // take position from Socket
		if (s!=-1)
		{
			nChar->pos.x=(buffer[s][11]<<8)+buffer[s][12];
			nChar->pos.y=(buffer[s][13]<<8)+buffer[s][14];
			nChar->dispz=nChar->pos.z=buffer[s][16]+Map->TileHeight((buffer[s][17]<<8)+buffer[s][18]);
		}
		break;
	case 3: // take position from Parms
		nChar->pos.x=posx;
		nChar->pos.y=posy;
		nChar->dispz=nChar->pos.z=posz;
		break;
	} // no default coz we tested on entry to function
   
	nChar->region = calcRegionFromXY(nChar->pos.x, nChar->pos.y);

	applyNpcSection( nChar, Section );

	return nChar;
}

////////////
// Name:	inGuardedArea
// history:	by Duke, 13.1.2002
// Purpose:	checks if the char is in a guarded region
// Remark:	the recalculation of the region is necessary because it is not maintained properly :(
//			I think it is better to do this only when needed
//
bool cChar::inGuardedArea()
{
	this->region=calcRegionFromXY(this->pos.x, this->pos.y);	// make sure it is set correctly
	return ::region[this->region].priv&1;
}

