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
#include "scriptmanager.h"
#include "serverconfig.h"
#include "basics.h"
#include <string.h>
#include <qdom.h>

void cBaseDef::processNode( const cElement* node ) {
	if (node->name() == "intproperty") {
		QString name = node->getAttribute("name");
		if (!name.isEmpty()) {
			QString value = node->getAttribute("value");
			if (value.isNull()) {
				value = node->text();
			}
			value = hex2dec(value); // Convert hexadecimal numbers accordingly
			
			bool ok;
			unsigned int intvalue = value.toInt(&ok);

			if (!ok) {
				Console::instance()->log(LOG_WARNING, QString("Basedef %1 has invalid integer property %2.\n").arg(id_).arg(name));
			}

			intproperties.insert(name.lower(), intvalue, true);
		}        
	} else if (node->name() == "strproperty") {
		QString name = node->getAttribute("name");
		if (!name.isEmpty()) {
			QString value = node->getAttribute("value");
			if (value.isNull()) {
				value = node->text();
			}			
			properties.insert(name.lower(), value, true);
		}
	}
	else if ( node->name() == "bindmenu" )
	{
		bindmenu_ = node->text();
	}
	else if ( node->name() == "basescripts" )
	{
		baseScriptList_ = node->text();
		refreshScripts();
	}
	else if ( node->name() == "basescript" )
	{
		if (baseScriptList_.isEmpty()) {
			baseScriptList_.prepend(node->text());
		} else {
			baseScriptList_.prepend(node->text() + ",");
		}
		refreshScripts();
	}
}

void cBaseDef::refreshScripts() {
	if (loaded) {
		QStringList scripts = QStringList::split(",", baseScriptList_);
		QStringList::const_iterator it;
		baseScripts_.clear();
		for (it = scripts.begin(); it != scripts.end(); ++it) {
			cPythonScript *script = ScriptManager::instance()->find((*it).latin1());
			if (script) {
				baseScripts_.append(script);
			}
		}
	}
}

void cBaseDef::reset() {
	loaded = false;
	intproperties.clear();
	properties.clear();
}

cCharBaseDef::cCharBaseDef( const QCString& id )
{
	id_ = id;
	reset();
}

cCharBaseDef::~cCharBaseDef()
{
}

void cCharBaseDef::reset()
{
	cBaseDef::reset();
	basesound_ = 0;
	soundmode_ = 0;
	figurine_ = 0;
	minDamage_ = 0;
	maxDamage_ = 0;
	minTaming_ = 0;
	controlSlots_ = 1;
	criticalHealth_ = 0;
}

void cCharBaseDef::processNode( const cElement* node )
{
	if ( node->name() == "basesound" )
	{
		basesound_ = node->value().toInt();
	}
	else if ( node->name() == "soundmode" )
	{
		soundmode_ = node->value().toInt();
	}
	else if ( node->name() == "figurine" )
	{
		figurine_ = node->value().toInt();
	}
	else if ( node->name() == "mindamage" )
	{
		minDamage_ = node->value().toInt();
	}
	else if ( node->name() == "maxdamage" )
	{
		maxDamage_ = node->value().toInt();
	}
	else if ( node->name() == "mintaming" )
	{
		minTaming_ = node->value().toInt();
	}
	else if ( node->name() == "controlslots" )
	{
		controlSlots_ = node->value().toInt();
	}
	else if ( node->name() == "criticalhealth" )
	{
		criticalHealth_ = node->value().toInt();
	}
	else if ( node->name() == "carve" )
	{
		carve_ = node->text();
	}
	else if ( node->name() == "lootpacks" )
	{
		lootPacks_ = node->text();
	}
	else
	{
		cBaseDef::processNode(node);
	}
}

// Load this definition from the scripts.
void cCharBaseDef::load()
{
	if ( !loaded )
	{
		loaded = true;
		const cElement* element = Definitions::instance()->getDefinition( WPDT_NPC, id_ );

		if ( !element )
		{
			if (!id_.isEmpty()) {
				Console::instance()->log( LOG_WARNING, QString( "Missing npc definition '%1'.\n" ).arg( id_ ) );
			}
			return;
		}

		applyDefinition(element);
	}
}

cCharBaseDef* cCharBaseDefs::get( const QCString& id )
{
	Iterator it = definitions.find( id );

	if ( it == definitions.end() )
	{
		cCharBaseDef* def = new cCharBaseDef( id );
		it = definitions.insert( id, def );
	}

	return it.data();
}

