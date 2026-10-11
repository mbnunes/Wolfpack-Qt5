#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Antigravity                    #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Peerless Master Key Script                                    #
#===============================================================#

import wolfpack
import wolfpack.gumps
from wolfpack.consts import *
from wolfpack import tr
from peerless.peerless_system import PEERLESS_CONFIG

def onUse(char, item):
	if not char.player:
		return True
	if item.getoutmostchar() != char:
		char.socket.clilocmessage(1042001)
		return True

	p_type = item.gettag("peerless_type", "dread_horn")
	cfg = PEERLESS_CONFIG.get(p_type, PEERLESS_CONFIG["dread_horn"])

	gump = wolfpack.gumps.cGump()
	gump.setCallback(teleport_callback)
	gump.setArgs([item.serial])

	gump.addResizeGump(50, 50, 5054, 340, 200)
	gump.addTiledGump(58, 58, 324, 184, 2624)
	gump.addCheckerTrans(58, 58, 324, 184)

	title = tr(f"<basefont color=#FFFF00><center>Enter {cfg['name']}'s Lair</center></basefont>")
	gump.addHtmlGump(60, 65, 320, 25, title, 0, 0)

	desc = tr("<basefont color=#FFFFFF>Using this Master Key will transport you and nearby allies directly into the boss chamber. Are you prepared to face the beast?</basefont>")
	gump.addHtmlGump(70, 95, 300, 60, desc, 0, 0)

	# Buttons
	gump.addButton(90, 165, 4005, 4007, 1)
	gump.addHtmlGump(125, 165, 80, 20, tr("<basefont color=#00FF00>Enter</basefont>"), 0, 0)

	gump.addButton(210, 165, 4005, 4007, 0)
	gump.addHtmlGump(245, 165, 80, 20, tr("<basefont color=#FF0000>Cancel</basefont>"), 0, 0)

	gump.send(char.socket)
	return True

def teleport_callback(char, args, response):
	if response.button != 1 or not args:
		return False

	key = wolfpack.finditem(args[0])
	if not key or key.getoutmostchar() != char:
		return False

	p_type = key.gettag("peerless_type", "dread_horn")
	cfg = PEERLESS_CONFIG.get(p_type, PEERLESS_CONFIG["dread_horn"])
	ax, ay, az, amap = cfg["arena"]

	dest = wolfpack.coord(ax, ay, az, amap)

	# Teleport player
	char.removefromview()
	char.moveto(dest)
	char.update()
	char.pos.soundeffect(0x1FE)
	wolfpack.effect(0x3728, char.pos, 10, 10)

	# Teleport nearby party/companions within 4 tiles
	for nearby in wolfpack.chars(char.pos.x, char.pos.y, char.pos.map, 4):
		if nearby and nearby.player and nearby != char:
			nearby.removefromview()
			nearby.moveto(dest)
			nearby.update()
			nearby.pos.soundeffect(0x1FE)

	char.socket.sysmessage("You step through the dimensional rift into the boss lair!")
	return True

def onShowTooltip(char, item, tooltip):
	tooltip.reset()
	p_type = item.gettag("peerless_type", "dread_horn")
	cfg = PEERLESS_CONFIG.get(p_type, PEERLESS_CONFIG["dread_horn"])
	tooltip.add(1042971, tr(f"{cfg['name']} Master Key"))
	tooltip.add(1042971, tr("<basefont color=#00FF00>Double click to enter the lair</basefont>"))
	return True
