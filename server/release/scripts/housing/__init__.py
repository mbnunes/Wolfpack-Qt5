
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
