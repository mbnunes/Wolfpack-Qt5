#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

# Dagger Script

import wolfpack
import wolfpack.time
from wolfpack.utilities import hex2dec
import wolfpack.utilities
import whrandom

# List of Blood IDs
blood = [ "122a", "122b", "122d", "122f" ]

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

# CARVE CORPSE
def carve_corpse( char, corpse ):
	if corpse.container:
		char.socket.sysmessage( "You can't carve corpses in a container" )
		return

	if char.distanceto( corpse ) > 3:
		char.socket.clilocmessage( 0, 500, 312, "", 0x3b2, 3, corpse ) # You cannot reach that
		return

	# Human Bodies can always be carved
	if corpse.bodyid == 0x190 or corpse.bodyid == 0x191:
		char.message( "You can't carve a human body right now" )
		return

	# Not carvable or already carved
	if corpse.carve == '':
		char.socket.clilocmessage( 0, 500, 485, "", 0x3b2, 3, corpse ) # You see nothing useful to carve..
		return
	
	# Create all items in the carve list
	carve = wolfpack.list( corpse.carve )

	for id in carve:
		amount = 1
		# Is amount contained in it ?
		if id.find( "," ) != -1:
			parts = id.split( "," )
			id = parts[0]
			amount = int( parts[1] )

		item = wolfpack.additem( id )
		item.amount = amount
		if not wolfpack.utilities.tocontainer( item, corpse ):
			item.update()

	# Create Random Blood
	bloodid = whrandom.choice( blood )
	blooditem = wolfpack.additem( bloodid )
	blooditem.moveto( corpse.pos )
	blooditem.decay = 1
	blooditem.decaytime = wolfpack.time.servertime() + ( 30 * 1000 ) # Decay after 30 Seconds
	blooditem.update()

	char.socket.clilocmessage( 0, 500, 467, "", 0x3b2, 3, corpse ) # You carve away some meat which remains on the corpse
	corpse.carve = ''

# HACK KINDLINGS
def hack_kindling( char, pos ):
	if pos.distance( char.pos ) > 3:
		char.socket.clilocmessage( 0, 500, 312 ) # You cannot reach that
		return 1

	direction = char.directionto( pos )
	if char.direction != direction:
		char.direction = direction
		char.updateflags()

	# N O T E: For anti-cheat prevention we have to check
	# if the statics REALLY contain the tree

	# Let's add some Kindling
	item = wolfpack.additem( "de1" )
	if not wolfpack.utilities.tobackpack( item, char ):
		item.update()

	# Let him hack
	char.action( 0x9 )
	char.soundeffect( 0x13e )
	char.socket.clilocmessage( 0, 500, 491 ) # You put some kindlings in your pack

# Is "model" a Tree art-tile?
def isTree( model ):
	trees = wolfpack.list( "IDS_TREE" )

	# List consists of hexadecimal integers
	for tree in trees:
		tree = hex2dec( tree )
		if tree == model:
			return 1

	return 0
