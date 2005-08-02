#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Naddel				#
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
import math
import random
from wolfpack import console
from wolfpack.consts import COOKING, LOG_ERROR, WPDT_MENU, WPDT_ITEM
from wolfpack import tr
from system.makemenus import CraftItemAction, MakeMenu, findmenu
from wolfpack.utilities import hex2dec, tobackpack
import beverage

ovens1 = range( 0x045d, 0x048f )
ovens2 = range( 0x08cf, 0x08e0 )
ovens3 = range( 0x092b, 0x096d )
ovens4 = range( 0x11ec, 0x11f0 )
ovens = ovens1 + ovens2 + ovens3 + ovens4

campfire = range( 0x0de3, 0x0de9 )
firefield = [ 0x398c, 0x3996 ]
fires = ovens + campfire + firefield

def onUse( char, item ):
	if not item.getoutmostchar() == char:
		char.socket.clilocmessage( 1042001 ) # That must be in your pack for you to use it.
		return True

	# simply open the sack
	if item.baseid in ["1039", "1045"]:
		item.id += 0x1
		item.baseid = 1046
		item.update()
		return True

	if not actions.has_key( item.baseid ):
		menu = findmenu('COOKING')
		if menu:
			menu.send(char, [item.serial])
		return True

	char.socket.attachtarget( "skills.cooking.%s" % actions[ item.baseid ], [item.serial] )
	return True

def Dough( char, args, target ):
	dough = wolfpack.finditem( args[0] ) 
	if not dough or not target.item:
		return False
	backpack = char.getbackpack()

	if not target.item.getoutmostchar() == char:
		char.socket.clilocmessage( 1042001 ) # That must be in your pack for you to use it.
		return True

	# Eggs
	if target.item.baseid == "9b5":
		eggshells = wolfpack.additem( "9b4" )
		unbaked_quiche = wolfpack.additem( "unbaked_quiche" )
		if not tobackpack( eggshells, char ):
			eggshells.update()
		if not tobackpack( unbaked_quiche, char ):
			unbaked_quiche.update()

	# Cheese Wheel
	elif target.item.baseid == "97e":
		unbaked_cheesepizza = wolfpack.additem( "unbaked_cheesepizza" )
		if not tobackpack( unbaked_cheesepizza, char ):
			unbaked_cheesepizza.update()
	# Sausage
	elif target.item.baseid in ["9c0", "9c1"]:
		unbaked_sausagepizza = wolfpack.additem( "unbaked_sausagepizza" )
		if not tobackpack( unbaked_sausagepizza, char ):
			unbaked_sausagepizza.update()
	# Apple
	elif target.item.baseid == "9d0":
		unbaked_applepie = wolfpack.additem( "unbaked_applepie" )
		if not tobackpack( unbaked_applepie, char ):
			unbaked_applepie.update()
	# Peach
	elif target.item.baseid in ["9d2", "172c"]:
		unbaked_peachcobbler = wolfpack.additem( "unbaked_peachcobbler" )
		if not tobackpack( unbaked_peachcobbler, char ):
			unbaked_peachcobbler.update()
	else:
		return False

	dough.delete()
	target.item.delete()

def SweetDough( char, args, target ):
	dough = wolfpack.finditem( args[0] ) 
	if not dough or not target.item:
		return False

	if not target.item.getoutmostchar() == char:
		char.socket.clilocmessage( 1042001 ) # That must be in your pack for you to use it.
		return True

	backpack = char.getbackpack()
	# Bowl Flour
	if target.item.baseid == "a1e":
		target.item.delete()
		cake_mix = wolfpack.additem( "cake_mix" )
		if not tobackpack( cake_mix, char ):
			cake_mix.update()
	# Campfire
	elif target.item.baseid == "de3":
		char.soundeffect( 0x225 )
		char.addtimer( 5000, delay_campfire, [target.item.serial] )
	else:
		return False

	dough.delete()

def delay_campfire( char, args ):
	campfire = wolfpack.finditem( args[0] )
	if not char.socket or not campfire:
		return False

	#if ( m_From.GetDistanceToSqrt( m_Campfire ) > 3 ):
	#	char.socket.clilocmessage( 500686 ) # You burn the food to a crisp! It's ruined.
	#	return

	if char.checkskill( COOKING, 0, 100 ):
		muffins = wolfpack.additem( "9eb" )
		if not tobackpack( muffins, char ):
			muffins.update()
		char.soundeffect( 0x57 )
	else:
		char.socket.clilocmessage( 500686 ) # You burn the food to a crisp! It's ruined.
	return True

