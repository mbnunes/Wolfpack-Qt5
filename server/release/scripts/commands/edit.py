
"""
	\command edit
	\description Shows a gump with equipped items for a character.
	\notes This command allows you to delete items (Delete), put items into your backpack (Bounce),
	or show an info gump for items (Info). This command is especially useful if a character has broken
	items equipped he cannot unequip or if you want to modify the hair or facial hair.
"""

from wolfpack.utilities import tobackpack
import wolfpack
from wolfpack.consts import *
import wolfpack.gumps
from math import ceil
from commands.info import iteminfo

#
# Gump response
#
def response(player, arguments, response):
	if response.button == 0:
		return

	command = (response.button >> 28) & 0xC
	item = wolfpack.finditem((response.button & 0x3FFFFFFF) | 0x40000000)

	# Delete Item
	if command == 0x04:
		player.log(LOG_MESSAGE, 'Deleting item 0x%x from character 0x%x.\n' % (item.serial, item.container.serial))
		item.delete()

	# Bounce Item
	elif command == 0x08:
		player.log(LOG_MESSAGE, 'Bouncing item 0x%x from character 0x%x.\n' % (item.serial, item.container.serial))
		
		if not tobackpack(item, player):			
			item.update()

	# Show Info For Item
	elif command == 0x0C:
		player.log(LOG_MESSAGE, 'Showing info gump for item 0x%x from character 0x%x.\n' % (item.serial, item.container.serial))
		iteminfo(player.socket, item)

	else:
		player.socket.sysmessage('Unknown command.')

#
# Show the edit gump
#
def callback(player, arguments, target):
	if not target.char:
		return

	dialog = wolfpack.gumps.cGump()
	dialog.setCallback("commands.edit.response")

	items = []

	for layer in range(LAYER_RIGHTHAND, LAYER_TRADING+1):
		item = target.char.itemonlayer(layer)
		if item:
			items.append(item)

	pages = ceil(len(items) / 4.0)

	dialog.startPage(0)
	dialog.addResizeGump(35, 12, 9260, 460, 504)
	dialog.addGump(1, 12, 10421, 0)
	dialog.addGump(30, -1, 10420, 0)
	dialog.addResizeGump(66, 40, 9200, 405, 65)
	dialog.addText(108, 52, "Wolfpack Edit Command", 2100)
	dialog.addText(108, 72, "Please choose the item you wan't to view closely.", 2100)
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

	for page in range(1, pages + 1):
		dialog.startPage(page)

		if page > 1:
			dialog.addPageButton(60, 467, 9909, 9911, page - 1)
			dialog.addText(88, 467, "Previous Page", 2100)

		if page < pages:
			dialog.addPageButton(448, 467, 9903, 9905, page + 1)
			dialog.addText(376, 468, "Next Page", 2100)

		yoffset = 0
		for i in range(0, 4):
			if (page - 1) * 4 + i >= len(items):
				break

			item = items[(page - 1) * 4 + i]

			dialog.addResizeGump(64, 108 + yoffset, 9200, 405, 82)
			dialog.addTilePic(92, 127 + yoffset, item.id)

			if LAYERNAMES.has_key(item.layer):
				layername = LAYERNAMES[item.layer]
			else:
				layername = 'Unknown'

			dialog.addText(164, 118 + yoffset, "Layer: %u (%s)" % (item.layer, layername), 2100)
			dialog.addText(164, 138 + yoffset, "Item Id: 0x%x" % item.id, 2100)

			if item.color == 0:
				textcolor = 2100
			else:
				textcolor = item.color - 1

			dialog.addText(164, 158 + yoffset, "Item Color: 0x%x" % item.color, textcolor)

			itemid = item.serial & 0x3FFFFFFF

			dialog.addText(348, 115 + yoffset, "Delete Item", 2100)
			dialog.addButton(428, 114 + yoffset, 9903, 9905, itemid | 0x40000000)

			dialog.addText(376, 139 + yoffset, "Bounce", 2100)
			dialog.addButton(428, 138 + yoffset, 9903, 9905, itemid | 0x80000000)

			dialog.addText(396, 163 + yoffset, "Info", 2100)
			dialog.addButton(428, 162 + yoffset, 9903, 9905, itemid | 0xC0000000)
			yoffset += 86


	dialog.send(player)

#
# Show the edit target
#
def edit(socket, command, arguments):
	socket.sysmessage('Please select an item or character you want to edit.')
	socket.attachtarget('commands.edit.callback', [])

#
# Edit debugging command
#
def onLoad():
	wolfpack.registercommand('edit', edit)
