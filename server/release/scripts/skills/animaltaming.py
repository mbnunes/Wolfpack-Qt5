#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################


import wolfpack.time
import skills
import wolfpack
import whrandom
from wolfpack.consts import *

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

# Button for AnimalTaming pressed on skill gump
def animaltaming( char, skill ):
	if char.socket.hastag( 'skill_delay' ):
		if wolfpack.time.currenttime() < char.socket.gettag( 'skill_delay' ):
			char.socket.clilocmessage( 500118, "", 0x3b2, 3 )
			return True
		else:
			char.socket.deltag( 'skill_delay' )

	# Assign the target request
	char.socket.clilocmessage( 502789, "", 0x3b2, 3 )
	char.socket.attachtarget("skills.animaltaming.response")
	return True

def response(char, args, target):
    dotame(char, target.char)
    return True

def dotame(char, totame):
	socket = char.socket

	# you cannot use skill while dead
	if char.dead:
		socket.clilocmessage( 502796, "", 0x3b2, 3, char )
		return

	# Check for a valid target
	if not totame:
		socket.clilocmessage( 502801, "", 0x3b2, 3, char )
		return

	# it's a player
	if not totame.npc:
		socket.clilocmessage( 502469, "", 0x3b2, 3, char )
		return

	# You can't reach that (too far away, no los, wrong map)
	if char.distanceto(totame) > TAMING_RANGE:
		socket.clilocmessage( 502803, "", 0x3b2, 3, totame )
		return
	if not char.canreach(totame, TAMING_RANGE):
		socket.clilocmessage( 502800, "", 0x3b2, 3, totame )
		return

	# Invulnerable Characters cannot be examined
	if totame.invulnerable or totame.dead:
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
	if char.skill[ TAMING ] < totame.mintaming:
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
	char.turnto( totame )

	# start taming
	socket.clilocmessage( 1010598, "", 0x3b2, 3, totame )

	char.socket.settag('skill_delay', wolfpack.time.currenttime() + TAMING_DELAY)

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
		success = char.checkskill( TAMING, totame.mintaming, 1200 )
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
			# remove "Tame" context menu
			totame.addevent('speech.pets')

			# success msg : 502799
			char.socket.clilocmessage( 502799, "", 0x3b2, 3, totame )
		else:
			removetags( totame )
			# fail msg : 502798
			char.socket.clilocmessage( 502798, "", 0x3b2, 3, totame )
#			if totame.ai:
#				totame.ai.tameattempt()
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

#		if totame.ai:
#			totame.ai.tameattempt()

#		print totame.ai.state

	char.socket.clilocmessage( msgID, "", 0x3b2, 3, char )
	char.addtimer( TAMING_DURATION, "skills.animaltaming.callback", [ havetamed, totame.serial, num_try ] )

def removetags( char ):
	if char.hastag( 'taming' ):
		char.deltag( 'taming' )
	if char.hastag( 'angry' ):
		char.deltag( 'angry' )

def onLoad():
	skills.register( TAMING, animaltaming )
