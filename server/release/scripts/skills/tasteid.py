#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
from wolfpack.utilities import *
import wolfpack
import wolfpack.time
import skills

TASTEID_DELAY = 1000

def tasteid(char, skill):
	socket = char.socket

	if socket.hastag('skill_delay'):
		if wolfpack.time.currenttime() < socket.gettag('skill_delay'):
			socket.clilocmessage(500118)
			return True
		else:
			socket.deltag('skill_delay')

	socket.clilocmessage(502807) # What would you like to taste?
	socket.attachtarget("skills.tasteid.response")
	return True

def response(char, args, target):
	socket = char.socket

	if skills.skilltable[ TASTEID ][ skills.UNHIDE ] and char.hidden:
		char.reveal()

	socket.settag('skill_delay', int( wolfpack.time.currenttime() + TASTEID_DELAY ) )

	if target.item:
		if not char.canreach(target.item, 2):
			socket.clilocmessage(502815) # You are too far away to taste that.
			return

		if target.item.hasscript( 'food' ):
			if char.checkskill(TASTEID, 0, 1000):
				if target.item.hastag( 'poisoning_char' ):
					target.item.say(1038284) # It appears to have poison smeared on it.
				else:
					target.item.say(1010600) # You detect nothing unusual about this substance.
			else:
				target.item.say(502823) # You cannot discern anything about this substance.

		elif target.item.hasscript( 'potions' ):
			target.item.say(502813) # You already know what kind of potion that is.
			name = target.item.name.split( '#' )
			target.item.say( int(name[1]) )

		elif target.item.hasscript( 'potionkeg' ):
			kegfill = 0
			if target.item.hastag( 'kegfill' ):
				kegfill = target.item.gettag( 'kegfill' )
			if kegfill <= 0:
				target.item.say(502228) # There is nothing in the keg to taste!
			else:
				target.item.say(502229) # You are already familiar with this keg's contents.
				name = target.item.name.split( '#' )
				target.item.say( int(name[1]) )
		else:
			target.item.say( 502820 ) # That's not something you can taste.

	elif target.char:
		socket.clilocmessage(502816) # You feel that such an action would be inappropriate.

	else:
		socket.clilocmessage(502820) # That's not something you can taste.

	return True

def onLoad():
	skills.register(TASTEID, tasteid)
