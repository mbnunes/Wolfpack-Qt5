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

#include "pythonscript.h"

#include "muls/maps.h"
#include "network/network.h"
#include "console.h"

// Library Includes
#include <qfile.h>
#include <qglobal.h>

// Extension includes
#include "python/utilities.h"
#include "python/target.h"

// Keep this in Synch with the Enum on the header file
static char *eventNames[] =
{
	/*
		\event onUse
		\param player The player who used the item.
		\param item The item that was used.
		\return Return 1 if your function should handle the event. If you return 0,
		the server will try to process the event internally or call other scripts.
		\condition Triggered when a player doubleclicks an item.
		\notes This event is called for the character first and then for the item.
	*/
	"onUse",

	/*
		\event onSingleClick
		\param item The item that was singleclicked.
		\param viewer The player who singleclicked on the item.
		\return Return 1 if your function handles the event. If you return 0,
		the server will try to process the event internally or call other scripts.
		\condition Triggered when the client requests the name of an immovable item.
		\notes This event is only called for the item clicked on and not for the character
		who clicked.
	*/
	"onSingleClick",

	/*
		\event onCollide
		\param char The character who stepped on the item.
		\param item The item that was stepped on.
		\return Return 1 if your function handles the event. If you return 0, the core will
		call the remaining scipts in the chain.
		\condition Triggered when a character walks onto the item. This event is only triggered
		if the character was not previously standing on the item.
		\notes This event is only called for the item and not for the character.
	*/
	"onCollide",

	/*
		\event onWalk
		\param char The character who requested to walk.
		\param dir The direction the character tries to walk to.
		\param sequence The packet sequence number.
		\return Return 1 if your function handles the event. If you return 0, the core will
		call the remaining scipts in the chain and try to handle the event itself.
		\condition Triggered when any character tries to walk. The event is called before
		any other checks are made.
		\note Please be careful when using this event. It's called very often and therefor
		could slow your server down if used extensively. Please note that you are
		responsible for denying and accepting walk requests if you return 1 in this event
		handler.
	*/
	"onWalk",

	/*
		\event onCreate
		\param object The object that was just created.
		\param id The definition id used to create the object.
		\condition Triggered when either a character or an item is created from a
		XML definition. Objects that are not created from a XML definition don't
		trigger this event. They wouldn't have a script to call anyway.
	*/
	"onCreate",

	/*
		\event onTalk
		\param player The player who talked.
		\param type The speech type. Valid values are:
		0x00 - Regular.
		0x01 - Broadcast.
		0x02 - Emote.
		0x08 - Whisper.
		0x09 - Yell.
		\param color The color or the text sent by the client.
		\param font The font for the speech. The default is 3.
		\param text The text of the speech request.
		\param lang The three letter code of the used language.
		\return Return 1 if you want to ignore the speech request or handle it otherwise.
		\condition Triggered when a player talks ingame. This event is not triggered for npcs.
	*/
	"onTalk",

	/*
		\event onWarModeToggle
		\param player The player who changed his warmode status.
		\param mode The new warmode status. Possible values are:
		0 - Peace
		1 - War
		\condition Triggered when a player changes his warmode status.
	*/
	"onWarModeToggle",

	/*
		\event onLogin
		\param player The player who logged in.
		\condition Triggered when a player enters the world.
	*/
	"onLogin",

	/*
		\event onLogout
		\param player The player who disconnected.
		\condition Triggered when a player leaves the world.
	*/
	"onLogout",

	/*
		\event onHelp
		\param player The player who pressed the button.
		\condition Triggered when a player pressed the help button on his paperdoll.
		\return Return 1 to override the internal server behaviour.
	*/
	"onHelp",

	/*
		\event onChat
		\param player The player who pressed the button.
		\condition Triggered when a player pressed the chat button on his paperdoll.
		\return Return 1 to override the internal server behaviour.
	*/
	"onChat",

	/*
		\event onSkillUse
		\param player The player who used the skill.
		\param skill The id of the skill that was used.
		\condition Triggered when a player tries to actively use a skill by selecting
		its button on the skill gump or activating a macro.
		\return Return 1 to override the internal server behaviour.
	*/
	"onSkillUse",

	/*
		\event onSkillGain
		\param player The player who has a chance to gain.
		\param skill The skill id that was used.
		\param min The lower difficulty range.
		\param max The upper difficulty range.
		\param success If the try to use the skill was successful.
		\condition Triggered when a player used any skill (passive and active)
		and has a chance to gain in that skill.
		\notes Please note that the server does not determine whether the
		skillgain was successful or not.
		\return Return 1 to override the remaining scripts in the script chain.
	*/
	"onSkillGain",

	/*
		\event onShowPaperdoll
		\param char The character whose paperdoll is requested.
		\param player The player who is requesting the paperdoll.
		\condition Triggered when a player requests the paperdoll of
		another character by doubleclicking on him.
		\return Return 1 to override the internal server behaviour.
	*/
	"onShowPaperdoll",

	/*
		\event onShowSkillGump
		\param player The player who requested a list of his skills.
		\condition Triggered when the complete list of skills is requested
		by the client.
		\return Return 1 to override the internal server behaviour.
	*/
	"onShowSkillGump",

	/*
		\event onDeath
		\param char The character who died.
		\param source The source of the lethal blow. This may be None.
		\param corpse The corpse of the dead character. This may be None.
		\condition Triggered when a character dies.
		\notes This event is called before the death shroud for players has been
		created.
	*/
	"onDeath",

	/*
		\event onShowPaperdollName
		\param char The character whose paperdoll is being requested.
		\param player The player who is requesting the paperdoll.
		\condition Triggered when the paperdoll for a character is about
		to be shown.
		\return Return the text you want to have shown on the paperdoll or
		otherwise None. Please note that the text may only be 59 characters
		long and may only contain 7-bit ASCII characters.
	*/
	"onShowPaperdollName",

	/*
		\event onContextEntry
		\param player The player who selected the context menu entry.
		\param object The object the context menu was assigned to.
		\param entry The id of the entry that was selected.
		\condition Triggered when a character selects an entry from a
		context menu.
		\notes This event is only called for context menus but neither for the
		object it was attached to nor for the player who selected the entry.
	*/
	"onContextEntry",

	/*
		\event onShowTooltip
		\param player The player who requested the tooltip.
		\param object The object the tooltip was requested for.
		\param tooltip The tooltip that is about to be sent.
		\condition Triggered just before a tooltip is sent for an object.
		\notes Please note that you cannot stop the tooltip from being sent.
		You can only modify the tooltip. This event is only called for the
		object and not for the player.
	*/
	"onShowTooltip",

	/*
		\event onCHLevelChange
		\param player The player changing the level of his house.
		\param level The level the player wants to change to.
		\condition Triggered when a player changes the current
		floor level while modifying his custom house.
	*/
	"onCHLevelChange",

	/*
		\event onSpeech
		\param npc The npc who heard the text.
		\param player The player the text is coming from.
		\param text The text.
		\param keywords A list of numeric keywords. This is passed from the
		client. See speech.mul for the meaning of keywords.
		\condition Triggered when a npc hears text said by a player.
		\return Return 1 if your npc understood what the player said,
		no other npc scripts will be called then.
	*/
	"onSpeech",

	/*
		\event onWearItem
		\param player The player who is trying to equip an item.
		\param char The character who is supposed to wear the item.
		\param item The item.
		\param layer The requested drop layer.
		\condition Triggered when a player tries to equip an item.
		\return Return 1 if the server should reject the equip request and
		should bounce the item back.
	*/
	"onWearItem",

	/*
		\event onEquip
		\param char The character who is equipping the item.
		\param item The item being equipped.
		\param layer The layer the item is being equipped on.
		\condition Triggered when a character equips an item.
	*/
	"onEquip",

	/*
		\event onUnequip
		\param char The previous wearer of the item.
		\param item The item being unequipped.
		\param layer The layer the item was equipped on.
		\condition Triggered when a character takes an item off.
	*/
	"onUnequip",

	/*
		\event onDropOnChar
		\param char The character the item is being dropped on.
		\param item The item.
		\condition Triggered when an item is dropped on another character.
		\return Return 1 to override the internal server behaviour. If you didn't
		delete or move the item and still return 1, the server will automatically
		bounce the item.
		\notes You can find the player who is dropping the item in item.container.
	*/
	"onDropOnChar",

	/*
		\event onDropOnItem
		\param target The item the player is dropping his item on.
		\param item The dropped item.
		\condition Triggered when an item is dropped on another item.
		\return Return 1 to override the internal server behaviour.
		\notes You can find the player who is dropping the item in item.container.
	*/
	"onDropOnItem",

	/*
		\event onDropOnGround
		\param item The item being dropped.
		\param pos The position the item is being dropped to.
		\condition Triggered when an item is dropped to the ground.
		\return Return 1 to override the internal server behaviour.
		\notes You can find the player who is dropping the item in item.container.
	*/
	"onDropOnGround",

	/*
		\event onPickup
		\param player The player trying to pick up an item.
		\param item The requested item.
		\condition Triggered when a player tries to pick up an item.
		\return Return 1 to prevent the item from being picked up.
		\notes The event is called first for the item and then for the
		player.
	*/
	"onPickup",

	/*
		\event onDamage
		\param char The character taking the damage.
		\param type The damage type.
		\param amount The amount of damage.
		\param source The source the damage comes from. May be an item,
		a character or None.
		\condition Triggered when a character takes damage.
		\return Return the new amount of damage as an integer.
		\notes This event is only called for the victim. Please be careful
		not to call the damage method of the character in this function to
		prevent an endless loop.
	*/
	"onDamage",

	/*
		\event onCastSpell
		\param player The player who requested to cast a spell.
		\param spell The id of the spell the player wants to cast.
		\condition Triggered when a player tries to cast a spell using his spellbook.
		\return Return 1 to override the remaining scripts in the scriptchain.
	*/
	"onCastSpell",

	/*
		\event onTrade
		\param player The player requesting to change the trade status.
		\param type The request type.
		\param buttonstate The accept button state.
		\param itemserial The trade container serial.
		\condition Triggered when a player requests to change the trade window state.
	*/
	"onTrade",

	/*
		\event onTradeStart
		\param player The player initiating the trade.
		\param partner The player the item was dropped on.
		\param item The first item.
		\condition Triggered when a player drops an item onto another player.
		\notes This event is only called once for the initiating player.
	*/
	"onTradeStart",

	/*
		\event onDelete
		\param object The object being deleted.
		\condition Called when an item or character is deleted.
	*/
	"onDelete",

	/*
		\event onSwing
		\param attacker The character swinging his weapon at another.
		\param defender The attack target of the attacker.
		\param time The servertime in miliseconds.
		\condition Called when a character swings his weapon at another one.
		\notes This is used to implement the combat system. Please note that this
		event is not called for characters but only called as a global hook.
	*/
	"onSwing",

	/*
		\event onShowStatus
		\param player The player who is requesting his status.
		\param packet The status packet.
		\condition Triggered just before the status packet is sent to a player.
		\notes This is used to implement the combat system. Please note that this
		event is not called for characters but only called as a global hook.
	*/
	"onShowStatus",

	/*
		\event onChangeRegion
		\param char The character changing regions.
		\param oldregion The last region the character was in.
		\param newregion The new region the character is entering.
		\condition Triggered when a character moves and the region he is in changed.
	*/
	"onChangeRegion",

	/*
		\event onAttach
		\param object The object.
		\condition Triggered when a script is attached to an object.
		\notes This is even triggered when the item is loaded from a worldfile.
	*/
	"onAttach",

	/*
		\event onDetach
		\param object The object.
		\condition Triggered when a script is removed from an object.
	*/
	"onDetach",

	/*
		\event onTimeChange
		\param player The player.
		\condition This event is called for every connected client once an ingame hour has elapsed.
	*/
	"onTimeChange",

	/*
		\event onDispel
		\param player The player that is affected.
		\param source The source character of the dispel effect. May be None.
		\param silent Indicates that the effects should be silently dispelled.
			This is a boolean value.
		\param force Is the dispel effect dispelling even undispellable effects?
			This is a boolean value.
		\param dispelid Is the dispel effect limited to one kind of effect?
			This is a string. If it's empty, all effects are affected.
		\param dispelargs A tuple of arguments passed to the dispel function of the effects.
			This may be an empty tuple.
		\condition This event is called before a dispel effect affects the effects on this
		character.
		\return Return 1 to ignore the dispel effect.
	*/
	"onDispel",

	/*
		\event onTelekinesis
		\param char The character trying to use telekinesis on this object.
		\param item The object being used.
		\condition This event is called when someone tries to use telekinesis on this
		item. It is only called for the item.
		\return Return behaviour is the same as for onUse.
	*/
	"onTelekinesis",

	0
};

