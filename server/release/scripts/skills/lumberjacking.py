
import wolfpack
import wolfpack.time
from wolfpack.consts import *
from wolfpack.utilities import hex2dec
import wolfpack.utilities
import whrandom
import weapons.blades
import skills

trees = []
trees = wolfpack.list( "IDS_TREE" )

# Check for Wood at a specific position
def check_wood( pos ):
	statics = wolfpack.statics( pos.x, pos.y, pos.map )

	for item in statics:
		id = item[ "id" ]

		if is_tree( id ):
			return 1

	# Check normal items
	dynamics = wolfpack.items( pos.x, pos.y, pos.map )

	for item in dynamics:
		if is_tree( item.id ):
			return 1

	return 0

# Delay for chopping trees and getting the logs
def chop_delay( time, args ):
	char = args[0]
	pos = args[1]
	resource = args[2]
	amount = args[3]
	
	# Skill Check against LUMBERJACKING
	if not char.checkskill( LUMBERJACKING, LUMBERJACKING_MIN_SKILL, LUMBERJACKING_MAX_SKILL ):
		char.socket.clilocmessage( 0x7A30F, "", 0x3b2, 3, char  ) # You hack at the tree for a while but fail to produce...
		return
	
	char.socket.clilocmessage( 0x7A312, "", 0x3b2, 3, char  ) # You put some logs into your backpack

	# Create an item in my pack (logs to be specific)
	item = wolfpack.additem( "1bdd" )
	item.amount = 10
	item.settag( 'resname', 'plainwood' )
	if not wolfpack.utilities.tobackpack( item, char ):
		item.update()

	# Create a resource item if neccesary
	if not resource:
		resource = wolfpack.additem( "1ea7" )
		resource.name = 'Resource Item: logs'
		resource.settag( 'resourcecount', str( amount - 1 ) )
		resource.settag( 'resource', 'logs' )
		resource.visible = 0 # GM Visible only
		resource.moveto( pos )
		resource.decay = 1
		resource.decaytime = wolfpack.time.servertime() + ( LUMBERJACKING_REFILLTIME * 1000 )
		resource.update() # Send to GMs
	else:
		resource.settag( 'resourcecount', amount - 1 )
	
	return OK

# Animation Sequence
def chop_tree_two( time, args ):
	char = args[0]
	pos = args[1]
	# Turn to our lumberjacking position
	direction = char.directionto( pos )
	if char.direction != direction:
		char.direction = direction
		char.updateflags()

	# Let him hack
	char.action( 0xd )
	char.soundeffect( 0x13e )
	return OK
	
def chop_tree_three( time, args ):
	char = args[0]
	pos = args[1]
	# Turn to our lumberjacking position
	direction = char.directionto( pos )
	if char.direction != direction:
		char.direction = direction
		char.updateflags()

	# Let him hack
	char.action( 0xd )
	char.soundeffect( 0x13e )
	return OK

# Is "model" a Tree art-tile?
def is_tree( model ):
	# List consists of hexadecimal integers
	for tree in trees:
		tree = hex2dec( tree )
		if tree == model:
			return 1
	return 0

# HACK LOGS
def hack_logs( char, pos ):
	# Try to find a resource item
	resource = None

	dynamics = wolfpack.items( pos.x, pos.y, pos.map )

	for item in dynamics:
		if item.id == 0x1ea7 and item.hastag( 'resourcecount' ) and item.hastag( 'resource' ) and item.gettag( 'resource' ) == 'logs':

			# Check if the resource item didn't expire already (could be a bug!)
			if item.decaytime <= wolfpack.time.servertime():
				item.delete()
			else:
				resource = item
				break

	# Default Range for each tree is 10 to 15 Logs
	if not resource:
		amount = whrandom.randint( LUMBERJACKING_MIN_LOGS, LUMBERJACKING_MAX_LOGS )
	else:
		amount = int( resource.gettag( 'resourcecount' ) )

	# No resource left to harvest?
	if amount < 1:
		char.socket.clilocmessage( 0x7A30D, "", 0x3b2, 3, char ) # There's not enough wood here to harvest.
		return

	# Turn to our lumberjacking position
	direction = char.directionto( pos )
	if char.direction != direction:
		char.direction = direction
		char.updateflags()

	# Let him hack
	char.action( 0xd )
	char.soundeffect( 0x13e )
	
	wolfpack.addtimer( 2000, "weapons.blades.chop_tree_two", [char, pos] ) 
	wolfpack.addtimer( 3500, "weapons.blades.chop_tree_three", [char, pos] )
	wolfpack.addtimer( 4000, "weapons.blades.chop_delay", [char, pos, resource, amount ] ) 
	return

# HACK KINDLINGS
def hack_kindling( char, pos ):
	if pos.distance( char.pos ) > 3:
		char.socket.clilocmessage( 0x7A258 ) # You cannot reach that
		return 1

	direction = char.directionto( pos )
	if char.direction != direction:
		char.direction = direction
		char.updateflags()

	# Let's add some Kindling
	item = wolfpack.additem( "de1" )
	if not wolfpack.utilities.tobackpack( item, char ):
		item.update()

	# Let him hack
	char.action( 0x9 )
	char.soundeffect( 0x13e )
	char.socket.clilocmessage( 0x7A30B ) # You put some kindlings in your pack
