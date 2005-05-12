#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  # Created by: codex
#   )).-' {{ ;'`   # Revised by: Dreoth
#  ( (  ;._ \\ ctr # Last Modification: Jan 26 2004
#################################################################

import wolfpack
import wolfpack.time
import skills
import random
from wolfpack.consts import MINING, GRAY, MINING_REFILLTIME, MINING_ORE, \
	MINING_MAX_DISTANCE, ANIM_ATTACK3, FELUCIA2XRESGAIN, MINING_SAND
from wolfpack import console
from wolfpack.utilities import ismountainorcave, issand, tobackpack


#mining calling from pickaxe.py and shovel.py

#in seconds
miningdelay = 1000

# Mining Harvest Table Items
REQSKILL = 0
MINSKILL = 1
SUCCESSMESSAGE = 2
COLORID = 3
FINDCHANCE = 4
ORENAME = 5
INGOTNAME = 6

# resname, reqSkill, minSkill, sucessmessage, color, find rate (will make a percentage of the total amount),
# name of ore, name of ingot
ORES = {
	'iron': [0, -250, 1007072, 0x0, 49, '#1042853', '#1042692'],
	'dullcopper': [650, 325, 1007073, 0x973, 11, '#1042845', '#1042684'],
	'shadowiron': [700, 350, 1007074, 0x966, 10, '#1042846', '#1042685'],
	'copper': [750, 375, 1007075, 0x96d, 8, '#1042847', '#1042686'],
	'bronze': [800, 400, 1007076, 0x972, 7, '#1042848', '#1042687'],
	'gold': [850, 425, 1007077, 0x8a5, 6, '#1042849', '#1042688'],
	'agapite': [900, 450, 1007078, 0x979, 4, '#1042850', '#1042689'],
	'verite': [950, 475, 1007079, 0x89f, 3, '#1042851', '#1042690'],
	'valorite': [990, 495, 1007080, 0x8ab, 2, '#1042852', '#1042691']
}

def canminesand(char):
	if char.hastag('sandmining'):
		return True
	return False

def mining( char, pos, tool, sand = False ):
	if not tool:
		return False
	if sand == True:
		char.addtimer( 1300, dosandmining, [ tool.serial, pos ] )
	else:
		char.addtimer( 1300, domining, [ tool.serial, pos ] )
	char.socket.settag( 'is_mining', ( wolfpack.time.currenttime() + miningdelay ) )
	char.turnto( pos )
	char.action( ANIM_ATTACK3 )
	return True

def createoregem(pos):
	gem = wolfpack.additem('ore_gem')
	gem.settag('resourcecount', random.randint(MINING_ORE[0], MINING_ORE[1])) # 10 - 34 ore
	gem.settag('resname', 'iron') # All veins should default to iron ore.

	# This will give it a chance to be a random ore type, this can change later.
	totalchance = 0
	for ore in ORES.values():
		totalchance += ore[FINDCHANCE]
	colorchance = random.randint(0, totalchance - 1)
	offset = 0
	maxreqskill = 0

	for (resname, ore) in ORES.items():
		if colorchance >= offset and colorchance < offset + ore[FINDCHANCE]:
			gem.settag('resname2', resname)
			maxreqskill = ore[REQSKILL]
			gem.color = ore[COLORID]
			break
		offset += ore[FINDCHANCE]

	# Find the ore which has the maximum req skill less than ours
	resname = 'iron'
	reqskill = 0

	#for (oreresname, ore) in ORES.items():
	#	if ore[REQSKILL] > reqskill and ore[REQSKILL] < maxreqskill:
	#		reqskill = ore[REQSKILL]
	#		resname = oreresname

	#gem.settag('resname2', resname)

	gem.name = 'Ore Gem (%s,%s)' % (resname, gem.gettag('resname2'))
	gem.moveto(pos)
	gem.visible = 0
	gem.update()
	return gem

def createsandgem(pos):
	gem = wolfpack.additem('sand_gem')
	gem.settag('resourcecount', random.randint(MINING_SAND[0], MINING_SAND[1])) # 6 - 13 ore
	gem.moveto(pos)
	gem.visible = 0
	gem.update()
	return gem

