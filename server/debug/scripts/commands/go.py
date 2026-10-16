r"""
	\command go
	\description Move to given coordinates or show a menu with predefined locations.
	\usage - <code>go</code>
	- <code>go x,y</code>
	- <code>go x,y,z</code>
	- <code>go x,y,z,map</code>
	- <code>go location</code>
	- <code>go reload</code>
	- <code>go maps</code>

	If you don't specify any of the parameters a menu with all categorized locations
	will be shown. It automatically opens at your current map or last visited location.
	If you specify the id or name of a location, you will be transported to that position.
	If you specify coordinates, you will be transported there.
"""

r"""
	\command send
	\description Move another character or item to the given coordinate, location or select the target.
	\usage - <code>send</code>
	- <code>send x,y</code>
	- <code>send x,y,z</code>
	- <code>send x,y,z,map</code>
	- <code>send location</code>

	If you don't specify any of the parameters, you will be able to select the location
	the object should be sent via a target cursor. Otherwise the selected object will be sent
	to the given location or coordinates. See the <command id="GO">GO</command> command for details.
"""

import wolfpack
try:
	from wolfpack import tr
except ImportError:
	tr = getattr(wolfpack, 'tr', lambda s: s)
import re
from wolfpack.consts import *
from system.makemenus import MakeMenu, MakeAction, findmenu, enabledhue, enabledcolor, disabledcolor, disabledhue
from wolfpack.utilities import isValidPosition

generated = 0

MAP_NAMES = {
	0: 'Felucca',
	1: 'Trammel',
	2: 'Ilshenar',
	3: 'Malas',
	4: 'Tokuno Islands',
	5: 'Ter Mur',
}

def findLocationMatch(query, prefer_map=None):
	query = query.strip().lower()
	if not query:
		return None

	query_words = set(re.findall(r'[a-zA-Z0-9]+', query))
	locations = wolfpack.getdefinitions(WPDT_LOCATION)

	# 1. Exact ID match (case-insensitive)
	for loc in locations:
		loc_id = loc.getattribute('id').lower()
		if loc_id == query:
			return loc

	# 2. Exact action name / last category match
	matches = []
	for loc in locations:
		if not loc.hasattribute('category'):
			continue
		cat = loc.getattribute('category')
		parts = cat.split('\\')
		last_part = parts[-1].lower() if parts else ''
		if last_part == query or query in cat.lower():
			matches.append(loc)

	if matches:
		if prefer_map is not None:
			for m in matches:
				try:
					map_id = int(m.text.split(',')[3])
					if map_id == prefer_map:
						return m
				except:
					pass
		return matches[0]

	# 3. All query words match within category or ID
	word_matches = []
	for loc in locations:
		cat = loc.getattribute('category').lower() if loc.hasattribute('category') else ''
		loc_id = loc.getattribute('id').lower()
		text = cat + ' ' + loc_id
		if query_words.issubset(set(re.findall(r'[a-zA-Z0-9]+', text))):
			word_matches.append(loc)

	if word_matches:
		if prefer_map is not None:
			for m in word_matches:
				try:
					map_id = int(m.text.split(',')[3])
					if map_id == prefer_map:
						return m
				except:
					pass
		return word_matches[0]

	return None

def sendresponse2(player, arguments, target):
	if target.item:
		if target.item.container:
			return
		else:
			pos = target.item.pos
	elif target.char:
		pos = target.char.pos
	else:
		pos = target.pos

	if arguments[0] >= 0x40000000:
		object = wolfpack.finditem(arguments[0])
	else:
		object = wolfpack.findchar(arguments[0])

	if not object:
		return
	object.removefromview()
	object.moveto(pos)
	object.update()
	if not object.ischar() or not object.socket:
		return
	object.socket.resendworld()
	return

