import wolfpack
import random
import time
from wolfpack import console
from wolfpack.consts import *
import crops
from wolfpack import tr
import wolfpack.utilities
import beverage

# How often should the object checks be started
# Default is once every 30 minutes
CHECKINTERVAL = 30 * 60 * 1000

#
# This is mostly a speed setting.
# If there are more than 250 
# objects in the check queue, a new check is made a little later.
#
CHECKSPERCYCLE = 250

#
# If another check cycle needs to be made, 
# this indicates the delay in ms between the checks.
#
CHECKDELAY = 1000

#
# This function registers a new object
#
def register(object):
	OBJECTS.append(object.serial)

#
# This unregisters an object
#
def unregister(object):
	serial = object.serial
	if serial in OBJECTS:
		OBJECTS.remove(serial)

#
# This is the list with all known object serials
#
OBJECTS = []

#
# This prevents too many growth checks
#
magic = random.random()

#
# This is the timer callback used to initiate a growth check for 
# all known objects
#
def gainresObject(obj, args):
	if args[0] != magic:
		return # This is an outdated timer

	# Notify the log that we're running a growth check
	console.log(LOG_MESSAGE, "Running farming growth check for %u objects.\n" % len(OBJECTS))

	# Copy the list of known object serials and
	# start the subprocessing function
	processGainresObject(None, ( OBJECTS[:], 0, magic ))
	
#
# This function is both, a timer callback and a function that can be normally
# called to process a list of object serials and check their growth.
# The args are:
# 1. The list or tuple with all object serials
# 2. The index in that list to start processing at
#
def processGainresObject(obj, args):
	if args[2] != magic:
		return # The scripts have been reloaded

	objectlist = args[0] # We keep a reference here to prevent reallocation
	index = args[1]
	upperindex = min(index + CHECKSPERCYCLE, len(objectlist))

	# Process the list of objects
	for i in range(index, upperindex):
		try:
			object = wolfpack.findobject(objectlist[i])
			if object:
				if object.hastag("resourcecount") and object.gettag("resourcecount") >= actions[ object.baseid ][1]:
					continue
				else:
					addresource(object)
		except Exception, e:
			console.log(LOG_ERROR, "An error occured while checking farming object 0x%x: %s.\n" % (object.serial, str(e)))

	i = upperindex

	# We're not finished yet
	if i < len(OBJECTS):
		wolfpack.addtimer(CHECKDELAY, processGainresObject, [objectlist, upperindex, magic], False)

	# We're done, so queue the next check
	else:
		wolfpack.addtimer(CHECKINTERVAL, gainresObject, [magic], False)

#
# + 1 to resourcecount of object
#
def addresource(object):
	if object.hastag("resourcecount"):
		rescount = object.gettag("resourcecount")
		object.settag("resourcecount", rescount + 1)
	else:
		object.settag("resourcecount", 1)
	return True

# If we double click on an item (tree e.g.)
def onUse(player, item):
	if not actions.has_key( item.baseid ):
		return False
	if not player.canreach(item, 2):
		player.socket.clilocmessage(500295) # You are too far away to do that.
		return False
	# Look if there's something to get
	if not checkresource(item, player):
		return False

	return actions[ item.baseid ]( player, item )

def cow(char, player):
	# List with id of empty pitcher
	pitcher_list = [0x9a7, 0xff6, 0xff7]
	found = False
	for pitcher in pitcher_list:
		if player.countresource(pitcher):
			found = True
			break
	if not found:
		player.socket.sysmessage(tr("You need something you can fill the milk in."))
		return False
	# consume the pitcher and add a full one
	player.soundeffect(0x30)
	player.useresource(1, pitcher)
	milk = wolfpack.additem("9ad")
	if not wolfpack.utilities.tobackpack( milk, player ):
		milk.update()
	char.settag( "resourcecount", char.gettag("resourcecount") - 1)
	player.socket.sysmessage(tr("You melk the cow and fill the milk in an empty pitcher."))
	return

def chicken(char, player):
	player.socket.sysmessage(tr("You get some eggs"))
	eggs = wolfpack.additem("9b5")
	if not wolfpack.utilities.tobackpack( eggs, player ):
		eggs.update()
	char.settag( "resourcecount", char.gettag("resourcecount") - 1)

# Table of IDs mapped to handler functions
actions = {
		# Baseid: [function, max resource amount]
		"cow": [cow, 30],
		"chicken": [chicken, 10]
	 }

def onShowPaperdoll(char, player):
	if not actions.has_key( char.baseid ):
		return False
	if not player.canreach(char, 2):
		player.socket.clilocmessage(500295) # You are too far away to do that.
		return False
	# Look if there's something to get
	if not checkresource(char, player):
		return False

	return actions[ char.baseid ][0]( char, player )

def checkresource(object, player):
	if not object.hastag('resourcecount') or object.gettag('resourcecount') <= 0:
		player.socket.sysmessage( tr("There's nothing you can get from it.") )
		return False
	return True

#
# Add the timer for checking the objects
#
def onLoad():
	global magic
	magic = random.random()

	wolfpack.addtimer(CHECKINTERVAL, gainresObject, [magic], False)

#
# Reset the magic
#
def onUnload():
	global magic
	magic = random.random()

#
# Register the object with the global registry
#
def onAttach(obj):
	register(obj)

#
# Unregister the object with the global registry
#
def onDetach(obj):
	unregister(obj)
