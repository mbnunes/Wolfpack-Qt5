
from client import *

def initialize(dialog):
	container = dialog.findByName("CharacterCreationContainer")
	
	charcreation1 = Gui.createDialog("CharacterCreation1")
	if charcreation1:
		container.addControl(charcreation1)
		
	charcreation2 = Gui.createDialog("CharacterCreation2")
	if charcreation2:
		container.addControl(charcreation2)

	charcreation3 = Gui.createDialog("CharacterCreation3")
	if charcreation3:
		container.addControl(charcreation3)

	charcreation4 = Gui.createDialog("CharacterCreation4")
	if charcreation4:
		container.addControl(charcreation4)
		