#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

# Script for igniting a fire of a kindling
# Item must be on the ground
# 0xDE1
# 0xDE2

import wolfpack
import wolfpack.time
from wolfpack.consts import *

def onUse( char, item ):
	if( not char.socket ):
		return 0

	# Wrong item-ids
	if ( item.id != 0xDE1 ) and ( item.id != 0xDE2 ):
		return 0

	owner = item.getoutmostchar()

	if owner and owner != char:
		char.socket.clilocmessage( 0, 500, 1694, "", 0x3b2, 0x3, item ) # You can't ignite that, it belongs to someone else
		return 1

	cont = item.getoutmostitem()

	if not owner:
		if( char.distanceto( cont ) > 5 ):
			char.socket.clilocmessage( 0, 500, 491, "", 0x3b2, 0x3, item ) # You cannot reach that
			return 1

	if not char.checkskill( CAMPING, 0, 500 ):
		char.socket.clilocmessage( 0, 500, 1696, "", 0x3b2, 0x3, item ) # You fail to ignite the campfire
		return 1

	# Get the position we want to have
	if owner:
		pos = owner.pos
	else:
		pos = cont.pos

	# Create the campfire
	campfire = wolfpack.additem( "de3" )
	campfire.moveto( pos )
	campfire.decay = 1
	campfire.decaytime = wolfpack.time.servertime() + ( 2 * 60 * 1000 ) # Decays in 2 Minutes
	campfire.update()
	
	# Delete the kindlings
	item.delete()
	
	return 1
