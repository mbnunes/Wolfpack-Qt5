
from client import *

### Weapon ability button doubleclicked
def abilityButtonClicked(button):
	pass
	
### Virtue button doubleclicked
def virtueButtonClicked(button):
	pass
	
### Profile button doubleclicked
def profileButtonClicked(button):
	pass
	
### Party button doubleclicked
def partyButtonClicked(button):
	pass

### Help button clicked
def helpButtonClicked(button):
	Network.requestHelp()

### Logout button clicked
def logoutButtonClicked(button):
	dialog = Gui.findByName("LogoutConfirm", False)
	
	if dialog:
		return
		
	dialog = Gui.createDialog("LogoutConfirm")
	dialog.x = (Gui.width - dialog.width) / 2
	dialog.y = (Gui.height - dialog.height) / 2
	Gui.addControl(dialog)

## Peace button has been clicked
def peaceButtonClicked(button):
	Network.changeWarmode(True)
	
## War button has been clicked
def warButtonClicked(button):
	Network.changeWarmode(False)

### Connect the dialog buttons to their handler functions
def initialize(dialog):
	if dialog.objectName == "PlayerPaperdoll":
		connect(dialog.findByName("LogoutButton"), "onButtonPress(cControl*)", logoutButtonClicked)
		connect(dialog.findByName("HelpButton"), "onButtonPress(cControl*)", helpButtonClicked)
		connect(dialog.findByName("WarButton"), "onButtonPress(cControl*)", warButtonClicked)
		connect(dialog.findByName("PeaceButton"), "onButtonPress(cControl*)", peaceButtonClicked)
		
		connect(dialog.findByName("AbilityButton"), "onDoubleClick(cControl*)", abilityButtonClicked)
		connect(dialog.findByName("VirtueButton"), "onDoubleClick(cControl*)", virtueButtonClicked)
		connect(dialog.findByName("ProfileButton"), "onDoubleClick(cControl*)", profileButtonClicked)
		connect(dialog.findByName("PartyButton"), "onDoubleClick(cControl*)", partyButtonClicked)
		
		paperdoll = dialog.findByName("Paperdoll")
		paperdoll.owner = Player
		
		warmodeChanged(Player.warmode, dialog)

def warmodeChanged(mode, dialog = None):
	if not dialog:
		dialog = Gui.findByName("PlayerPaperdoll")
	if dialog:
		warButton = dialog.findByName("WarButton")
		peaceButton = dialog.findByName("PeaceButton")
		
		if mode:
			peaceButton.visible = False
			warButton.visible = True
		else:
			peaceButton.visible = True
			warButton.visible = False	

connect(Network, "warmodeChanged(bool)", warmodeChanged)
