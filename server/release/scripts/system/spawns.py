
from wolfpack import console
from wolfpack.consts import *
import wolfpack
from threading import Thread, Event, Lock
import time
import random
from system.debugging import DEBUG_SPAWNS

#
# Triggers the spawn of an object
#
def spawn(spawner, spawntype, spawndef, current, area):
	try:
		if spawntype == 1:
			npc = wolfpack.addnpc(spawndef, spawner.pos)
			npc.settag('spawner', spawner.serial)
			npc.wandertype = 3
			npc.wanderx1 = spawner.pos.x
			npc.wandery1 = spawner.pos.y
			npc.wanderradius = area
			npc.addscript( 'system.spawns' )
			npc.update()
		elif spawntype == 0:
			pass
	except:
		if( spawner ):
			console.log(LOG_WARNING, "Invalid spawner: 0x%x.\n" % spawner.serial)
		return

	spawner.settag('current', current + 1)

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
			valid = 0
		if valid and not item.hastag('spawntype') or not item.hastag('spawndef'):
			valid = 0
	
		if not valid:
			if DEBUG_SPAWNS == 1:
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
			mininterval = 1
			maxinterval = 1
			if item.hastag('interval'):
				interval = item.gettag('interval')
				if ',' in interval:
					(mininterval, maxinterval) = interval.split(',', 1)
					try:
						mininterval = max(1, int(mininterval))
						maxinterval = max(1, int(maxinterval))
						if maxinterval < mininterval:
							temp = mininterval
							mininterval = maxinterval
							maxinterval = temp
					except:
						mininterval = 1
						maxinterval = 1
	
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
				item.settag('nextspawn', currenttime + random.randint(mininterval, maxinterval) * 60 * 1000)
				continue
	
			elif current >= maximum:
				item.deltag('nextspawn')
				continue
	
			if nextspawn <= currenttime:
				spawn(item, spawntype, spawndef, current, area)
				#console.log(LOG_MESSAGE, "SPAWNTIME REACHED!")
				item.deltag('nextspawn')
	
			if DEBUG_SPAWNS == 1:
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

			process = self.unprocessed[:50]
			self.unprocessed = self.unprocessed[50:]

			if DEBUG_SPAWNS == 1:
				console.log(LOG_MESSAGE, "Found %u spawn items." % len(process))

			wolfpack.queuecode(processSpawns, (process, ))

			self.processed += process

			if len(self.unprocessed) == 0:
				self.unprocessed = self.processed
				self.processed = []

			self.unlock()
			self.stopped.wait(15.0) # Every 15 seconds.

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
# of the spawning item is decremented.
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
	except:
		return 0

	return 0
