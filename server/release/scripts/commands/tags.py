
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

import wolfpack
from wolfpack.utilities import hex2dec

#
# Target response
#
def settagResponse(player, arguments, target):
	(name, value) = arguments

	if target.char:
		if target.char.rank >= player.rank and player != target.char:
			player.socket.sysmessage("You've burnt your fingers!")
			return

		target.char.settag(name, value)
		target.char.resendtooltip()
		player.socket.sysmessage('You modify the tag of the character.')
	elif target.item:
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
	elif argtype != 'string':
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
			target.char.deltag(name)
			target.char.resendtooltip()
			player.socket.sysmessage("You delete the tag named '%s'." % name)
	elif target.item:
		if not target.item.hastag(name):
			player.socket.sysmessage("This item has no tag named '%s'." % name)
		else:
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
