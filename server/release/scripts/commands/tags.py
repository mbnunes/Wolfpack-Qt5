
"""
	\command settag
	\description Attach tags to objects or change existing ones.
	\usage - <code>settag name type value</code>
	- <code>settag name value</code>
	Type is either int, float or string. If type is not given,
	it defaults to string. Name is the name of the tag you
	want to change or attach while value is the desired tag value.
"""

"""
	\command gettag
	\description Retrieve the value and type of a tag.
	\usage - <code>gettag name</code>
	Shows the type and value of the tag with the given name.
"""

"""
	\command deltag
	\description Delete a tag attached to an object.
	\usage - <code>deltag name</code>
	Deletes the tag with the given name on the selected object.
"""

"""
	\command taginfo
	\description This command shows a menu with all tags assigned to the 
	targetted object.
	\usage - <code>taginfo</code>
"""

import wolfpack
from wolfpack.utilities import hex2dec
from wolfpack.gumps import cGump
from wolfpack.consts import *

def taginfo_callback(player, tagnames, response):
	if response.button != 1:
		return
	
	object = wolfpack.findobject(tagnames[0])	
	if not object:
		return	
	tagnames = tagnames[1:]

	switches = response.switches
	for i in range(0, len(tagnames)):
		# Should the tag be deleted?
		if i * 2 + 1 in switches:
			oldvalue = object.gettag(tagnames[i])
			player.log(LOG_MESSAGE, u"Deleting tag '%s' from object 0x%x. Was: '%s' (%s).\n" % \
				(tagnames[i], object.serial, unicode(oldvalue), type(oldvalue).__name__))
			object.deltag(tagnames[i])
			continue
		
		if response.text.has_key(i * 2 + 0) and response.text.has_key(i * 2 + 1):
			# Get value and name of the tag
			name = response.text[i * 2]
			value = response.text[i * 2 + 1]
			
			# Should the value be interpreted as a number?
			if i * 2 in switches:
				if '.' in value or ',' in value:
					try:
						value = float(value)
					except:
						player.socket.sysmessage("Invalid floating point value for tag '%s': '%s'." % (name, value))
						continue # Skip to next tag
				else:
					try:
						value = int(value)
					except:
						player.socket.sysmessage("Invalid integer value for tag '%s': '%s'." % (name, value))
						continue # Skip to next tag
						
			try:
				name = str(name)
			except:
				player.socket.sysmessage("Invalid tagname: '%s'." % name)
				continue # Skip to next tag
			
			# Set the new tag value for the player
			oldvalue = object.gettag(tagnames[i])
			change = (name != tagnames[i]) or (type(oldvalue) != type(value)) or (oldvalue != value)
			
			if change:			
				player.log(LOG_MESSAGE, u"Settings tag '%s' on object 0x%x to '%s' (%s).\n" % (unicode(name), object.serial, unicode(value), type(value).__name__))
				object.deltag(tagnames[i])
				object.settag(name, value)

