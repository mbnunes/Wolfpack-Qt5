#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: MagnusBr                       #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

####################################################################################
###################     Imports      ###############################################
####################################################################################

import wolfpack
import random
from wolfpack import tr, properties

####################################################################################
###################     Using      #################################################
####################################################################################

def onUse(player, item):

	# Get Owner of the Treaure Chest and Party Leader
	owner = item.gettag('owner')
	if item.hastag('party'):
		party = item.gettag('party')

	# If you are not the owner or is not in the party of chest so... lets make you criminal!
	if not str(player.serial) == str(owner):
		if not item.hastag('party'):
			player.criminal()
		else:
			if not str(party) == str(player.party.leader.serial):
				player.criminal()