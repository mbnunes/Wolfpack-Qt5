
import wolfpack
import wolfpack.gumps
import plants.plant
import plants
from consts import *
import potions.utilities
import potionkeg
import plants.emptybowlgump
import plants.reproductiongump

#
# Add the background to the gump
#
def addBackground(dialog):
	dialog.addResizeGump(50, 50, 0xe10, 200, 150)
	dialog.addTilePic(45, 45, 0xcef)
	dialog.addTilePic(45, 118, 0xcf0)
	dialog.addTilePic(211, 45, 0xceb)
	dialog.addTilePic(211, 118, 0xcec)	

#
# Add the plant to the gump
#
def addPlant(dialog, plant):
	status = plants.plant.getStatus(plant)
	genus = plants.plant.getGenus(plant)
	hue = plants.plant.getHue(plant)

	# If it's not full grown yet, show the growth status
	if status < STATUS_FULLGROWN:
		dialog.addGump(110, 85, 0x589)
		dialog.addTilePic(122, 94, 0x914)
		dialog.addTilePic(135, 94, 0x914)
		dialog.addTilePic(120, 112, 0x914)
		dialog.addTilePic(135, 112, 0x914)

		# Display leafs
		if status >= STATUS_STAGE2:
			dialog.addTilePic(127, 112, 0xc62)
		if status in [STATUS_STAGE3, STATUS_STAGE4]:
			dialog.addTilePic(129, 85, 0xc7e)
		if status >= STATUS_STAGE4:
			dialog.addTilePic(121, 117, 0xc62)
			dialog.addTilePic(133, 117, 0xc62)
		if status >= STATUS_STAGE5:
			dialog.addTilePic(110, 100, 0xc62)
			dialog.addTilePic(140, 100, 0xc62)
			dialog.addTilePic(110, 130, 0xc62)
			dialog.addTilePic(140, 130, 0xc62)
		if status >= STATUS_STAGE6:
			dialog.addTilePic(105, 115, 0xc62)
			dialog.addTilePic(145, 115, 0xc62)
			dialog.addTilePic(125, 90, 0xc62)
			dialog.addTilePic(125, 135, 0xc62)

	# Otherwise display the full grown product
	else:
		dialog.addTilePic(130 + genus.xoffset, 96 + genus.yoffset, genus.itemid)

	# If it's not a plant yet, don't display the health information
	if status != STATUS_DIRT:
		health = plants.plant.getHealthStatus(plant)
		clilocid = 1060822 + health

		if health == HEALTH_DYING:
			dialog.addTilePic(92, 167, 0x1b9d)
			dialog.addTilePic(161, 167, 0x1b9d)
			dialog.addXmfHtmlGump(136, 167, 42, 20, clilocid, False, False, 0x00FC00)
		elif health == HEALTH_WILTED:
			dialog.addTilePic(91, 164, 0x18e6)
			dialog.addTilePic(161, 164, 0x18e6)
			dialog.addXmfHtmlGump(132, 167, 42, 20, clilocid, False, False, 0x00c207)
		elif health == HEALTH_HEALTHY:
			dialog.addTilePic(96, 168, 0xc61)
			dialog.addTilePic(162, 168, 0xc61)
			dialog.addXmfHtmlGump(129, 167, 42, 20, clilocid, False, False, 0x008200)
		elif health == HEALTH_VIBRANT:
			dialog.addTilePic(93, 162, 0x1a99)
			dialog.addTilePic(162, 162, 0x1a99)
			dialog.addXmfHtmlGump(129, 167, 42, 20, clilocid, False, False, 0x0083e0)

def addPlus(dialog, x, y, level):
	if level == 1:
		dialog.addText(x, y, '+', 0x35)
	elif level == 2:
		dialog.addText(x, y, '+', 0x21)

def addPlusMinus(dialog, x, y, level):
	if level == 0:
		dialog.addText(x, y, '-', 0x21)
	elif level == 1:
		dialog.addText(x, y, '-', 0x35)
	elif level == 3:
		dialog.addText(x, y, '+', 0x35)
	elif level == 4:
		dialog.addText(x, y, '+', 0x21)

