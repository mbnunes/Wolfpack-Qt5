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

#include "menuactions.h"
#include "srvparams.h"

// Library Includes
#include "qdom.h"

enum eItemState
{
	IS_NORMAL = 0,
	IS_EXCEPTIONAL,
	IS_SUPERIOR
};

void playSkillSound( UOXSOCKET Socket, UI08 Skill, bool Success )
{
	P_CHAR Character = currchar[ Socket ];

	if( Character == NULL )
		return;

	// what else can i say, do actions based on the skill
	// tailoring
	// carpentry 
	// Blacksmithing (!)
	
	switch( Skill )
	{
	// Alchemy
	case 0x00:
		//soundeffect2( Character, 0x243 );
		// Alchemy itself does not make any sound. 
		// What we do is using resource - sound - using resource - sound - using resouce - sound - skilcheck - itemadd - sound
		break;

	// Blacksmithing
	case 0x07:
		action( Socket, 0x9 );
		soundeffect2( Character, 0x2A );
		break;

	// Carpentry
	case 0x11:
		action( Socket, 0x9 );
		soundeffect2( Character, 0x23D );
		break;
	
	};
}

void applyRank( UOXSOCKET Socket, P_ITEM Item, UI08 Rank )
{
	QString Message;

	// Variables to change: LODAMAGE,HIDAMAGE,ATT,DEF,HP,MAXHP
	if( Item->lodamage > 0 ) 
		Item->lodamage = ( Rank * Item->lodamage ) / 7;

	if( Item->hidamage > 0 ) 
		Item->hidamage = ( Rank * Item->hidamage ) / 7;

	if( Item->att > 0 ) 
		Item->att = ( Rank * Item->att ) / 7;

	if( Item->def > 0 ) 
		Item->def = ( Rank * Item->def ) / 7;

	if( Item->hp > 0 ) 
		Item->hp = ( Rank * Item->hp ) / 7;

	if( Item->maxhp > 0 ) 
		Item->maxhp = ( Rank * Item->maxhp ) / 7;

	switch( Rank )
	{
		case 1: 
			Message = "You made an item with no quality!";
			break;

		case 2: 
			Message = "You made an item very below standard quality!";
			break;

		case 3: 
			Message = "You made an item below standard quality!";
			break;

		case 4: 
			Message = "You made a weak quality item!";
			break;

		case 5: 
			Message = "You made a standard quality item!";
			break;

		case 6: 
			Message = "You made a nice quality item!";
			break;

		case 7: 
			Message = "You made a good quality item!";
			break;

		case 8: 
			Message = "You made a great quality item!";
			break;

		case 9: 
			Message = "You made a beautiful quality item!";
			break;

		case 10: 
			Message = "You made a perfect quality item!";
			break;
	}

	sysmessage( Socket, Message );
}

// Calculates a rank for the given parameters
UI08 calcRank( P_CHAR Character, UI08 Skill, UI08 MinSkill, UI08 MaxSkill )
{
	// Minrank and Maxrank are 1 and 10

	int rk_range,rank;
	float sk_range,randnum,randnum1;

	rk_range = 10;
	sk_range = (float)50.00 + Character->skill[ Skill ] - MinSkill;
	
	if( sk_range <= 0 )
		rank = 1;
	else if ( sk_range >= 1000 )
		rank = 10;

	randnum = static_cast<float>(rand()%1000);

	if( randnum <= sk_range )
		rank = 10;
	else
	{
		if( SrvParams->skillLevel() == 11 )
			randnum1 = (float)( rand() % 1000 );
		else
			randnum1 = (float)( rand() % 1000 ) - ( ( randnum - sk_range ) / ( 11 - SrvParams->skillLevel() ) );

		rank = (int)( randnum1 * rk_range ) / 1000;
		
		if( rank > 10 ) rank = 10;
		if( rank < 1 ) rank = 1;
	}

	return rank;
}

