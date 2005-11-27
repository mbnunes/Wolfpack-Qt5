
import random

#
# Global registry of possible plant genuses
#
GENUSES = []

#
# Get a genus by its name
#
def getByIndex(index):
	if index >= 0 and index < len(GENUSES):
		return GENUSES[index]
	elif len(GENUSES) > 0:
		return GENUSES[0]
	else:
		raise RuntimeError, 'Empty GENUSES array.'

#
# Get a genus by its name
#
def getById(id):
	# Try to find the given genus by its name
	for genus in GENUSES:
		if genus.id == id:
			return genus

	# We didn't find anything by that name
	if len(GENUSES) > 0:
		return GENUSES[0]
	else:
		raise RuntimeError, 'Empty GENUSES array.'

#
# Very lightweight class that stores information about a genus
#
class Genus:
	# id is the unique identifier for this genus
	# itemid is the item id of the full grown plant
	# xoffset,yoffset are the gump offsets
	# plant This is a utility flag to indicate that the name already contains a "plant" suffix
	# flower Indicates that this is a flower. (Higher infestation chance)
	# Crossable indicates that this is a crossable plant (produces polls etc. etc.)
	# Specialtitle is the cliloc id of the title used instead of the hue if applicable
	def __init__(self, id, itemid, xoffset, yoffset, plant = False, flower = False, crossable = False, specialtitle = 0):
		self.id = id
		self.itemid = itemid
		self.name = '#%u' % (1020000 + itemid)
		self.xoffset = xoffset
		self.yoffset = yoffset
		self.plant = plant
		self.flower = flower
		self.crossable = crossable
		self.specialtitle = specialtitle

		# AutoRegister
		global GENUSES
		self.index = len(GENUSES)
		GENUSES.append(self)

	# Cross with another genus
	def cross(self, other):
		# Check if both are crossable
		if not self.crossable or not other.crossable:
			return getByIndex(0) # Fallback to default

		# Both genuses are in the same kind of genus (stupid expression)
		if self.index + 1 == other.index or other.index + 1 == self.index:
			if random.random() < 0.50:
				return self
			else:
				return other
		# Otherwise meet in the middle
		else:
			return getByIndex((self.index + other.index) / 2)

#
# Build table with all possible genuses sorted by item-id
#
Genus('campionflowers', 0xc83, 0, 0, False, True, True) # HAS TO COME FIRST! Fallback Genus.
Genus('poppies', 0xc86, 0, 0, False, True, True)
Genus('snowdrops', 0xc88, 0, 10, False, True, True)
Genus('bulrushes', 0xc94, -15, 0, False, True, True)
Genus('lilies', 0xc8b, 0, 0, False, True, True)
Genus('pampasgrass', 0xca5, -8, 0, False, True, True)
Genus('rushes', 0xca7, -10, 0, False, True, True)
Genus('elephantearplant', 0xc97, -20, 0, True, False, True)
Genus('fern', 0xc9f, -20, 0, False, False, True)
Genus('ponytailpalm', 0xca6, -16, -5, False, False, True)
Genus('smallpalm', 0xc9c, -5, -10, False, False, True)
Genus('centuryplant', 0xd31, 0, -27, True, False, True)
Genus('waterplant', 0xd04, 0, 10, True, False, True)
Genus('snakeplant', 0xca9, 0, 0, True, False, True)
Genus('pricklypearcactus', 0xd2c, 0, 10, False, False, True)
Genus('barrelcactus', 0xd26, 0, 10, False, False, True)
Genus('tribarrelcactus', 0xd27, 0, 10, False, False, True)
Genus('commongreenbonsai', 0x28dc, -5, 5, True, False, False, specialtitle = 1063335) # Common
Genus('commonpinkbonsai', 0x28df, -5, 5, True, False, False, specialtitle = 1063335) # Common
Genus('uncommongreenbonsai', 0x28dd, -5, 5, True, False, False, specialtitle = 1063336) # Uncommon
Genus('uncommonpinkbonsai', 0x28e0, -5, 5, True, False, False, specialtitle = 1063336) # Uncommon 
Genus('raregreenbonsai', 0x28de, -5, 5, True, False, False, specialtitle = 1063337) # Rare
Genus('rarepinkbonsai', 0x28e1, -5, 5, True, False, False, specialtitle = 1063337) # Rare
Genus('exceptionalbonsai', 0x28e2, -5, 5, True, False, False, specialtitle = 1063341) # Exceptional
Genus('exoticbonsai', 0x28e3, -5, 5, True, False, False, specialtitle = 1063342) # Exotic
