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

# Leather types used by tailoring.
LEATHERS = [
		['Leather',					TAILORING, 0, ['leather_cut', 'leather_hides'], 0x0, 'leather'],
		['Spined Leather', 	TAILORING, 650, ['spined_leather_cut', 'spined_leather_hides'], 0x283, 'spined_leather'],
		['Horned Leather', 	TAILORING, 800, ['horned_leather_cut', 'horned_leather_hides'], 0x227, 'horned_leather'],
		['Barbed Leather', 	TAILORING, 990, ['barbed_leather_cut', 'barbed_leather_hides'], 0x1c1, 'barbed_leather'],
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
# Bring up the tailoring menu
#
def onUse(char, item):
	if not checktool(char, item):
		return 1

	menu = findmenu('TAILORING')
	if menu:
		menu.send(char, [item.serial])
	return 1

#
# Tailor an item.
#
class TailorItemAction(CraftItemAction):
	def __init__(self, parent, title, itemid, definition):
		CraftItemAction.__init__(self, parent, title, itemid, definition)
		self.markable = 1 # All tailoring items are markable

	#
	# Check if we did an exceptional job.
	#
	def getexceptionalchance(self, player, arguments):
		# Only works if this item requires blacksmithing
		if not self.skills.has_key(TAILORING):
			return 0

		minskill = self.skills[TAILORING][0]
		maxskill = self.skills[TAILORING][1]
		chance = ( (player.skill[TAILORING] - minskill) / (maxskill - minskill) ) / 10.0

		return chance

	#
	# Apply resname and color to the item.
	#
	def applyproperties(self, player, arguments, item, exceptional):
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

		# Reduce the uses remain count
		checktool(player, wolfpack.finditem(arguments[0]), 1)

	#
	# Check for the used tool.
	#
	def make(self, player, arguments):
		assert(len(arguments) > 0, 'Arguments has to contain a tool reference.')

		if not checktool(player, wolfpack.finditem(arguments[0])):
			return 0

		return CraftItemAction.make(self, player, arguments)

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
		#self.allowenhance = 1
		self.submaterials1 = LEATHERS
		self.submaterial1missing = 1044463
		self.submaterial1noskill = 1049311
		self.gumptype = 0x4f1ba411 # This should be unique

	#
	# Repair an item
	#
	def repair(self, player, arguments, target):
		if not checktool(player, wolfpack.finditem(arguments[0])):
			return

		if not target.item:
			player.socket.clilocmessage(500426)
			return

		if target.item.container != player.getbackpack():
			player.socket.clilocmessage(1044275)
			return
		
		item = target.item
		weapon = itemcheck(item, ITEM_WEAPON)
		shield = itemcheck(item, ITEM_SHIELD)
		armor = itemcheck(item, ITEM_ARMOR)

		if weapon or armor or shield:
			# Item in full repair
			if item.maxhealth <= 0 or item.health >= item.maxhealth:
				player.socket.clilocmessage(500423)
				return

			skill = player.skill[TAILORING]
			if skill >= 900:
				weaken = 1
			elif skill >= 700:
				weaken = 2
			else:
				weaken = 3

			action = self.findcraftitem(item.baseid)

			# We can't craft it, so we can't repair it.
			if not action:
				player.socket.clilocmessage(1044277)
				return

			# We will either destroy or repair it from here on
			# So we can play the craft effect.
			player.soundeffect(0x248)

			if item.maxhealth <= weaken:
				player.socket.clilocmessage(500424)
				item.delete()
			elif player.checkskill(TAILORING, 0, 1000):
				player.socket.clilocmessage(1044279)
				item.maxhealth -= weaken
				item.health = item.maxhealth
				item.resendtooltip()
			else:
				player.socket.clilocmessage(1044280)
				item.maxhealth -= weaken
				item.health = max(0, item.health - weaken)
				item.resendtooltip()

			# Warn the user if we'll break the item next time
			if item.maxhealth <= weaken:
				player.socket.clilocmessage(1044278)

			return

		player.socket.clilocmessage(1044277)

	#
	# Get the material used by the character from the tags
	#
	def getsubmaterial1used(self, player, arguments):
		if not player.hastag('tailoring_leather'):
			return 0
		else:
			material = int(player.gettag('tailoring_leather'))
			if material < len(self.submaterials1):
				return material
			else:
				return 0

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
				 
					# How much of the primary resource should be consumed
					if subchild.name == 'leather':
						action.submaterial1 = hex2dec(subchild.getattribute('amount', '0'))

					# Normal Material
					elif subchild.name == 'material':
						if not subchild.hasattribute('id'):
							console.log(LOG_ERROR, "Material element without id list in menu %s.\n" % menu.id)
							break
						else:
							ids = subchild.getattribute('id').split(';')
							try:
								amount = hex2dec(subchild.getattribute('amount', '1'))
							except:
								console.log(LOG_ERROR, "Material element with invalid id list in menu %s.\n" % menu.id)
								break
							action.materials.append([ids, amount])

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
# Load the blacksmithing menu.
#
def onLoad():
	loadMenu('TAILORING')