def sendresponse(player, arguments, target):
	if target.item:
		object = target.item
	elif target.char:
		if target.char.rank > player.rank and player != target.char:
			player.socket.sysmessage("You've burnt your fingers!")
			return

		object = target.char
	else:
		player.socket.sysmessage('You need to target a character or an item.')
		return

	# Maybe we need to choose a location
	if len(arguments) == 0:
		player.socket.sysmessage('Where do you want to send the targetted object?')
		player.socket.attachtarget('commands.go.sendresponse2', [object.serial])
	else:
		# Try to parse a target coordinate for the object.
		parts = [p.strip() for p in arguments[0].split(',')]
		pos = player.pos
		try:
			if len(parts) >= 2:
				pos.x = int(parts[0])
				pos.y = int(parts[1])
				if len(parts) >= 3:
					pos.z = int(parts[2])
				if len(parts) >= 4:
					pos.map = int(parts[3])

				if not isValidPosition( pos ):
					player.socket.sysmessage( "Error: Destination invalid!" )
					return False

				object.removefromview()
				object.moveto(pos)
				object.update()
				if not object.ischar() or not object.socket:
					return
				object.socket.resendworld()
				return
		except:
			pass

		# If we reach this point it was no valid coordinate, try looking up location definition
		location = wolfpack.getdefinition(WPDT_LOCATION, arguments[0])
		if not location:
			location = findLocationMatch(arguments[0], player.pos.map)

		if location:
			(x, y, z, map) = location.text.split(',')
			pos = wolfpack.coord(int(x), int(y), int(z), int(map))
			if not isValidPosition( pos ):
				player.socket.sysmessage( "Error: Destination invalid!" )
				return False
			object.removefromview()
			object.moveto(pos)
			object.update()

			if object.ischar() and object.socket:
				object.socket.resendworld()
		else:
			player.socket.sysmessage('Usage: send <x, y, z, map>|<location>')
	return

#
# Send an object to a given location
#
def send(socket, command, arguments):
	if len(arguments) > 0:
		socket.sysmessage("Please choose the object you want to send to '%s'." % arguments)
		socket.attachtarget('commands.go.sendresponse', [arguments])
	else:
		socket.sysmessage("Please choose the object you want to teleport.")
		socket.attachtarget('commands.go.sendresponse', [])
	return

#
# This action moves the character to a given position
#
class GoAction(MakeAction):
	def __init__(self, parent, title, pos):
		MakeAction.__init__(self, parent, title)
		self.pos = pos
		return

	def make(self, player, arguments, nodelay=0):
		# Append to player's last10 history
		if self.index != -1:
			historyname = 'makehistory%u' % self.parent.gumptype if self.parent.gumptype != 0 else 'makehistory'
			identifier = '%s:%u' % (self.parent.id, self.index)
			history = []
			if player.hastag(historyname):
				history = str(player.gettag(historyname)).split(';')
				if identifier in history:
					history.remove(identifier)
			history = [identifier] + history[:9]
			player.settag(historyname, ';'.join(history))

		# Remember last branch for this map and overall
		player.settag('last_go_branch_%d' % self.pos.map, self.parent.id)
		player.settag('last_go_menu', self.parent.id)

		player.removefromview()
		player.moveto(self.pos)
		player.update()
		player.socket.resendworld()
		return

#
# Subclass of MakeMenu tailored for travel navigation
#
class GoMenu(MakeMenu):
	def __init__(self, id, parent, title):
		MakeMenu.__init__(self, id, parent, title)
		self.gumptype = 0x474F # Unique gump ID for Go Menu
		self.name_makelast = tr("All Maps")

	def addbuttons(self, gump, player, arguments, submenu = 0):
		# EXIT button (0xFB1, 0xFB3)
		gump.addButton(15, 350, 0xFB1, 0xFB3, 0)
		gump.addText(50, 353, tr("Exit"), enabledhue)

		# ALL MAPS / ROOT button (return value 2)
		if self.parent:
			gump.addButton(15, 330, 4005, 4007, 2)
			gump.addText(50, 333, tr("All Maps"), enabledhue)

		# PREVIOUS MENU button (return value 3)
		if submenu:
			gump.addButton(15, 310, 0xFAE, 0xFB0, 9)
			gump.addText(50, 313, tr("Back"), enabledhue)
		elif self.parent:
			gump.addButton(15, 310, 0xFAE, 0xFB0, 3)
			gump.addText(50, 313, tr("Previous Menu"), enabledhue)

		# LAST 10 (return value 1)
		if not submenu:
			gump.addButton(15, 60, 0xFAB, 0xFAD, 1)
			gump.addText(50, 63, tr("Last Ten"), enabledhue)

	def response(self, player, response, arguments):
		# Button 2: Jump directly to All Maps / Root
		if response.button == 2:
			root = findmenu('GOMENU')
			if root:
				root.send(player, arguments)
			return

		# Button 3: Previous menu
		if response.button == 3:
			if self.parent:
				player.settag('last_go_branch_%d' % player.pos.map, self.parent.id)
				player.settag('last_go_menu', self.parent.id)
				self.parent.send(player, arguments)
			return

		# Enter submenu (button & 0x01000000)
		if response.button & 0x01000000:
			submenu = response.button & ~ 0x01000000
			if submenu < len(self.submenus):
				sub = self.submenus[submenu]
				player.settag('last_go_branch_%d' % player.pos.map, sub.id)
				player.settag('last_go_menu', sub.id)
				sub.send(player, arguments)
			return

		# Delegate remaining actions (Exit, Last 10, Selections, etc.) to MakeMenu
		MakeMenu.response(self, player, response, arguments)

