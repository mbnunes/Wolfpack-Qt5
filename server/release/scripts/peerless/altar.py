#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Antigravity                    #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Peerless Altar Script                                         #
#===============================================================#

import time
import wolfpack
from wolfpack.consts import *
from wolfpack import tr
from peerless.peerless_system import PEERLESS_CONFIG

def onCreate(item, definition):
	item.settag("peerless_type", "dread_horn")
	item.settag("active", "0")
	item.settag("keys_inserted", "")
	item.movable = 0

def onUse(char, item):
	p_type = item.gettag("peerless_type", "dread_horn")
	cfg = PEERLESS_CONFIG.get(p_type, PEERLESS_CONFIG["dread_horn"])
	req_keys = cfg["keys"]

	inserted = []
	if item.hastag("keys_inserted") and str(item.gettag("keys_inserted")).strip():
		inserted = [k.strip() for k in str(item.gettag("keys_inserted")).split(",") if k.strip()]

	char.socket.sysmessage(f"Altar: {cfg['altar_name']}")
	char.socket.sysmessage(f"Keys deposited: {len(inserted)} / {len(req_keys)}")
	for k_code, k_info in req_keys.items():
		status = "[Placed]" if k_code in inserted else "[Missing]"
		char.socket.sysmessage(f" - {k_info['name']}: {status}")
	return True

def onDropOnItem(char, altar, dropped):
	"""
	Players drop keys directly onto the altar to activate it.
	"""
	p_type = altar.gettag("peerless_type", "dread_horn")
	cfg = PEERLESS_CONFIG.get(p_type, PEERLESS_CONFIG["dread_horn"])
	req_keys = cfg["keys"]

	if altar.gettag("active", "0") == "1":
		char.socket.sysmessage("The master of this realm has already been summoned!")
		return False

	if not dropped.hastag("peerless_key"):
		char.socket.sysmessage("This is not a proper key for this altar.")
		return False

	key_code = str(dropped.gettag("peerless_key")).strip()
	if key_code not in req_keys:
		char.socket.sysmessage("This key does not belong to this realm.")
		return False

	inserted = []
	if altar.hastag("keys_inserted") and str(altar.gettag("keys_inserted")).strip():
		inserted = [k.strip() for k in str(altar.gettag("keys_inserted")).split(",") if k.strip()]

	if key_code in inserted:
		char.socket.sysmessage("This key has already been placed in the altar.")
		return False

	# Accept key
	inserted.append(key_code)
	altar.settag("keys_inserted", ",".join(inserted))
	dropped.delete()

	altar.pos.soundeffect(0x29)
	wolfpack.effect(0x3728, altar.pos, 10, 10)
	char.socket.sysmessage(f"You placed the key onto the altar! ({len(inserted)} / {len(req_keys)})")

	# Check if all keys inserted
	if len(inserted) >= len(req_keys):
		activate_peerless(char, altar, cfg)

	return True

def activate_peerless(char, altar, cfg):
	altar.settag("active", "1")
	altar.deltag("keys_inserted")

	# 1. Spawn Boss in Arena
	ax, ay, az, amap = cfg["arena"]
	boss_pos = wolfpack.coord(ax, ay, az, amap)
	boss = wolfpack.addnpc(cfg["boss_id"], boss_pos)

	if boss:
		boss.settag("peerless_altar", altar.serial)
		boss.addscript("peerless.boss")
		altar.settag("boss_serial", boss.serial)

	# 2. Create Master Key and give to player
	mkey = wolfpack.additem("peerless_masterkey")
	if mkey:
		mkey.name = f"{cfg['name']} Master Key"
		mkey.settag("peerless_type", altar.gettag("peerless_type"))
		mkey.settag("altar_serial", altar.serial)
		mkey.color = 0x482

		pack = char.getbackpack()
		if not (pack and wolfpack.utilities.tocontainer(mkey, pack)):
			mkey.moveto(char.pos)

	altar.pos.soundeffect(0x211)
	wolfpack.effect(0x3709, altar.pos, 10, 30)
	char.socket.sysmessage(f"You have awakened {cfg['name']}! The Master Key is now in your backpack. Hurry to enter the lair!")

	# Schedule 90 min deadline
	wolfpack.addtimer(altar, 5400000, "peerless.altar.timeout_timer", [altar.serial])

def timeout_timer(args):
	if not args:
		return
	altar = wolfpack.finditem(args[0])
	if altar and altar.gettag("active") == "1":
		reset_altar(altar)

def reset_altar(altar):
	altar.settag("active", "0")
	altar.deltag("keys_inserted")

	if altar.hastag("boss_serial"):
		try:
			boss = wolfpack.findchar(int(altar.gettag("boss_serial")))
			if boss:
				boss.delete()
		except Exception:
			pass
		altar.deltag("boss_serial")

def on_boss_defeated(altar):
	altar.deltag("boss_serial")
	# 15 minutes grace period before reset
	wolfpack.addtimer(altar, 900000, "peerless.altar.cleanup_timer", [altar.serial])

def cleanup_timer(args):
	if not args:
		return
	altar = wolfpack.finditem(args[0])
	if altar:
		reset_altar(altar)

def onShowTooltip(char, item, tooltip):
	tooltip.reset()
	p_type = item.gettag("peerless_type", "dread_horn")
	cfg = PEERLESS_CONFIG.get(p_type, PEERLESS_CONFIG["dread_horn"])
	active = (item.gettag("active", "0") == "1")

	tooltip.add(1042971, tr(f"{cfg['altar_name']} ({'Awakened' if active else 'Dormant'})"))
	return True
