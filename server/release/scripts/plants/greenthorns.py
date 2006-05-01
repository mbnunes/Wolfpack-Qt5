
import wolfpack
import wolfpack.time
import random
import system.lootlists
from wolfpack import tr

GREENTHORN_DELAY = 3 * 60 * 1000

DIRT_TILES = 	range(0x71, 0x7d) + range(0x82, 0xa8) + range(0xdc, 0xe5) + \
		range(0xe8, 0xec) + range(0x141, 0x145) + range(0x14c, 0x150) + \
		range(0x169, 0x175) + range(0x1dc, 0x1e8) + range(0x1ec, 0x1f0) + \
		range(0x272, 0x276) + range(0x27e, 0x282) + range(0x2d0, 0x2d8) + \
		range(0x2e5, 0x300) + range(0x303, 0x320) + range(0x32c, 0x330) + \
		range(0x33d, 0x341) + range(0x345, 0x34d) + range(0x355, 0x359) + \
		range(0x367, 0x36f) + range(0x377, 0x37b) + range(0x38d, 0x391) + \
		range(0x395, 0x39d) + range(0x3a5, 0x3a9) + range(0x3f6, 0x406) + \
		range(0x547, 0x54f) + range(0x553, 0x557) + range(0x597, 0x59f) + \
		range(0x623, 0x63b) + range(0x6f3, 0x6fb) + range(0x777, 0x792) + \
		range(0x79a, 0x7aa) + range(0x7ae, 0x7b2)

FURROWS_TILES = range(0x9, 0x16) + range(0x150, 0x15d)

SWAMP_TILES = 	range(0x9c4, 0x9ec) + range(0x3d65, 0x3d66) + range(0x3dc0, 0x3dda) + \
		range(0x3ddb, 0x3ddd) + range(0x3dde, 0x3ef1) + range(0x3ff6, 0x3ff7) + \
		range(0x3ffc, 0x3fff)

SNOW_TILES = 	range(0x10c, 0x110) + range(0x114, 0x118) + range(0x119, 0x11e) + \
		range(0x179, 0x18b) + range(0x385, 0x38d) + range(0x391, 0x395) + \
		range(0x39d, 0x3a5) + range(0x3a9, 0x3ad) + range(0x5bf, 0x5d7) + \
		range(0x5df, 0x5e3) + range(0x745, 0x749) + range(0x751, 0x759) + \
		range(0x75d, 0x761) + range(0x76d, 0x774)

SAND_TILES =	range(0x16, 0x3b) + range(0x44, 0x4c) + range(0x11e, 0x122) + \
		range(0x126, 0x12e) + range(0x192, 0x193) + range(0x1a8, 0x1ac) + \
		range(0x1b9, 0x1d2) + range(0x282, 0x286) + range(0x28a, 0x292) + \
		range(0x335, 0x33d) + range(0x341, 0x345) + range(0x34d, 0x355) + \
		range(0x359, 0x35d) + range(0x3b7, 0x3bf) + range(0x3c7, 0x3cb) + \
		range(0x5a7, 0x5b3) + range(0x64b, 0x653) + range(0x657, 0x65b) + \
		range(0x663, 0x66b) + range(0x66f, 0x673) + range(0x7bd, 0x7d1)

def checkUse(player):
	if player.gm:
		return True

	if player.socket and player.socket.hastag('greenthorn_delay'):
		delay = int(player.socket.gettag('greenthorn_delay'))

		if wolfpack.time.currenttime() < delay:
			return False

	return True

def onUse(player, item):
	if item.getoutmostitem() != player.getbackpack():
		player.socket.clilocmessage(1042038) # You must have the object in your backpack to use it.
		return True

	if not checkUse(player):
		player.message(1061908) # * You must wait a while before planting another thorn. *
		return True

	player.socket.clilocmessage(1061906)
	player.socket.attachtarget('plants.greenthorns.target', [item.serial])
	return True

