
import wolfpack
from wolfpack.utilities import tobackpack, ObjectWrapper
from wolfpack.gumps import cGump
import math
from wolfpack import console
from wolfpack.consts import *
import random

# Known menus
menus = {}

# Return a menu
def findmenu(id):
	global menus
	if not menus.has_key(id):
		return None
	else:
		return menus[id]

# Constants
whitehtml = '<basefont color="#FFFFFF">%s'
grayhtml = '<basefont color="#999999">%s'
centerhtml = '<basefont color="#FFFFFF"><div align="center">%s</div>'

#
# Response action for a MakeAction Details gump.
#
def MakeActionResponse(player, arguments, response):
	(menu, action) = arguments[0].split(':')
	action = int(action)
	global menus
	if menus.has_key(menu):
		menu = menus[menu]
		if action < len(menu.subactions):
			menu.subactions[action].response(player, response, list(arguments)[1:])

#
# This class encapsulates all the functionality related to
# an item you can select in one of the MakeMenu categories.
#
class MakeAction:
	#
	# Constructor for an "action" inside of a submenu.
	# This is a baseclass. Override the make() and
	# details() methods.
	#
	def __init__(self, parent, title):
		assert(parent)
		self.parent = parent
		self.title = title
		self.parent.subactions.append(self)
		self.hasdetails = 0

	#
	# Checks if this option is visible to
	# the given character.
	#
	def visible(self, char, arguments):
		return 1

	#
	# Default response function (does nothing).
	#
	def response(self, player, response, arguments):
		pass

	#
	# Executes this action.
	# Arguments are the arguments passed down from the makemenu.
	#
	def make(self, player, arguments, nodelay=0):
		if self.index == -1:
			return

		# Append this action to the players "last10" history
		history = []
		if self.parent.gumptype != 0:
			historyname = 'makehistory%u' % self.parent.gumptype
		else:
			historyname = 'makehistory'
		identifier = '%s:%u' % (self.parent.id, self.index)
		if player.hastag(historyname):
			history = unicode(player.gettag(historyname)).split(';')
			if identifier in history:
				history.remove(identifier)

		history = [identifier] + history[:9]
		player.settag(historyname, ';'.join(history))

	#
	# Shows a detail page for this action.
	# This is optional. Arguments are the arguments
	# passed down from the makemenu instance.
	#
	def details(self, player, arguments):
		pass

