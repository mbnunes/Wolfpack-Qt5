
import wolfpack.gumps
import wolfpack
from wolfpack.consts import *
from wolfpack import properties
import magic.runebook

HUES = [
	# Dull Copper
	[ 1018332, 2419, 2420, 2421, 2422, 2423, 2424 ],
	# Shadow Iron
	[ 1018333, 2406, 2407, 2408, 2409, 2410, 2411, 2412 ],
	# Copper
	[ 1018334, 2413, 2414, 2415, 2416, 2417, 2418 ],
	# Bronze
	[ 1018335, 2414, 2415, 2416, 2417, 2418 ],
	# Golden
	[ 1018336, 2213, 2214, 2215, 2216, 2217, 2218 ],
	# Agapite
	[ 1018337, 2425, 2426, 2427, 2428, 2429, 2430 ],
	# Verite
	[ 1018338, 2207, 2208, 2209, 2210, 2211, 2212 ],
	# Valorite
	[ 1018339, 2219, 2220, 2221, 2222, 2223, 2224 ],
	# Reds
	[ 1018340, 2113, 2114, 2115, 2116, 2117, 2118 ],
	# Blues
	[ 1018341, 2119, 2120, 2121, 2122, 2123, 2124 ],
	# Greens
	[ 1018342, 2126, 2127, 2128, 2129, 2130 ],
	# Yellows
	[ 1018343, 2213, 2214, 2215, 2216, 2217, 2218 ],
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
# Runebook Dyetub
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
	dialog.addText(235, 400, "DEFAULT")
	
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
		
	if not target.item or (not magic.runebook.isRunebook(target.item) and not target.item.baseid in ['1f14', '1f15', '1f16', '1f17']):
		player.socket.clilocmessage(1049775) # You can only dye runestones or runebooks with this tub.
		return

	if target.item.getoutmostchar() != player:
		player.socket.clilocmessage(500446) # Too far away
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
	player.socket.clilocmessage(1049774) # Target the runebook or runestone to dye
	player.socket.attachtarget("runedye.target", [item.serial])
	return True