def target(player, arguments, target):
	# Check the green thorn first
	item = wolfpack.finditem(arguments[0])

	# Check if we have the item in our backpack
	if not item or item.getoutmostitem() != player.getbackpack():
		player.socket.clilocmessage(1042038) # You must have the object in your backpack to use it.
		return

	# Check the green thorn delay
	if not checkUse(player):
		player.message(1061908) # * You must wait a while before planting another thorn. *
		return

	# This check should only happen for solen hives
	if player.pos.map not in [0, 1]:
		player.message(tr("No solen lairs exist on this facet.  Try again in Trammel or Felucca."), 0x2b2)
		return

	# Can we reach the target location?
	if not player.canreach(target.pos, 3):
		player.message(502825) # That location is too far away
		return

	# Get the land-tile id at the given location
	landid = wolfpack.map(target.pos.x, target.pos.y, player.pos.map)['id']

	callback = None # Callback for the green thorns effect

	# Check which effect should be used for the landtile type
	if landid in DIRT_TILES:
		callback = dirtCallback

	elif landid in FURROWS_TILES:
		callback = furrowsCallback

	elif landid in SWAMP_TILES:
		callback = swampCallback

	elif landid in SNOW_TILES:
		callback = snowCallback

	elif landid in SAND_TILES:
		callback = sandCallback

	if not callback:
		player.message(1061913)
	else:
		if item.amount <= 1:
			item.delete()
		else:
			item.amount -= 1
			item.update()

		# Show a message that we're planting a thorn
		player.socket.clilocmessage(1061914, "", 0x961, 3, player, "", False, True) # * You push the strange green thorn into the ground *
		listeners = wolfpack.chars(player.pos.x, player.pos.y, player.pos.map, 18)

		for listener in listeners:
			if listener != player and listener.socket:
				listener.socket.clilocmessage(1061915, player.name, 0x961, 3, player, "", False, True) # * ~1_PLAYER_NAME~ pushes a strange green thorn into the ground. *

		# Set the next use delay for this kind of item
		player.socket.settag('greenthorn_delay', wolfpack.time.currenttime() + GREENTHORN_DELAY)

		# Start the effect
		player.addtimer(2500, callback, [target.pos, 0])

#
# Delete Item Callback
#
def deleteCallback(obj, args):
	item = wolfpack.finditem(args[0])

	if item and not item.container:
		item.delete()

#
# Move an item to a safe location at the given spot
#
def spawnItem(pos, item):
	# Try it 5 times
	for i in range(0, 5):
		x = pos.x + random.randint(-1, 1)
		y = pos.y + random.randint(-1, 1)
		itempos = wolfpack.coord(x, y, pos.z, pos.map)
		
		if not itempos.validspawnspot():
			itempos.z = 127
			if not itempos.validspawnspot():
				continue

		item.moveto(itempos)
		return True

	return False

#
# Move a npc to a safe location at the given spot
#
def spawnNpc(pos, npc):
	# Try it 5 times
	for i in range(0, 5):
		x = pos.x + random.randint(-1, 1)
		y = pos.y + random.randint(-1, 1)
		npcpos = wolfpack.coord(x, y, pos.z, pos.map)

		if not npcpos.validspawnspot():
			npcpos.z = 127
			if not npcpos.validspawnspot():
				continue

		npc.moveto(npcpos)
		return True

	return False	

#
# Spawn reagents at the given position
#
def spawnReagents(pos):
	amount = random.randint(10, 25)
	baseid = random.choice(system.lootlists.DEF_PLAINREGS + ['f81']) # f81 is fertile dirt

	item = wolfpack.additem(baseid)
	item.amount = amount
	if not spawnItem(pos, item):
		item.delete()
	else:
		item.update()

#
# Callback for dirt
#
def dirtCallback(player, args):
	(pos, stroke) = args

	if stroke == 0:
		pos.soundeffect(0x106)
		pos.effect(0x3735, 1, 182)
		player.addtimer(4000, dirtCallback, [pos, 1])

	elif stroke == 1:
		pos.soundeffect(0x222)
		player.addtimer(4000, dirtCallback, [pos, 2])

	elif stroke == 2:
		pos.soundeffect(0x21f)
		player.addtimer(5000, dirtCallback, [pos, 3])

	elif stroke == 3:
		dummy = wolfpack.additem('1')
		dummy.name = 'Ground'
		dummy.moveto(pos)
		dummy.update()
		dummy.say(tr("* The ground erupts with chaotic growth! *"))
		wolfpack.addtimer(20000, deleteCallback, [dummy.serial], True)

		pos.soundeffect(0x12d)

		spawnReagents(pos)
		spawnReagents(pos)
		player.addtimer(2000, dirtCallback, [pos, 4])

	elif stroke == 4:
		pos.soundeffect(0x12d)

		spawnReagents(pos)
		spawnReagents(pos)

		player.addtimer(2000, dirtCallback, [pos, 5])

	elif stroke == 5:
		pos.soundeffect(0x12d)

		spawnReagents(pos)
		spawnReagents(pos)

		player.addtimer(3000, dirtCallback, [pos, 6])

	elif stroke == 6:
		pos.soundeffect(0x12d)

		spawnReagents(pos)
		spawnReagents(pos)

