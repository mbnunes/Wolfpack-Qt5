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

from wolfpack import console
from wolfpack.consts import LOG_MESSAGE, EVENT_SERVERHOUR

######################################################################################
#############   Initializing Global Event   ##########################################
######################################################################################

def onLoad():
	wolfpack.registerglobal(EVENT_SERVERHOUR, "weather")

######################################################################################
#############   The Event   ##########################################################
######################################################################################

def onServerHour():

	# Starting a Loop between sockets to check region things for weather
	
	worldsocket = wolfpack.sockets.first()
	while worldsocket:

		# Lets try to find the region or parent
		region = worldsocket.player.region

		if region.parent:
			region = region.parent

		# Ok. Now, lets check if this Region have to begins to Rain or Snow
		rainchance = region.rainchance
		snowchance = region.snowchance
		randomrain = random.randint(1, 100)
		randomsnow = random.randint(1, 100)

		if randomrain <= rainchance:
			region.startrain()
			console.log(LOG_MESSAGE, "Raining in " + region.name )

		if randomsnow <= snowchance:
			region.startsnow()	
			console.log(LOG_MESSAGE, "Snowing in " + region.name )
	
		
		# So... next socket!
		worldsocket = wolfpack.sockets.next()