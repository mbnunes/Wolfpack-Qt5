#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#

from wolfpack import console
from wolfpack.consts import *
import math
import wolfpack
from system.makemenus import CraftItemAction, MakeMenu, findmenu, generateNamefromDef
from wolfpack.utilities import hex2dec, tobackpack
from wolfpack.properties import itemcheck, fromitem
import random
from skills import blacksmithing
from wolfpack import tr
from copy import deepcopy

# last list: [ring, necklace, earrings, bracelet]
GEMS = [
		[tr('Star Sapphire'), 0, 0, ['f0f', 'f1b', 'f21'], 0x4f2, 'starsapphire', ["#1044176","#1044194","#1044203","#1044221"]],
		[tr('Emerald'), 0, 0, ['f10', 'f2f'], 0xa3, 'emerald', ["#1044177","#1044195","#1044204","#1044222"]],
		[tr('Sapphire'), 0, 0, ['f11', 'f12', 'f19', 'f1f'], 0xc2, 'sapphire', ["#1044178","#1044196","#1044205","#1044223"]],
		[tr('Ruby'), 0, 0, ['f13', 'f14', 'f1a', 'f1c', 'f1d', 'f2a', 'f2b'], 0x21, 'ruby', ["#1044179","#1044197","#1044206","#1044224"]],
		[tr('Citrine'), 0, 0, ['f15', 'f23', 'f24', 'f2c'], 0x30, 'citrine', ["#1044180","#1044198","#1044207","#1044225"]],
		[tr('Amethyst'), 0, 0, ['f16', 'f17', 'f22', 'f2e'], 0x13, 'amethyst', ["#1044181","#1044199","#1044208","#1044226"]],
		[tr('Tourmaline'), 0, 0, ['f18', 'f1e', 'f20', 'f2d'], 0x9a, 'tourmaline', ["#1044182","#1044200","#1044209","#1044227"]],
		[tr('Amber'), 0, 0, ['f25'], 0x7da, 'amber', ["#1044183","#1044201","#1044210","#1044228"]],
		[tr('Diamond'), 0, 0, ['f26','f27','f28','f29','f30'], 0x47e, 'diamond', ["#1044184","#1044202","#1044211","#1044229"]]
]

# Use metals list from Blacksmithing, but test for tinkering skill
TINKERINGMETALS = deepcopy(blacksmithing.METALS)
for metal in TINKERINGMETALS:
	metal[1] = TINKERING

def name_item( item, material ):
	# ring
	if item.baseid == "1f09":
		return material[6][0]
	# Bead Necklace
	elif item.baseid in [ "1f05", "1f08" ]:
		return material[6][1]
	# earrings
	elif item.baseid == "1f07":
		return material[6][2]
	# bracelet
	elif item.baseid == "1f06":
		return material[6][3]

#
# Bring up the tinkering menu
#
def onUse(char, item):
	menu = findmenu('TINKERING')
	if menu:
		menu.send(char, [item.serial])
	return True

class TinkerItemAction(CraftItemAction):
	def __init__(self, parent, title, itemid, definition):
		CraftItemAction.__init__(self, parent, title, itemid, definition)
		self.markable = 1
		self.retaincolor = 0
		self.stackable = 0

	#
	# Check if we did an exceptional job.
	#
	def getexceptionalchance(self, player, arguments):
		if not self.markable:
			return False

		if not self.skills.has_key(TINKERING):
			return False

		minskill = self.skills[TINKERING][0]
		maxskill = self.skills[TINKERING][1]
		penalty = self.skills[TINKERING][2]

		if not penalty:
			penalty = 250

		minskill += penalty
		maxskill += penalty

		chance = ( player.skill[TINKERING] - minskill ) / 10

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
		item.decay = 1

		# Use all available resources if the item we make is
		# flagged as "stackable".
		if self.stackable:
			backpack = player.getbackpack()
			count = -1
			for (materials, amount, name) in self.materials:
				items = backpack.countitems(materials)
				if count == -1:
					count = items / amount
				else:
					count = min(count, items / amount)
			for (materials, amount, name) in self.materials:
				backpack.removeitems( materials, count )
			if count != -1:
				item.amount += count
			else:
				item.amount = 1 + count
			item.update()

		# See if this item
		if self.retaincolor and self.submaterial1 > 0:
			material = self.parent.getsubmaterial1used(player, arguments)
			material = self.parent.submaterials1[material]
			item.color = material[4]
			item.settag('resname', material[5])

		# Apply properties of secondary material
		if self.submaterial2 > 0:
			material = self.parent.getsubmaterial2used(player, arguments)
			material = self.parent.submaterials2[material]
			item.color = material[4]
			item.name = name_item( item, material )
			item.settag('resname2', material[5])

		# Apply one-time boni
		healthbonus = fromitem(item, DURABILITYBONUS)
		if healthbonus != 0:
			bonus = int(math.ceil(item.maxhealth * (healthbonus / 100.0)))
			item.maxhealth = max(1, item.maxhealth + bonus)
			item.health = item.maxhealth

	#
	# Play a simple soundeffect
	#
	def playcrafteffect(self, player, arguments):
		player.soundeffect(0x2a)