#
# Callback for furrows
#
def furrowsCallback(player, args):
	(pos, stroke) = args

	if stroke == 0:
		pos.soundeffect(0x106)
		pos.effect(0x3735, 1, 182)
		player.addtimer(4000, furrowsCallback, [pos, 1])

	elif stroke == 1:
		pos.soundeffect(0x222)
		player.addtimer(4000, furrowsCallback, [pos, 2])

	elif stroke == 2:
		pos.soundeffect(0x21f)
		player.addtimer(4000, furrowsCallback, [pos, 3])

	elif stroke == 3:
		dummy = wolfpack.additem('1')
		dummy.name = 'Swamp'
		dummy.moveto(pos)
		dummy.update()
		dummy.say(tr("* A magical bunny leaps out of its hole, disturbed by the thorn's effect! *"))
		wolfpack.addtimer(20000, deleteCallback, [dummy.serial], True)

		# Spawn one giant ice worm and three ice snakes
		npc = wolfpack.addnpc('vorpal_bunny', pos)
		if not spawnNpc(pos, npc):
			npc.delete()
		else:
			npc.update()

			# Start timer to dig away
			npc.addtimer(3 * 60 * 1000, vorpalbunny_dig, [], True)

#
# Start digging hole
#		
def vorpalbunny_dig(npc, args):
	if npc.pos.map == 0xFF:
		return

	# Spawn a bunny hole
	hole = wolfpack.additem('913')
	hole.decay = True
	hole.movable = 3
	hole.color = 1
	hole.name = tr('a mysterious rabbit hole')
	hole.moveto(npc.pos)
	hole.update()
	wolfpack.addtimer(40000, deleteCallback, [hole.serial], True) # Delete after 40 seconds

	npc.say(tr("* The bunny begins to dig a tunnel back to its underground lair *"))
	npc.frozen = True
	npc.soundeffect(0x247)

	# Delete in 5 seconds
	npc.addtimer(5000, deleteNpc, [], True)

#
# Delete a npc
#
def deleteNpc(npc, args):
	npc.delete()

#
# Callback for swamp
#
def swampCallback(player, args):
	(pos, stroke) = args

	if stroke == 0:
		pos.soundeffect(0x106)
		pos.effect(0x3735, 1, 182)
		player.addtimer(4000, swampCallback, [pos, 1])

	elif stroke == 1:
		pos.soundeffect(0x222)
		player.addtimer(4000, swampCallback, [pos, 2])

	elif stroke == 2:
		pos.soundeffect(0x21f)
		player.addtimer(1000, swampCallback, [pos, 3])

	elif stroke == 3:
		dummy = wolfpack.additem('1')
		dummy.name = 'Swamp'
		dummy.moveto(pos)
		dummy.update()
		dummy.say(tr("* Strange green tendrils rise from the ground, whipping wildly! *"))
		wolfpack.addtimer(20000, deleteCallback, [dummy.serial], True)

		pos.soundeffect(0x2b0)

		# Spawn one giant ice worm and three ice snakes
		npc = wolfpack.addnpc('whipping_vine', pos)
		if not spawnNpc(pos, npc):
			npc.delete()
		else:
			npc.update()

#
# Callback for snow
#
def snowCallback(player, args):
	(pos, stroke) = args

	if stroke == 0:
		pos.soundeffect(0x106)
		pos.effect(0x3735, 1, 182)
		player.addtimer(4000, snowCallback, [pos, 1])

	elif stroke == 1:
		pos.soundeffect(0x222)
		player.addtimer(4000, snowCallback, [pos, 2])

	elif stroke == 2:
		pos.soundeffect(0x21f)
		player.addtimer(4000, snowCallback, [pos, 3])

	elif stroke == 3:
		dummy = wolfpack.additem('1')
		dummy.name = 'Snow'
		dummy.moveto(pos)
		dummy.update()
		dummy.say(tr("* Slithering ice serpents rise to the surface to investigate the disturbance! *"))
		wolfpack.addtimer(20000, deleteCallback, [dummy.serial], True)

		# Spawn one giant ice worm and three ice snakes
		npc = wolfpack.addnpc('giant_ice_worm', pos)
		if not spawnNpc(pos, npc):
			npc.delete()
		else:
			npc.update()

		for i in range(0, 3):
			npc = wolfpack.addnpc('ice_snake', pos)
			if not spawnNpc(pos, npc):
				npc.delete()
			else:
				npc.update()			

#
# Callback for sand
#
def sandCallback(player, args):
	(pos, stroke) = args

	#
	# TODO: The solen stuff isn't done so i'm leaving this out
	#

	#player.message('SAND')
