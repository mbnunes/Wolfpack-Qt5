#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Nacor                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

# yarn
import wolfpack
import wolfpack.utilities
from wolfpack.consts import GRAY

ids = [ 0x1063, 0x1066, 0x105f, 0x1061 ]

def onUse( char, item ):
	# Needs to be on ourself
	if item.getoutmostchar() != char:
		char.socket.clilocmessage( 500312, '', GRAY ) # You cannot reach that.
		return True

	char.socket.clilocmessage( 500366, '', GRAY ) # Select a loom to use that on.
	char.socket.attachtarget( "yarn.response", [ item.serial ] )
	return True


def response( char, args, target ):
	direction = char.directionto( target.pos )
	if not char.direction == direction:
		char.direction = direction
		char.update()
	item = wolfpack.finditem( args[0] )

	if ( ( char.pos.x-target.pos.x )**2 + ( char.pos.y-target.pos.y )**2 > 4):
		char.socket.clilocmessage( 502648, '', GRAY) # You are too far away to do that.
		return True

	if abs( char.pos.z - target.pos.z ) > 5:
		char.socket.clilocmessage( 502648, '', GRAY) # You are too far away to do that.
		return True

	# Check target (only item targets valid)
	if not target.item:
		char.socket.clilocmessage( 500367, '', GRAY ) # Try using that on a loom.
		return True

	if target.item.id in ids:
		color = item.color
		if ( item.amount > 1 ):
			item.amount = item.amount -1
			item.update()
		else:
			item.delete()

		# Make sure it has a tag, if not, set and default to 0 ammount.
		if target.item.hastag( 'amount' ):
			amount = target.item.gettag( 'amount' )
		else:
			target.item.settag( 'amount', 0 )
			amount = target.item.gettag( 'amount' )

		amount += 1

		if amount < 5:
			if amount == 1:
				#The bolt of cloth has just been started.
				char.socket.clilocmessage( 1010001, '', GRAY )
			elif amount == 2:
				#The bolt of cloth needs quite a bit more.
				char.socket.clilocmessage( 1010002, '', GRAY )
			elif amount == 3:
				#The bolt of cloth needs a little more.
				char.socket.clilocmessage( 1010003, '', GRAY )
			elif amount == 4:
				#The bolt of cloth is almost finished.
				char.socket.clilocmessage( 1010004, '', GRAY )
			target.item.settag( 'amount', amount )

		elif amount == 5:
			target.item.settag( 'amount', 0 )
			item_new = wolfpack.additem( "f9a" ) # Adds a bolt of cloth.
			item_new.color = color
			if not wolfpack.utilities.tocontainer( item_new, char.getbackpack() ):
				item_new.update()
			char.socket.clilocmessage( 500368, '', GRAY ) # You create some cloth and put it in your backpack.
