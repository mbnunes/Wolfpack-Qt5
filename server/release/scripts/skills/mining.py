#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: codex                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Sep, 30 '03	        #
#################################################################

from wolfpack.consts import *
import whrandom
import wolfpack
import skills
from wolfpack.time import *
from wolfpack.utilities import *
from random import randrange

#mining calling from pickaxe.py and shovel.py

oresound = 0x126
#in seconds
miningdelay = 1000
orerespawndelay = randrange( 5, 10 ) * 60000 # 5 - 10 minutes
# Name, reqSkill, minSkill, maxSkill, successClilocId, itemId, color, mutateVeinChance%, VeinChanceToFallback%

# 0x19b7, 0x19b8, 0x19ba give 1 ingot.
# 0x19b9 gives 2 ingots.
oreids = [ 0x19b7, 0x19b8, 0x19ba, 0x19b9 ]
oredefs = [ '19b7', '19b8', '19ba', '19b9' ]
oretable = \
{
	'iron':			[ 0, 0, 1000, 1007072, 0x0, 'iron ore' ],
	'dullcopper':	[ 650, 250, 1050, 1007073, 0x973, 'dull copper ore' ],
	'shadowiron':	[ 700, 300, 1100, 1007074, 0x966, 'shadow iron ore' ], 
	'copper':		[ 750, 350, 1150, 1007075, 0x96d, 'copper ore' ],
	'bronze':		[ 800, 400, 1200, 1007076, 0x972, 'bronze ore' ],
	'gold':			[ 850, 450, 1250, 1007077, 0x8a5, 'gold ore' ],
	'silver':			[ 875, 475, 1275, 1007077, 0x430, 'silver ore' ],
	'agapite':		[ 900, 500, 1300, 1007078, 0x979, 'agapite ore' ],
	'verite':			[ 950, 550, 1350, 1007079, 0x89f, 'verite ore' ],
	'valorite':		[ 990, 590, 1390, 1007080, 0x8ab, 'valorite ore' ],
	'merkite':		[ 995, 600, 1400, 1007080, 0x8b0, 'merkite ore' ],
	'mythril':		[ 1000, 605, 1405, 1007080, 0x84c, 'mythril ore' ]
}

def mining( char, pos, tool ):
	wolfpack.addtimer( 1300, "skills.mining.domining", [ char, oresound, tool, pos ] )
	char.settag( 'is_mining', str( servertime() + miningdelay ) )
	char.turnto( pos )
	char.action( ANIM_ATTACK3 )
	return OK

def getvein( socket, pos ):
	#Check if we have ore_gems near ( range = 4 )
	gems = wolfpack.items( pos.x, pos.y, pos.map, 4 )
	if len( gems ) < 1:
		gem = wolfpack.additem( 'ore_gem' )
		gem.settag( 'resourcecount', str( randrange( 10, 34 ) ) ) # 10 - 34 ore
		gem.settag( 'resname', 'iron' ) # All veins should default to iron ore.
		
		# This will give it a chance to be a random ore type, this can change later.
		colorchance = randrange( 0, 100 )
		if colorchance >= 99 and colorchance <= 100: # %2 chance for merkite (99 - 100)
			gem.settag( 'resname2', 'mythril' )
		elif colorchance >= 96 and colorchance <= 98: # %3 chance for merkite (96 - 99)
			gem.settag( 'resname2', 'merkite')
		elif colorchance >= 92 and colorchance <= 95: # 4% chance for valorite (92 - 95)
			gem.settag( 'resname2', 'valorite' )
		elif colorchance >= 87 and colorchance <= 91: # 5% chance for verite (87 - 91)
			gem.settag( 'resname2', 'verite')
		elif colorchance >= 81 and colorchance <= 86: # 6% chance for agapite (81 - 86)
			gem.settag( 'resname2', 'agapite')
		elif colorchance >= 74 and colorchance <= 80: # 7% chance for silver (74 - 80)
			gem.settag( 'resname2', 'silver')
		elif colorchance >= 66 and colorchance <= 73: # 8% chance for gold (66 - 73)
			gem.settag( 'resname2', 'gold')
		elif colorchance >= 57 and colorchance <= 65: # 9% chance for bronze (57 - 65)
			gem.settag( 'resname2', 'bronze')
		elif colorchance >= 47 and colorchance <= 56: # 10% chance for copper (47 - 56)
			gem.settag( 'resname2', 'copper')
		elif colorchance >= 36 and colorchance <= 46: # 11% chance for shadow iron (36 - 46)
			gem.settag( 'resname2', 'shadowiron')
		elif colorchance >= 24 and colorchance <= 35: # 12% chance for dull copper (24 - 35)
			gem.settag( 'resname2', 'dullcopper')

		gem.moveto( pos )
		gem.visible = 0
		gem.update()
		return gem
	else:
		return gems[0]