def JarHoney( char, args, target ):
	honey = wolfpack.finditem( args[0] ) 
	if not honey or not target.item:
		return False

	if not target.item.getoutmostchar() == char:
		char.socket.clilocmessage( 1042001 ) # That must be in your pack for you to use it.
		return True

	backpack = char.getbackpack()
	# Dough
	if target.item.baseid == "103d":
		sweet_dough = wolfpack.additem( "sweet_dough" )
		if not tobackpack( sweet_dough, char ):
			sweet_dough.update()
	# Bowl Flour
	if target.item.baseid == "a1e":
		cookie_mix = wolfpack.additem( "103f" )
		if not tobackpack( cookie_mix, char ):
			cookie_mix.update()
	else:
		return False

	honey.delete()
	consume()

def SackFlourOpen( char, args, target ):
	item = wolfpack.finditem( args[0] ) 
	if not item or not target.item:
		return False

	if not target.item.getoutmostchar() == char:
		char.socket.clilocmessage( 1042001 ) # That must be in your pack for you to use it.
		return True

	backpack = char.getbackpack()
	# wooden bowl
	if target.item.baseid == "15f8":
		bowl_flour = wolfpack.additem( "a1e" )
		if not tobackpack( bowl_flour, char ):
			bowl_flour.update()
	# tribal berry
	elif target.item.baseid == "tribal_berry":
		if char.skill[COOKING] >= 800:
			tribal_paint = wolfpack.additem( "tribal_paint" )
			if not tobackpack( tribal_paint, char ):
				tribal_paint.update()

			char.socket.clilocmessage( 1042002 ) # You combine the berry and the flour into the tribal paint worn by the savages.
		else:
			char.socket.clilocmessage( 1042003 ) # You don't have the cooking skill to create the body paint.
	else:
		return False

	item.delete()
	consume(target.item)

# Table of IDs mapped to handler functions
actions =	{
			"103d": "Dough",

			"sweet_dough": "SweetDough",

			"9ec": "JarHoney",

			"1046": "SackFlourOpen",
			"103a": "SackFlourOpen"
		}

def consume( item ):
	quantity = 0
	if item.hastag('quantity'):
		quantity = int(item.gettag('quantity'))

	if quantity <= 0:
		return False # Couldn't consume

	quantity -= 1

	# Empty
	if quantity <= 0:
		item.delete()
	else:
		item.settag('quantity', int(quantity))
		item.resendtooltip()
	return True

def find( char, object = None ):
	# check for dynamic items
	items = wolfpack.items( char.pos.x, char.pos.y, char.pos.map, 2 )
	for item in items:
		if item.id in object:
			return True

	# Check for static items
	for x in range(-2, 3):
		for y in range(-2, 3):
			statics = wolfpack.statics(char.pos.x + x, char.pos.y + y, char.pos.map, True)
			for tile in statics:
				dispid = tile[0]
				if dispid in object:
					return True

	return False

#
# Cook an item
#
class CookItemAction(CraftItemAction):
	def __init__(self, parent, title, itemid, definition):
		CraftItemAction.__init__(self, parent, title, itemid, definition)
		self.needheat = False
		self.needoven = False
		self.water = False
		self.useallres = False
		self.flour = False
		self.markable = 1 # All cooking items are markable, exceptions handled through <nomark /> tag

	#
	# Process special options
	#
	def processnode(self, node, menu):
		if node.name == 'needoven':
			self.needoven = True
		elif node.name == 'needheat':
			self.needheat = True
		elif node.name == 'water':
			self.water = True
		elif node.name == 'nomark':
			self.markable = 0
		elif node.name == 'useallres':
			self.useallres = True
		elif node.name == 'flour':
			self.flour = True

		else:
			CraftItemAction.processnode(self, node, menu)

	#
	# Add our water to the materials list
	#
	def getmaterialshtml(self, player, arguments):
		materialshtml = CraftItemAction.getmaterialshtml(self, player, arguments)
		
		if self.water:
			materialshtml += tr("Water: 1<br>")
		if self.flour:
			materialshtml += tr("Flour: 1<br>")

		return materialshtml

	#
	# Check for water and oven or heat source
	#
	def checkmaterial(self, player, arguments, silent = 0):
		result = CraftItemAction.checkmaterial(self, player, arguments, silent)
		if not result:
			return False

		if self.flour:
			found = False
			backpack = player.getbackpack()
			for item in backpack.content:
				if item.baseid in ['1045', '1046', '1039', '103a'] and item.hastag('quantity'):
					quantity = int(item.gettag('quantity'))
					if quantity > 0:
						found = True
						break
			if not found:
				if not silent:
					player.socket.clilocmessage(1044253) # You don't have the components needed to make that.
				return False

		# Check if we have enough water in our backpack
		if self.water:
			found = False # Found at laest one unit of water?
			backpack = player.getbackpack()
			for item in backpack.content:
				if item.hasscript('beverage') and item.gettag('fluid') == 'water' and item.hastag('quantity'):
					quantity = int(item.gettag('quantity'))
					if quantity > 0:
						found = True
						break

			if not found:
				if not silent:
					player.socket.clilocmessage(1044253) # You don't have the components needed to make that.
				return False

		if self.needheat and not find( player, fires ):
				player.socket.clilocmessage(1044487) # You must be near a fire source to cook.
				return False

		if self.needoven and not find( player, ovens ):
			player.socket.clilocmessage(1044493) # You must be near an oven to bake that.
			return False

		return result

	#
	# Consume material
	#
	def consumematerial(self, player, arguments, half = 0):
		result = CraftItemAction.consumematerial(self, player, arguments, half)
		if not result:
			return False

		if self.flour:
			result = False
			content = player.getbackpack().content
			for item in content:
				if item.baseid in ["1045", "1046", "1039", "103a"] and item.hastag('quantity'):
					if consume(item):
						result = True
						break

		# Check if we have enough water in our backpack
		if self.water:
			content = player.getbackpack().content
			for item in content:
				if item.hasscript('beverage') and item.gettag('fluid') == 'water' and item.hastag('quantity'):
					if beverage.consume(item):
						return True

			player.socket.clilocmessage(1044253) # You don't have the components needed to make that.
			return False

		return result

	def make(self, player, arguments, nodelay=0):
		if self.useallres:
			for item in player.getbackpack().content:
				if item.baseid == self.materials[0][0][0]:
					player.socket.sysmessage( "2r" )
					nodelay = True
					# stop if making fails
					if not CraftItemAction.make(self, player, arguments, nodelay):
						break

		else:
			return CraftItemAction.make(self, player, arguments, nodelay)
