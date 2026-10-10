#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Antigravity                    #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Small Bulk Order Deed Script                                  #
#===============================================================#

import wolfpack
from wolfpack.consts import *
from wolfpack import tr
from bod.bods_gumps import send_small_bod_gump
from bod.bodsystem import (
	SMITH_MATERIALS, TAILOR_MATERIALS,
	SMITH_ITEMS, TAILOR_ITEMS,
	compute_points, give_smith_reward, give_tailor_reward
)

def onCreate(item, definition):
	item.settag("bod_type", "smith")
	item.settag("item_def", "broadsword")
	item.settag("item_name", "broadsword")
	item.settag("amount_max", 10)
	item.settag("amount_cur", 0)
	item.settag("exceptional", "0")
	item.settag("material", 0)
	item.hue = 0x44E

def onUse(char, item):
	if not char.player:
		return True
	if item.getoutmostchar() != char:
		char.socket.clilocmessage(1042001) # That must be in your pack for you to use it.
		return True

	send_small_bod_gump(char, item)
	return True

def combine_target(char, args, target):
	if not args:
		return False
	deed = wolfpack.finditem(args[0])
	if not deed or deed.getoutmostchar() != char:
		char.socket.clilocmessage(1042001)
		return False

	target_item = target.item
	if not target_item:
		char.socket.sysmessage("You must target an item.")
		return False

	if target_item.getoutmostchar() != char:
		char.socket.clilocmessage(1042001)
		return False

	amount_max = int(deed.gettag("amount_max", 10))
	amount_cur = int(deed.gettag("amount_cur", 0))
	if amount_cur >= amount_max:
		char.socket.sysmessage("This bulk order is already full.")
		return False

	bod_type = deed.gettag("bod_type", "smith")
	req_def = deed.gettag("item_def", "")
	req_exceptional = (deed.gettag("exceptional", "0") == "1")
	req_mat_idx = int(deed.gettag("material", 0))

	# 1. Check item definition matching
	t_def = target_item.baseid.lower()
	if t_def != req_def.lower() and not t_def.endswith(req_def.lower()):
		char.socket.clilocmessage(1045155) # The item is not of the requested type.
		return False

	# 2. Check exceptional requirement
	if req_exceptional and not target_item.hastag("exceptional"):
		char.socket.clilocmessage(1045155) # The item is not exceptional.
		return False

	# 3. Check material requirement
	materials = SMITH_MATERIALS if bod_type == "smith" else TAILOR_MATERIALS
	expected_res = materials.get(req_mat_idx, {}).get("resname", "")

	if req_mat_idx > 0:
		item_res = ""
		if target_item.hastag("resname"):
			item_res = str(target_item.gettag("resname")).lower()
		elif target_item.hasstrproperty("resname"):
			item_res = str(target_item.getstrproperty("resname")).lower()

		if item_res != expected_res.lower():
			char.socket.sysmessage("The item is not made from the requested material (%s)." % materials[req_mat_idx]["name"])
			return False

	# Valid match! Consume item and increment
	if target_item.amount > 1:
		target_item.amount -= 1
	else:
		target_item.delete()

	amount_cur += 1
	deed.settag("amount_cur", amount_cur)
	char.pos.soundeffect(0x2A) # Success sound
	char.socket.clilocmessage(1045152) # The item has been combined with the deed.

	deed.resendtooltip()
	send_small_bod_gump(char, deed)
	return True

def onDropOnChar(item, char):
	# The player is dropping this BOD onto an NPC
	player = item.getoutmostchar()
	if not player or not player.player:
		return False

	amount_max = int(item.gettag("amount_max", 10))
	amount_cur = int(item.gettag("amount_cur", 0))
	bod_type = item.gettag("bod_type", "smith")

	# Check if vendor matches
	npc_name = char.name.lower()
	is_valid_vendor = False
	if bod_type == "smith" and ("blacksmith" in npc_name or "armorer" in npc_name or "weaponsmith" in npc_name or char.hasscript("npc.vendor")):
		is_valid_vendor = True
	elif bod_type == "tailor" and ("tailor" in npc_name or "weaver" in npc_name or char.hasscript("npc.vendor")):
		is_valid_vendor = True

	if not is_valid_vendor:
		char.say("I have no use for this order.")
		return False

	if amount_cur < amount_max:
		char.say("This bulk order is not yet complete.")
		return False

	# Reward calculation
	exceptional = (item.gettag("exceptional", "0") == "1")
	mat_idx = int(item.gettag("material", 0))
	points = compute_points(bod_type, amount_max, exceptional, mat_idx, False)

	if bod_type == "smith":
		gold, reward = give_smith_reward(player, points)
	else:
		gold, reward = give_tailor_reward(player, points)

	char.say(f"Thank you for completing this order, {player.name}! Here is your reward of {gold} gold.")
	player.pos.soundeffect(0x2E6)

	if reward:
		player.socket.sysmessage(f"You received an additional reward: {reward.name}!")

	item.delete()
	return True

def onShowTooltip(char, item, tooltip):
	tooltip.reset()
	bod_type = item.gettag("bod_type", "smith")
	item_name = item.gettag("item_name", "item").capitalize()
	amount_max = int(item.gettag("amount_max", 10))
	amount_cur = int(item.gettag("amount_cur", 0))
	exceptional = (item.gettag("exceptional", "0") == "1")
	mat_idx = int(item.gettag("material", 0))

	materials = SMITH_MATERIALS if bod_type == "smith" else TAILOR_MATERIALS
	mat_name = materials.get(mat_idx, {}).get("name", "Iron/Cloth")

	tooltip.add(1042971, tr("A Bulk Order Deed: %s (%d/%d)" % (item_name, amount_cur, amount_max)))
	if exceptional:
		tooltip.add(1042971, tr("Quality: Exceptional"))
	if mat_idx > 0:
		tooltip.add(1042971, tr("Material: %s" % mat_name))
	if amount_cur >= amount_max:
		tooltip.add(1042971, tr("<basefont color=#00FF00>[Order Complete]</basefont>"))
	return True
