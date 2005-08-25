#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
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

# Granites used by masonry
# The first four values are required by the makemenu system.
GRANITES = [
	[tr('Granite'),		BLACKSMITHING, 0, ['granite'], 0x0],
	[tr('Dull Copper Granite'), BLACKSMITHING, 0, ['dullcopper_granite'], 0x973],
	[tr('Shadow Granite'), 	BLACKSMITHING, 0, ['shadowiron_granite'], 0x966],
	[tr('Copper Granite'),	BLACKSMITHING, 0, ['copper_granite'], 0x96d],
	[tr('Bronze Granite'),	BLACKSMITHING, 0, ['bronze_granite'], 0x972],
	[tr('Gold Granite'),	BLACKSMITHING, 0, ['gold_granite'], 0x8a5],
	[tr('Agapite Granite'),	BLACKSMITHING, 0, ['agapite_granite'], 0x979],
	[tr('Verite Granite'),	BLACKSMITHING, 0, ['verite_granite'], 0x89f],
	[tr('Valorite Granite'),	BLACKSMITHING, 0, ['valorite_granite'], 0x8ab],
]

#
# Bring up the masonry menu
#
def onUse(char, item):
	menu = findmenu('MASONRY')
	if menu:
		menu.send(char, [item.serial])
	return True
	
#
# Craft an item.
#
class StonecrafterItemAction(CraftItemAction):
	def __init__(self, parent, title, itemid, definition):
		CraftItemAction.__init__(self, parent, title, itemid, definition)
		self.markable = 0 # All masonry items are not markable
		
	def visible(self, player, arguments):
		return True
		
	def checkskills(self, player, arguments, check = 0):
		success = 1
		percent = self.percentage
		rate = random.randint(0, 100)
		player.socket.sysmessage(str(check))
		if check:
			if rate >= percent:
				success = 0
		return success

	#
	# Apply color to the item.
	#
	def applyproperties(self, player, arguments, item, exceptional):
		# See if special granite was used in the creation of
		# this item. All items crafted gain the color!
		if self.submaterial1 > 0:
			material = self.parent.getsubmaterial1used(player, arguments)
			material = self.parent.submaterials1[material]
			item.color = material[4]

	#
	# Play a simple soundeffect
	#
	def playcrafteffect(self, player, arguments):
		player.soundeffect(0x2a)

#
# The user has to have Samurai Empire installed
#
class SeStonecrafterItemAction(StonecrafterItemAction):
	def __init__(self, parent, title, itemid, definition):
		StonecrafterItemAction.__init__(self, parent, title, itemid, definition)

	def visible(self, char, arguments):
		if char.socket and char.socket.flags & 0x10 == 0:
			return False
		else:
			return StonecrafterItemAction.visible(self, char, arguments)
			
	def checkmaterial(self, player, arguments, silent = 0):
		if player.socket and player.socket.flags & 0x10 == 0:
			return False
		else:
			return StonecrafterItemAction.checkmaterial(self, player, arguments, silent)

#
# A masonry menu. The most notable difference is the
# button for selecting another granite.
#
class MasonryMenu(MakeMenu):
	def __init__(self, id, parent, title):
		MakeMenu.__init__(self, id, parent, title)
		self.allowmark = False
		self.allowrepair = False
		self.allowenhance = False
		self.allowsmelt = False
		self.submaterials1 = GRANITES
		self.submaterial1noskill = 1044268
		self.gumptype = 0x4f1ba414 # This should be unique
		self.requiretool = True
		self.checklearned = True

	# Check if player has learned masonry
	def haslearned(self, player, item):
		if not player.gm and not (player.hastag( 'masonry' ) and player.skill[MINING] >= 1000):
			player.socket.clilocmessage(1044633) # You havent learned masonry.
			return False
		return True

	#
	# Get the material used by the character from the tags
	#
	def getsubmaterial1used(self, player, arguments):
		if not player.hastag('masonry_granite'):
			return False
		else:
			material = int(player.gettag('masonry_granite'))
			if material < len(self.submaterials1):
				return material
			else:
				return False

	#
	# Save the material preferred by the user in a tag
	#
	def setsubmaterial1used(self, player, arguments, material):
		player.settag('masonry_granite', material)

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
	menu = MasonryMenu(id, parent, name)

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
		elif child.name in ['masonry', 'semasonry']:
			if not child.hasattribute('definition'):
				console.log(LOG_ERROR, "Masonry action without definition in menu %s.\n" % menu.id)
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
					if child.name == 'semasonry':
						action = SeStonecrafterItemAction(menu, name, int(itemid), itemdef)
					else:
						action = StonecrafterItemAction(menu, name, int(itemid), itemdef)		
				except:
					console.log(LOG_ERROR, "Masonry action with invalid item id in menu %s.\n" % menu.id)

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
	loadMenu('MASONRY')
