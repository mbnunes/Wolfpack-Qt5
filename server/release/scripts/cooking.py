#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Nacor                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

# Cooking
import wolfpack
import wolfpack.utilities
from wolfpack.consts import COOKING
import random

# Ids of items we can cook on
ids_heat = [ 	0xde3, # Campfire
		0x92b, 0x92c, 0x930, 0x931, 0x937, 0x93d, 0x945, 0x94b, 0x953, 0x959, 0x961, 0x967, # Stone Ovens + Fireplaces
		0xe31, # Brazier
		0x461, 0x462, 0x46a, 0x46f, 0x475, 0x47b, 0x482, 0x489 # Sandstone Ovens + Fireplaces
	    ]

# The first value is the original uncooked item id
# The List contains two values:
# a) the id if cooking succeeds
# b) the id if cooking fails, empty if item should just be deleted
# we won't use "1eb0" because it is not stackable
ids = {
	# raw bird : cooked bird
	0x9b9 : [ "9b7", "burned_bird" ],
	0x9ba : [ "9b8", "burned_bird" ],

	# raw fish : cooked fish
	0x97a : [ "97b", "burned_fish" ],

	# raw rib : cooked rib
	0x9f1 : [ "9f2", "burned_rib" ],

	# dough : bread
	0x103d : [ "103b", "" ],

	# cookie mix : cookie
	0x103f : [ "160c", "" ],

	# bowl of flour : cake
	0xa1e : [ "9e9", "" ],

	# raw chickenleg : cooked chickenleg
	0x1607 : [ "1608", "burned_chickenleg" ],

	# raw lamleg : cooked lamleg
	0x1609 : [ "160a", "burned_lambleg" ],

	# unbaked pie : baked pie
	0x1042 : [ "1041", "" ],

	# uncooked pizza : pizza
	0x1083 : [ "1040", "" ]

	}

def onUse( char, item ):
	# Needs to be on ourself
	if item.getoutmostchar() != char:
		char.socket.clilocmessage( 0x7A258 ) # That doesnt belong to you
		return True

	if ids.has_key( item.id ):
		char.socket.clilocmessage( 0x7A1FE ) # What should i cook this on
		char.socket.attachtarget( "cooking.response", [ item.serial ] )
		return True

	return False

def response( char, args, target ):
	direction = char.directionto( target.pos )

	if char.direction != direction:
		char.direction = direction
		char.update()

	item = wolfpack.finditem( args[0] ) # What we want to cook
	id = item.id

	if not item or item.getoutmostchar() != char:
		char.socket.clilocmessage( 0x7ACA2 ) # That belongs to someone else.
		return True

	# Are we too far away from the target ?
	if ( ( char.pos.x-target.pos.x )**2 + ( char.pos.y-target.pos.y )**2 > 4):
		char.socket.clilocmessage( 0x7A247 ) # You are too far away to do that.
		return True

	if abs( char.pos.z - target.pos.z ) > 5:
		char.socket.clilocmessage( 0x7A247 ) # You are too far away to do that.
		return True

	# We can only cook on dynamic ovens/fireplaces
	if not target.item or not target.item.id in ids_heat:
		char.socket.clilocmessage( 0x7A3D2 ) # You can't cook on that.
		return True

	# We're cooking one by one
	if item.amount > 1:
		item.amount -= 1
		item.update()
	else:
		item.delete()

	# Succeess ?
	if char.checkskill( COOKING, 0, 1000 ):
		item = wolfpack.additem( ids[ id ][0] )
		if not wolfpack.utilities.tobackpack( item, char ):
			item.update()

		char.socket.clilocmessage( random.choice( [ 0x7A3CF, 0x7A3D0 ] ) ) # Either "Looks delicious." or "Mmmm, smells good."
		char.socket.clilocmessage( 0x7A3D1 ) # You put the cooked food into your backpack.

	else:
		burned_id = ids[ id ][1]

		if burned_id != "":
			burned = wolfpack.additem( burned_id )
			if not wolfpack.utilities.tobackpack( burned, char ):
				burned.update()

		char.socket.clilocmessage( 0x7A3CE ) # You burn the food to a crisp! It's ruined.
