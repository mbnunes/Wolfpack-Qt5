#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Nacor                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

# yarn
import wolfpack
import wolfpack.utilities

ids = [ 0x1063, 0x1066, 0x105f, 0x1061 ]

def onUse( char, item ):
	# Needs to be on ourself
	if item.getoutmostchar() != char:
		char.socket.clilocmessage( 0x7A258 ) # You can't reach...
		return 1

	char.socket.clilocmessage( 0x7A28E ) # Select a loom to use that on.
	char.socket.attachtarget( "yarn.response", [ item.serial ] )
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
		char.socket.clilocmessage( 0x7A28F ) # Try using that on a loom.
		return 1
	
	if target.item.id in ids:
		color = item.color
		if ( item.amount > 1 ):
			item.amount = item.amount -1
			item.update()
		else:
			item.delete()

		# Make sure it has a tag, if not, set and default to 0 ammount.
		if target.item.hastag( 'amount' ):
			amount = int( target.item.gettag( 'amount' ) )
		else:
			target.item.settag( 'amount', '0' )
			amount = int( target.item.gettag( 'amount' ) )

		amount += 1
		
		if amount < 5:
			if amount == 1:
				char.socket.sysmessage( 'You have just started the bolt of cloth.' )
			elif amount == 2:
				char.socket.sysmessage( 'The bolt of cloth could use quite a bit more.' )
			elif amount == 3:
				char.socket.sysmessage( 'The bolt of cloth could use a bit more.' )
			elif amount == 4:
				char.socket.sysmessage( 'The bolt of cloth is almost finished.' )
			target.item.settag( 'amount', str(amount) )
		
		elif amount == 5:
			target.item.settag( 'amount', '0' )
			item_new = wolfpack.additem( "f9a" ) # Adds a bolt of cloth.
			item_new.color = color
			if not wolfpack.utilities.tocontainer( item_new, char.getbackpack() ):
				item_new.update()
			char.socket.clilocmessage( 0x7A290 ) # You create some cloth and put it in your backpack.