#
# This action creates an item and shows a
# detail page for it.
#
class MakeItemAction(MakeAction):
	#
	# Creates a MakeItemAction instance.
	#
	def __init__(self, parent, title, itemid, definition, amount = 1):
		MakeAction.__init__(self, parent, title)
		self.itemid = itemid
		self.definition = str(definition)
		self.amount = amount
		self.otherhtml = ''
		self.materialshtml = ''
		self.skillshtml = ''
		self.hasdetails = 1

	#
	# Create the item in the players backpack.
	#
	def make(self, player, arguments, nodelay=0):
		item = wolfpack.additem(self.definition)
		if not item:
			console.log(LOG_ERROR, "Unknown item definition used in action %u of menu %s.\n" % \
				(self.parent.subactions.index(self), self.parent.id))
		else:
			if self.amount > 0:
				item.amount = self.amount
				player.log(LOG_MESSAGE, "Created %u items %s (0x%x).\n" % (self.amount, self.definition, item.serial))
			else:
				player.log(LOG_MESSAGE, "Created item %s (0x%x).\n" % (self.definition, item.serial))
			if not tobackpack(item, player):
				item.update()			
			player.socket.sysmessage('You put the new item into your backpack.')
			MakeAction.make(self, player, arguments, nodelay)

	#
	# Process a response from the details gump.
	#
	def response(self, player, response, arguments):
		if response.button == 0:
			self.parent.send(player, arguments)
		elif response.button == 1:
			self.make(player, arguments)

	#
	# Shows a detail page for this action.
	#
	def details(self, player, arguments):
		if self.parent.gumptype != 0:
			player.socket.closegump(self.parent.gumptype, 0xFFFF)

		gump = cGump()
		gump.setType(self.parent.gumptype)
		gump.setArgs(['%s:%u' % (self.parent.id, self.parent.subactions.index(self))] + arguments)
		gump.setCallback("system.makemenus.MakeActionResponse")

		gump.addResizeGump(0, 0, 5054, 530, 417)
		gump.addTiledGump(10, 10, 510, 22, 2624)
		gump.addTiledGump(165,	37, 355, 88, 2624)
		gump.addTiledGump(165, 130, 355, 80, 2624)
		gump.addTiledGump(165, 215, 355, 80, 2624)
		gump.addTiledGump(165, 300, 355, 80, 2624)
		gump.addTiledGump(10,	37, 150, 88, 2624)
		gump.addTiledGump(10, 130, 150, 22, 2624)
		gump.addTiledGump(10, 215, 150, 22, 2624)
		gump.addTiledGump(10, 300, 150, 22, 2624)
		gump.addCheckerTrans(10, 10, 510, 397)

		gump.addHtmlGump(10, 12, 510, 20, centerhtml % self.title)
		if self.itemid != 0:
			gump.addTilePic(15, 42, self.itemid)
		gump.addHtmlGump(10, 132, 150, 20, centerhtml % "SKILLS")
		gump.addHtmlGump(10, 217, 150, 20, centerhtml % "MATERIALS")
		gump.addHtmlGump(10, 302, 150, 20, centerhtml % "OTHER")
		gump.addHtmlGump(170, 39, 70, 20, whitehtml % "ITEM")
		gump.addButton(15, 387, 0xFAE, 0xFB0, 0) # Back to the parent menu of this node
		gump.addText(50, 389, "Back", 0x480)
		gump.addButton(375, 387, 4005, 4007, 1) # Make the item
		gump.addText(410, 389, "Make Now", 0x480)

		# Item Name
		gump.addText(245, 39, self.title, 0x480)

		# Scrollable Skill List
		gump.addHtmlGump(170, 132, 345, 76, whitehtml % self.skillshtml, 0, self.skillshtml.count('<br>') > 4)

		# Scrollable Material List
		gump.addHtmlGump(170, 217, 345, 76, whitehtml % self.materialshtml, 0, self.materialshtml.count('<br>') > 4)

		# Description
		gump.addHtmlGump(170, 302, 345, 76, whitehtml % self.otherhtml, 0, 1)

		gump.send(player)

