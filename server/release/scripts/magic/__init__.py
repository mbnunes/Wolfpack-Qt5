#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Correa                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Main Magic Script												#
#===============================================================#

from wolfpack.consts import *
from magic.utilities import *
from magic.spell import *

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
	eventlist = char.events

	if not spells.has_key(spell):
		if socket:
			socket.log(LOG_ERROR, "Trying to cast unknown spell: %d\n" % spell)
			socket.sysmessage( 'ERROR: Unknown Spell' )
		return

	# We are frozen
	if char.frozen:
		char.socket.sysmessage(502643)
		return

	# We are already casting a spell
	if 'magic' in eventlist or ( socket and socket.hastag( 'cast_target' ) ):
		char.socket.message(502642)
		return

	# If we are using a spellbook to cast, check if we do have
	# the spell in our spellbook (0-based index)
	if mode == 0 and not hasSpell( char, spell - 1 ):
		char.message( "You don't know the spell you want to cast." )
		return

	# Are the requirements met?
	spell = spells[ spell ]
	
	if not spell.checkrequirements( char, mode ):
		return
	
	# Unhide the Caster
	if char.hidden:
		char.hidden = 0
		char.update()
	
	if spell.mantra:
		char.say( spell.mantra )
	
	# Precasting
	char.events = ['magic'] + eventlist
	char.action(ANIM_CASTAREA)	
	char.addtimer(spell.calcdelay(), 'magic.callback', [spell.spellid, mode], 0, 0, "cast_delay")

def callback( char, args ):
	eventlist = char.events
	eventlist.remove('magic')
	char.events = eventlist

	spell = spells[args[0]]
	spell.cast(char, args[1])

# Target Cancel
def target_cancel(char):
	if char.socket.hastag('cast_target'):
		char.socket.deltag('cast_target')
		
# Target Timeout
def target_timeout(char):
	char.socket.deltag('cast_target')
	char.socket.sysmessage(500641)
	fizzle(char)

# Target Response
def target_response( char, args, target ):
	# No more npc saftey from here
	char.socket.deltag( 'cast_target' )
	
	spell = spells[args[0]]
	mode = args[1]
	
	# Char Targets
	if target.char and ( spell.validtarget == TARGET_IGNORE or spell.validtarget == TARGET_CHAR ):
		spell.target(char, mode, TARGET_CHAR, target.char)
		
	# Item Target
	elif target.item and ( spell.validtarget == TARGET_IGNORE or spell.validtarget == TARGET_ITEM ):
		spell.target(char, mode, TARGET_ITEM, target.item)
		
	# Ground Target
	elif (target.item or target.char or target.pos) and (spell.validtarget == TARGET_IGNORE or spell.validtarget == TARGET_GROUND):
		pos = target.pos
		if target.item:
			pos = target.item.pos
		elif target.char:
			pos = target.char.pos
		spell.target(char, mode, TARGET_GROUND, pos)

	else:
		char.socket.sysmessage(501857)

def onCastSpell(char, spell):
	castSpell(char, spell)

# These Events happen for characters  who are casting a spell right now
def onDamage(char, type, amount, source):
	char.socket.sysmessage(500641)
	fizzle( char )

def onWalk( char, direction, sequence ):	
	running = direction & 0x80
	direction &= 0x7F

	# Just turning
	if direction != char.direction:
		return

	char.socket.sysmessage(500641)
	fizzle(char)
	
def onWarModeToggle(char, warmode):
	char.socket.sysmessage(500641)
	fizzle(char)
	
def onLogin(char):
	fizzle(char)