cPythonScript::cPythonScript() : loaded(false)
{
	codeModule = 0;
	for (unsigned int i = 0; i < EVENT_COUNT; ++i) {
		events[i] = 0;
	}
}

cPythonScript::~cPythonScript()
{
	if ( loaded )
		unload();
}

void cPythonScript::unload( void )
{
	loaded = false;

	// Free Cached Events
	for (unsigned int i = 0; i < EVENT_COUNT; ++i) {
		if (events[i]) {
			Py_XDECREF(events[i]);
			events[i] = 0;
		}
	}

	callEventHandler("onUnload");

	Py_XDECREF(codeModule);
	codeModule = 0;
}

// Find our module name
bool cPythonScript::load(const QString &name)
{
	if (name.isEmpty()) {
		return false;
	}

	setName(name);

	codeModule = PyImport_ImportModule(const_cast<char*>(name.latin1()));

	if(!codeModule) {
		reportPythonError(name);
		return false;
	}

	// Call the onLoad event
	callEventHandler("onLoad", 0, true);

	if (PyErr_Occurred()) {
		reportPythonError(name_);
	}

	// Cache Event Functions
	for (unsigned int i = 0; i < EVENT_COUNT; ++i) {
		if (PyObject_HasAttrString(codeModule, eventNames[i])) {
			events[i] = PyObject_GetAttrString(codeModule, eventNames[i]);

			if (events[i] && !PyCallable_Check(events[i])) {
				Console::instance()->log( LOG_ERROR, QString( "Script %1 has non callable event: %1" ).arg( eventNames[ i ] ) );

				Py_DECREF(events[i]);
				events[i] = 0;
			}
		}
	}

	loaded = true;
	return true;
}

