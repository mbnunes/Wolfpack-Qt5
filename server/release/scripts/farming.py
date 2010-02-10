import wolfpack
import random
import time
from wolfpack import console, tr
from wolfpack.consts import *
import crops
import wolfpack.utilities
import beverage
from environment import actions

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
	if object.baseid in actions.keys():	
		OBJECTS.append(object.serial)

#
# This unregisters an object
#
def unregister(object):
	serial = object.serial
	try:
		OBJECTS.remove(serial)
	except:
		pass
#
# This is the list with all known object serials
#
OBJECTS = []

#
# Add the timer for checking the objects
#
def onLoad():
	global magic_farming
	magic_farming = random.random()

	wolfpack.addtimer(CHECKINTERVAL, gainresObject, [magic_farming], False)

#
# Reset the magic_farming
#
def onUnload():
	global magic_farming
	magic_farming = random.random()

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

#
# This prevents too many growth checks
#
magic_farming = random.random()

#
# This is the timer callback used to initiate a growth check for 
# all known objects
#
def gainresObject(obj, args):
	if args[0] != magic_farming:
		return # This is an outdated timer

	# Notify the log that we're running a growth check
	console.log(LOG_MESSAGE, "Running farming growth check for %u objects.\n" % len(OBJECTS))

	# Copy the list of known object serials and
	# start the subprocessing function
	processGainresObject(None, ( OBJECTS[:], 0, magic_farming ))

#
# This function is both, a timer callback and a function that can be normally
# called to process a list of object serials and check their growth.
# The args are:
# 1. The list or tuple with all object serials
# 2. The index in that list to start processing at
#
def processGainresObject(obj, args):
	if args[2] != magic_farming:
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
		wolfpack.addtimer(CHECKDELAY, processGainresObject, [objectlist, upperindex, magic_farming], False)

	# We're done, so queue the next check
	else:
		wolfpack.addtimer(CHECKINTERVAL, gainresObject, [magic_farming], False)

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
	if not item.baseid in ACTIONS:
		return False
	if not player.canreach(item, 2):
		player.socket.clilocmessage(500295) # You are too far away to do that.
		return False
	# Look if there's something to get
	if not checkresource(item, player):
		return True

	return actions[ item.baseid ][0]( player, item )

def appletree(char, item):
	apple = wolfpack.additem("9d0")
	if not wolfpack.utilities.tobackpack( apple, char ):
		apple.update()
	char.soundeffect(0x57)
	item.settag( "resourcecount", item.gettag("resourcecount") - 1)
	return True

def peachtree(char, item):
	peach = wolfpack.additem("9d2")
	if not wolfpack.utilities.tobackpack( peach, char ):
		peach.update()
	char.soundeffect(0x57)
	item.settag( "resourcecount", item.gettag("resourcecount") - 1)
	return True

def peartree(char, item):
	pear = wolfpack.additem("994")
	if not wolfpack.utilities.tobackpack( pear, char ):
		pear.update()
	char.soundeffect(0x57)
	item.settag( "resourcecount", item.gettag("resourcecount") - 1)
	return True

def grapevines(char, item):
	grape = wolfpack.additem("9d1")
	if not wolfpack.utilities.tobackpack( grape, char ):
		grape.update()
	char.soundeffect(0x57)
	item.settag( "resourcecount", item.gettag("resourcecount") - 1)
	return True

def onShowPaperdoll(char, player):
	if not char.baseid in ACTIONS:
		return False
	if not player.canreach(char, 2):
		player.socket.clilocmessage(500295) # You are too far away to do that.
		return False
	# Look if there's something to get
	if not checkresource(char, player):
		return True

	return actions[ char.baseid ][0]( char, player, 1 )

def cow(char, player, amount):
	# List with id of empty pitcher
	pitcher_list = [0x9a7, 0xff6, 0xff7]
	found = 0
	for pitcher in pitcher_list:
		foundn = player.countresource(pitcher)
		found += foundn
	if not found:
		player.socket.sysmessage(tr("You need something you can fill the milk in."))
		return False
	player.soundeffect(0x30)
	if found < amount:
		amount = found
	amount_down = amount
	for pitcher in pitcher_list:
		# consume as many pitchers as needed
		if amount_down > 0:
			if player.useresource(amount, pitcher):
				amount_down -= 1
		else:
			break
	# add as many pitchers as consumed (item.amount not possible, because not generic)
	for i in range(0, amount):
		milk = wolfpack.additem("9ad")
		if not wolfpack.utilities.tobackpack( milk, player ):
			milk.update()
	char.settag( "resourcecount", char.gettag("resourcecount") - amount)
	player.socket.sysmessage(tr("You melk the cow and fill the milk in an empty pitcher."))
	return

def chicken(char, player, amount):
	player.socket.sysmessage(tr("You get some eggs"))
	player.soundeffect(0x57)
	eggs = wolfpack.additem("9b5")
	if not wolfpack.utilities.tobackpack( eggs, player ):
		eggs.amount = amount
		eggs.update()
	char.settag( "resourcecount", char.gettag("resourcecount") - amount)

def checkresource(object, player):
	if not object.hastag('resourcecount') or object.gettag('resourcecount') <= 0:
		player.socket.sysmessage( tr("There's nothing you can get from it.") )
		return False
	return True

# Table of IDs mapped to handler functions
ACTIONS = {
		# Baseid: [function, max resource amount]
		# NPCs
		"cow": [cow, 30],
		"chicken": [chicken, 10],

		# Items (trees)
		"d96": [appletree, 40],
		"d9a": [appletree, 40],
		"d9e": [peachtree, 40],
		"da2": [peachtree, 40],
		"da6": [peartree, 40],
		"daa": [peartree, 40],
		"d1b": [grapevines, 10],
		"d1c": [grapevines, 10],
		"d1d": [grapevines, 10],
		"d1e": [grapevines, 10],
		"d1f": [grapevines, 10],
		"d20": [grapevines, 10],
		"d21": [grapevines, 10],
		"d22": [grapevines, 10],
		"d23": [grapevines, 10],
		"d24": [grapevines, 10],
		"d23": [grapevines, 10],
		"3d12": [grapevines, 10]
	 }
