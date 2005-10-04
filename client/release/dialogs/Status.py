
from client import *

def strengthLockClicked(button):
	status = Player.status
	print status.strengthlock
	print status.dexteritylock
	print status.intelligencelock
	Network.setStatLock(0, 2)
	
def dexterityLockClicked(button):
	pass
	
def intelligenceLockClicked(button):
	pass		

def initialize(dialog):
	# Connect to the statlock buttons
	lock = dialog.findByName("StrengthLock")
	if lock:
		connect(lock, "onButtonPress(cControl*)", strengthLockClicked)	
	lock = dialog.findByName("DexterityLock")
	if lock:
		connect(lock, "onButtonPress(cControl*)", dexterityLockClicked)
	lock = dialog.findByName("IntelligenceLock")
	if lock:
		connect(lock, "onButtonPress(cControl*)", intelligenceLockClicked)
