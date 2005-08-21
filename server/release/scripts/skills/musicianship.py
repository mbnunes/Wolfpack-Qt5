#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
import wolfpack
import wolfpack.time
import wolfpack.utilities
import environment
import skills

MUSICIANSHIP_DELAY = 1000
# instrument id : good sound ( poor sound = good sound + 1 )
sounds = {
		0x0e9c: 0x0038,
		0x0e9d: 0x0052,
		0x0e9e: 0x0052,
		0x0eb1: 0x0045,
		0x0eb2: 0x0045,
		0x0eb3: 0x004c,
		0x0eb4: 0x004c,
		0x2805: 0x003d
	}

# reduce the remaining uses
def wearout( player, item ):
	# We do not allow "invulnerable" tools.
	if not item.hastag('remaining_uses'):
		player.socket.clilocmessage(1044038)
		item.delete()
		return False

	uses = int(item.gettag('remaining_uses'))
	if uses <= 1:
		player.socket.clilocmessage(1044038)
		item.delete()
		return False
	else:
		item.settag('remaining_uses', uses - 1)
		item.resendtooltip()
	return True

def onUse( char, item ):
	if not wolfpack.utilities.isinstrument( item ):
		return False

	socket = char.socket

	if socket.hastag( 'skill_delay' ):
		if wolfpack.time.currenttime() < socket.gettag( 'skill_delay' ):
			socket.clilocmessage( 500118, "", 0x3b2, 3 )
			return True
		else:
			socket.deltag( 'skill_delay' )

	if skills.skilltable[ MUSICIANSHIP ][ skills.UNHIDE ] and char.hidden:
		char.reveal()

	success = char.checkskill( MUSICIANSHIP, 0, 1000 )

	char.socket.settag( 'skill_delay', int( wolfpack.time.currenttime() + MUSICIANSHIP_DELAY ) )

	return play_instrument( char, item, success )

# play sound acording to the instrument good/poor
def play_instrument( char, item, success ):
	# this check is needed ? other bard skills will use this function but..
	# check will not be needed - will be removed
	if not item.id in sounds.keys():
		return 0
	# when we use bard skill, if we have more than one instrument,
	# the 'last used' item will be used
	# set last use item for musicianship
	item.settag( 'last_musicianship_use', wolfpack.time.currenttime() )
	char.socket.settag( 'instrument', item.serial )
	if success:
		if wearout( char, item ):
			sound = sounds[ item.id ]
	else:
		sound = sounds[ item.id ] + 1
	char.soundeffect( sound )
	return True

# The base range of all bard abilities is 5 tiles, with each 15 points of skill in the ability being used increasing this range by one tile.
def bard_range( char ):
	musi_range = 5 + char.skill[ MUSICIANSHIP ] / 150
	return musi_range
