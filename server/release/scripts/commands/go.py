
"""
	\command go
	\description Move to given coordinates or show a menu with predefined locations.
	\usage - <code>go</code>
	- <code>go x,y</code>
	- <code>go x,y,z</code>
	- <code>go x,y,z,map</code>
	- <code>go location</code>
	
	If you don't specify any of the parameters a menu with all categorized locations 
	will be shown. If you specify the id of a location, you will be transported to the 
	locations position. If you specify a coordinate, you will be transported there.	
"""

"""
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
from wolfpack.consts import *
from system.makemenus import MakeMenu, MakeAction, findmenu

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

def sendresponse(player, arguments, target):
	if target.item:
		object = target.item
	elif target.char:
		if target.char.rank >= player.rank and player != target.char:
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
		# Try to parse a target location for the object.
		parts = arguments[0].split(',')
		pos = player.pos
		try:
			pos.x = int(parts[0])
			pos.y = int(parts[1])
			if len(parts) >= 3:
				pos.z = int(parts[2])

			if len(parts) >= 4:
				pos.map = int(parts[3])

			object.removefromview()
			object.moveto(pos)
			object.update()
			if not object.ischar() or not object.socket:
				return
			object.socket.resendworld()
			return
		except:
			pass

		# If we reach this point it was no valid coordinate
		# See if we can get a def
		location = wolfpack.getdefinition(WPDT_LOCATION, arguments[0])
		
		if location:
			(x,y,z,map) = location.text.split(',')		
			pos = wolfpack.coord(int(x), int(y), int(z), int(map))
			object.removefromview()
			object.moveto(pos)
			object.update()
		else:
			player.socket.sysmessage('Usage: send <x, y, z, map>|<location>')		

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

#
# Shows a certain makemenu to the user.
#
def go(socket, command, arguments):
	player = socket.player

	if len(arguments) == 0:
		menu = findmenu('GOMENU')
		if menu:
			socket.sysmessage('Bringing up the travel gump.')
			menu.send(player)
		else:
			socket.sysmessage("Didn't find the GOMENU menu.")
		return
	elif arguments.count(',') >= 1:
		parts = arguments.split(',')
		pos = player.pos

		try:
			pos.x = int(parts[0])
			pos.y = int(parts[1])
			if len(parts) >= 3:
				pos.z = int(parts[2])

			if len(parts) >= 4:
				pos.map = int(parts[3])

			player.removefromview()
			player.moveto(pos)
			player.update()
			player.socket.resendworld()			
			return
		except:
			pass

	# If we reach this point it was no valid coordinate
	# See if we can get a def
	location = wolfpack.getdefinition(WPDT_LOCATION, arguments)

	if location:
		(x,y,z,map) = location.text.split(',')		
		pos = wolfpack.coord(int(x), int(y), int(z), int(map))
		player.removefromview()
		player.moveto(pos)
		player.update()
		player.socket.resendworld()
	else:
		socket.sysmessage('Usage: <x, y, z, map>|<location>')

#
# This action moves the character to a given position
#
class GoAction(MakeAction):
	#
	# Create a new makemenu entry.
	#
	def __init__(self, parent, title, pos):
		MakeAction.__init__(self, parent, title)
		self.pos = pos

	#
	# Teleport the user to the position of this
	# makemenu action. We don't add it to the 
	# "make last" action because it makes no sense.
	#
	def make(self, player, arguments):
		player.removefromview()
		player.moveto(self.pos)
		player.update()
		player.socket.resendworld()

#
# Generate a menu structure out of the
# category tags of our locations.
#
def generateGoMenu():
	locations = wolfpack.getdefinitions(WPDT_LOCATION)
	
	gomenu = MakeMenu('GOMENU', None, 'Go Menu')
	submenus = {}

	for location in locations:
		if not location.hasattribute('category'):
			continue
		categories = location.getattribute('category').split('\\')
		description = categories[len(categories)-1] # Name of the action
		categories = categories[:len(categories)-1]

		# Iterate trough the categories and see if they're all there		
		category = ''
		if len(categories) > 0 and not submenus.has_key('\\'.join(categories) + '\\'):
			for subcategory in categories:
				if not submenus.has_key(category + subcategory + '\\'):
					# Category is our parent category 
					parent = None
					if len(category) == 0:
						parent = gomenu
					elif submenus.has_key(category):
						parent = submenus[category]

					category += subcategory + '\\'
					menu = MakeMenu('GOMENU_' + category, parent, subcategory)
					submenus[category] = menu
				else:
					category += subcategory + '\\'

		# Parse the position of this makemenu entry
		if location.text.count(',') != 3:
			raise RuntimeError, "Wrong position information for location %s." % location.getattribute('id')

		(x, y, z, map) = location.text.split(',')
		pos = wolfpack.coord(int(x), int(y), int(z), int(map))

		if len(categories) == 0:
			GoAction(gomenu, description, pos)
		else:
			GoAction(submenus['\\'.join(categories) + '\\'], description, pos)
	
	gomenu.sort()
	for menu in submenus.values():
		menu.sort()

#
# Kommando registrieren.
#
def onLoad():
	generateGoMenu()
	wolfpack.registercommand('go', go)
	wolfpack.registercommand('send', send)
