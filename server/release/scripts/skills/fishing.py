#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
import wolfpack
import wolfpack.time
import wolfpack.utilities
from math import floor
import random

# This is the rather complex fishing script

# List of valid map-ids for water
mapWater = [ 0xa8, 0xa9, 0xaa, 0xab ]
staticWater = [ 0x1797, 0x1798, 0x1799, 0x179a, 0x179b, 0x179c ]

# List of what we can fish
fishingItems = [
	# This is a goddamn long list *grumbles*
	# propability (out of 120) = M * skill-level + I (1 percent steps)
	# You specify M and I plus a minimum skill level here
	# At the moment only linear graphs are possible
	# Minimum Skill (0-1000), M, I, List of IDs
	[ 0, -1, 110, [ '170f', '1710', '1711', '1712', '170b', '170c' ], 'shoes' ], # If the value becomes negative we don't care it's omitted then
	[ 0,  1, 0, [ '9cc', '9cd', '9ce', '9cf' ], 'a fish' ]
]

def onUse( char, item ):
	socket = char.socket

	# Can't fish on horses
	if char.itemonlayer( LAYER_MOUNT ):
		socket.clilocmessage( 0x7A4EB, "", 0x3b2, 3 ) # You can't fish while riding!
		return True

	# Are we already fishing?
	if socket.hastag( 'is_fishing' ) and socket.gettag( 'is_fishing' ) > wolfpack.time.currenttime():
		socket.clilocmessage( 0x7A4EC, "", 0x3b2, 3 ) # You are already fishing.
		return True

	iserial = item.serial
	hand1 = char.itemonlayer( LAYER_LEFTHAND )
	hand2 = char.itemonlayer( LAYER_RIGHTHAND )
	# Assign the target request
	if hand1 and hand1.serial != item.serial:
		socket.clilocmessage( 502641, "", 0x3b2, 3 ) # Must equip this item to use it!
		return True
	elif hand2 and hand2.serial != item.serial:
		socket.clilocmessage( 502641, "", 0x3b2, 3 ) # Must equip this item to use it!
		return True
	else:
		socket.clilocmessage( 0x7A4EE, "", 0x3b2, 3 ) # What water do you want to fish in?
		socket.attachtarget( "skills.fishing.response" )
	return True

def response( char, args, target ):
	socket = char.socket
	pos = target.pos

	# First: Check Distance (easiest)
	if char.pos.map != pos.map or char.pos.distance( pos ) > FISHING_MAX_DISTANCE:
		socket.clilocmessage( 0x7a4f0, "", 0x3b2, 3, char ) # You need to be closer to the water to fish!
		return

	# Second: Check Map/Static/Dynamic Water and eventual blocking stuff above it
	validspot   = 0
	blockedspot = 0
	deepwater   = 0

	mapTile = wolfpack.map( pos.x, pos.y, pos.map )

	# Simple check for saving CPU time (trusted-check)
	if not target.model in staticWater and not mapTile[ "id" ] in mapWater:
		socket.clilocmessage( 0x7a4f2, "", 0x3b2, 3, char ) # You need water to fish in!
		return

	# Check dynamics first ( And check if any objects start at z -> z + 13 )
	items = wolfpack.items( pos.x, pos.y, pos.map, 1 ) # 1: Exact

	for item in items:
		if item.id in staticWater and item.pos.z == pos.z and item.visible:
			validspot = 1
			deepwater = 0
			break

	# Then check statics ( And check if any objects start at z -> z + 13 )
	staticitems = wolfpack.statics( pos.x, pos.y, pos.map, 1 ) # Don't put this in any conditional we need it later

	if not validspot:
		for item in staticitems:
			if item[ 'id' ] in staticWater and item[ 'z' ] == pos.z:
				validspot = 1
				deepwater = 0
				break

	# Last resort check (Map)
	if not validspot and ( mapTile[ "id" ] in mapWater and mapTile[ "z" ] == pos.z ):
		validspot = 1
		deepwater = 0

	if not validspot:
		socket.clilocmessage( 0x7a4f2, "", 0x3b2, 3, char ) # You need water to fish in!
		return

	# Check if there is *anything* above that spot
	for item in items:
		if ( not item.id in staticWater ) and ( item.pos.z >= pos.z ) and ( item.pos.z <= pos.z + FISHING_BLOCK_RANGE ):
			# Check if the tiledata defines this as "impassable" or "surface"
			tile = wolfpack.tiledata( item.id )

			if tile[ "blocking" ] or tile[ "floor" ]:
				blockedspot = 1
				break

	# only check if we're not blocked already
	if not blockedspot:
		for item in staticitems:
			if ( not item[ 'id' ] in staticWater ) and ( item[ "z" ] >= pos.z ) and ( item[ "z" ] <= pos.z + FISHING_BLOCK_RANGE ):
				tile = wolfpack.tiledata( item.id )

				if tile[ "blocking" ] or tile[ "floor" ]:
					blockedspot = 1
					break

	# If the targetted tile is below map height check that as well
	mapZ = mapTile[ "z" ]
	if not blockedspot and pos.z < mapZ:
		if pos.z > ( mapZ - FISHING_BLOCK_RANGE ):
			blockedspot = 1

	if blockedspot:
		socket.clilocmessage( 0x7a4f5, "", 0x3b2, 3, char ) # You can't reach the water there.
		return

	# Turn to the position we're fishing at
	char.turnto( pos )

	# Show the Fishing animation
	char.action( 0x0c )

	# Wearout of fishing poles ?
	# ID: 0x7AD86 (0)
	# You broke your fishing pole.

	socket.settag( 'is_fishing', int( wolfpack.time.currenttime() + 5000 ) ) # Times out after 5000ms
	char.addtimer( 2500, "skills.fishing.effecttimer", [ pos, deepwater ] )
	char.addtimer( 5000, "skills.fishing.itemtimer", [ pos, deepwater ] )

	pass

