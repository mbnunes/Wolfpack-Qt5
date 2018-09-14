
import wolfpack
import random
import time
from wolfpack import console
from wolfpack.consts import *
import plants.plant

# How often should the plant checks be started
# Default is once every 30 minutes
CHECKINTERVAL = 30 * 60 * 1000

#
# This is mostly a speed setting.
# If there are more than 250 
# plants in the check queue, a new check is made a little later.
#
CHECKSPERCYCLE = 250

#
# If another check cycle needs to be made, 
# this indicates the delay in ms between the checks.
#
CHECKDELAY = 1000

#
# This function registers a new plant
#
def register(plant):
	PLANTS.append(plant.serial)
	
#
# This unregisters a plant
#
def unregister(plant):
	serial = plant.serial
	if serial in PLANTS:
		PLANTS.remove(serial)

#
# This is the list with all known plant serials
#
PLANTS = []

#
# This prevents too many growth checks
#
magic = random.random()

#
# This is the timer callback used to initiate a growth check for 
# all known plants
#
def growthCheck(obj, args):
	if args[0] != magic:
		return # This is an outdated timer

	# Notify the log that we're running a growth check
	console.log(LOG_MESSAGE, "Running plant growth check for %u plants.\n" % len(PLANTS))

	# Copy the list of known plant serials and
	# start the subprocessing function
	processGrowthCheck(None, ( PLANTS[:], 0, magic ))

#
# This function is both, a timer callback and a function that can be normally
# called to process a list of plant serials and check their growth.
# The args are:
# 1. The list or tuple with all plant serials
# 2. The index in that list to start processing at
#
def processGrowthCheck(obj, args):
	if args[2] != magic:
		return # The scripts have been reloaded

	plantlist = args[0] # We keep a reference here to prevent reallocation
	index = args[1]
	upperindex = min(index + CHECKSPERCYCLE, len(plantlist))
	planthash = hash('plant')

	# Process the list of plants
	for i in range(index, upperindex):
		try:
			plant = wolfpack.finditem(plantlist[i])
			if plant and hash(plant.baseid) == planthash:
				plants.plant.growthCheck(plant)
		except Exception, e:
			console.log(LOG_ERROR, "An error occured while checking plant 0x%x: %s.\n" % (plant.serial, str(e)))

	i = upperindex

	# We're not finished yet
	if i < len(PLANTS):
		wolfpack.addtimer(CHECKDELAY, processGrowthCheck, [plantlist, upperindex, magic], False)

	# We're done, so queue the next check
	else:
		wolfpack.addtimer(CHECKINTERVAL, growthCheck, [magic], False)

#
# Add the timer for checking the plants
#
def onLoad():
	global magic
	magic = random.random()

	wolfpack.addtimer(CHECKINTERVAL, growthCheck, [magic], False)

#
# Reset the magic
#
def onUnload():
	global magic
	magic = random.random()

#
# Helper Function
#
def sendCodexOfWisdom(socket, topic, display = True):
	packet = wolfpack.packet(0xbf, 11)
	packet.setshort(1, 11)
	packet.setshort(3, 0x17) # SubCommand
	packet.setbyte(5, 1) # Unknown
	packet.setint(6, int(topic)) # The Topic ID
	packet.setbyte(10, int(display)) # Should the topic be displayed?
	packet.send(socket)