def getvein(socket, pos):
	# 8x8 resource grid
	gem_x = (pos.x / 8) * 8
	gem_y = (pos.y / 8) * 8

	gems = wolfpack.items(gem_x, gem_y, pos.map, 0)
	for gem in gems:
		if gem.hastag('resource') and gem.gettag('resource') == 'ore' and gem.hastag('resname'):
			return gem

	pos.x = gem_x
	pos.y = gem_y
	return createoregem(pos)

def getsandvein(socket, pos):
	# 8x8 resource grid
	gem_x = (pos.x / 8) * 8
	gem_y = (pos.y / 8) * 8

	gems = wolfpack.items(gem_x, gem_y, pos.map, 0)
	for gem in gems:
		if gem.hastag('resource') and gem.gettag('resource') == 'sand':
			return gem

	pos.x = gem_x
	pos.y = gem_y
	return createsandgem(pos)

#Response from mining tool
def response( char, args, target ):
	socket = char.socket
	if not socket:
		return False

	pos = target.pos

	# Player can reach that ?
	if char.pos.map != pos.map or char.pos.distance( pos ) > MINING_MAX_DISTANCE:
		# That is too far away
		socket.clilocmessage( 500446, "", GRAY )
		return True

	tool = wolfpack.finditem(args[0])

	#Player also can't mine when riding, polymorphed and dead.
	#Mine char ?!
	if target.char:
		# You can't mine that.
		socket.clilocmessage( 501863, "", GRAY )
		return True

	#Find tile by it's position if we haven't model
	elif target.model == 0:
		map = wolfpack.map( target.pos.x, target.pos.y, target.pos.map )
		if ismountainorcave( map['id'] ):
			mining( char, target.pos, tool, sand = False )
		elif issand( map['id'] ) and canminesand(char):
			mining( char, target.pos, tool, sand = True )
		else:
			# You can't mine there.
			socket.clilocmessage( 501862, "", GRAY )
		return True

	#Find tile by it's model
	elif target.model != 0:
		if ismountainorcave( target.model ):
			#add new ore gem here and mine
			mining( char, target.pos, tool )
		else:
			socket.clilocmessage( 501862, "", GRAY ) # You can't mine there.
		return True

	else:
		return False

	return True

#Sound effect
def domining(char, args):
	char.soundeffect( random.choice([0x125, 0x126]) )
	tool = wolfpack.finditem(args[0])
	pos = args[1]
	socket = char.socket
	socket.deltag( 'is_mining' )

	if not tool:
		return False

	# Recheck distance
	#if not char.canreach(pos, MINING_MAX_DISTANCE):
	#	socket.clilocmessage(501867)
	#	return False

	veingem = getvein(socket, pos)

	if not veingem or not veingem.hastag('resourcecount'):
		return False

	# 50% chance to dig up primary resource,
	# even if the vein has something else.
	if veingem.hastag('resname2') and random.random() >= 0.50:
		resname = veingem.gettag('resname2')
	else:
		resname = veingem.gettag('resname')

	resourcecount = veingem.gettag('resourcecount')
	reqskill = ORES[resname][REQSKILL]

	# Refill the resource gem.
	if resourcecount == 0:
		socket.sysmessage("There is no ore here to mine.")

		if not veingem.hastag('resource_empty'):
			duration = random.randint(MINING_REFILLTIME[0], MINING_REFILLTIME[1])
			veingem.addtimer( duration, respawnvein, [], True )
			veingem.settag('resource_empty', 1)
		return False

	# You loosen some rocks but fail to find any usable ore.
	if char.skill < reqskill:
		socket.clilocmessage(501869)
		return False

	chance = max(0, char.skill[MINING] - ORES[resname][MINSKILL]) / 1000.0
	success = 0

	if not skills.checkskill(char, MINING, chance):
		socket.clilocmessage(501869)
		return False

	# Digs up the large ore.
	if resourcecount >= 5:
		successmining(char, veingem, resname, 3)

	# Picks one of the smaller ore types
	elif resourcecount == 3 or resourcecount == 4:
		successmining(char, veingem, resname, 2)

	# Smallest ore only
	elif resourcecount == 1 or resourcecount == 2:
		successmining(char, veingem, resname, 1)
	wearout(char, tool)

