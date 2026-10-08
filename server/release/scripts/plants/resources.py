
import wolfpack
from consts import *

# Array of resources
# First element is the string id of the genus
# Second element is the color constant
# Third element is the item id
RESOURCES = [
	['elephantearplant', COLOR_BRIGHTRED, 'redleaves'],
	['ponytailpalm', COLOR_BRIGHTRED, 'redleaves'],
	['centuryplant', COLOR_BRIGHTRED, 'redleaves'],
	['poppies', COLOR_BRIGHTORANGE, 'orangepetals'],
	['bulrushes', COLOR_BRIGHTORANGE, 'orangepetals'],
	['pampasgrass', COLOR_BRIGHTORANGE, 'orangepetals'],
	['snakeplant', COLOR_BRIGHTGREEN, 'greenthorns'],
	['barrelcactus', COLOR_BRIGHTGREEN, 'greenthorns'],
]

#
# Checks if the given genus and hue produce some sort of resource
#
def canProduce(genus, hue):
	for item in RESOURCES:
		if item[0] == genus.id and item[1] == hue.id:
			return True

	return False

#
# Create the resource produced by the given genus/hue
#
def create(genus, hue):
	for item in RESOURCES:
		if item[0] == genus.id and item[1] == hue.id:
			return wolfpack.additem(item[2])

	return None