// This Function will actually CONSUME the stuff
UI32 consumeResources( P_ITEM Container, UI32 Amount, const vector< UI16 > &MatchingIDs, const vector< UI16 >&MatchingColors, const QString &MatchingName, const UI08 MatchingType, bool CheckID, bool CheckColor, bool CheckName, bool CheckType )
{
	// Relatively easy:
	vector< SERIAL > ItemList = contsp.getData( Container->serial );

	if( ItemList.empty() )
		return Amount;

	// Walk all contained items
	for( UI32 i = 0; i < ItemList.size(); i++ )
	{
		P_ITEM Item = FindItemBySerial( ItemList[ i ] );

		if( Item == NULL )
			continue;

		// If it's a container add it's contained items to our amount (recursive)
		if( Item->type == 1 )
		{
			Amount = consumeResources( Item, Amount, MatchingIDs, MatchingColors, MatchingName, MatchingType, CheckID, CheckColor, CheckName, CheckType );

			if( Amount == 0 )
				return Amount;

			continue;
		}

		// Before we start checking the item we should ofcourse check FIRST if the user
		// is even able to see the resource AND if the user can even move it
		if( ( Item->magic != 0 ) && ( Item->magic != 1 ) )
			continue;
		 
		// Check if the Item (it's not a container)
		// matches the requested informations
		bool MatchID = true;
		bool MatchColor = true; // By default we ignore color
	
		// ======= ID =========
		if( CheckID )
		{
			MatchID = false;

			for( UI08 j = 0; j < MatchingIDs.size(); j++ )
			{
				if( MatchingIDs[ j ] == Item->id() )
				{
					MatchID = true;
					break; // We already found out that this is one of the item-ids we want
				}
			}
		}

		// If we didnt find any Matching IDs we should continue with the next item
		if( !MatchID )
			continue;

		// ======= COLOR =======
		if( CheckColor )
		{
			MatchColor = false; // We are forced to search for a color so by default it does NOT match

			for( UI08 j = 0; j < MatchingColors.size(); j++ )
			{
				if( MatchingColors[ j ] == Item->color() )
				{
					MatchColor = true;
				}
			}
		}

		if( !MatchColor )
			continue;

		// ======== TYPE =======
		if( CheckType && ( Item->type != MatchingType ) )
				continue;

		// ======== NAME =======
		if( CheckName && ( Item->name() != MatchingName ) )
				continue;			

		// The Item matches our criterias

		// We have less in the pile than we need to have
		if( Item->amount() < Amount )
		{
			Amount -= Item->amount();
			Items->DeleItem( Item );
		}
		// The Item stack is bigger than the amount
		else
		{
			Item->ReduceAmount( Amount );
			Amount = 0;
			return 0;
		}
	}

	return Amount;
}

// This function will count all specified resources in Sockets' pack
UI32 countResources( P_ITEM Container, const vector< UI16 > &MatchingIDs, const vector< UI16 >&MatchingColors, const QString &MatchingName, const UI08 MatchingType, bool CheckID, bool CheckColor, bool CheckName, bool CheckType )
{
	UI32 Amount = 0;

	vector< SERIAL > Items = contsp.getData( Container->serial );

	if( Items.size() < 1 )
		return 0;

	// Walk all contained items
	for( UI32 i = 0; i < Items.size(); i++ )
	{
		P_ITEM Item = FindItemBySerial( Items[ i ] );

		if( Item == NULL )
			continue;

		// If it's a container add it's contained items to our amount (recursive)
		if( Item->type == 1 )
		{
			Amount += countResources( Item, MatchingIDs, MatchingColors, MatchingName, MatchingType, CheckID, CheckColor, CheckName, CheckType );
			continue;
		}

		// Before we start checking the item we should ofcourse check FIRST if the user
		// is even able to see the resource AND if the user can even move it
		if( ( Item->magic != 0 ) && ( Item->magic != 1 ) )
			continue;

		// Check if the Item (it's not a container)
		// matches the requested informations
		bool MatchID = true;
		bool MatchColor = true; // By default we ignore color
	
		// ======= ID =========
		if( CheckID )
		{
			MatchID = false;

			for( UI08 j = 0; j < MatchingIDs.size(); j++ )
			{
				if( MatchingIDs[ j ] == Item->id() )
				{
					MatchID = true;
					break; // We already found out that this is one of the item-ids we want
				}
			}
		}

		// If we didnt find any Matching IDs we should continue with the next item
		if( !MatchID )
			continue;

		// ======= COLOR =======
		if( CheckColor )
		{
			MatchColor = false; // We are forced to search for a color so by default it does NOT match

			for( UI08 j = 0; j < MatchingColors.size(); j++ )
			{
				if( MatchingColors[ j ] == Item->color() )
				{
					MatchColor = true;
				}
			}
		}

		if( !MatchColor )
			continue;

		// ======== TYPE =======
		if( CheckType && ( Item->type != MatchingType ) )
				continue;

		// ======== NAME =======
		if( CheckName && ( Item->name() != MatchingName ) )
				continue;			

		Amount += Item->amount();
	}

	return Amount;
}

