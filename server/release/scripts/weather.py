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

ENABLEDSEASON = int( wolfpack.settings.getbool( "Season", "Enable Season System", False, True ) )
DAYSTOCHANGESEASON = int( wolfpack.settings.getnumber( "Season", "Days to Change Season", 90, True ) )
ENABLEDESOLATIONASSEASON = int( wolfpack.settings.getbool( "Season", "Enable Desolation as Season", False, True ) )

######################################################################################
#############   Season Table   #######################################################
######################################################################################
# Rain Chance MOD, Snow Chance MOD, Intensity MOD, Persistance MOD
# Note: I make this table experiencing Rain and Snow in Brazil (Rain) and Germany (Snow). So... try to make always your own table hehehe :P

SEASONS = {
	0: [0, 5, 0, 0],	# Spring
	1: [10, 0, 30, -2],	# Summer
	2: [5, 5, 0, 0],	# Fall
	3: [0, 10, 0, 0],	# Winter
	4: [-10, -10, 0, 0]		# Desolation
}

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

	# Season modifications
	season = returnseason()

	# Ok. Now, lets check if this Region have to begins to Rain or Snow
	rainchance = region.rainchance + SEASONS[season][0]
	snowchance = region.snowchance + SEASONS[season][1]
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
		weatherintensity( region )

	# Snow
	elif randomsnow <= snowchance:
		region.startsnow()	
		console.log(LOG_MESSAGE, "Snowing on " + region.name )

		weatherduration( region )
		weatherintensity( region )

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
		if unicode(topregion) == unicode(region):
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

	# Season Modifications
	season = returnseason()
	duration += SEASONS[season][3]
	
	duration = random.randint( duration - range, duration + range)

	if duration < 1:
		duration = 1

	####################
	# Setting
	####################

	actualhour = wolfpack.time.hour()

	day = wolfpack.time.days() + (actualhour + duration)/24
	hour = (actualhour + duration)%24

	# Set Next Weather update
	region.setweatherhour( hour )
	region.setweatherday( day )

######################################################################################
#############   Set Intensity of new Weather   #######################################
######################################################################################

def weatherintensity( region ):

	intensity = random.randint( region.minintensity, region.maxintensity )

	# Season Modifications
	season = returnseason()
	intensity += SEASONS[season][2]

	region.setweatherintensity( intensity )

######################################################################################
#############   Return Season   ######################################################
######################################################################################

def returnseason():

	if ENABLEDSEASON:

		seasoncicles = wolfpack.time.days() / DAYSTOCHANGESEASON

		if ENABLEDESOLATIONASSEASON:
			seasonnumber = seasoncicles % 5
	
		else:
			seasonnumber = seasoncicles % 4

	
		return seasonnumber

	else:
		return 0