def addGrowthIndicator(dialog, x, y, plant):
	status = plants.plant.getStatus(plant)

	if status >= STATUS_STAGE1 and status <= STATUS_STAGE9:
		gi = plants.plant.getGrowthIndicator(plant)

		if gi == GROWTH_INVALIDLOCATION:
			dialog.addText(x, y, '!', 0x21)
		elif gi == GROWTH_NOTHEALTHY:
			dialog.addText(x, y, '-', 0x21)
		elif gi == GROWTH_DELAY:
			dialog.addText(x, y, '-', 0x35)
		elif gi == GROWTH_GROWN:
			dialog.addText(x, y, '+', 0x3)
		elif gi == GROWTH_DOUBLEGROWN:
			dialog.addText(x, y, '+', 0x3f)

#
# Build and send the mainmenu for the given plant
#
def send(player, plant):
	dialog = wolfpack.gumps.cGump(x=20, y=20)

	addBackground(dialog) # Add to the background
	addPlant(dialog, plant) # Add the plant image to the gump

	dialog.addButton(71, 67, 0xd4, 0xd4, 1) # Reproduction Menu
	dialog.addTilePic(59, 68, 0xd08)

	# Add Status Indicators
	dialog.addButton(71, 91, 0xd4, 0xd4, 2) # Infestation
	dialog.addTilePic(8, 96, 0x372)
	addPlus(dialog, 95, 92, plants.plant.getInfestation(plant))

	dialog.addButton(71, 115, 0xd4, 0xd4, 3) # Fungus
	dialog.addTilePic(58, 115, 0xd16)
	addPlus(dialog, 95, 116, plants.plant.getFungus(plant))

	dialog.addButton(71, 139, 0xd4, 0xd4, 4) # Poison
	dialog.addTilePic(59, 143, 0x1ae4)
	addPlus(dialog, 95, 140, plants.plant.getPoison(plant))

	dialog.addButton(71, 163, 0xd4, 0xd4, 5) # Disease
	dialog.addTilePic(55, 167, 0x1727)
	addPlus(dialog, 95, 164, plants.plant.getDisease(plant))

	dialog.addButton(209, 67, 0xd2, 0xd2, 6) # Water
	dialog.addTilePic(193, 67, 0x1f9d)
	addPlusMinus(dialog, 196, 67, plants.plant.getWater(plant))

	dialog.addButton(209, 91, 0xd4, 0xd4, 7) # Poison potion
	dialog.addTilePic(197, 91, 0xf0a)
	dialog.addText(196, 91, str(plants.plant.getPoisonPotion(plant)), 0x835)

	dialog.addButton(209, 115, 0xd4, 0xd4, 8) # Cure potion
	dialog.addTilePic(192, 115, 0xf07)
	dialog.addText(196, 115, str(plants.plant.getCurePotion(plant)), 0x835)

	dialog.addButton(209, 139, 0xd4, 0xd4, 9) # Heal potion
	dialog.addTilePic(190, 139, 0xf0c)
	dialog.addText(196, 139, str(plants.plant.getHealPotion(plant)), 0x835)

	dialog.addButton(209, 163, 0xd4, 0xd4, 10) # Strength potion
	dialog.addTilePic(193, 163, 0xf09)
	dialog.addText(196, 163, str(plants.plant.getStrengthPotion(plant)), 0x835)

	dialog.addGump(48, 47, 0xd2)	
	dialog.addText(54, 47, str(plants.plant.getStatus(plant)), 0x835)

	dialog.addGump(232, 47, 0xd2)
	addGrowthIndicator(dialog, 239, 47, plant)

	dialog.addButton(48, 183, 0xd2, 0xd2, 11) # Help
	dialog.addText(54, 183, '?', 0x835)

	dialog.addButton(232, 183, 0xd4, 0xd4, 12) # Empty the bowl
	dialog.addTilePic(219, 180, 0x15fd)

	dialog.setCallback(response)
	dialog.setArgs([plant.serial])
	dialog.send(player)

