#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
import wolfpack
from wolfpack.time import *
import wolfpack.utilities

# GGS : skill can be gained every 15 min
MUSICIANSHIP_GAIN_DELAY = 15 * 60 * 1000
# instrument id : good sound ( poor sound = good sound + 1 )
sounds = { 0x0e9c:0x0038, 0x0e9d:0x0052, 0x0e9e:0x0052, 0x0eb1:0x0045, 0x0eb2:0x0045, 0x0eb3:0x004c, 0x0eb4:0x004c }

def onUse( char, item ):
	if not isinstrument( item ):
		return 0

	# first introduction of GGS
	success = 0
	cur_time = servertime()
	if not char.hastag( 'musicianship_gain_time' ):
		success = char.checkskill( MUSICIANSHIP, 0, 1000 )
		if success:
			char.settag( 'musicianship_gain_time', cur_time )
	else:
		success = 0
		last_gain = char.gettag( 'musicianship_gain_time' )
		if cur_time - last_gain >= MUSICIANSHIP_GAIN_DELAY:
			success = char.checkskill( MUSICIANSHIP, 0, 1000 )
			if not success:
				char.skill[ MUSICIANSHIP ] += 1
			success = 1
			char.settag( 'musicianship_gain_time', cur_time )

	# item wear out - will be added

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
	cur_time = servertime()
	item.settag( 'last_musicianship_use', cur_time )
	char.settag( 'instrument', item.serial )
	if success:
		sound = sounds[ item.id ]
	else:
		sound = sounds[ item.id ] + 1
	char.soundeffect( sound )
	return 1

# The base range of all bard abilities is 5 tiles, with each 15 points of skill in the ability being used increasing this range by one tile.
def bard_range( char ):
	musi_range = 5 + char.skill[ MUSICIANSHIP ] / 150
	return musi_range
