
import wolfpack
import wolfpack.gumps
import plants.plant
import plants
from consts import *
import potions.utilities
import potionkeg
import plants.reproductiongump
from wolfpack import tr

#
# Add the background to the gump
#
def addBackground(dialog):
	dialog.addResizeGump(50, 50, 0xe10, 200, 150)
	dialog.addTilePic(45, 45, 0xcef)
	dialog.addTilePic(45, 118, 0xcf0)
	dialog.addTilePic(211, 45, 0xceb)
	dialog.addTilePic(211, 118, 0xcec)

def send(player, plant):
	dialog = wolfpack.gumps.cGump(20, 20)
	
	addBackground(dialog)

	dialog.addText(115, 85, tr('Set plant'), 0x44)
	dialog.addText(82, 105, tr('to decorative mode'), 0x44)
	
	dialog.addButton(98, 140, 0x47e, 0x480, 1) # Cancel
	
	dialog.addButton(138, 141, 0xd2, 0xd2, 2) # Help
	dialog.addText(143, 141, '?', 0x835)
	
	dialog.addButton(168, 140, 0x481, 0x483, 3) # Ok

	dialog.setCallback(response)
	dialog.setArgs([plant.serial])
	dialog.send(player)
	
def response(player, arguments, response):
	plant = wolfpack.finditem(arguments[0])
	
	if response.button == 0 or not plant or plants.plant.getStatus(plant) != STATUS_STAGE9 or not player.canreach(plant, 3):
		return # Cancel
	
	if not plants.plant.checkAccess(player, plant):
		plant.say(1061856, '', '', False, 0x3b2, player.socket)
		return
	
	# Cancel	
	if response.button == 1:
		plants.reproductiongump.send(player, plant)
		
	# Help
	elif response.button == 2:
		send(player, plant)
		plants.sendCodexOfWisdom(player.socket, 70)		
	
	# Turn into a decorative plant
	elif response.button == 3:		
		plants.plant.setStatus(plant, STATUS_DECORATIVE)
		plant.say(1053077, '', '', False, 0x3b2, player.socket) # You prune the plant. This plant will no longer produce resources or seeds, but will require no upkeep.