bool cPythonScript::isLoaded() const
{
	return loaded;
}

PyObject* cPythonScript::callEvent( ePythonEvent event, PyObject *args, bool ignoreErrors )
{
	PyObject *result = 0;

	if( event < EVENT_COUNT && events[ event ] )
	{
		result = PyObject_CallObject( events[ event ], args );

		if( !ignoreErrors )
			reportPythonError( name_ );
	}

	return result;
}

PyObject* cPythonScript::callEvent( const QString &name, PyObject *args, bool ignoreErrors )
{
	PyObject *result = 0;

	if( codeModule && !name.isEmpty() && PyObject_HasAttrString( codeModule, const_cast< char* >( name.latin1() ) ) )
	{
		PyObject *event = PyObject_GetAttrString( codeModule, const_cast< char* >( name.latin1() ) );

		if( event && PyCallable_Check( event ) )
		{
			result = PyObject_CallObject( event, args );

			if( !ignoreErrors )
				reportPythonError( name_ );
		}
	}

	return result;
}

bool cPythonScript::canHandleEvent(const QString &event) {
	if (codeModule && !event.isEmpty() && PyObject_HasAttrString(codeModule, const_cast<char*>(event.latin1()))) {
		PyObject *object = PyObject_GetAttrString(codeModule, const_cast<char*>(event.latin1()));

		if (object) {
			if (PyCallable_Check(object)) {
				Py_DECREF(object);
				return true;
			}

			Py_DECREF(object);
		}
	}
	return false;
}

