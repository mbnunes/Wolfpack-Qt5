
from client import *

def initialize(dialog):	
	charcreation1 = Gui.createDialog("CharacterCreation1")
	if charcreation1:
		dialog.addControl(charcreation1)
		
	charcreation2 = Gui.createDialog("CharacterCreation2")
	if charcreation2:
		dialog.addControl(charcreation2)

	charcreation3 = Gui.createDialog("CharacterCreation3")
	if charcreation3:
		dialog.addControl(charcreation3)
