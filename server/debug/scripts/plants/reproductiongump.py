
from wolfpack import tr
import wolfpack
import wolfpack.gumps
import plants.plant
import plants.resources
import plants.seed
import plants
from consts import *
import potions.utilities
import potionkeg
import plants.maingump
import plants.makedecorativegump

# Baseid of plants for pollinating
PLANT_BASEID = 'plant'

#
# Add the background to the gump
#
def addBackground(dialog):
	dialog.addResizeGump(50, 50, 0xe10, 200, 150)
	dialog.addTilePic(45, 45, 0xcef)
	dialog.addTilePic(45, 118, 0xcf0)
	dialog.addTilePic(211, 45, 0xceb)
	dialog.addTilePic(211, 118, 0xcec)

	dialog.addGump(60, 90, 0xe17)
	dialog.addGump(120, 90, 0xe17)
	dialog.addGump(60, 145, 0xe17)
	dialog.addGump(120, 145, 0xe17)	

#
# Add different state information
#
def addPollinationState(dialog, x, y, plant):
	genus = plants.plant.getGenus(plant)
	hue = plants.plant.getHue(plant)
	status = plants.plant.getStatus(plant)

	if status < STATUS_FULLGROWN or not genus.crossable or not hue.crossable:
		dialog.addText(x, y, '-', 0x35)
	elif not plants.plant.getPollinated(plant):
		dialog.addText(x, y, '!', 0x35)
	else:
		dialog.addText(x, y, '+', 0x3f)

def addResourcesState(dialog, x, y, plant):
	available = plants.plant.getAvailableResources(plant)
	remaining = plants.plant.getRemainingResources(plant)
	genus = plants.plant.getGenus(plant)
	hue = plants.plant.getHue(plant)

	if (available == 0 and remaining == 0) or not plants.resources.canProduce(genus, hue):
		dialog.addText(x + 5, y, 'X', 0x21)
	else:
		dialog.addText(x, y, "%u/%u" % (available, available + remaining), hue.gumpcolor)

def addSeedsState(dialog, x, y, plant):
	available = plants.plant.getAvailableSeeds(plant)
	remaining = plants.plant.getRemainingSeeds(plant)
	genus = plants.plant.getGenus(plant)
	hue = plants.plant.getHue(plant)

	if (available == 0 and remaining == 0) or (not genus.crossable and not hue.crossable):
		dialog.addText(x + 5, y, 'X', 0x21)
	else:
		dialog.addText(x, y, "%u/%u" % (available, available + remaining), hue.gumpcolor)

#
# Send a reproduction gump for the plant to the player
#
def send(player, plant):
	dialog = wolfpack.gumps.cGump(20, 20)

	addBackground(dialog)

	dialog.addButton(70, 67, 0xd4, 0xd4, 1) # Main Menu
	dialog.addTilePic(57, 65, 0x1600)

	dialog.addText(108, 67, tr('Reproduction'), 0x835)

	status = plants.plant.getStatus(plant)

	# If it's in the last stage of development, we can turn it
	# into a decorative plant
	if status == STATUS_STAGE9:
		dialog.addButton(212, 67, 0xd4, 0xd4, 2) # Set to decorative
		dialog.addTilePic(202, 68, 0xc61)
		dialog.addText(216, 66, '/', 0x21)

	dialog.addButton(80, 116, 0xd4, 0xd4, 3) # Pollination
	dialog.addTilePic(66, 117, 0x1aa2)
	addPollinationState(dialog, 106, 116, plant)

	dialog.addButton(128, 116, 0xd4, 0xd4, 4) # Resources
	dialog.addTilePic(113, 120, 0x1021)
	addResourcesState(dialog, 149, 116, plant)

	dialog.addButton(177, 116, 0xd4, 0xd4, 5) # Seeds
	dialog.addTilePic(160, 121, 0xdcf)
	addSeedsState(dialog, 199, 116, plant)	

	dialog.addButton(70, 163, 0xd2, 0xd2, 6) # Gather pollen
	dialog.addTilePic(56, 164, 0x1aa2)

	dialog.addButton(138, 163, 0xd2, 0xd2, 7) # Gather resources
	dialog.addTilePic(123, 167, 0x1021)

	dialog.addButton(212, 163, 0xd2, 0xd2, 8) # Gather seeds
	dialog.addTilePic(195, 168, 0xdcf)

	dialog.setCallback(response)
	dialog.setArgs([plant.serial])
	dialog.send(player)

