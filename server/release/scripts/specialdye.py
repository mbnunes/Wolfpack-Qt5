
import wolfpack.gumps
import wolfpack
from wolfpack.consts import *
from wolfpack import properties, tr

HUES = [
	# Violet 
	[1018345, 1230, 1231, 1232, 1233, 1234, 1235],
	# Tan 
	[1018346, 1501, 1502, 1503, 1504, 1505, 1506, 1507, 1508],
	# Brown 
	[1018347, 2012, 2013, 2014, 2015, 2016, 2017],
	# Dark Blue 
	[1018348, 1303, 1304, 1305, 1306, 1307, 1308],
	# Forest Green 
	[1018349, 1420, 1421, 1422, 1423, 1424, 1425, 1426],
	# Pink 
	[1018350, 1619, 1620, 1621, 1622, 1623, 1624, 1625, 1626],
	# Red 
	[1018351, 1640, 1641, 1642, 1643, 1644],
	# Olive 
	[1018352, 2001, 2002, 2003, 2004, 2005],
]

#
# Gump Callback
#
def pickHueCallback(player, arguments, response):
	dyetub = wolfpack.finditem(arguments[0])

	if not player.canreach(dyetub, 1):
		player.socket.clilocmessage(500446) # You can't reach that
		return

	if response.button == 1:
		if len(response.switches) == 0:
			return

		switch = response.switches[0]

		(i, j) = (switch / 15, switch % 15)

		if i >= len(HUES) or j + 1 >= len(HUES[i]):
			return

		dyetub.color = HUES[i][j + 1] # Special Hue
		dyetub.update()
		player.soundeffect(0x023e)

	elif response.button == 2:
		dyetub.color = 0 # Default Hue
		dyetub.update()
		player.soundeffect(0x023e)

#
# Special Dyetub
#
def pickHue(player, dyetub):
	dialog = wolfpack.gumps.cGump()

	dialog.setArgs([dyetub.serial])
	dialog.setCallback(pickHueCallback)

	dialog.startPage(0)
	dialog.addResizeGump(0, 0, 5054, 450, 450)
	dialog.addResizeGump(10, 10, 3000, 430, 430)

	dialog.addButton(20, 400, 4005, 4007, 1) # Ok
	dialog.addXmfHtmlGump(55, 400, 200, 25, 1011036)

	dialog.addButton(200, 400, 4005, 4007, 2) # Default
	dialog.addText(235, 400, tr("DEFAULT") )

	# Create the list of hues
	global HUES

	for i in range(0, len(HUES)):
		dialog.addPageButton(30, 85 + i * 25, 5224, 5224, 1 + i)
		dialog.addXmfHtmlGump(55, 85 + i * 25, 200, 25, HUES[i][0])

	for i in range(0, len(HUES)):
		dialog.startPage(1 + i)

		colors = HUES[i][1:]
		for j in range(0, len(colors)):
			dialog.addRadioButton(260, 90 + j * 25, 210, 211, 15 * i + j)
			dialog.addText(278, 90 + j * 25, "*****", colors[j] - 1)

	dialog.send(player)

#
# Dying response
#
def target(player, arguments, target):
	dyetub = wolfpack.finditem(arguments[0])

	if not dyetub or not player.canreach(dyetub, 1):
		player.socket.clilocmessage(500446)
		return

	if not target.item or not target.item.dye:
		player.socket.clilocmessage(1042083) # You can not dye that.
		return

	if target.item.getoutmostchar() != player:
		player.socket.clilocmessage(500446) # Too far away
		return

	if target.item.container == player:
		player.socket.clilocmessage(500861) # Can't Dye clothing that is being worn.
		return

	target.item.color = dyetub.color
	target.item.update()
	player.log( LOG_MESSAGE, "Dying item (%x,%x) using tub (%x,%x)\n" % ( target.item.serial, target.item.color, dyetub.serial, dyetub.color ) )
	player.soundeffect(0x23e)

#
# Dye an item
#
def onUse(player, item):
	if not player.canreach(item, 1):
		player.socket.clilocmessage(500446)
		return True

	# Show dye target
	player.socket.clilocmessage(500859) # Select the clothing to dye.
	player.socket.attachtarget("specialdye.target", [item.serial])
	return True
