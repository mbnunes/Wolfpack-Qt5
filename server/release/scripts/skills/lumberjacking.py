
import wolfpack
import wolfpack.time
import skills
from wolfpack.consts import *
from wolfpack.utilities import *
from random import randint, random
import time
from wolfpack import console, tr

extended_carpentry = int( wolfpack.settings.getbool("General", "Extended Carpentry", False, True) )
woodrespawndelay = randint( LUMBERJACKING_REFILLTIME[0], LUMBERJACKING_REFILLTIME[1] )
woodspawnamount = randint( LUMBERJACKING_LOGS[0], LUMBERJACKING_LOGS[1] )
chopdistance = LUMBERJACKING_MAX_DISTANCE
nextchopdelay = 3000

# Lumberjacking Harvest Table Items
REQSKILL = 0
MINSKILL = 1
LOGBASEID =  2

if extended_carpentry:
	LOGS = {
		'beech':	[0,  -250, '1bdd'],
		'apple':	[50,  -50, 'log_apple'],
		'peach':	[100,   0, 'log_peach'],
		'pear':		[150,  50, 'log_pear'],
		'cedar':	[200, 100, 'log_cedar'],
		'willow':	[250, 150, 'log_willow'],
		'cypress':	[300, 200, 'log_cypress'],
		'oak':		[350, 250, 'log_oak'],
		'walnut':	[400, 300, 'log_walnut'],
		'yew':		[450, 350, 'log_yew'],
		'tropical':	[500, 400, 'log_tropical'],
	}
else:
	LOGS = {
		'wood':	[0,  10, '1bdd'],
	}

# tree id, wood name
TREES = {
	3277: 'beech',
	3280: 'beech',
	3283: 'beech',
	3276: 'beech',
	3476: 'apple',
	3480: 'apple',
	3274: 'apple',
	3275: 'apple',
	3484: 'peach',
	3486: 'peach',
	3488: 'peach',
	3492: 'pear',
	3496: 'pear',
	3286: 'cedar',
	3288: 'cedar',
	3302: 'willow',
	8778: 'willow',
	8779: 'willow',
	3230: 'willow',
	3320: 'cypress',
	3323: 'cypress',
	3326: 'cypress',
	3329: 'cypress',
	3290: 'oak',
	3293: 'oak',
	8780: 'oak',
	8781: 'oak',
	3296: 'walnut',
	3299: 'walnut',
	4789: 'yew',
	4790: 'yew',
	4791: 'yew',
	4792: 'yew',
	4793: 'yew',
	4794: 'yew',
	4795: 'yew',
	4796: 'yew',
	4797: 'yew',
	3221: 'tropical',
	3222: 'tropical',
	3242: 'tropical',
	3393: 'tropical',
	3394: 'tropical',
	3395: 'tropical',
	3396: 'tropical',
	3415: 'tropical',
	3416: 'tropical',
	3417: 'tropical',
	3418: 'tropical',
	3419: 'tropical',
	3438: 'tropical',
	3439: 'tropical',
	3440: 'tropical',
	3441: 'tropical',
	3442: 'tropical',
	3460: 'tropical',
	3461: 'tropical',
	3462: 'tropical',
}

#
# Check if the character is using the right tool
#
def checktool(char, item, wearout = False):
	if not item:
		return False

	# Has to be in our posession
	if item.getoutmostchar() != char:
		char.socket.clilocmessage(500364)
		return False

	# We do not allow "invulnerable" tools.
	if not( item.hastag('remaining_uses') ):
		item.settag('remaining_uses', 35)
		item.resendtooltip()

	if( wearout ):
		uses = int(item.gettag('remaining_uses'))
		if( uses <= 1 ):
			# You broke your axe.
			char.socket.clilocmessage( 500499 )
			item.delete()
			return False
		else:
			item.settag('remaining_uses', uses - 1)
			item.resendtooltip()

	return True

def response( args ):
	target = args[0]
	tool = args[1]
	char = args[2]
	socket = char.socket
	pos = target.pos

	if not socket:
		return False

	if socket.hastag( 'is_lumberjacking' ) and ( socket.gettag( 'is_lumberjacking' ) > wolfpack.time.currenttime() ):
		socket.clilocmessage( 500119, "", GRAY )
		return False
	else:
		socket.deltag('is_lumberjacking')

	# Player can reach that ?
	if char.pos.map != pos.map or char.pos.distance( pos ) > chopdistance:
		# That is too far away
		socket.clilocmessage( 500446, "", GRAY )
		return True

	#riding?
	if( char.itemonlayer( LAYER_MOUNT ) ):
		# You can't use this while on a mount!
		char.socket.clilocmessage( 1049627 )
		return False
    
        #polymorphed? 
	if( char.polymorph ):
                # You can't do that while polymorphed.
		char.socket.clilocmessage( 1061628 )
		return False

	#dead ?
	if( char.dead ):
		# You cannot use skills while dead.
		char.socket.clilocmessage( 500012 )
		return False

        # todo: check for target.model in server statics
	veingem = getvein( socket, pos, target )

	socket.settag( 'is_lumberjacking', int( wolfpack.time.servertime() + nextchopdelay ) )
	hack_logs( char, target, tool, veingem )

	return True