def wearout(char, tool):
	# Remaining Tool Uses
	if not tool.hastag('remaining_uses'):
		tool.settag('remaining_uses', tool.health )
	else:
		remaining_uses = int(tool.gettag('remaining_uses'))
		if remaining_uses > 1:
			tool.settag('remaining_uses', remaining_uses - 1 )
			tool.resendtooltip()
		else:
			tool.delete()
			char.socket.clilocmessage(1044038) # You have worn out your tool!
	return True

#Sound effect
def dosandmining(char, args):
	char.soundeffect( random.choice([0x125, 0x126]))
	tool = wolfpack.finditem(args[0])
	pos = args[1]
	socket = char.socket
	socket.deltag('is_mining')

	if not tool:
		return False

	veingem = getsandvein(socket, pos)

	if not veingem or not veingem.hastag('resourcecount'):
		return False

	# 9.04% chance to dig up sand
	chance = random.random()
	success = 0
	if chance <= 0.0904:
		success = 1

	resourcecount = veingem.gettag('resourcecount')

	# Refill the resource gem.
	if resourcecount == 0:
		char.socket.clilocmessage(1044629)

		if not veingem.hastag('resource_empty'):
			duration = random.randint(MINING_REFILLTIME[0], MINING_REFILLTIME[1])
			veingem.addtimer( duration, respawnvein, [], True )
			veingem.settag('resource_empty', 1)
		return False

	if success:
		successsandmining(char, veingem)
	else:
		char.socket.clilocmessage(1044630)
	wearout(char, tool)

def successsandmining(char, gem):
	sand = wolfpack.additem("sand")
	if not tobackpack(sand, char):
		sand.update()

	# Resend weight
	char.socket.resendstatus()

	resourcecount = max( 1, int( gem.gettag('resourcecount') ) )
	gem.settag('resourcecount', resourcecount - 1)

	# Start respawning the sand
	if not gem.hastag('resource_empty') and resourcecount <= 1:
		delay = random.randint(MINING_REFILLTIME[0], MINING_REFILLTIME[1])
		gem.addtimer( delay, respawnvein, [], True )
		gem.settag( 'resource_empty', 1 )

	char.socket.clilocmessage(1044631) # You carefully dig up sand of sufficient quality for glassblowing.

def successmining(char, gem, resname, size):
	if not char:
		return False

	if skills.skilltable[ MINING ][ skills.UNHIDE ] and char.hidden:
		char.removefromview()
		char.hidden = False
		char.update()

	# Create the ore and put it into the players backpack
	if size == 1:
		item = wolfpack.additem("19b7")
	elif size == 2:
		item = wolfpack.additem(random.choice(["19b8", "19ba"]))
	elif size == 3:
		item = wolfpack.additem("19b9")
	else:
		raise RuntimeException, "Invalid ore size: %u" % size

	item.settag('resname', resname)
	item.color = ORES[resname][COLORID]

	if FELUCIA2XRESGAIN and char.pos.map == 0:
		item.amount = 2
	else:
		item.amount = 1

	if not tobackpack(item, char):
		item.update()

	# Resend weight
	char.socket.resendstatus()

	resourcecount = max( 1, int( gem.gettag('resourcecount') ) )
	gem.settag('resourcecount', resourcecount - 1)

	# Start respawning the ore
	if not gem.hastag('resource_empty') and resourcecount <= 1:
		delay = random.randint(MINING_REFILLTIME[0], MINING_REFILLTIME[1])
		gem.addtimer( delay, respawnvein, [], True )
		gem.settag( 'resource_empty', 1 )

	message = ORES[resname][SUCCESSMESSAGE]
	# You dig some %s and put it in your backpack.
	if type( message ) == int:
		char.socket.clilocmessage( message, "", GRAY )
	else:
		char.socket.sysmessage( unicode( message ) )
	return True

def respawnvein( vein, args ):
	if vein and vein.hastag('resource_empty') and vein.gettag('resourcecount') == 0:
		vein.settag('resourcecount', random.randint( MINING_ORE[0], MINING_ORE[1] ) )
		vein.deltag('resource_empty')
	return True
