
#
# RunUO has a nice system for managing the possible hues.
# It's more or less like a bitfield for every color
#

from consts import *
import random

HUES = {} # Registry

#
# Get a hue info by its id
#
def getById(id):
	if id not in HUES:
		return HUES[COLOR_PLAIN]
	else:
		return HUES[id]

#
# HueInfo Class
# id is the color constant from above
# color is the color of the plant
# name is the cliloc id of the color name
# gumpcolor if specified overrides the plant color for the gump
#
class Hue:
	def __init__(self, id, color, name, gumpcolor=None):
		if gumpcolor == None:
			gumpcolor = color
			
		self.id = id
		self.color = color
		self.name = name
		self.gumpcolor = gumpcolor
		self.bright = (id & FLAG_BRIGHT) != 0
		self.crossable = (id & FLAG_CROSSABLE) != 0
		
		global HUES
		HUES[id] = self
		
	# Cross with another hue
	def cross(self, other):
		if not self.crossable or not other.crossable:
			return HUES[COLOR_PLAIN]
			
		# There's a 10% chance to get black or white
		if random.random() < 0.01:
			if random.random() < 0.5:
				return HUES[COLOR_BLACK]
			else:
				return HUES[COLOR_WHITE]

		# Crossing with plain makes no sense.
		# Always returns plain
		if self.id == COLOR_PLAIN or other.id == COLOR_PLAIN:
			return HUES[COLOR_PLAIN]

		# Remove the bright flag
		notBrightSelf = self.id & ~ FLAG_BRIGHT
		notBrightOther = other.id & ~ FLAG_BRIGHT
		
		# Crossing with the same color gets us the bright color
		if notBrightSelf == notBrightOther:
			return HUES[notBrightSelf | FLAG_BRIGHT]
			
		# Check Primary Colors
		selfPrimary = notBrightSelf in [COLOR_RED, COLOR_BLUE, COLOR_YELLOW]
		otherPrimary = notBrightOther in [COLOR_RED, COLOR_BLUE, COLOR_YELLOW]

		# Primary Colors get mixed
		if selfPrimary and otherPrimary:
			return HUES[notBrightSelf | notBrightOther]
			
		# Primaries override non primaries
		if selfPrimary and not otherPrimary:
			return HUES[notBrightSelf]
			
		if otherPrimary and not selfPrimary:
			return HUES[notBrightOther]

		# Otherwise just merge them to what they both have
		return HUES[notBrightSelf & notBrightOther]

#
# Register Default Hues
#
Hue(COLOR_PLAIN, 0, 1060813, 0x835)
Hue(COLOR_RED, 0x66d, 1060814, 0x24)
Hue(COLOR_BLUE, 0x53d, 1060815, 0x6)
Hue(COLOR_YELLOW, 0x8a5, 1060818, 0x38)
Hue(COLOR_BRIGHTRED, 0x21, 1060814, 0x21)
Hue(COLOR_BRIGHTBLUE, 0x5, 1060815, 0x6)
Hue(COLOR_BRIGHTYELLOW, 0x38, 1060818, 0x35)
Hue(COLOR_PURPLE, 0xd, 1060816, 0x10)
Hue(COLOR_GREEN, 0x59b, 1060819, 0x42)
Hue(COLOR_ORANGE, 0x46f, 1060817, 0x2e)
Hue(COLOR_BRIGHTPURPLE, 0x10, 1060816, 0xd)
Hue(COLOR_BRIGHTGREEN, 0x42, 1060819, 0x3f)
Hue(COLOR_BRIGHTORANGE, 0x2b, 1060817, 0x2b)
Hue(COLOR_BLACK, 0x455, 1060820, 0)
Hue(COLOR_WHITE, 0x481, 1060821, 0x481)
Hue(COLOR_PINK, 0x48e, 1061854)
Hue(COLOR_MAGENTA, 0x486, 1061852)
Hue(COLOR_AQUA, 0x495, 1061853)
Hue(COLOR_FIRERED, 0x489, 1061855)

#
# Check certain combinations
#
