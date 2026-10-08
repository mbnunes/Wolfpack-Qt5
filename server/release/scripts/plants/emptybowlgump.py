
import wolfpack
import wolfpack.gumps
import plants.plant
import plants
import plants.bowl
import plants.seed
from consts import *
import potions.utilities
import potionkeg
import plants.maingump
from wolfpack import tr
import wolfpack.utilities

def addBackground(dialog):
	dialog.addResizeGump(50, 50, 0xe10, 200, 150)
	dialog.addTilePic(45, 45, 0xcef)
	dialog.addTilePic(45, 118, 0xcf0)
	dialog.addTilePic(211, 45, 0xceb)
	dialog.addTilePic(211, 118, 0xcec)

def addPicture(dialog, plant):
	dialog.addTilePic(90, 100, 0x1602)
	dialog.addGump(140, 102, 0x15e1)
	dialog.addTilePic(160, 100, 0x15fd)

	status = plants.plant.getStatus(plant)

	if status != STATUS_DIRT and status < STATUS_PLANT:
		dialog.addTilePic(156, 130, 0xdcf)

def send(player, plant):
	dialog = wolfpack.gumps.cGump(20, 20)

	addBackground(dialog)
	dialog.addText(90, 70, tr("Empty the bowl?"), 0x44)
	addPicture(dialog, plant)

	dialog.addButton(98, 150, 0x47e, 0x480, 1) # Cancel
	dialog.addButton(138, 151, 0xd2, 0xd2, 2) # Help
	dialog.addText(143, 151, '?', 0x835)
	dialog.addButton(168, 150, 0x481, 0x483, 3) # Ok

	dialog.setArgs([plant.serial])
	dialog.setCallback(response)
	dialog.send(player)

def response(player, arguments, response):
	plant = wolfpack.finditem(arguments[0])

	if response.button == 0 or not plant or plants.plant.getStatus(plant) >= STATUS_DECORATIVE or not player.canreach(plant, 3):
		return # Cancel

	if not plants.plant.checkAccess(player, plant):
		plant.say(1061856, '', '', False, 0x3b2, player.socket)
		return

	# Cancel
	if response.button == 1:
		plants.maingump.send(player, plant)

	# Help
	elif response.button == 2:
		send(player, plant)
		plants.sendCodexOfWisdom(player.socket, 71)

	# Ok
	elif response.button == 3:		
		bowl = wolfpack.additem(plants.bowl.BOWL_BASEID) # Create an empty bowl
		if not wolfpack.utilities.tobackpack(bowl, player):
			bowl.update()

		status = plants.plant.getStatus(plant)

		# Create a seed if we have one
		if status > STATUS_DIRT and status < STATUS_PLANT:
			seed = plants.seed.create(plants.plant.getGenus(plant), plants.plant.getHue(plant), plants.plant.getShowType(plant))
			if not wolfpack.utilities.tobackpack(seed, player):
				seed.update()

		plant.delete() # Delete the plant