#
# Generate a menu structure out of the
# category tags of our locations.
#
def generateGoMenu():
	locations = wolfpack.getdefinitions(WPDT_LOCATION)

	gomenu = GoMenu('GOMENU', None, 'Go Menu')
	submenus = {}

	for location in locations:
		if not location.hasattribute('category'):
			continue
		categories = location.getattribute('category').split('\\')
		description = categories[len(categories)-1] # Name of the action
		categories = categories[:len(categories)-1]

		# Iterate through the categories and see if they're all there
		category = ''
		if len(categories) > 0 and ('\\'.join(categories) + '\\') not in submenus:
			for subcategory in categories:
				if (category + subcategory + '\\') not in submenus:
					# Category is our parent category
					parent = None
					if len(category) == 0:
						parent = gomenu
					elif category in submenus:
						parent = submenus[category]

					category += subcategory + '\\'
					menu = GoMenu('GOMENU_' + category, parent, subcategory)
					submenus[category] = menu
				else:
					category += subcategory + '\\'

		# Parse the position of this makemenu entry
		if location.text.count(',') != 3:
			raise RuntimeError( "Wrong position information for location %s." % location.getattribute('id') )

		(x, y, z, map) = location.text.split(',')
		pos = wolfpack.coord(int(x), int(y), int(z), int(map))

		if len(categories) == 0:
			GoAction(gomenu, description, pos)
		else:
			GoAction(submenus['\\'.join(categories) + '\\'], description, pos)

	for menu in submenus.values():
		menu.sort()

	gomenu.sort()
	return

#
# Shows a certain travel menu to the user.
#
def go(socket, command, arguments):
	player = socket.player

	global generated
	if not generated:
		socket.sysmessage('Generating go menu...')
		generateGoMenu()
		generated = 1

	# No arguments -> open travel gump
	if len(arguments) == 0:
		# Auto-open at player's map or last visited branch
		current_map = player.pos.map
		map_name = MAP_NAMES.get(current_map, 'Felucca')
		last_branch_tag = 'last_go_branch_%d' % current_map

		target_menu = None
		if player.hastag(last_branch_tag):
			saved_id = str(player.gettag(last_branch_tag))
			target_menu = findmenu(saved_id)

		if not target_menu:
			target_menu = findmenu('GOMENU_' + map_name + '\\')

		# Fallback checks (e.g. Tokuno)
		if not target_menu and current_map == 4:
			target_menu = findmenu('GOMENU_Tokuno\\')

		if not target_menu:
			target_menu = findmenu('GOMENU')

		if target_menu:
			target_menu.send(player)
		else:
			socket.sysmessage("Didn't find the Go Menu.")
		return

	# Explicit root menu request
	if arguments.lower() in ('menu', 'maps', 'root', 'all'):
		menu = findmenu('GOMENU')
		if menu:
			menu.send(player)
		return

	# Admin reload
	if arguments.lower() == 'reload':
		generateGoMenu()
		socket.sysmessage('Go menu regenerated successfully.')
		return

	# Coordinate input: x,y or x,y,z or x,y,z,map
	if arguments.count(',') >= 1:
		parts = [p.strip() for p in arguments.split(',')]
		pos = player.pos

		try:
			pos.x = int(parts[0])
			pos.y = int(parts[1])
			if len(parts) >= 3:
				pos.z = int(parts[2])
			if len(parts) >= 4:
				pos.map = int(parts[3])

			if not isValidPosition( pos ):
				socket.sysmessage( "Error: Destination invalid!" )
				return False

			player.removefromview()
			player.moveto(pos)
			player.update()
			player.socket.resendworld()
			return
		except:
			pass

	# Named location search
	location = wolfpack.getdefinition(WPDT_LOCATION, arguments)
	if not location:
		location = wolfpack.getdefinition(WPDT_LOCATION, arguments.lower())
	if not location:
		location = findLocationMatch(arguments, player.pos.map)

	if location:
		(x,y,z,map) = location.text.split(',')
		pos = wolfpack.coord(int(x), int(y), int(z), int(map))
		if not isValidPosition( pos ):
			socket.sysmessage( "Error: Destination invalid!" )
			return False
		player.removefromview()
		player.moveto(pos)
		player.update()
		player.socket.resendworld()
	else:
		socket.sysmessage('Usage: go [<location_name> | <x,y> | <x,y,z> | <x,y,z,map>]')
	return

#
# Register commands.
#
def onLoad():
	wolfpack.registercommand('go', go)
	wolfpack.registercommand('send', send)
	return

def onUnload():
	global generated
	generated = 0
	return