#
# Gump Response
#
def response(player, arguments, response):
	plant = wolfpack.finditem(arguments[0])

	if response.button == 0 or not plant or plants.plant.getStatus(plant) >= STATUS_DECORATIVE or not player.canreach(plant, 3):
		return # Cancel

	if not plants.plant.checkAccess(player, plant):
		plant.say(1061856, '', '', False, 0x3b2, player.socket)
		return

	# Reproduction Menu
	if response.button == 1:
		if plants.plant.getStatus(plant) > STATUS_DIRT:
			plants.reproductiongump.send(player, plant)
			pass
		else:
			player.socket.clilocmessage(1061885) # You need to plant a seed in the bowl first.
			send(player, plant)

	# Infestation
	elif response.button == 2:
		send(player, plant)	
		plants.sendCodexOfWisdom(player.socket, 54)

	# Fungus
	elif response.button == 3:
		send(player, plant)	
		plants.sendCodexOfWisdom(player.socket, 56)

	# Poison
	elif response.button == 4:
		send(player, plant)	
		plants.sendCodexOfWisdom(player.socket, 58)

	# Disease
	elif response.button == 5:
		send(player, plant)
		plants.sendCodexOfWisdom(player.socket, 60)

	# Water
	elif response.button == 6:
		addWater(player, plant)

	# Poison Potion
	elif response.button == 7:
		addPotion(player, plant, [16, 17])

	# Cure Potion
	elif response.button == 8:
		addPotion(player, plant, [6])

	# Heal Potion
	elif response.button == 9:
		addPotion(player, plant, [3])

	# Strength Potion
	elif response.button == 10:
		addPotion(player, plant, [10])

	# Help
	elif response.button == 11:
		send(player, plant)	
		plants.sendCodexOfWisdom(player.socket, 48)

	# Empty the Bowl
	elif response.button == 12:
		plants.emptybowlgump.send(player, plant)
		pass

#
# This is an addition to the normal osi plant system
# it allows adding water just like potions
#
def addWater(player, plant):
	# Try to find an item to pour into the plant
	potion = None
	backpack = player.getbackpack()
	for item in backpack.content:
		# Beverage -> Water
		if item.hasscript('beverage'):
			# It's not water or doesn't have enough content
			if item.gettag('fluid') == 'water' and item.gettag('quantity'):
				potion = item
				break

	if not potion:
		player.socket.clilocmessage(1060808, "#%u" % plants.plant.getStatusCliloc(plant)) # Target the container you wish to use to water the ~1_val~.
		player.socket.attachtarget('plants.plant.pour_target', [plant.serial])
	else:
		plants.plant.pour(player, plant, potion)

	send(player, plant) # Resend the main gump

def addPotion(player, plant, potiontypes):
	# Try to find an item to pour into the plant
	potion = None
	backpack = player.getbackpack()
	for item in backpack.content:
		if item.hasscript('potions'):
			potiontype = potions.utilities.getPotionType(item)
			if potiontype in potiontypes:
				potion = item
				break
		elif potionkeg.isPotionkeg(item) and item.gettag('kegfill'):
			potiontype = potions.utilities.getPotionType(item)
			if potiontype in potiontypes:
				potion = item
				break

	if potion:
		plants.plant.pour(player, plant, potion)
	else:
		# If we CAN apply a potion of the type, show a target
		if plants.plant.applyPotion(player, plant, potiontypes[0], True):
			player.socket.clilocmessage(1061884) # You don't have any strong potions of that type in your pack.	
			player.socket.clilocmessage(1060808, "#%u" % plants.plant.getStatusCliloc(plant)) # Target the container you wish to use to water the ~1_val~.
			player.socket.attachtarget('plants.plant.pour_target', [plant.serial])		

		# otherwise just print a message
		else:
			plant.say(1053065, '', '', False, 0x3b2, player.socket) # The plant is already soaked with this type of potion!

	send(player, plant) # Resend the main gump