#
# Action for crafting an item.
# This takes materials and skills into account.
#
class CraftItemAction(MakeItemAction):
	def __init__(self, parent, title, itemid, definition, amount = 1):
		MakeItemAction.__init__(self, parent, title, itemid, definition, amount)
		self.skills = {} # SKILL: [min, max]
		self.submaterial1 = 0 # The amount of submaterial1 required
		self.submaterial2 = 0 # The amount of submaterial2 required
		self.materials = [] # [baseids], [amount]
		self.markable = 0
		# Sysmessage sent if you don't have enough material.
		# Integer or String allowed
		self.lackmaterial = "You don't have enough material to make that."

	#
	# Make invisible if we dont have the minimum
	# skill requirements yet
	#
	def visible(self, player, arguments):
		return self.checkskills(player, arguments, 0)

	#
	# Returns true if the player has the required material.
	#
	def checkmaterial(self, player, arguments, silent = 0):
		backpack = player.getbackpack()

		# See if we have enough of the two
		# user selectable materials and check if we
		# can even craft them too
		if self.submaterial1 > 0:
			materials = self.parent.submaterials1
			material = self.parent.getsubmaterial1used(player, arguments)
			if material >= len(materials):
				if not silent:
					player.socket.sysmessage("You try to craft with an invalid material.")
				return 0
			material = materials[material]
			# Check the skill requirement of the material.
			if material[2] and player.skill[material[1]] < material[2]:
				if not silent:
					if self.parent.submaterial1noskill != 0:
						player.socket.clilocmessage(self.parent.submaterial1noskill)
					else:
						player.socket.clilocmessage(1044153)
				return 0

			# Check the required amount of the material
			count = backpack.countitems(material[3])
			if count < self.submaterial1:
				if not silent:
					if self.parent.submaterial1missing != 0:
						player.socket.clilocmessage(self.parent.submaterial1missing)
					else:
						player.socket.sysmessage(self.lackmaterial)
				return 0

		if self.submaterial2 > 0:
			materials = self.parent.submaterials2
			material = self.parent.getsubmaterial2used(player, arguments)
			if material >= len(materials):
				if not silent:
					player.socket.sysmessage("You try to craft with an invalid material.")
				return 0
			material = materials[material]
			# Check the skill requirement of the material.
			if material[2] and player.skill[material[1]] < material[2]:
				if not silent:
					if self.parent.submaterial2noskill != 0:
						player.socket.clilocmessage(self.parent.submaterial2noskill)
					else:
						player.socket.clilocmessage(1044153)
				return 0

			# Check the required amount of the material
			count = backpack.countitems(material[3])
			if count < self.submaterial2:
				if not silent:
					if self.parent.submaterial2missing != 0:
						player.socket.clilocmessage(self.parent.submaterial2missing)
					else:
						player.socket.sysmessage(self.lackmaterial)
				return 0

		for material in self.materials:
			(baseids, amount) = material[:2]
			count = backpack.countitems(baseids)
			if count < amount:
				if not silent:
					player.socket.sysmessage(self.lackmaterial)
				return 0

		return 1

	#
	# Consume material to make the item.
	#
	def consumematerial(self, player, arguments, half = 0):
		backpack = player.getbackpack()

		if self.submaterial1 > 0:
			materials = self.parent.submaterials1
			material = self.parent.getsubmaterial1used(player, arguments)
			if half:
				amount = int(math.ceil(self.submaterial1 * 0.5))
			else:
				amount = self.submaterial1
			count = backpack.removeitems(materials[material][3], amount)
			if count != 0:
				return 0

		if self.submaterial2 > 0:
			materials = self.parent.submaterials2
			material = self.parent.getsubmaterial2used(player, arguments)
			if half:
				amount = int(math.ceil(self.submaterial2 * 0.5))
			else:
				amount = self.submaterial2
			count = backpack.removeitems(materials[material][3], amount)
			if count != 0:
				return 0

		for material in self.materials:
			(baseids, amount) = material[:2]
			if half:
				amount = int(math.ceil(amount * 0.5))
			count = backpack.removeitems(baseids, amount)
			if count != 0:
				return 0
		return 1

	#
	# Sees if the player has all the required skills to make
	# this item.
	#
	def checkskills(self, player, arguments, check = 0):
		success = 1
		for (skill, values) in self.skills.items():
			if player.skill[skill] < values[0]:
				return 0
			if check:
				success = player.checkskill(skill, values[0], values[1])
		return success

	#
	# Play a craft effect.
	#
	def playcrafteffect(self, player, arguments):
		pass

	#
	# Apply any special properties to the created item.
	#
	def applyproperties(self, player, arguments, item, exceptional):
		pass

	#
	# Generate the HTML used on the skills field on the details gump
	#
	def getskillshtml(self, player, arguments):
		skillshtml = ''
		for (skill, values) in self.skills.items():
			skillshtml += '%s: %.1f%%<br>' % (SKILLNAMES[skill].capitalize(), max(0, values[0] / 10.0))
		return skillshtml

	#
	# Generate the HTML used on the other field on the details gump
	#
	def getotherhtml(self, player, arguments):
		chance = self.getexceptionalchance(player, arguments) * 100
		return 'Chance to create an exceptional item: %0.02f%%.<br>' % chance

	#
	# Generates the HTML used on the materials field of the details gump
	#
	def getmaterialshtml(self, player, arguments):
		materialshtml = ''
		if self.submaterial1 > 0:
			materials = self.parent.submaterials1
			material = self.parent.getsubmaterial1used(player, arguments)
			materialshtml += "%s: %u<br>" % (materials[material][0], self.submaterial1)

		if self.submaterial2 > 0:
			materials = self.parent.submaterials2
			material = self.parent.getsubmaterial2used(player, arguments)
			materialshtml += "%s: %u<br>" % (materials[material][0], self.submaterial2)

		for material in self.materials:
			materialshtml += "%s: %u<br>" % (material[2], material[1])

		return materialshtml

	#
	# Generate the list of skills and materials required
	# to make this item and then process it normally.
	#
	def details(self, player, arguments):
		self.materialshtml = self.getmaterialshtml(player, arguments)
		self.skillshtml = self.getskillshtml(player, arguments)
		self.otherhtml = self.getotherhtml(player, arguments)
		MakeItemAction.details(self, player, arguments)

	#
	# Check if we did an exceptional job.
	#
	def getexceptionalchance(self, player, arguments):
		return 0

	#
	# We failed to create the item.
	# LostMaterials indicates whether materials where lost.
	#
	def fail(self, player, arguments, lostmaterials=0):
		if lostmaterials:
			player.socket.clilocmessage(1044043)
		else:
			player.socket.clilocmessage(1044157)

	#
	# Successful crafting of item.
	#
	def success(self, player, arguments, item, exceptional=0, marked=0):
		if exceptional:
			if marked:
				player.socket.clilocmessage(1044156)
			else:
				player.socket.clilocmessage(1044155)
		else:
			player.socket.clilocmessage(1044154)

	#
	# Try to make the item and consume the resources.
	#
	def make(self, player, arguments, nodelay=0):
		# See if we have enough skills to attempt
		if not self.checkskills(player, arguments):
			player.socket.clilocmessage(1044153)
			return 0

		# See if we have enough material first
		if not self.checkmaterial(player, arguments):
			self.parent.send(player, arguments)
			return 0

		if player.socket.hastag('makemenu_crafting'):
			player.socket.clilocmessage(500119)
			self.parent.send(player, arguments)
			return 0

		# Is this action delayed?
		if self.parent.delay != 0 and not nodelay:
			self.playcrafteffect(player, arguments)
			player.socket.settag('makemenu_crafting', 1)

			# Create a copy of the arguments list and transform all char/item objects using
			# the ObjectWrapper function.
			wrapped = []
			for arg in arguments:
				if type(arg).__name__ in ['wpchar', 'wpitem']:
					wrapped.append(ObjectWrapper(arg))
				else:
					wrapped.append(arg)

			player.addtimer(self.parent.delay, "system.makemenus.craft_timer", [self, wrapped])
			return 0
		elif self.parent.delay == 0:
			self.playcrafteffect(player, arguments)

		success = 0
		success = self.checkskills(player, arguments, 1)

		# 50% chance to loose half of the material
		if not success:
			lostmaterials = 0.5 >= random.random()
			if lostmaterials:
				self.consumematerial(player, arguments, 1)

			self.fail(player, arguments, lostmaterials)
			self.parent.send(player, arguments)
		else:
			self.consumematerial(player, arguments, 0)

			# Calculate if we did an exceptional job
			exceptional = self.getexceptionalchance(player, arguments) >= random.random()

			# Create the item
			item = wolfpack.additem(self.definition)
			item.decay = 1 # Should always decay
			item.movable = 1 # Should always be movable

			if self.amount > 0:
				item.amount = self.amount

			if not item:
				console.log(LOG_ERROR, "Unknown item definition used in action %u of menu %s.\n" % \
					(self.parent.subactions.index(self), self.parent.id))
			else:
				self.applyproperties(player, arguments, item, exceptional)

				if exceptional:
					player.log(LOG_MESSAGE, "Crafted exceptional item %s (0x%x). Amount: %u.\n" % (self.definition, item.serial, item.amount))
					if self.parent.allowmark and self.markable and player.hastag('markitem'):
						item.settag('exceptional', int(player.serial))
						self.success(player, arguments, item, 1, 1)
					else:
						item.settag('exceptional', 0)
						self.success(player, arguments, item, 1, 0)
				else:
					player.log(LOG_MESSAGE, "Crafted item %s (0x%x). Amount: %u.\n" % (self.definition, item.serial, item.amount))
					self.success(player, arguments, item, 0, 0)					

			if not tobackpack(item, player):
				item.update()

		# Register in make history
		MakeAction.make(self, player, arguments, nodelay)
		return success