#
# Cooking Menu
#
class CookingMenu(MakeMenu):
	def __init__(self, id, parent, title):
		MakeMenu.__init__(self, id, parent, title)
		self.allowmark = True # generally allow marking, but not all items are markable
		self.delay = 1250
		self.gumptype = 0xb10cae72 # This should be unique
		self.requiretool = True # Require a tool

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
	menu = CookingMenu(id, parent, name)

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
		elif child.name == 'cook':
			if not child.hasattribute('definition') or not child.hasattribute('name'):
				console.log(LOG_ERROR, "Cooking action without definition or name in menu %s.\n" % menu.id)
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
							console.log(LOG_ERROR, "Cooking action with invalid definition %s in menu %s.\n" % (itemdef, menu.id))
					else:
						itemid = hex2dec(child.getattribute('itemid', '0'))
					action = CookItemAction(menu, name, int(itemid), itemdef)
				except:
					console.log(LOG_ERROR, "Cooking action with invalid item id in menu %s.\n" % menu.id)

				# Process subitems
				for j in range(0, child.childcount):
					subchild = child.getchild(j)
					action.processnode(subchild, menu)

	# Sort the menu. This is important for the makehistory to make.
	menu.sort()

#
# Load the cooking menu.
#
def onLoad():
	loadMenu('COOKING')

"""
MILL_RANGE = 2
flour_mill = [ 0x1920, 0x1921, 0x1922, 0x1923, 0x1924, 0x1925, 0x1926, 0x1927, 0x1928, 0x1929, 0x192a, 0x192b, 0x192c, 0x192d, 0x192e, 0x192f, 0x1930, 0x1931, 0x1932, 0x1933 ]
# how many doughs a sack of flour can make
FLOUR_AMOUNT = 20
# all the flours
flours = [ 0x1039, 0x103a, 0x1045, 0x1046 ]
# not used flours
flours1 = [ 0x1039, 0x1045 ]
WATER_RANGE = 2
# how much times a pitch of water can be used
WATER_AMOUNT = 10
# these should be in backpack
waters1 = [ 0x0ff8, 0x0ff9 ]
# these should be in range
waters2 = [ 0x0b41, 0x0b42, 0x0b43, 0x0b44, 0x0e7b, 0x0ffa, 0x154d ]
FIRE_RANGE = 2
ovens1 = range( 0x045d, 0x048e )
ovens2 = range( 0x08cf, 0x08df )
ovens3 = range( 0x092b, 0x096c )
ovens4 = range( 0x11ec, 0x11ef )
campfire = range( 0x0de3, 0x0de8 )
firefield = [ 0x398c, 0x3996 ]
fires = ovens1 + ovens2 + ovens3 + ovens4 + campfire, firefield

#def onLoad():
#	wolfpack.registerglobal( HOOK_CHAR, EVENT_SKILLUSE, "skills.cooking" )

def checkmill( pos ):
	found = 0
	items = wolfpack.items( pos.x + i, pos.y + j, pos.map, MILL_RANGE )
	for item in items:
		if item.id in flour_mill:
			found = 1
			break
	return found

def checkfire( pos ):
	found_fire = 0
	items = wolfpack.items( pos.x + i, pos.y + j, pos.map, FIRE_RANGE )
	for item in items:
		if item.id in fires:
			found_fire = 1
			break
	return found_fire
"""
