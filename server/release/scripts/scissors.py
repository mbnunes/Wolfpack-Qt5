#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

# Scissors
import wolfpack

def onUse( char, item ):
	# Needs to be on ourself
	if item.getoutmostchar() != char:
		char.socket.clilocmessage( 0, 500, 312 ) # You can't reach...
		return 1

	char.socket.clilocmessage( 0, 500, 2434 ) # What do you want to use this scissors on?
	char.socket.attachtarget( "scissors.response", [ item.serial ] )
	return 1


def response( char, args, target ):
	# 2440: Scissors can not be used on that to produce anything
	# 2437: The item you wish to cut must be in your backpack	

	item = wolfpack.finditem( args[0] )

	if not item or item.getoutmostchar() != char:
		char.socket.clilocmessage( 0, 500, 312 ) # You can't reach...
		return

	# Check target (only item targets valid)
	if not target.item:
		char.socket.clilocmessage( 0, 500, 2440, "", 0x3b2, 3, char ) # Scissors can not be used on that to produce anything
		return
		
	char.message( "cutting is disabled right now" )