bool cPythonScript::callEventHandler( ePythonEvent event, PyObject *args, bool ignoreErrors )
{
	PyObject *result = callEvent( event, args, ignoreErrors );
	bool handled = false;

	if( result )
	{
		handled = PyObject_IsTrue( result ) == 0 ? false : true;
		Py_DECREF( result );
	}

	return handled;
}

bool cPythonScript::callEventHandler( const QString &name, PyObject* args, bool ignoreErrors )
{
	PyObject* result = callEvent( name, args, ignoreErrors );
	bool handled = false;

	if( result )
	{
		handled = PyObject_IsTrue( result ) == 0 ? false : true;
		Py_DECREF( result );
	}

	return handled;
}

// Standard Handler for Python ScriptChains assigned to objects
bool cPythonScript::callChainedEventHandler( ePythonEvent event, cPythonScript** chain, PyObject* args )
{
	bool handled = false;

	if( chain )
	{
		// Measure
		unsigned int count = reinterpret_cast< unsigned int >( chain[0] );
		cPythonScript **copy = new cPythonScript*[ count ];
		for( unsigned int j = 0; j < count; ++j )
			copy[j] = chain[j+1];

		// Find a valid handler function
		for( unsigned int i = 0; i < count; ++i )
		{
			PyObject *result = copy[i]->callEvent(event, args);

			if( result )
			{
				if( PyObject_IsTrue( result ) )
				{
					handled = true;
					Py_DECREF( result );
					break;
				}

				Py_DECREF( result );
			}
		}

		delete [] copy;
	}

	return handled;
}