def createwoodgem( target, pos ):
	gem = wolfpack.additem( 'wood_gem' )
	gem.settag( 'resourcecount', int( woodspawnamount ) )
	if extended_carpentry:
		gem.settag( 'resname', TREES[target.model] )
	else:
		gem.settag( 'resname', 'wood' )
	gem.visible = 0
	gem.decay = True
	gem.moveto( pos )
	gem.update()
	return gem

def getvein( socket, pos, target ):
	#Check if we have wood_gems on this spot
	gems = wolfpack.items( pos.x, pos.y, pos.map )
	for gem in gems:
		if gem.hastag('resource') and gem.gettag('resource') == 'wood' and gem.hastag('resname') and gem.baseid == 'wood_gem':
			return gem
			break
	gem = createwoodgem(target, pos)
	return gem

# Delay for chopping trees and getting the logs
# Animation Sequence
def chop_tree( char, args ):
	pos = args[0]
	socket = char.socket

	if char.pos.map != pos.map or char.pos.distance( pos ) > chopdistance:
		socket.deltag( 'is_lumberjacking' )
		return False

	# Turn to our lumberjacking position
	direction = char.directionto( pos )
	if char.direction != direction:
		char.direction = direction
		char.updateflags()

	# Let him hack
	char.action( 0xd )
	char.soundeffect( 0x13e )
	return True

# HACK LOGS
def hack_logs( char, target, tool, resource ):
	socket = char.socket
	pos = target.pos

	resname = resource.gettag( 'resname' )
	amount = int( resource.gettag( 'resourcecount' ) )

	resourcecount = int( resource.gettag( 'resourcecount' ) )

	# No resource left to harvest?
	if resourcecount <= 0:
		socket.clilocmessage( 500488 )
		return False

	# Turn to our lumberjacking position
	direction = char.directionto( pos )
	if char.direction != direction:
		char.direction = direction
		char.updateflags()

	# Let him hack
	char.action( 0xd )
	char.soundeffect( 0x13e )
	char.addtimer( 2000, chop_tree, [pos] )
	char.addtimer( 3500, chop_tree, [pos] )
	char.addtimer( 4000, successlumberjacking, [pos, resource.serial, amount, tool.serial, resname, LOGS ] )
	return

# HACK KINDLINGS
def hack_kindling( char, pos ):
	socket = char.socket

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

	# Resend weight
	char.socket.resendstatus()

	# Let him hack
	char.action( 0x9 )
	char.soundeffect( 0x13e )
	char.socket.clilocmessage( 0x7A30B ) # You put some kindlings in your pack

def successlumberjacking( char, args ):
	if not char:
		return False

	if skills.skilltable[ LUMBERJACKING ][ skills.UNHIDE ] and char.hidden:
		char.reveal()

	socket = char.socket
	pos = args[0] # Target POS
	resource = wolfpack.finditem(args[1])
	amount = args[2]
	tool = wolfpack.finditem(args[3])
	resname = args[4]
	table = args[5]

	# Lets make sure we stayed next to the tree
	# Player can reach that ?
	if char.pos.map != pos.map or char.pos.distance( pos ) > chopdistance:
		socket.sysmessage("Ihr habt Euch zu weit vom Baum entfernt um Holz zu hacken.")
		socket.deltag( 'is_lumberjacking' )
		return False

	if not resource.hastag( 'resourcecount' ):
		return False

	if int( resource.gettag( 'resourcecount' ) ) <= 0:
		char.socket.clilocmessage( 500488 )
		return False

	reqskill = table[resname][REQSKILL]
	chance = int( ( char.skill[LUMBERJACKING] - table[resname][MINSKILL] ) / 10 )

	if char.skill[LUMBERJACKING] < reqskill:
		socket.sysmessage("Du bist noch zu unerfahren, um Holz von diesem Baum zu schlagen.")
		#char.socket.clilocmessage( 500298 ) # You are not skilled enough...
		return False
	else:
		char.checkskill( LUMBERJACKING, reqskill, 1200 )
		# Skill Check against LUMBERJACKING
		if ( chance >= randint(1, 100) ):
			char.socket.clilocmessage( 500498 ) # You put some logs into your backpack
			checktool( char, tool, True )
			char.socket.deltag( 'is_lumberjacking' )
			success = True
		else:
			char.socket.clilocmessage( 500495 ) # You hack at the tree for a while but fail to produce...
			success = False
			return False

	if( success ):
		# Check for a backpack
		backpack = char.getbackpack()
		if not backpack:
			return False
		# Create an item in my pack
		resourceitem = wolfpack.additem( table[resname][LOGBASEID] )
		resourceitem.amount = randint( 3,10 )
		if not wolfpack.utilities.tobackpack( resourceitem, char ):
			resourceitem.update()

		# Resend weight
		char.socket.resendstatus()

		if resource.gettag( 'resourcecount' ) >= 1:
			resource.settag( 'resourcecount', int( amount - 1 ) )

		return True
