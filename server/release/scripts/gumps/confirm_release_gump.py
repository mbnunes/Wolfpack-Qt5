import wolfpack
from wolfpack.gumps import cGump
from speech.pets import release

def ConfirmReleaseGump(char, pet):
	dialog = cGump( 0, 0, 0, 0, 40 )
	dialog.setCallback(response)
	dialog.startPage(0)

	dialog.addBackground(5054, 270, 120)
	dialog.addResizeGump(10, 10, 3000,250, 100)

	dialog.addXmfHtmlGump(20, 15, 230, 60, 1046257, 1, 1 ) # Are you sure you want to release your pet?

	dialog.addButton( 20, 80, 4005, 4007, 1 )
	dialog.addXmfHtmlGump(55, 80, 75, 20, 1011011, 0,0 ) # CONTINUE

	dialog.addButton(135, 80, 4005, 4007, 0)
	dialog.addXmfHtmlGump(170, 80, 75, 20, 1011012, 0,0 ) # CANCEL

	dialog.setArgs([pet.serial])
	dialog.send(char.socket)

def response(player, args, choice):
	pet = wolfpack.findchar(args[0])
	if not pet:
		return False
	if choice.button == 0:
		return False
	if choice.button == 1:
		release(pet)
	return True