void cCharBaseDefs::loadBodyInfo() {
	// Null the existing one
	memset(bodyinfo, 0, sizeof(bodyinfo));

	QString filename = Config::instance()->getString("General", "Bodyinfo File", "definitions/system/bodyinfo.xml", true);
	QFile file(filename);

	if (!file.open(IO_ReadOnly)) {
		Console::instance()->log(LOG_WARNING, QString("Unable to load body information from %1.\n").arg(filename));
	}

	QDomDocument document;
	document.setContent(&file);

	QDomNode parent = document.namedItem("bodyinfo");
	if (parent.isElement()) {
		for (int i = 0; i < parent.childNodes().count(); ++i) {
			QDomElement element = parent.childNodes().item(i).toElement();

			if (!element.isNull()) {
				QString id = hex2dec(element.attribute("id"));
				bool ok = false;
				
				stBodyInfo bodyinfo;

				// The body id (mandatory)
				bodyinfo.body = id.toUShort(&ok);
				if (!ok) {
					Console::instance()->log(LOG_WARNING, QString("Invalid body id in bodyinfo file: %1.\n").arg(id));
					continue;
				}

				// The offset for sounds this creature is using
				QString basesound = hex2dec(element.attribute("basesound"));
				if (!basesound.isNull()) {
					bodyinfo.basesound = basesound.toUShort(&ok);
					if (!ok) {
						Console::instance()->log(LOG_WARNING, QString("Invalid basesound in bodyinfo file: %1.\n").arg(basesound));
						continue;
					}
				} else {
					bodyinfo.basesound = 0;
				}

				// Load the figurine for shrinking (this is just a display id)
				QString figurine = hex2dec(element.attribute("figurine"));
				if (!figurine.isNull()) {
					bodyinfo.figurine = figurine.toUShort(&ok);
					if (!ok) {
						Console::instance()->log(LOG_WARNING, QString("Invalid figurine in bodyinfo file: %1.\n").arg(figurine));
						continue;
					}
				} else {
					bodyinfo.figurine = 0;
				}

				// Soundmode for skipping non existing sounds
				QString soundmode = hex2dec(element.attribute("soundmode"));
				if (!soundmode.isNull()) {
					bodyinfo.soundmode = soundmode.toUShort(&ok);
					if (!ok) {
						Console::instance()->log(LOG_WARNING, QString("Invalid soundmode in bodyinfo file: %1.\n").arg(soundmode));
						continue;
					}
				} else {
					bodyinfo.soundmode = 0;
				}

				// Flags for this creature (noblink, canfly, nocorpse)
				QString flags = hex2dec(element.attribute("flags"));
				if (!flags.isNull()) {
					bodyinfo.flags = flags.toUShort(&ok);
					if (!ok) {
						Console::instance()->log(LOG_WARNING, QString("Invalid flags in bodyinfo file: %1.\n").arg(flags));
						continue;
					}
				} else {
					bodyinfo.flags = 0;
				}

				// The offset for sounds this creature is using
				QString bodytype = hex2dec(element.attribute("type"));
				if (!bodytype.isNull()) {
					bodyinfo.type = bodytype.toUShort(&ok);
					if (!ok) {
						Console::instance()->log(LOG_WARNING, QString("Invalid bodytype in bodyinfo file: %1.\n").arg(bodytype));
						continue;
					}
				} else {
					bodyinfo.type = 0;
				}

				if (bodyinfo.body < 0x400) {
					this->bodyinfo[bodyinfo.body] = bodyinfo;
				}
			}
		}
	}

	document.clear();
}

cCharBaseDefs::cCharBaseDefs()
{
}

cCharBaseDefs::~cCharBaseDefs()
{
	Iterator it;
	for ( it = definitions.begin(); it != definitions.end(); ++it )
	{
		delete it.data();
	}
	definitions.clear();
}

void cCharBaseDefs::reset()
{
	Iterator it;
	for ( it = definitions.begin(); it != definitions.end(); ++it )
	{
		it.data()->reset();
	}
}

void cCharBaseDefs::refreshScripts()
{
	Iterator it;
	for ( it = definitions.begin(); it != definitions.end(); ++it )
	{
		it.data()->refreshScripts();
	}
}


/*
	cItemBaseDef and cItemBaseDefs
*/
cItemBaseDef::cItemBaseDef( const QCString& id )
{
	id_ = id;
	reset();
}

cItemBaseDef::~cItemBaseDef()
{
}

void cItemBaseDef::reset()
{
	cBaseDef::reset();
	weight_ = 0.0f;
	decaydelay_ = 0;
	sellprice_ = 0;
	buyprice_ = 0;
	type_ = 0;
	lightsource_ = 0;
	flags_ = 0;
}

void cItemBaseDef::processNode( const cElement* node )
{
	if ( node->name() == "weight" )
	{
		weight_ = node->text().toFloat();
	}
	else if ( node->name() == "buyprice" )
	{
		buyprice_ = node->value().toUInt();
	}
	else if ( node->name() == "sellprice" )
	{
		sellprice_ = node->value().toUInt();
	}
	else if ( node->name() == "type" )
	{
		type_ = node->value().toUShort();
	}
	else if ( node->name() == "lightsource" )
	{
		lightsource_ = node->value().toUShort();
	}
	else if ( node->name() == "decaydelay" )
	{
		decaydelay_ = node->value().toUInt();
	}
	else if ( node->name() == "watersource" )
	{
		setWaterSource( node->value().toUInt() != 0 );
	}
	else 
	{
		cBaseDef::processNode(node);
	}
}

// Load this definition from the scripts.
void cItemBaseDef::load()
{
	if ( !loaded )
	{
		loaded = true;
		const cElement* element = Definitions::instance()->getDefinition( WPDT_ITEM, id_ );

		if ( !element )
		{
			if (!id_.isEmpty()) {
				Console::instance()->log( LOG_WARNING, QString( "Missing item definition '%1'.\n" ).arg( id_ ) );
			}
			return;
		}

		applyDefinition( element );
	}
}

cItemBaseDef* cItemBaseDefs::get( const QCString& id )
{
	Iterator it = definitions.find( id );

	if ( it == definitions.end() )
	{
		cItemBaseDef* def = new cItemBaseDef( id );
		it = definitions.insert( id, def );
	}

	return it.data();
}

cItemBaseDefs::cItemBaseDefs()
{
}

cItemBaseDefs::~cItemBaseDefs()
{
	Iterator it;
	for ( it = definitions.begin(); it != definitions.end(); ++it )
	{
		delete it.data();
	}
	definitions.clear();
}

void cItemBaseDefs::reset()
{
	Iterator it;
	for ( it = definitions.begin(); it != definitions.end(); ++it )
	{
		it.data()->reset();
	}
}

void cItemBaseDefs::refreshScripts()
{
	Iterator it;
	for ( it = definitions.begin(); it != definitions.end(); ++it )
	{
		it.data()->refreshScripts();
	}
}
