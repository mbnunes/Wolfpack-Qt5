#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

# Dagger Script

import wolfpack
from wolfpack.utilities import hex2dec

def onUse( char, item ):
	char.socket.clilocmessage( 1, 10, 18 ) # What do you want to use this on?
	char.socket.attachtarget( "dagger.response" )
	return 1

def response( char, args, target ):
	# Corpse => Carve
	# Wood => Kindling
	model = 0

	if target.item:
		if target.item.id == 0x2006 and target.item.corpse:
			carve_corpse( char, target.item )
			return
		else:
			model = target.item.id
	elif target.char:
		char.socket.clilocmessage( 0, 500, 494 ) # You can't use a bladed item on that
		return
	else:
		model = target.model

	# Check for Trees
	if not isTree( model ):
		char.socket.clilocmessage( 0, 500, 494 ) # You can't use a bladed item on that
	else:
		hack_kindling( char, target.pos )

def carve_corpse( char, pos ):
	char.message( "You carve a corpse" )

def hack_kindling( char, pos ):
	if pos.distance( char.pos ) > 3:
		char.socket.clilocmessage( 0, 500, 312 ) # You cannot reach that
		return 1

	direction = char.directionto( pos )
	if char.direction != direction:
		char.direction = direction
		char.updateflags()

	# Let's add some Kindling
	item = wolfpack.additem( "de1" )
	item.container = char.getbackpack()

	# Let him hack
	char.action( 0x9 )
	char.soundeffect( 0x13e )
	char.socket.clilocmessage( 0, 500, 491 ) # You cannot reach that		
	
def isTree( model ):
	trees = wolfpack.list( "IDS_TREE" )

	# List consists of hexadecimal integers
	for tree in trees:
		tree = hex2dec( tree )
		if tree == model:
			return 1

	return 0
