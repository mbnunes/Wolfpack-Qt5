
from wolfpack.consts import *
import whrandom
import wolfpack
import skills
from wolfpack.time import *
from wolfpack.utilities import *
from random import randrange
import weapons.blades

def response( args ):
	target = args[0]
	tool = args[1]
	char = args[2]
	socket = char.socket

	if not socket:
		return OOPS
	
	pos = target.pos
	
	# Player can reach that ?
	if char.pos.map != pos.map or char.pos.distance( pos ) > LUMBERJACKING_MAX_DISTANCE:
		# That is too far away
		socket.clilocmessage( 500446, "", GRAY ) 
		return OK

	#Player also can't lumberjack when riding, polymorphed and dead.
	if char.itemonlayer( LAYER_MOUNT ):
		# You can't use this while on a mount!
		char.socket.clilocmessage( 1049627, "", GRAY ) 
		return OOPS

	hack_logs( char, target, tool )

	return OK

def getvein( socket, pos ):
	#Check if we have wood_gems near ( range = 4 )
	wood = wolfpack.items( pos.x, pos.y, pos.map, 5 )
	if not len( wood ) < 1:
		wood = wolfpack.additem( 'wood_gem' )
		wood.settag( 'resourcecount', str( randrange( LUMBERJACKING_MIN_LOGS, LUMBERJACKING_MAX_LOGS ) ) )
		wood.settag( 'resname', 'plainwood' ) # All veins should default to plain wood
		wood.moveto( pos )
		wood.visible = 0
		wood.update()
		return wood
	else:
		return wood[0]

# Delay for chopping trees and getting the logs
# Animation Sequence
def chop_tree_one( time, args ):
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

# HACK LOGS
def hack_logs( char, target, tool ):
	socket = char.socket
	pos = target.pos
	
	if char.hastag( 'wood_gem' ):
		veingem = wolfpack.finditem( char.gettag( 'wood_gem' ) )
		if not veingem:
			veingem = getvein( socket, pos )
			if not veingem:
				char.deltag( 'wood_gem' )
				return OOPS
	else:
		veingem = getvein( socket, pos )
		if not veingem:
			return OOPS
		else:
			char.settag( 'wood_gem', str( veingem.serial ) )
	
	if char.distanceto( veingem ) > LUMBERJACKING_MAX_DISTANCE:
		veingem = getvein( socket, pos )
	
	if not veingem:
		return OOPS
	
	if not veingem.hastag( 'resname' ) or not veingem.hastag( 'resourcecount' ):
		return OOPS
	
	resource = veingem
	resname = veingem.gettag( 'resname' )
	amount = int( veingem.gettag( 'resourcecount' ) )

	resourcecount = int( veingem.gettag( 'resourcecount' ) )

	# No resource left to harvest?
	if resourcecount < 1:
		char.socket.clilocmessage( 0x7A30D, "", 0x3b2, 3, char ) # There's not enough wood here to harvest.
		return OOPS

	# Turn to our lumberjacking position
	direction = char.directionto( pos )
	if char.direction != direction:
		char.direction = direction
		char.updateflags()

	# Let him hack
	char.action( 0xd )
	char.soundeffect( 0x13e )
	
	wolfpack.addtimer( 2000, "weapons.blades.chop_tree_one", [char, pos] ) 
	wolfpack.addtimer( 3500, "weapons.blades.chop_tree_two", [char, pos] )
	wolfpack.addtimer( 4000, "weapons.blades.successlumberjacking", [char, pos, resource, amount, tool ] ) 
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

def successlumberjacking( time, args ):
	char = args[0]
	pos = args[1]
	resource = args[2]
	amount = args[3]
	tool = args[4]
	socket = char.socket
	# Check for a backpack
	backpack = char.getbackpack()
	if not backpack:
		return OOPS
	# Create an item in my pack (logs to be specific)
	resourceitem = wolfpack.additem( "1bdd" )
	resourceitem.amount = 10
	resourceitem.settag( 'resname', 'plainwood' )
	
	if not resource.hastag('resourcecount'):
		return OOPS
	
	# Skill Check against LUMBERJACKING
	if not char.checkskill( LUMBERJACKING, LUMBERJACKING_MIN_SKILL, LUMBERJACKING_MAX_SKILL ):
		char.socket.clilocmessage( 500495 ) # You hack at the tree for a while but fail to produce...
		return OOPS
	else:
		if not wolfpack.utilities.tobackpack( resourceitem, char ):
			resourceitem.update()
		char.socket.clilocmessage( 500498 ) # You put some logs into your backpack
		if int( tool.gettag('remaining_uses') ) > 1:
			tool.settag( 'remaining_uses', str( int(tool.gettag('remaining_uses')) - 1 ) )
			tool.resendtooltip()
		elif int(tool.gettag('remaining_uses')) == 1:
			tool.delete()
			# You broke your axe!
			socket.clilocmessage( 500499, '', GRAY ) 
		char.deltag('wood_gem')

	if int( resource.gettag('resourcecount')) >= 1:
		resource.settag( 'resourcecount', amount - 1 )
		
	elif int( resource.gettag( 'resourcecount' ) ) == 0:
		if not resource.hastag ('resource_empty') and int( resource.gettag( 'resourcecount' ) ) == 0:
			resource.settag( 'resource_empty', 'true' )
			wolfpack.addtimer( int( LUMBERJACKING_REFILLTIME * 1000 ), "skills.lumberjacking.respawnvein", [ resource ] )

	return OK

def respawnvein( time, args ):
	vein = args[0]
	if vein.hastag ('resource_empty') and int( vein.gettag( 'resourcecount' ) ) == 0:
		vein.settag( 'resourcecount', str( randrange( LUMBERJACKING_MIN_LOGS, LUMBERJACKING_MAX_LOGS ) ) )
		vein.deltag('resource_empty')
		return OK
	else:
		return OOPS