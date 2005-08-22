import wolfpack
import random
import time
from wolfpack import console
from wolfpack.consts import *
import crops
from wolfpack import tr

# How often should the plant checks be started
# Default is once every 30 minutes
CHECKINTERVAL = 30 * 60 * 1000

#
# This is mostly a speed setting.
# If there are more than 250 
# crops in the check queue, a new check is made a little later.
#
CHECKSPERCYCLE = 250

#
# If another check cycle needs to be made, 
# this indicates the delay in ms between the checks.
#
CHECKDELAY = 1000

#
# This function registers a new crop
#
def register(crop):
	CROPS.append(crop.serial)

#
# This unregisters a crop
#
def unregister(crop):
	serial = crop.serial
	if serial in CROPS:
		CROPS.remove(serial)

#
# This is the list with all known crop serials
#
CROPS = []

#
# This prevents too many growth checks
#
magic = random.random()

stages = crops.stages

#
# This is the timer callback used to initiate a growth check for 
# all known crops
#
def growthCheck(obj, args):
	if args[0] != magic:
		return # This is an outdated timer

	# Notify the log that we're running a growth check
	console.log(LOG_MESSAGE, "Running crop growth check for %u crops.\n" % len(CROPS))

	# Copy the list of known plant serials and
	# start the subprocessing function
	processGrowthCheck(None, ( CROPS[:], 0, magic ))
	
#
# This function is both, a timer callback and a function that can be normally
# called to process a list of crop serials and check their growth.
# The args are:
# 1. The list or tuple with all crop serials
# 2. The index in that list to start processing at
#
def processGrowthCheck(obj, args):
	if args[2] != magic:
		return # The scripts have been reloaded

	croplist = args[0] # We keep a reference here to prevent reallocation
	index = args[1]
	upperindex = min(index + CHECKSPERCYCLE, len(croplist))

	# Process the list of plants
	for i in range(index, upperindex):
		try:
			crop = wolfpack.finditem(croplist[i])
			if crop and crop.baseid in stages.keys():
				growCrop(crop)
			else:
				console.log(LOG_ERROR, "An error occured while checking crop 0x%x: Baseid is not in Stages list.\n" % crop.serial)
				crop.removescript("crops.growing")
				unregister(crop)
		except Exception, e:
			console.log(LOG_ERROR, "An error occured while checking crop 0x%x: %s.\n" % (crop.serial, str(e)))

	i = upperindex

	# We're not finished yet
	if i < len(CROPS):
		wolfpack.addtimer(CHECKDELAY, processGrowthCheck, [croplist, upperindex, magic], False)

	# We're done, so queue the next check
	else:
		wolfpack.addtimer(CHECKINTERVAL, growthCheck, [magic], False)

#
# Grow the crop
#
def growCrop(crop):
	base = crop.baseid

	if not crop.hastag( "stage" ):
		crop.settag( "stage", 0 )
	else:
		crop.settag( "stage", crop.gettag("stage")+1)
	stage = crop.gettag( "stage" )

	# Set the new id
	new_id = random.choice(stages[base][stage])
	crop.id = new_id
	crop.update()

	# if it's ripe, remove it from growth check
	if len(stages[base][:-4]) == stage:
		unregister(crop)
		crop.removescript("crops.growing")
		crop.addscript("crops.harvest")
		crop.deltag( "stage" )

def onUse(char, crop):
	char.socket.sysmessage( tr("That is not ripe yet.") )
	return True

#
# Add the timer for checking the crops
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
# Register the crop with the global registry
#
def onAttach(obj):
	register(obj)

#
# Unregister the crop with the global registry
#
def onDetach(obj):
	unregister(obj)
