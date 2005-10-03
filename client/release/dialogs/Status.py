
from client import *

def getStatus(mobile):
	if mobile == Player:
		dialog = Gui.findByName("PlayerStatus")
	else:
		dialog = Gui.findByName("Status%x" % mobile.serial)
	return dialog

#
# Close the status associated with the mobile
#
def mobileDestroyed(mobile):
	dialog = getStatus(mobile)
		
	if dialog:
		Gui.queueDelete(dialog)

#
# Update the status for the given mobile or 
# otherwise disconnect the mobile if the status
# has been closed.
#
def updateStatus():
	mobile = sender()
	dialog = getStatus(mobile)
	
	if not dialog:
		return
	
	print "Stats changed for mobile 0x%x" % mobile.serial
	
	label = dialog.findByName("HealthLabel")	
	label.text = str(mobile.health)
	label = dialog.findByName("MaxHealthLabel")
	label.text = str(mobile.maxhealth)
	label = dialog.findByName("StaminaLabel")	
	label.text = str(mobile.stamina)
	label = dialog.findByName("MaxStaminaLabel")
	label.text = str(mobile.maxstamina)
	label = dialog.findByName("ManaLabel")
	label.text = str(mobile.mana)
	label = dialog.findByName("MaxManaLabel")
	label.text = str(mobile.maxmana)

def initialize(dialog, mobile):
	# To avoid double-connections
	disconnect(mobile, updateStatus)
	disconnect(mobile, mobileDestroyed)
	
	# Connect the mobile with the update status function
	connect(mobile, "statsChanged()", updateStatus)
	connect(mobile, "destroyed(QObject*)", mobileDestroyed)
