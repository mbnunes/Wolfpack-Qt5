#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Darkstorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack import console
from wolfpack.consts import *
import math
import wolfpack
from system.makemenus import CraftItemAction, MakeMenu, findmenu, generateNamefromDef
from wolfpack.utilities import hex2dec, tobackpack
from wolfpack.properties import itemcheck, fromitem
import random
from wolfpack import tr

# Metals used by Blacksmithing
# The first four values are required by the makemenu system.
# The last value is the resname to use for newly crafted items
METALS = [
	[tr('Iron'),	BLACKSMITHING, 0, ['iron_ingot'], 0x0, 'iron'],
	[tr('Dull Copper'), BLACKSMITHING, 650, ['dullcopper_ingot'], 0x973, 'dullcopper'],
	[tr('Shadow Iron'), BLACKSMITHING, 700, ['shadowiron_ingot'], 0x966, 'shadowiron'],
	[tr('Copper'),	BLACKSMITHING, 750, ['copper_ingot'], 0x96d, 'copper'],
	[tr('Bronze'),	BLACKSMITHING, 800, ['bronze_ingot'], 0x972, 'bronze'],
	[tr('Gold'),	BLACKSMITHING, 850, ['gold_ingot'], 0x8a5, 'gold'],
	[tr('Agapite'),	BLACKSMITHING, 900, ['agapite_ingot'], 0x979, 'agapite'],
	[tr('Verite'),	BLACKSMITHING, 950, ['verite_ingot'], 0x89f, 'verite'],
	[tr('Valorite'),	BLACKSMITHING, 990, ['valorite_ingot'], 0x8ab, 'valorite'],
]

#
# A list of scales used by the blacksmithing menu.
#
SCALES = [
	[tr('Red Scales'), 0, 0, ['red_scales'], 0x66D, 'red_scales'],
	[tr('Yellow Scales'), 0, 0, ['yellow_scales'], 0x8A8, 'yellow_scales'],
	[tr('Black Scales'), 0, 0, ['black_scales'], 0x455, 'black_scales'],
	[tr('Green Scales'), 0, 0, ['green_scales'], 0x851, 'green_scales'],
	[tr('White Scales'), 0, 0, ['white_scales'], 0x8FD, 'white_scales'],
	[tr('Blue Scales'), 0, 0, ['blue_scales'], 0x8B0, 'blue_scales'],
]

# Lists of forges and anvils.
FORGES = [ 0xfb1, 0x2dd8 ] + range( 0x197a, 0x19aa )
ANVILS = [ 0xfaf, 0xfb0, 0x2dd5, 0x2dd6]

#
# Check for anvil and forge
#
def checkanvilandforge(char):
	if char.gm:
		return True

	# Check dynamic items.
	forge = False
	anvil = False
	items = wolfpack.items(char.pos.x, char.pos.y, char.pos.map, 5)
	for item in items:
		if item.id in ANVILS:
			anvil = True
		elif item.id in FORGES:
			forge = True

		if anvil and forge:
			return True

	# Check for static items
	for x in range(-2, 3):
		for y in range(-2, 3):
			statics = wolfpack.statics(char.pos.x + x, char.pos.y + y, char.pos.map, True)

			for tile in statics:
				dispid = tile[0]
				if dispid in ANVILS:
					anvil = True
				elif dispid in FORGES:
					forge = True

				if anvil and forge:
					return True

	return False

#
# Bring up the blacksmithing menu
#
def onUse(char, item):
	menu = findmenu('BLACKSMITHING')
	if menu:
		menu.send(char, [item.serial])
	return True

#
# Smith an item.
# Used for scales + ingots
#
class SmithItemAction(CraftItemAction):
	def __init__(self, parent, title, itemid, definition):
		CraftItemAction.__init__(self, parent, title, itemid, definition)
		self.markable = 1 # All blacksmith items are markable

	#
	# Check if we did an exceptional job.
	#
	def getexceptionalchance(self, player, arguments):
		# Only works if this item requires blacksmithing
		if not self.skills.has_key(BLACKSMITHING):
			return False

		minskill = self.skills[BLACKSMITHING][0]
		maxskill = self.skills[BLACKSMITHING][1]
		penalty = self.skills[BLACKSMITHING][2]

		if not penalty:
			penalty = 250

		minskill += penalty
		maxskill += penalty

		chance = ( player.skill[BLACKSMITHING] - minskill ) / 10

		# chance = 0 - 100
		if chance > 100:
			chance = 100
		elif chance < 0:
			chance = chance * -1

		# chance range 0.00 - 1.00
		chance = chance * .01
		return chance

	#
	# Apply resname and color to the item.
	#
	def applyproperties(self, player, arguments, item, exceptional):
		# See if special ingots were used in the creation of
		# this item. All items crafted by blacksmiths gain the
		# color!
		if self.submaterial1 > 0:
			material = self.parent.getsubmaterial1used(player, arguments)
			material = self.parent.submaterials1[material]
			item.color = material[4]
			item.settag('resname', material[5])

		# Apply properties of secondary material
		if self.submaterial2 > 0:
			material = self.parent.getsubmaterial2used(player, arguments)
			material = self.parent.submaterials2[material]
			item.color = material[4]
			item.settag('resname2', material[5])

		# Apply one-time boni
		healthbonus = fromitem(item, DURABILITYBONUS)
		if healthbonus != 0:
			bonus = int(math.ceil(item.maxhealth * (healthbonus / 100.0)))
			item.maxhealth = max(1, item.maxhealth + bonus)
			item.health = item.maxhealth

		# Distribute another 6 points randomly between the resistances this armor already has
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
			elif itemcheck(item, ITEM_WEAPON):
				# Increase the damage bonus by 20%
				bonus = fromitem(item, DAMAGEBONUS)
				bonus += 20
				item.settag('aos_boni_damage', bonus)

	#
	# Play a simple soundeffect
	#
	def playcrafteffect(self, player, arguments):
		player.soundeffect(0x2a)

