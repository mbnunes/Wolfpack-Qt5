#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
from wolfpack.time import *
import wolfpack
import whrandom

# max number an animal can be tamed
MAXTAME = 5
# increase in taming skill required on each successful taming
TAME_UPS = [ 0.0, 4.8, 14.4, 24.0, 33.3 ]
# max taming range
TAMING_RANGE = 3
# taming msgs
TAMING_MSGS = [ 502790, 502791, 502792, 502793, 502805 ]
# FIXME : duration between taming msgs
TAMING_DURATION = 1500

TAMING_DELAY = 10000

# Register as a global script
def onLoad():
	wolfpack.registerglobal( HOOK_CHAR, EVENT_SKILLUSE, "skills.animaltaming" )

# Button for AnimalTaming pressed on skill gump
def onSkillUse( char, skill ):
	# We only handle animaltaming
	if skill != TAMING:
		return 0

	socket = char.socket
	if char.hastag( 'skill_delay' ):
		cur_time = servertime()
		if cur_time < char.gettag( 'skill_delay' ):
			socket.clilocmessage( 500118, "", 0x3b2, 3 )
			return 1
		else:
			char.deltag( 'skill_delay' )

	# Assign the target request
	socket.clilocmessage( 502789, "", 0x3b2, 3 )
	socket.attachtarget( "skills.animaltaming.response" )

	return 1

def response( char, args, target ):

	socket = char.socket

	# you cannot use skill while dead
	if char.dead:
		socket.clilocmessage( 502796, "", 0x3b2, 3, char )
		return

	# Check for a valid target
	if not target.char:
		socket.clilocmessage( 502801, "", 0x3b2, 3, char )
		return

	# it's a player
	if target.char.socket:
		socket.clilocmessage( 502469, "", 0x3b2, 3, char )
		return

	totame = target.char

	# You can't reach that (too far away, no los, wrong map)
	if char.distanceto( totame ) > TAMING_RANGE:
		socket.clilocmessage( 502803, "", 0x3b2, 3, totame )
		return
	if not char.canreach( totame, TAMING_RANGE ):
		socket.clilocmessage( 502800, "", 0x3b2, 3, totame )
		return

	# Invulnerable Characters cannot be examined
	if target.char.invulnerable or target.char.dead:
		socket.clilocmessage( 502675, "", 0x3b2, 3, totame )
		return

	# already tamed
	if totame.tamed:
		socket.clilocmessage( 502804, "", 0x3b2, 3, totame )
		return

	# other player trying to tame
	if totame.hastag( 'taming' ):
		socket.clilocmessage( 502466, "", 0x3b2, 3, totame )
		return

	# skill is too low
	if char.skill[ TAMING ] < totame.totame:
		socket.clilocmessage( 1042590, "", 0x3b2, 3, totame )
		return

	havetamed = 0
	if totame.hastag( 'num_tamed' ):
		num_tamed = totame.gettag( 'num_tamed' )
		# tamed too many times
		if num_tamed >= MAXTAME:
			socket.clilocmessage( 1005615, "", 0x3b2, 3, totame )
			return
	else:
		totame.settag( 'num_tamed', 0 )

	# check follower control solt - will be added

	# if you have already tamed it, no skill-up check
	for i in range( 0, MAXTAME ):
		str = "tamer%i" % i
		if totame.hastag( str ):
			if totame.gettag( str ) == char.serial:
				havetamed = 1
		else:
			totame.settag( str, 0 )

	# flags
	totame.settag( 'taming', 1 )
	totame.settag( 'angry', 0 )

	# Turn toward the char we want to look at
	char.turnto( target.char )
	
	# start taming
	socket.clilocmessage( 1010598, "", 0x3b2, 3, totame )

	cur_time = servertime()
	char.settag( 'skill_delay', cur_time + TAMING_DELAY )

	# set timer
	char.addtimer( TAMING_DURATION, "skills.animaltaming.callback", [ havetamed, totame.serial, 0 ] )

def callback( char, args ):
	if len( args ) < 2:
		return

	# animal = args[ 1 ]
	totame = wolfpack.findchar( args[ 1 ] )
	if not totame:
		return
	
	if not char:
		return
	
	# tamer dead : 502796
	if char.dead:
		removetags( totame )
		char.socket.clilocmessage( 502796, "", 0x3b2, 3, totame )
		return

	# out of range : 502795
	if char.distanceto( totame ) > TAMING_RANGE:
		removetags( totame )
		char.socket.clilocmessage( 502795, "", 0x3b2, 3, totame )
		return

	# no los : 502800
	if not char.canreach( totame, TAMING_RANGE ):
		removetags( totame )
		char.socket.clilocmessage( 502800, "", 0x3b2, 3, totame )
		return

	# angry : 502794  -> let it attack the char ?
	if totame.hastag( 'angry' ) and totame.gettag( 'angry' ) > 1:
		removetags( totame )
		char.socket.clilocmessage( 502794, "", 0x3b2, 3, totame )
		return

	havetamed = args[ 0 ]
	# no. trying = args[ 2 ]
	num_try = args[ 2 ]
	if num_try > 3:
		# if have-tamed, do not advance the skill - will be added
		success = char.checkskill( TAMING, totame.totame, 1200 )
		if success:
			removetags( totame )
			# set owner
			totame.tamed = 1
			totame.owner = char
			# increase follower control slot - will be added
			# set tamed number
			num_tamed = 1
			if totame.hastag( 'num_tamed' ):
				num_tamed = totame.gettag( 'num_tamed' ) + 1
			totame.settag( 'num_tamed', num_tamed )
			# increase required taming skill
			totame.totame += TAME_UPS[ num_tamed ]
			# success msg : 502799
			char.socket.clilocmessage( 502799, "", 0x3b2, 3, totame )
		else:
			removetags( totame )
			# fail msg : 502798
			char.socket.clilocmessage( 502798, "", 0x3b2, 3, totame )
			if totame.ai:
				totame.ai.tameattempt()
		return

	num_try += 1
	# taming msg : 502790 - 502793
	# become angry : 502805
	msgID = whrandom.choice( TAMING_MSGS )
	if msgID == 502805:
		if totame.hastag( 'angry' ):
			m = totame.gettag( 'angry' )
			totame.settag( 'angry', m + 1 )
		else:
			totame.settag( 'angry', 1 )
			
		if totame.ai:
			totame.ai.tameattempt()
			
		print totame.ai.state

	char.socket.clilocmessage( msgID, "", 0x3b2, 3, totame )
	char.addtimer( TAMING_DURATION, "skills.animaltaming.callback", [ havetamed, totame.serial, num_try ] )

def removetags( char ):
	if char.hastag( 'taming' ):
		char.deltag( 'taming' )
	if char.hastag( 'angry' ):
		char.deltag( 'angry' )
