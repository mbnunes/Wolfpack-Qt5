"""
	\command tilecolor
	\description Dyes all items in an area with the given color.
	\usage - <code>tilecolor color</code>
	- <code>tilecolor color1,color2,...,colorn</coode>
	Color is the id of the color you want to hue the items in.
	If you give more than one color, one color is randomly selected
	from the given list for every tile that is found.		
"""

import wolfpack
from wolfpack.consts import *
import random
from wolfpack.utilities import hex2dec

#
# Gather another corner if neccesary and start tiling
#
def tileResponse(player, arguments, target):
	if len(arguments) < 2:
		player.socket.sysmessage('Please select the second corner.')
		player.socket.attachtarget("commands.tilecolor.tileResponse", [arguments[0], target.pos])
		return

	x1 = min(arguments[1].x, target.pos.x)
	x2 = max(arguments[1].x, target.pos.x)
	y1 = min(arguments[1].y, target.pos.y)
	y2 = max(arguments[1].y, target.pos.y)
	colors = arguments[0]
	
	unlimited = player.account.authorized('Misc', 'Unlimited Tilecolor')
	count = ((x2 - x1) + 1) * ((y2 - y1) + 1)
	
	# Cap at 500 items if not an admin is using it
	if not unlimited and count > 250:
		player.socket.sysmessage('You are not allowed to tile more than 250 items at once.')
		return

	player.log(LOG_MESSAGE, "Dying items (%s) from %u,%u to %u,%u.\n" % (", ".join(colors), x1, y1, x2, y2))
	player.socket.sysmessage('Dying items from %u,%u to %u,%u.' % (x1, y1, x2, y2))
	
	try:
		colors = map(hex2dec, colors)
	except:
		player.socket.sysmessage('You have to pass a comma separated list of numbers.')
		player.socket.sysmessage('Hexadecimal numbers have to be in the 0x123 format.')
		raise
	
	mapid = player.pos.map
	for x in range(x1, x2 + 1):
		for y in range(y1, y2 + 1):
			items = wolfpack.items(x, y, mapid, 0)
			for item in items:
				color = random.choice(colors)
				# This should be a trace log
				#player.log(LOG_MESSAGE, "Dying item 0x%x (%s) from color 0x%x to 0x%x.\n" % (item.serial, item.baseid, item.color, color))
				item.color = color
				item.update()

#
# Gather arguments and validate id list.
#
def commandTilecolor(socket, command, arguments):
	if len(arguments) == 0 or arguments.count(' ') > 0:
		socket.sysmessage('Usage: .tilecolor id{,id}')
		return

	(ids, ) = arguments.split(' ')

	ids = ids.split(',')
	
	socket.sysmessage('Please select the first corner.')
	socket.attachtarget('commands.tilecolor.tileResponse', [ids])

#
# Register the command
#
def onLoad():
	wolfpack.registercommand("tilecolor", commandTilecolor)
