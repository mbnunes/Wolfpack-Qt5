#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  # Created by: codex
#   )).-' {{ ;'`   # Revised by: Dreoth
#  ( (  ;._ \\ ctr # Last Modification: Jan 26 2004
#################################################################

from wolfpack.consts import *
from wolfpack import console
import wolfpack
import skills
from wolfpack.time import *
from wolfpack.utilities import *
import random
from random import randint

#mining calling from pickaxe.py and shovel.py

#in seconds
miningdelay = 1000

# 0x19b7, 0x19b8, 0x19ba give 1 ingot.
# 0x19b9 gives 2 ingots.
oreids = [ 0x19b7, 0x19b8, 0x19ba, 0x19b9 ]
oredefs = [ '_ore_1', '_ore_2', '_ore_4', '_ore_3' ]

# Mining Harvest Table Items
REQSKILL = 0
MINSKILL = 1
SUCCESSCLILOC = 2
COLORID = 3
FINDCHANCE = 4

# resname, reqSkill, minSkill, successClilocId, color, find rate (will make a percentage of the total amount)
oretable = {
	'iron': 			[0, -250, 1007072, 0x0, 49],
	'dullcopper': [650, 325, 1007073, 0x973, 11],
	'shadowiron': [700, 350, 1007074, 0x966, 10],
	'copper': 		[750, 375, 1007075, 0x96d, 8],
	'bronze': 		[800, 400, 1007076, 0x972, 7],
	'gold': 			[850, 425, 1007077, 0x8a5, 6],
	'agapite': 		[900, 450, 1007078, 0x979, 4],
	'verite': 		[950, 475, 1007079, 0x89f, 3],
	'valorite': 	[990, 495, 1007080, 0x8ab, 2]
}

def mining( char, pos, tool ):
	wolfpack.addtimer( 1300, "skills.mining.domining", [ char, SOUND_MINING, tool, pos ] )
	char.socket.settag( 'is_mining', ( servertime() + miningdelay ) )
	char.turnto( pos )
	char.action( ANIM_ATTACK3 )
	return OK

def createoregem(pos):
	gem = wolfpack.additem('ore_gem')
	gem.settag('resourcecount', randint(MINING_MIN_ORE, MINING_MAX_ORE)) # 10 - 34 ore
	gem.settag('resname', 'iron') # All veins should default to iron ore.
	
	# This will give it a chance to be a random ore type, this can change later.	
	totalchance = 0
	for ore in oretable.values():
		totalchance += ore[FINDCHANCE]
	colorchance = randint(0, totalchance - 1)
	offset = 0

	for (resname, ore) in oretable.items():
		if colorchance >= offset and colorchance < offset + ore[FINDCHANCE]:
			gem.settag('resname2', resname)
			gem.color = ore[COLORID]
			break
		offset += ore[FINDCHANCE]

	gem.moveto(pos)
	gem.visible = 0
	gem.update()
	return gem

def getvein(socket, pos):
	# 4x4 resource grid
	gem_x = (pos.x / 4) * 4
	gem_y = (pos.y / 4) * 4
	
	gems = wolfpack.items(gem_x, gem_y, pos.map, 0)
	for gem in gems:
		if gem.hastag('resource') and gem.gettag('resource') == 'ore' and gem.hastag('resname'):
			return gem

	pos.x = gem_x
	pos.y = gem_y
	return createoregem(pos)

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
def domining(time, args):
	char = args[0]
	char.soundeffect(args[1])
	tool = args[2]
	pos = args[3]
	socket = char.socket
	socket.deltag('is_mining')

	# Recheck distance
	if not char.canreach(pos, MINING_MAX_DISTANCE):
		socket.clilocmessage(501867)
		return 0

	veingem = getvein(socket, pos)

	if not veingem or not veingem.hastag('resourcecount'):
		return 0

	# 50% chance to dig up primary resource, 
	# even if the vein has something else.
	if veingem.hastag('resname2') and random.random() >= 0.50:
		resname = veingem.gettag('resname2')
	else:
		resname = veingem.gettag('resname')

	resourcecount = veingem.gettag('resourcecount')
	reqskill = oretable[resname][REQSKILL]

	# Refill the resource gem.
	if resourcecount == 0:
		socket.sysmessage("There is no ore here to mine.")
		
		if not veingem.hastag('resource_empty'):
			duration = randint(MINING_MIN_REFILLTIME, MINING_MAX_REFILLTIME)
			wolfpack.addtimer(duration, "skills.mining.respawnvein", [veingem], 1)
			veingem.settag('resource_empty', 1)
		return 0
		
	# You loosen some rocks but fail to find any usable ore.
	if char.skill < reqskill:
		socket.clilocmessage(501869)
		return 0

	chance = max(0, char.skill[MINING] - oretable[resname][MINSKILL]) / 1000.0
	success = 0

	if not skills.checkskill(char, MINING, chance):
		socket.clilocmessage(501869)
		return 0	
	
	# Digs up the large ore.
	if resourcecount >= 5:
		successmining(char, veingem, oretable, resname, 1, oredefs[3])
	
	# Picks one of the smaller ore types
	elif resourcecount == 3 or resourcecount == 4:
		successmining(char, veingem, oretable, resname, 1, oredefs[randint( 1, 2 )])
	
	# Smallest ore only
	elif resourcecount == 1 or resourcecount == 2:
		successmining(char, veingem, oretable, resname, 1, oredefs[0])

	# Remaining Tool Uses
	if not tool.hastag('remaining_uses'):
		tool.settag('remaining_uses', tool.health)
	else:
		remaining_uses = int(tool.gettag('remaining_uses'))
		if remaining_uses > 1:
			tool.settag('remaining_uses', remaining_uses - 1)
			tool.resendtooltip()
		else:
			tool.delete()
			socket.clilocmessage(1044038) # You have worn out your tool!

	return 1

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

	if not gem.hastag('resource_empty') and gem.gettag('resourcecount') == 0:
		gem.settag('resource_empty', 1)
		wolfpack.addtimer(randint(MINING_MIN_REFILLTIME, MINING_MAX_REFILLTIME), "skills.mining.respawnvein", [gem], 1)

	if not wolfpack.utilities.tocontainer( resourceitem, backpack ):
		resourceitem.update()

	# You dig some %s and put it in your backpack.
	if type(message) == int:
		socket.clilocmessage( message, "", GRAY )
	else:
		socket.systemmessage(unicode(message))
	return OK

def respawnvein( time, args ):
	vein = args[0]
	if vein and vein.hastag('resource_empty') and vein.gettag('resourcecount') == 0:
		vein.settag('resourcecount', randint(MINING_MIN_ORE, MINING_MAX_ORE))
		vein.deltag('resource_empty')
	return OK
