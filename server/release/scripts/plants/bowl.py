
#
# An empty plant bowl
# It has to be filled with dirt to start growing a plant.
#

import wolfpack
import plants

BOWL_BASEID = 'plantbowl' # Base ID of the plant bowl.
FERTILEDIRT_BASEID = 'f81' # Baseid of fertile dirt.
PLANT_BASEID = 'plant' # The id of the plant thats created.

# List of item-ids for static dirt patches
STATIC_DIRT = range(0x1b27, 0x1b3f) + range(0x31f4, 0x31fb) + range(0x32c9, 0x32ca)

# List of tile ids for map dirt
MAP_DIRT = range(0x9, 0x16) + range(0x71, 0x7d) + range(0x82, 0xa8) + range(0xdc, 0xe4) + \
			range(0xe8, 0xec) + range(0x141, 0x145) + range(0x14c, 0x15d) + range(0x169, 0x175) + \
			range(0x1dc, 0x1f0) + range(0x272, 0x276) + range(0x27e, 0x282) + range(0x2d0, 0x2d8) + \
			range(0x2e5, 0x300) + range(0x303, 0x320) + range(0x32c, 0x330) + range(0x33d, 0x341) + \
			range(0x345, 0x34d) + range(0x355, 0x359) + range(0x367, 0x36f) + range(0x377, 0x37b) + \
			range(0x38d, 0x391) + range(0x395, 0x39d) + range(0x3a5, 0x3a9) + range(0x3f6, 0x406) + \
			range(0x547, 0x54f) + range(0x553, 0x557) + range(0x597, 0x59f) + range(0x623, 0x63b) + \
			range(0x6f3, 0x6fb) + range(0x777, 0x792) + range(0x79a, 0x7aa) + range(0x7ae, 0x7b2) + \
			range(0x98c, 0x9a0) + range(0x9ac, 0x9c0)

#
# Make a plant out of the bowl.
# Fertile indicates if fertile dirt was used.
# Returns True on success, False otherwise.
#
def makePlant(player, bowl, fertile = False):	
	player.socket.clilocmessage(1061895) # You fill the bowl with fresh dirt.
	bowl.delete()
	plant = wolfpack.additem(PLANT_BASEID)
	
	plants.plant.setFertile(plant, fertile) # Set the fertile status
	plants.plant.updatePlant(plant) # Update the plant
	
	if not wolfpack.utilities.tobackpack(plant, player):
		plant.update()	
		
	return True

def target(player, arguments, target):
	bowl = wolfpack.finditem(arguments[0]) # Find the bowl that has been used
	
	# The bowl still has to exist and be a bowl
	if not bowl or bowl.baseid != BOWL_BASEID:
		return
	
	# Still has to be in our backpack
	if bowl.getoutmostitem() != player.getbackpack():
		player.socket.clilocmessage(1042664) # You must have the object in your backpack to use it.
		return
		
	# Check if we can reach the target (3 tiles)
	if not player.canreach(target, 3):
		player.socket.clilocmessage(502825) # That location is too far away
		return

	# Check if its an item and fertile dirt
	if target.item and target.item.baseid == FERTILEDIRT_BASEID:
		# Has to be in the backpack to use it
		if target.item.getoutmostitem() != player.getbackpack():
			player.socket.clilocmessage(1042664) # You must have the object in your backpack to use it.
			
		# Need at least 40 fertile dirt to fill bowl
		elif target.item.amount < 40:
			player.socket.clilocmessage(1061896) # You need more dirt to fill a plant bowl!
		
		# If we can create the bowl, consume the dirt
		elif makePlant(player, bowl, True):
			# Consume 40 dirt
			if target.item.amount == 40:
				target.item.delete()
			else:
				target.item.amount -= 40
				target.item.update()		

	# Check if we targetted a dynamic/static dirt patch
	elif target.item and target.item.id in STATIC_DIRT and not player.canpickup(target.item):
		makePlant(player, bowl)
				
	# Assume the client sent valid data (this is BAD), the core should check if a static target is correct. MUCH faster.
	# This is a static tile
	elif not target.item and not target.char and target.model in STATIC_DIRT:
		makePlant(player, bowl)
		
	# Otherwise get the map at the given position
	elif not target.item and wolfpack.map(target.pos.x, target.pos.y, target.pos.map)['id'] in MAP_DIRT:
		makePlant(player, bowl)
		
	else:
		player.socket.clilocmessage(1061893) # You'll want to gather fresh dirt in order to raise a healthy plant!

def onUse(player, bowl):
	if bowl.getoutmostitem() != player.getbackpack():
		player.socket.clilocmessage(1042664) # You must have the object in your backpack to use it.
	else:
		player.socket.clilocmessage(1061897) # Choose a patch of dirt to scoop up.
		player.socket.attachtarget('plants.bowl.target', [bowl.serial])

	return True
