#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  # Created by: codex
#   )).-' {{ ;'`   # Revised by: Dreoth
#  ( (  ;._ \\ ctr # Last Modification: Jan 26 2004
#################################################################

from wolfpack.consts import *
import whrandom
import wolfpack
import skills
from wolfpack.time import *
from wolfpack.utilities import *
from random import randint

#mining calling from pickaxe.py and shovel.py

#in seconds
miningdelay = 1000
orerespawndelay = randint( MINING_MIN_REFILLTIME, MINING_MAX_REFILLTIME )
orespawnamount = randint( MINING_MIN_ORE, MINING_MAX_ORE )

# 0x19b7, 0x19b8, 0x19ba give 1 ingot.
# 0x19b9 gives 2 ingots.
oreids = [ 0x19b7, 0x19b8, 0x19ba, 0x19b9 ]
oredefs = [ '_ore_1', '_ore_2', '_ore_4', '_ore_3' ]

# Mining Harvest Table Items
REQSKILL = 0
MINSKILL = 1
SUCCESSCLILOC = 2
COLORID = 3
RESOURCENAME = 4
# resname, reqSkill, minSkill, successClilocId, color, 'ore name'
oretable = \
{
	'iron':			[ 0, 0, 1007072, 0x0, 'iron ore' ],
	'dullcopper':	[ 650, 0, 1007073, 0x973, 'dull copper ore' ],
	'shadowiron':	[ 700, 0, 1007074, 0x966, 'shadow iron ore' ],
	'copper':		[ 750, 0, 1007075, 0x96d, 'copper ore' ],
	'bronze':		[ 800, 80, 1007076, 0x972, 'bronze ore' ],
	'gold':			[ 850, 170, 1007077, 0x8a5, 'gold ore' ],
	'agapite':		[ 900, 270, 1007078, 0x979, 'agapite ore' ],
	'verite':			[ 950, 380, 1007079, 0x89f, 'verite ore' ],
	'valorite':		[ 990, 475, 1007080, 0x8ab, 'valorite ore' ]
}

def mining( char, pos, tool ):
	wolfpack.addtimer( 1300, "skills.mining.domining", [ char, SOUND_MINING, tool, pos ] )
	char.socket.settag( 'is_mining', ( servertime() + miningdelay ) )
	char.turnto( pos )
	char.action( ANIM_ATTACK3 )
	return OK

def createoregem( pos ):
	gem = wolfpack.additem( 'ore_gem' )
	gem.settag( 'resourcecount', orespawnamount ) # 10 - 34 ore
	gem.settag( 'resname', 'iron' ) # All veins should default to iron ore.
	# This will give it a chance to be a random ore type, this can change later.
	colorchance = randint( 0, 100 )
	if colorchance >= 100 and colorchance <= 99: # 2% chance for valorite (99 - 100)
		gem.settag( 'resname2', 'valorite' )
	elif colorchance >= 96 and colorchance <= 98: # 3% chance for verite (96 - 98)
		gem.settag( 'resname2', 'verite')
	elif colorchance >= 92 and colorchance <= 95: # 4% chance for agapite (92 - 95)
		gem.settag( 'resname2', 'agapite')
	elif colorchance >= 86 and colorchance <= 91: # 6% chance for gold (87 - 91)
		gem.settag( 'resname2', 'gold')
	elif colorchance >= 79 and colorchance <= 85: # 7% chance for bronze (81 - 86)
		gem.settag( 'resname2', 'bronze')
	elif colorchance >= 71 and colorchance <= 78: # 8% chance for copper (73 - 80)
		gem.settag( 'resname2', 'copper')
	elif colorchance >= 61 and colorchance <= 70: # 10% chance for shadow iron (63 - 72)
		gem.settag( 'resname2', 'shadowiron')
	elif colorchance >= 50 and colorchance <= 60: # 11% chance for dull copper (51 - 62)
		gem.settag( 'resname2', 'dullcopper')
	gem.moveto( pos )
	gem.visible = 0
	gem.update()
	return gem

def getvein( socket, pos ):
	#Check if we have ore_gems near ( range = 4 )
	gems = wolfpack.items( pos.x, pos.y, pos.map, 4 )
	for gem in gems:
		if wolfpack.finditem( gem.serial ):
			if gem.hastag( 'resource' ) and gem.gettag( 'resource' ) == 'ore' and gem.hastag( 'resname' ) and gem.id == hex2dec( 0x1ea7 ):
				return gem
				break
	if not gems:
		gem = createoregem( pos )
		return gem

