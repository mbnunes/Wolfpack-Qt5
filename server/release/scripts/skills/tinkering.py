
#################################################################
#	 )			(\_		 # WOLFPACK 13.0.0 Scripts										#
#	((		_/{	"-;	# Created by: DarkStorm											#
#	 )).-' {{ ;'`	 # Revised by:																#
#	( (	;._ \\ ctr # Last Modification: Created								 #
#################################################################

from wolfpack import console
from wolfpack.consts import *
import math
import wolfpack
from system.makemenus import CraftItemAction, MakeMenu, findmenu
from wolfpack.utilities import hex2dec, tobackpack
from wolfpack.properties import itemcheck, fromitem
import random
from skills import blacksmithing

GEMS = [
		['Star Sapphire', 0, 0, ['f0f', 'f1b', 'f21'], 0x0, 'starsapphire'],
		['Emerald', 0, 0, ['f10', 'f2f'], 0, 'emerald'],
		['Sapphire', 0, 0, ['f11', 'f12', 'f19', 'f1f'], 0, 'sapphire'],
		['Ruby', 0, 0, ['f13', 'f14', 'f1a', 'f1c', 'f1d', 'f2a', 'f2b'], 0, 'ruby'],
		['Bronze',			0, 0, ['bronze_ingot'], 0x972, 'bronze'],
		['Gold',				0, 0, ['gold_ingot'], 0x8a5, 'gold'],
		['Agapite',		 	0, 0, ['agapite_ingot'], 0x979, 'agapite'],
		['Verite',			0, 0, ['verite_ingot'], 0x89f, 'verite'],
		['Valorite',		0, 0, ['valorite_ingot'], 0x8ab, 'valorite'],
]

#
# Check if the character is using the right tool
#
def checktool(char, item, wearout = 0):
	if not item:
		return 0

	# Has to be in our posession
	if item.getoutmostchar() != char:
		char.socket.clilocmessage(500364)
		return 0

	# We do not allow "invulnerable" tools.
	if not item.hastag('remaining_uses'):
		char.socket.clilocmessage(1044038)
		item.delete()
		return 0

	if wearout:
		uses = int(item.gettag('remaining_uses'))
		if uses <= 1:
			char.socket.clilocmessage(1044038)
			item.delete()
			return 0
		else:
			item.settag('remaining_uses', uses - 1)

	return 1

#
# Bring up the tinkering menu
#
def onUse(char, item):
	if not checktool(char, item):
		return 1

	menu = findmenu('TINKERING')
	if menu:
		menu.send(char, [item.serial])
	return 1

class TinkerItemAction(CraftItemAction):
	def __init__(self, parent, title, itemid, definition):
		CraftItemAction.__init__(self, parent, title, itemid, definition)
		self.markable = 1
		self.retaincolor = 0

	#
	# Check if we did an exceptional job.
	#
	def getexceptionalchance(self, player, arguments):
		if not self.markable:
			return 0

		if not self.skills.has_key(TINKERING):
			return 0

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

		# See if this item
		if self.retaincolor and self.submaterial1 > 0:
			material = self.parent.getsubmaterial1used(player, arguments)
			material = self.parent.submaterials1[material]
			item.color = material[4]
			item.settag('resname', material[5])

		# Apply one-time boni
		healthbonus = fromitem(item, DURABILITYBONUS)
		if healthbonus != 0:
			bonus = int(math.ceil(item.maxhealth * (healthbonus / 100.0)))
			item.maxhealth = max(1, item.maxhealth + bonus)
			item.health = item.maxhealth

		# Reduce the uses remain count
		checktool(player, wolfpack.finditem(arguments[0]), 1)

	#
	# First check if we are near an anvil and forge.
	#
	def make(self, player, arguments, nodelay=0):
		assert(len(arguments) > 0, 'Arguments has to contain a tool reference.')

		if not checktool(player, wolfpack.finditem(arguments[0])):
			return 0

		return CraftItemAction.make(self, player, arguments, nodelay)

	#
	# Play a simple soundeffect
	#
	def playcrafteffect(self, player, arguments):
		player.soundeffect(0x2a)

class TinkeringMenu(MakeMenu):
	def __init__(self, id, parent, title):
		MakeMenu.__init__(self, id, parent, title)
		self.allowmark = 1
		#self.allowrepair = 1
		self.submaterials1 = blacksmithing.METALS
		self.submaterials2 = GEMS
		self.submaterial1missing = 1044037
		self.submaterial1noskill = 1044268
		self.submaterial2missing = 1044240
		self.gumptype = 0x41afb410 # This should be unique

	#
	# Get the material used by the character from the tags
	#
	def getsubmaterial1used(self, player, arguments):
		if not player.hastag('blacksmithing_ore'):
			return 0
		else:
			material = int(player.gettag('blacksmithing_ore'))
			if material < len(self.submaterials1):
				return material
			else:
				return 0

	#
	# Save the material preferred by the user in a tag
	#
	def setsubmaterial1used(self, player, arguments, material):
		player.settag('blacksmithing_ore', material)

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
		elif child.name == 'tinker':
			if not child.hasattribute('definition') or not child.hasattribute('name'):
				console.log(LOG_ERROR, "Tinker action without definition or name in menu %s.\n" % menu.id)
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
