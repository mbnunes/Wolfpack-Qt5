
from wolfpack.consts import *
import whrandom
import wolfpack
import skills
from wolfpack.time import *
from wolfpack.utilities import *
from random import randrange
#import weapons.blades

woodrespawndelay = randrange( LUMBERJACKING_MIN_REFILLTIME, LUMBERJACKING_MAX_REFILLTIME )
woodspawnamount = randrange( LUMBERJACKING_MIN_LOGS, LUMBERJACKING_MAX_LOGS )
chopdistance = LUMBERJACKING_MAX_DISTANCE
nextchopdelay = 3000

# Lumberjacking Harvest Table Items
REQSKILL = 0
MINSKILL = 1
MAXSKILL = 2
COLORID = 3
RESOURCENAME = 4
# resname, reqSkill, minSkill, maxSkill, color, 'wood name'
woodtable = \
{
	'plainwood':		[ 0, 0, 850, 0x0, 'wooden log' ],
	'yewwood':		[ 750, 750, 1500, 0x908, 'yew log' ],
}
yewtree = [ 4789, 4790, 4791, 4792, 4793, 4794, 4795, 4796, 4797 ]

def response( args ):
	target = args[0]
	tool = args[1]
	char = args[2]
	socket = char.socket
	pos = target.pos
	
	if not socket:
		return OOPS
		
	if char.hastag('is_lumberjacking') and ( int( char.gettag( 'is_lumberjacking' ) ) < servertime() ):
		char.socket.clilocmessage( 500119, "", GRAY )
		return OOPS
	
	# Player can reach that ?
	if char.pos.map != pos.map or char.pos.distance( pos ) > chopdistance:
		# That is too far away
		socket.clilocmessage( 500446, "", GRAY ) 
		return OK

	#Player also can't lumberjack when riding, polymorphed and dead.
	if char.itemonlayer( LAYER_MOUNT ):
		# You can't use this while on a mount!
		char.socket.clilocmessage( 1049627, "", GRAY ) 
		return OOPS
	
	##########
	if char.hastag( 'wood_gem' ):
		veingem = wolfpack.finditem( char.gettag( 'wood_gem' ) )
		if not veingem:
			veingem = getvein( socket, pos, target )
			if not veingem:
				char.deltag( 'wood_gem' )
				veingem = createwoodgem( target, pos )
				char.settag( 'wood_gem', str( veingem.serial ) )
	else:
		veingem = getvein( socket, pos, target )
		# OK, we still don't have a veingem to get our wood, lets create it here.
		if not veingem:
			veingem = createwoodgem( target, pos )
		else:
			char.settag( 'wood_gem', str( veingem.serial ) )

	if not veingem:
		veingem = createwoodgem( target, pos )

	if not veingem.hastag( 'resname' ) or not veingem.hastag( 'resourcecount' ):
		return OOPS

	elif veingem.hastag( 'resname' ):
		resname = veingem.gettag( 'resname' )

	char.settag( 'is_lumberjacking', str( servertime() + nextchopdelay ) )
	hack_logs( char, target, tool, veingem )

	return OK

def createwoodgem( target, pos ):
	gem = wolfpack.additem( 'wood_gem' )
	gem.settag( 'resourcecount', str( woodspawnamount ) ) # 10 - 34 ore
	if ( target.model in yewtree ) or (target.id in yewtree):
		resource.settag( 'resname', 'yewwood' )
	else:
		gem.settag( 'resname', 'plainwood' )
	gem.moveto( pos )
	gem.visible = 0
	gem.update()
	return gem

def getvein( socket, pos, target ):
	#Check if we have wood_gems near ( range = 4)
	gems = wolfpack.items( pos.x, pos.y, pos.map, 4 )
	for gem in gems:
		if wolfpack.finditem( gem.serial ):
			if gem.hastag('resource') and gem.gettag('resource') == 'wood' and gem.hastag('resname') and gem.id == hex2dec(0x1ea7):
				return gem
				break
	if not gems:
		gem = createwoodgem()
		return gem

# Delay for chopping trees and getting the logs
# Animation Sequence
def chop_tree( time, args ):
	char = args[0]
	pos = args[1]
	
	if char.pos.map != pos.map or char.pos.distance( pos ) > chopdistance:
		char.deltag('is_lumberjacking')
		return OOPS
		
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
def hack_logs( char, target, tool, resource ):
	socket = char.socket
	pos = target.pos
	
	resname = resource.gettag( 'resname' )
	amount = int( resource.gettag( 'resourcecount' ) )

	resourcecount = int( resource.gettag( 'resourcecount' ) )

	# No resource left to harvest?
	if int(resource.gettag('resourcecount')) <= 0:
		char.socket.clilocmessage( 500488, '', GRAY )
		return OOPS

	# Turn to our lumberjacking position
	direction = char.directionto( pos )
	if char.direction != direction:
		char.direction = direction
		char.updateflags()

	# Let him hack
	char.action( 0xd )
	char.soundeffect( 0x13e )
	
	wolfpack.addtimer( 2000, "weapons.blades.chop_tree", [char, pos] ) 
	wolfpack.addtimer( 3500, "weapons.blades.chop_tree", [char, pos] )
	wolfpack.addtimer( 4000, "weapons.blades.successlumberjacking", [char, pos, resource, amount, tool, resname, woodtable ] ) 
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
	pos = args[1] # Target POS
	resource = args[2]
	amount = args[3]
	tool = args[4]
	resname = args[5]
	table = args[6]
	
	# Lets make sure we stayed next to the tree
	# Player can reach that ?
	if char.pos.map != pos.map or char.pos.distance( pos ) > chopdistance:
		char.socket.sysmessage("You have moved too far away to gather any wood.")
		char.deltag('is_lumberjacking')
		return OOPS
	
	socket = char.socket
	# Check for a backpack
	backpack = char.getbackpack()
	if not backpack:
		return OOPS
	# Create an item in my pack (logs to be specific)
	resourceitem = wolfpack.additem( "1bdd" )
	resourceitem.name = str( table[ resname ][ RESOURCENAME ] )
	resourceitem.color = table[ resname ][ COLORID ]
	if (FELUCIA2XRESGAIN == TRUE) and (char.pos.map  == 0):
		resourceitem.amount = 20
	else:
		resourceitem.amount = 10
	resourceitem.settag( 'resname', str( resname ) ) # Used when crafting
	
	if not resource.hastag('resourcecount'):
		return OOPS
	
	if int(resource.gettag('resourcecount')) <= 0:
		char.socket.clilocmessage( 500488, '', GRAY )
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
		char.deltag('is_lumberjacking')

	if int( resource.gettag('resourcecount')) >= 1:
		resource.settag( 'resourcecount', amount - 1 )
		
	elif int( resource.gettag( 'resourcecount' ) ) == 0:
		if not resource.hastag ('resource_empty') and int( resource.gettag( 'resourcecount' ) ) == 0:
			resource.settag( 'resource_empty', 'true' )
			wolfpack.addtimer( woodrespawndelay, "skills.lumberjacking.respawnvein", [ resource ], 1 )

	return OK

def respawnvein( time, args ):
	vein = args[0]
	if vein.hastag ('resource_empty') and int( vein.gettag( 'resourcecount' ) ) == 0:
		vein.settag( 'resourcecount', str( woodspawnamount ) )
		vein.deltag('resource_empty')
		return OK
	else:
		return OOPS