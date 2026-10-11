#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Antigravity                    #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Bulk Order Gumps (Small & Large BODs)                         #
#===============================================================#

import wolfpack
import wolfpack.gumps
from wolfpack import tr
from bod.bodsystem import (
	SMITH_MATERIALS, TAILOR_MATERIALS,
	SMITH_ITEMS, TAILOR_ITEMS,
	SMITH_SETS, TAILOR_SETS
)

#
# Small BOD Gump
#
def send_small_bod_gump(player, deed):
	bod_type = deed.gettag("bod_type", "smith")
	item_def = deed.gettag("item_def", "dagger")
	item_name = deed.gettag("item_name", item_def.replace("_", " ").capitalize())
	graphic = int(deed.gettag("graphic", 0xF52))
	amount_max = int(deed.gettag("amount_max", 10))
	amount_cur = int(deed.gettag("amount_cur", 0))
	exceptional = (deed.gettag("exceptional", "0") == "1")
	mat_idx = int(deed.gettag("material", 0))

	materials = SMITH_MATERIALS if bod_type == "smith" else TAILOR_MATERIALS
	mat_name = materials.get(mat_idx, {}).get("name", "Iron/Cloth")
	mat_color = materials.get(mat_idx, {}).get("color", 0)

	gump = wolfpack.gumps.cGump()
	gump.setCallback(small_bod_gump_callback)
	gump.setArgs([deed.serial])

	# Layout sizes
	extra_h = 0
	if exceptional:
		extra_h += 24
	if mat_idx > 0:
		extra_h += 24

	gump.addResizeGump(50, 10, 5054, 455, 245 + extra_h)
	gump.addTiledGump(58, 20, 438, 226 + extra_h, 2624)
	gump.addCheckerTrans(58, 20, 438, 226 + extra_h)

	# Corners
	gump.addImage(45, 5, 10460)
	gump.addImage(480, 5, 10460)
	gump.addImage(45, 230 + extra_h, 10460)
	gump.addImage(480, 230 + extra_h, 10460)

	# Title: A bulk order
	gump.addXmfHtmlGump(225, 25, 120, 20, 1045133, 0, 0, 0xFFFFFF)

	# Amount to make:
	gump.addXmfHtmlGump(75, 48, 200, 20, 1045138, 0, 0, 0xFFFFFF)
	gump.addHtmlGump(275, 48, 100, 20, tr("<basefont color=#FFFFFF>%d</basefont>" % amount_max), 0, 0)

	# Amount finished:
	gump.addXmfHtmlGump(275, 76, 150, 20, 1045153, 0, 0, 0xFFFFFF)
	gump.addHtmlGump(275, 96, 100, 20, tr("<basefont color=#00FF00>%d</basefont>" % amount_cur), 0, 0)

	# Item requested:
	gump.addXmfHtmlGump(75, 72, 150, 20, 1045136, 0, 0, 0xFFFFFF)
	gump.addHtmlGump(75, 96, 190, 20, tr("<basefont color=#FFFF00>%s</basefont>" % item_name.capitalize()), 0, 0)

	# Item Graphic display
	gump.addItem(410, 72, graphic, mat_color)

	y = 120
	if exceptional or mat_idx > 0:
		gump.addXmfHtmlGump(75, y, 200, 20, 1045140, 0, 0, 0xFFFFFF) # Special requirements to meet:
		y += 24

	if exceptional:
		gump.addXmfHtmlGump(75, y, 300, 20, 1045141, 0, 0, 0xFFFFFF) # All items must be exceptional.
		y += 24

	if mat_idx > 0:
		gump.addHtmlGump(75, y, 350, 20, tr("<basefont color=#FFFFFF>All items must be made with %s.</basefont>" % mat_name), 0, 0)
		y += 24

	# Combine Button: Combine this deed with the item requested
	if amount_cur < amount_max:
		gump.addButton(125, y, 4005, 4007, 2)
		gump.addXmfHtmlGump(160, y, 300, 20, 1045154, 0, 0, 0xFFFFFF) # Combine this deed with the item requested.
	else:
		gump.addHtmlGump(125, y, 300, 20, tr("<basefont color=#00FF00><b>This bulk order is complete! Give it to a vendor.</b></basefont>"), 0, 0)

	# Exit button
	gump.addButton(430, 210 + extra_h, 4017, 4019, 0)

	gump.send(player.socket)

