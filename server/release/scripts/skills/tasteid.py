#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
from wolfpack.utilities import *
from wolfpack.time import *
import wolfpack
import skills

TASTEID_DELAY = 5000
TASTE_RANGE = 4
potion_keg = [ 0x0e7f, 0x1ad6, 0x1ad7 ]

def onLoad():
	wolfpack.registerglobal( HOOK_CHAR, EVENT_SKILLUSE, "skills.tasteid" )

def tasteid( char, skill ):
	if skill != TASTEID:
		return 0

	if char.hastag( 'skill_delay' ):
		cur_time = servertime()
		if cur_time < char.gettag( 'skill_delay' ):
			char.socket.clilocmessage( 500118, "", 0x3b2, 3 )
			return 1
		else:
			char.deltag( 'skill_delay' )

	char.socket.clilocmessage( 502807, "", 0x3b2, 3 )
	char.socket.attachtarget( "skills.tasteid.response" )
	return 1

def response( char, args, target ):
	# we can taste potion keg/potion/food only
	if target.char:
		char.socket.clilocmessage( 502816, "", 0x3b2, 3, target.char )
		return
	if not target.item:
		return
	item = target.item
	if not item.id in potion_keg and not item.id in range( 0x0f06, 0x0f0d ) and item.type != 14:
		char.socket.clilocmessage( 502820, "", 0x3b2, 3, item )
		return
	# we have it in our backpack or near
	if not item.getoutmostchar() == char:
		if not char.distanceto( target ) > TASTE_RANGE:
			char.socket.clilocmessage( 502815, "", 0x3b2, 3 )
			return
		if not char.canreach( target, TASTE_RANGE ):
			char.socket.clilocmessage( 502827, "", 0x3b2, 3 )
			return

	# set skill delay
 	cur_time = servertime()
 	char.settag( 'skill_delay', cur_time + TASTEID_DELAY )

	char.socket.clilocmessage( 502814, "", 0x3b2, 3 )

	success = char.checkskill( TASTEID, 0, 1000 )
	if not success:
		char.socket.clilocmessage( 502823, "", 0x3b2, 3 )

	# potion keg / potion
	if item.id in potion_keg or item.id in range( 0x0f06, 0x0f0d ):
		if success:
			char.socket.clilocmessage( 502809, "", 0x3b2, 3, item )
			char.socket.showspeech( item, item.name )
		else:
			#char.socket.clilocmessage( 502821, "", 0x3b2, 3, item )
			# poison potion/keg
		return
	
	# TODO : more specific checkskill result is needed
	# food
	if item.type == 14:
		if success:
			char.socket.clilocmessage( 1010600, "", 0x3b2, 3, item )
		else:
			# poisoned item
			if item.poisoned > 0:
				rand = whrandom.randint( 0, 10 )
				if rand < 5:
					char.socket.clilocmessage( 1010599, "", 0x3b2, 3, item )
				else:
					char.socket.clilocmessage( 502822, "", 0x3b2, 3, item )
					if item.poisoned > char.poisoned:
						char.poisoned = item.poisoned

def onLoad():
	skills.register( TASTEID, tasteid )
	