// ACTIONS YOU CAN USE IN YOUR XML ACTION FILES
bool useItem( UOXSOCKET Socket, QDomElement& Action, bool Failed )
{
	bool CheckColor = false;
	bool CheckName = false;
	bool CheckType = false;
	bool CheckID = false;
	UI08 i;
	
	vector< UI16 > MatchingIDs;
	vector< UI16 > MatchingColors;
	QString MatchingName;
	UI32 Amount = 1;
	UI08 MatchingType;

	// Only check for the ID if the users asks for it
	if( Action.attributes().contains( "id" ) )
	{
		QStringList MatchingStrIDs = QStringList::split( ",", Action.attributeNode( "id" ).nodeValue() );
	
		for( i = 0; i < MatchingStrIDs.count(); i++ )
			MatchingIDs.push_back( MatchingStrIDs[ i ].toUShort( NULL, 16 ) );

		if( MatchingIDs.size() > 0 )
			CheckID = true;
	}

	// We have Color(s) to check for
	if( Action.attributes().contains( "color" ) )
	{
		QStringList ColorStrings = QStringList::split( ",", Action.attributeNode( "color" ).nodeValue() );

		for( i = 0; i < ColorStrings.count(); i++ )
			MatchingColors.push_back( ColorStrings[ i ].toUShort( NULL, 16 ) );

		if( MatchingColors.size() > 0 )
			CheckColor = true;
	}

	// Does the admin want us to check for the name?
	if( Action.attributes().contains( "name" ) )
	{
		MatchingName = Action.attributeNode( "name" ).nodeValue();
		CheckName = true;
	}

	// Amount?
	if( Action.attributes().contains( "amount" ) )
	{
		Amount = Action.attributeNode( "amount" ).nodeValue().toUInt();
	}

	// Type?
	if( Action.attributes().contains( "type" ) )
	{
		MatchingType = Action.attributeNode( "type" ).nodeValue().toUShort();
		CheckType = true;
	}

	if( !CheckID && !CheckColor && !CheckName && !CheckType )
		return true;

	// As we're supporting MORE than one item-type we need to count all items first
	// If the user does not have a backpack then we cannot count the resources he
	// needs to make a specific item = fail
	P_CHAR Character = currchar[ Socket ];
	if( Character == NULL )
		return false;

	P_ITEM Backpack = Packitem( Character );
	if( Backpack == NULL )
		return false;

	// Before we start to consume the resources 
	if( Failed )
	{
		Amount = (UI32)floor( Amount / 2 );
		
		if( Amount < 1 )
			Amount = 1;
	}

	UI32 Found = countResources( Backpack, MatchingIDs, MatchingColors, MatchingName, MatchingType, CheckID, CheckColor, CheckName, CheckType );
	if(  Found < Amount )
	{
		// Display the failure
		// If there's a custom failure message display it
		if( Action.attributes().contains( "fail" ) )
		{
			sysmessage( Socket, Action.attributeNode( "fail" ).nodeValue() );
		}
		else
		{
			sysmessage( Socket, "You lack the resources to make this!" );
		}

		return false;
	}

	// Now after we know that we have enough resources we're going to consume them
	consumeResources( Backpack, Amount, MatchingIDs, MatchingColors, MatchingName, MatchingType, CheckID, CheckColor, CheckName, CheckType );

	return true;
}

// Checks if the user has a specific item
bool hasItem( UOXSOCKET Socket, QDomElement& Action )
{
	bool CheckColor = false;
	bool CheckName = false;
	bool CheckType = false;
	bool CheckID = false;
	UI08 i;
	
	vector< UI16 > MatchingIDs;
	vector< UI16 > MatchingColors;
	QString MatchingName;
	UI32 Amount = 1;
	UI08 MatchingType;

	// Only check for the ID if the users asks for it
	if( Action.attributes().contains( "id" ) )
	{
		QStringList MatchingStrIDs = QStringList::split( ",", Action.attributeNode( "id" ).nodeValue() );
	
		for( i = 0; i < MatchingStrIDs.count(); i++ )
			MatchingIDs.push_back( MatchingStrIDs[ i ].toUShort( NULL, 16 ) );

		if( MatchingIDs.size() > 0 )
			CheckID = true;
	}

	// We have Color(s) to check for
	if( Action.attributes().contains( "color" ) )
	{
		QStringList ColorStrings = QStringList::split( ",", Action.attributeNode( "color" ).nodeValue() );

		for( i = 0; i < ColorStrings.count(); i++ )
			MatchingColors.push_back( ColorStrings[ i ].toUShort( NULL, 16 ) );

		if( MatchingColors.size() > 0 )
			CheckColor = true;
	}

	// Does the admin want us to check for the name?
	if( Action.attributes().contains( "name" ) )
	{
		MatchingName = Action.attributeNode( "name" ).nodeValue();
		CheckName = true;
	}

	// Amount?
	if( Action.attributes().contains( "amount" ) )
	{
		Amount = Action.attributeNode( "amount" ).nodeValue().toUInt();
	}

	// Type?
	if( Action.attributes().contains( "type" ) )
	{
		MatchingType = Action.attributeNode( "type" ).nodeValue().toUShort();
		CheckType = true;
	}

	if( !CheckID && !CheckColor && !CheckName && !CheckType )
		return true;

	// As we're supporting MORE than one item-type we need to count all items first
	// If the user does not have a backpack then we cannot count the resources he
	// needs to make a specific item = fail
	P_CHAR Character = currchar[ Socket ];
	if( Character == NULL )
		return false;

	P_ITEM Backpack = Packitem( Character );
	if( Backpack == NULL )
		return false;

	// This is exactly the same as in useItem
	// But we just dont use the resources but just check for them
	UI32 Found = countResources( Backpack, MatchingIDs, MatchingColors, MatchingName, MatchingType, CheckID, CheckColor, CheckName, CheckType );
	if(  Found < Amount )
	{
		// Display the failure
		// If there's a custom failure message display it
		if( Action.attributes().contains( "fail" ) )
		{
			sysmessage( Socket, Action.attributeNode( "fail" ).nodeValue() );
		}
		else
		{
			sysmessage( Socket, "You lack the resources to make this!" );
		}

		return false;
	}
	
	return true;
}

