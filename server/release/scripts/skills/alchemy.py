
import wolfpack
import math
import random
from wolfpack import console
from wolfpack.consts import ALCHEMY, LOG_ERROR, WPDT_MENU, WPDT_ITEM, skillnamesids
from wolfpack import properties
from system.makemenus import CraftItemAction, MakeMenu, findmenu
from wolfpack.utilities import hex2dec, tobackpack, createlockandkey


#
# Check if the character is using the right tool
#
def checktool(char, item, wearout = 0):
	if not item:
		return False

	# Has to be in our posession
	if item.getoutmostchar() != char:
		char.socket.clilocmessage(1044263)
		return False

	# We do not allow "invulnerable" tools.
	if not item.hastag('remaining_uses'):
		char.socket.clilocmessage(1044038)
		item.delete()
		return False

	if wearout:
		uses = int(item.gettag('remaining_uses'))
		if uses <= 1:
			char.socket.clilocmessage(1044038)
			item.delete()
			return False
		else:
			item.settag('remaining_uses', uses - 1)
			item.resendtooltip()

	return True

#
# Bring up the alchemy menu
#
def onUse(char, item):
	if not checktool(char, item):
		return True

	menu = findmenu('ALCHEMY')
	if menu:
		menu.send(char, [item.serial])
	return True

#
# Brew an item.
# Used for scales + ingots
#
class BrewItemAction(CraftItemAction):
	def __init__(self, parent, title, itemid, definition):
		CraftItemAction.__init__(self, parent, title, itemid, definition)
		self.markable = 1 # All alchemy items are not markable

	#
	# No exceptional alchemist items.
	#
	def getexceptionalchance(self, player, arguments):
		return False

	#
	# Play the "grind" sound when we fail to create a potion
	#
	def fail(self, player, arguments, lostmaterials):
		player.socket.clilocmessage(500287)

		# Re-create the empty bottle in the users backpack
		if lostmaterials:
			bottle = wolfpack.additem('f0e')
			if not wolfpack.utilities.tobackpack(bottle, player):
				bottle.update()

	#
	# Print a nicer success message
	#
	def success(self, player, arguments, item, exceptional=0, marked=0):
		player.soundeffect(0x240)
		player.socket.clilocmessage(500279)

	#
	# There are no special properties
	# but we need to wear out the tool
	#
	def applyproperties(self, player, arguments, item, exceptional):
		checktool(player, wolfpack.finditem(arguments[0]), 1)

	#
	# Check for the used tool.
	#
	def make(self, player, arguments, nodelay=0):
		assert(len(arguments) > 0, 'Arguments has to contain a tool reference.')

		if not checktool(player, wolfpack.finditem(arguments[0])):
			return False

		return CraftItemAction.make(self, player, arguments, nodelay)

	#
	# Play a simple soundeffect
	#
	def playcrafteffect(self, player, arguments):
		player.soundeffect(0x242)

#
# An alchemy menu.
#
class AlchemyMenu(MakeMenu):
	def __init__(self, id, parent, title):
		MakeMenu.__init__(self, id, parent, title)
		self.allowmark = 0
		self.delay = 2000
		self.gumptype = 0x9ca51fca # This should be unique

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
	menu = AlchemyMenu(id, parent, name)

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
		elif child.name == 'craft':
			if not child.hasattribute('definition') or not child.hasattribute('name'):
				console.log(LOG_ERROR, "craft action without definition or name in menu %s.\n" % menu.id)
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
				except Exception, e:
					console.log(LOG_ERROR, "Craft action with invalid item id in menu %s: %s\n" % (menu.id, str(e)))

				action = BrewItemAction(menu, name, int(itemid), itemdef)

				# Process subitems
				for j in range(0, child.childcount):
					subchild = child.getchild(j)

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
# Load the alchemy menu.
#
def onLoad():
	loadMenu('ALCHEMY')
