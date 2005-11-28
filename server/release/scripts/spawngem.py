
import wolfpack
import wolfpack.gumps
from wolfpack import tr
from system import spawns

ADVANCEDSPAWNS = 0

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
# Tooltips
#
# Show a nice tooltip
def onShowTooltip(sender, target, tooltip):
	if target.hastag('mininterval') and target.hastag('maxinterval') and target.hastag('area'):
		area = target.gettag('area')
		mininterval = target.gettag('mininterval')
		maxinterval = target.gettag('maxinterval')

		appendix = tr('Radius: %s\nInterval: %s to %s minutes') % (area, mininterval, maxinterval)

		tooltip.add(1060847, "%s\t\n" % appendix)

#
# Gump configuration response
#
def response(player, arguments, response):
	if response.button == 0:
		return

	item = wolfpack.finditem(arguments[0])
	if not item:
		return

	if response.button == 2:
		openadvancedoptions(player, item)
	

	# Validate parameters
	if not 0 in response.switches and not 1 in response.switches:
		player.socket.sysmessage( tr('You have to choose a spawntype.') )
		return

	spawntype = response.switches[0]

	try:
		definition = response.text[0]
		area = int(response.text[1])
		mininterval = int(response.text[2])
		maxinterval = int(response.text[3])
	except:
		player.socket.sysmessage( tr('You entered an invalid value.') )
		return

	item.settag('spawntype', spawntype)
	item.settag('spawndef', definition)
	item.settag('area', area)
	item.settag('mininterval', mininterval)
	item.settag('maxinterval', maxinterval)

	item.resendtooltip()
#
# Show a configuration gump to gms.
#
def onUse(player, item):
	if not player.gm:
		return True

	dialog = wolfpack.gumps.cGump()
	dialog.setCallback(response)
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
	dialog.addText(173, 52, tr("Wolfpack Spawn System"), 194)
	dialog.addResizeGump(65, 112, 9200, 405, 345)
	dialog.addText(173, 72, tr("Spawn Gem Properties"), 2100)
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
	dialog.addText(80, 124, tr("Spawn Type"), 2100)
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
	dialog.addText(80, 184, tr("Item or NPC Definition"), 2100)
	dialog.addResizeGump(80, 208, 9300, 208, 28)
	dialog.addInputField(84, 211, 200, 20, 2100, 0, unicode(spawndef))
	dialog.addText(80, 248, tr("Area or Wander Distance"), 2100)
	dialog.addResizeGump(80, 272, 9300, 75, 28)
	dialog.addInputField(84, 276, 63, 20, 2100, 1, unicode(area))
	dialog.addText(80, 312, tr("Min. Interval in Minutes"), 2100)
	dialog.addResizeGump(80, 336, 9300, 75, 28)
	dialog.addInputField(84, 340, 63, 20, 2100, 2, unicode(mininterval))
	dialog.addText(80, 376, tr("Max. Interval in Minutes"), 2100)
	dialog.addResizeGump(80, 400, 9300, 75, 28)
	dialog.addInputField(84, 404, 63, 20, 2100, 3, unicode(maxinterval))

	if ADVANCEDSPAWNS:
		dialog.addButton(320, 471, 1209, 1210, 2)
		dialog.addText(340, 468, "Advanced Options", 2100)

	dialog.send(player)

	return True

