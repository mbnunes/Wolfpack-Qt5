
import wolfpack
from wolfpack import console, tr
from system.makemenus import MakeAction, MakeMenu, findmenu
from magic import castSpell

class PolymorphAction(MakeAction):
	def __init__(self, parent, title, bodyid):
		MakeAction.__init__(self, parent, title)
		self.bodyid = bodyid

	def make(self, player, arguments, nodelay=0):
		MakeAction.make(self, player, arguments, nodelay)
		castSpell(player, 56, 0, [self.bodyid])

class PolymorphMenu(MakeMenu):
	def __init__(self, id, parent, title):
		MakeMenu.__init__(self, id, parent, title)
		self.gumptype = 0xf4132a42 # This should be unique
		self.name_makelast = tr("Select Last")

def showmenu(player):
	menu = findmenu("POLYMORPH")
	menu.send(player)

# Populate the Polymorph menu
menu = PolymorphMenu("POLYMORPH", None, 1015234) # <center>Polymorph Selection Menu</center>

animals = PolymorphMenu("POLYMORPH_ANIMALS", menu, 1015235) # Animals
PolymorphAction(animals, 1015236, 0xD0) # Chicken
PolymorphAction(animals, 1015237, 0xD9) # Dog
PolymorphAction(animals, 1015238, 0xE1) # Wolf
PolymorphAction(animals, 1015239, 0xD6) # Panther
PolymorphAction(animals, 1015240, 0x1D) # Gorilla
PolymorphAction(animals, 1015241, 0xD3) # Black Bear
PolymorphAction(animals, 1015242, 0xD4) # Grizzly Bear
PolymorphAction(animals, 1015243, 0xD5) # Polar Bear
PolymorphAction(animals, 1015244, 0x190) # Human Male
animals.sort()

monster = PolymorphMenu("POLYMORPH_MONSTER", menu, 1015245) # Monsters
PolymorphAction(monster, 1015246, 0x33) # Slime
PolymorphAction(monster, 1015247 , 0x11) # Orc
PolymorphAction(monster, 1015248, 0x21) # Lizard Man
PolymorphAction(monster, 1015249, 0x04) # Gargoyle
PolymorphAction(monster, 1015250, 0x01) # Ogre
PolymorphAction(monster, 1015251, 0x36) # Troll
PolymorphAction(monster, 1015252, 0x02) # Ettin
PolymorphAction(monster, 1015253, 0x09) # Daemon
PolymorphAction(monster, 1015254, 0x191) # Human Female
monster.sort()

menu.sort()