def taginfo_response(player, arguments, target):
	if target.char:
		object = target.char
	elif target.item:
		object = target.item
	else:
		player.socket.sysmessage('You have to target an item or a character.')
		return

	tags = object.tags

	dialog = wolfpack.gumps.cGump()
	dialog.setCallback("commands.tags.taginfo_callback")
	dialog.setArgs([object.serial] + tags)
	
	dialog.startPage(0)
	dialog.addResizeGump(35, 12, 9260, 460, 504)
	dialog.addGump(1, 12, 10421, 0)
	dialog.addGump(30, -1, 10420, 0)
	dialog.addResizeGump(66, 40, 9200, 405, 65)
	dialog.addText(108, 52, "Wolfpack Taginfo Command", 2100)
	dialog.addTiledGump(90, 11, 164, 17, 10250, 0)
	dialog.addGump(474, 12, 10431, 0)
	dialog.addGump(439, -1, 10430, 0)
	dialog.addGump(14, 200, 10422, 0)
	dialog.addGump(468, 200, 10432, 0)
	dialog.addGump(249, 11, 10254, 0)
	dialog.addGump(74, 45, 10464, 0)
	dialog.addGump(435, 45, 10464, 0)
	dialog.addGump(461, 408, 10412, 0)
	dialog.addGump(-15, 408, 10402, 0)
	dialog.addTiledGump(281, 11, 158, 17, 10250, 0)
	dialog.addGump(265, 11, 10252, 0)
	dialog.addButton(60, 476, 247, 248, 1)
	dialog.addButton(136, 476, 242, 241, 0)
	
	# This is a group
	count = (len(tags) + 3) / 4
	
	for i in range(0, count):		
		page = i + 1
		dialog.startPage(page)
		
		if page > 1:
			dialog.addText(88, 445, "Previous Page", 2100)
			dialog.addPageButton(59, 444, 9909, 9911, page - 1)

		if page < count:
			dialog.addText(376, 445, "Next Page", 2100)
			dialog.addPageButton(448, 445, 9903, 9905, page + 1)
		
		for j in range(0, 4):
			tagid = i * 4 + j			
			if tagid >= len(tags):
				continue
				
			tag = tags[tagid]
			value = object.gettag(tag)
			yoffset = j * 80
			dialog.addResizeGump(65, 109 + yoffset, 9200, 405, 68)			
			dialog.addText(78, 117 + yoffset, "Name", 2100)				
			dialog.addText(78, 146 + yoffset, "Value", 2100)
			dialog.addResizeGump(123, 144 + yoffset, 9300, 250, 26)
			dialog.addInputField(129, 147 + yoffset, 240, 20, 2100, tagid * 2 + 1, unicode(value))	
			dialog.addResizeGump(123, 115 + yoffset, 9300, 250, 26)
			dialog.addInputField(128, 118 + yoffset, 240, 20, 2100, tagid * 2 + 0, tag)
			
			dialog.addCheckbox(380, 118 + yoffset, 208, 209, tagid * 2 + 1, 0)
			dialog.addText(405, 118 + yoffset, "Delete", 2100)			
			
			dialog.addCheckbox(380, 147 + yoffset, 208, 209, tagid * 2 + 0, type(value) == float or type(value) == int)
			dialog.addText(405, 147 + yoffset, "Number", 2100)

	dialog.send(player)

#
# Tag info
#
def commandTaginfo(socket, command, arguments):
	socket.sysmessage("Select an object you want to use this command on.")
	socket.attachtarget("commands.tags.taginfo_response", [])
	
#
# Target response
#
def settagResponse(player, arguments, target):
	(name, value) = arguments

	if target.char:
		if target.char.rank >= player.rank and player != target.char:
			player.socket.sysmessage("You've burnt your fingers!")
			return

		if not target.char.hastag(name):
			change = 1
		else:		
			oldvalue = target.char.gettag(name)
			change = (type(oldvalue) != type(value)) or (oldvalue != value)
		
		if change:
			player.log(LOG_MESSAGE, u"Settings tag '%s' on object 0x%x to '%s' (%s).\n" % (unicode(name), target.char.serial, unicode(value), type(value).__name__))
			target.char.settag(name, value)
			target.char.resendtooltip()
		player.socket.sysmessage('You modify the tag of the character.')
	elif target.item:
		if not target.item.hastag(name):
			change = 1
		else:		
			oldvalue = target.item.gettag(name)
			change = (type(oldvalue) != type(value)) or (oldvalue != value)
			
		if change:
			player.log(LOG_MESSAGE, u"Settings tag '%s' on object 0x%x to '%s' (%s).\n" % (unicode(name), target.item.serial, unicode(value), type(value).__name__))
			target.item.settag(name, value)
			target.item.resendtooltip()

		player.socket.sysmessage('You modify the tag of the item.')

	else:
		player.socket.sysmessage('You need to target a character or an item.')

