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
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
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

void cBaseDef::processNode( const cElement* node )
{
	if ( node->name() == "intproperty" )
	{
		QString name = node->getAttribute( "name" );
		if ( !name.isEmpty() )
		{
			QString value = node->getAttribute( "value" );
			if ( value.isNull() )
			{
				value = node->text();
			}
			value = hex2dec( value ); // Convert hexadecimal numbers accordingly

			bool ok;
			unsigned int intvalue = value.toInt( &ok );

			if ( !ok )
			{
				Console::instance()->log( LOG_WARNING, tr( "Basedef %1 has invalid integer property %2.\n" ).arg( id_ ).arg( name ) );
			}

			intproperties.insert( name.lower(), intvalue, true );
		}
	}
	else if ( node->name() == "strproperty" )
	{
		QString name = node->getAttribute( "name" );
		if ( !name.isEmpty() )
		{
			QString value = node->getAttribute( "value" );
			if ( value.isNull() )
			{
				value = node->text();
			}
			properties.insert( name.lower(), value, true );
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
		if ( baseScriptList_.isEmpty() )
		{
			baseScriptList_.prepend( node->text() );
		}
		else
		{
			baseScriptList_.prepend( node->text() + "," );
		}
		refreshScripts();
	}
}

void cBaseDef::refreshScripts()
{
	if ( loaded )
	{
		QStringList scripts = QStringList::split( ",", baseScriptList_ );
		QStringList::const_iterator it;
		baseScripts_.clear();
		for ( it = scripts.begin(); it != scripts.end(); ++it )
		{
			cPythonScript *script = ScriptManager::instance()->find( ( *it ).latin1() );
			if ( script )
			{
				baseScripts_.append( script );
			}
		}
	}
}

void cBaseDef::reset()
{
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
	figurine_ = 0;
	minDamage_ = 0;
	maxDamage_ = 0;
	minTaming_ = 0;
	controlSlots_ = 1;
	criticalHealth_ = 0;
	wanderSpeed_ = 400;
	actionSpeed_ = 200;
	// Initialize the overrides to not override...
	attackSound_.clear();
	idleSound_.clear();
	hitSound_.clear();
	gethitSound_.clear();
	deathSound_.clear();
}

void cCharBaseDef::processNode( const cElement* node )
{
	if ( node->name() == "basesound" )
	{
		basesound_ = node->value().toInt();
	}
	else if ( node->name() == "attacksound" )
	{
		QStringList parts = QStringList::split( ",", node->text() );
		for ( int i = 0; i < parts.size(); ++i )
		{
			unsigned short value = hex2dec( hex2dec( parts[i] ) ).toUShort();
			attackSound_.append( value );
		}
	}
	else if ( node->name() == "idlesound" )
	{
		QStringList parts = QStringList::split( ",", node->text() );
		for ( int i = 0; i < parts.size(); ++i )
		{
			unsigned short value = hex2dec( hex2dec( parts[i] ) ).toUShort();
			idleSound_.append( value );
		}
	}
	else if ( node->name() == "hitsound" )
	{
		QStringList parts = QStringList::split( ",", node->text() );
		for ( int i = 0; i < parts.size(); ++i )
		{
			unsigned short value = hex2dec( hex2dec( parts[i] ) ).toUShort();
			hitSound_.append( value );
		}
	}
	else if ( node->name() == "gethitsound" )
	{
		QStringList parts = QStringList::split( ",", node->text() );
		for ( int i = 0; i < parts.size(); ++i )
		{
			unsigned short value = hex2dec( hex2dec( parts[i] ) ).toUShort();
			gethitSound_.append( value );
		}
	}
	else if ( node->name() == "deathsound" )
	{
		QStringList parts = QStringList::split( ",", node->text() );
		for ( int i = 0; i < parts.size(); ++i )
		{
			unsigned short value = hex2dec( hex2dec( parts[i] ) ).toUShort();
			deathSound_.append( value );
		}
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
	else if ( node->name() == "inherit" )
	{
		QString inheritID;
		if ( node->hasAttribute( "id" ) )
			inheritID = node ->getAttribute( "id" );
		else
			inheritID = node->value();

		const cElement* element = Definitions::instance()->getDefinition( WPDT_NPC, inheritID );
		if ( element )
			applyDefinition( element );
	}
	else if ( node->name() == "speed" && node->hasAttribute( "wander" ) && node->hasAttribute( "action" ) )
	{
		bool ok1, ok2;

		// Convert to unsigned ints
		unsigned int wanderSpeed = node->getAttribute( "wander" ).toUInt( &ok1 );
		unsigned int actionSpeed = node->getAttribute( "action" ).toUInt( &ok2 );

		if ( !ok1 || !ok2 )
		{
			Console::instance()->log( LOG_WARNING, tr( "Base definition '%1' has invalid speed tag.\n" ).arg( id_ ) );
		}
		else
		{
			actionSpeed_ = actionSpeed;
			wanderSpeed_ = wanderSpeed;
		}
	}
	else
	{
		cBaseDef::processNode( node );
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
			if ( !id_.isEmpty() )
			{
				Console::instance()->log( LOG_WARNING, tr( "Missing npc definition '%1'.\n" ).arg( id_ ) );
			}
			return;
		}

		applyDefinition( element );
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

void cCharBaseDefs::loadBodyInfo()
{
	// Null the existing one
	memset( bodyinfo, 0, sizeof( bodyinfo ) );

	// Manually initialize the sounds to -1...
	for ( int i = 0; i < 0x400; ++i )
	{
		bodyinfo[i].attackSound = -1;
		bodyinfo[i].deathSound = -1;
		bodyinfo[i].gethitSound = -1;
		bodyinfo[i].hitSound = -1;
		bodyinfo[i].idleSound = -1;
	}

	QString filename = Config::instance()->getString( "General", "Bodyinfo File", "definitions/system/bodyinfo.xml", true );
	QFile file( filename );

	if ( !file.open( IO_ReadOnly ) )
	{
		Console::instance()->log( LOG_WARNING, tr( "Unable to load body information from %1.\n" ).arg( filename ) );
	}

	QDomDocument document;
	document.setContent( &file );

	QDomNode parent = document.namedItem( "bodyinfo" );
	if ( parent.isElement() )
	{
		for ( unsigned int i = 0; i < parent.childNodes().count(); ++i )
		{
			QDomElement element = parent.childNodes().item( i ).toElement();

			if ( !element.isNull() )
			{
				QString id = hex2dec( element.attribute( "id" ) );
				bool ok = false;

				stBodyInfo bodyinfo;

				// The body id (mandatory)
				bodyinfo.body = id.toUShort( &ok );
				if ( !ok )
				{
					Console::instance()->log( LOG_WARNING, tr( "Invalid body id in bodyinfo file: %1.\n" ).arg( id ) );
					continue;
				}

				// The offset for sounds this creature is using
				QString basesound = hex2dec( element.attribute( "basesound" ) );
				if ( !basesound.isNull() )
				{
					bodyinfo.basesound = basesound.toUShort( &ok );
					if ( !ok )
					{
						Console::instance()->log( LOG_WARNING, tr( "Invalid basesound in bodyinfo file: %1.\n" ).arg( basesound ) );
						continue;
					}
				}
				else
				{
					bodyinfo.basesound = 0;
				}

				// Load the figurine for shrinking (this is just a display id)
				QString figurine = hex2dec( element.attribute( "figurine" ) );
				if ( !figurine.isNull() )
				{
					bodyinfo.figurine = figurine.toUShort( &ok );
					if ( !ok )
					{
						Console::instance()->log( LOG_WARNING, tr( "Invalid figurine in bodyinfo file: %1.\n" ).arg( figurine ) );
						continue;
					}
				}
				else
				{
					bodyinfo.figurine = 0;
				}

				// Load the override sounds
				QString attacksound = hex2dec( element.attribute( "attacksound" ) );
				if ( !attacksound.isNull() )
				{
					bodyinfo.attackSound = attacksound.toUShort( &ok );
					if ( !ok )
					{
						Console::instance()->log( LOG_WARNING, tr( "Invalid attacksound in bodyinfo file: %1.\n" ).arg( attacksound ) );
						continue;
					}
				}
				else
				{
					bodyinfo.attackSound = -1;
				}

				QString idlesound = hex2dec( element.attribute( "idlesound" ) );
				if ( !idlesound.isNull() )
				{
					bodyinfo.idleSound = idlesound.toUShort( &ok );
					if ( !ok )
					{
						Console::instance()->log( LOG_WARNING, tr( "Invalid idlesound in bodyinfo file: %1.\n" ).arg( idlesound ) );
						continue;
					}
				}
				else
				{
					bodyinfo.idleSound = -1;
				}

				QString hitsound = hex2dec( element.attribute( "hitsound" ) );
				if ( !hitsound.isNull() )
				{
					bodyinfo.hitSound = hitsound.toUShort( &ok );
					if ( !ok )
					{
						Console::instance()->log( LOG_WARNING, tr( "Invalid hitsound in bodyinfo file: %1.\n" ).arg( hitsound ) );
						continue;
					}
				}
				else
				{
					bodyinfo.hitSound = -1;
				}

				QString gethitsound = hex2dec( element.attribute( "gethitsound" ) );
				if ( !gethitsound.isNull() )
				{
					bodyinfo.gethitSound = gethitsound.toUShort( &ok );
					if ( !ok )
					{
						Console::instance()->log( LOG_WARNING, tr( "Invalid gethitsound in bodyinfo file: %1.\n" ).arg( gethitsound ) );
						continue;
					}
				}
				else
				{
					bodyinfo.gethitSound = -1;
				}

				QString deathsound = hex2dec( element.attribute( "deathsound" ) );
				if ( !deathsound.isNull() )
				{
					bodyinfo.deathSound = deathsound.toUShort( &ok );
					if ( !ok )
					{
						Console::instance()->log( LOG_WARNING, tr( "Invalid deathsound in bodyinfo file: %1.\n" ).arg( deathsound ) );
						continue;
					}
				}
				else
				{
					bodyinfo.deathSound = -1;
				}

				// Load the mount item for mounting
				QString mountid = hex2dec( element.attribute( "mountid" ) );
				if ( !mountid.isNull() )
				{
					bodyinfo.mountid = mountid.toUShort( &ok );
					if ( !ok )
					{
						Console::instance()->log( LOG_WARNING, tr( "Invalid mountid in bodyinfo file: %1.\n" ).arg( mountid ) );
						continue;
					}
				}
				else
				{
					bodyinfo.mountid = 0;
				}

				// Flags for this creature (noblink, canfly, nocorpse)
				QString flags = hex2dec( element.attribute( "flags" ) );
				if ( !flags.isNull() )
				{
					bodyinfo.flags = flags.toUShort( &ok );
					if ( !ok )
					{
						Console::instance()->log( LOG_WARNING, tr( "Invalid flags in bodyinfo file: %1.\n" ).arg( flags ) );
						continue;
					}
				}
				else
				{
					bodyinfo.flags = 0;
				}

				// The offset for sounds this creature is using
				QString bodytype = hex2dec( element.attribute( "type" ) );
				if ( !bodytype.isNull() )
				{
					bodyinfo.type = bodytype.toUShort( &ok );
					if ( !ok )
					{
						Console::instance()->log( LOG_WARNING, tr( "Invalid bodytype in bodyinfo file: %1.\n" ).arg( bodytype ) );
						continue;
					}
				}
				else
				{
					bodyinfo.type = 0;
				}

				if ( bodyinfo.body < 0x400 )
				{
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
	definitionType = WPDT_ITEM;
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
	else if ( node->name() == "inherit" )
	{
		QString inheritID;
		if ( node->hasAttribute( "id" ) )
			inheritID = node ->getAttribute( "id" );
		else
			inheritID = node->value();

		const cElement* element = Definitions::instance()->getDefinition( definitionType, inheritID );
		if ( element )
			applyDefinition( element );
	}
	else
	{
		cBaseDef::processNode( node );
	}
}

// Load this definition from the scripts.
void cItemBaseDef::load()
{
	if ( !loaded )
	{
		loaded = true;
		const cElement* element = Definitions::instance()->getDefinition( definitionType, id_ );

		if ( !element )
		{
			if ( !id_.isEmpty() )
			{
				Console::instance()->log( LOG_WARNING, tr( "Missing item definition '%1'.\n" ).arg( id_ ) );
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
		// Is this a multi base definition?
		if ( Definitions::instance()->getDefinition( WPDT_MULTI, id ) )
		{
			cMultiBaseDef* def = new cMultiBaseDef( id );
			it = definitions.insert( id, def );
		}
		else
		{
			cItemBaseDef* def = new cItemBaseDef( id );
			it = definitions.insert( id, def );
		}
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

// Python Interface
const char* cBaseDef::className() const
{
	return "basedef";
}

bool cBaseDef::implements( const QString& name ) const
{
	if ( name == "basedef" )
	{
		return true;
	}
	else
	{
		return cPythonScriptable::implements( name );
	}
}

/*
	\object basedef
	\description This object type reprsents the base definitions of items and characters.
*/
struct wpBasedef
{
	PyObject_HEAD;
	cBaseDef* basedef;
};

static PyObject* wpBasedef_getAttr( wpBasedef* self, char* name );
static int wpBasedef_compare( PyObject* a, PyObject* b );

PyTypeObject wpBasedefType =
{
PyObject_HEAD_INIT( NULL )
0,
"basedef",
sizeof( wpBasedefType ),
0,
wpDealloc,
0,
( getattrfunc ) wpBasedef_getAttr,
0,
wpBasedef_compare,
0,
};

/*
	\method basedef.getintproperty
	\description Get an integer property from this definition.
	\param name The name of the property. This name is not case sensitive.
	\param default The default value that is returned if this property doesnt
	exist. Defaults to 0.
	\return The property value or the given default value.
*/
PyObject* wpBasedef_getintproperty( wpBasedef* self, PyObject* args )
{
	unsigned int def = 0;
	PyObject *pyname;

	if ( !PyArg_ParseTuple( args, "O|i:basedef.getintproperty(name, def)", &pyname, &def ) )
	{
		return 0;
	}

	QString name = Python2QString( pyname );
	return PyInt_FromLong( self->basedef->getIntProperty( name, def ) );
}

/*
	\method basedef.getstrproperty
	\description Get a string property from this definition.
	\param name The name of the property. This name is not case sensitive.
	\param default The default value that is returned if this property doesnt
	exist. Defaults to an empty string.
	\return The property value or the given default value.
*/
static PyObject* wpBasedef_getstrproperty( wpBasedef* self, PyObject* args )
{
	PyObject *pydef = 0;
	PyObject *pyname;

	if ( !PyArg_ParseTuple( args, "O|O:basedef.getstrproperty(name, def)", &pyname, &pydef ) )
	{
		return 0;
	}

	QString name = Python2QString( pyname );
	QString def = Python2QString( pydef );

	return QString2Python( self->basedef->getStrProperty( name, def ) );
}

/*
	\method basedef.hasstrproperty
	\description Checks if this definition has a string property with the given name.
	\param name The name of the property. This name is not case sensitive.
	\return True if the definition has the property, False otherwise.
*/
static PyObject* wpBasedef_hasstrproperty( wpBasedef* self, PyObject* args )
{
	PyObject *pyname;

	if ( !PyArg_ParseTuple( args, "O:char.hasstrproperty(name)", &pyname ) )
	{
		return 0;
	}

	QString name = Python2QString( pyname );

	if ( self->basedef->hasStrProperty( name ) )
	{
		Py_RETURN_TRUE;
	}
	else
	{
		Py_RETURN_FALSE;
	}
}

/*
	\method basedef.hasintproperty
	\description Checks if this definition has an integer property with the given name.
	\param name The name of the property. This name is not case sensitive.
	\return True if the definition has the property, False otherwise.
*/
static PyObject* wpBasedef_hasintproperty( wpBasedef* self, PyObject* args )
{
	PyObject *pyname;

	if ( !PyArg_ParseTuple( args, "O:basedef.hasintproperty(name)", &pyname ) )
	{
		return 0;
	}

	QString name = Python2QString( pyname );

	if ( self->basedef->hasIntProperty( name ) )
	{
		Py_RETURN_TRUE;
	}
	else
	{
		Py_RETURN_FALSE;
	}
}

static PyMethodDef wpBasedefMethods[] = {
{"getintproperty", ( getattrofunc ) wpBasedef_getintproperty, METH_VARARGS, 0},
{"getstrproperty", ( getattrofunc ) wpBasedef_getstrproperty, METH_VARARGS, 0},
{"hasintproperty", ( getattrofunc ) wpBasedef_hasintproperty, METH_VARARGS, 0},
{"hasstrproperty", ( getattrofunc ) wpBasedef_hasstrproperty, METH_VARARGS, 0},
{0, 0, 0, 0}
};

PyObject* cBaseDef::getPyObject()
{
	wpBasedef * returnVal = PyObject_New( wpBasedef, &wpBasedefType );
	returnVal->basedef = this;
	return ( PyObject * ) returnVal;
}

static PyObject* wpBasedef_getAttr( wpBasedef* self, char* name )
{
	PyObject *result = self->basedef->getProperty( name );

	if ( result )
	{
		return result;
	}

	return Py_FindMethod( wpBasedefMethods, ( PyObject * ) self, name );
}

static int wpBasedef_compare( PyObject* a, PyObject* b )
{
	if ( a->ob_type != &wpBasedefType || b->ob_type != &wpBasedefType )
		return -1;

	return !( ( ( wpBasedef * ) a )->basedef == ( ( wpBasedef * ) b )->basedef );
}

PyObject* cBaseDef::getProperty( const QString& name )
{
	PY_PROPERTY( "id", id() ) // \rproperty basedef.id The definition id of this base definition.
	PY_PROPERTY( "basescripts", baseScriptList() ) // \rproperty basedef.basescripts The names of all basescripts assigned to this base definition.
	PY_PROPERTY( "bindmenu", bindmenu() ) // \rproperty basedef.bindmenu The id of the context menu assigned to this base definition.
	return cPythonScriptable::getProperty( name );
}

PyObject* cCharBaseDef::getProperty( const QString& name )
{
	PY_PROPERTY( "controlslots", controlSlots() ) // \rproperty basedef.controlslots The amount of controlslots consumed by this npc.
	PY_PROPERTY( "criticalhealth", criticalHealth() ) // \rproperty basedef.criticalhealth The percentage of health this NPC will start fleeing at.
	PY_PROPERTY( "mindamage", minDamage() ) // \rproperty basedef.mindamage The mindamage for this npc.
	PY_PROPERTY( "mintaming", minTaming() ) // \rproperty basedef.mintaming The minimum taming skill required for this npc.
	PY_PROPERTY( "lootpacks", lootPacks() ) // \rproperty basedef.lootpacks The lootpacks for this npc.
	return cBaseDef::getProperty( name );
}

PyObject* cItemBaseDef::getProperty( const QString& name )
{
	PY_PROPERTY( "decaydelay", decaydelay() ) // \rproperty basedef.decaydelay The delay until this item will decay when its dropped to ground.
	PY_PROPERTY( "weight", weight() ) // \rproperty basedef.weight The weight of this item.
	PY_PROPERTY( "sellprice", sellprice() ) // \rproperty basedef.sellprice The sellprice of this item.
	PY_PROPERTY( "buyprice", buyprice() ) // \rproperty basedef.buyprice The buyprice for this item.
	PY_PROPERTY( "type", type() ) // \rproperty basedef.type The type of this item.
	PY_PROPERTY( "lightsource", lightsource() ) // \rproperty.lightsource The lightmap id for this item.
	PY_PROPERTY( "watersource", isWaterSource() ) // \rproperty.watersource Indicates whether this item is a source of water.
	return cBaseDef::getProperty( name );
}

cMultiBaseDef::cMultiBaseDef( const QCString& id ) : cItemBaseDef( id )
{
	definitionType = WPDT_MULTI;
}
