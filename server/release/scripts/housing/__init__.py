
import wolfpack

#
# Global house registry
#
HOUSES = {}

#
# Register a house in the global registry
#
def registerHouse(house):
	if house.owner:
		HOUSES[house.owner.serial] = house.serial

#
# Unregister a house from the global registry
#
def unregisterHouse(house):
	if house.owner:
		serial = house.owner.serial
		if serial in HOUSES:
			del HOUSES[serial]

#
# Find a house for the given character in the registry
#
def findHouse(player):
	serial = player.serial
	if serial in HOUSES:
		return wolfpack.finditem(HOUSES[serial])
	else:
		return None
