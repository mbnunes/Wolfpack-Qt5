#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

# wool
import wolfpack
import wolfpack.utilities

ids = [ 0x10a4, 0x10a5, 0x1015, 0x1016, 0x101c, 0x101d, 0x1019, 0x101a ]

def onUse( char, item ):
	# Needs to be on ourself
	if item.getoutmostchar() != char:
		char.socket.clilocmessage( 0x7A258 ) # You can't reach...
		return 1

	char.socket.clilocmessage( 0x7AB7F ) # What spinning wheel do you wish to spin this on?
	char.socket.attachtarget( "wool.response", [ item.serial ] )
	return 1


def response( char, args, target ):

	direction = char.directionto( target.pos )
	if not char.direction == direction:
	    char.direction = direction
	    char.update()
	item = wolfpack.finditem( args[0] )
	
	if ( ( char.pos.x-target.pos.x )**2 + ( char.pos.y-target.pos.y )**2 > 4):
		char.socket.clilocmessage( 0x7A247 ) # You are too far away to do that.
		return 1
		
	if abs( char.pos.z - target.pos.z ) > 5:
		char.socket.clilocmessage( 0x7A247 ) # You are too far away to do that.
		return 1
	
	# Check target (only item targets valid)
	if not target.item:
		char.socket.clilocmessage( 0x7AB82 ) # Use that on a spinning wheel.
		return 1
	
	if target.item.id in ids:
		# action
		# soundeffect
		char.action( 0x9 )
		char.soundeffect( 0x2c4 )
		if ( item.amount > 1 ):
			item.amount = item.amount -1
			item.update()
		else:
			item.delete()

		item_new = wolfpack.additem( "e1d" )
		item_new.amount = 3
		if not wolfpack.utilities.tocontainer( item_new, char.getbackpack() ):
		    item_new.update()

	else:
		char.socket.clilocmessage( 0x7AB82 ) # Use that on a spinning wheel.
		return
