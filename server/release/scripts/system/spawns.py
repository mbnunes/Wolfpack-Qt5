
from wolfpack import console
from wolfpack.consts import *
import wolfpack
from threading import Thread, Event, Lock
import time
import random
from system.debugging import DEBUG_SPAWNS

SHOWNPCICON = 1

SpawnGemsCheckAmount = int(wolfpack.settings.getnumber("Game Speed", "Spawngems Checked Per Cycle", 50, 1))
SpawnGemsCheckTime = float(wolfpack.settings.getnumber("Game Speed", "Spawngems Check Time", 15, 1))

#
# Triggers the spawn of an object
#
def spawn(spawner, spawntype, spawndef, current, area):
	try:
		if spawntype == 1:
			npc = wolfpack.addnpc(spawndef, spawner.pos)
			npc.settag('spawner', spawner.serial)
			# NPCs with Area 0 have to stay on Origin Point
			if not area == 0:
				npc.wandertype = 3
			else:
				npc.wandertype = 0
			npc.wanderx1 = spawner.pos.x
			npc.wandery1 = spawner.pos.y
			npc.wanderradius = area
			npc.addscript( 'system.spawns' )

			# Now Adv. Settings
			if spawner.hastag('color'):
				npc.skin = int(spawner.gettag('color'))

			if spawner.hastag('modstr'):
				npc.strength = npc.strength * float(spawner.gettag('modstr'))

			if spawner.hastag('moddex'):
				npc.dexterity = npc.dexterity * float(spawner.gettag('moddex'))

			if spawner.hastag('modint'):
				npc.intelligence = npc.intelligence * float(spawner.gettag('modint'))

			if spawner.hastag('modmaxhits'):
				npc.maxhitpoints = npc.maxhitpoints * float(spawner.gettag('modmaxhits'))

			if spawner.hastag('modmaxstam'):
				npc.maxstamina = npc.maxstamina * float(spawner.gettag('modmaxstam'))

			if spawner.hastag('modmaxmana'):
				npc.maxmana = npc.maxmana * float(spawner.gettag('modmaxmana'))

			if spawner.hastag('name'):
				npc.name = spawner.gettag('name')

			if spawner.hastag('nameprefix'):
				npc.settag('name.prefix', spawner.gettag('nameprefix'))

			if spawner.hastag('namesuffix'):
				npc.settag('name.suffix', spawner.gettag('namesuffix'))

			npc.update()

			# Updating visual of Spawn
			if SHOWNPCICON:
				bodyinfo = wolfpack.bodyinfo(npc.id)
				spawner.id = bodyinfo['figurine']
				spawner.update()
		elif spawntype == 0:
			newitem = wolfpack.additem(spawndef)
			newitem.pos = spawner.pos
			newitem.settag('spawner', spawner.serial)
			newitem.decay = 0
			newitem.addscript('system.spawns')
			newitem.update()
	except:
		if( spawner ):
			console.log(LOG_WARNING, "Invalid spawner: 0x%x.\n" % spawner.serial)
		return

	spawner.settag('current', current + 1)
	spawner.resendtooltip()

