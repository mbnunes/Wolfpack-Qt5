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

#include "console.h"
#include "log.h"
#include "basedef.h"
#include "definitions.h"
#include "basics.h"
#include <string.h>

cCharBaseDef::cCharBaseDef(const QCString &id) {
	id_ = id;
	reset();
}

cCharBaseDef::~cCharBaseDef() {
}

void cCharBaseDef::reset() {
	loaded = false;
	basesound_ = 0;
	soundmode_ = 0;
	flags_ = 0;
	figurine_ = 0;
	minDamage_ = 0;
	maxDamage_ = 0;
	minTaming_ = 0;
	controlSlots_ = 1;
	criticalHealth_ = 0;
}

void cCharBaseDef::processNode(const cElement *node) {
	if (node->name() == "basesound") {
		basesound_ = node->value().toInt();
	} else if (node->name() == "soundmode") {
		soundmode_ = node->value().toInt();
	} else if (node->name() == "figurine") {
		figurine_ = node->value().toInt();
	} else if (node->name() == "mindamage") {
		minDamage_ = node->value().toInt();
	} else if (node->name() == "maxdamage") {
		maxDamage_ = node->value().toInt();
	} else if (node->name() == "mintaming") {
		minTaming_ = node->value().toInt();
	} else if (node->name() == "controlslots") {
		controlSlots_ = node->value().toInt();
	} else if (node->name() == "criticalhealth") {
		criticalHealth_ = node->value().toInt();
	} else if (node->name() == "carve") {
		carve_ = node->text();
	} else if (node->name() == "lootpacks") {
		lootPacks_ = node->text();
	} else if (node->name() == "canfly") {
		flags_ |= 0x01;
	} else if (node->name() == "antiblink") {
		flags_ |= 0x02;
	} else if (node->name() == "nocorpse") {
		flags_ |= 0x04;
	}
}

// Load this definition from the scripts.
void cCharBaseDef::load() {
	if (!loaded) {
		loaded = true;
		const cElement *element = Definitions::instance()->getDefinition(WPDT_NPC, id_);

		if (!element) {
			Console::instance()->log(LOG_WARNING, QString("Missing npc definition '%1'.\n").arg(id_));
			return;
		}

		applyDefinition(element);
	}
}

cCharBaseDef *cCharBaseDefs::get(const QCString &id) {
	Iterator it = definitions.find(id);

	if (it == definitions.end()) {
		cCharBaseDef *def = new cCharBaseDef(id);
		it = definitions.insert(id, def);
	}

	return it.data();
}

cCharBaseDefs::cCharBaseDefs() {
}

cCharBaseDefs::~cCharBaseDefs() {
	Iterator it;
	for (it = definitions.begin(); it != definitions.end(); ++it) {
		delete it.data();
	}
	definitions.clear();
}

void cCharBaseDefs::reset() {
	Iterator it;
	for (it = definitions.begin(); it != definitions.end(); ++it) {
		it.data()->reset();
	}
}
