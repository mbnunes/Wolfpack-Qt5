#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
from wolfpack.consts import *
from wolfpack.utilities import *
import whrandom

# % it will attack you when you fail to provocate
ATTACK_IF_FAIL	= 20

def onLoad():
	wolfpack.registerglobal( HOOK_CHAR, EVENT_SKILLUSE, "skills.provocation" )

def onSkillUse( char, skill ):
	if skill != PROVOCATION:
		return 0

	# check instrument in backpack
	backpack = char.getbackpack()
	if not backpack:
		return 0
	contents = backpack.content
	items = []
	for item in contents:
		if isinstrument( item ):
			items.append( item )

	# if we don't have an instrument in backpack, send target cursor to select an item - will be added
	if not len( items ):
		#char.socket.clilocmessage( 500617, "" 0x3b2, 3 )
		#char.socket.attachtarget( "skills.provocation.selectinstrument" )
		return 1

	# if there are more than one instruments, select last used one
	last_time = 0
	if len( items ) > 1:
		instrument = items[ 0 ]
		for item in items:
			if item.hastag( 'last_musicianship_use' ):
				use_time = item.gettag( 'last_musicianship_use' )
				if use_time > last_time:
					last_time = use_time
					instrument = item
	else:
		instrument = items[ 0 ]
	
	char.settag( 'provocation_instrument', instrument.serial )
	char.settag( 'provocation_step', 0 )
	char.socket.clilocmessage( 501581, "", 0x3b2, 3 )
	char.socket.attachtarget( "skills.provocation.response" )
	return 1

def response( char, args, target ):
	if not char.hastag( 'provocation_step' ):
		clear_tags( char )
		return 0
	# we can only incite an npc
	if not target.char or not target.char.npc:
		char.socket.clilocmessage( 501589, "", 0x3b2, 3 )
		clear_tags( char )
		return 1
	provo_range = musicianship.bard_range( char )
	# too far away
	if char.distanceto( target.char ) > provo_range:
		char.socket.clilocmessage( 501584, "", 0x3b2, 3 )
		clear_tags( char )
		return 1
	# cannot see it
	if not char.canreach( target.char, provo_range ):
		char.socket.clilocmessage( 501583, "", 0x3b2, 3 )
		clear_tags( char )
		return 1

	# ok, select next target
	if char.gettag( 'provocation_step' ) == 0:
		char.settag( 'provocatoin_target1', target.char.serial )
		char.settag( 'provocation_step', 1 )
		char.socket.clilocmessage( 1008085, "", 0x3b2, 3 )
		char.socket.attachtarget( "skills.provocation.response" )
		return 1

	# now, we go into provocation
	if not char.hastag( 'provocation_instrument' ):
		clear_tags( char )
		return 0
	# check the instrument
	instrument = wolfpack.finditem( char.gettag( 'provocation_instrument' ) )
	if not instrument:
		clear_tags( char )
		return 0
	# check targets
	if not char.hastag( 'provocation_target1' ):
		clear_tags( char )
		return 0
	target1 = wolfpack.findchar( char.gettag( 'provocation_target1' ) )
	if not target1:
		clear_tags( char )
		return 0
	target2 = target.char

	# target1 and target2 must be 'canreach' at the same time
	if not char.canreach( target1 ) or not char.canreach( target2 ):
		char.socket.clilocmessage( 1049449, "", 0x3b2, 3 )
		clear_tags( char )
		return 1

	if target1 == target2:
		char.socket.clilocmessage( 501593, "", 0x3b2, 3 )
		clear_tags( char )
		return 1

	clear_tags( char )

	# skill check : we should get the bard difficulty from xml definition
	loskill = 0
	hiskill = 1000
	if char.skill[ PROVOCATION ] < loskill or char.skill[ MUSICIANSHIP ] < loskill:
		char.socket.clilocmessage( 1049446, "", 0x3b2, 3 )
		char.socket.clilocmessage( 501600, "", 0x3b2, 3, target1 )
		return 1
	result = char.checkskill( MUSICIANSHIP, loskill, hiskill )
	musicianship.play_instrument( char, instrument, result )
	# play poor
	if not result:
		char.socket.clilocmessage( 501596, "", 0x3b2, 3 )
		return 1
	result = char.checkskill( PROVOCATION, loskill, hiskill )
	# provocation fail
	if not result:
		char.socket.clilocmessage( 501599, "", 0x3b2, 3 )
		# sometimes, it will attack you
		chance = whrandom.randint( 0, 100 )
		if chance < ATTACK_IF_FAIL:
			target1.attack( char )
		return 1

	# start combat
	target1.attack( target2 )
	target2.attack( target1 )
	return 1

def clear_tags( char ):
	if char.hastag( 'provocation_step' ):
		char.deltag( 'provocation_step' )
	if char.hastag( 'provocation_instrument' ):
		char.deltag( 'provocation_instrument' )
	if char.hastag( 'provocation_target1' ):
		char.deltag( 'provocation_target1' )