# Show Effect + Sound Splash
def effecttimer( char, args ):
	if len( args ) != 2:
		return

	char.soundeffect( 0x364 )
	wolfpack.effect( 0x352d, args[0], 4, 16 )

# Get a random fish (tuple: first item -> id, second item -> name)
def getFish( fishSkill, deepwater ):

	# 120 is the maximum skill we're going to use here
	skill = min( 120, floor( fishSkill / 10 ) )

	itemid = None
	itemname = None
	possibleitems = []
	maxvalue = 0
	# We calculate a random value from 0 to this value and
	# then check the array above for matching items

	# Fill items and name with real values
	for fishItem in fishingItems:
		# Basically Ignore items we can't fish anyway
		if fishItem[ 0 ] > fishSkill:
			continue

		# Calculate the min+max values on our scala (out of 120)
		# M * SKILL + I
		value = fishItem[ 1 ] * skill + fishItem[ 2 ]

		if value > 0:
			# Debug
			# char.message( "Calculated Value for '%s': %d" % ( fishItem[4], value ) )
			possibleitems.append( [ maxvalue, maxvalue + value, fishItem[ 3 ], fishItem[ 4 ] ] ) # First item: startrange, second: stoprange, third: list of item-ids, fourth: nice name
			maxvalue += value

	# Try to find a valid item in possibleitems
	choice = random.randint( 0, maxvalue )

	for item in possibleitems:
		if choice >= item[ 0 ] and choice <= item[ 1 ]:
			itemid = item[ 2 ][ random.randrange( 0, len( item[ 2 ] ) ) ]
			itemname = item[ 3 ]
			break

	return ( itemid, itemname )

def findResourceGem( pos ):
	xBlock = int( floor( pos.x / 8 ) ) * 8
	yBlock = int( floor( pos.y / 8 ) ) * 8

	items = wolfpack.items( xBlock, yBlock, pos.map, 1 ) # 1: Exact

	for item in items:
		if item.id == 0x1ea7 and item.hastag( 'resourcecount' ) and item.hastag( 'resource' ) and item.gettag( 'resource' ) == 'fish':
			return item

	return None

# Do the real skill checks
def itemtimer( char, args ):
	if len( fishingItems ) < 1:
		socket.sysmessage( 'This script has not been configured correctly.' )
		return 0

	socket = char.socket
	socket.deltag( 'is_fishing' )

	if len( args ) != 2:
		return

	# Is there any fish left there? (8x8 grid)
	resource = findResourceGem( args[0] )

	if resource:
		amount = max( 0, int( resource.gettag( 'resourcecount' ) ) ) # We need this twice
	else:
		amount = random.randint( FISHING_FISH[0], FISHING_FISH[1] )

	if resource and amount <= 0:
		socket.clilocmessage( 0x7ad80, "", 0x3b2, 3, char ) # The fish don't seem to be biting here.
		return

	# Fail
	if not char.checkskill( FISHING, 0, 1000 ):
		socket.clilocmessage( 0x7AD83, "", 0x3b2, 3, char )
		return

	spawnmonster = 0 # Should i spawn a monster ?
	monster = '' # NPC id of what i should spawn near the player

	( itemid, itemname ) = getFish( char.skill[ FISHING ], args[1] )

	# Only add if there is something to add
	if itemid:
		if not resource:
			# Create a resource gem
			resource = wolfpack.additem( "1ea7" )
			resource.name = 'Resource Item: fish'
			resource.settag( 'resourcecount', int( amount - 1 ) )
			resource.settag( 'resource', 'fish' )
			resource.visible = 0 # GM Visible only

			pos = args[ 0 ]
			resource.moveto( wolfpack.coord( int( floor( pos.x / 8 ) ) * 8, int( floor( pos.y / 8 ) ) * 8, int( pos.z - 5 ), pos.map ) )
			resource.decay = 0
			decaytime = random.randint( FISHING_REFILLTIME[0], FISHING_REFILLTIME[1] )
			resource.addtimer( decaytime, 'skills.fishing.resourceDecayTimer', True )
			resource.update() # Send to GMs

		else:
			# Use the old one
			resource.settag( 'resourcecount', resource.gettag( 'resourcecount' ) - 1 )

		item = wolfpack.additem( itemid )

		# Was there an error?
		if not item:
			socket.sysmessage( "Please report to a gamemaster that the item '%s' couldn't be found." % itemid )

		# Otherwise try to stack it
		elif not wolfpack.utilities.tobackpack( item, char ):
			item.update()

	# Success!
	if not spawnmonster:
		socket.clilocmessage( 0xf61fc, "", 0x3b2, 3, 0, str(itemname) ) # You pull out an item :
	else:
		socket.clilocmessage( 0xf61fd, "", 0x3b2, 3, 0, str(itemname) ) # You pull out an item along with a monster :

def resourceDecayTimer( resource, arguments ):
	resource.delete()
	return
