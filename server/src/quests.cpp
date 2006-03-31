/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2006 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In addition to that license, if you are running this program or modified
 * versions of it on a public system you HAVE TO make the complete source of
 * the version used by you available or provide people with a location to
 * download it.
 *
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#include "definitions.h"
#include "basics.h"
#include "serverconfig.h"
#include "log.h"
#include "console.h"
#include "quests.h"

// System Includes
#include <math.h>

void cQuests::load()
{
	quests.clear();

	// Try to get all Quests (Max 10.000 quests)
	quint32 i;

	for ( i = 1; i < 10000; ++i )
	{
		const cElement* quest = Definitions::instance()->getDefinition( WPDT_QUEST, QString::number( i ) );

		if ( !quest )
		{
			Console::instance()->send( tr( "QUESTS: %1 Quests Found.\n" ).arg( i - 1 ) );
			break;
		}

		stQuest nQuest;

		for ( unsigned int j = 0; j < quest->childCount(); ++j )
		{
			const cElement* node = quest->getChild( j );
			// Basics
			if ( node->name() == "name" )
				nQuest.name = node->text();
			else if ( node->name() == "description" )
				nQuest.description = node->text();
			// Required Section
			else if ( node->name() == "requiredquests" )
				nQuest.requiredQuests = node->text();
			else if ( node->name() == "requiredclasses" )
				nQuest.requiredClasses = node->text();
			else if ( node->name() == "requiredraces" )
				nQuest.requiredRaces = node->text();
			else if ( node->name() == "requiredlevel" )
				nQuest.requiredLevel = node->text();
			else if ( node->name() == "requiredextra1" )
				nQuest.requiredExtra1 = node->text();
			else if ( node->name() == "requiredextra2" )
				nQuest.requiredExtra2 = node->text();
			else if ( node->name() == "requiredextra3" )
				nQuest.requiredExtra3 = node->text();
			// Module: Kill NPCs
			else if ( node->name() == "npctargets" )
				nQuest.npcTargets = node->text();
			else if ( node->name() == "npcamounts" )
				nQuest.npcAmounts = node->text();
			else if ( node->name() == "npcregions" )
				nQuest.npcRegions = node->text();
			// Module: Wanted Items
			else if ( node->name() == "itemtargets" )
				nQuest.itemTargets = node->text();
			else if ( node->name() == "itemamounts" )
				nQuest.itemAmounts = node->text();
			else if ( node->name() == "itemloots" )
				nQuest.itemLoots = node->text();
			else if ( node->name() == "itemlootregions" )
				nQuest.itemLootRegions = node->text();
			// Module: Timed Quests
			else if ( node->name() == "questtimer" )
				nQuest.questTimer = node->text();
			// Module: Report
			else if ( node->name() == "reportuid" )
				nQuest.reportUID = node->text();
			else if ( node->name() == "reportid" )
				nQuest.reportID = node->text();
			// Rewards
			else if ( node->name() == "rewarditems" )
				nQuest.rewardItems = node->text();
			else if ( node->name() == "rewardamounts" )
				nQuest.rewardAmounts = node->text();
			else if ( node->name() == "rewardxp" )
				nQuest.rewardXp = node->text();
			// Functions of each quest
			else if ( node->name() == "functionassign" )
				nQuest.functionAssign = node->text();
			else if ( node->name() == "functionresign" )
				nQuest.functionResign = node->text();
			else if ( node->name() == "functionfail" )
				nQuest.functionFail = node->text();
			else if ( node->name() == "functioncomplete" )
				nQuest.functionComplete = node->text();
		}

		quests.push_back( nQuest );
	}

}

void cQuests::unload()
{
	quests.clear();
}

const QString& cQuests::getQuestName( quint16 quest ) const
{
	if ( quest >= quests.size() )
	{
		Console::instance()->log( LOG_ERROR, tr( "Quest id out of range: %u" ).arg( quest ) );
		return QString::null;
	}

	return quests[quest].name;
}
