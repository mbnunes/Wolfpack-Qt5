
from client import *

def strengthLockClicked(button):
	status = Player.status
	Network.setStatLock(0, (status.strengthlock + 1) % 3)
	
def dexterityLockClicked(button):
	status = Player.status
	Network.setStatLock(1, (status.dexteritylock + 1) % 3)
	
def intelligenceLockClicked(button):
	status = Player.status
	Network.setStatLock(2, (status.intelligencelock + 1) % 3)

def toggleStatusSize(dialog):
	if dialog.objectName == "PlayerStatus":		
		Network.showSmallStatus()
	else:
		Network.showLargeStatus()

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

	if dialog.objectName == "PlayerStatus" or dialog.objectName == "SmallPlayerStatus":
		connect(dialog, "onDoubleClick(cControl*)", toggleStatusSize)
