
from client import *

def okPressed(button):
	dialog = Gui.findByName("LogoutConfirm")
	Gui.queueDelete(dialog)
	UoSocket.disconnect()
	
def cancelPressed(button):
	dialog = Gui.findByName("LogoutConfirm")
	Gui.queueDelete(dialog)

def initialize(dialog):
	connect(dialog.findByName("Ok"), "onButtonPress(cControl*)", okPressed)
	connect(dialog.findByName("Cancel"), "onButtonPress(cControl*)", cancelPressed)

