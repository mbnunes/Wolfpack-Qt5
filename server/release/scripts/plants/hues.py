
#
# RunUO has a nice system for managing the possible hues.
# It's more or less like a bitfield for every color
#

from consts import *

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
		
		global HUES
		HUES[id] = self

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
