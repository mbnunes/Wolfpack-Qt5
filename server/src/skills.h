/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
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
 * Wolfpack Homepage: http://wpdev.sf.net/
 */

#ifndef __SKILLS_H
#define __SKILLS_H

#include "defines.h"

#include "typedefs.h"
#include "targetrequests.h"

// Library Includes
#include <qmap.h>
#include <qstringlist.h>
#include <qvaluevector.h>

// Forward Declaration
class cUOSocket;

struct stSkill {
	QString name, defname, title;
};

class cSkills {
private:
	QStringList skillRanks;
	QValueVector<stSkill> skills;
public:
	// Skill management methods
	void load();
	void unload();
	void reload() { unload(); load(); }
	QString getSkillTitle( P_CHAR pChar ) const;
	const QString &getSkillName( UINT16 skill ) const;
	const QString &getSkillDef( UINT16 skill ) const;
	INT16 findSkillByDef( const QString &defname ) const; // -1 = Not Found

	// Skill Usage methods
	void Meditation(cUOSocket* s);

	static void RandomSteal( cUOSocket*, SERIAL );
	void Track(P_CHAR pc_i);
	void SkillUse( cUOSocket*, UINT16 );
	void Snooping(P_PLAYER, P_ITEM);
};

typedef SingletonHolder<cSkills> Skills;

#endif