def small_bod_gump_callback(char, args, response):
	if response.button != 2 or not args:
		return False

	deed = wolfpack.finditem(args[0])
	if not deed or deed.getoutmostchar() != char:
		char.socket.clilocmessage(1042001) # That must be in your pack for you to use it.
		return False

	amount_max = int(deed.gettag("amount_max", 10))
	amount_cur = int(deed.gettag("amount_cur", 0))
	if amount_cur >= amount_max:
		char.socket.sysmessage("This bulk order is already finished.")
		return False

	# Attach target to combine item
	char.socket.sysmessage("Target the item you wish to combine with this bulk order:")
	char.socket.attachtarget("bod.smallbod.combine_target", [deed.serial])
	return True

#
# Large BOD Gump
#
def send_large_bod_gump(player, deed):
	bod_type = deed.gettag("bod_type", "smith")
	set_name = deed.gettag("set_name", "ringmail")
	amount_max = int(deed.gettag("amount_max", 10))
	exceptional = (deed.gettag("exceptional", "0") == "1")
	mat_idx = int(deed.gettag("material", 0))

	materials = SMITH_MATERIALS if bod_type == "smith" else TAILOR_MATERIALS
	mat_name = materials.get(mat_idx, {}).get("name", "Iron/Cloth")
	mat_color = materials.get(mat_idx, {}).get("color", 0)

	sets_dict = SMITH_SETS if bod_type == "smith" else TAILOR_SETS
	items_registry = SMITH_ITEMS if bod_type == "smith" else TAILOR_ITEMS
	item_keys = sets_dict.get(set_name, [])

	total_lines = len(item_keys)
	height = 200 + (total_lines * 32)
	if exceptional or mat_idx > 0:
		height += 48

	gump = wolfpack.gumps.cGump()
	gump.setCallback(large_bod_gump_callback)
	gump.setArgs([deed.serial])

	gump.addResizeGump(50, 10, 5054, 455, height)
	gump.addTiledGump(58, 20, 438, height - 20, 2624)
	gump.addCheckerTrans(58, 20, 438, height - 20)

	# Corners
	gump.addImage(45, 5, 10460)
	gump.addImage(480, 5, 10460)
	gump.addImage(45, height - 15, 10460)
	gump.addImage(480, height - 15, 10460)

	# Title: A large bulk order
	gump.addXmfHtmlGump(215, 25, 150, 20, 1045151, 0, 0, 0xFFFFFF) # A large bulk order
	gump.addXmfHtmlGump(75, 48, 200, 20, 1045138, 0, 0, 0xFFFFFF) # Amount to make:
	gump.addHtmlGump(275, 48, 100, 20, tr("<basefont color=#FFFFFF>%d</basefont>" % amount_max), 0, 0)

	# Items list
	y = 75
	is_all_complete = True
	for key in item_keys:
		item_info = items_registry.get(key, {"name": key, "graphic": 0xF52})
		line_cur = int(deed.gettag("line_" + key, 0))
		if line_cur < amount_max:
			is_all_complete = False

		gump.addHtmlGump(75, y, 180, 20, tr("<basefont color=#FFFF00>%s</basefont>" % item_info["name"].capitalize()), 0, 0)
		gump.addHtmlGump(275, y, 80, 20, tr("<basefont color=#00FF00>%d / %d</basefont>" % (line_cur, amount_max)), 0, 0)
		gump.addItem(380, y - 5, item_info["graphic"], mat_color)
		y += 32

	if exceptional or mat_idx > 0:
		y += 10
		gump.addXmfHtmlGump(75, y, 200, 20, 1045140, 0, 0, 0xFFFFFF) # Special requirements
		y += 20
		if exceptional:
			gump.addXmfHtmlGump(75, y, 300, 20, 1045141, 0, 0, 0xFFFFFF) # Exceptional
			y += 20
		if mat_idx > 0:
			gump.addHtmlGump(75, y, 350, 20, tr("<basefont color=#FFFFFF>All items must be made with %s.</basefont>" % mat_name), 0, 0)
			y += 20

	y += 10
	if not is_all_complete:
		gump.addButton(125, y, 4005, 4007, 2)
		gump.addXmfHtmlGump(160, y, 300, 20, 1045154, 0, 0, 0xFFFFFF) # Combine this deed with the item requested.
	else:
		gump.addHtmlGump(125, y, 300, 20, tr("<basefont color=#00FF00><b>This large bulk order is complete! Give it to a vendor.</b></basefont>"), 0, 0)

	# Exit
	gump.addButton(430, height - 35, 4017, 4019, 0)

	gump.send(player.socket)

def large_bod_gump_callback(char, args, response):
	if response.button != 2 or not args:
		return False

	deed = wolfpack.finditem(args[0])
	if not deed or deed.getoutmostchar() != char:
		char.socket.clilocmessage(1042001)
		return False

	char.socket.sysmessage("Target the small bulk order deed you wish to combine into this large order:")
	char.socket.attachtarget("bod.largebod.combine_target", [deed.serial])
	return True
