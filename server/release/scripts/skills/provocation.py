#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Viper                          #
#   )).-" {{ ;"`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################
# to do: karma and fame for the provocateur

import wolfpack
import wolfpack.utilities
from wolfpack.consts import PROVOCATION, MUSICIANSHIP, ALLSKILLS, MAGERY, LOG_MESSAGE, PLAYER_BODIES_ALIVE
from math import floor
import skills
from musicianship import play_instrument

# Some chars are unprovocable
def unprovocable(char):
	return False

def provocation( char, skill ):
	socket = char.socket

	if not char.socket:
		return False
	# Only handle provocation
	if skill != PROVOCATION:
		return False

	if not socket.hastag( 'instrument' ):
		socket.clilocmessage( 0x7A74E, '', 0x3b2, 3 ) # What instrument shall I play music on?
		socket.attachtarget( "skills.provocation.findinstrument" )
		return True

	instserial = socket.gettag( 'instrument' )
	instrument = wolfpack.finditem( instserial )
	backpack = char.getbackpack()

	if not instrument or not instrument.getoutmostitem() == backpack:
		socket.clilocmessage( 0x7A74E, "", 0x3b2, 3 ) # What instrument shall I play music on?
		socket.attachtarget( "skills.provocation.findinstrument" )
		return True

	socket.clilocmessage( 0x7A74D, "", 0x3b2, 3 ) # What do you wish to incite?
	socket.attachtarget( "skills.provocation.response1", [instrument.serial] )
	return True

def response1( char, args, target ):
	socket = char.socket
	instrument = wolfpack.finditem(args[0])

	if target.item:
		socket.clilocmessage( 0x7A755, "", 0x3b2, 3 ) # You can't incite that!
		return False

	if target.char:
		if target.char.player:
			socket.clilocmessage( 0x7A754, "", 0x3b2, 3 ) # Verbal taunts might be more effective!
			return False

		if target.char.tamed:
			socket.clilocmessage( 0x7A756, "", 0x3b2, 3 ) # They are too loyal to their master to be provoked.
			return False

		if target.char.invulnerable or target.char.dead:
			socket.clilocmessage( 0x7A755, "", 0x3b2, 3 ) # You can't incite that!
			return False

		if target.char.id in PLAYER_BODIES_ALIVE:
			socket.clilocmessage( 0x7A757, "", 0x3b2, 3, target.char )
			return False

		if unprovocable(target.char):
			socket.clilocmessage( 0x100366, "", 0x3b2, 3 ) # You have no chance of provoking those creatures.
			return False

		play_instrument( char, instrument, True )
		socket.clilocmessage( 0xF61D5, "", 0x3b2, 3 ) # You play your music and your target becomes angered.  Whom do you wish them to attack?
		socket.attachtarget( "skills.provocation.response2", [target.char.serial, instrument.serial] )
		return True

def response2( char, args, target ):
	socket = char.socket

	if not target.char:
		return False

	creature1 = wolfpack.findchar(args[0])
	instrument = wolfpack.finditem(args[1])

	if target.char.socket or target.char.gm:
		socket.clilocmessage( 0xF463A, "", 0x3b2, 3 ) # You cannot perform negative acts on your target.
		return False

	if target.char == creature1:
		socket.clilocmessage( 0x7A759, "", 0x3b2, 3 ) # You can't tell someone to attack themselves!
		return False

	if unprovocable(target.char):
		socket.clilocmessage( 0x100366, "", 0x3b2, 3 ) # You have no chance of provoking those creatures.
		return False

	if not char.distanceto( target.char ) <= 12 or not char.distanceto( creature1 ) <= 12:
		socket.clilocmessage( 0x100369, "", 0x3b2, 3 ) # You are too far away from one or both of the creatures for your music to have an effect.
		return False

	if not target.char.distanceto( creature1 ) < 20:
		socket.clilocmessage( 0x10036A, "", 0x3b2, 3 ) # The creatures you are trying to provoke are too far away from each other for your music to have an effect.
		return False

	if not char.checkskill( MUSICIANSHIP, 0, 1000 ):
		play_instrument( char, instrument, False )
		socket.clilocmessage( 0x7A75C, "", 0x3b2, 3 ) # You play rather poorly, and to no effect.
		return False

	diff = ((skills.musicianship.GetDifficultyFor( char, instrument, creature1 ) + skills.musicianship.GetDifficultyFor( char, instrument, target.char )) * 0.5) - 5.0
	music = char.skill[MUSICIANSHIP] / 10.0
	if music > 100.0:
		diff -= (music - 100.0) * 0.5

	if not char.checkskill( PROVOCATION, diff-25.0, diff+25.0 ):
		play_instrument( char, instrument, False )
		creature1.war = 1
		creature1.target = char
		creature1.update()
		socket.clilocmessage( 0x7A75F, "", 0x3b2, 3 ) # Your music fails to incite enough anger.
		return False

	if skills.skilltable[ PROVOCATION ][ skills.UNHIDE ] and char.hidden:
		char.reveal()

	play_instrument( char, instrument, True )
	creature1.war = 1
	creature1.attacktarget = target.char
	creature1.fight(target.char)
	target.char.war = 1
	target.char.attacktarget = creature1
	target.char.fight(creature1)
	creature1.update()
	target.char.update()
	socket.clilocmessage( 0x7A762, "", 0x3b2, 3 ) # Your music succeeds, as you start a fight.

	char.log(LOG_MESSAGE,"Provocating Fight beetween %s (%s,%d) and %s (%s,%d)" % (creature1.name, creature1.baseid, creature1.serial, target.char.name, target.char.baseid, target.char.serial) )

	return True

def findinstrument( char, args, target ):
	socket = char.socket

	if not char.socket:
		return False

	if not target.item:
		socket.clilocmessage( 0x7A752, "", 0x3b2, 3 ) # That isn't a musical instrument.
		return False

	backpack = char.getbackpack()
	serial = target.item.serial

	if not target.item.getoutmostitem() == backpack:
		if not char.canreach( target.item, 8 ):
			socket.clilocmessage( 0x7A74F, "", 0x3b2, 3 ) # You cannot see that.
			return False

		if not char.distanceto( target.item ) <= 3:
			socket.clilocmessage( 0x7A750, "", 0x3b2, 3 ) # That is too far away.
			return False

	if not isinstrument(target.item):
		socket.clilocmessage( 0x7A752, "", 0x3b2, 3 ) # That isn't a musical instrument.
		return False

	socket.settag( 'instrument', serial )
	socket.clilocmessage( 0x7A753, "", 0x3b2, 3, ) # Whom do you wish to incite?
	socket.attachtarget( "skills.provocation.response1", [target.item.serial] )
	return True

def onLoad():
	skills.register( PROVOCATION, provocation )