#
# The user has to have Samurai Empire installed
#
class SeSmithItemAction(SmithItemAction):
	def __init__(self, parent, title, itemid, definition):
		SmithItemAction.__init__(self, parent, title, itemid, definition)

	def visible(self, char, arguments):
		if char.socket and char.socket.flags & 0x10 == 0:
			return False
		else:
			return SmithItemAction.visible(self, char, arguments)

	def checkmaterial(self, player, arguments, silent = 0):
		if player.socket and player.socket.flags & 0x10 == 0:
			return False
		else:
			return SmithItemAction.checkmaterial(self, player, arguments, silent)

#
# A blacksmith menu. The most notable difference is the
# button for selecting another ore.
#
class BlacksmithingMenu(MakeMenu):
	def __init__(self, id, parent, title):
		MakeMenu.__init__(self, id, parent, title)
		self.allowmark = True
		self.allowrepair = True
		self.allowenhance = True
		self.allowsmelt = True
		self.submaterials1 = METALS
		self.submaterials2 = SCALES
		self.submaterial2missing = 1060884
		self.submaterial1missing = 1044037
		self.submaterial1noskill = 1044268
		self.gumptype = 0x4f1ba410 # This should be unique
		self.repairsound = 0x2a
		self.requiretool = True

	#
	# Check for an anvil too when checking for the tool
	#
	def checktool(self, player, item, wearout = False):
		if not MakeMenu.checktool(self, player, item, wearout):
			return False

		if not checkanvilandforge(player):
			player.socket.clilocmessage(1044267)
			return False

		return True

	#
	# Smelt an item and create new ingots from it.
	#
	def smelt(self, player, arguments, target):
		if not self.checktool(player, wolfpack.finditem(arguments[0])):
			return False

		tool = wolfpack.finditem(arguments[0])
		if tool == target.item:
			player.socket.clilocmessage(1044271)
			return

		# Smelt a weapon
		item = target.item
		weapon = itemcheck(item, ITEM_WEAPON)
		shield = itemcheck(item, ITEM_SHIELD)
		armor = itemcheck(item, ITEM_ARMOR)

		# See if it's in our ore list.
		if weapon or shield or armor:
			if item.container != player.getbackpack():
				player.socket.clilocmessage(1044274)
				return

			if item.hastag('resname'):
				resname = str(item.gettag('resname'))
			else:
				resname = None

			for metal in METALS:
				if metal[5] == resname:
					# Try to find out how many ingots this item would need
					action = self.topmostmenu().findcraftitem(item.baseid)
					returned = 0.25 + min(1000, player.skill[MINING]) / 2000

					if action and action.submaterial1 > 0:
						amount = int(math.floor(action.submaterial1 * returned))
					else:
						amount = 1

					if amount > 0:
						# Randomly select one of the resources required by that metal
						item.delete()
						ingots = wolfpack.additem(random.choice(metal[3]))
						ingots.amount = amount
						if not tobackpack(ingots, player):
							ingots.update()

						player.soundeffect(0x2a)
						player.soundeffect(0x240)

						player.socket.clilocmessage(1044270)
						self.send(player, arguments)
						return

		player.socket.clilocmessage(1044272)
		self.send(player, arguments)

	#
	# Get the material used by the character from the tags
	#
	def getsubmaterial1used(self, player, arguments):
		if not player.hastag('blacksmithing_ore'):
			return False
		else:
			material = int(player.gettag('blacksmithing_ore'))
			if material < len(self.submaterials1):
				return material
			else:
				return False

	#
	# Get the material used by the character from the tags
	#
	def getsubmaterial2used(self, player, arguments):
		if not player.hastag('blacksmithing_scales'):
			return False
		else:
			material = int(player.gettag('blacksmithing_scales'))
			if material < len(self.submaterials2):
				return material
			else:
				return False

	#
	# Save the material preferred by the user in a tag
	#
	def setsubmaterial1used(self, player, arguments, material):
		player.settag('blacksmithing_ore', material)

	#
	# Save the material preferred by the user in a tag
	#
	def setsubmaterial2used(self, player, arguments, material):
		player.settag('blacksmithing_scales', material)

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
	menu = BlacksmithingMenu(id, parent, name)

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
		elif child.name in ['smith', 'sesmith']:
			if not child.hasattribute('definition'):
				console.log(LOG_ERROR, "Smith action without definition in menu %s.\n" % menu.id)
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
							console.log(LOG_ERROR, "Smith action with invalid definition %s in menu %s.\n" % (itemdef, menu.id))
					else:
						itemid = hex2dec(child.getattribute('itemid', '0'))
					if child.hasattribute('name'):
						name = child.getattribute('name')
					else:
						name = generateNamefromDef(itemdef)
					if child.name == 'sesmith':
						action = SeSmithItemAction(menu, name, int(itemid), itemdef)
					else:
						action = SmithItemAction(menu, name, int(itemid), itemdef)
				except:
					console.log(LOG_ERROR, "Smith action with invalid item id in menu %s.\n" % menu.id)

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
	loadMenu('BLACKSMITHING')
