#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Naddel                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#

from wolfpack import console, tr
from wolfpack.consts import *
import math
import wolfpack
from system.makemenus import CraftItemAction, MakeMenu, findmenu, generateNamefromDef
from wolfpack.utilities import hex2dec, tobackpack
from wolfpack.properties import itemcheck, fromitem
import random

# Sand used by Glassblowing
# The first four values are required by the makemenu system.
SAND = [
	[tr('Sand'), ALCHEMY, 0, ['sand'], 0x0]
]

#
# Check for forge
#
def checkforge(char):
	if char.gm:
		return True

	# Check dynamic items.
	items = wolfpack.items(char.pos.x, char.pos.y, char.pos.map, 5)
	for item in items:
		if item.id == 0xFB1 or (item.id >= 0x197A and item.id <= 0x19A9):
			return True

	# Check for static items
	for x in range(-2, 3):
		for y in range(-2, 3):
			statics = wolfpack.statics(char.pos.x + x, char.pos.y + y, char.pos.map, True)

			for tile in statics:
				dispid = tile[0]
				if dispid == 0xFB1 or (dispid >= 0x197A and dispid <= 0x19A9):
					return True

	return False

#
# Bring up the glassblowing menu
#
def onUse(char, item):
	menu = findmenu('GLASSBLOWING')
	if menu:
		menu.send(char, [item.serial])
	return True

#
# Craft an item.
#
class GlassblowerItemAction(CraftItemAction):
	def __init__(self, parent, title, itemid, definition):
		CraftItemAction.__init__(self, parent, title, itemid, definition)
		self.markable = 0 # All glassblowing items are not markable
		
	def visible(self, player, arguments):
		return True
		
	def checkskills(self, player, arguments, check = 0):
		success = 1
		percent = self.percentage
		rate = random.randint(0, 100)
		if check:
			if rate >= percent:
				success = 0
		return success

	#
	# Play a simple soundeffect
	#
	def playcrafteffect(self, player, arguments):
		player.soundeffect(0x2b)

	def fail(self, player, arguments, lostmaterials=0):
		if lostmaterials:
			player.socket.clilocmessage(1044043)
		else:
			player.socket.clilocmessage(1044157)
		player.soundeffect(0x41)

#
# A glassblowing menu.
#
class GlassblowingMenu(MakeMenu):
	def __init__(self, id, parent, title):
		MakeMenu.__init__(self, id, parent, title)
		self.allowmark = False
		self.allowrepair = False
		self.allowenhance = False
		
		self.allowsmelt = False
		self.submaterials1 = SAND
		self.submaterial1missing = 1053098
		self.submaterial1noskill = 1044268
		self.gumptype = 0x4f1ba419 # This should be unique
		self.requiretool = True
		self.checklearned = True

	#
	# Check for the tool and if the player has learned glassblowing
	#
	def checktool(self, player, item, wearout = False):
		if not MakeMenu.checktool(self, player, item, wearout):
			return False

		if not checkforge(player):
			player.socket.clilocmessage(1044628) # You must be near a forge to blow glass.)
			return False
		return True

	# Check if player has learned glassblowing
	def haslearned(self, player, item):
		if not player.gm and not (player.hastag( 'glassblowing' ) and player.skill[ALCHEMY] >= 1000):
			player.socket.clilocmessage(1044634) # You havent learned glassblowing.
			return False
		return True

#
# Load a menu with a given id and
# append it to the parents submenus.
#
def loadMenu(id, parent = None):
	definition = wolfpack.getdefinition(WPDT_MENU, id)
	if not definition:
		if parent:
			console.log(LOG_ERROR, "Unknown submenu %s in menu %s.\n" % (id, parent.id))
		else:
			console.log(LOG_ERROR, "Unknown menu: %s.\n" % id)
		return

	name = definition.getattribute('name', '')
	menu = GlassblowingMenu(id, parent, name)

	# See if we have any submenus
	for i in range(0, definition.childcount):
		child = definition.getchild(i)
		# Submenu
		if child.name == 'menu':
			if not child.hasattribute('id'):
				console.log(LOG_ERROR, "Submenu with missing id attribute in menu %s.\n" % menu.id)
			else:
				loadMenu(child.getattribute('id'), menu)

		# Craft an item
		elif child.name == 'glassblowing':
			if not child.hasattribute('definition'):
				console.log(LOG_ERROR, "Glassblowing action without definition in menu %s.\n" % menu.id)
			else:
				itemdef = child.getattribute('definition')
				try:
					# See if we can find an item id if it's not given
					if not child.hasattribute('itemid'):
						item = wolfpack.getdefinition(WPDT_ITEM, itemdef)
						itemid = 0
						if item:
							itemchild = item.findchild('id')
							if itemchild:
								itemid = itemchild.value
					else:
						itemid = hex2dec(child.getattribute('itemid', '0'))
					if child.hasattribute('name'):
						name = child.getattribute('name')
					else:
						name = generateNamefromDef(itemdef)
					action = GlassblowerItemAction(menu, name, int(itemid), itemdef)
				except:
					console.log(LOG_ERROR, "Glassblowing action with invalid item id in menu %s.\n" % menu.id)

				# Process subitems
				for j in range(0, child.childcount):
					subchild = child.getchild(j)
					action.processnode(subchild, menu)

	# Sort the menu. This is important for the makehistory to make.
	menu.sort()

#
# Load the masonry menu.
#
def onLoad():
	loadMenu('GLASSBLOWING')
