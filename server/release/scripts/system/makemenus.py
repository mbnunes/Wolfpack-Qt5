
import wolfpack
from wolfpack.utilities import tobackpack, ObjectWrapper
from wolfpack.gumps import cGump
import math
from wolfpack import console, properties, tr
from wolfpack.consts import *
import random
from wolfpack.utilities import hex2dec

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
		
		# Show ourself to the user again
		self.parent.send(player, arguments)

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
			player.socket.sysmessage(tr('You put the new item into your backpack.'))
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
		gump.addHtmlGump(10, 132, 150, 20, centerhtml % tr("SKILLS"))
		gump.addHtmlGump(10, 217, 150, 20, centerhtml % tr("MATERIALS"))
		gump.addHtmlGump(10, 302, 150, 20, centerhtml % tr("OTHER"))
		gump.addHtmlGump(170, 39, 70, 20, whitehtml % tr("ITEM"))
		gump.addButton(15, 387, 0xFAE, 0xFB0, 0) # Back to the parent menu of this node
		gump.addText(50, 389, tr("Back"), 0x480)
		gump.addButton(375, 387, 4005, 4007, 1) # Make the item
		gump.addText(410, 389, tr("Make Now"), 0x480)

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
	# Process a craft node for this item and use its properties to initialize this
	#
	def processnode(self, node, menu):
		pass

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
		self.lackmaterial = tr("You don't have enough material to make that.")

	#
	# Process an XML node
	#
	def processnode(self, node, menu):
		# Define several common names for submaterial1
		submaterial1names = ['leather', 'ingots']
		submaterial2names = ['scales']
		
		if node.name in submaterial1names:
			self.submaterial1 = hex2dec(node.getattribute('amount', '0'))
			
		elif node.name in submaterial2names:		
			self.submaterial2 = hex2dec(node.getattribute('amount', '0'))
		
		# Normal Material
		elif node.name == 'material':
			if not node.hasattribute('id'):
				console.log(LOG_ERROR, "Material element without id list in menu %s.\n" % menu.id)
				return
			else:
				ids = node.getattribute('id').split(';')
				try:
					amount = hex2dec(node.getattribute('amount', '1'))
					materialname = node.getattribute('name', 'Unknown')
				except:
					console.log(LOG_ERROR, "Material element with invalid id list in menu %s.\n" % menu.id)
					return
				self.materials.append([ids, amount, materialname])

		# Skill requirement
		elif node.name in skillnamesids:
			skill = skillnamesids[node.name]
			try:
				minimum = hex2dec(node.getattribute('min', '0'))
			except:
				console.log(LOG_ERROR, "%s element with invalid min value in menu %s.\n" % (node.name, menu.id))
				return

			try:
				maximum = hex2dec(node.getattribute('max', '1200'))
			except:
				console.log(LOG_ERROR, "%s element with invalid max value in menu %s.\n" % (node.name, menu.id))
				return

			try:
				penalty = hex2dec(node.getattribute('penalty','0'))
			except:
				console.log(LOG_ERROR, "%s element with invalid max value in menu %s.\n" % (node.name, menu.id))

			self.skills[skill] = [minimum, maximum, penalty]

	#
	# Make invisible if we dont have the minimum
	# skill requirements yet
	#
	def visible(self, player, arguments):
		return self.checkskills(player, arguments, 0)

	#
	# Returns true if the player has the required material.
	# If materials is an empty list, it's correctly filled with data.
	#
	def checkmaterial(self, player, arguments, silent = 0):
		backpack = player.getbackpack()

		submaterial1amount = 0 # Amount of submaterial 1 left to consume
		submaterial1baseids = [] # List of baseids that statisfy this submaterial
		submaterial2amount = 0 # Amount of submaterial 2 left to consume
		submaterial2baseids = [] # List of baseids that statisfy this submaterial		

		# Do numerous checks to see if the first submaterial can be used
		if self.submaterial1 > 0:
			materials = self.parent.submaterials1
			material = self.parent.getsubmaterial1used(player, arguments)
			if material >= len(materials):
				if not silent:
					player.socket.sysmessage(tr("You try to craft with an invalid material."))
				return False

			material = materials[material]

			# Check the skill requirement of the material.
			if material[2] and player.skill[material[1]] < material[2]:
				if not silent:
					if self.parent.submaterial1noskill != 0:
						player.socket.clilocmessage(self.parent.submaterial1noskill)
					else:
						player.socket.clilocmessage(1044153)
				return False
				
			submaterial1amount = self.submaterial1 # Make a copy of the amount that has to be consumed
			submaterial1baseids = material[3] # This is a list of baseids that are accepted for this material

		if self.submaterial2 > 0:
			materials = self.parent.submaterials2
			material = self.parent.getsubmaterial2used(player, arguments)
			if material >= len(materials):
				if not silent:
					player.socket.sysmessage(tr("You try to craft with an invalid material."))
				return False
			material = materials[material]

			# Check the skill requirement of the material.
			if material[2] and player.skill[material[1]] < material[2]:
				if not silent:
					if self.parent.submaterial2noskill != 0:
						player.socket.clilocmessage(self.parent.submaterial2noskill)
					else:
						player.socket.clilocmessage(1044153)
				return False

			submaterial2amount = self.submaterial2 # Make a copy of the amount that has to be consumed
			submaterial2baseids = material[3] # This is a list of baseids that are accepted for this material

		materials = [] # Local copy of material list
		for material in self.materials:
			materials.append(material[:2]) # Last element is the amount left to find

		# This loop checks for all required materials at once.
		for item in backpack.content:
			# Check if the pile is used by the main material
			if item.baseid in submaterial1baseids:
				submaterial1amount -= item.amount
				continue
			
			# Check if the pile is used by the secondary material	
			if item.baseid in submaterial2baseids:
				submaterial2amount -= item.amount
				continue
				
			for material in materials:
				if item.baseid in material[0]:
					material[1] -= item.amount
					break # Break the inner loop

		# We didn't succeed in finding enough of submaterial1
		if submaterial1amount > 0:
			if not silent:
				if self.parent.submaterial1missing != 0:
					player.socket.clilocmessage(self.parent.submaterial1missing)
				else:
					player.socket.sysmessage(self.lackmaterial)
			return False
			
		# We didn't succeed in finding enough of submaterial2
		if submaterial2amount > 0:
			if not silent:
				if self.parent.submaterial2missing != 0:
					player.socket.clilocmessage(self.parent.submaterial2missing)
				else:
					player.socket.sysmessage(self.lackmaterial)
			return False
			
		# Check if we found all the normal material we need to produce this item.
		for material in materials:
			if material[1] > 0:
				player.socket.sysmessage(self.lackmaterial)
				return False

		return True

	#
	# Consume material to make the item.
	#
	def consumematerial(self, player, arguments, half = 0):
		backpack = player.getbackpack()

		submaterial1amount = 0 # Amount of submaterial 1 left to consume
		submaterial1baseids = [] # List of baseids that statisfy this submaterial
		submaterial2amount = 0 # Amount of submaterial 2 left to consume
		submaterial2baseids = [] # List of baseids that statisfy this submaterial		

		# Do numerous checks to see if the first submaterial can be used
		if self.submaterial1 > 0:
			materials = self.parent.submaterials1
			material = self.parent.getsubmaterial1used(player, arguments)
			if material >= len(materials):
				if not silent:
					player.socket.sysmessage(tr("You try to craft with an invalid material."))
				return False

			material = materials[material]

			# Check the skill requirement of the material.
			if material[2] and player.skill[material[1]] < material[2]:
				if not silent:
					if self.parent.submaterial1noskill != 0:
						player.socket.clilocmessage(self.parent.submaterial1noskill)
					else:
						player.socket.clilocmessage(1044153)
				return False
				
			submaterial1amount = self.submaterial1 # Make a copy of the amount that has to be consumed
			if half:
				submaterial1amount = int(math.ceil(submaterial1amount * 0.5))
			submaterial1baseids = material[3] # This is a list of baseids that are accepted for this material

		if self.submaterial2 > 0:
			materials = self.parent.submaterials2
			material = self.parent.getsubmaterial2used(player, arguments)
			if material >= len(materials):
				if not silent:
					player.socket.sysmessage(tr("You try to craft with an invalid material."))
				return False
			material = materials[material]

			# Check the skill requirement of the material.
			if material[2] and player.skill[material[1]] < material[2]:
				if not silent:
					if self.parent.submaterial2noskill != 0:
						player.socket.clilocmessage(self.parent.submaterial2noskill)
					else:
						player.socket.clilocmessage(1044153)
				return False

			submaterial2amount = self.submaterial2 # Make a copy of the amount that has to be consumed
			if half:
				submaterial2amount = int(math.ceil(submaterial2amount * 0.5))
			submaterial2baseids = material[3] # This is a list of baseids that are accepted for this material

		materials = [] # Local copy of material list
		for material in self.materials:
			m = material[:2]
			if half:
				m[1] = int(math.ceil(m[1] * 0.5))
			materials.append(m) # Last element is the amount left to find

		# This loop checks for all required materials at once.
		for item in backpack.content:
			# Check if the pile is used by the main material
			if item.baseid in submaterial1baseids:
				if item.amount < submaterial1amount: # We have less or equal than we need
					submaterial1amount -= item.amount
					item.delete()
				else: # We have more than we need
					item.amount -= submaterial1amount
					item.update()
					submaterial1amount = 0
				continue
			
			# Check if the pile is used by the secondary material	
			if item.baseid in submaterial2baseids:
				if item.amount < submaterial2amount: # We have less or equal than we need
					submaterial2amount -= item.amount
					item.delete()
				else: # We have more than we need
					item.amount -= submaterial2amount
					item.update()
					submaterial2amount = 0
				continue
				
			for material in materials:
				if item.baseid in material[0]:
					if item.amount < material[1]: # We have less or equal than we need
						material[1] -= item.amount
						item.delete()
					else: # We have more than we need
						item.amount -= material[1]
						item.update()
						material[1] = 0
					break # Break the inner loop

		# We didn't succeed in finding enough of submaterial1
		if submaterial1amount > 0:
			if not silent:
				if self.parent.submaterial1missing != 0:
					player.socket.clilocmessage(self.parent.submaterial1missing)
				else:
					player.socket.sysmessage(self.lackmaterial)
			return False
			
		# We didn't succeed in finding enough of submaterial2
		if submaterial2amount > 0:
			if not silent:
				if self.parent.submaterial2missing != 0:
					player.socket.clilocmessage(self.parent.submaterial2missing)
				else:
					player.socket.sysmessage(self.lackmaterial)
			return False
			
		# Check if we found all the normal material we need to produce this item.
		for material in materials:
			if material[1] > 0:
				player.socket.sysmessage(self.lackmaterial)
				return False

		return True

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
		return tr('Chance to create an exceptional item: %0.02f%%.<br>') % chance

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
	# Get the main skill used by this action
	# -1 if there isn't any
	#
	def getmainskill(self):
		cmaxv = 0 # Current maximum skill value
		cskill = -1 # Current maximum skill
		for (skill, values) in self.skills.items():
			if values[1] >= cmaxv:
				cskill = skill
				cmaxv = values[1]
		return cskill			

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
		# Check the tool
		if self.parent and self.parent.requiretool:
			if not self.parent.checktool(player, wolfpack.finditem(arguments[0])):
				return False

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
			# Check the tool
			if self.parent and self.parent.requiretool:
				if not self.parent.checktool(player, wolfpack.finditem(arguments[0]), True):
					return False

			self.consumematerial(player, arguments, 0)

			# Calculate if we did an exceptional job
			exceptional = self.getexceptionalchance(player, arguments) >= random.random()

			# Create the item
			item = wolfpack.additem(self.definition)

			if not item:
				console.log(LOG_ERROR, "Unknown item definition used in action %u of menu %s.\n" % \
					(self.parent.subactions.index(self), self.parent.id))
			else:
				item.decay = 1 # Should always decay
				item.movable = 1 # Should always be movable
	
				if self.amount > 0:
					item.amount = self.amount			
			
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
		self.name_makelast = tr("Make Last")
		self.delay = 0 # Delay in ms until item is crafted.
		self.requiretool = False # Don't check for a craft tool in arguments[0] by default
		
		self.repairsound = 0 # The soundeffect played for repairing an item

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
	# Get topmost menu
	#
	def topmostmenu(self):
		if self.parent:
			return self.parent.topmostmenu()
		else:
			return self

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
	# This function is used as a callback by the makemenus to reduce the durability
	# of the crafting tool used
	#
	def checktool(self, player, item, wearout = False):
		if not self.requiretool:
			return True

		if not item:
			return False

		# Has to be in our posession
		if item.getoutmostchar() != player:
			player.socket.clilocmessage(500364)
			return False

		# We do not allow "invulnerable" tools.
		if not item.hastag('remaining_uses'):
			player.socket.clilocmessage(1044038)
			item.delete()
			return False

		# See if we have another tool equipped
		equipped = player.itemonlayer(LAYER_RIGHTHAND)
		if equipped and equipped != item:
			player.socket.clilocmessage(1048146)
			return False

		if wearout:
			uses = int(item.gettag('remaining_uses'))
			if uses <= 1:
				player.socket.clilocmessage(1044038)
				item.delete()
				return False
			else:
				item.settag('remaining_uses', uses - 1)
				item.resendtooltip()

		return True

	#
	# Repair an item.
	# This is a generic repair function that should work for any subclass.
	#
	def repair(self, player, arguments, target):
		if self.requiretool and not self.checktool(player, wolfpack.finditem(arguments[0])):
			return
		
		if not target.item:
			player.socket.clilocmessage(500426)
			return

		if not player.canreach(target.item, -1):
			player.socket.clilocmessage(1044275)
			return

		item = target.item
		weapon = properties.itemcheck(item, ITEM_WEAPON)
		shield = properties.itemcheck(item, ITEM_SHIELD)
		armor = properties.itemcheck(item, ITEM_ARMOR)

		if weapon or armor or shield:
			# Item in full repair
			if item.maxhealth <= 0 or item.health >= item.maxhealth:
				player.socket.clilocmessage(500423)
				return False # Fully repaired

			action = self.topmostmenu().findcraftitem(item.baseid)
			
			if action:
				mainskill = action.getmainskill()
			else:
				mainskill = -1

			# We can't craft it, so we can't repair it.
			if mainskill == -1:
				player.socket.clilocmessage(1044277)
				return False # Can't craft the item

			skill = player.skill[mainskill]
			if skill >= 900:
				weaken = 1
			elif skill >= 700:
				weaken = 2
			else:
				weaken = 3				

			# We will either destroy or repair it from here on
			# So we can play the craft effect.
			if self.repairsound != 0:
				player.soundeffect(self.repairsound)

			if item.maxhealth <= weaken:
				player.socket.clilocmessage(500424)
				player.log(LOG_MESSAGE, "Tries to repair item %s (0x%x) and destroys it.\n" % (item.baseid, item.serial))
				item.delete()
			elif player.checkskill(mainskill, 0, 1000):
				player.socket.clilocmessage(1044279)
				item.maxhealth -= weaken
				item.health = item.maxhealth				
				item.resendtooltip()
				player.log(LOG_MESSAGE, "Repairs item %s (0x%x) and weakens it by %u points.\n" % (item.baseid, item.serial, weaken))
			else:
				player.socket.clilocmessage(1044280)
				item.maxhealth -= weaken
				item.health = max(0, item.health - weaken)
				item.resendtooltip()
				player.log(LOG_MESSAGE, "Fails to repair item %s (0x%x) and weakens it by %u points.\n" % (item.baseid, item.serial, weaken))

			# Consume one use of the tool
			if self.requiretool:
				self.checktool(player, wolfpack.finditem(arguments[0]), True)

			# Warn the user if we'll break the item next time
			if item.maxhealth <= weaken:
				player.socket.clilocmessage(1044278)

			return True # Repaired the item

		player.socket.clilocmessage(1044277)
		return False # Cannot repair this

	#
	# Smelt an item.
	#
	def smelt(self, player, arguments, target):
		pass

	#
	# Enhance an item.
	#
	def enhance(self, player, arguments, target):
		if self.requiretool and not self.checktool(player, wolfpack.finditem(args[0])):
			return		
		
		if not target.item:
			self.send(player, arguments)
			return
		
		if not player.canreach(target.item, -1):
			player.socket.clilocmessage(1061005)
			self.send(player, arguments)
			return
			
		# Check if we have a special material selected (not the default one)
		index = self.getsubmaterial1used(player, arguments)
		if index == 0:
			player.socket.clilocmessage(1061010)
			self.send(player, arguments)
			return
			
		# Check if we are skilled enough to use the material
		minvalue = self.submaterials1[index][2]
		skillid = self.submaterials1[index][1]
		
		if player.skill[skillid] < minvalue:
			if self.submaterial1noskill != 0:
				player.socket.clilocmessage(self.submaterial1noskill)
			else:
				player.socket.clilocmessage(1044153)

		# Only armors and weapons and shields can be enhanced
		item = target.item
		shield = properties.itemcheck(item, ITEM_SHIELD)
		armor = properties.itemcheck(item, ITEM_ARMOR)
		weapon = properties.itemcheck(item, ITEM_WEAPON)
		if not shield and not armor and not weapon:
			player.socket.clilocmessage(1061011)
			self.send(player, arguments)
			return

		# Try to find the craft action for the item
		action = self.findcraftitem(item.baseid)

		if not action or action.submaterial1 == 0:
			player.socket.clilocmessage(1061011)
			self.send(player, arguments)
			return

		# Do we meet the minimum skill requirements?
		if not action.checkskills(player, arguments, False):
			player.socket.clilocmessage(1044153)
			self.send(player, arguments)
			return

		# Is the item already enhanced?
		if item.hastag('resname'):
			resname = str(item.gettag('resname'))
			# Only the first would be allowed
			if self.submaterials1[0][5] != resname:
				player.socket.clilocmessage(1061012)
				self.send(player, arguments)
				return

		# Do we have the required amount of resources?
		if not action.checkmaterial(player, arguments):
			self.send(player, arguments)
			return 0

		# Collect information about the important bonuses of the item.
		physical = 0
		fire = 0
		cold = 0
		poison = 0
		energy = 0
		durability = 0
		luck = 0
		lowerrequirements = 0
		damageincrease = 0

		# These are flags
		physicalBonus = False
		fireBonus = False
		coldBonus = False
		energyBonus = False
		poisonBonus = False
		durabilityBonus = False
		luckBonus = False
		lowerrequirementsBonus = False
		damageincreaseBonus = False
		
		resname = self.submaterials1[index][5] # This resname is for the material used to enhance
	
		if weapon:
			failChance = 20
			durability = item.maxhealth
			luck = properties.fromitem(item, LUCK)
			damageincrease = properties.fromitem(item, DAMAGEBONUS)
			lowerrequirements = properties.fromitem(item, LOWERREQS)
			
			fireBonus = properties.fromresource(resname, DAMAGE_FIRE, ITEM_WEAPON) > 0
			coldBonus = properties.fromresource(resname, DAMAGE_COLD, ITEM_WEAPON) > 0
			energyBonus = properties.fromresource(resname, DAMAGE_ENERGY, ITEM_WEAPON) > 0
			poisonBonus = properties.fromresource(resname, DAMAGE_POISON, ITEM_WEAPON) > 0

			durabilityBonus = properties.fromresource(resname, DURABILITYBONUS, ITEM_ARMOR) > 0
			luckBonus = properties.fromresource(resname, LUCK, ITEM_ARMOR) > 0
			lowerrequirementsBonus = properties.fromresource(resname, LOWERREQS, ITEM_ARMOR) > 0
			damageincreaseBonus = damageincrease > 0
			itemtype = ITEM_WEAPON
		else:
			failChance = 20
			physical = properties.fromresource(resname, RESISTANCE_PHYSICAL, ITEM_ARMOR)
			fire = properties.fromresource(resname, RESISTANCE_FIRE, ITEM_ARMOR)
			cold = properties.fromresource(resname, RESISTANCE_COLD, ITEM_ARMOR)
			poison = properties.fromresource(resname, RESISTANCE_POISON, ITEM_ARMOR)
			energy = properties.fromresource(resname, RESISTANCE_ENERGY, ITEM_ARMOR)

			durability = item.maxhealth
			luck = properties.fromitem(item, LUCK)
			lowerrequirements = properties.fromitem(item, LOWERREQS)
			
			physicalBonus = physical > 0
			fireBonus = fire > 0
			coldBonus = cold > 0
			energyBonus = energy > 0
			poisonBonus = poison > 0
			durabilityBonus = properties.fromresource(resname, DURABILITYBONUS, ITEM_ARMOR) > 0
			luckBonus = properties.fromresource(resname, LUCK, ITEM_ARMOR) > 0
			lowerrequirementsBonus = properties.fromresource(resname, LOWERREQS, ITEM_ARMOR) > 0
			damageincreaseBonus = False
			itemtype = ITEM_ARMOR

		# failChance = chance to fail
		# Get the primary skill for crafting the item
		primarySkill = -1
		primarySkillValue = -1
		
		for (skill, value) in action.skills.items():
			if value > primarySkillValue:
				primarySkill = skill
				primarySkillValue = value
				
		if primarySkill != -1:
			if player.skill[primarySkill] >= 1000:
				failChance -= (player.skill[primarySkill] - 900) / 100
		
		# Check for every property the item has.
		result = 1		
		if physicalBonus:
			result = self.checkenhancement(result, failChance + physical, player)
			
		if fireBonus:
			result = self.checkenhancement(result, failChance + fire, player)
			
		if coldBonus:
			result = self.checkenhancement(result, failChance + cold, player)
			
		if poisonBonus:
			result = self.checkenhancement(result, failChance + poison, player)
			
		if energyBonus:
			result = self.checkenhancement(result, failChance + energy, player)
			
		if durabilityBonus:
			result = self.checkenhancement(result, failChance + durability / 40, player)
			
		if luckBonus:
			result = self.checkenhancement(result, failChance + 10 + luck / 2, player)
			
		if lowerrequirementsBonus:
			result = self.checkenhancement(result, failChance + lowerrequirements / 4, player)
			
		if damageincreaseBonus:
			result = self.checkenhancement(result, failChance + damageincrease / 5, player)
	
		# Broken
		if result == -1:
			action.consumematerial(player, arguments, True) # Consume half the material
			item.delete() # Delete the item
			player.socket.clilocmessage(1061080)
		
		# Failure
		elif result == 0:
			action.consumematerial(player, arguments, True) # Consume half the material
			player.socket.clilocmessage(1061082)

		# Success
		else:
			action.consumematerial(player, arguments, False) # Consume the material
			player.socket.clilocmessage(1061008)
			
			# Attach the properties to the item
			item.settag('resname', resname)
			# Check for every property if the item had a tag already
			for (key, info) in properties.PROPERTIES.items():
				if item.hastag(info[0]): # See if the property is overriden
					value = item.gettag(info[0]) # Get the overriden value
					resvalue = properties.fromresource(resname, key, itemtype) # Get the bonus for the resource
					if resvalue != info[1]: # See if the resource has a bonus
						value = value + resvalue # Add the resources bonus
						item.settag(info[0], value) # Save the new overriden value for the item

			# Manually increase the durability bonus
			healthbonus = properties.fromresource(resname, DURABILITYBONUS, itemtype)
			if healthbonus != 0:
				bonus = int(math.ceil(item.maxhealth * (healthbonus / 100.0)))
				item.maxhealth = max(1, item.maxhealth + bonus)
				item.health = item.maxhealth

			item.color = self.submaterials1[index][4] # Change the color to the resources color
			item.update()
			item.resendtooltip()

		self.send(player, arguments)
		
	#
	# Check the enhancement result for a given property
	# with a given fail chance.
	#
	def checkenhancement(self, result, chance, player):
		if result: # Only calculate if we still have a chance to succeed
			rnd = random.randrange(0, 100)
			
			if 10 > rnd: # 10% failure chance
				result = 0
			elif chance > rnd: # Rest break chance
				result = -1

		return result
				
	#
	# Adds the neccesary buttons to a gump.
	#
	def addbuttons(self, gump, player, arguments, submenu = 0):
		# Allow repairing items.
		if self.allowrepair:
			gump.addButton(350, 310, 4005, 4007, 10)
			gump.addText(385, 313, tr("Repair Item"), 0x480)

		# Allow enhancement of items
		if self.allowenhance:
			gump.addButton(350, 330, 4005, 4007, 11)
			gump.addText(385, 333, tr("Enhance Item"), 0x480)

		# Allow smelting of items
		if self.allowsmelt:
			gump.addButton(350, 350, 4005, 4007, 12)
			gump.addText(385, 353, tr("Smelt Item"), 0x480)

		# EXIT button , return value: 0
		gump.addButton(15, 350, 0xFB1, 0xFB3, 0)
		gump.addText(50, 353, tr("Exit"), 0x480)

		# MAKE LAST button , return value: 2
		gump.addButton(15, 330, 4005, 4007, 2)
		gump.addText(50, 333, self.name_makelast, 0x480)

		# PREVIOUS MENU button, return value: 3
		# Or: if we're on one of the subpages for settings
		# just return to the normal page
		if submenu:
			gump.addButton(15, 310, 0xFAE, 0xFB0, 9)
			gump.addText(50, 313, tr("Back"), 0x480)
		elif self.parent:
			gump.addButton(15, 310, 0xFAE, 0xFB0, 3)
			gump.addText(50, 313, tr("Previous Menu"), 0x480)

		# LAST 10
		if not submenu:
			gump.addButton(15, 60, 0xFAB, 0xFAD, 1)
			gump.addText(50, 63, tr("Last Ten"), 0x480)

		# MARK ITEM button
		if self.allowmark:
			if not player.hastag('markitem'):
				buttonid = 4 # DONT MARK ITEM -> MARK ITEM
				message = tr("Dont Mark Item")
			else:
				buttonid = 5 # Turns to "DONT MARK ITEM"
				message= tr("Mark Item")

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
		if self.requiretool and not self.checktool(player, wolfpack.finditem(arguments[0])):
			return
		
		# Show a gump with the last 10 items the player made.
		if response.button == 1:
			self.makehistory(player, arguments)

		# Directly execute the last action the user made.
		elif response.button == 2:
			if not self.makelast(player, arguments):
				player.socket.sysmessage(tr("You didn't make anything yet."))
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
			# Check if we have a special material selected (not the default one)
			index = self.getsubmaterial1used(player, arguments)
			if index == 0:
				player.socket.clilocmessage(1061010)				
				self.send(player, arguments) # Resend menu
			else:
				# Check if we are skilled enough to use the material
				minvalue = self.submaterials1[index][2]
				skillid = self.submaterials1[index][1]

				if player.skill[skillid] < minvalue:
					if self.submaterial1noskill != 0:
						player.socket.clilocmessage(self.submaterial1noskill)
					else:
						player.socket.clilocmessage(1044153)
						
					self.send(player, arguments) # Resend menu
				else:
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
					player.socket.sysmessage(tr('You are not skilled enough to use this material.'))
				else:
					self.setsubmaterial1used(player, arguments, material)
			else:
				player.socket.sysmessage(tr('You selected an invalid material.'))
			self.send(player, arguments)

		# MaterialSelection: Secondary Material
		elif response.button & 0x02000000:
			material = response.button & ~ 0x02000000
			if material < len(self.submaterials2):
				data = self.submaterials2[material]
				if data[2] and player.skill[data[1]] < data[2]:
					player.socket.sysmessage(tr('You are not skilled enough to use this material.'))
				else:
					self.setsubmaterial2used(player, arguments, material)
			else:
				player.socket.sysmessage(tr('You selected an invalid material.'))
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
		gump.addHtmlGump(10, 39, 200, 20, centerhtml % tr("CATEGORIES"))
		gump.addHtmlGump(215, 39, 305, 20, centerhtml % tr("SELECTIONS"))

		return gump

	#
	# Send this menu to a character. If the gump
	# has not yet been generated. Autogenerate it.
	# Args are additional arguments you want to have
	# passed on between gump calls.
	#
	def send(self, player, args = []):
		if self.requiretool and not self.checktool(player, wolfpack.finditem(args[0])):
			return
		
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
				gump.addText(50, 293, tr("Previous Page"), 0x480)

			# Add a next button
			if i+1 < pages:
				gump.addPageButton(350, 290, 4005, 4007, i + 2)
				gump.addText(385, 293, tr("Next Page"), 0x480)

		gump.setArgs([self.id] + args)
		gump.send(player)
