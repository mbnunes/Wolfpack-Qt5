#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Viper                          #
#   )).-" {{ ;"`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
import wolfpack.utilities
from wolfpack.consts import PROVOCATION, MUSICIANSHIP, ALLSKILLS, MAGERY
from math import floor
import skills

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
	socket.attachtarget( "skills.provocation.response1", [instrument] )
	return True

def response1( char, args, target ):
	socket = char.socket
	instrument = args[0]

	if target.item:
		socket.clilocmessage( 0x7A755, "", 0x3b2, 3 ) # You can't incite that!
		return False

	if target.char:
		if target.char.socket:
			socket.clilocmessage( 0x7A754, "", 0x3b2, 3 ) # Verbal taunts might be more effective!
			return False

		if target.char.tamed:
			socket.clilocmessage( 0x7A756, "", 0x3b2, 3 ) # They are too loyal to their master to be provoked.
			return False

		if target.char.priv or target.char.dead: #if target.char.priv & 0x04 or target.char.dead:
			socket.clilocmessage( 0x7A755, "", 0x3b2, 3 ) # You can't incite that!
			return False

		if target.char.id == 0x191 or target.char.id == 0x190:
			socket.clilocmessage( 0x7A757, "", 0x3b2, 3, target.char )
			return False

		tobardtarget1 = tobard( target.char, "provocation" )

		playinstrument( char, instrument, "success" )
		socket.clilocmessage( 0xF61D5, "", 0x3b2, 3 ) # You play your music and your target becomes angered.  Whom do you wish them to attack?
		socket.attachtarget( "skills.provocation.response2", [target.char, tobardtarget1, instrument] )
		return True

def response2( char, args, target ):
	socket = char.socket

	if not target.char:
		return False

	creature1 = args[0]
	tobardtarget1 = args[1]
	instrument = args[2]
	tobardtarget2 = tobard( target.char, "provoke" )
	minimum = int( ( ( tobardtarget1 + tobardtarget2 ) / 2.0 ) - 25.0 )
	maximum = int( ( ( tobardtarget1 + tobardtarget2 ) / 2.0 ) + 25.0 )

	if target.char.socket or target.char.priv & 0x04:
		socket.clilocmessage( 0xF463A, "", 0x3b2, 3 ) # You cannot perform negative acts on your target.
		return False

	if target.char == creature1:
		socket.clilocmessage( 0x7A759, "", 0x3b2, 3 ) # You can't tell someone to attack themselves!
		return False

	if target.char.owner:
		socket.clilocmessage( 0x7A756, "", 0x3b2, 3 ) # They are too loyal to their master to be provoked.
		return False

	if not char.distanceto( target.char ) <= 12 or not char.distanceto( creature1 ) <= 12:
		socket.clilocmessage( 0x100369, "", 0x3b2, 3 ) # You are too far away from one or both of the creatures for your music to have an effect.
		return False

	if not target.char.distanceto( creature1 ) < 20:
		socket.clilocmessage( 0x10036A, "", 0x3b2, 3 ) # The creatures you are trying to provoke are too far away from each other for your music to have an effect.
		return False

	# TODO : bonus for slaying items

	if not char.skill[ PROVOCATION ] >= minimum:
		socket.clilocmessage( 0x100366, "", 0x3b2, 3 ) # You have no chance of provoking those creatures.
		return False

	if not char.checkskill( MUSICIANSHIP, 0, 1000 ):
		playinstrument( char, instrument, "fail" )
		socket.clilocmessage( 0x7A75C, "", 0x3b2, 3 ) # You play rather poorly, and to no effect.
		return False

	if not char.checkskill( PROVOCATION, minimum, maximum ):
		playinstrument( char, instrument, "fail" )
		creature1.war = 1
		creature1.target = char
		creature1.update()
		socket.clilocmessage( 0x7A75F, "", 0x3b2, 3 ) # Your music fails to incite enough anger.
		return False

	playinstrument( char, instrument, "success" )
	creature1.war = 1
	creature1.target = target.char
	target.char.war = 1
	target.char.target = creature1
	creature1.update()
	target.char.update()
	socket.clilocmessage( 0x7A762, "", 0x3b2, 3 ) # Your music succeeds, as you start a fight.
	return True

def findinstrument( char, args, target ):
	socket = char.socket

	if not char.socket:
		return False

	if not target.item:
		socket.clilocmessage( 0x7A752, "", 0x3b2, 3 ) # That isn't a musical instrument.
		return False

	instruments = [ 0xE9C, 0xE9D, 0xE9E, 0xEB1, 0xEB2, 0xEB3, 0xEB4 ]
	backpack = char.getbackpack()
	serial = target.item.serial

	if not target.item.getoutmostitem() == backpack:
		if not char.canreach( target.item, 8 ):
			socket.clilocmessage( 0x7A74F, "", 0x3b2, 3 ) # You cannot see that.
			return False

		if not char.distanceto( target.item ) <= 3:
			socket.clilocmessage( 0x7A750, "", 0x3b2, 3 ) # That is too far away.
			return False

	if not target.item.id in instruments:
		socket.clilocmessage( 0x7A752, "", 0x3b2, 3 ) # That isn't a musical instrument.
		return False

	socket.settag( 'instrument', serial )
	socket.clilocmessage( 0x7A753, "", 0x3b2, 3, )
	socket.attachtarget( "skills.provocation.response1", [target.item] )
	return True

def playinstrument( char, item, how ):
	socket = char.socket
	# Instrument ID: [ Success Sound, Fail Sound ]
	instruments = {
		0xE9C:[ 0x38, 0x39 ],# Drum
		0xE9D:[ 0x52, 0x53 ],# Tambourine
		0xE9E:[ 0x52, 0x53 ],# Tambourine with tassle
		0xEB1:[ 0x43, 0x44 ],# Standing Harp
		0xEB2:[ 0x45, 0x46 ],# Harp
		0xEB3:[ 0x4C, 0x4D ],# Lute (N/S)
		0xEB4:[ 0x4C, 0x4D ] # Lute (E/S)
	}

	if how == "success":
		char.soundeffect( instruments[ item.id ][ 0 ] )
		return True
	else:
		char.soundeffect( instruments[ item.id ][ 1 ] )
		return True

def tobard( char, type ):
	socket = char.socket
	value = skilltotal( char )
	value +=  ( char.hitpoints + char.stamina + char.mana )

	if char.skill[ MAGERY ] > 0:
		value = value + 100

	# Does the Npc Have Fire Breathe Abilities?
	#if npc.spattack > 8191:
		#value += 100

	if char.poison > 0:
		value += ( char.poison * 20 )

	if value <= 700:
		value = ( floor( value ) ) / 10

	if value > 700:
		value -= 700
		value = value / 3.67
		value += 700
		value = ( floor( value ) ) / 10

	if type == "peacemaking" or type == "discord":
		value -= 5.0

	return value

def skilltotal( char ):

	total = 0

	for skill in range( 0, ALLSKILLS ):
		total += char.skill[ skill ]

	total /= 10.0
	return total

def onLoad():
	skills.register( PROVOCATION, provocation )
