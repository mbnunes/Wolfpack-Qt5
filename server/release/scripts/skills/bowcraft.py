#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by: Incanus                        #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack import console
from wolfpack.consts import *
from wolfpack import properties, settings, getdefinition
from math import ceil
from system.makemenus import CraftItemAction, MakeMenu, findmenu, generateNamefromDef
from wolfpack.utilities import hex2dec, tobackpack, tr
from wolfpack.properties import itemcheck, fromitem

extended_carpentry = int( settings.getbool("General", "Extended Carpentry", False, True) )

if extended_carpentry:
	WOOD = [
			[tr('Beech Wood'), BOWCRAFT, 0, ['1bd7', '1bda', '1bdd', '1bde', '1bdf', '1be0', '1be1', '1be2'], 0, 'beech'],
			[tr('Apple Wood'), BOWCRAFT, 300, ['board_apple', 'log_apple'], 1748, 'apple'],
			[tr('Peach Wood'), BOWCRAFT, 350, ['board_peach', 'log_peach'], 1856, 'peach'],
			[tr('Pear Wood'), BOWCRAFT, 400, ['board_pear', 'log_pear'], 1002, 'pear'],
			[tr('Cedar Wood'), BOWCRAFT, 450, ['board_cedar', 'log_cedar'], 2107, 'cedar'],
			[tr('Willow Wood'), BOWCRAFT, 500, ['board_willow', 'log_willow'], 1513, 'willow'],
			[tr('Cypress Wood'), BOWCRAFT, 550, ['board_cypress', 'log_cypress'], 2413, 'cypress'],
			[tr('Oak Wood'), BOWCRAFT, 600, ['board_oak', 'log_oak'], 1049, 'oak'],
			[tr('Walnut Wood'), BOWCRAFT, 650, ['board_walnut', 'log_walnut'], 1839, 'walnut'],
			[tr('Yew Wood'), BOWCRAFT, 700, ['board_yew', 'log_yew'], 1502, 'yew'],
			[tr('Tropical Wood'), BOWCRAFT, 750, ['board_tropical', 'log_tropical'], 1889, 'tropical']
	]
else:
	WOOD = [
			[tr('Beech Wood'), BOWCRAFT, 0, ['1bd7', '1bda', '1bdd', '1bde', '1bdf', '1be0', '1be1', '1be2'], 0, 'beech'],
	]

#
# Display fletching craftmenu.
#
def onUse(player, item):
	menu = findmenu('BOWCRAFT')
	if menu:
		menu.send(player, [item.serial])
	return True

#
# Craft an item.
#
class FletchItemAction(CraftItemAction):
	def __init__(self, parent, title, itemid, definition):
		CraftItemAction.__init__(self, parent, title, itemid, definition)
		self.markable = True
		self.stackable = False

	#
	# Check if we did an exceptional job.
	#
	def getexceptionalchance(self, player, arguments):
		if not self.skills.has_key(BOWCRAFT):
			return False

		minskill = self.skills[BOWCRAFT][0]
		maxskill = self.skills[BOWCRAFT][1]
		penalty = self.skills[BOWCRAFT][2]
		if not penalty:
			penalty = 0

		minskill += penalty
		maxskill += penalty
		chance = ( player.skill[BOWCRAFT] - (minskill *0.65) ) / 10

		if chance > 100:
			chance = 100
		elif chance < 0:
			chance = 0

		# chance range 0.00 - 1.00
		chance = chance * .01
		return chance

	#
	# Apply resname and color to the item.
	#
	def applyproperties(self, player, arguments, item, exceptional):
		# Use all available resources if the item we make is
		# flagged as "stackable".
		if self.stackable:
			backpack = player.getbackpack()
			count = -1
			# count  how many items we could make
			for (materials, amount, name) in self.materials:
				items = backpack.countitems(materials)
				if count == -1:
					count = items / amount
				else:
					count = min(count, items / amount)

			# count  how many items we could make
			if( self.submaterial1 > 0 ):
				material = self.parent.getsubmaterial1used(player, arguments)
				material = self.parent.submaterials1[material]
				items = backpack.countitems(material[3])
				if count == -1:
					count = items / self.submaterial1
				else:
					count = min(count, items / self.submaterial1)

			# consume material
			for (materials, amount, name) in self.materials:
				backpack.removeitems( materials, int(count * amount) )

			# consume material
			if self.submaterial1 > 0:
				material = self.parent.getsubmaterial1used(player, arguments)
				material = self.parent.submaterials1[material]
				backpack.removeitems( material[3], int(count * self.submaterial1) )

			if count != -1:
				item.amount += count
			else:
				item.amount = 1 + count
			item.update()

			# Resend weight
			player.socket.resendstatus()

		else:
			if extended_carpentry:
				if self.submaterial1 > 0:
					material = self.parent.getsubmaterial1used(player, arguments)
					material = self.parent.submaterials1[material]
					item.color = material[4]
					item.settag('resname', material[5])

			# Apply one-time boni
			healthbonus = properties.fromitem(item, DURABILITYBONUS)
			if healthbonus != 0:
				bonus = int( ceil(item.maxhealth * (healthbonus / 100.0)) )
				item.maxhealth = max(1, item.maxhealth + bonus)
				item.health = item.maxhealth

			# increase damage of exceptional weapons
			if exceptional:
				if itemcheck(item, ITEM_WEAPON):
					# Increase the damage bonus by 20%
					bonus = fromitem(item, DAMAGEBONUS)
					bonus += 20
					item.settag('aos_boni_damage', bonus)

	#
	# Play a simple soundeffect
	#
	def playcrafteffect(self, player, arguments):
		player.soundeffect(0x55)

