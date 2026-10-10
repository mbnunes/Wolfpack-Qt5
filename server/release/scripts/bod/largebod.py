#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Antigravity                    #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Large Bulk Order Deed Script                                  #
#===============================================================#

import wolfpack
from wolfpack.consts import *
from wolfpack import tr
from bod.bods_gumps import send_large_bod_gump
from bod.bodsystem import (
	SMITH_MATERIALS, TAILOR_MATERIALS,
	SMITH_SETS, TAILOR_SETS,
	compute_points, give_smith_reward, give_tailor_reward
)

def onCreate(item, definition):
	item.settag("bod_type", "smith")
	item.settag("set_name", "ringmail")
	item.settag("amount_max", 10)
	item.settag("exceptional", "0")
	item.settag("material", 0)
	item.hue = 0x44E

def onUse(char, item):
	if not char.player:
		return True
	if item.getoutmostchar() != char:
		char.socket.clilocmessage(1042001)
		return True

	send_large_bod_gump(char, item)
	return True

def combine_target(char, args, target):
	if not args:
		return False
	large_deed = wolfpack.finditem(args[0])
	if not large_deed or large_deed.getoutmostchar() != char:
		char.socket.clilocmessage(1042001)
		return False

	small_deed = target.item
	if not small_deed or small_deed.getoutmostchar() != char:
		char.socket.clilocmessage(1042001)
		return False

	if not (small_deed.hastag("item_def") and small_deed.hastag("amount_cur")):
		char.socket.sysmessage("You must target a valid small bulk order deed.")
		return False

	s_cur = int(small_deed.gettag("amount_cur", 0))
	s_max = int(small_deed.gettag("amount_max", 10))
	if s_cur < s_max:
		char.socket.sysmessage("The small bulk order deed must be completely filled first.")
		return False

	# Verify compatibility
	l_type = large_deed.gettag("bod_type", "smith")
	s_type = small_deed.gettag("bod_type", "smith")
	if l_type != s_type:
		char.socket.sysmessage("That bulk order deed is for a different profession.")
		return False

	l_max = int(large_deed.gettag("amount_max", 10))
	if s_max != l_max:
		char.socket.sysmessage("The small bulk order deed does not have the same requested amount.")
		return False

	l_exp = (large_deed.gettag("exceptional", "0") == "1")
	s_exp = (small_deed.gettag("exceptional", "0") == "1")
	if l_exp and not s_exp:
		char.socket.clilocmessage(1045155) # The item is not exceptional.
		return False

	l_mat = int(large_deed.gettag("material", 0))
	s_mat = int(small_deed.gettag("material", 0))
	if l_mat != s_mat:
		char.socket.sysmessage("The small bulk order deed is not made from the same material.")
		return False

	set_name = large_deed.gettag("set_name", "")
	sets_dict = SMITH_SETS if l_type == "smith" else TAILOR_SETS
	set_keys = sets_dict.get(set_name, [])

	s_item_def = small_deed.gettag("item_def", "")
	if s_item_def not in set_keys:
		char.socket.sysmessage("That item is not part of this large bulk order set.")
		return False

	line_tag = "line_" + s_item_def
	line_val = int(large_deed.gettag(line_tag, 0))
	if line_val >= l_max:
		char.socket.sysmessage("That part of the order has already been completed.")
		return False

	# Successful combine!
	large_deed.settag(line_tag, l_max)
	small_deed.delete()

	char.pos.soundeffect(0x2A)
	char.socket.clilocmessage(1045152) # The item has been combined with the deed.

	large_deed.resendtooltip()
	send_large_bod_gump(char, large_deed)
	return True

def onDropOnChar(item, char):
	player = item.getoutmostchar()
	if not player or not player.player:
		return False

	bod_type = item.gettag("bod_type", "smith")
	set_name = item.gettag("set_name", "")
	amount_max = int(item.gettag("amount_max", 10))

	# Verify vendor
	npc_name = char.name.lower()
	is_valid = False
	if bod_type == "smith" and ("blacksmith" in npc_name or "armorer" in npc_name or "weaponsmith" in npc_name or char.hasscript("npc.vendor")):
		is_valid = True
	elif bod_type == "tailor" and ("tailor" in npc_name or "weaver" in npc_name or char.hasscript("npc.vendor")):
		is_valid = True

	if not is_valid:
		char.say("I have no use for this order.")
		return False

	# Verify all parts are full
	sets_dict = SMITH_SETS if bod_type == "smith" else TAILOR_SETS
	set_keys = sets_dict.get(set_name, [])

	for key in set_keys:
		if int(item.gettag("line_" + key, 0)) < amount_max:
			char.say("This large bulk order is not yet completely filled.")
			return False

	# Large BOD Reward!
	exceptional = (item.gettag("exceptional", "0") == "1")
	mat_idx = int(item.gettag("material", 0))
	points = compute_points(bod_type, amount_max, exceptional, mat_idx, True, set_name)

	if bod_type == "smith":
		gold, reward = give_smith_reward(player, points)
	else:
		gold, reward = give_tailor_reward(player, points)

	char.say(f"Magnificent craftsmanship! Here is your substantial reward of {gold} gold!")
	player.pos.soundeffect(0x2E6)

	if reward:
		player.socket.sysmessage(f"You received an extraordinary reward: {reward.name}!")

	item.delete()
	return True

def onShowTooltip(char, item, tooltip):
	tooltip.reset()
	bod_type = item.gettag("bod_type", "smith")
	set_name = item.gettag("set_name", "set").replace("_", " ").capitalize()
	amount_max = int(item.gettag("amount_max", 10))
	exceptional = (item.gettag("exceptional", "0") == "1")
	mat_idx = int(item.gettag("material", 0))

	materials = SMITH_MATERIALS if bod_type == "smith" else TAILOR_MATERIALS
	mat_name = materials.get(mat_idx, {}).get("name", "Iron/Cloth")

	sets_dict = SMITH_SETS if bod_type == "smith" else TAILOR_SETS
	set_keys = sets_dict.get(item.gettag("set_name", ""), [])

	done_count = sum(1 for k in set_keys if int(item.gettag("line_" + k, 0)) >= amount_max)
	total_parts = len(set_keys)

	tooltip.add(1042971, tr("A Large Bulk Order Deed: %s (%d/%d parts)" % (set_name, done_count, total_parts)))
	tooltip.add(1042971, tr("Amount per part: %d" % amount_max))
	if exceptional:
		tooltip.add(1042971, tr("Quality: Exceptional"))
	if mat_idx > 0:
		tooltip.add(1042971, tr("Material: %s" % mat_name))
	if done_count >= total_parts:
		tooltip.add(1042971, tr("<basefont color=#00FF00>[Order Complete]</basefont>"))
	return True
