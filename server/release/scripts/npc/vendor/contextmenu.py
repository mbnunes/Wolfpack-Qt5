#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Correa                         #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification:                         #
#################################################################

from npc.vendor import getClassFromVendor

def onContextEntry(player, vendor, tag):
	vclass = getClassFromVendor(vendor)

	if not vclass:
		return False

	if not vclass.checkAccess(vendor, player):
		return False
	
	if tag == 1:
		vclass.onBuy(vendor, player)

	elif tag == 2:
		vclass.onSell(vendor, player)

	return True

def onContextCheckVisible(player, vendor, tag):
	vclass = getClassFromVendor(vendor)

	if not vclass:		
		return False

	return True
