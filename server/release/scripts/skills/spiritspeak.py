#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Viper                          #
#   )).-" {{ ;"`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
import wolfpack
from math import floor
import random

SPSPEAK_DELAY = 5000

def onLoad():
	wolfpack.registerglobal( HOOK_CHAR, EVENT_SKILLUSE, "skills.spiritspeak" )

def onSkillUse( char, skill ):
	if skill != SPIRITSPEAK:
		return 0
	socket = char.socket

	if char.hastag( 'skill_delay' ):
		cur_time = wolfpack.servertime()
		if cur_time < char.gettag( 'skill_delay' ):
			socket.clilocmessage( 500118, "", 0x3b2, 3 )
			return 1
		else:
			char.deltag( 'skill_delay' )

	socket.clilocmessage( 0x1034BA, "", 0x3b2, 3, char )
	char.action( 0x11 )
	char.addtimer( 1000, "skills.spiritspeak.effect", [skill] )

def effect( char, args ):

	cur_time = wolfpack.servertime()
	char.settag( 'skill_delay', cur_time + SPSPEAK_DELAY )

	if not char.checkskill( SPIRITSPEAK, 0, 1000 ):
		char.socket.clilocmessage( 0x7AAAB, "", 0x3b2, 3 )
		return 0

	pos = char.pos
	x = pos.x
	y = pos.y
	map = pos.map

	items = wolfpack.items( x, y, map, 2 )
	corpses = []
	for item in items:
		if item.id == 0x2006:
			if not item.hastag( "drained" ):
				corpses.append( item )

	if len(corpses) > 0:
		corpse = random.choice( corpses )
		min = floor ( char.fame / 50 ) + 1
		max = floor ( char.skill[ SPIRITSPEAK ] / 20 )
		if min > max:
				min = max
		corpse.color = 0x3da
		char.health += random.randint( min, max )
		char.action( 0x11 )
		char.effect( 0x375a )
		char.socket.clilocmessage( 0x1031A7, "", 0x3b2, 3 )
		corpse.settag( "drained", 1 )
		corpse.update()
		return 1
	else:
		max = floor ( char.skill[ SPIRITSPEAK ] / 100 ) + 4
		min = floor( max / 2 )

		if not char.mana > 10:
			char.socket.clilocmessage( 0x1031A5, "", 0x3b2, 3 )
			return 0

		char.health += random.randint( min, max )
		char.mana -= 10
		char.effect( 0x375a )
		char.socket.clilocmessage( 0x1031A6, "", 0x3b2, 3 )
		char.updatehealth()
		char.updatemana()
		return 1