#
# The user has to have Samurai Empire installed
#
class SeFletchItemAction(FletchItemAction):
	def __init__(self, parent, title, itemid, definition):
		FletchItemAction.__init__(self, parent, title, itemid, definition)

	def visible(self, char, arguments):
		if char.socket and char.socket.flags & 0x10 == 0:
			return False
		else:
			return FletchItemAction.visible(self, char, arguments)

	def checkmaterial(self, player, arguments, silent = 0):
		if player.socket and player.socket.flags & 0x10 == 0:
			return False
		else:
			return FletchItemAction.checkmaterial(self, player, arguments, silent)

#
# The bowcraft menu.
#
class BowcraftMenu(MakeMenu):
	def __init__(self, id, parent, title):
		MakeMenu.__init__(self, id, parent, title)
		self.allowmark = True
		self.allowrepair = True
		self.submaterials1 = WOOD
		self.submaterial1missing = 1042081
		self.submaterial1noskill = 500298
		self.gumptype = 0x4f1bff56 # This should be unique
		self.requiretool = True

	#
	# Get the material used by the character from the tags
	#
	def getsubmaterial1used(self, player, arguments):
		if not player.hastag('bowcraft_wood'):
			return False
		else:
			material = int(player.gettag('bowcraft_wood'))
			if material < len(self.submaterials1):
				return material
			else:
				return False

	#
	# Save the material preferred by the user in a tag
	#
	def setsubmaterial1used(self, player, arguments, material):
		player.settag('bowcraft_wood', material)

	#
	# Repair an item
	#
	def repair(self, player, arguments, target):
		if not target.item:
			player.socket.clilocmessage(500426)
			return

		if target.item.container != player.getbackpack():
			player.socket.clilocmessage(1044275)
			return

		item = target.item
		weapon = itemcheck(item, ITEM_WEAPON)

		if weapon:
			# Item in full repair
			if item.maxhealth <= 0 or item.health >= item.maxhealth:
				player.socket.clilocmessage(500423)
				return

			skill = player.skill[BOWCRAFT]
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
			player.soundeffect(0x55)

			if item.maxhealth <= weaken:
				player.socket.clilocmessage(500424)
				item.delete()
			elif player.checkskill(BOWCRAFT, 0, 1000):
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
# Load a menu with a given id and
# append it to the parents submenus.
#
def loadMenu(id, parent = None):
	definition = getdefinition(WPDT_MENU, id)
	if not definition:
		if parent:
			console.log(LOG_ERROR, "Unknown submenu %s in menu %s.\n" % (id, parent.id))
		else:
			console.log(LOG_ERROR, "Unknown menu: %s.\n" % id)
		return

	name = definition.getattribute('name', '')
	menu = BowcraftMenu(id, parent, name)

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
		elif child.name in ['fletch', 'sefletch']:
			if not child.hasattribute('definition'):
				console.log(LOG_ERROR, "Bowcraft action without definition in menu %s.\n" % menu.id)
			else:
				itemdef = child.getattribute('definition')
				try:
					# See if we can find an item id if it's not given
					if not child.hasattribute('itemid'):
						item = getdefinition(WPDT_ITEM, itemdef)
						itemid = 0
						if item:
							itemchild = item.findchild('id')
							if itemchild:
								itemid = itemchild.value
						else:
							console.log(LOG_ERROR, "Bowcraft action with invalid definition %s in menu %s.\n" % (itemdef, menu.id))
					else:
						itemid = hex2dec(child.getattribute('itemid', '0'))
					if child.hasattribute('name'):
						name = child.getattribute('name')
					else:
						name = generateNamefromDef(itemdef)
					if child.name == 'sefletch':
						action = SeFletchItemAction(menu, name, int(itemid), itemdef)
					else:
						action = FletchItemAction(menu, name, int(itemid), itemdef)
				except:
					console.log(LOG_ERROR, "Bowcraft action with invalid item id in menu %s.\n" % menu.id)

				# Process subitems
				for j in range(0, child.childcount):
					subchild = child.getchild(j)

					# How much of the primary resource should be consumed
					if subchild.name == 'logs':
						action.submaterial1 = hex2dec(subchild.getattribute('amount', '0'))

					# Normal Material
					if subchild.name == 'material':
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

					# This item will never be exceptional
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

						try:
							penalty = hex2dec(subchild.getattribute('penalty', '0'))
						except:
							console.log(LOG_ERROR, "%s element with invalid penalty value in menu %s.\n" % (subchild.name, menu.id))

						action.skills[skill] = [minimum, maximum, penalty]

	# Sort the menu. This is important for the makehistory to make.
	menu.sort()

#
# Load the bowcraft menu.
#
def onLoad():
	loadMenu('BOWCRAFT')