def craft_timer(player, arguments):
	player.socket.deltag('makemenu_crafting')
	(menu, args) = arguments

	realargs = []
	for arg in args:
		if isinstance(arg, ObjectWrapper):
			realargs.append(arg.get())
		else:
			realargs.append(arg)

	menu.make(player, realargs, 1)

#
# Internal function for sorting a list of
# objects with a title property.
#
def comparetitle(a, b):
	return cmp(a.title.lower(), b.title.lower())

#
# MakeMenu Gump Response Handler
#
def MakeMenuResponse(player, arguments, response):
	assert(len(arguments) >= 1)
	menu = arguments[0]
	arguments = list(arguments)[1:]

	global menus
	if not menus.has_key(menu):
		raise RuntimeError, "Unknown makemenu: %s" % menu
	else:
		menus[menu].response(player, response, arguments)

#
# MakeMenu Target Request Response Handler
#
def MakeMenuTarget(player, arguments, target):
	assert(len(arguments) >= 2)
	menu = arguments[0]
	action = arguments[1]
	arguments = list(arguments)[2:]

	global menus
	if not menus.has_key(menu):
		raise RuntimeError, "Unknown makemenu: %s." % menu
	else:
		if action == 1:
			menus[menu].repair(player, arguments, target)
		elif action == 2:
			menus[menu].enhance(player, arguments, target)
		elif action == 3:
			menus[menu].smelt(player, arguments, target)
		else:
			raise RuntimeError, "Unknown subaction: %u." % action