#
# Run this in the mainloop
#
def processSpawns(process):
	# Process the designated partition
	for i in range(0, len(process)):
		item = wolfpack.finditem(process[i])
	
		# Check if the spawn is valid.
		valid = item != None
		if valid and not item.hasscript( 'spawngem' ):
			valid = False
		if valid and (not item.hastag('spawntype') or not item.hastag('spawndef')):
			valid = False
	
		if not valid:
			if DEBUG_SPAWNS:
				console.log(LOG_WARNING, "Invalid spawn item: 0x%x.\n" % item.serial)
			pass
		else:
			spawntype = int(item.gettag('spawntype')) # 0: Items, 1: NPCs
			spawndef = str(item.gettag('spawndef')) # Definition
	
			# This is either how far the npcs will wander
			# or how far from the spawn the items will be spawned.
			area = 0
			if item.hastag('area'):
				area = int(item.gettag('area'))
	
			# This is a minimum/maximum spawn interval in minutes
			mininterval = 5
			if item.hastag('mininterval'):
				try:
					mininterval = int(item.gettag('mininterval'))
				except:
					mininterval = 10
					
			maxinterval = mininterval
			if item.hastag('maxinterval'):
				try:
					maxinterval = int(item.gettag('maxinterval'))
				except:
					maxinterval = 10
	
			# Normalize min and maxinterval (min. is 1)
			if mininterval < 1:
				mininterval = 1
			if maxinterval < 1:
				maxinterval = 1
			
			if mininterval > maxinterval:
				temp = maxinterval
				maxinterval = mininterval
				mininterval = temp
				
			# Currently / Maximimum spawned by this gem
			current = 0
			if item.hastag('current'):
				try:
					current = int(item.gettag('current'))
				except:
					current = 0
	
			maximum = 1
			if item.hastag('maximum'):
				try:
					maximum = int(item.gettag('maximum'))
				except:
					maximum = 1
	
			nextspawn = 0
			if item.hastag('nextspawn'):
				try:
					nextspawn = int(item.gettag('nextspawn'))
				except:
					nextspawn = 0
	
			currenttime = wolfpack.tickcount()
	
			# It's possible that the spawntime got too far into the future (server-reload etc.)
			# reset it here.
			if nextspawn - currenttime > maxinterval * 60 * 1000:
				nextspawn = 0
	
			# If we didn't have a spawntime set yet.
			if nextspawn == 0 and current < maximum:
				delay = random.randint(mininterval, maxinterval) * 60 * 1000
				item.settag('nextspawn', currenttime + delay)
				if DEBUG_SPAWNS:
					console.log(LOG_MESSAGE, "Set spawntime for spawngem 0x%x to %u miliseconds in the future.\n" % (item.serial, delay))
				continue
	
			elif current >= maximum:
				item.deltag('nextspawn')
				continue
	
			if nextspawn <= currenttime:
				spawn(item, spawntype, spawndef, current, area)
				#console.log(LOG_MESSAGE, "SPAWNTIME REACHED!")
				item.deltag('nextspawn')
	
			if DEBUG_SPAWNS:
				console.log(LOG_MESSAGE, "Valid Spawnpoint: %x, Cur/Max: %u/%u, Def: %s, Type: %u, Interval: %u,%u, Time: %d/%d" % \
					(item.serial, current, maximum, spawndef, spawntype, mininterval, maxinterval, currenttime, nextspawn))

#
# The threadclass for spawns
#
class SpawnThread(Thread):
	#
	# Initialize the thread.
	#
	def __init__(self):
		Thread.__init__(self)
		self.stopped = Event()
		self.mlock = Lock()
		self.unprocessed = [] # List of serials
		self.processed = [] # List of processed spawngems

	#
	# Lock the thread-own mutex.
	#
	def lock(self):
		self.mlock.acquire()

	#
	# Unlock the thread own mutex.
	#
	def unlock(self):
		self.mlock.release()

	#
	# Cancel the thread.
	#
	def cancel(self):
		self.stopped.set()

	#
	# Run the spawnloop
	#
	def run(self):
		while not self.stopped.isSet():
			self.lock()

			process = self.unprocessed[:SpawnGemsCheckAmount]
			self.unprocessed = self.unprocessed[SpawnGemsCheckAmount:]

			if DEBUG_SPAWNS:
				console.log(LOG_MESSAGE, "Found %u spawn items." % len(process))

			wolfpack.queuecode(processSpawns, (process, ))

			self.processed += process

			if len(self.unprocessed) == 0:
				self.unprocessed = self.processed
				self.processed = []

			self.unlock()
			self.stopped.wait(SpawnGemsCheckTime) # Every 15 seconds default.

#
# Our global thread object.
#
thread = None

#
# Start the spawn thread.
#
def onLoad():
	global thread
	thread = SpawnThread()
	thread.start()

#
# Den Synchthread stoppen
#
def onUnload():
	global thread
	if thread:
		thread.cancel()
		time.sleep(0.03)
		thread.join()
		thread = None

#
# Register a spawn
#
def register(spawn):
	global thread
	if thread:
		serial = spawn.serial
		thread.lock()
		if serial not in thread.unprocessed and serial not in thread.processed:
			thread.unprocessed.append(spawn.serial)
		thread.unlock()

#
# Unregister a spawn
#
def unregister(spawn):
	global thread
	if thread:
		serial = spawn.serial
		thread.lock()
		while serial in thread.unprocessed:
			thread.unprocessed.remove(serial)
		while serial in thread.processed:
			thread.processed.remove(serial)
		thread.unlock()

#
# When this script is attached to a
# npc and he is removed, the current counter
# of the spawning item is decremented
# the same if an item is picked up
#
def onDelete(object):
	if not object.hastag('spawner'):
		return 0

	try:
		spawner = wolfpack.finditem(int(object.gettag('spawner')))
		if spawner and spawner.hastag('current'):
			current = int(spawner.gettag('current'))
			current -= 1

			if current <= 0:
				spawner.deltag('current')
			else:
				spawner.settag('current', current)

			spawner.resendtooltip()
	except:
		return 0

	object.removescript('system.spawns')
	object.deltag('spawner')
	return 0

#
# If it's an item, trigger for respawn is if it's picked up
#
def onPickup(player, item):
	onDelete(item)
	item.decay = 1
	return 0