#
# The user has to have Samurai Empire installed
#
class SeTinkerItemAction(TinkerItemAction):
	def __init__(self, parent, title, itemid, definition):
		TinkerItemAction.__init__(self, parent, title, itemid, definition)

	def visible(self, char, arguments):
		if char.socket and char.socket.flags & 0x10 == 0:
			return False
		else:
			return TinkerItemAction.visible(self, char, arguments)

	def checkmaterial(self, player, arguments, silent = 0):
		if player.socket and player.socket.flags & 0x10 == 0:
			return False
		else:
			return TinkerItemAction.checkmaterial(self, player, arguments, silent)

class TinkeringMenu(MakeMenu):
	def __init__(self, id, parent, title):
		MakeMenu.__init__(self, id, parent, title)
		self.allowmark = True
		self.allowrepair = True
		self.submaterials1 = TINKERINGMETALS
		self.submaterials2 = GEMS
		self.submaterial1missing = 1044037
		self.submaterial1noskill = 1044268
		self.submaterial2missing = 1044240
		self.gumptype = 0x41afb410 # This should be unique
		self.requiretool = True

	#
	# Get the material used by the character from the tags
	#
	def getsubmaterial1used(self, player, arguments):
		if not player.hastag('tinkering_ore'):
			return False
		else:
			material = int(player.gettag('tinkering_ore'))
			if material < len(self.submaterials1):
				return material
			else:
				return False

	#
	# Get the material used by the character from the tags
	#
	def getsubmaterial2used(self, player, arguments):
		if not player.hastag('tinkering_gems'):
			return False
		else:
			material = int(player.gettag('tinkering_gems'))
			if material < len(self.submaterials2):
				return material
			else:
				return False

	#
	# Save the material preferred by the user in a tag
	#
	def setsubmaterial1used(self, player, arguments, material):
		player.settag('tinkering_ore', material)

	#
	# Save the material preferred by the user in a tag
	#
	def setsubmaterial2used(self, player, arguments, material):
		player.settag('tinkering_gems', material)

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
	menu = TinkeringMenu(id, parent, name)

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
		elif child.name in ['tinker', 'setinker']:
			if not child.hasattribute('definition'):
				console.log(LOG_ERROR, "Tinker action without definition in menu %s.\n" % menu.id)
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
					if child.name == 'setinker':
						action = SeTinkerItemAction(menu, name, int(itemid), itemdef)
					else:
						action = TinkerItemAction(menu, name, int(itemid), itemdef)
				except:
					console.log(LOG_ERROR, "Tinker action with invalid item id in menu %s.\n" % menu.id)

				# Process subitems
				for j in range(0, child.childcount):
					subchild = child.getchild(j)

					# How much of the primary resource should be consumed
					if subchild.name == 'ingots':
						action.submaterial1 = hex2dec(subchild.getattribute('amount', '0'))

					elif subchild.name == 'gems':
						action.submaterial2 = hex2dec(subchild.getattribute('amount', '0'))

					# Standard material
					elif subchild.name == 'logs':
						amount = hex2dec(subchild.getattribute('amount', '0'))
						materialname = subchild.getattribute('name', 'Unknown')
						action.materials.append([['1bdd','1bde','1bdf','1be0','1be1','1be2','1bd7','1bd8','1bd9','1bda','1bdb','1bdc'], amount, 'Boards, Logs'])

					# Consume all available materials scaled by the
					# amount of each submaterial
					elif subchild.name == 'stackable':
						action.stackable = 1

					elif subchild.name == 'nomark':
						action.markable = 0

					elif subchild.name == 'retaincolor':
						action.retaincolor = 1

					# Normal Material
					elif subchild.name == 'material':
						if not subchild.hasattribute('id'):
							console.log(LOG_ERROR, "Material element without id list in menu %s.\n" % menu.id)
							break
						else:
							ids = subchild.getattribute('id').split(';')
							try:
								amount = hex2dec(subchild.getattribute('amount', '1'))
								materialname = subchild.getattribute('name', 'Unknown')
							except:
								console.log(LOG_ERROR, "Material element with invalid id list in menu %s.\n" % menu.id)
								break
							action.materials.append([ids, amount, materialname])

					# Skill requirement
					elif subchild.name in skillnamesids:
						skill = skillnamesids[subchild.name]
						try:
							minimum = hex2dec(subchild.getattribute('min', '0'))
						except:
							console.log(LOG_ERROR, "%s element with invalid min value in menu %s.\n" % (subchild.name, menu.id))

						try:
							maximum = hex2dec(subchild.getattribute('max', '1200'))
						except:
							console.log(LOG_ERROR, "%s element with invalid max value in menu %s.\n" % (subchild.name, menu.id))

						try:
							penalty = hex2dec(subchild.getattribute('penalty','0'))
						except:
							console.log(LOG_ERROR, "%s element with invalid max value in menu %s.\n" % (subchild.name, menu.id))

						action.skills[skill] = [minimum, maximum, penalty]

	# Sort the menu. This is important for the makehistory to make.
	menu.sort()

def onLoad():
	loadMenu('TINKERING')