// This will perform a skillcheck
// If will play sounds if it's not "muted" by using quiet="1"
// And if it fails it will consoume half of the resources needed
// To create the item
bool checkSkill( UOXSOCKET Socket, QDomElement& Action )
{
	SI32 Minimum = 0;
	SI32 Maximum = 1000;
	UI08 Skill;

	P_CHAR Character = currchar[ Socket ];

	if( Character == NULL )
		return false;

	if( !Action.attributes().contains( "id" ) )
		return false;

	Skill = Action.attributeNode( "id" ).nodeValue().toUShort();
	
	// Minimum and Maximum ARE optional
	if( Action.attributes().contains( "min" ) )
		Minimum = Action.attributeNode( "min" ).nodeValue().toInt();

	if( Action.attributes().contains( "max" ) )
		Maximum = Action.attributeNode( "max" ).nodeValue().toInt();

	// We only make items depending on the server-setting
	if( Character->baseskill[ Skill ] < Minimum ) 
	{
		// If the server owner wants it
		//if( !SrvParams->BelowMinSkillFails() ) <<<<< DOESNT WORK ?!?!?
		//{
			sysmessage( Socket, "You lack the skill to make this item" );
			return false;
		//}			
	}

	// Play the sound for the corresponding skill
	playSkillSound( Socket, Skill, true );

	if( ( Character->baseskill[ Skill ] >= Minimum ) && ( Skills->CheckSkill( Character, Skill, Minimum, Maximum ) ) )
		return true;

	// Handle the failure(!)
	QString Message = "You did not manage to make the item";

	// If there's a custom failure message display it instead of ours
	if( Action.attributes().contains( "fail" ) )
		Message = Action.attributeNode( "fail" ).nodeValue();

	sysmessage( Socket, Message );

	// After we notified the user use up 1/2 of the resources
	QDomNodeList Nodes = Action.parentNode().childNodes();
	for( SI32 i = 0; i < Nodes.count(); i++ )
	{
		// Only process Resource nodes
		if( ( !Nodes.item( i ).isElement() ) || ( Nodes.item( i ).toElement().nodeName() != "useitem" ) )
			continue;

		// Process the Resource Node
		useItem( Socket, Nodes.item( i ).toElement(), true );
	}

	return false;
}

// This will craft an item
bool makeItem( UOXSOCKET Socket, QDomElement& Action )
{
	// Things we need to do in this "node":
	// Calculate a Rank for the created item
	// do that not if we dont find a skill in the parent node
	// or if rank="no" is set
	bool doRanking = true;
	bool foundSkill = false;
	P_CHAR Character = currchar[ Socket ];

	if( Character == NULL )
		return false;

	// Search for the first skill-node
	QDomNodeList NodeList = Action.parentNode().childNodes();

	UI08 Rank = 7;

	for( UI32 i = 0; i < NodeList.count(); i++ )
	{
		if( ( !NodeList.item( i ).isElement() ) && ( NodeList.item( i ).nodeName() != "checkskill" ) )
			continue;

		if( !Action.attributes().contains( "id" ) )
			continue;

		UI08 Skill = Action.attributeNode( "id" ).nodeValue().toUShort();
		
		UI16 MinSkill = 0;
		UI16 MaxSkill = 1000;

		if( Action.attributes().contains( "min" ) )
			MinSkill = Action.attributeNode( "min" ).nodeValue().toULong();

		if( Action.attributes().contains( "max" ) )
			MaxSkill = Action.attributeNode( "max" ).nodeValue().toULong();

		Rank = calcRank( Character, Skill, MinSkill, MaxSkill );
		break;
	}

	// Create the item in the users backpack
	UI32 ItemNum = Action.attributeNode( "id" ).nodeValue().toUInt();

	P_ITEM Item = Items->SpawnItemBackpack2( Socket, ItemNum, false );

	applyRank( Socket, Item, Rank );

	sysmessage( Socket, "You put the item in your backpack." );

	// 0x58
	if( Character != NULL )
		soundeffect2( Character, 0x57 );

	return true;
}