#
# Show advanced options gump to gms.
#
def openadvancedoptions(player, item):
	
	if not player.gm:
		return True

	dialog = wolfpack.gumps.cGump()
	dialog.setCallback(advresponse)
	dialog.setArgs([item.serial])

	if item.hastag('color'):
		spawncolor = unicode(item.gettag('color'))
	else:
		spawncolor = '0'

	if item.hastag('modstr'):
		spawnmodstr = unicode(item.gettag('modstr'))
	else:
		spawnmodstr = 1.0

	if item.hastag('moddex'):
		spawnmoddex = unicode(item.gettag('moddex'))
	else:
		spawnmoddex = 1.0

	if item.hastag('modint'):
		spawnmodint = unicode(item.gettag('modint'))
	else:
		spawnmodint = 1.0

	if item.hastag('name'):
		spawnname = unicode(item.gettag('name'))
	else:
		spawnname = ''

	if item.hastag('nameprefix'):
		spawnprefix = unicode(item.gettag('nameprefix'))
	else:
		spawnprefix = ''

	if item.hastag('namesuffix'):
		spawnsuffix = unicode(item.gettag('namesuffix'))
	else:
		spawnsuffix = ''

	if item.hastag('modmaxhits'):
		spawnmodmaxhits = unicode(item.gettag('modmaxhits'))
	else:
		spawnmodmaxhits = 1.0

	if item.hastag('modmaxstam'):
		spawnmodmaxstam = unicode(item.gettag('modmaxstam'))
	else:
		spawnmodmaxstam = 1.0

	if item.hastag('modmaxmana'):
		spawnmodmaxmana = unicode(item.gettag('modmaxmana'))
	else:
		spawnmodmaxmana = 1.0

	dialog.startPage(0)
	dialog.addResizeGump(35, 11, 9260, 460, 504)
	dialog.addGump(1, 12, 10421, 0)
	dialog.addGump(30, -1, 10420, 0)
	dialog.addResizeGump(66, 40, 9200, 405, 65)
	dialog.addText(173, 52, "Advanced Spawn Options", 194)
	dialog.addResizeGump(65, 112, 9200, 405, 345)
	dialog.addText(173, 72, tr("Spawn Gem Adv. Properties"), 2100)
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
	
	# Now, Adv. Properties

	# Color
	dialog.addText(80, 120, "Color", 2100)
	dialog.addResizeGump(80, 144, 9300, 208, 28)
	dialog.addInputField(84, 147, 200, 20, 2100, 0, unicode(spawncolor))
	# MODs for Stats
	dialog.addText(80, 184, "Mod Str", 2100)
	dialog.addResizeGump(80, 208, 9300, 75, 28)
	dialog.addInputField(84, 211, 200, 20, 2100, 1, unicode(spawnmodstr))
	dialog.addText(180, 184, "Mod Dex", 2100)
	dialog.addResizeGump(180, 208, 9300, 75, 28)
	dialog.addInputField(184, 211, 200, 20, 2100, 2, unicode(spawnmoddex))
	dialog.addText(280, 184, "Mod Int", 2100)
	dialog.addResizeGump(280, 208, 9300, 75, 28)
	dialog.addInputField(284, 211, 200, 20, 2100, 3, unicode(spawnmodint))
	# MaxHits, MaxMana e MaxStam
	dialog.addText(80, 248, "Mod Max Hits", 2100)
	dialog.addResizeGump(80, 272, 9300, 75, 28)
	dialog.addInputField(84, 276, 63, 20, 2100, 4, unicode(spawnmodmaxhits))
	dialog.addText(180, 248, "Mod Max Stamina", 2100)
	dialog.addResizeGump(180, 272, 9300, 75, 28)
	dialog.addInputField(184, 276, 63, 20, 2100, 5, unicode(spawnmodmaxstam))
	dialog.addText(280, 248, "Mod Max Mana", 2100)
	dialog.addResizeGump(280, 272, 9300, 75, 28)
	dialog.addInputField(284, 276, 63, 20, 2100, 6, unicode(spawnmodmaxmana))
	# NameChange, Name.Preffix and Name.Suffix
	dialog.addText(80, 312, "Name", 2100)
	dialog.addResizeGump(80, 336, 9300, 120, 28)
	dialog.addInputField(84, 340, 63, 20, 2100, 7, unicode(spawnname))
	dialog.addText(210, 312, "Prefix", 2100)
	dialog.addResizeGump(210, 336, 9300, 120, 28)
	dialog.addInputField(214, 340, 63, 20, 2100, 8, unicode(spawnprefix))
	dialog.addText(340, 312, "Suffix", 2100)
	dialog.addResizeGump(340, 336, 9300, 120, 28)
	dialog.addInputField(344, 340, 63, 20, 2100, 9, unicode(spawnsuffix))

	dialog.send(player)

	return True

#
# Adv. configuration Gump response
#
def advresponse(player, arguments, response):
	if response.button == 0:
		return

	item = wolfpack.finditem(arguments[0])
	if not item:
		return	

	try:
		color = int(response.text[0])
		modstr = float(response.text[1])
		moddex = float(response.text[2])
		modint = float(response.text[3])
		modmaxhp = float(response.text[4])
		modmaxst = float(response.text[5])
		modmaxmn = float(response.text[6])
		name = response.text[7]
		prefix = response.text[8]
		suffix = response.text[9]
	except:
		player.socket.sysmessage( tr('You entered an invalid value.') )
		return

	if color:
		item.settag('color', color)
	
	item.settag('modstr', modstr)
	item.settag('moddex', moddex)
	item.settag('modint', modint)
	item.settag('modmaxhits', modmaxhp)
	item.settag('modmaxstam', modmaxst)
	item.settag('modmaxmana', modmaxmn)

	if name:
		item.settag('name', name)
	
	if prefix:
		item.settag('nameprefix', prefix)

	if suffix:
		item.settag('namesuffix', suffix)

	item.resendtooltip()