#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts		        #
#  ((    _/{  "-;  | Created by: Correa			        #
#   )).-' {{ ;'`   | Revised by: see svn logs			#
#  ( (  ;._ \\ ctr | Last Modification: see svn logs	        #
#===============================================================#
# Boats module initialization				        #
#===============================================================#

import wolfpack

#
# Global boat registry
#
BOATS = {}

#
# Register a house in the global registry
#
def registerBoat(boat):
	if boat.owner:
		BOATS[boat.owner.serial] = boat.serial

#
# Unregister a house from the global registry
#
def unregisterBoat(boat):
	if boat.owner:
		serial = boat.owner.serial
		if serial in BOATS:
			del BOATS[serial]

#
# Find a house for the given character in the registry
#
def findBoat(player):
	serial = player.serial
	if serial in BOATS:
		return wolfpack.finditem(BOATS[serial])
	else:
		return None