#
# Gump response
#
def response(player, arguments, response):
	plant = wolfpack.finditem(arguments[0])

	if response.button == 0 or not plant or plants.plant.getStatus(plant) >= STATUS_DECORATIVE or not player.canreach(plant, 3):
		return # Cancel

	if not plants.plant.checkAccess(player, plant):
		plant.say(1061856, '', '', False, 0x3b2, player.socket)
		return

	# Back to maingump
	if response.button == 1:
		plants.maingump.send(player, plant)

	# Set to decorative mode
	elif response.button == 2:
		status = plants.plant.getStatus(plant)
		if status == STATUS_STAGE9:
			plants.makedecorativegump.send(player, plant)

	# Pollination Help
	elif response.button == 3:
		send(player, plant)
		plants.sendCodexOfWisdom(player.socket, 67)

	# Resources Help
	elif response.button == 4:
		send(player, plant)
		plants.sendCodexOfWisdom(player.socket, 69)

	# Seed Help
	elif response.button == 5:
		send(player, plant)
		plants.sendCodexOfWisdom(player.socket, 68)

	# Gather Pollen
	elif response.button == 6:
		genus = plants.plant.getGenus(plant)
		hue = plants.plant.getHue(plant)
		status = plants.plant.getStatus(plant)

		# Not crossable -> no pollen
		if not genus.crossable or not hue.crossable:
			plant.say(1053050, '', '', False, 0x3b2, player.socket) # You cannot gather pollen from a mutated plant!

		# Too early to gather pollen
		elif status < STATUS_FULLGROWN:
			plant.say(1053051, '', '', False, 0x3b2, player.socket) # You cannot gather pollen from a plant in this stage of development!

		# Not healthy enough
		elif plants.plant.getHealthStatus(plant) in [HEALTH_WILTED, HEALTH_DYING]:
			plant.say(1053052, '', '', False, 0x3b2, player.socket) # You cannot gather pollen from an unhealthy plant!

		# Show a target to pollinate another plant
		else:
			plant.say(1053054, '', '', False, 0x3b2, player.socket) # Target the plant you wish to cross-pollinate to.
			player.socket.attachtarget('plants.reproductiongump.pollinate_target', [plant.serial])

		send(player, plant) # Resend our gump

	# Gather resources
	elif response.button == 7:
		available = plants.plant.getAvailableResources(plant)
		genus = plants.plant.getGenus(plant)
		hue = plants.plant.getHue(plant)

		# The plant doesn't produce resources
		if not plants.resources.canProduce(genus, hue):
			if not genus.crossable or not hue.crossable:
				plant.say(1053055, '', '', False, 0x3b2, player.socket) # Mutated plants do not produce resources!
			else:
				plant.say(1053056, '', '', False, 0x3b2, player.socket) # This plant has no resources to gather!

		# Nothing available at the moment
		elif available == 0:
			plant.say(1053056, '', '', False, 0x3b2, player.socket) # This plant has no resources to gather!

		# Create resources
		else:
			res = plants.resources.create(genus, hue)
			if not wolfpack.utilities.tobackpack(res, player):
				res.update()

			plants.plant.setAvailableResources(plant, available - 1) # Reduce available resources
			plant.say(1053059, '', '', False, 0x3b2, player.socket) # You gather resources from the plant.

		send(player, plant)

	# Gather Seeds
	elif response.button == 8:
		available = plants.plant.getAvailableSeeds(plant)
		genus = plants.plant.getGenus(plant)
		hue = plants.plant.getHue(plant)

		# The plant doesn't produce seeds
		if not genus.crossable or not hue.crossable:
			plant.say(1053060, '', '', False, 0x3b2, player.socket) # Mutated plants do not produce seeds!

		# Nothing available at the moment
		elif available == 0:
			plant.say(1053061, '', '', False, 0x3b2, player.socket) # This plant has no seeds to gather!

		# Create seeds
		else:
			seed = plants.seed.create(plants.plant.getSeedGenus(plant), plants.plant.getSeedHue(plant), True)
			if not wolfpack.utilities.tobackpack(seed, player):
				seed.update()

			plants.plant.setAvailableSeeds(plant, available - 1) # Reduce available seeds.
			plant.say(1053063, '', '', False, 0x3b2, player.socket) # You gather seeds from the plant.

		send(player, plant)

