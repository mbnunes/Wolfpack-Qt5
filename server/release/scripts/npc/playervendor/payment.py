
#
# This file manages payment checks for all registered vendors
# It could also be used to export information about known player
# vendors to an external file.
#

from threading import Thread, Event, Lock
import wolfpack
from wolfpack.consts import *
import time
import npc.playervendor

# Check interval
interval = float(300) # In Seconds (5 minutes)

# Dont change
paymentthread = None

#
# This thread does nothing more than manage a list of vendor serials
# and check for their correct payment every x minutes.
#
class PaymentThread(Thread):
	# Initialize this thread
	def __init__(self):
		Thread.__init__(self)
		self.stopped = Event() # Cancel Event
		self.mutex = Lock()
		self.processed = True
		self.serials = []
	
	# This code runs in a separate thread
	def run(self):
		global interval

		while not self.stopped.isSet():
			# Get the processing state
			self.mutex.acquire()
			processed = self.processed
			self.mutex.release()

			# Only gather new data if the old has been processed
			if processed and wolfpack.isrunning():
				self.mutex.acquire()
				serials = self.serials[:] # Copy
				self.processed = False
				self.mutex.release()
				
				wolfpack.queuecode(checkPayment, (serials,))

			# Wait until canceled or the next interval
			self.stopped.wait(interval)

#
# This function checks the given list of serials for 
# cash-day vendors
#
def checkPayment(serials):	
	partition = serials[:50] # Process 50 vendors
	rest = serials[50:]
		
	# Queue up the rest of the serials
	if len(rest) > 0:
		wolfpack.queuecode(checkPayment, (rest,))
	else:
		# Finished processing
		global paymentthread
		paymentthread.mutex.acquire()
		paymentthread.processed = True
		paymentthread.mutex.release()
	
	for serial in partition:
		vendor = wolfpack.findchar(serial)
		if vendor:
			npc.playervendor.checkPayment(vendor)

#
# Register vendor
#
def registerVendor(vendor):
	global paymentthread
	serial = vendor.serial
	paymentthread.mutex.acquire()
	if not serial in paymentthread.serials:
		paymentthread.serials.append(serial)
	paymentthread.mutex.release()

#
# Unregister vendor
#
def unregisterVendor(vendor):
	global paymentthread
	serial = vendor.serial
	paymentthread.mutex.acquire()
	while serial in paymentthread.serials:
		paymentthread.serials.remove(serial)
	paymentthread.mutex.release()

#
# Start the processing thread
#
def onLoad():
	# Create the worker thread
	global paymentthread
	paymentthread = PaymentThread()
	paymentthread.start()

#
# Stop the processing thread
#
def onUnload():
	global paymentthread
	if paymentthread:
		paymentthread.stopped.set()
		time.sleep(0.01) # Sleep a little to give the thread time to exit
		if paymentthread:
			paymentthread.join()
		paymentthread = None
