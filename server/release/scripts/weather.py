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
	if not ENABLEDWEATHER:
		return

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

		weatherduration( region )

	# Snow
	elif randomsnow <= snowchance:
		region.startsnow()	
		console.log(LOG_MESSAGE, "Snowing on " + region.name )

		weatherduration( region )

	# Dry
	else:
		if region.israining:
			region.stoprain()
		if region.issnowing:
			region.stopsnow()

		weatherduration( region )

	# Updating all Clients in this Region (I think socket iterator is cheaper than regionchars iterator)
	worldsocket = wolfpack.sockets.first()
	while worldsocket:
		# Finding TopRegion
		topregion = worldsocket.player.region
		if topregion.parent:
				topregion = topregion.parent
		# Checking Region
		if str(topregion) == str(region):
			worldsocket.player.update()

		worldsocket = wolfpack.sockets.next()

######################################################################################
#############   Set persistance of new Weather   #####################################
######################################################################################

def weatherduration( region ):
	####################
	# Duration
	####################
	if region.israining:
		if region.issnowing:
			duration =  random.randint(region.rainduration, region.snowduration)
			range = random.randint(region.rainrangeduration, region.snowrangeduration)
		else:
			duration = region.rainduration
			range = region.rainrangeduration
	elif region.issnowing:
		duration = region.snowduration
		range = region.snowrangeduration
	else:
		duration = region.dryduration
		range = region.dryrangeduration

	duration = random.randint( duration - range, duration + range)

	####################
	# Setting
	####################

	actualhour = wolfpack.time.hour()

	day = wolfpack.time.days() + (actualhour + duration)/24
	hour = (actualhour + duration)%24

	# Set Next Weather update
	region.setweatherhour( hour )
	region.setweatherday( day )
