
import wolfpack
import math
import random
from wolfpack import console
from wolfpack.consts import CARTOGRAPHY, LOG_ERROR, WPDT_MENU, WPDT_ITEM, skillnamesids
from wolfpack import properties
from system.makemenus import CraftItemAction, MakeMenu, findmenu, generateNamefromDef
from wolfpack.utilities import hex2dec, tobackpack, createlockandkey
import skills


#
# Bring up the cartography menu
#
def onUse(char, item):
	menu = findmenu('CARTOGRAPHY')
	if menu:
		menu.send(char, [item.serial])
	return True

#
# Create an item.
#
class CartItemAction(CraftItemAction):
	def __init__(self, parent, title, itemid, definition):
		CraftItemAction.__init__(self, parent, title, itemid, definition)

	#
	# Play the "grind" sound when we fail to create a potion
	#
	def fail(self, player, arguments, lostmaterials):
		if lostmaterials:
			player.socket.clilocmessage(1044043)
		else:
			player.socket.clilocmessage(1044157)


	#
	# Print a nicer success message
	#
	def success(self, player, arguments, item, exceptional=0, marked=0):
		
		
		if exceptional and marked:
			player.socket.clilocmessage(1044156)
		elif exceptional:
			player.socket.clilocmessage(1044155)
		else:
			player.socket.clilocmessage(1044154)

	#
	# Play a simple soundeffect
	#
	def playcrafteffect(self, player, arguments):
		player.soundeffect(0x249)

	#
	# Apply range of map to the item.
	#
	def applyproperties(self, player, arguments, item, exceptional):
		skillValue = player.skill[CARTOGRAPHY]/10

		if item.baseid == 'map_local':
			dist = 64 + (skillValue * 2)

			self.setDisplay(item, player.pos.x - dist, player.pos.y - dist, player.pos.x + dist, player.pos.y + dist, 200, 200)

		elif item.baseid == 'map_city':
			dist = 64 + (skillValue * 4)
			if ( dist < 200 ):
				dist = 200

			size = 32 + (skillValue * 2)
			if ( size < 200 ):
				size = 200
			elif ( size > 400 ):
				size = 400

			self.setDisplay(item, player.pos.x - dist, player.pos.y - dist, player.pos.x + dist, player.pos.y + dist, size, size);

		elif item.baseid == 'sea_chart':
			dist = 64 + (skillValue * 10)
			if ( dist < 200 ):
				dist = 200

			size = 24 + (skillValue * 3.3)
			if ( size < 200 ):
				size = 200
			elif ( size > 400 ):
				size = 400

			self.setDisplay(item, player.pos.x - dist, player.pos.y - dist, player.pos.x + dist, player.pos.y + dist, size, size);


	def setDisplay(self, item, x1, y1, x2, y2, width, height):
		if ( x1 < 0 ):
			x1 = 0
		if ( y1 < 0 ):
			y1 = 0
		if ( x2 >= 5120 ):
			x2 = 5119
		if ( y2 >= 4096 ):
			y2 = 4095

		item.settag('xtop', x1)
		item.settag('xbottom', x2)
		item.settag('ytop', y1)
		item.settag('ybottom', y2)
		item.settag('width', width)
		item.settag('height', height)


#
# A cartography menu.
#
class CartographyMenu(MakeMenu):
	def __init__(self, id, parent, title):
		MakeMenu.__init__(self, id, parent, title)
		self.allowmark = 0
		self.delay = 2000
		self.gumptype = 0x9ca51fcb # This should be unique
		self.requiretool = True

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
	menu = CartographyMenu(id, parent, name)

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
			if not child.hasattribute('definition'):
				console.log(LOG_ERROR, "craft action without definition in menu %s.\n" % menu.id)
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
				except Exception, e:
					console.log(LOG_ERROR, "Craft action with invalid item id in menu %s: %s\n" % (menu.id, str(e)))

				if child.hasattribute('name'):
					name = child.getattribute('name')
				else:
					name = generateNamefromDef(itemdef)
				action = CartItemAction(menu, name, int(itemid), itemdef)

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

def cartography( char, skill ):
	char.socket.clilocmessage(500014) # That skill cannot be used directly.

#
# Load the cartography menu.
#
def onLoad():
	loadMenu('CARTOGRAPHY')
	skills.register( CARTOGRAPHY, cartography )