#
# Sets a tag for the item
#
def commandSettag(socket, command, arguments):
	# Split Arguments
	if arguments.count(' ') < 2:
		socket.sysmessage('Usage: settag name (int|string|float) value...')
		return

	(name, argtype, value) = arguments.split(' ', 2)

	if argtype == 'int':
		try:
			value = hex2dec(value)
		except:
			socket.sysmessage('You specified an invalid integer value.')
			return
	elif argtype == 'float':
		try:
			value = float(value)
		except:
			socket.sysmessage('You specified an invalid floating point value.')
			return
	elif argtype == 'string':
			value = unicode(value)
	else:
		socket.sysmessage('Usage: settag name (int|string|float) value...')
		return

	socket.attachtarget("commands.tags.settagResponse", [name, value])

#
# Target response
#
def deltagResponse(player, arguments, target):
	name = arguments[0]

	if target.char:
		if target.char.rank >= player.rank and player != target.char:
			player.socket.sysmessage("You've burnt your fingers!")
			return

		if not target.char.hastag(name):
			player.socket.sysmessage("This character has no tag named '%s'." % name)
		else:
			oldvalue = target.char.gettag(name)
			player.log(LOG_MESSAGE, u"Deleting tag '%s' from object 0x%x. Was: '%s' (%s).\n" % \
				(name, target.char.serial, unicode(oldvalue), type(oldvalue).__name__))
			target.char.deltag(name)
			target.char.resendtooltip()
			player.socket.sysmessage("You delete the tag named '%s'." % name)
	elif target.item:
		if not target.item.hastag(name):
			player.socket.sysmessage("This item has no tag named '%s'." % name)
		else:
			oldvalue = target.item.gettag(name)
			player.log(LOG_MESSAGE, u"Deleting tag '%s' from object 0x%x. Was: '%s' (%s).\n" % \
				(name, target.item.serial, unicode(oldvalue), type(oldvalue).__name__))
			target.item.deltag(name)
			target.item.resendtooltip()
			player.socket.sysmessage("You delete the tag named '%s'." % name)

	else:
		player.socket.sysmessage('You need to target a character or an item.')

#
# Target response
#
def gettagResponse(player, arguments, target):
	name = arguments[0]

	if target.char:
		if target.char.rank >= player.rank and player != target.char:
			player.socket.sysmessage("You've burnt your fingers!")
			return

		if not target.char.hastag(name):
			player.socket.sysmessage("This character has no tag named '%s'." % name)
			return
		else:
			value = target.char.gettag(name)
	elif target.item:
		if not target.item.hastag(name):
			player.socket.sysmessage("This item has no tag named '%s'." % name)
			return
		else:
			value = target.item.gettag(name)

	else:
		player.socket.sysmessage('You need to target a character or an item.')
		return

	if type(value) == unicode:
		typename = 'string'
	elif type(value) == int:
		typename = 'integer'
	elif type(value) == float:
		typename = 'float'

	player.socket.sysmessage("The %s value for tag '%s' is: %s" % (typename, name, str(value)))

#
# Gets a tag from the item
#
def commandGettag(socket, command, arguments):
	if len(arguments) == 0 or ' ' in arguments:
		socket.sysmessage('Usage: gettag name')
		return

	socket.sysmessage("From which object do you want to get tag '%s'?" % arguments)
	socket.attachtarget("commands.tags.gettagResponse", [arguments])

#
# Delete a tag
#
def commandDeltag(socket, command, arguments):
	if len(arguments) == 0 or ' ' in arguments:
		socket.sysmessage('Usage: deltag name')
		return

	socket.sysmessage("From which object do you want to remove the tag '%s'?" % arguments)
	socket.attachtarget("commands.tags.deltagResponse", [arguments])

#
# Register the command with the server
#
def onLoad():
	wolfpack.registercommand('settag', commandSettag)
	wolfpack.registercommand('gettag', commandGettag)
	wolfpack.registercommand('deltag', commandDeltag)
	wolfpack.registercommand('taginfo', commandTaginfo)