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

def onUse( char, item ):
	if( not char.socket ):
		return 0

	# Wrong item-ids
	if ( item.id != 0xDE1 ) and ( item.id != 0xDE2 ):
		return 0

	if( item.container ):
		char.socket.sysmessage( "You need to place this on the ground to light it." )
		return 1
	
	if( char.distanceto( item ) > 5 ):
		char.socket.sysmessage( "You are too far away to use this." )
		return 1

	if not char.checkskill( wolfpack.CAMPING, 0, 500 ):
		char.socket.sysmessage( "You fail to light a fire." )
		return 1

	# Create the campfire
	campfire = wolfpack.additem( "de3" )
	campfire.moveto( item.pos )
	campfire.update()
	
	# Delete the kindling
	item.delete()
	
	# Start a timer	

	return 1