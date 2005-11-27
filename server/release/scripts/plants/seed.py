
import plants.genuses
import plants.hues
import wolfpack
import random
from consts import *

SEED_BASEID = 'seed'

#
# Manage the seed genus
#
def getGenus(seed):
	if not seed.hastag('genus'):
		return plants.genuses.getByIndex(0)
	else:
		return plants.genuses.getByIndex(int(seed.gettag('genus')))

def setGenus(seed, genus):
	if type(genus) == int:
		seed.settag('genus', genus)
	else:
		seed.settag('genus', genus.index)

#
# Manage the seed hue
#
def getHue(seed):
	if not seed.hastag('hue'):
		return plants.hues.getById(plants.hues.COLOR_PLAIN)
	else:
		return plants.hues.getById(int(seed.gettag('hue')))

def setHue(seed, hue):
	if type(hue) == int:
		seed.settag('hue', hue)
		color = plants.hues.getById(hue).color
	else:
		seed.settag('hue', hue.id)
		color = hue.color

	if color != seed.color:
		seed.color = color
		seed.update()

#
# Set/Get the showtype flag from a seed
#
def getShowType(seed):
	return seed.gettag('showtype') == 1
def setShowType(seed, value):
	if not value:
		seed.deltag('showtype')
	else:
		seed.settag('showtype', 1)

#
# Create a seed and return the created item
#
def create(genus, hue, showtype=True):
	seed = wolfpack.additem(SEED_BASEID)
	setShowType(seed, showtype)
	setGenus(seed, genus)
	setHue(seed, hue)
	return seed

#
# Show a nicer tooltip for the seed
#
def onShowTooltip(viewer, seed, tooltip):
	hue = getHue(seed)
	genus = getGenus(seed)
	showtype = getShowType(seed)

	# Special Title replacing the hue
	if genus.specialtitle != 0:
		title = genus.specialtitle
	else:
		title = hue.name

	if showtype:
		if hue.bright:
			tooltip.add(1061918, '#%u\t%s' % (title, genus.name)) # bright ~1_COLOR~ ~2_TYPE~ seed
		else:
			tooltip.add(1061917, '#%u\t%s' % (title, genus.name)) # ~1_COLOR~ ~2_TYPE~ seed
	else:
		if hue.bright:
			tooltip.add(1060839, '#%u' % title) # bright ~1_val~ seed
		else:
			tooltip.add(1060838, '#%u' % title) # ~1_val~ seed

#
# Plant the seed into a bowl
#
def onUse(player, seed):
	if seed.getoutmostitem() != player.getbackpack():
		player.socket.clilocmessage(1042664) # You must have the object in your backpack to use it.
	else:
		player.socket.clilocmessage(1061897) # Choose a patch of dirt to scoop up.
		seed.say(1061916, '', '', False, 0x3b2, player.socket) # Choose a bowl of dirt to plant this seed in.
		player.socket.attachtarget('plants.seed.plant_target', [seed.serial])

	return True

#
# Target callback for planting a seed
#
def plant_target(player, arguments, target):
	seed = wolfpack.finditem(arguments[0])

	if not seed or seed.getoutmostitem() != player.getbackpack():
		player.socket.clilocmessage(1042664) # You must have the object in your backpack to use it.
		return

	if target.item and target.item.hasscript('plants.plant'):
		plants.plant.plantSeed(player, target.item, seed)
	elif target.item:
		seed.say(1061919, '', '', False, 0x3b2, player.socket) # You must use a seed on a bowl of dirt!
	else:
		player.socket.clilocmessage(1061919) # You must use a seed on a bowl of dirt!

#
# Create a certain seed type?
#
def onCreate(seed, definition):
	if not seed.hastag('genus'):
		seed.settag('genus', random.choice([0, 8, 16])) # Campion, Fern, Tribarrel
	if not seed.hastag('hue'):
		seed.settag('hue', random.choice([COLOR_PLAIN, COLOR_RED, COLOR_BLUE, COLOR_YELLOW]))
		hue = getHue(seed)
		seed.color = hue.color
		seed.update()
