
import wolfpack
from wolfpack.consts import *
from wolfpack import console
from system import spawns
import wolfpack.gumps

#
# Register the spawngem with the spawning system.
#
def onAttach(object):
	if wolfpack.isstarting():
		object.deltag('nextspawn')

	spawns.register(object)

#
# Detach the spawngem from the spawning system.
#
def onDetach(object):
	spawns.unregister(object)

#
# Gump configuration response
#
def response(player, arguments, response):
	if response.button == 0:
		return

	item = wolfpack.finditem(arguments[0])
	if not item:
		return

	# Validate parameters
	if not 0 in response.switches and not 1 in response.switches:
		player.socket.sysmessage('You have to choose a spawntype.')
		return

	spawntype = response.switches[0]

	try:
		definition = response.text[0]
		area = int(response.text[1])
		mininterval = int(response.text[2])
		maxinterval = int(response.text[3])
	except:
		player.socket.sysmessage('You entered an invalid value.')
		return

	item.settag('spawntype', spawntype)
	item.settag('spawndef', definition)
	item.settag('area', area)
	item.settag('mininterval', mininterval)
	item.settag('maxinterval', maxinterval)

#
# Show a configuration gump to gms.
#
def onUse(player, item):
	if not player.gm:
		return 1

	dialog = wolfpack.gumps.cGump()
	dialog.setCallback("spawngem.response")
	dialog.setArgs([item.serial])

	if item.hastag('spawndef'):
		spawndef = unicode(item.gettag('spawndef'))
	else:
		spawndef = ''

	if item.hastag('spawntype'):
		spawntype = min(1, max(0, int(item.gettag('spawntype'))))
	else:
		spawntype = 0

	if item.hastag('maxinterval'):
		maxinterval = unicode(max(0, item.gettag('maxinterval')))
	else:
		maxinterval = 1

	if item.hastag('mininterval'):
		mininterval = unicode(max(0, item.gettag('mininterval')))
	else:
		mininterval = 1

	if item.hastag('area'):
		area = unicode(max(0, item.gettag('area')))
	else:
		area = 0

	dialog.startPage(0)
	dialog.addResizeGump(35, 11, 9260, 460, 504)
	dialog.addGump(1, 12, 10421, 0)
	dialog.addGump(30, -1, 10420, 0)
	dialog.addResizeGump(66, 40, 9200, 405, 65)
	dialog.addText(173, 52, "Wolfpack Spawn System", 194)
	dialog.addResizeGump(65, 112, 9200, 405, 345)
	dialog.addText(173, 72, "Spawn Gem Properties", 2100)
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
	dialog.addTilePic(114, 50, 9653)
	dialog.addButton(140, 468, 242, 241, 0)
	dialog.addButton(68, 468, 247, 248, 1)
	dialog.addText(80, 124, "Spawn Type", 2100)
	dialog.startGroup(0)
	if spawntype == 0:
		dialog.addRadioButton(80, 148, 9721, 9724, 0, 1)
	else:
		dialog.addRadioButton(80, 148, 9721, 9724, 0, 0)
	dialog.addText(116, 153, "Item", 2100)
	if spawntype == 1:
		dialog.addRadioButton(164, 148, 9721, 9724, 1, 1)
	else:
		dialog.addRadioButton(164, 148, 9721, 9724, 1, 0)
	dialog.addText(202, 153, "NPC", 2100)
	dialog.addText(80, 184, "Item or NPC Definition", 2100)
	dialog.addResizeGump(80, 208, 9300, 208, 28)
	dialog.addInputField(84, 211, 200, 20, 2100, 0, unicode(spawndef))
	dialog.addText(80, 248, "Area or Wander Distance", 2100)
	dialog.addResizeGump(80, 272, 9300, 75, 28)
	dialog.addInputField(84, 276, 63, 20, 2100, 1, unicode(area))
	dialog.addText(80, 312, "Min. Interval in Minutes", 2100)
	dialog.addResizeGump(80, 336, 9300, 75, 28)
	dialog.addInputField(84, 340, 63, 20, 2100, 2, unicode(mininterval))
	dialog.addText(80, 376, "Max. Interval in Minutes", 2100)
	dialog.addResizeGump(80, 400, 9300, 75, 28)
	dialog.addInputField(84, 404, 63, 20, 2100, 3, unicode(maxinterval))
	dialog.send(player)

	return 1
