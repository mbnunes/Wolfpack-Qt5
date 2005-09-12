from wolfpack import console
from wolfpack.consts import *
from wolfpack import properties
from wolfpack.utilities import tr
import math
import wolfpack
from system.makemenus import CraftItemAction, MakeMenu, findmenu, generateNamefromDef
from wolfpack.utilities import hex2dec, tobackpack, createlockandkey
import random
import skills.blacksmithing
from copy import deepcopy

extended_carpentry = int( wolfpack.settings.getbool("General", "Extended Carpentry", False, True) )

# Use metals list from Blacksmithing, but test for carpentry skill
CARPENTRYMETALS = deepcopy(skills.blacksmithing.METALS)
for metal in CARPENTRYMETALS:
	metal[1] = CARPENTRY

if extended_carpentry:
	WOOD = [
			[tr('Beech Wood'), CARPENTRY, 0, ['1bd7', '1bda'], 0, 'beech'],
			[tr('Apple Wood'), CARPENTRY, 300, ['board_apple'], 1748, 'apple'],
			[tr('Peach Wood'), CARPENTRY, 350, ['board_peach'], 1856, 'peach'],
			[tr('Pear Wood'), CARPENTRY, 400, ['board_pear'], 1002, 'pear'],
			[tr('Cedar Wood'), CARPENTRY, 450, ['board_cedar'], 2107, 'cedar'],
			[tr('Willow Wood'), CARPENTRY, 500, ['board_willow'], 1513, 'willow'],
			[tr('Cypress Wood'), CARPENTRY, 550, ['board_cypress'], 2413, 'cypress'],
			[tr('Oak Wood'), CARPENTRY, 600, ['board_oak'], 1049, 'oak'],
			[tr('Walnut Wood'), CARPENTRY, 650, ['board_walnut'], 1839, 'walnut'],
			[tr('Yew Wood'), CARPENTRY, 700, ['board_yew'], 1502, 'yew'],
			[tr('Tropical Wood'), CARPENTRY, 750, ['board_tropical'], 1889, 'tropical']
	]
else:
	WOOD = [
			[tr('Wood'), CARPENTRY, 0, ['1bd7', '1bda'], 0, 'wood'],
	]
	
#
# Bring up the carpentry menu
#
def onUse(char, item):
	menu = findmenu('CARPENTRY')
	if menu:
		menu.send(char, [item.serial])
	return True

#
# Carp an item.
#
class CarpItemAction(CraftItemAction):
	def __init__(self, parent, title, itemid, definition):
		CraftItemAction.__init__(self, parent, title, itemid, definition)
		self.markable = 1 # All carpentry items are markable
		self.stackable = 0

	#
	# Check if we did an exceptional job.
	#
	def getexceptionalchance(self, player, arguments):
		# Only works if this item requires carpentry
		if not self.skills.has_key(CARPENTRY):
			return False

		minskill = self.skills[CARPENTRY][0]
		maxskill = self.skills[CARPENTRY][1]

		if self.skills.has_key(TINKERING):
			minskill -= self.skills[TINKERING][0]
			maxskill -= self.skills[TINKERING][1]

		if minskill == maxskill or maxskill == 0:
			maxskill += 1

		chance = ( (player.skill[CARPENTRY]) / (maxskill) ) / 5.0

		return chance

	#
	# Apply resname and color to the item.
	#
	def applyproperties(self, player, arguments, item, exceptional):
		# Stackable items consume all resources
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
		
		# All carpentry items crafted out of ingots keep a resname
		if self.submaterial1 > 0:
			material = self.parent.getsubmaterial1used(player, arguments)
			material = self.parent.submaterials1[material]
			item.color = material[4]
			item.settag('resname', material[5])

		if self.submaterial2 > 0:
			material = self.parent.getsubmaterial2used(player, arguments)
			material = self.parent.submaterials2[material]
			if not self.submaterial1:
				item.color = material[4]
			item.settag('resname2', material[5])

		# Apply one-time boni
		healthbonus = properties.fromitem(item, DURABILITYBONUS)
		if healthbonus != 0:
			bonus = int(math.ceil(item.maxhealth * (healthbonus / 100.0)))
			item.maxhealth = max(1, item.maxhealth + bonus)
			item.health = item.maxhealth
				
		# Distribute another 6 points randomly between the resistances an armor alread
		# has. There are no tailored weapons.
		if exceptional:
			if properties.itemcheck(item, ITEM_SHIELD):
				# Copy all the values to tags
				boni = [0, 0, 0, 0, 0]

				for i in range(0, 6):
					boni[random.randint(0,4)] += 1

				item.settag('res_physical', properties.fromitem(item, RESISTANCE_PHYSICAL) + boni[0])
				item.settag('res_fire', properties.fromitem(item, RESISTANCE_FIRE) + boni[1])
				item.settag('res_cold', properties.fromitem(item, RESISTANCE_COLD) + boni[2])
				item.settag('res_energy', properties.fromitem(item, RESISTANCE_ENERGY) + boni[3])
				item.settag('res_poison', properties.fromitem(item, RESISTANCE_POISON) + boni[4])

			elif properties.itemcheck(item, ITEM_WEAPON):
				# Increase the damage bonus by 20%
				bonus = properties.fromitem(item, DAMAGEBONUS)
				bonus += 20
				item.settag('aos_boni_damage', bonus)

			# Exceptional Containers get a key
			if item.type == 1:
				if item.baseid == '9aa' or item.baseid == '9a9' or item.baseid == 'e3f' or item.baseid == 'e3d' or item.baseid == 'e42' or item.baseid == 'a4d' or item.baseid == 'a4f':
					createlockandkey( item )

	#
	# Check for the used tool.
	#
	def make(self, player, arguments, nodelay=0):
		assert(len(arguments) > 0, 'Arguments has to contain a tool reference.')

		return CraftItemAction.make(self, player, arguments, nodelay)

	#
	# Play a simple soundeffect
	#
	def playcrafteffect(self, player, arguments):
		player.soundeffect(0x23d)