#Response from mining tool
def response( char, args, target ):
	socket = char.socket
	if not socket:
		return OOPS
	
	pos = target.pos
	
	# Player can reach that ?
	if char.pos.map != pos.map or char.pos.distance( pos ) > MINING_MAX_DISTANCE:
		socket.clilocmessage( 500446, "", GRAY, NORMAL ) # That is too far away
		return OK
	
	tool = args[0]
	
	#Player also can't mine when riding, polymorphed and dead.
	#Mine char ?!
	if target.char:
		socket.clilocmessage( 501863, "", GRAY, NORMAL ) # You can't mine that.
		return OK
	
	#Check if item is ore gem
	elif target.item and isoregem( target.item ):
		#Mine if ore gem is validated
		socket.sysmessage( 'Ore gem founded' )
		mining( char, target.pos, tool )
		return OK
	
	#Find tile by it's position if we haven't model
	elif target.model == 0:
		map = wolfpack.map( target.pos.x, target.pos.y, target.pos.map )
		if ismountainorcave( map['id'] ):
			mining( char, target.pos, tool )
		else:
			socket.clilocmessage( 501862, "", GRAY, NORMAL ) # You can't mine there.
		return OK
	
	#Find tile by it's model
	elif target.model != 0:
		if ismountainorcave( target.model ):
			#add new ore gem here and mine
			mining( char, target.pos, tool )
		else:
			socket.clilocmessage( 501862, "", GRAY, NORMAL ) # You can't mine there.
		return OK
	else:
		return OOPS
	
	return OK

#Sound effect
def domining( time, args ):
	char = args[0]
	char.soundeffect( args[1] )
	tool = args[2]
	pos = args[3]
	socket = char.socket

	if char.hastag( 'ore_gem' ):
		veingem = wolfpack.finditem( char.gettag( 'ore_gem' ) )
		if not veingem:
			veingem = getvein( socket, pos )
			if not veingem:
				char.deltag( 'ore_gem' )
				return OOPS
	else:
		veingem = getvein( socket, pos )
		if not veingem:
			return OOPS
		else:
			char.settag( 'ore_gem', str( veingem.serial ) )
	
	if char.distanceto( veingem ) > MINING_MAX_DISTANCE:
		veingem = getvein( socket, pos )
	
	if not veingem:
		return OOPS
	
	if not veingem.hastag( 'resname' ) or not veingem.hastag( 'resourcecount' ):
		return OOPS
	
	if veingem.hastag( 'resname2' ):
		chance = randrange( 0, 10 )
		if chance >= 6: # 50% chance to dig up whatever colored ore may exist.
			resname = veingem.gettag( 'resname2' )
		else: # Default to the iron if failed to find color.
			resname = veingem.gettag( 'resname' )

	elif not veingem.hastag( 'resname2' ) and veingem.hastag( 'resname' ):
		resname = veingem.gettag( 'resname' )

	resourcecount = int( veingem.gettag( 'resourcecount' ) )
	reqskill = oretable[ resname ][ REQSKILL ]

	success = 0
	char.update()
	char.updatestats()
	
	# Are you skilled enough ? And here is ore ?
	if ( resourcecount >= 1 ) and ( char.skill[ MINING ] >= reqskill ):
		skills.checkskill( char, veingem, MINING, 0 )
		if resourcecount >= 5: # Digs up the large ore.
			successmining( char, veingem, oretable, resname, 1, oredefs[3] )
		elif resourcecount == 3 or resourcecount == 4: # Picks one of the smaller ore types
			randomore = randrange( 1, 2 ) 
			successmining( char, veingem, oretable, resname, 1, oredefs[randomore] )
		elif resourcecount == 1 or resourcecount == 2: # Smallest ore only
			successmining( char, veingem, oretable, resname, 1, oredefs[0] )
		# tool durability drain
		if tool.health > 1:
			tool.health -= 1
			tool.update()
		elif tool.health == 1:
			tool.delete()
		success = 1
	
	elif resourcecount == 0:
		socket.sysmessage( "There is no ore left here to mine...", GRAY, NORMAL )
		if not veingem.hastag( 'resource_empty' ):
			wolfpack.addtimer( orerespawndelay, "skills.mining.respawnvein", [ veingem ] )
			veingem.settag( 'resource_empty', 'true')
		# veingem
		success = 1
	
	if success == 0:
		socket.clilocmessage( 501869, "", GRAY, NORMAL ) # You loosen some rocks but fail to find any usable ore.
	
	char.deltag('is_mining')
	
	return OK

def successmining( char, gem, table, resname, amount, ore ):
	socket = char.socket
	message = table[ resname ][ SUCCESSCLILOC ]
	backpack = char.getbackpack()
	if not backpack:
		return OOPS
	resourceitem = wolfpack.additem( ore )
	resourceitem.decay = 1
	resourceitem.name = str( table[ resname ][ RESOURCENAME ] )
	resourceitem.color = table[ resname ][ COLORID ]
	resourceitem.amount = amount
	resourceitem.settag( 'resname', str( resname ) ) # Used when smelting
	
	if not gem.hastag('resourcecount'):
		return OOPS
		
	gem.settag( 'resourcecount', str( int( gem.gettag( 'resourcecount' ) ) - amount ) )
	
	if int( gem.gettag( 'resourcecount' ) ) == 0:
		if not gem.hastag ('resource_empty') and int( gem.gettag( 'resourcecount' ) ) == 0:
			gem.settag( 'resource_empty', 'true' )
			wolfpack.addtimer( orerespawndelay, "skills.mining.respawnvein", [ gem ] )
	
	if not wolfpack.utilities.tocontainer( resourceitem, backpack ):
		resourceitem.update()

	if resname == 'silver' or resname == 'merkite' or resname == 'mythril':
		socket.sysmessage( "You dig some " + table[ resname ][ RESOURCENAME ]  + " and put it in your backpack.", GRAY, NORMAL )
	else:
		socket.clilocmessage( message, "", GRAY, NORMAL )
	return OK

def respawnvein( time, args ):
	vein = args[0]
	if vein.hastag ('resource_empty') and int( vein.gettag( 'resourcecount' ) ) == 0:
		vein.settag( 'resourcecount', str( randrange( 10, 34 ) ) )
		vein.deltag('resource_empty')
	return OK
