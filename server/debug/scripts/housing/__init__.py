import wolfpack

#
# Global house registry
#
HOUSES = {}

#
# Helper to iterate over all items in the world safely
#
def for_each_world_item():
	try:
		it = wolfpack.itemiterator()
		item = getattr(it, 'first', None)
		while item:
			yield item
			item = getattr(it, 'next', None)
	except Exception:
		return

#
# Register a house in the global registry
#
def registerHouse(house):
	if not house:
		return
	owner = getattr(house, 'owner', None) or getattr(house, 'ownerserial', 0)
	owner_serial = int(getattr(owner, 'serial', owner) or 0)
	if owner_serial:
		HOUSES[owner_serial] = int(house.serial)

#
# Unregister a house from the global registry
#
def unregisterHouse(house):
	if not house:
		return
	owner = getattr(house, 'owner', None) or getattr(house, 'ownerserial', 0)
	owner_serial = int(getattr(owner, 'serial', owner) or 0)
	if owner_serial and owner_serial in HOUSES:
		del HOUSES[owner_serial]

	# Also remove by house serial in case owner changed or was deleted
	h_serial = int(getattr(house, 'serial', 0))
	if h_serial:
		stale_keys = [k for k, v in HOUSES.items() if v == h_serial]
		for k in stale_keys:
			del HOUSES[k]

#
# Rebuild the registry by scanning all multi items in the world
#
def refreshHouses():
	global HOUSES
	HOUSES.clear()
	try:
		for item in for_each_world_item():
			if not item:
				continue
			is_multi = (getattr(item, 'objects', None) is not None) or (getattr(item, 'id', 0) >= 0x4000)
			if is_multi:
				owner_val = getattr(item, 'owner', None) or getattr(item, 'ownerserial', 0)
				owner_serial = int(getattr(owner_val, 'serial', owner_val) or 0)
				if owner_serial:
					registerHouse(item)
	except Exception:
		pass

#
# Find a house for the given character in the registry
#
def findHouse(player):
	if not player:
		return None
	serial = int(getattr(player, 'serial', player) or 0)
	if serial in HOUSES:
		house = wolfpack.finditem(HOUSES[serial])
		if house:
			owner_val = getattr(house, 'owner', None) or getattr(house, 'ownerserial', 0)
			h_owner_serial = int(getattr(owner_val, 'serial', owner_val) or 0)
			if h_owner_serial == serial:
				return house
		HOUSES.pop(serial, None)

	# Direct scan fallback if not cached in registry
	try:
		for item in for_each_world_item():
			if not item:
				continue
			is_multi = (getattr(item, 'objects', None) is not None) or (getattr(item, 'id', 0) >= 0x4000)
			if is_multi:
				owner_val = getattr(item, 'owner', None) or getattr(item, 'ownerserial', 0)
				h_owner_serial = int(getattr(owner_val, 'serial', owner_val) or 0)
				if h_owner_serial == serial:
					registerHouse(item)
					return item
	except Exception:
		pass

	return None

#
# Returns all registered houses (refreshes if empty)
#
def getHouses():
	if not HOUSES:
		refreshHouses()
	return HOUSES

#
# Server startup event hook
#
def onServerStart():
	refreshHouses()