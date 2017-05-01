/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2017 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: https://github.com/Mutilador/Wolfpack
 */

#include "defines.h"

#include "typedefs.h"
#include "targetrequests.h"
#include "objectdef.h"

// Library Includes
#include <QMap>
#include <qstringlist.h>
#include <QList>

// Forward Declaration
class cUOSocket;

struct stQuest
{
	QString name,
		    description,
			// Required Section
			requiredQuests,		// Quests Required before assign this quest
			requiredClasses,	// Classes that are allowed to assign this quest (Class Shards)
			requiredRaces,		// Races that are allowed to assign this quest (Race Shards)
			requiredLevel,		// Level that is allowed to assign this quest (XP and Level Shards)
			requiredExtra1,		// Extra field for required things
			requiredExtra2,		// Extra field for required things
			requiredExtra3,		// Extra field for required things
			// Module: Kill NPCs
			npcTargets,			// IDs of NPCs that have to be killed to complete this Quest 
			npcAmounts,			// Amounts of each NPC in "npctargets" that have to be killed
			npcRegions,			// Regions where each NPC in "npctargets" have to be killed
			// Module: Wanted Items
			itemTargets,		// IDs of Items that have to on Backpack to complete this quest
			itemAmounts,		// Amounts of each item in "itemtargets" for this quest
			itemLoots,			// IDs of NPCs where these items can be found
			itemLootRegions,	// Regions where the NPCs in "itemloots" have to be killed to give player the item
			// Module: Timed Quests
			questTimer,			// The time that quest have to be finished, in seconds
			// Module: Report
			reportUID,			// The UID(s) of NPC(s) where quest have to be reported
			reportID,			// The ID(s) of NPC(s) where quest have to be reported
			// Rewards
			rewardItems,		// Items for Reward for that Quest
			rewardAmounts,		// Amount of every item on "rewardItems"
			rewardXp,			// XP reward for this quest (For XP and Level Shards)
			// Functions of each quest
			functionAssign,		// Function that have to be called on Assignment
			functionResign,		// Function that have to be called on Resign
			functionFail,		// Function that have to be called when quest fails
			functionComplete;	// Function that have to be called when player complete the quest
};

class cQuests
{
	OBJECTDEF( cQuests )
private:
	QList<stQuest> quests;
public:
	// Skill management methods
	void load();
	void unload();
	void reload()
	{
		unload(); load();
	}
	const QString& getQuestName( quint16 quest ) const;
};

typedef Singleton<cQuests> Quests;