#
# This class encapsulates all functionality for a single
# category in the makemenus.
#
class MakeMenu:
	#
	# Self registering constructor for a makemenu.
	#
	def __init__(self, id, parent, title):
		assert(not ';' in id)
		assert(not ':' in id)
		global menus
		menus[id] = self
		self.id = id
		self.parent = parent
		self.submenus = []
		self.subactions = []
		self.title = title
		self.gumptype = 0
		self.name_makelast = "Make Last"
		self.delay = 0 # Delay in ms until item is crafted.

		# Display a repair item button on the makemenu
		self.allowrepair = 0

		# Allow enhancement of items.
		self.allowenhance = 0

		# Allow smelting items.
		self.allowsmelt = 0

		self.submaterial1missing = 0
		self.submaterial1noskill = 0
		self.submaterials1 = [] # A list of possible submaterials. A list to select from will be autogenerated.
														# Format: ['name', minskill, ['listofconsumeables']]
														# NOTE: The list will not be sorted _and_ most importantly you're responsible for
														# getting a default material. Otherwise the first index is used.

		self.submaterial2missing = 0
		self.submaterial2noskill = 0
		self.submaterials2 = [] # A list of possible secondary submaterial. A list to select from will be autogenerated.
														# Format: ['name', minskill, ['baseid1', ...]]

		# Set this to true if you want to have a
		# "mark item" option on the gump.
		self.allowmark = 0

		# Ssee if the parent has the correct type
		if self.parent and not isinstance(parent, MakeMenu):
			raise TypeError, "MakeMenu %s has an invalid parent of type %s." % (self.id, type(parent).__name__)

		if self.parent:
			self.parent.submenus.append(self)

		# See if all submenus have the correct type
		for menu in self.submenus:
			if not isinstance(menu, MakeMenu):
				raise TypeError, "MakeMenu %s has an invalid submenu of type %s." % (self.id, type(menu).__name__)

		# See if all subactions have the correct type
		for action in self.subactions:
			if not isinstance(menu, MakeAction):
				raise TypeError, "MakeMenu %s has an invalid subaction of type %s." % (self.id, type(action).__name__)

	#
	# Try to find a craftitem for a certain definition id
	#
	def findcraftitem(self, definition):
		for item in self.subactions:
			if isinstance(item, CraftItemAction) and item.definition == definition:
				return item
		for menu in self.submenus:
			if isinstance(menu, MakeMenu):
				item = menu.findcraftitem(definition)
				if item:
					return item
		return None

	#
	# Get the submaterial1 used by the character.
	# This should be an index to self.submaterials1
	#
	def getsubmaterial1used(self, player, arguments):
		return 0

	#
	# Get the submaterial2 used by the character.
	# This should be an index to self.submaterials2
	#
	def getsubmaterial2used(self, player, arguments):
		return 0

	#
	# Set a new submaterial1 used by the character.
	# This function does nothing by default.
	#
	def setsubmaterial1used(self, player, material, arguments):
		pass

	#
	# Set a new submaterial2 used by the character.
	# This function does nothing by default.
	#
	def setsubmaterial2used(self, player, material, arguments):
		pass

	#
	# Display a makehistory for the user.
	# No pagebuttons, no subactions. Just the last 10
	# actions this user executed.
	#
	def makehistory(self, player, arguments):
		gump = self.generate(arguments)
		self.addbuttons(gump, player, arguments, 1)

		if self.gumptype != 0:
			historyname = 'makehistory%u' % self.gumptype
		else:
			historyname = 'makehistory'

		# We know that we only have 10 makeactions if
		# we show the history. So we don't need a huge iteration
		# here.
		if player.hastag(historyname):
			history = unicode(player.gettag(historyname)).split(';')[:10]
			yoffset = 60
			j = 0
			for item in history:
				(menu, action) = item.split(':')
				action = int(action)
				global menus
				if menus.has_key(menu):
					menu = menus[menu]
					if action < len(menu.subactions):
						gump.addButton(220, yoffset, 4005, 4007, 0x10000000 | j)
						if menu.subactions[action].hasdetails:
							gump.addButton(480, yoffset, 4011, 4012, 0x08000000 | j)
						gump.addText(255, yoffset + 3, menu.subactions[action].title, 0x480)
						yoffset += 20
				j += 1 # Always increase to keep in sync

		gump.send(player)

	#
	# Show a material selection gump.
	# If secondary is set to true, the secondary material will be shown.
	#
	def materialselection(self, player, arguments, secondary = 0):
		gump = self.generate(arguments)
		self.addbuttons(gump, player, arguments, 1)

		if not secondary:
			materials = self.submaterials1
			mask = 0x04000000
		else:
			materials = self.submaterials2
			mask = 0x02000000

		yoffset = 60
		xoffset = 220
		for i in range(0, len(materials)):
			if i == 10:
				xoffset += 150
				yoffset = 60

			yoffset = 60 + (i % 10) * 20
			if materials[i][2] != 0 and player.skill[materials[i][1]] < materials[i][2]:
				gump.addText(xoffset + 35, yoffset + 3, materials[i][0], 0x3b1)
			else:
				gump.addButton(xoffset, yoffset, 4005, 4007, i | mask)
				gump.addText(xoffset + 35, yoffset + 3, materials[i][0], 0x480)

		gump.send(player)

	#
	# Repair an item.
	#
	def repair(self, player, arguments, target):
		pass

	#
	# Smelt an item.
	#
	def smelt(self, player, arguments, target):
		pass

	#
	# Enhance an item.
	#
	def enhance(self, player, arguments, target):
		pass

	#
	# Adds the neccesary buttons to a gump.
	#
	def addbuttons(self, gump, player, arguments, submenu = 0):
		# Allow repairing items.
		if self.allowrepair:
			gump.addButton(350, 310, 4005, 4007, 10)
			gump.addText(385, 313, "Repair Item", 0x480)

		# Allow enhancement of items
		if self.allowenhance:
			gump.addButton(350, 330, 4005, 4007, 11)
			gump.addText(385, 333, "Enhance Item", 0x480)

		# Allow smelting of items
		if self.allowsmelt:
			gump.addButton(350, 350, 4005, 4007, 12)
			gump.addText(385, 353, "Smelt Item", 0x480)

		# EXIT button , return value: 0
		gump.addButton(15, 350, 0xFB1, 0xFB3, 0)
		gump.addText(50, 353, "Exit", 0x480)

		# MAKE LAST button , return value: 2
		gump.addButton(15, 330, 4005, 4007, 2)
		gump.addText(50, 333, self.name_makelast, 0x480)

		# PREVIOUS MENU button, return value: 3
		# Or: if we're on one of the subpages for settings
		# just return to the normal page
		if submenu:
			gump.addButton(15, 310, 0xFAE, 0xFB0, 9)
			gump.addText(50, 313, "Back", 0x480)
		elif self.parent:
			gump.addButton(15, 310, 0xFAE, 0xFB0, 3)
			gump.addText(50, 313, "Previous Menu", 0x480)

		# LAST 10
		if not submenu:
			gump.addButton(15, 60, 0xFAB, 0xFAD, 1)
			gump.addText(50, 63, "Last Ten", 0x480)

		# MARK ITEM button
		if self.allowmark:
			if not player.hastag('markitem'):
				buttonid = 4 # DONT MARK ITEM -> MARK ITEM
				message = "Dont Mark Item"
			else:
				buttonid = 5 # Turns to "DONT MARK ITEM"
				message= "Mark Item"

			gump.addButton(165, 350, 4005, 4007, buttonid)
			gump.addText(200, 353, message, 0x480)

		# Button for the primary submaterial
		if len(self.submaterials1) > 0:
			gump.addButton(165, 310, 4005, 4007, 7) # Display change res1 type gump
			index = self.getsubmaterial1used(player, arguments)
			gump.addText(200, 313, self.submaterials1[index][0], 0x480)

		# Button for the secondary submaterial
		if len(self.submaterials2) > 0:
			gump.addButton(165, 330, 4005, 4007, 8) # Display change res2 type gump
			index = self.getsubmaterial2used(player, arguments)
			gump.addText(200, 333, self.submaterials2[index][0], 0x480)

	#
	# Helper function for making the last action
	# the player executed. Returns true if the
	# action is actually executed.
	#
	def makelast(self, player, arguments):
		if self.gumptype != 0:
			historyname = 'makehistory%u' % self.gumptype
		else:
			historyname = 'makehistory'

		if player.hastag(historyname):
			history = unicode(player.gettag(historyname)).split(';')
			if len(history) > 0:
				(menu, action) = history[0].split(":")
				global menus
				if menus.has_key(menu):
					menu = menus[menu]
					action = int(action)
					if action < len(menu.subactions):
						menu.subactions[action].make(player, arguments)
						return 1

		return 0

	#
	# Handle a response to this makemenu.
	#
	def response(self, player, response, arguments):
		# Show a gump with the last 10 items the player made.
		if response.button == 1:
			self.makehistory(player, arguments)

		# Directly execute the last action the user made.
		elif response.button == 2:
			if not self.makelast(player, arguments):
				player.socket.sysmessage("You didn't make anything yet.")
				self.send(player, arguments)

		# Show the parent menu.
		elif response.button == 3:
			if self.parent:
				self.parent.send(player, arguments)

		# DO NOT MARK ITEM -> MARK ITEM
		elif response.button == 4:
			player.settag('markitem', 0)
			self.send(player, arguments)

		# MARK ITEM -> DO NOT MARK ITEM
		elif response.button == 5:
			player.deltag('markitem')
			self.send(player, arguments)

		elif response.button == 6:
			pass

		# Show the primary material selection dialog
		elif response.button == 7:
			self.materialselection(player, arguments, 0)

		# Show the secondary material selection dialog
		elif response.button == 8:
			self.materialselection(player, arguments, 1)

		# Show ourself
		elif response.button == 9:
			self.send(player, arguments)

		# Repair Item
		elif response.button == 10:
			player.socket.clilocmessage(1044276)
			player.socket.attachtarget("system.makemenus.MakeMenuTarget", [self.id, 1] + arguments)

		# Enhance Item
		elif response.button == 11:
			player.socket.clilocmessage(1061004)
			player.socket.attachtarget("system.makemenus.MakeMenuTarget", [self.id, 2] + arguments)

		# Smelt Item
		elif response.button == 12:
			player.socket.clilocmessage(1044273)
			player.socket.attachtarget("system.makemenus.MakeMenuTarget", [self.id, 3] + arguments)

		# Submenu
		elif response.button & 0x01000000:
			submenu = response.button & ~ 0x01000000
			if submenu < len(self.submenus):
				self.submenus[submenu].send(player, arguments)

		# Directly make subaction
		elif response.button & 0x40000000:
			subaction = response.button & ~ 0x40000000
			if subaction < len(self.subactions):
				self.subactions[subaction].make(player, arguments)

		# Enter subaction detail page
		elif response.button & 0x20000000:
			subaction = response.button & ~ 0x20000000
			if subaction < len(self.subactions):
				self.subactions[subaction].details(player, arguments)

		# MakeHistory: Make Item
		elif response.button & 0x10000000:
			if self.gumptype != 0:
				historyname = 'makehistory%u' % self.gumptype
			else:
				historyname = 'makehistory'

			subaction = response.button & ~ 0x10000000
			if player.hastag(historyname):
				history = unicode(player.gettag(historyname)).split(';')
				if subaction < len(history):
					(menu, action) = history[subaction].split(':')
					action = int(action)
					global menus
					if menus.has_key(menu) and action < len(menus[menu].subactions):
						menus[menu].subactions[action].make(player, arguments)

		# MakeHistory: Show Detail Page
		elif response.button & 0x08000000:
			if self.gumptype != 0:
				historyname = 'makehistory%u' % self.gumptype
			else:
				historyname = 'makehistory'

			subaction = response.button & ~ 0x08000000
			if player.hastag(historyname):
				history = unicode(player.gettag(historyname)).split(';')
				if subaction < len(history):
					(menu, action) = history[subaction].split(':')
					action = int(action)
					global menus
					if menus.has_key(menu) and action < len(menus[menu].subactions):
						menus[menu].subactions[action].details(player, arguments)

		# MaterialSelection: Primary Material
		elif response.button & 0x04000000:
			material = response.button & ~ 0x04000000
			if material < len(self.submaterials1):
				data = self.submaterials1[material]
				if data[2] and player.skill[data[1]] < data[2]:
					player.socket.sysmessage('You are not skilled enough to use this material.')
				else:
					self.setsubmaterial1used(player, arguments, material)
			else:
				player.socket.sysmessage('You selected an invalid material.')
			self.send(player, arguments)

		# MaterialSelection: Secondary Material
		elif response.button & 0x02000000:
			material = response.button & ~ 0x02000000
			if material < len(self.submaterials2):
				data = self.submaterials2[material]
				if data[2] and player.skill[data[1]] < data[2]:
					player.socket.sysmessage('You are not skilled enough to use this material.')
				else:
					self.setsubmaterial2used(player, arguments, material)
			else:
				player.socket.sysmessage('You selected an invalid material.')
			self.send(player, arguments)

	#
	# Sort all subactions and submenus.
	#
	def sort(self):
		self.submenus.sort(comparetitle)
		self.subactions.sort(comparetitle)

		# Assign ids to every subaction
		for i in range(0, len(self.subactions)):
			self.subactions[i].index = i
	#
	# Generate the gump out of the properties of this
	# menu.
	#
	def generate(self, arguments):
		gump = cGump()
		gump.callback = "system.makemenus.MakeMenuResponse"
		gump.setArgs([self.id] + arguments)
		gump.setType(self.gumptype)
		gump.startPage(0)
		gump.addBackground(0x13be, 530, 388)
		gump.addTiledGump(10, 10, 510, 22, 0xA40)
		gump.addTiledGump(10, 37, 200, 250, 0xA40)
		gump.addTiledGump(215, 37, 305, 250, 0xA40)
		gump.addCheckerTrans(10, 10, 510, 368)

		gump.addHtmlGump(10, 12, 510, 20, centerhtml % self.title)
		gump.addHtmlGump(10, 39, 200, 20, centerhtml % "CATEGORIES")
		gump.addHtmlGump(215, 39, 305, 20, centerhtml % "SELECTIONS")

		return gump

	#
	# Send this menu to a character. If the gump
	# has not yet been generated. Autogenerate it.
	# Args are additional arguments you want to have
	# passed on between gump calls.
	#
	def send(self, player, args = []):
		# Close gumps of the same type
		if self.gumptype != 0:
			player.socket.closegump(self.gumptype, 0xFFFF)

		gump = self.generate(args)
		self.addbuttons(gump, player, args)

		# Calculate how many pages are required
		menupages = math.ceil(len(self.submenus) / 9.0)
		actionpages = math.ceil(len(self.subactions) / 10.0)
		pages = int(max(menupages, actionpages))

		actions = 0
		menus = 0

		for i in range(0, pages):
			gump.startPage(i+1)

			# Fill the page with submenus
			for j in range(0, 9):
				if actions + j < len(self.submenus):
					yoffset = 80 + 20 * j
					gump.addButton(15, yoffset, 4005, 4007, 0x01000000 | (actions + j))
					gump.addText(50, yoffset + 3, self.submenus[actions + j].title, 0x480)
			actions += 9

			# Fill the page with subactions
			for j in range(0, 10):
				if menus + j < len(self.subactions):
					yoffset = 60 + 20 * j
					if self.subactions[menus + j].visible(player, args):
						gump.addButton(220, yoffset, 4005, 4007, 0x40000000 | (menus + j))
						if self.subactions[menus + j].hasdetails:
							gump.addButton(480, yoffset, 4011, 4012, 0x20000000 | (menus + j))
						gump.addText(255, yoffset+3, self.subactions[menus + j].title, 0x480)
					else:
						gump.addText(255, yoffset+3, self.subactions[menus + j].title, 0x3b1)
			menus += 9

			# Add a back button
			if i > 0:
				gump.addPageButton(15, 290, 0xFAE, 0xFB0, i)
				gump.addText(50, 293, "Previous Page", 0x480)

			# Add a next button
			if i+1 < pages:
				gump.addPageButton(350, 290, 4005, 4007, i + 2)
				gump.addText(385, 293, "Next Page", 0x480)

		gump.setArgs([self.id] + args)
		gump.send(player)