#
# Response to pollinate target
#
def pollinate_target(player, arguments, target):
	plant = wolfpack.finditem(arguments[0])

	if not plant or plants.plant.getStatus(plant) >= STATUS_DECORATIVE:
		return # Cancel

	if not plants.plant.checkAccess(player, plant):
		plant.say(1061856, '', '', False, 0x3b2, player.socket)
		return

	genus = plants.plant.getGenus(plant)
	hue = plants.plant.getHue(plant)
	status = plants.plant.getStatus(plant)

	# Not crossable -> no pollen
	if not genus.crossable or not hue.crossable:
		plant.say(1053050, '', '', False, 0x3b2, player.socket) # You cannot gather pollen from a mutated plant!

	# Too early to gather pollen
	elif status < STATUS_FULLGROWN:
		plant.say(1053051, '', '', False, 0x3b2, player.socket) # You cannot gather pollen from a plant in this stage of development!

	# Not healthy enough
	elif plants.plant.getHealthStatus(plant) in [HEALTH_WILTED, HEALTH_DYING]:
		plant.say(1053052, '', '', False, 0x3b2, player.socket) # You cannot gather pollen from an unhealthy plant!

	# Check the target item now
	elif not target.item or not target.item.hasscript('plants.plant'):
		plant.say(1053070, '', '', False, 0x3b2, player.socket) # You can only pollinate other specially grown plants!

	else:
		# Get the neccesary information for the target
		tgenus = plants.plant.getGenus(target.item)
		thue = plants.plant.getHue(target.item)
		tstatus = plants.plant.getStatus(target.item)

		# It's not really a plant
		if tstatus >= STATUS_DECORATIVE or tstatus <= STATUS_DIRT:
			plant.say(1053070, '', '', False, 0x3b2, player.socket) # You can only pollinate other specially grown plants!

		# It's not reachable
		elif not plants.plant.checkAccess(player, target.item):
			target.item.say(1061856, '', '', False, 0x3b2, player.socket) # You must have the item in your backpack or locked down in order to use it.

		# It's not crossable
		elif not tgenus.crossable or not thue.crossable:
			target.item.say(1053073, '', '', False, 0x3b2, player.socket) # You cannot cross-pollinate with a mutated plant!

		# It's not producing pollen
		elif tstatus < STATUS_FULLGROWN:
			target.item.say(1053074, '', '', False, 0x3b2, player.socket) # This plant is not in the flowering stage. You cannot pollinate it!

		# Not healthy enough
		elif plants.plant.getHealthStatus(target.item) in [HEALTH_WILTED, HEALTH_DYING]:
			target.item.say(1053075, '', '', False, 0x3b2, player.socket) # You cannot pollinate an unhealthy plant!

		# Already pollinated
		elif plants.plant.getPollinated(target.item):
			target.item.say(1053072, '', '', False, 0x3b2, player.socket) # This plant has already been pollinated!

		# Self Pollination
		elif target.item == plant:
			plants.plant.setPollinated(target.item, True)
			plants.plant.setSeedGenus(target.item, genus)
			plants.plant.setSeedHue(target.item, hue)
			target.item.say(1053071, '', '', False, 0x3b2, player.socket) # You pollinate the plant with its own pollen.

		# Cross Pollination
		else:
			plants.plant.setPollinated(target.item, True)
			plants.plant.setSeedGenus(target.item, genus.cross(tgenus))
			plants.plant.setSeedHue(target.item, hue.cross(thue))
			target.item.say(1053076, '', '', False, 0x3b2, player.socket) # You successfully cross-pollinate the plant.

	send(player, plant) # In the end always resend the gump
