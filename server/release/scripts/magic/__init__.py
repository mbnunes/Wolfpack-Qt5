#===============================================================#
#	 )			(\_		 | WOLFPACK 13.0.0 Scripts										#
#	((		_/{	"-;	| Created by: Correa												 #
#	 )).-' {{ ;'`	 | Revised by:																#
#	( (	;._ \\ ctr | Last Modification: Created								 #
#===============================================================#
# Main Magic Script												#
#===============================================================#

from wolfpack.consts import *
from magic.utilities import *
from magic.spell import *
from wolfpack import console

import wolfpack

# Spell Registry
spells = {}

# Register spell information in the global registry
def registerspell( id, spell ):
	global spells

	if spells.has_key( id ):
		return

	spells[ id ] = spell

#
# Set up our global hook on script load
#
def onLoad():
	wolfpack.registerglobal(EVENT_CASTSPELL, "magic")

#
# A Spell has been Selected from our SpellBook
# This part of the code has to be npc safe!
#
# Mode: 0 = Book
def castSpell( char, spell, mode = 0, args = [] ):
	if char.dead:
		return

	socket = char.socket

	if not spells.has_key(spell):
		if socket:
			socket.log(LOG_ERROR, "Trying to cast unknown spell: %d\n" % spell)
			socket.sysmessage('ERROR: Unknown Spell')
		return

	spells[spell].precast(char, mode, args)

# Target Cancel
def target_cancel(char):
	if char.socket.hastag('cast_target'):
		char.socket.deltag('cast_target')

# Target Timeout
def target_timeout(char):
	char.socket.deltag('cast_target')
	char.socket.clilocmessage(500641)
	fizzle(char)

# Target Response
def target_response( char, args, target ):
	# No more npc saftey from here
	char.socket.deltag('cast_target')

	spell = args[0]
	mode = args[1]

	# Char Targets
	if target.char and (spell.validtarget == TARGET_IGNORE or spell.validtarget == TARGET_CHAR):
		if target.char.invulnerable and spell.harmful:
			if char.socket:
				char.socket.clilocmessage(1061621)
			return
		if target.char.dead and not spell.affectdead:
			if char.socket:
				char.socket.clilocmessage(501857)
			return
		if not char.cansee(target.char):
			if char.socket:
				char.socket.clilocmessage(500237)
			return
		if char.distanceto(target.char) > spell.range:
			if char.socket:
				char.socket.clilocmessage(500446)
			return
		if not char.canreach(target.char, spell.range):
			if char.socket:
				char.socket.clilocmessage(500237)
			return

		spell.target(char, mode, TARGET_CHAR, target.char)

	# Item Target
	elif target.item and ( spell.validtarget == TARGET_IGNORE or spell.validtarget == TARGET_ITEM ):
		if not char.cansee(target.item):
			if char.socket:
				char.socket.clilocmessage(500237)
			return
		if not char.canreach(target.item, spell.range):
			if char.socket:
				char.socket.clilocmessage(500237)
			return

		spell.target(char, mode, TARGET_ITEM, target.item)

	# Ground Target
	elif (target.item or target.char or target.pos) and (spell.validtarget == TARGET_IGNORE or spell.validtarget == TARGET_GROUND):
		pos = target.pos
		if target.item:
			item = target.item.getoutmostitem()
			if not item.container:
				pos = item.pos
			else:
				pos = item.container.pos
		elif target.char:
			pos = target.char.pos

		# See if the target is accesible
		if char.distanceto(pos) > spell.range:
			if char.socket:
				char.socket.clilocmessage(500446)
			return
		if not char.canreach(pos, spell.range):
			if char.socket:
				char.socket.clilocmessage(500237)
			return

		spell.target(char, mode, TARGET_GROUND, pos)

	else:
		char.socket.clilocmessage(501857)

def onCastSpell(char, spell):
	castSpell(char, spell)

# These Events happen for characters	who are casting a spell right now
def onDamage(char, type, amount, source):
	# You cannot be disturbed while using protection
	if char.propertyflags & 0x20000:
		return amount

	char.socket.clilocmessage(500641)
	fizzle(char)
	return amount

def onWalk( char, direction, sequence ):
	running = direction & 0x80
	direction &= 0x7F

	# Just turning
	if direction != char.direction:
		return

	char.socket.clilocmessage(500641)
	fizzle(char)

def onWarModeToggle(char, warmode):
	char.socket.clilocmessage(500641)
	fizzle(char)

def onLogin(char):
	fizzle(char)
