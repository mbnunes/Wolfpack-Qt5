#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Darkstorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack import console, tr
from wolfpack.consts import *
import math
import wolfpack
from system.makemenus import CraftItemAction, MakeMenu, findmenu
from wolfpack.utilities import hex2dec, tobackpack
from wolfpack.properties import itemcheck, fromitem
import random

# Leather types used by tailoring.
LEATHERS = [
		['Leather',		TAILORING, 0, 	['leather_cut', 'leather_hides'], 				0x0,   'leather'],
		['Spined Leather', 	TAILORING, 650, ['spined_leather_cut', 'spined_leather_hides'], 0x283, 'spined_leather'],
		['Horned Leather', 	TAILORING, 800, ['horned_leather_cut', 'horned_leather_hides'], 0x227, 'horned_leather'],
		['Barbed Leather', 	TAILORING, 990, ['barbed_leather_cut', 'barbed_leather_hides'], 0x1c1, 'barbed_leather'],
]

# Baseids used for clothing
CLOTH = ['175d', '175e', '175f', '1760', '1761', '1762', '1763', '1764', '1765', '1766', '1767', '1768']

#
# Bring up the tailoring menu
#
def onUse(char, item):
	menu = findmenu('TAILORING')
	if menu:
		menu.send(char, [item.serial])
	return True

#
# Response for the cloth select target
#
def ClothSelectResponse(player, arguments, target):
	global CLOTH
	action = arguments[0]
	args = list(arguments[1:])
	
	if not target.item:
		player.socket.clilocmessage(1046439) # That is not a valid target
		action.parent.send(player, args)
	elif target.item.baseid not in CLOTH:
		player.socket.clilocmessage(1046439) # That is not a valid target
		action.parent.send(player, args)
	elif target.item.container != player.getbackpack():
		player.socket.clilocmessage(1060640) # The item must be in your backpack to use it
		action.parent.send(player, args)
	else:
		action.make(player, args, False, target.item)

