/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2005 by holders identified in AUTHORS.txt
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

#include "defines.h"

#include "typedefs.h"
#include "targetrequests.h"
#include "objectdef.h"

// Library Includes
#include <qmap.h>
#include <qstringlist.h>
#include <qvaluevector.h>

// Forward Declaration
class cUOSocket;

struct stQuest
{
	QString name, description, requiredquests, npctargets, npcamounts, rewards, rewardamounts;
};

class cQuests
{
	OBJECTDEF( cQuests )
private:
	QValueVector<stQuest> quests;
public:
	// Skill management methods
	void load();
	void unload();
	void reload()
	{
		unload(); load();
	}
	const QString& getQuestName( Q_UINT16 quest ) const;
};

typedef Singleton<cQuests> Quests;