PyObject *cPythonScript::callChainedEvent( ePythonEvent event, cPythonScript **chain, PyObject *args )
{
	PyObject *result = 0;

	if( chain )
	{
		// Measure
		unsigned int count = reinterpret_cast< unsigned int >( chain[0] );
		cPythonScript **copy = new cPythonScript*[ count ];
		for( unsigned int j = 0; j < count; ++j )
			copy[j] = chain[j+1];

		// Find a valid handler function
		for( unsigned int i = 0; i < count; ++i )
		{
			result = copy[i]->callEvent( event, args );

			if( result )
				break;
		}

		delete [] copy;
	}

	return result;
}

bool cPythonScript::canChainHandleEvent( ePythonEvent event, cPythonScript **chain )
{
	if (!chain) {
		return false;
	}

	bool result = false;

	if( event < EVENT_COUNT )
	{
		unsigned int count = reinterpret_cast< unsigned int >( *(chain++) );

		for( unsigned int i = 0; i < count; ++i )
		{
			if( chain[i]->canHandleEvent( event ) )
			{
				result = true;
				break;
			}
		}
	}
	return result;
}

stError *cPythonScriptable::setProperty(const QString &name, const cVariant &value) {
	// No settable properties are available for this class
	PROPERTY_ERROR(-1, QString( "Property not found: '%1'" ).arg(name))
}

stError *cPythonScriptable::getProperty(const QString &name, cVariant &value) {
	GET_PROPERTY("classname", className());
	PROPERTY_ERROR(-1, QString( "Property not found: '%1'" ).arg(name))
}

bool cPythonScriptable::implements(const QString &name) const {
	if (name == cPythonScriptable::className()) {
		return true;
	} else {
		return false;
	}
}

const char *cPythonScriptable::className() const {
	return "pythonscriptable";
}