#
# Tailor an item.
#
class TailorItemAction(CraftItemAction):
	def __init__(self, parent, title, itemid, definition):
		CraftItemAction.__init__(self, parent, title, itemid, definition)
		self.markable = 1 # All tailoring items are markable
		self.cloth = 0 # How many cloth this part is using (Special processing because of color)
		self.requiretool = True

	#
	# This check is here to make sure that we only use one kind of 
	# cloth to make the item
	# Resource is the item used to statisfy the cloth needs.
	#
	def make(self, player, arguments, nodelay=0, clothitem = None):
		global CLOTH

		# We have not been passed a cloth reference.
		# Try to statisfy it automatically
		if self.cloth > 0:
			if not clothitem:
				backpack = player.getbackpack()
				count = 0 # Valid piles of cloth
				for item in backpack.content:
					if item.baseid in CLOTH and item.amount >= self.cloth:
						clothitem = item
						count += 1
	
				# This makes the user select a pile of cloth
				if count > 1:
					player.socket.clilocmessage(502928) # Which material would you like to work ...
					player.socket.attachtarget('skills.tailoring.ClothSelectResponse', [self, arguments])
					return
				elif count < 1:
					player.socket.clilocmessage(1044456) # You don't have any ready cloth
					return

			# Pass the clothitem on to the next function as an argument
			if len(arguments) < 2:
				arguments.append(clothitem.serial)
			else:
				arguments[1] = clothitem.serial

		return CraftItemAction.make(self, player, arguments, nodelay)

	#
	# Checkmaterial checks that the selected cloth is actually in the backpack
	# of the player
	#
	def checkmaterial(self, player, arguments, silent = 0):
		result = CraftItemAction.checkmaterial(self, player, arguments, silent)
		
		# Check for cloth
		if result and self.cloth > 0:
			assert(len(arguments) >= 2)
			cloth = wolfpack.finditem(arguments[1])
			if not player.canreach(cloth, -1):
				player.socket.clilocmessage(1044456) # You don't have any ready cloth...
				return False
			elif cloth.amount < self.cloth:
				player.socket.clilocmessage(1044287) # You don't have enough...
				return False
		
		return result
		
	#
	# Consume material
	#
	def consumematerial(self, player, arguments, half = 0):
		result = CraftItemAction.consumematerial(self, player, arguments, half)
		
		if result and self.cloth > 0:			
			if half:
				needed = int(math.ceil(self.cloth / 2))
			else:
				needed = self.cloth
		
			assert(len(arguments) >= 2)
			cloth = wolfpack.finditem(arguments[1])
			if not player.canreach(cloth, -1):
				player.socket.clilocmessage(1044456) # You don't have any ready cloth...
				return False
			elif cloth.amount < needed:
				player.socket.clilocmessage(1044287) # You don't have enough...
				return False

			# Replace the second argument with the color of the cloth
			arguments[1] = cloth.color

			if needed == cloth.amount:
				cloth.delete()
			else:
				cloth.amount -= needed
				cloth.update()

		return result

	#
	# Process additional XML nodes for this action type
	#
	def processnode(self, node, menu):
		global CLOTH
		if node.name == 'cloth':
			amount = hex2dec(node.getattribute('amount', '1'))
			if amount > 0:
				self.cloth = amount
		else:
			CraftItemAction.processnode(self, node, menu)

	#
	# Check if we did an exceptional job.
	#
	def getexceptionalchance(self, player, arguments):
		# Only works if this item requires blacksmithing
		if not self.skills.has_key(TAILORING):
			return False

		minskill = self.skills[TAILORING][0]
		maxskill = self.skills[TAILORING][1]
		chance = ( (player.skill[TAILORING] - minskill) / (maxskill - minskill) ) / 10.0

		return chance

	#
	# Add our cloth to the materials list
	#
	def getmaterialshtml(self, player, arguments):
		materialshtml = CraftItemAction.getmaterialshtml(self, player, arguments)
		
		if self.cloth > 0:
			materialshtml += "%s: %u<br>" % (tr('Cloth'), self.cloth)

		return materialshtml

	#
	# Apply resname and color to the item.
	#
	def applyproperties(self, player, arguments, item, exceptional):
		# Get the color of the cloth this item has been crafted with
		if self.cloth > 0:
			assert(len(arguments) >= 2)
			item.color = arguments[1]		
		
		# All tailoring items crafted out of leather gain the special color
		if self.submaterial1 > 0:
			material = self.parent.getsubmaterial1used(player, arguments)
			material = self.parent.submaterials1[material]
			item.color = material[4]
			item.settag('resname', material[5])

		# Distribute another 6 points randomly between the resistances an armor alread
		# has. There are no tailored weapons.
		if exceptional:
			if itemcheck(item, ITEM_ARMOR) or itemcheck(item, ITEM_SHIELD):
				# Copy all the values to tags
				boni = [0, 0, 0, 0, 0]

				for i in range(0, 6):
					boni[random.randint(0,4)] += 1

				item.settag('res_physical', fromitem(item, RESISTANCE_PHYSICAL) + boni[0])
				item.settag('res_fire', fromitem(item, RESISTANCE_FIRE) + boni[1])
				item.settag('res_cold', fromitem(item, RESISTANCE_COLD) + boni[2])
				item.settag('res_energy', fromitem(item, RESISTANCE_ENERGY) + boni[3])
				item.settag('res_poison', fromitem(item, RESISTANCE_POISON) + boni[4])

	#
	# Play a simple soundeffect
	#
	def playcrafteffect(self, player, arguments):
		player.soundeffect(0x248)

#
# The Tailor menu.
#
class TailoringMenu(MakeMenu):
	def __init__(self, id, parent, title):
		MakeMenu.__init__(self, id, parent, title)
		self.allowmark = 1
		self.allowrepair = 1
		self.allowenhance = True
		self.submaterials1 = LEATHERS
		self.submaterial1missing = 1044463
		self.submaterial1noskill = 1049311
		self.gumptype = 0x4f1ba411 # This should be unique
		self.repairsound = 0x248 # Repair soundeffect

	#
	# Get the material used by the character from the tags
	#
	def getsubmaterial1used(self, player, arguments):
		if not player.hastag('tailoring_leather'):
			return False
		else:
			material = int(player.gettag('tailoring_leather'))
			if material < len(self.submaterials1):
				return material
			else:
				return False

	#
	# Save the material preferred by the user in a tag
	#
	def setsubmaterial1used(self, player, arguments, material):
		player.settag('tailoring_leather', material)

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
	menu = TailoringMenu(id, parent, name)

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
		elif child.name == 'tailor':
			if not child.hasattribute('definition') or not child.hasattribute('name'):
				console.log(LOG_ERROR, "Tailor action without definition or name in menu %s.\n" % menu.id)
			else:
				itemdef = child.getattribute('definition')
				name = child.getattribute('name')
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
					action = TailorItemAction(menu, name, int(itemid), itemdef)
				except:
					console.log(LOG_ERROR, "Tailor action with invalid item id in menu %s.\n" % menu.id)

				# Process subitems
				for j in range(0, child.childcount):
					subchild = child.getchild(j)
					action.processnode(subchild, menu)
					
	# Sort the menu. This is important for the makehistory to make.
	menu.sort()

#
# Load the blacksmithing menu.
#
def onLoad():
	loadMenu('TAILORING')