#Response from mining tool
def response( char, args, target ):
	socket = char.socket
	if not socket:
		return OOPS

	pos = target.pos

	# Player can reach that ?
	if char.pos.map != pos.map or char.pos.distance( pos ) > MINING_MAX_DISTANCE:
		# That is too far away
		socket.clilocmessage( 500446, "", GRAY )
		return OK

	tool = args[0]

	#Player also can't mine when riding, polymorphed and dead.
	#Mine char ?!
	if target.char:
		# You can't mine that.
		socket.clilocmessage( 501863, "", GRAY )
		return OK

	#Find tile by it's position if we haven't model
	elif target.model == 0:
		map = wolfpack.map( target.pos.x, target.pos.y, target.pos.map )
		if ismountainorcave( map['id'] ):
			mining( char, target.pos, tool )
		else:
			# You can't mine there.
			socket.clilocmessage( 501862, "", GRAY )
		return OK

	#Find tile by it's model
	elif target.model != 0:
		if ismountainorcave( target.model ):
			#add new ore gem here and mine
			mining( char, target.pos, tool )
		else:
			socket.clilocmessage( 501862, "", GRAY ) # You can't mine there.
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

	if char.socket.hastag( 'ore_gem' ):
		veingem = wolfpack.finditem( char.socket.gettag( 'ore_gem' ) )
		if not veingem:
			veingem = getvein( socket, pos )
			if not veingem:
				char.socket.deltag( 'ore_gem' )
				veingem = createoregem( pos )
				char.socket.settag( 'ore_gem', veingem.serial )
	else:
		veingem = getvein( socket, pos )
		if not veingem:
			veingem = createoregem( pos )
			char.socket.settag( 'ore_gem', veingem.serial )
		else:
			char.socket.settag( 'ore_gem', veingem.serial )

	if char.distanceto( veingem ) > MINING_MAX_DISTANCE:
		veingem = getvein( socket, pos )

	if not veingem:
		veingem = createoregem( pos )

	if not veingem.hastag( 'resname' ) or not veingem.hastag( 'resourcecount' ):
		return OOPS

	if veingem.hastag( 'resname2' ):
		chance = randint( 1, 10 )
		if chance >= 6: # 50% chance to dig up whatever colored ore may exist.
			resname = veingem.gettag( 'resname2' )
		else: # Default to the iron if failed to find color.
			resname = veingem.gettag( 'resname' )

	elif not veingem.hastag( 'resname2' ) and veingem.hastag( 'resname' ):
		resname = veingem.gettag( 'resname' )

	resourcecount = veingem.gettag( 'resourcecount' )
	reqskill = oretable[ resname ][ REQSKILL ]
	chance = int( ( oretable[ resname ][ REQSKILL ] - oretable[ resname ][ MINSKILL ] ) / 10 )
	if chance > 100:
		chance = 100
	elif chance < 0:
		chance = 0

	success = 0

	if char.pos.map != pos.map or char.pos.distance( pos ) > MINING_MAX_DISTANCE:
		char.socket.clilocmessage( 501867, '', GRAY )
		return OOPS

	# Are you skilled enough ? And here is ore ?
	if ( resourcecount >= 1 ) and ( char.skill[ MINING ] >= reqskill ):
		if not skills.checkskill( char, veingem, MINING, 0 ):
			# You loosen some rocks but fail to find any usable ore.
			socket.clilocmessage( 501869, "", GRAY )
			success = 0
			return
		elif chance >= randint(1, 100):
			if resourcecount >= 5: # Digs up the large ore.
				successmining( char, veingem, oretable, resname, 1, oredefs[3] )
			elif resourcecount == 3 or resourcecount == 4: # Picks one of the smaller ore types
				randomore = randint( 1, 2 )
				successmining( char, veingem, oretable, resname, 1, oredefs[randomore] )
			elif resourcecount == 1 or resourcecount == 2: # Smallest ore only
				successmining( char, veingem, oretable, resname, 1, oredefs[0] )
			# tool durability drain
			if not tool.hastag( 'remaining_uses' ):
				tool.settag( 'remaining_uses', tool.health )
			else:
				if int( tool.gettag( 'remaining_uses' ) ) > 1:
					tool.settag( 'remaining_uses', int( int( tool.gettag( 'remaining_uses' ) ) - 1 ) )
					tool.resendtooltip()
				elif tool.gettag( 'remaining_uses' ) == 1:
					tool.delete()
					# You have worn out your tool!
					socket.clilocmessage( 1044038, '', GRAY )
				char.socket.deltag( 'ore_gem' ) # To save memory, we don't really need this.
			success = 1
		else:
			success = 0

	elif resourcecount == 0:
		socket.sysmessage( "There is no metal here to mine.", GRAY )
		if not veingem.hastag( 'resource_empty' ):
			wolfpack.addtimer( orerespawndelay, "skills.mining.respawnvein", [ veingem ], 1 )
			veingem.settag( 'resource_empty', 'true' )
		# veingem
		success = 1

	if success == 0:
		# You loosen some rocks but fail to find any usable ore.
		socket.clilocmessage( 501869, "", GRAY )

	char.socket.deltag( 'is_mining' )

	return OK

def successmining( char, gem, table, resname, amount, ore ):
	socket = char.socket
	message = table[ resname ][ SUCCESSCLILOC ]
	backpack = char.getbackpack()
	if not backpack:
		return OOPS
	resore = "%s%s" % ( resname, ore )
	resourceitem = wolfpack.additem( str(resore) )
	resourceitem.decay = 1
	if (FELUCIA2XRESGAIN == TRUE) and (char.pos.map  == 0):
		resourceitem.amount = amount * 2
	else:
		resourceitem.amount = amount

	if not gem.hastag('resourcecount'):
		return OOPS

	gem.settag( 'resourcecount', ( gem.gettag( 'resourcecount' ) - amount ) )

	if gem.gettag( 'resourcecount' ) == 0:
		if not gem.hastag ( 'resource_empty' ) and gem.gettag( 'resourcecount' ) == 0:
			gem.settag( 'resource_empty', 'true' )
			wolfpack.addtimer( orerespawndelay, "skills.mining.respawnvein", [ gem ], 1 )

	if not wolfpack.utilities.tocontainer( resourceitem, backpack ):
		resourceitem.update()

	# You dig some %s and put it in your backpack.
	socket.clilocmessage( message, "", GRAY )
	return OK

def respawnvein( time, args ):
	vein = args[0]
	if vein.hastag( 'resource_empty' ) and vein.gettag( 'resourcecount' ) == 0:
		vein.settag( 'resourcecount', orespawnamount )
		vein.deltag( 'resource_empty' )
	return OK
