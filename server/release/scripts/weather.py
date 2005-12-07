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
import wolfpack.time

from wolfpack import console
from wolfpack.consts import LOG_MESSAGE, EVENT_SERVERHOUR

######################################################################################
#############   Constants   ##########################################################
######################################################################################

ENABLEDWEATHER = int( wolfpack.settings.getbool("Weather", "Enable Weather System", False, True) )

######################################################################################
#############   Initializing Global Event   ##########################################
######################################################################################

def onLoad():
	wolfpack.registerglobal(EVENT_SERVERHOUR, "weather")

######################################################################################
#############   The Event   ##########################################################
######################################################################################

def onServerHour():

	if ENABLEDWEATHER:

		# Starting a Loop between sockets to check region things for weather
	
		worldsocket = wolfpack.sockets.first()
		while worldsocket:

			# Lets try to find the region or parent
			region = worldsocket.player.region

			if region.parent:
				region = region.parent

			# Lets see if this region is in the time to change weather or not, right?
			actualday = wolfpack.time.days()
			changeday = region.weatherday

			if actualday > changeday:
				changeweather( region )

			elif actualday == changeday:
				actualhour = wolfpack.time.hour()
				changehour = region.weatherhour

				if actualhour >= changehour:
					changeweather( region )

			# So... next socket!
			worldsocket = wolfpack.sockets.next()

######################################################################################
#############   Changing Weather   ###################################################
######################################################################################

def changeweather( region ):

	# Ok. Now, lets check if this Region have to begins to Rain or Snow
	rainchance = region.rainchance
	snowchance = region.snowchance
	randomrain = random.randint(1, 100)
	randomsnow = random.randint(1, 100)

	# Rain (And possible Snow too)
	if randomrain <= rainchance:
		region.startrain()

		if randomsnow <= snowchance:
			region.startsnow()
			console.log(LOG_MESSAGE, "Raining and Snowing on " + region.name )

		else:
			console.log(LOG_MESSAGE, "Raining on " + region.name )

		weatherduration( region, 2 )

	# Snow
	elif randomsnow <= snowchance:
		region.startsnow()	
		console.log(LOG_MESSAGE, "Snowing on " + region.name )

		weatherduration( region, 2 )

	# Dry
	else:
		if region.israining:
			region.stoprain()
		if region.issnowing:
			region.stopsnow()

		weatherduration( region, 2 )

######################################################################################
#############   Set persistance of new Weather   #####################################
######################################################################################

def weatherduration( region, hours ):

	actualhour = wolfpack.time.hour()

	day = wolfpack.time.days() + (actualhour + hours)/24
	hour = (actualhour + hours)%24

	# Set Next Weather update
	region.setweatherhour( hour )
	region.setweatherday( day )