#
# The user has to have Samurai Empire installed
#
class SeCarpItemAction(CarpItemAction):
	def __init__(self, parent, title, itemid, definition):
		CarpItemAction.__init__(self, parent, title, itemid, definition)

	def visible(self, char, arguments):
		if char.socket and char.socket.flags & 0x10 == 0:
			return False
		else:
			return CarpItemAction.visible(self, char, arguments)

	def checkmaterial(self, player, arguments, silent = 0):
		if player.socket and player.socket.flags & 0x10 == 0:
			return False
		else:
			return CarpItemAction.checkmaterial(self, player, arguments, silent)

#
# A carpentry menu. The most notable difference is the
# button for selecting another ore.
#
class CarpentryMenu(MakeMenu):
	def __init__(self, id, parent, title):
		MakeMenu.__init__(self, id, parent, title)
		self.allowmark = True
		self.allowrepair = False
		self.submaterials1 = WOOD
		self.submaterials2 = CARPENTRYMETALS
		self.submaterial1missing = 1042081 # Boards
		self.submaterial1noskill = 500298
		self.submaterial2missing = 1042081 # Ingots
		self.submaterial2noskill = 500298
		self.gumptype = 0x466b5b1a # This should be unique
		self.requiretool = True

	#
	# Get the material used by the character from the tags
	#
	def getsubmaterial1used(self, player, arguments):
		if not player.hastag('carpentry_wood'):
			return False
		else:
			material = int(player.gettag('carpentry_wood'))
			if material < len(self.submaterials1):
				return material
			else:
				return False

	def getsubmaterial2used(self, player, arguments):
		if not player.hastag('carpentry_ore'):
			return False
		else:
			material = int(player.gettag('carpentry_ore'))
			if material < len(self.submaterials2):
				return material
			else:
				return False

	#
	# Save the material preferred by the user in a tag
	#
	def setsubmaterial1used(self, player, arguments, material):
		player.settag('carpentry_wood', material)

	def setsubmaterial2used(self, player, arguments, material):
		player.settag('carpentry_ore', material)

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
	menu = CarpentryMenu(id, parent, name)

	# See if we have any submenus
	for i in range(0, definition.childcount):
		child = definition.getchild(i)
		# Submenu
		if child.name == 'menu':
			if not child.hasattribute('id'):
				console.log(LOG_ERROR, "Submenu with missing id attribute in menu %s.\n" % menu.id)
			else:
				menuid = child.getattribute('id')
				# Don't show boards menu for simple carpentry
				if extended_carpentry or menuid != 'CARPENTRY_BOARDS':			
					loadMenu( menuid, menu )

		# Craft an item
		elif child.name in ['carpenter', 'secarpenter']:
			if not child.hasattribute('definition'):
				console.log(LOG_ERROR, "Carpenter action without definition in menu %s.\n" % menu.id)
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
							console.log(LOG_ERROR, "Carpenter action with invalid definition %s in menu %s.\n" % (itemdef, menu.id))
					else:
						itemid = hex2dec(child.getattribute('itemid', '0'))
					if child.hasattribute('name'):
						name = child.getattribute('name')
					else:
						name = generateNamefromDef(itemdef)
					if child.name == 'secarpenter':
						action = SeCarpItemAction(menu, name, int(itemid), itemdef)
					else:
						action = CarpItemAction(menu, name, int(itemid), itemdef)
				except:
					console.log(LOG_ERROR, "Carpenter action with invalid item id in menu %s.\n" % menu.id)

				# Process subitems
				for j in range(0, child.childcount):
					subchild = child.getchild(j)

					# How much of the primary resource should be consumed
					if subchild.name == 'boards':
						action.submaterial1 = hex2dec(subchild.getattribute('amount', '0'))
					# How much of the secondary resource should be consumed
					if subchild.name == 'ingots':
						action.submaterial2 = hex2dec(subchild.getattribute('amount', '0'))

					# Normal Material
					if subchild.name == 'wood' or subchild.name == 'cloth' or subchild.name == 'material':
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

					# Consume all available materials scaled by the
					# amount of each submaterial
					elif subchild.name == 'stackable':
						action.stackable = True

					elif subchild.name == 'nomark':
						action.markable = False

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
						action.skills[skill] = [minimum, maximum]
	# Sort the menu. This is important for the makehistory to make.
	menu.sort()

#
# Load the carpentry menu.
#
def onLoad():
	loadMenu('CARPENTRY')
