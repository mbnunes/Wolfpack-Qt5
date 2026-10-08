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

from treasure.treasure_spawn import treaspickspawn

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

####################################################################################
###################     Picking Items from Container      ##########################
####################################################################################

def onPickupFromContainer(player, item, container):

	# Lets call the chance to summon a new guardian to this Chest
	treaspickspawn( container )

####################################################################################
###################     Context Menu      ##########################################
####################################################################################

def onContextEntry( char, target, tag  ):

	if ( tag == 1 ):
		if target.countitem():
			char.socket.sysmessage('You have to empty the Treasure Chest First!')
		else:
			target.delete()

	return 1