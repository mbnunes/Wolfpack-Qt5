#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
import wolfpack
import wolfpack.time
import whrandom
import math

# range in which the bandage exist ( when it is not in the backpack )
BANDAGE_RANGE = 2
# distance the healer must stay in while healing
HEAL_RANGE = 2
# distance check delay. this script will check the distance every n msec
CHECK_DELAY = 2000
# healing, anatomy required to cure a char
CURE_HEALING = 600
CURE_ANATOMY = 600
HEAL_OTHER_DELAY = 5000
CURE_OTHER_DELAY = 6000
# healing, anatomy required to res a char
RES_ANATOMY = 800
RES_HEALING = 800
# delay for res a char
RES_DELAY = 10000

#def onLoad():
#	wolfpack.registerglobal( HOOK_CHAR, EVENT_SKILLUSE, "skills.healing" )

# healing using the bandages
def onUse( char, item ):
	if not char or not item:
		return
	# it must be in his/her backpack or around
	if item.getoutmostchar() != char:
		if char.distanceto( item ) > BANDAGE_RANGE:
			# msg
			return
	char.socket.clilocmessage( 500948, "", 0x3b2, 3 )
	char.socket.attachtarget( "skills.healing.response" )
	return True

def response( char, args, target ):
	if not target.char:
		char.socket.clilocmessage( 500970, "", 0x3b2, 3 )
		return True
	# will be added : a golem would not healed by bandages

	# count bandage
	if not char.countresource( 0x0e21 ):
		return True

	anatomy = char.skill[ ANATOMY ]
	healing = char.skill[ HEALING ]

	# dead char : res it
	if target.char.dead:
		# you cannot res yourself
		if char == target.char:
			#char.socket.clilocmessage()
			return True
		if healing >= RES_HEALING and anatomy >= RES_ANATOMY:
			res_char( char, target.char, healing, anatomy )
		else:
			char.socket.clilocmessage( 1049656, "", 0x3b2, 3 )
		return True
	if target.char.poisoned:
		if healing >= CURE_HEALING and anatomy >= CURE_ANATOMY:
			cure_char( char, target.char, healing, anatomy )
		#else:
			#char.socket.clilocmessage()
		return True

	# calc total heal amount : used formula from UOSS
	heal_min = 3 + ( char.skill[ ANATOMY ] + char.skill[ HEALING ] ) / 50
	heal_max = 10 + char.skill[ ANATOMY ] / 50 + char.skill[ HEALING ] / 20
	heal_amount = whrandom.choice( range( heal_min, heal_max ) )
	if not heal_amount:
		heal_amount = 1

	# calc total delay : used formula from UOSS
	if char == target.char:
		delay = 9400 + 60 * ( 120 - char.dexterity )
	else:
		delay = HEAL_OTHER_DELAY

	char.socket.clilocmessage( 500956, "", 0x3b2, 3 )
	# loop
	start_time = wolfpack.time.servertime()
	end_time = start_time + delay
	chance = 1
	char.addtimer( CHECK_DELAY, "skills.healing.delay_check", [ resto.serial, chance, start_time, end_time, heal_amount + 2 ] )

# NOTICE: here, i don't use success/fail result from checkskill for resurrection/cure cases
def res_char( char, resto, healing, anatomy ):
	# res chance : 25% at 80/80 skill, max 75% at 100/100
	chance = 250 + ( healing - RES_HEALING ) + ( anatomy - RES_ANATOMY )
	end_time = wolfpack.time.currenttime() + RES_DELAY
	char.addtimer( CHECK_DELAY, "skills.healing.delay_check", [ resto.serial, chance, start_time, end_time, 0 ] )

def cure_char( char, cureto, healing, anatomy ):
	# 80% at skill 60/60, 100% over skill 80/80
	chance = min( 800 + ( healing - CURE_HEALING ) / 2 + ( anatomy - CURE_ANATOMY ) / 2, 1000 )
	if char == cureto:
		end_time = wolfpack.time.currenttime() + 8400 + 60 * ( 120 - char.dexterity )
	else:
		end_time = wolfpack.time.currenttime() + CURE_OTHER_DELAY
	char.addtimer( CHECK_DELAY, "skills.healing.delay_check", [ cureto.serial, chance, start_time, end_time, 1 ] )

def delay_check( char, args ):
	if not char:
		return
	if len( args ) < 4:
		return

	# you dead : cannot continue
	if char.dead:
		char.socket.clilocmessage( 500962, "", 0x3b2, 3 )
		return

	healto = wolfpack.findchar( args[ 0 ] )
	if not healto:
		return

	# bandage check
	if not char.countresource( 0x0e21 ):
		return

	# you must stay near the target
	if char.distanceto( healto ) > HEAL_RANGE:
		char.socket.clilocmessage( 500963, "", 0x3b2, 3 )
		return

	chance = args[ 1 ]
	start_time = args[ 2 ]
	end_time = args[ 3 ]
	heal_type = args[ 4 ]

	# useup one bandage
	char.useresource( 1, 0x0e21 )

	if wolfpack.time.currenttime() >= end_time:
		# resurrect
		if heal_type == 0:
			if chance >= whrandom.randint( 0, 1000 ):
				char.socket.clilocmessage( 500965, "", 0x3b2, 3 )
				healto.resurrect()
			else:
				char.socket.clilocmessage( 500966, "", 0x3b2, 3 )
			# here, i don't use the checkskill result for res/cure case
			if not char.checkskill( HEALING, RES_HEALING, 1000 ):
				char.checkskill( ANATOMY, RES_ANATOMY, 1000 )
		# cure
		elif heal_type == 1:
			if chance >= whrandom.randint( 0, 1000 ):
				char.socket.clilocmessage( 503260, "", 0x3b2, 3 )
				healto.poisoned = 0
				if healto.hastag( 'poisoned' ):
					healto.deltag( 'poisoned' )
			else:
				char.socket.clilocmessage( 503259, "", 0x3b2, 3 )
			# here, i don't use the checkskill result for res/cure case
			if not char.checkskill( HEALING, CURE_HEALING, 1000 ):
				char.checkskill( ANATOMY, CURE_ANATOMY, 1000 )
		# heal
		else:
			# i used the checkskill result only for the heal case
			chance1 = char.checkskill( HEALING, 0, 1000 )
			if not chance1:
				chance1 = char.checkskill( ANATOMY, 0, 1000 )
			if chance1:
				char.socket.clilocmessage( 500969, "", 0x3b2, 3 )
				heal_amount = heal_type - 2
				maxhp = 50 + char.strength / 2
				healto.hp = min( maxhp, healto.hp + heal_amount )
			else:
				char.socket.clilocmessage( 500968, "", 0x3b2, 3 )
		return

	# go loop
	delay = CHECK_DELAY
	if ( wolfpack.time.currenttime() + CHECK_DELAY ) > end_time:
		delay = end_time - current_time
	char.addtimer( delay, "skills.healing.delay_check", [ healto.serial, chance, start_time, end_time, heal_type ] )
